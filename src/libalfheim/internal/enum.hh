// SPDX-License-Identifier: BSD-3-Clause
/* internal/enum.hh - C++ Enum utilities */
#pragma once
#if !defined(libalfheim_internal_enum_hh)
#define libalfheim_internal_enum_hh

#include <type_traits>

namespace Alfheim::Internal {

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator|(T l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) | static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator|=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) | static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator&(T l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) & static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator&=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) & static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator^(T l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) ^ static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator^=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) ^ static_cast<U>(r)
		);
	}

	template<typename T>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T>, T>
	operator~(T l) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			~static_cast<U>(l)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator|(T l, V r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) | r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator|=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) | r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator&(T l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) & r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator&=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) & r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator^(T l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return static_cast<T>(
			static_cast<U>(l) ^ r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_enum_v<T> && std::is_integral_v<V>, T>
	operator^=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<T>;
		return l = static_cast<T>(
			static_cast<U>(l) ^ r
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator|(T l, V r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return static_cast<T>(
			l | static_cast<U>(r)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator|=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return l = static_cast<T>(
			l | static_cast<U>(r)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator&(T l, T r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return static_cast<T>(
			l & static_cast<U>(r)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator&=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return l = static_cast<T>(
			l & static_cast<U>(r)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator^(T l, T r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return static_cast<T>(
			l ^ static_cast<U>(r)
		);
	}

	template<typename T, typename V>
	[[nodiscard]]
	constexpr typename std::enable_if_t<std::is_integral_v<T> && std::is_enum_v<V>, T>
	operator^=(T& l, T r) noexcept {
		using U = typename std::underlying_type_t<V>;
		return l = static_cast<T>(
			l ^ static_cast<U>(r)
		);
	}

}

#endif /* libalfheim_internal_enum_hh */
