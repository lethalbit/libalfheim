# SPDX-License-Identifier: BSD-3-Clause

library_hdrs_pe32 = files([
	'types.hh',
])

library_srcs += files([

])

if not meson.is_subproject()
	install_headers(
		library_hdrs_pe32,
		subdir: 'libalfheim' / 'pe32'
	)
endif
