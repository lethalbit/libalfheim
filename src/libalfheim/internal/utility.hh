// SPDX-License-Identifier: BSD-3-Clause
/* internal/utility.hh - Various helpers and such */
#pragma once
#if !defined(libalfheim_internal_utility_hh)
#define libalfheim_internal_utility_hh

#include <libalfheim/config.hh>
#include <libalfheim/internal/defs.hh>

#include <cstdint>
#include <type_traits>
#include <array>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#	include <unistd.h>
#else
#	include <io.h>
#endif

namespace Alfheim::Internal {	namespace Units {
		/* IEC Units*/
		constexpr std::uint64_t operator ""_KiB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1024; }
		constexpr std::uint64_t operator ""_MiB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1048576; }
		constexpr std::uint64_t operator ""_GiB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1073741824; }
		constexpr std::uint64_t operator ""_TiB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1099511627776; }
		constexpr std::uint64_t operator ""_PiB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1125899906842624; }

		/* SI Units */
		constexpr std::uint64_t operator ""_KB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1000; }
		constexpr std::uint64_t operator ""_MB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1000000; }
		constexpr std::uint64_t operator ""_GB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1000000000; }
		constexpr std::uint64_t operator ""_TB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1000000000000; }
		constexpr std::uint64_t operator ""_PB(const unsigned long long value) noexcept { return std::uint64_t(value) * 1000000000000000; }
	}

	namespace Types {
		#if defined(_WINDOWS)
			using mode_t = std::int32_t;
			using stat_t = struct ::_stat64;
		#	if defined(_WIN64)
			using ssize_t = __int64;
			using off_t   = std::int64_t;
		#	else
			using ssize_t = int;
			using off_t   = std::int32_t;
		#	endif
		#else
		using stat_t  = struct ::stat;
		using mode_t  = std::int32_t;
		using ssize_t = typename std::make_signed<std::size_t>::type;
		using off_t   = std::int64_t;
		#endif
	}

	template<typename T>
	struct has_nullable_ctor final {
		template<typename U>
		static std::true_type _ctor(decltype(U(std::nullptr_t()))*);
		template<typename U>
		static std::false_type _ctor(...);

		static const bool value = std::is_same<decltype(_ctor<T>(nullptr)), std::true_type>::value;
	};

	template<typename T>
	constexpr bool has_nullable_ctor_v = has_nullable_ctor<T>::value;

	template<typename>
	struct is_vector : std::false_type {};

	template<typename T, typename U>
	struct is_vector<std::vector<T, U>> : std::true_type {};

	template<typename>
	struct is_array : std::false_type {};

	template<typename T, std::size_t len>
	struct is_array<std::array<T, len>> : std::true_type {};

	template<typename T>
	constexpr inline bool is_vector_v = is_vector<T>::value;

	template<typename T>
	constexpr inline bool is_array_v = is_array<T>::value;

	template<typename T>
	using value_t = typename T::value_type;


	template<typename T, bool = std::is_unsigned_v<T>>
	struct promoted_type;

	template<typename T>
	struct promoted_type<T, true> { using type = std::uint32_t; };

	template<typename T>
	struct promoted_type<T, false> { using type = std::int32_t; };

	template<>
	struct promoted_type<std::uint64_t, true> { using type = std::uint64_t; };

	template<>
	struct promoted_type<std::int64_t, false> { using type = std::int64_t; };

	template<>
	struct promoted_type<std::size_t, !std::is_same_v<std::uint64_t, std::size_t>> { using type = std::size_t; };

	template<>
	struct promoted_type<Types::ssize_t, std::is_same_v<std::int64_t, Types::ssize_t>> { using type = Types::ssize_t; };

	template<typename T>
	using promoted_type_t = typename promoted_type<T>::type;

	/* Helper methods */
	[[nodiscard]]
	inline constexpr bool is_be() noexcept {
		return Alfheim::Config::target_endian == Alfheim::Config::endian_t::big;
	}

	[[nodiscard]]
	inline constexpr bool is_le() noexcept {
		return  Alfheim::Config::target_endian == Alfheim::Config::endian_t::little;
	}
}

#endif /* libalfheim_internal_utility_hh */
