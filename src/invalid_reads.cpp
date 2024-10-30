#include "exprtk.hpp"
#include <iostream>
struct ParserErrorEntry {
	std::size_t number;
	std::size_t position;
	std::string mess;
	std::string diag;
};

using Error = std::vector<ParserErrorEntry>;

auto build_error = [](auto& parser) {
	Error error_list;
	for (std::size_t i = 0; i < parser.error_count(); ++i)
	{
		auto error = parser.get_error(i);
		std::string modestring = exprtk::parser_error::to_str(error.mode);
		error_list.emplace_back(ParserErrorEntry{ i, error.token.position, modestring, error.diagnostic });
	};
	return error_list;
};

void error_dynamic_array() {

	exprtk::expression<double> expression;

	// All the options are disabled in the parser, most importantly the implicit multiplication, 
	// to avoid x[0] being parsed as x*0 if x is accidentally a value rather than a vector
	exprtk::parser<double> parser{ static_cast<std::size_t>(0) };

	std::string expression_string = "// comment \n [x[1], x[0]];";

	std::vector<double> x = { 1.1, 1.1, 1.1 };
	exprtk::symbol_table<double> symbol_table;
	double y = 4.2;
	symbol_table.add_vector("x", x);
	symbol_table.add_variable("y", y);
	expression.register_symbol_table(symbol_table);

	bool parse_ok = parser.compile(expression_string, expression);

	auto errs = build_error(parser);
	std::cout << expression.value(); 
}

int main() {
	error_dynamic_array(); // 
}
