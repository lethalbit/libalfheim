// SPDX-License-Identifier: BSD-3-Clause

#include <libalfheim/config.hh>

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(libalfheim, m) {

	m.doc() = "A library for parsing and generating various object file formats.";
	m.attr("__version__") = Alfheim::Config::version;

}
