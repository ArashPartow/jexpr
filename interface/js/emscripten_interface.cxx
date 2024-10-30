/// *********************************************************************************
///                     EMSCRIPTEN (for javascript)
/// *********************************************************************************

#include <emscripten/bind.h>
using namespace emscripten;

#include <jexpr/jexpr.hpp>
using namespace JExpr;

// Binding code
EMSCRIPTEN_BINDINGS(jexpr) {

    register_vector<double>("VectorDouble");
    register_vector<std::string>("VectorString");

    using Ex = Expression<double>;
    class_<Ex>("Expression")
        .constructor<const std::string&>()
        .property("expr", &Ex::get_expr)
        .property("variables", &Ex::get_variables)
        .function("freeze", &Ex::freeze)
        .function("callJSON", &Ex::callJSON)
        ;
}