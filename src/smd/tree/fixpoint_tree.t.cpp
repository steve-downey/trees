// src/smd/tree/fixpoint_tree.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree.hpp> // Re-inclusion check

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iomanip>
#include <sstream>
#include <string>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::eval_algebra;
using smd::tree::Expr;
using smd::tree::ExprAdd;
using smd::tree::ExprConst;
using smd::tree::ExprF;
using smd::tree::ExprMul;
using smd::tree::fmap_expr;
using smd::tree::fmap_expr_fn;
using smd::tree::mul_expr;

TEST_CASE("FixpointTree - ConstExprConstruction") {
    auto c = const_expr(42.0);
    const auto &layer = smd::fixpoint::unwrap(c);
    CHECK(std::holds_alternative<ExprConst<Expr>>(layer));
}

TEST_CASE("FixpointTree - AddExprConstruction") {
    auto e = add_expr(const_expr(1.0), const_expr(2.0));
    const auto &layer = smd::fixpoint::unwrap(e);
    CHECK(std::holds_alternative<ExprAdd<Expr>>(layer));
}

TEST_CASE("FixpointTree - MulExprConstruction") {
    auto e = mul_expr(const_expr(3.0), const_expr(4.0));
    const auto &layer = smd::fixpoint::unwrap(e);
    CHECK(std::holds_alternative<ExprMul<Expr>>(layer));
}

TEST_CASE("FixpointTree - EvalConst") { CHECK(eval(const_expr(42.0)) == 42.0); }

TEST_CASE("FixpointTree - EvalAdd") {
    CHECK(eval(add_expr(const_expr(1.0), const_expr(2.0))) == 3.0);
}

TEST_CASE("FixpointTree - EvalMul") {
    CHECK(eval(mul_expr(const_expr(3.0), const_expr(4.0))) == 12.0);
}

TEST_CASE("FixpointTree - EvalComplex") {
    // (1 + 2) * 4 == 12
    auto e =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(4.0));
    CHECK(eval(e) == 12.0);
}

TEST_CASE("FixpointTree - EvalNested") {
    // (1 + 2) * (3 + 4) == 21
    auto e = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                      add_expr(const_expr(3.0), const_expr(4.0)));
    CHECK(eval(e) == 21.0);
}

TEST_CASE("FixpointTree - EvalDeeplyNested") {
    // ((1 + 2) + (3 + 4)) * 2 == 20
    auto left = add_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    auto e = mul_expr(left, const_expr(2.0));
    CHECK(eval(e) == 20.0);
}

TEST_CASE("FixpointTree - CustomPrettyPrintAlgebra") {
    using smd::fixpoint::cata;
    using smd::fixpoint::overloaded;

    auto format_constant = [](double value) -> std::string {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(1) << value;
        return stream.str();
    };

    auto print_algebra =
        [format_constant](const ExprF<std::string> &expr) -> std::string {
        return std::visit(
            overloaded{
                [format_constant](const ExprConst<std::string> &c) {
                    return format_constant(c.value);
                },
                [](const ExprAdd<std::string> &a) {
                    return "(" + *a.left + " + " + *a.right + ")";
                },
                [](const ExprMul<std::string> &m) {
                    return "(" + *m.left + " * " + *m.right + ")";
                },
            },
            expr);
    };

    auto e =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(4.0));
    auto result = cata<std::string>(print_algebra, fmap_expr_fn, e);
    CHECK(result == "((1.0 + 2.0) * 4.0)");
}

TEST_CASE("FixpointTree - FmapExprDirect") {
    using smd::fixpoint::make_box;

    ExprF<int> add_layer = ExprAdd<int>{make_box<int>(10), make_box<int>(20)};
    auto doubled = fmap_expr([](int x) { return x * 2; }, add_layer);

    REQUIRE(std::holds_alternative<ExprAdd<int>>(doubled));
    const auto &a = std::get<ExprAdd<int>>(doubled);
    CHECK(*a.left == 20);
    CHECK(*a.right == 40);
}

TEST_CASE("FixpointTree - FmapExprConst") {
    ExprF<int> const_layer = ExprConst<int>{3.14};
    auto mapped =
        fmap_expr([](int x) { return std::to_string(x); }, const_layer);

    REQUIRE(std::holds_alternative<ExprConst<std::string>>(mapped));
    CHECK(std::get<ExprConst<std::string>>(mapped).value == 3.14);
}

TEST_CASE("FixpointTree - EvalAlgebraDirect") {
    using smd::fixpoint::make_box;

    ExprF<double> layer =
        ExprAdd<double>{make_box<double>(10.0), make_box<double>(20.0)};
    CHECK(eval_algebra(layer) == 30.0);
}
