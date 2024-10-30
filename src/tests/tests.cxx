#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "jexpr/jexpr.hpp"

using Catch::Approx;

using JExp = JExpr::Expression<double>;

const auto PI = exprtk::details::numeric::constant::pi;

TEST_CASE("Simple expressions") {
	SECTION("comment") {
		CHECK(std::get<double>(JExp("// I'm a comment\nreturn [3.1];")({})) == Approx(3.1));
	}
	SECTION("trivial") {
		CHECK(std::get<double>(JExp("3.4+8.7")({})) == Approx(12.1));
	}
	SECTION("times PI") {
		CHECK(std::get<double>(JExp("3.4*pi")({})) == Approx(3.4 * PI));
	}
	SECTION("simple variable") {
		JExp::val_map_type values = {
			{"x", 1.2}
		};
		CHECK(std::get<double>(JExp("3.4+x")(values)) == Approx(3.4 + 1.2));
	}
	SECTION("wrong variable provided") {
		JExp::val_map_type values = { {"y", 1.2} };
		CHECK_THROWS(std::get<double>(JExp("3.4+x")(values)));
	}
	SECTION("variable of type vector but should be double") {
		std::vector<double> x = { 1.1, 1.1, 1.1 };
		JExp::val_map_type values = {
			{"x", x}
		};
		auto dd = JExp("3.4+x")(values);
		CHECK(std::get<double>(JExp("3.4+x")(values)) == Approx(3.4 + 1.1)); // Only the first value is returned
	}
	SECTION("dynamic vector accesssed out of bounds") {
		std::vector<double> x = { 1.1, 1.1, 1.1 };
		JExp::val_map_type values = {
			{"x", x}
		};
		CHECK_THROWS(std::get<double>(JExp("3.4+x[10]")(values)) == Approx(3.4 + 1.2));
	}
	SECTION("static vector accesssed out of bounds") {
		JExp::val_map_type values = {};
		CHECK_THROWS(std::get<double>(JExp("var x[3] := {1.1, 1.1, 1.1}; x[10]")(values)));
	}
	SECTION("variable of type double but should be vector") {
		JExp::val_map_type values = {
			{"x", 3.9}
		};
		CHECK_THROWS(std::get<double>(JExp("3.4+x[2]")(values)));
	}
}

TEST_CASE("Transport", "[Transport]") {
	std::string expr = R"(
var a[13] := {0, 9.129712e-1,-1.001470e-1,-2.454742e-2, 3.145009e-2,-4.456257e-3, -4.511243e-3, 2.237544e-3,-1.455422e-4,-2.006385e-4, 8.341288e-5, -1.520236e-5, 1.159085e-6};
var p[13] := {0,1,2,3,4,5,6,7,8,9,10,11,12};
var eta_ref := 25.3062/1e6; // Pa*s
var y := sum(a*T^p);
return [y];
)"; 
	
	std::string expr2 = R"(
var a[13] := {0, 9.129712e-1,-1.001470e-1,-2.454742e-2, 3.145009e-2,-4.456257e-3, -4.511243e-3, 2.237544e-3,-1.455422e-4,-2.006385e-4, 8.341288e-5, -1.520236e-5, 1.159085e-6};
var p[13] := {0,1,2,3,4,5,6,7,8,9,10,11,12};
var eta_ref := 25.3062/1e6; // Pa*s
sum(a*T^p);
)"; 
	
	SECTION("variables"){	
		CHECK_NOTHROW(JExpr::collect_variables(expr2));
	}
	SECTION("variables") {
		auto missing = JExpr::get_missing_entities<double>(expr);
		CHECK_NOTHROW(JExpr::get_missing_entities<double>(expr));
	}
	//SECTION("dilute") {
	//	std::string expr = R"(
	//		// From Polychroniadou
	//		var a[13] := {0, 9.129712e-1,-1.001470e-1,-2.454742e-2, 3.145009e-2,-4.456257e-3, -4.511243e-3, 2.237544e-3,-1.455422e-4,-2.006385e-4, 8.341288e-5, -1.520236e-5, 1.159085e-6};
	//		var p[13] := {0,1,2,3,4,5,6,7,8,9,10,11,12};
	//		var eta_ref := 25.3062/1e6; // Pa*s
	//		//sum(a*T^p);
	//		var x := log(T/298.15);
	//		//var eta0 := eta_ref*exp(sum(a*x^p));
	//		//eta0;
	//	)";
	//	JExp::val_map_type values = { {"t", 298.15} };
	//	CHECK(std::get<double>(JExp(expr)(values)) == Approx(25.3062e-6));
	//}
}