# SPDX-License-Identifier: BSD-3-Clause

message('Building bindings: Python')

py = import('python')
py = py.find_installation(
	'python3',
	required: false,
	modules: []
)

pybind11 = dependency('pybind11',  required: false, version: '>=2.7.0')
if not pybind11.found()
	message('Did not find local pybind11 install, bundling')
	pybind11_wrap = subproject('pybind11', default_options: [])
	pybind11 = pybind11_wrap.get_variable('pybind11_dep')
endif

libalfheim_py = py.extension_module(
	'libalfheim',
	files([
		'libalfheim.cc'
	]),

	include_directories: [
		library_inc,
	],

	dependencies: [
		pybind11
	],

	link_with: [
		libalfheim,
	],

	cpp_args: [
		'-DLIBALFHEIM_BINDINGS_PYTHON'
	],

	install: (not meson.is_subproject())
)
