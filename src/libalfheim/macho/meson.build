# SPDX-License-Identifier: BSD-3-Clause

library_hdrs_macho = files([
	'types.hh',
])

library_srcs += files([

])

if not meson.is_subproject()
	install_headers(
		library_hdrs_macho,
		subdir: 'libalfheim' / 'macho'
	)
endif
