#include <string>
#include <iostream>
#include <vector>
#include <variant>

#include "exprtk.hpp"
#include <jexpr/jexpr.hpp>

int main()
{
    using namespace JExpr;

    Expression<double> p("7*x[1]+2");
    decltype(p)::val_map_type valmap = {
        {"x", std::vector<double>{1.3, 3, 7}},
        {"y", std::vector<double>{3.4,4,5,6}},
        {"c", std::vector<double>{88}}
    };
    auto val = p(valmap);
    
    // Sanity check...
    if (std::get<double>(val) != 23){
        throw std::invalid_argument("bad value");
    }

    std::cout << std::get<double>(val) << std::endl;
    
    for (auto i = 0; i < 10; ++i){
        std::get<std::vector<double>>(valmap["x"])[1] = i;
        val = p(valmap); std::cout << std::get<double>(val) << std::endl;
    }
    auto valmap2 = valmap;
    p.freeze(valmap2);
    for (auto i = 0; i < 10; ++i) {
        std::get<std::vector<double>>(valmap2["x"])[1] = i;
        val = p(valmap2); std::cout << std::get<double>(val) << std::endl;
    }
    return 0;
}

