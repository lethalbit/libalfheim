// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#if !defined(libalfheim_internal_bits_hh)
#define libalfheim_internal_bits_hh

#include <libalfheim/internal/utility.hh>

#include <limits>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace Fortress::Internal {
	/* LEB128 encoding and decoding */
	template<typename T>
	[[nodiscard]]
	std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, std::vector<std::uint8_t>>
	leb128_encode(T num) {
		using U = typename std::make_unsigned<T>::type;
		using V = promoted_type_t<U>;
		std::vector<std::uint8_t> enc{};
		bool more{true};
		while (more) {
			auto byte{std::uint8_t(static_cast<V>(num) & 0x7FU)};
			num >>= 7U;

			if ((!num && !(byte & 0x40U)) || (num == -1 && (byte & 0x40U)))
				more = false;
			else
				byte |= 0x80U;
			enc.emplace_back(byte);
		}
		return enc;
	}

	template<typename T>
	[[nodiscard]]
	std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, T>
	leb128_decode(const std::vector<std::uint8_t> &vec) {
		using U = typename std::make_unsigned<T>::type;
		using V = promoted_type_t<U>;
		V enc{};
		std::size_t shift{};
		for (const auto &byte : vec) {
			enc |= V{byte & 0x7FU} << shift;
			shift += 7U;
		}
		if (shift && shift < sizeof(T) * 8U) {
			--shift;
			for (std::size_t i{1U}; i < (sizeof(T) * 8U) - shift; ++i)
				enc |= (enc & (V{1U} << shift)) << i;
		}
		return static_cast<T>(enc);
	}

	template<typename T>
	[[nodiscard]]
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, std::vector<std::uint8_t>>
	leb128_encode(const T value) {
		using U = typename std::make_unsigned<T>::type;
		using V = promoted_type_t<U>;
		auto num{static_cast<V>(value)};
		std::vector<std::uint8_t> enc{};
		do{
			std::uint8_t byte = num & 0x7FU;
			num >>= 7U;
			if (num != 0U)
				byte |= 0x80U;
			enc.emplace_back(byte);
		}
		while (num != 0U);
		return enc;
	}

	template<typename T>
	[[nodiscard]]
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
	leb128_decode(const std::vector<std::uint8_t> &vec) {
		using U = typename std::make_unsigned<T>::type;
		using V = promoted_type_t<U>;
		V enc{};
		std::size_t shift{};
		for (const auto &byte : vec) {
			enc |= V{byte & 0x7FU} << shift;
			shift += 7U;
		}
		return static_cast<T>(enc);
	}

	/* endian swapping facilities */
	[[nodiscard]]
	inline constexpr std::uint16_t swap16(const std::uint16_t x) noexcept {
		return std::uint16_t(
			((x & 0x00FFU) << 8U) |
			((x & 0xFF00U) >> 8U)
		);
	}

	[[nodiscard]]
	inline constexpr std::uint32_t swap32(const std::uint32_t x) noexcept {
		return std::uint32_t(
			((x & 0x000000FFU) << 24U) |
			((x & 0x0000FF00U) << 8U ) |
			((x & 0x00FF0000U) >> 8U ) |
			((x & 0xFF000000U) >> 24U)
		);
	}

	[[nodiscard]]
	inline constexpr std::uint64_t swap64(const std::uint64_t x) noexcept{
		return std::uint64_t(
			((x & 0x00000000000000FFUL) << 56U) |
			((x & 0x000000000000FF00UL) << 40U) |
			((x & 0x0000000000FF0000UL) << 24U) |
			((x & 0x00000000FF000000UL) << 8U ) |
			((x & 0x000000FF00000000UL) >> 8U ) |
			((x & 0x0000FF0000000000UL) >> 24U) |
			((x & 0x00FF000000000000UL) >> 40U) |
			((x & 0xFF00000000000000UL) >> 56U)
		);
	}


	template<typename T>
	[[nodiscard]]
	inline constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
	rotl(T x, const std::size_t k) noexcept {
		constexpr auto bits = std::numeric_limits<T>::digits;
		return (x << k) | (x >> (bits - k));
	}

	template<typename T>
	[[nodiscard]]
	inline constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
	rotr(T x, const std::size_t k) noexcept {
		constexpr auto bits = std::numeric_limits<T>::digits;
		return (x >> k) | (x << (bits - k));
	}


	template<std::size_t _lsb, std::size_t _msb>
	struct bitspan_t final {
		static constexpr std::size_t size = (_msb - _lsb) + 1;
		static_assert(_lsb <= _msb, "bitspan LSB must be smaller than or equal to the MSB");

		template<typename T, std::size_t _msb_ = _msb, std::size_t _lsb_ = _lsb>
		struct field final {
			using value_type = T;
			using vu_type = typename std::make_unsigned_t<value_type>;

			static constexpr auto msb = _msb_;
			static constexpr auto lsb = _lsb_;
			static constexpr std::size_t width = std::numeric_limits<vu_type>::digits;
			static_assert(msb <= width, "MSB must be less than or equal to the width of the bitspan type");
			static constexpr vu_type computed_mask = (((vu_type(1) << (vu_type(msb) + vu_type(1)) - vu_type(lsb)) - vu_type(1)) << vu_type(lsb));


			/* Get the value of this field in the given register */
			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr std::enable_if_t<!std::is_enum<V>::value, V> get(const V v) noexcept {
				return (vu_type((v) & computed_mask) >> lsb);
			}

			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr std::enable_if_t<std::is_enum<V>::value, V> get(const V v) noexcept {
				return static_cast<V>(
					vu_type((v) & computed_mask) >> lsb
				);
			}

			/* Set the value of this field in the given register */
			template<typename V>
			[[nodiscard]]
			static inline constexpr std::enable_if_t<!std::is_enum_v<V>> set(V& f, const V v) noexcept {
				using Vp = promoted_type_t<V>;
				f = V(Vp(((f) & ~computed_mask) | ((vu_type(v) << lsb) & computed_mask)));
			}

			template<typename V>
			[[nodiscard]]
			static inline constexpr std::enable_if_t<std::is_enum_v<V> set(V& f, const V v) noexcept {
				using Vt = typename std::underlying_type_t<V>;
				using Vp = promoted_type_t<Vt>;

				f = Vp(((f) & ~computed_mask) | ((vu_type(v) << lsb) & computed_mask));
			}
		};
	};

	template<std::size_t idx>
	using bit_t = bitspan_t<idx, idx>;

	namespace {
		template<std::size_t, typename...>
		struct type_at_index_t;

		template<std::size_t N, typename T, typename... U>
		struct type_at_index_t<N, T, U...> {
			using type = typename type_at_index_t<N - 1, U...>::type;
		};

		template<typename T, typename... U>
		struct type_at_index_t<0, T, U...> {
			using type = T;
		};
	}

	template<typename T, typename... U>
	struct bitfield_t final {
		using value_type = T;
		using vu_type = typename std::make_unsigned_t<value_type>;

		static constexpr auto width = std::numeric_limits<T>::digits;
		static constexpr auto size = width;
		static constexpr std::size_t field_count = sizeof... (U);

		/* Returns the field requested by index */
		template<std::size_t idx>
		using field = typename type_at_index_t<idx, U...>::type::template field<T>;

		/* This is functionally equivalent to ::fields<idx>::get() */
		template<std::size_t idx, typename V = vu_type>
		[[nodiscard]]
		static inline constexpr auto get(const V v) noexcept {
			static_assert(idx < field_count, "field index out of range");
			return field<idx>::template get<V>(v);
		}

		/* This is functionally equivalent to ::fields<idx>::set(v) */
		template<std::size_t idx, typename V>
		static inline constexpr void set(V& f, const V v) noexcept {
			static_assert(idx < field_count, "field index out of range");
			field<idx>::set(f, v);
		}

	};
}

#endif /* libalfheim_internal_bits_hh */
