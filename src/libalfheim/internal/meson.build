# SPDX-License-Identifier: BSD-3-Clause

library_hdrs_internal += files([
	'defs.hh',
	'utility.hh',
])

library_srcs += files([

])

if not meson.is_subproject()
	install_headers(
		library_hdrs_internal,
		subdir: 'libalfheim' / 'internal'
	)
endif
