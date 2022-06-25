// SPDX-License-Identifier: BSD-3-Clause
/* internal/zlib.hh - zlib RAII wrapper */
#pragma once
#if !defined(libalfheim_internal_zlib_hh)
#define libalfheim_internal_zlib_hh

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <cstring>
#include <type_traits>
#include <functional>

#include <libalfheim/config.hh>

#include <libalfheim/internal/bits.hh>
#include <libalfheim/internal/defs.hh>
#include <libalfheim/internal/fd.hh>
#include <libalfheim/internal/utility.hh>

extern "C" {
	#define ZLIB_CONST
	#include <zlib.h>
	#include <zconf.h>
	#undef ZLIB_CONST
}

namespace Alfheim::Internal {
	using namespace Alfheim::Internal::Units;

	struct zlib_t final {
	private:
		enum struct zmode_t : std::uint8_t {
			inflate = 0x00U,
			deflate = 0x01U
		};

		template<std::uint64_t chunk_size = 8_KiB>
		struct zctx_t final {
		private:
			zlib_t::zmode_t _mode;
			z_stream _stream;
			std::array<uint8_t, chunk_size> _buffer;
			bool _eos;
		public:
			[[nodiscard]]
			zctx_t(zlib_t::zmode_t mode) noexcept :
				_mode{mode}, _stream{}, _buffer{}, _eos{} {
				if (_mode == zlib_t::zmode_t::inflate) {
					_eos = (::inflateInit(&_stream) != Z_OK);
				} else {
					_eos = (::deflateInit(&_stream, Z_DEFAULT_COMPRESSION) != Z_OK);
				}
			}

			~zctx_t() noexcept {
				if (_mode == zlib_t::zmode_t::inflate) {
					_eos = (::inflateEnd(&_stream) != Z_OK);
				} else {
					_eos = (::deflateEnd(&_stream) != Z_OK);
				}
			}

			[[nodiscard]]
			bool valid() const noexcept { return !_eos; }

			[[nodiscard]]
			std::optional<std::vector<std::uint8_t>> process(const std::uint8_t* data, const std::size_t len) noexcept {
				using processor_t = bool(zctx_t&, std::vector<std::uint8_t>&,const std::uint8_t*,const std::size_t);

				const auto n_chunks = (len + chunk_size - 1) / chunk_size;
				const std::function<processor_t> processor{(_mode == zlib_t::zmode_t::inflate) ? &zctx_t::inflate : &zctx_t::deflate};

				std::vector<std::uint8_t> _output{};

				for(std::size_t idx{}; idx < n_chunks && !_eos; ++idx) {
					const auto *const buffer = data + (chunk_size * idx);
					const auto buffer_len = (idx == n_chunks - 1) ? len - ((n_chunks - 1) * chunk_size) : chunk_size;

					if (!processor(*this, _output, buffer, buffer_len)) {
						return std::nullopt;
					}
				}
				// TODO: lonk input buffers don't like this
				[[maybe_unused]]
				const auto _ = reset();
				return std::make_optional(_output);
			}

			[[nodiscard]]
			std::optional<std::vector<std::uint8_t>> process(const std::vector<std::uint8_t>& data) noexcept {
				return process(data.data(), data.size());
			}

			template<std::size_t len>
			[[nodiscard]]
			std::optional<std::vector<std::uint8_t>> process(const std::array<std::uint8_t, len>& data) noexcept {
				return process(data.data(), len);
			}

		private:
			[[nodiscard]]
			bool inflate(std::vector<std::uint8_t>& out, const std::uint8_t* buff, const std::size_t buff_size) noexcept {
				_stream.next_in = buff;
				_stream.avail_in = buff_size;
				_stream.avail_out = 0;

				while (_stream.avail_in && (_stream.avail_out == 0) && !_eos) {
					_stream.next_out = _buffer.data();
					_stream.avail_out = _buffer.size();

					const auto ret = ::inflate(&_stream, Z_NO_FLUSH);

					if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR)
						return false;
					else if (ret == Z_STREAM_END)
						_eos = true;

					const auto copy_len = _buffer.size() - _stream.avail_out;
					const auto offset = out.size();
					out.resize(out.size() + copy_len);
					std::memcpy(out.data() + offset, _buffer.data(), copy_len);

				}
				return true;
			}

			[[nodiscard]]
			bool deflate(std::vector<std::uint8_t>& out, const std::uint8_t* buff, const std::size_t buff_size) noexcept {
				_stream.next_in = buff;
				_stream.avail_in = buff_size;
				_stream.avail_out = 0;

				while (_stream.avail_in && (_stream.avail_out == 0) && !_eos) {
					_stream.next_out = _buffer.data();
					_stream.avail_out = _buffer.size();

					const auto ret = ::deflate(&_stream, (buff_size < chunk_size) ? Z_FINISH : Z_NO_FLUSH);

					if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR)
						return false;
					else if (ret == Z_STREAM_END)
						_eos = true;

					const auto copy_len = _buffer.size() - _stream.avail_out;
					const auto offset = out.size();
					out.resize(out.size() + copy_len);
					std::memcpy(out.data() + offset, _buffer.data(), copy_len);

				}
				return true;
			}

			[[nodiscard]]
			bool reset() noexcept {
				_eos = false;
				if (_mode == zlib_t::zmode_t::inflate)
					return inflateReset(&_stream) == Z_OK;
				else
					return deflateReset(&_stream) == Z_OK;

			}
		};

		zctx_t<> _inflate;
		zctx_t<> _deflate;

	public:
		[[nodiscard]]
		zlib_t() noexcept :
			_inflate{zlib_t::zmode_t::inflate},
			_deflate{zlib_t::zmode_t::deflate}
		{ /* NOP */ }

		[[nodiscard]]
		bool valid() const noexcept { return _inflate.valid() && _deflate.valid(); }

		template<typename T, std::size_t len>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T> && !is_array_v<T> && !is_vector_v<T>, std::optional<T>>
		inflate(const std::array<std::uint8_t, len>& data) noexcept {
			return inflate<T>(data.data(), len);
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T> && !is_array_v<T> && !is_vector_v<T>, std::optional<T>>
		inflate(const std::vector<std::uint8_t>& data) noexcept {
			return inflate<T>(data.data(), data.size());
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T> && !is_array_v<T> && !is_vector_v<T>, std::optional<T>>
		inflate(const std::uint8_t* data, const std::size_t len) noexcept {
			T _tmp{};

			if(auto res = _inflate.process(data, len)) {
				if (res->size() > sizeof(T))
					return std::nullopt;
				std::memcpy(&_tmp, res->data(), res->size());
				return std::make_optional(_tmp);
			} else {
				return std::nullopt;
			}
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<value_t<T>> && is_vector_v<T> && !is_array_v<T>, std::optional<T>>
		inflate(const std::vector<std::uint8_t>& data) noexcept {
			return inflate<T>(data.data(), data.size());
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<value_t<T>> && is_vector_v<T> && !is_array_v<T>, std::optional<T>>
		inflate(const std::uint8_t* data, const std::size_t len) noexcept {
			T objs{};
			if (auto res = _inflate.process(data, len)) {
				const auto obj_count = (res->size() + sizeof(value_t<T>) - 1) / sizeof(value_t<T>);
				objs.resize(obj_count);

				std::memcpy(objs.data(), res->data(), res->size());
				return std::make_optional(objs);
			} else {
				return std::nullopt;
			}
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<value_t<T>> && is_array_v<T> && !is_vector_v<T>, std::optional<T>>
		inflate(const std::vector<std::uint8_t>& data) noexcept {
			return inflate<T>(data.data(), data.size());
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<value_t<T>> && is_array_v<T> && !is_vector_v<T>,  std::optional<T>>
		inflate(const std::uint8_t* data, const std::size_t data_len) noexcept {
			T objs{};

			if (auto res = _inflate.process(data, data_len)) {
				if (res->size() > sizeof(value_t<T>) * objs.size())
					return std::nullopt;
				std::memcpy(objs.data(), res->data(), res->size());
				return std::make_optional(objs);
			} else {
				return std::nullopt;
			}
		}

		template<std::size_t len>
		[[nodiscard]]
		std::optional<std::vector<std::uint8_t>>
		inflate(const std::array<std::uint8_t, len>& data) noexcept {
			return _inflate.process(data);
		}

		[[nodiscard]]
		std::optional<std::vector<std::uint8_t>>
		inflate(const std::vector<std::uint8_t>& data) noexcept {
			return _inflate.process(data);
		}

		[[nodiscard]]
		std::optional<std::vector<std::uint8_t>>
		inflate(const std::uint8_t* data, const std::size_t len) noexcept {
			return _inflate.process(data, len);
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T>, std::optional<std::vector<std::uint8_t>>>
		deflate(T& obj) noexcept {
			std::array<uint8_t, sizeof(T)> buff{};
			std::memcpy(buff.data(), &obj, sizeof(T));
			return _deflate.process(buff);
		}

		template<typename T, std::size_t len>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T>, std::optional<std::vector<std::uint8_t>>>
		deflate(const std::array<T, len>& objs) noexcept {
			std::array<std::uint8_t, len * sizeof(T)> buff{};
			std::memcpy(buff.data(), objs.data(), len);
			return _deflate.process(buff);
		}

		template<typename T>
		[[nodiscard]]
		std::enable_if_t<std::is_pod_v<T>, std::optional<std::vector<std::uint8_t>>>
		deflate(const std::vector<T>& objs) noexcept {
			std::vector<std::uint8_t> buff{sizeof(T) * objs.size()};
			std::memcpy(buff.data(), objs.data(), sizeof(T) * objs.size());
			return _deflate.process(buff);
		}

		[[nodiscard]]
		std::optional<std::vector<std::uint8_t>>
		deflate(const std::vector<std::uint8_t>& data) noexcept {
			return _deflate.process(data);
		}

		template<std::size_t len>
		[[nodiscard]]
		std::optional<std::vector<std::uint8_t>>
		deflate(const std::array<std::uint8_t, len>& data) noexcept {
			return _deflate.process(data);
		}
	};
}

#endif /* libalfheim_internal_zlib_hh */
