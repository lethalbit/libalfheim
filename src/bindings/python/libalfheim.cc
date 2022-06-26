// SPDX-License-Identifier: BSD-3-Clause

#include <libalfheim/config.hh>

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(libalfheim, m) {

	m.doc() = "A library for parsing and generating various object file formats.";
	m.attr("__version__") = Alfheim::Config::version;

	[[maybe_unused]]
	auto aout = m.def_submodule("aout", "Alfheim module for parsing and generating a.out binaries");
	[[maybe_unused]]
	auto coff = m.def_submodule("coff", "Alfheim module for parsing and generating COFF binaries");
	[[maybe_unused]]
	auto ecoff = m.def_submodule("ecoff", "Alfheim module for parsing and generating ECOFF binaries");
	[[maybe_unused]]
	auto elf = m.def_submodule("elf", "Alfheim module for parsing and generating ELF32/ELF64 binaries");
	[[maybe_unused]]
	auto macho = m.def_submodule("macho", "Alfheim module for parsing and generating Mach-O binaries");
	[[maybe_unused]]
	auto os360 = m.def_submodule("os360", "Alfheim module for parsing and generating os360 binaries");
	[[maybe_unused]]
	auto pe32 = m.def_submodule("pe32", "Alfheim module for parsing and generating PE32/PE32+ binaries");
	[[maybe_unused]]
	auto xcoff = m.def_submodule("xcoff", "Alfheim module for parsing and generating XCOFF binaries");

}
