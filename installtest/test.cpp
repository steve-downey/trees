// testinstall/test.cpp                                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <smd/example/name.hpp>

int main() {
    std::cout << "name: |" << example::name() << '|' << '\n';
    return 0;
}
