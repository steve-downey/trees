// examples/base_derived_cast.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>

struct derived;
extern derived d;
struct base {
    virtual ~base() = default;
    operator derived&() { return d; }
};

struct derived : base {};

derived d;

int example() {
    base                                b;
    derived&                            dref(b); // ok
    beman::optional::optional<derived&> dopt(b); // ok
    (void)dref;
    return 0;
}

int main() { example(); }
