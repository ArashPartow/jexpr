#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <variant>
#include <map>

#include "exprtk.hpp"

#include "nlohmann/json.hpp"

namespace JExpr{

using ParserErrorEntry = exprtk::parser_error::type;
using Error = std::vector<ParserErrorEntry>;
using ValueOrError = std::variant<double, Error>;

std::string nice_error(const exprtk::parser_error::type& error) {
    return (std::to_string(error.token.position) + 
    exprtk::parser_error::to_str(error.mode) +
    error.diagnostic);
}

auto collect_variables(const std::string& expr) {
    std::vector<std::string> variable_list;
    bool got_ok = exprtk::collect_variables(expr, variable_list);
    if (!got_ok) {
        throw std::invalid_argument("Could not collect variables");
    }
    return variable_list;
}

template<typename T>
auto get_missing_entities(const std::string& expression_string) {
    std::vector<std::string> missing_entities;
    // All the options are disabled in the parser, most importantly the implicit multiplication, 
    // to avoid x[0] being parsed as x*0 if x is accidentally a value rather than a vector
    using settings_t = typename exprtk::parser<T>::settings_t;
    exprtk::parser<T> parser{ settings_t::e_bracket_check };

    using parser_t = exprtk::parser<T>;

    typedef typename parser_t::dependent_entity_collector::symbol_t symbol_t;

    exprtk::expression<T> m_expression;

    //Collect only variable and function symbols
    parser.dec().collect_variables() = true;
    parser.dec().collect_functions() = true;

    if (!parser.compile(expression_string, m_expression))
    {
        // error....
        auto build_error = [](auto& parser) {
            Error error_list;
            for (std::size_t i = 0; i < parser.error_count(); ++i) {
                error_list.emplace_back(parser.get_error(i));
            };
            return error_list;
        };
        Error errs = build_error(parser);
        for (auto& err : errs) {
            if (err.diagnostic.find("Undefined symbol") != std::string::npos) {
                missing_entities.push_back(err.token.value);
            }
            else {
                throw std::invalid_argument(nice_error(err));
            }
        }
    }
    return missing_entities;

}

class BaseExpression {
public:
    const std::string m_expr;
public:
    BaseExpression(const std::string& expr) : m_expr(expr) {};
    auto collect_variables() const {
        std::vector<std::string> variable_list;
        bool got_ok = exprtk::collect_variables(m_expr, variable_list);
        if (!got_ok) {
            throw std::invalid_argument("Could not collect variables");
        }
        exprtk::symbol_table<double> symbol_table;
        symbol_table.add_constants();
        return variable_list;
    }
};



template<typename Container>
auto strjoin(const Container& container) {
    std::string o;
    if (container.size() > 0) { o = container[0]; }
    if (container.size() > 1) {
        for (auto i = 1; i < container.size(); ++i) { o += "," + container[i]; }
    };
    return o;
}

template<typename T>
class Expression{

public:
    using NumericTypes = std::variant<T, std::vector<T>>;

    exprtk::symbol_table<T> symbol_table;
    exprtk::expression<T> m_expression;

    // All the options are disabled in the parser, most importantly the implicit multiplication, 
    // to avoid x[0] being parsed as x*0 if x is accidentally a value rather than a vector
    exprtk::parser<T> parser{static_cast<std::size_t>(0)};

    const BaseExpression m_e;
    const std::string m_expr;
    const std::vector<std::string> m_variables;
    using val_map_type = std::map<std::string, NumericTypes>;
    std::map<std::string, T*> variable_references;
    std::map<std::string, std::vector<T>*> vector_references;

    val_map_type freezer_val_map;

    bool _is_frozen = false;

    auto get_expr() const{
        return m_expr;
    }
    
    Expression(const std::string &expr) : m_e(BaseExpression(expr)), m_expr(expr), m_variables(m_e.collect_variables()) {
        //collect(expr);
        symbol_table.add_constants();
        m_expression.register_symbol_table(symbol_table);
    } ;
    Expression(BaseExpression && e) : m_e(e), m_expr(e.m_expr), m_variables(e.collect_variables()) {

        //// Build and register the symbol table with placeholders for the variables
        symbol_table.add_constants();
        m_expression.register_symbol_table(symbol_table);
    };

    void build_symbol_table(const val_map_type& val_map, std::vector<std::string> &not_provided_variables, std::vector<std::string> &bad_type_variables){
        for (const auto& k : m_variables) {
            if (k == "pi") { continue; }

            // Check for unmatched variables
            if (val_map.find(k) == val_map.end()) {
                not_provided_variables.push_back(k);
                // Could be more than one missing variable, so don't terminate immediately.
            }
            else {
                auto adder = [&](const auto& val) {
                    bool is_constant = true;
                    using ValType = std::decay_t<decltype(val)>; // Remove const and & to yield base object type
                    if constexpr (std::is_same_v<ValType, T>) {
                        if (_is_frozen) {
                            if (symbol_table.get_variable(k) == 0) {
                                bad_type_variables.push_back(k);
                            }
                            else {
                                // It's there, just refresh the reference
                                variable_references[k] = &const_cast<T&>(val);
                            }
                        }
                        else {
                            symbol_table.add_variable(k, const_cast<T&>(val), is_constant);
                            variable_references[k] = &const_cast<T&>(val);
                        }
                    }
                    else if constexpr (std::is_same_v<ValType, std::vector<T>>) {
                        if (_is_frozen) {
                            if (symbol_table.get_vector(k) == 0) { // Can't get the vector
                                bad_type_variables.push_back(k);
                            }
                            else {
                                // Figure out how to change the pointer to the array
                                // It has to keep its knowledge about the AST, but just change the underlying location the view is pointing to
                                // Otherwise we have to re-parse, which is what we are trying to avoid
                                auto view = symbol_table.get_vector(k);
                                auto N1 = view->size();
                                auto N2 = val.size();
                                // TODO: error if not the same size!!
                                for (auto i = 0; i < N1; ++i){
                                    *(view->data() + i) = val[i];
                                }
                            }
                        }
                        else {
                            symbol_table.add_vector(k, const_cast<std::vector<T>&>(val));
                            vector_references[k] = &const_cast<std::vector<T>&>(val);
                        }
                    }
                    else {
                        throw std::invalid_argument("Bad (and unknown) argument type");
                    }
                };
                std::visit(adder, val_map.at(k));
            }
        }
    }
    void freeze(const val_map_type& val_map){

        // Make a copy of the value map
        freezer_val_map = val_map;

        // Call the evaluation function, to force a parse, setting pointers properly
        (*this)(freezer_val_map);

        // Set flag for freezing
        _is_frozen = true;
    }
    auto get_variables() const{
        return m_variables;
    }
    ValueOrError operator()(const val_map_type &val_map){
        
        std::vector<std::string> not_provided_variables;
        std::vector<std::string> bad_type_variables;

        if (!_is_frozen) {
            symbol_table.clear();
            symbol_table.add_constants();
        }
        
        build_symbol_table(val_map, not_provided_variables, bad_type_variables);

        // Check for unmatched variables
        if (!not_provided_variables.empty()){
            std::string baddies = strjoin(not_provided_variables);
            throw std::invalid_argument("These variables were not provided: " + baddies);
        }

        // Check for wrong type variables (if frozen)
        if (!bad_type_variables.empty()) {
            std::string baddies = strjoin(bad_type_variables);
            throw std::invalid_argument("Bad type for these variables: " + baddies);
        }
        
        auto build_error = [](auto &parser){
            Error error_list;
            using error_type = exprtk::parser_error::type;
            for (std::size_t i = 0; i < parser.error_count(); ++i){
                auto error = parser.get_error(i);
                std::string modestring = exprtk::parser_error::to_str(error.mode);
                error_list.emplace_back(error);
            };
            return error_list;
        };

        // Compilation is a no-op if frozen, otherwise parse the expression
        bool is_ok = true;
        if (!_is_frozen){
            is_ok = parser.compile(m_expr, m_expression);
        }

        if (!is_ok){
            auto errs = build_error(parser);
            return errs;
        }
        else{
            return m_expression.value();
        }
    }
    auto JSON_to_args(const std::string &j){
        // Define our container
        val_map_type val_map;
        // Create JSON object from the JSON-formatted string
        nlohmann::json doc = nlohmann::json::parse(j);
        // Iterate over the provided arguments, copying into the C++ variant map from the JSON one
        for (auto & kv : doc.at("args").items()){
            auto val = kv.value();
            if (val.is_number()){
                val_map[kv.key()] = val.get<T>();
            }
            else if (val.is_array()){
                val_map[kv.key()] = val.get<std::vector<T>>();
            }
        }
        return val_map;
    }
    std::string callJSON(const std::string &j){
        try{
            // Call the expression object with these arguments after converting from JSON to std::map
            auto o = (*this)(JSON_to_args(j));
            // Return the successful evaluation in JSON format
            return std::to_string(std::get<T>(o));
        }
        catch(std::exception &e){
            std::cout << e.what() << std::endl;
            return std::to_string(-1.0);
        }
    }
};

};
