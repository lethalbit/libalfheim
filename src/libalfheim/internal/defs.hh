// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#if !defined(libalfheim_internal_defs_hh)
#define libalfheim_internal_defs_hh

#if defined(_MSC_VER) && !defined(_WINDOWS)
#	define _WINDOWS 1
#endif

#ifdef _WINDOWS
#	ifdef LIBALFHEIM_BUILD_INTERNAL
		// NOLINTNEXTLINE
#		define LIBALFHEIM_CLS_API __declspec(dllexport)
#	else
		// NOLINTNEXTLINE
#		define LIBALFHEIM_CLS_API __declspec(dllimport)
#	endif
	// NOLINTNEXTLINE
#	define LIBALFHEIM_API extern LIBALFHEIM_CLS_API
	// NOLINTNEXTLINE
#	define LIBALFHEIM_CLS_MAYBE_API
#else
	// NOLINTNEXTLINE
#	define LIBALFHEIM_CLS_API __attribute__ ((visibility("default")))
	// NOLINTNEXTLINE
#	define LIBALFHEIM_CLS_MAYBE_API LIBALFHEIM_CLS_API
	// NOLINTNEXTLINE
#	define LIBALFHEIM_API extern LIBALFHEIM_CLS_API
#endif

#endif /* libalfheim_internal_defs_hh */
