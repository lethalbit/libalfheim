// SPDX-License-Identifier: BSD-3-Clause
/* internal/mmap.hh - RAII mmap wrapper */
#pragma once
#if !defined(libalfheim_internal_mmap_hh)
#define libalfheim_internal_mmap_hh

#include <libalfheim/internal/defs.hh>

#if !defined(_WINDOWS)
#	include <unistd.h>
#	include <sys/mman.h>
#else
#	include <io.h>
#	include <io.h>
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	undef WIN32_LEAN_AND_MEAN
#	undef min
#	undef max
#endif

#include <type_traits>
#include <utility>
#include <memory>
#include <array>
#include <string>
#include <string_view>
#include <cstdint>
#include <stdexcept>
#include <cstring>

#include <libalfheim/config.hh>

#include <libalfheim/internal/bits.hh>
#include <libalfheim/internal/utility.hh>



namespace Alfheim::Internal {
#if defined(_WINDOWS)
	constexpr DWORD PROT_READ{PAGE_READONLY};
	constexpr DWORD PROT_WRITE{PAGE_READWRITE};
	constexpr auto  MADV_SEQUENTIAL{0};
	constexpr auto  MADV_WILLNEED{0};
	constexpr auto  MADV_DONTDUMP{0};
#endif
	struct mmap_t final {
	private:
		std::size_t _len{0};
	#if defined(_WINDOWS)
		HANDLE _mapping{INVALID_HANDLE_VALUE};
	#endif
		void* _addr{nullptr};
		std::int32_t _fd{-1};
	#if !defined(_WINDOWS)
		[[nodiscard]]
		mmap_t(const mmap_t& map, const std::size_t len, const std::int32_t prot,
			const std::int32_t flags = MAP_SHARED, void* const addr = nullptr
		) noexcept : _len{len}, _addr{[&]() noexcept -> void* {
			const auto ptr{::mmap(addr, len, prot, flags, map._fd, 0)};
			return (ptr == MAP_FAILED) ? nullptr : ptr;
		}()}, _fd{-1} { /* NOP */ }
	#endif

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<
			std::is_pod_v<T> && !has_nullable_ctor_v<T> && !std::is_same_v<T, void*>,
		T*>
		index(const std::size_t idx) const {
			if (idx < _len) {
				const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
				return new (reinterpret_cast<void*>(addr + (idx * sizeof(T)))) T{};
			}
			throw std::out_of_range("mmap_t index out of range");
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<
			has_nullable_ctor_v<T> && !std::is_same_v<T, void*>,
		T*>
		index(const std::size_t idx) const {
			if (idx < _len) {
				const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
				return new (reinterpret_cast<void*>(addr + (idx * sizeof(T)))) T{nullptr};
			}
			throw std::out_of_range("mmap_t index out of range");
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_same_v<T, void*>, void*>
		index(const std::size_t idx) const {
			if (idx < _len) {
				const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
				return reinterpret_cast<void*>(addr + idx );
			}
			throw std::out_of_range("mmap_t index out of range");
		}

	#if defined(_WINDOWS)
		constexpr static DWORD clear_prot(const DWORD prot) noexcept {
			if (prot & PAGE_READWRITE)
				return prot & ~PAGE_READONLY;
			return prot;
		}

		constexpr static DWORD prot_to_access(const DWORD prot) noexcept {
			if (prot & PAGE_READWRITE)
				return FILE_MAP_WRITE;
			else if (prot & PAGE_READONLY)
				return FILE_MAP_READ;
			else if (prot & PAGE_WRITECOPY)
				return FILE_MAP_WRITE;
			return {};
		}
	#endif

	public:
		[[nodiscard]]
		constexpr mmap_t() noexcept = default;

		mmap_t(const mmap_t&) = delete;
		mmap_t& operator=(const mmap_t&) = delete;

		[[nodiscard]]
		bool operator==(const mmap_t& b) const noexcept {
			return _fd == b._fd && _addr == b._addr && _len == b._len;
		}
		[[nodiscard]]
		bool operator!=(const mmap_t& b) const noexcept { return !(*this == b); }

	#if !defined(_WINDOWS)
		[[nodiscard]]
		mmap_t(const std::int32_t fd, const std::size_t len, const std::int32_t prot,
			const std::int32_t flags = MAP_SHARED, void* const addr = nullptr
		) noexcept : _len{len}, _addr{[&]() noexcept -> void* {
			const auto ptr{::mmap(addr, len, prot, flags, fd, 0)};
			return (ptr == MAP_FAILED) ? nullptr : ptr;
		}()}, _fd{fd} { /* NOP */ }

		[[nodiscard]]
		mmap_t(mmap_t&& map) noexcept : mmap_t{} { swap(map); }
		void operator=(mmap_t&& map) noexcept { swap(map); }
		~mmap_t() noexcept {
			if (_addr)
				::munmap(_addr, _len);
			if (_fd != -1)
				::close(_fd);
		}

		[[nodiscard]]
		constexpr bool valid() const noexcept { return _addr; }
	#endif

		void swap(mmap_t& map) noexcept {
			std::swap(_fd, map._fd);
			std::swap(_addr, map._addr);
			std::swap(_len, map._len);
		#if defined(_WINDOWS)
			std::swap(_mapping, map._mapping);
		#endif
		}

		[[nodiscard]]
		std::size_t length() const noexcept { return _len; }

		[[nodiscard]]
		mmap_t dup(const std::int32_t prot, const std::size_t len, const std::int32_t flags, void* const addr) const noexcept {
			if (!valid())
				return {};
			return {*this, len, prot, flags, addr};
		}

		[[nodiscard]]
		bool chperm(const std::int32_t prot) const noexcept {
			return ::mprotect(_addr, _len, prot) == 0;
		}

		template<typename T>
		[[nodiscard]]
		T* address() noexcept { return static_cast<T*>(_addr); }

		template<typename T>
		[[nodiscard]]
		const T* address() const noexcept { return static_cast<T*>(_addr); }

		void* address(const std::size_t offset) noexcept { return index<void*>(offset); }
		const void* address(const std::size_t offset) const noexcept { return index<const void*>(offset); }

		template<typename T>
		[[nodiscard]]
		T* operator[](const std::size_t idx) { return index<T>(idx); }
		template<typename T>
		[[nodiscard]]
		const T* operator[](const std::size_t idx) const { return index<const T>(idx); }

		template<typename T>
		[[nodiscard]]
		T* at(const std::size_t idx) { return index<T>(idx); }
		template<typename T>
		[[nodiscard]]
		const T* at(const std::size_t idx) const { return index<const T>(idx); }

		[[nodiscard]]
		std::uintptr_t numeric_address() const noexcept {
			return reinterpret_cast<std::uintptr_t>(_addr);
		}

		[[nodiscard]]
		bool lock() const noexcept {
			return lock(_len);
		}

		[[nodiscard]]
		bool lock(const std::size_t len) const noexcept {
			return ::mlock(_addr, len) == 0;
		}

		[[nodiscard]]
		bool lock_at(const std::size_t idx, const std::size_t len) const noexcept {
			const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
			return ::mlock(reinterpret_cast<void*>(addr + idx), len) == 0;
		}

		[[nodiscard]]
		bool unlock() const noexcept {
			return unlock(_len);
		}

		[[nodiscard]]
		bool unlock(const std::size_t len) const noexcept {
			return ::munlock(_addr, len) == 0;
		}

		[[nodiscard]]
		bool unlock_at(const std::size_t idx, const std::size_t len) const noexcept {
			const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
			return ::munlock(reinterpret_cast<void*>(addr + idx), len) == 0;
		}

		[[nodiscard]]
		bool remap(const std::int32_t flags, const std::size_t len) noexcept {
			const auto old_len = _len;
			_len = len;
			return (_addr = ::mremap(_addr, old_len, _len, flags)) != MAP_FAILED;
		}

		[[nodiscard]]
		bool remap(const std::int32_t flags, const std::size_t len, const std::uintptr_t addr) noexcept {
			const auto old_len = _len;
			_len = len;
			const void* wanted_addr = reinterpret_cast<void*>(addr);
			return (_addr = ::mremap(_addr, old_len, _len, flags, wanted_addr)) != MAP_FAILED;
		}

	#if !defined(_WINDOWS)
		[[nodiscard]]
		bool sync(const std::int32_t flags = MS_SYNC | MS_INVALIDATE) const noexcept {
			return sync(flags, _len);
		}

		[[nodiscard]]
		bool sync(const std::int32_t flags, const std::size_t len) const noexcept {
			return ::msync(_addr, len, flags) == 0;
		}

		[[nodiscard]]
		bool advise(const std::int32_t advice) const noexcept {
			return advise(advice, _len);
		}

		[[nodiscard]]
		bool advise(const std::int32_t advice, const std::size_t len) const noexcept {
			return ::madvise(_addr, len, advice) == 0;
		}

		[[nodiscard]]
		bool advise_at(const std::int32_t advice, const std::size_t len, const std::size_t idx) const noexcept {
			const auto addr = reinterpret_cast<std::uintptr_t>(_addr);
			return ::madvise(reinterpret_cast<void*>(addr + idx), len, advice) == 0;
		}

	#endif
	};

	inline void swap(mmap_t& a, mmap_t& b) noexcept { a.swap(b); }
}

#endif /* libalfheim_internal_mmap_hh */
