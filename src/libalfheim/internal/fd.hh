// SPDX-License-Identifier: BSD-3-Clause
/* internal/fd.hh - RAII File wrapper */
#pragma once
#if !defined(libalfheim_internal_fd_hh)
#define libalfheim_internal_fd_hh

#include <type_traits>
#include <fcntl.h>
#include <utility>
#include <memory>
#include <array>
#include <string>
#include <string_view>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _MSC_VER
#	include <unistd.h>
#else
#	include <io.h>
#endif

#include <libalfheim/config.hh>

#include <libalfheim/internal/bits.hh>
#include <libalfheim/internal/defs.hh>
#include <libalfheim/internal/utility.hh>

// #include <libalfheim/internal/mmap.hh>

namespace Alfheim::Internal {
	namespace fs = std::filesystem;

	namespace {
	#ifdef _WINDOWS
		[[nodiscard]]
		inline Types::ssize_t fdread(cosnt std::int32_t fd, void* const buff, const std::size_t, len) noexcept {
			return read(fd, buff, std::uint32_t(len));
		}

		[[nodiscard]]
		inline Types::ssize_t fdwrite(const std::int32_t fd, const void* const buff, const std::size_t len) noexcept {
			return write(fd, buff, std::uint32_t(len));
		}

		[[nodiscard]]
		inline int fstat(const std::int32_t fd, Types::stat_t* stat) noexcept {
			return _fstat64(fd, stat);
		}

		[[nodiscard]]
		inline Types::off_t fdseek(const std::int32_t fd, const Types::off_t offset, const std::int32_t whence) noexcept {
			return _lseeki64(fd, offset, whence);
		}

		[[nodiscard]]
		inline Types::off_t fdtell(const std::int32_t fd) noexcept {
			return _telli64(fd);
		}

		[[nodiscard]]
		inline std::int32_t fdtruncate(const std::int32_t fd, const Types::off_t size) noexcept {
			return _chsize_s(fd, size);
		}
	#else
		using ::fstat;

		[[nodiscard]]
		inline Types::ssize_t fdread(const std::int32_t fd, void* const buff, const std::size_t len) noexcept {
			return ::read(fd, buff, len);
		}

		[[nodiscard]]
		inline Types::ssize_t fdwrite(const std::int32_t fd, const void* const buff, const std::size_t len) noexcept {
			return ::write(fd, buff, len);
		}

		[[nodiscard]]
		inline Types::off_t fdseek(const std::int32_t fd, const Types::off_t offset, const std::int32_t whence) noexcept {
			return ::lseek(fd, offset, whence);
		}

		[[nodiscard]]
		inline Types::off_t fdtell(const std::int32_t fd) noexcept {
			return ::lseek(fd, 0, SEEK_CUR);
		}

		[[nodiscard]]
		inline std::int32_t fdtruncate(const std::int32_t fd, const Types::off_t size) noexcept {
			return ::ftruncate(fd, size);
		}
	#endif
	}

	struct fd_t final {
	private:
		std::int32_t _fd{-1};
		bool _eof{false};
		Types::off_t _len{-1};
	public:
		constexpr fd_t() noexcept = default;
		constexpr fd_t(const std::int32_t fd) noexcept : _fd{fd} { /* NOP */ }

		fd_t(const char* const file, const int flags, const Types::mode_t mode = 0) noexcept :
			_fd{::open(file, flags, mode)} { /* NOP */ }

		fd_t(const std::string& file, const int flags, const Types::mode_t mode = 0) noexcept :
			_fd{::open(file.c_str(), flags, mode)} { /* NOP */ }

		fd_t(const fs::path& file, const int flags, const Types::mode_t mode = 0) noexcept :
			_fd{::open(file.c_str(), flags, mode)} { /* NOP */ }

		fd_t(fd_t&& fd) noexcept : fd_t{} { swap(fd); }
		fd_t(const fd_t&) = delete;
		fd_t& operator=(const fd_t&) = delete;


		~fd_t() noexcept {
			if (_fd != -1)
				::close(_fd);
		}

		void operator=(fd_t&& fd) noexcept { swap(fd); }
		[[nodiscard]]
		operator std::int32_t() const noexcept { return _fd; }
		[[nodiscard]]
		bool operator==(const std::int32_t desc) const noexcept { return _fd == desc; }
		[[nodiscard]]
		bool valid() const noexcept { return _fd != -1; }
		[[nodiscard]]
		bool is_eof() const noexcept { return _eof; }
		void invalidate() noexcept { _fd = -1; }

		void swap(fd_t& desc) {
			std::swap(_fd, desc._fd);
			std::swap(_len, desc._len);
			std::swap(_eof, desc._eof);
		}

		[[nodiscard]]
		Types::off_t seek(const Types::off_t offset, const std::int32_t whence) noexcept {
			const auto res = fdseek(_fd, offset, whence);
			_eof = (res == length());
			return res;
		}

		[[nodiscard]]
		bool seek_rel(const Types::off_t offset) noexcept {
			const auto pos = tell();
			if (pos == -1 || pos + offset < 0)
				return false;
			return seek(offset, SEEK_CUR) == (pos + offset);
		}

		[[nodiscard]]
		Types::off_t tell() const noexcept { return fdtell(_fd); }

		[[nodiscard]]
		bool head() noexcept { return seek(0, SEEK_SET) == 0; }

		[[nodiscard]]
		fd_t dup() const noexcept { return ::dup(_fd); }

		[[nodiscard]]
		bool tail() noexcept {
			const auto off = length();
			if (off < 0)
				return false;
			return seek(off, SEEK_SET) == off;
		}

		[[nodiscard]]
		Types::stat_t stat() const noexcept {
			Types::stat_t fd_stat{};
			if (!fstat(_fd, &fd_stat))
				return fd_stat;
			return {};
		}

		[[nodiscard]]
		Types::off_t length() noexcept {
			if (_len != -1)
				return _len;

			Types::stat_t fd_stat{};
			const auto res = fstat(_fd, &fd_stat);
			_len = res ? -1 : fd_stat.st_size;
			return _len;
		}

		[[nodiscard]]
		bool resize(const Types::off_t size) const noexcept {
			return fdtruncate(_fd, size) == 0;
		}

		[[nodiscard]]
		Types::ssize_t read(void* const buff, const std::size_t len, std::nullptr_t) noexcept {
			const auto res = fdread(_fd, buff, len);

			if (!res && len)
				_eof = true;

			return res;
		}

		[[nodiscard]]
		Types::ssize_t write(const void* const buff, const std::size_t len, std::nullptr_t) const noexcept {
			return fdwrite(_fd, buff, len);
		}

		[[nodiscard]]
		bool read(void* const val, const std::size_t len, std::size_t& res_len) noexcept {
			const auto res = read(val, len, nullptr);
			if (res < 0)
				return false;
			return (res_len = std::size_t(res)) == len;
		}

		[[nodiscard]]
		bool read(void* const val, const std::size_t len) noexcept {
			std::size_t res_len{0};
			return read(val, len, res_len);
		}

		[[nodiscard]]
		bool write(const void* const val, const std::size_t len) const noexcept {
			const auto res = write(val, len, nullptr);
			if (res < 0)
				return false;
			return std::size_t(res) == len;
		}

		template<typename T>
		[[nodiscard]]
		bool read(T& val) noexcept {
			return read(&val, sizeof(T));
		}

		template<typename T>
		[[nodiscard]]
		bool write(const T& val) const noexcept {
			return write(&val, sizeof(T));
		}

		template<typename T>
		[[nodiscard]]
		bool read(std::unique_ptr<T>& val) noexcept {
			return read(val.get(), sizeof(T));
		}

		template<typename T>
		[[nodiscard]]
		bool read(const std::unique_ptr<T>& val) noexcept {
			return read(val.get(), sizeof(T));
		}

		template<typename T>
		[[nodiscard]]
		bool write(const std::unique_ptr<T>& val) const noexcept {
			return write(val.get(), sizeof(T));
		}

		template<typename T>
		[[nodiscard]]
		bool read(const std::unique_ptr<T[]>& val, const std::size_t len) noexcept {
			return read(val.get(), sizeof(T) * len);
		}

		template<typename T>
		[[nodiscard]]
		bool write(const std::unique_ptr<T[]>& val, const std::size_t len) const noexcept {
			return write(val.get(), sizeof(T) * len);
		}

		template<typename T, std::size_t N>
		[[nodiscard]]
		bool read(std::array<T, N>& val) noexcept {
			return read(val.data(), sizeof(T) * N);
		}

		template<typename T, std::size_t N>
		[[nodiscard]]
		bool write(const std::array<T, N>& val) noexcept {
			return write(val.data(), sizeof(T) * N);
		}

		[[nodiscard]]
		bool write(const std::string& val) const noexcept {
			return write(val.data(), val.size());
		}

		[[nodiscard]]
		bool write(const std::string_view& val) const noexcept {
			return write(val.data(), val.size());
		}

		[[nodiscard]]
		bool read_le(std::uint16_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return read(val);
			} else {
				std::uint16_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap16(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_le(const std::uint16_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap16(val));
			}
		}

		[[nodiscard]]
		bool read_le(std::uint32_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return read(val);
			} else {
				std::uint32_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap32(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_le(const std::uint32_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap32(val));
			}
		}

		[[nodiscard]]
		bool read_le(std::uint64_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return read(val);
			} else {
				std::uint64_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap64(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_le(const std::uint64_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_le()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap64(val));
			}
		}

		template<
			typename T,
			typename = typename std::enable_if_t<
				std::is_integral_v<T> && std::is_same_v<T, bool> &&
				std::is_signed_v<T>   && sizeof(T) >= 2
			>
		>
		[[nodiscard]]
		bool read_le(T& val) noexcept {
			typename std::make_unsigned_t<T> data{};
			const auto res = read_le(data);
			val = static_cast<T>(data);
			return res;
		}

		template<
			typename T,
			typename = typename std::enable_if_t<
				std::is_integral_v<T> && std::is_same_v<T, bool> &&
				std::is_signed_v<T>   && sizeof(T) >= 2
			>
		>
		[[nodiscard]]
		bool write_le(T& val) noexcept {
			return write_le(
				static_cast<typename std::make_unsigned_t<T>>(
					val
				)
			);
		}

		[[nodiscard]]
		bool read_be(std::uint16_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return read(val);
			} else {
				std::uint16_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap16(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_be(const std::uint16_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap16(val));
			}
		}

		[[nodiscard]]
		bool read_be(std::uint32_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return read(val);
			} else {
				std::uint32_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap32(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_be(const std::uint32_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap32(val));
			}
		}

		[[nodiscard]]
		bool read_be(std::uint64_t& val) noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return read(val);
			} else {
				std::uint64_t _value{};
				const auto res = read(_value);
				val = Alfheim::Internal::swap64(_value);
				return res;
			}
		}

		[[nodiscard]]
		bool write_be(const std::uint64_t val) const noexcept {
			if constexpr (Alfheim::Internal::is_be()) {
				return write(val);
			} else {
				return write(Alfheim::Internal::swap64(val));
			}
		}

		template<
			typename T,
			typename = typename std::enable_if_t<
				std::is_integral_v<T> && std::is_same_v<T, bool> &&
				std::is_signed_v<T>   && sizeof(T) >= 2
			>
		>
		[[nodiscard]]
		bool read_be(T& val) noexcept {
			typename std::make_unsigned_t<T> data{};
			const auto res = read_be(data);
			val = static_cast<T>(data);
			return res;
		}

		template<
			typename T,
			typename = typename std::enable_if_t<
				std::is_integral_v<T> && std::is_same_v<T, bool> &&
				std::is_signed_v<T>   && sizeof(T) >= 2
			>
		>
		[[nodiscard]]
		bool write_be(T& val) noexcept {
			return write_be(
				static_cast<typename std::make_unsigned_t<T>>(
					val
				)
			);
		}
	};


	inline void swap(fd_t& fd_a, fd_t& fd_b) noexcept { fd_a.swap(fd_b); }
}


#endif /* libalfheim_internal_fd_hh */
