// example/name.test.cpp                                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/example/name.hpp>

#include <smd/example/name.hpp> // test 2nd include OK

#include <gtest/gtest.h>

TEST(Test, Fail) { SUCCEED(); }

// 03013d1f-bcc1-4d3e-9701-3ed1a15c6370
TEST(TestName, Steve) { ASSERT_EQ(example::name(), "Steve"); }
// 03013d1f-bcc1-4d3e-9701-3ed1a15c6370 end
