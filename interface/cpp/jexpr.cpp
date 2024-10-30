#include <jexpr/jexpr.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

using namespace JExpr;
namespace py = pybind11;

void init_jexpr(py::module& m) {

    using E = Expression<double>;
    py::class_<E>(m, "Expression")
        .def(py::init<const std::string &>())
        .def("freeze", &E::freeze)
        .def("__call__", &E::operator(), py::is_operator());

    py::class_<ParserErrorEntry>(m, "ParserErrorEntry")
        .def_readonly("diagnostic", &ParserErrorEntry::diagnostic)
        .def_readonly("column_no", &ParserErrorEntry::column_no)
        .def_readonly("error_line", &ParserErrorEntry::error_line)
        .def_readonly("line_no", &ParserErrorEntry::line_no)
        .def("__repr__", [](const ParserErrorEntry &e){ return e.line_no + ": " + e.diagnostic;})
        ;
}

PYBIND11_MODULE(jexpr, m) {
    m.doc() = "JEXPR: JSON Mathematical Expressions";
    m.attr("__version__") = JEXPRVERSION;
    init_jexpr(m);
}
