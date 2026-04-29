// tests/beman/optional/optional_ref.test.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>
#include <beman/optional/test_types.hpp>

#include <gtest/gtest.h>

TEST(OptionalRefTest, Constructors) {
    beman::optional::optional<int&> i1;
    beman::optional::optional<int&> i2{beman::optional::nullopt};
    std::ignore = i1;
    std::ignore = i2;

    int                             i  = 0;
    beman::optional::optional<int&> i3 = i;
    std::ignore                        = i3;

    using beman::optional::tests::empty;

    beman::optional::optional<empty&> e1;
    beman::optional::optional<empty&> e2{beman::optional::nullopt};
    std::ignore = e1;
    std::ignore = e2;

    empty                             e{};
    beman::optional::optional<empty&> e3 = e;
    std::ignore                          = e3;

    using beman::optional::tests::no_default_ctor;

    beman::optional::optional<no_default_ctor&> nd1;
    beman::optional::optional<no_default_ctor&> nd2{beman::optional::nullopt};
    std::ignore = nd1;
    std::ignore = nd2;

    no_default_ctor no_def{e};

    beman::optional::optional<no_default_ctor&> nd3 = no_def;
    std::ignore                                     = nd3;

    beman::optional::optional<int&> ie;
    beman::optional::optional<int&> i4 = ie;
    EXPECT_FALSE(i4);

    using beman::optional::tests::base;
    using beman::optional::tests::derived;

    base                             b{1};
    derived                          d(1, 2);
    beman::optional::optional<base&> b1 = b;
    beman::optional::optional<base&> b2 = d;

    beman::optional::optional<derived&> d2 = d;
    beman::optional::optional<base&>    b3 = d2;
    beman::optional::optional<base&>    b4{d2};

    beman::optional::optional<derived&> emptyDerived;
    beman::optional::optional<base&>    fromEmpty(emptyDerived);
    beman::optional::optional<base&>    fromEmpty2 = emptyDerived;

    /*
     * template <class U>
     *   requires(!is_derived_from_optional<decay_t<U>>)
     * constexpr explicit(!is_convertible_v<U, T>) optional(U&& u) noexcept;
     *
     * Not selected -- use default constructor of optional<T&> so it is
     * *DIS*engaged
     */

    beman::optional::optional<int&> tempint = {};
    EXPECT_FALSE(tempint);
}

namespace {
// https://gcc.godbolt.org/z/aGGW3TYov
struct derived;
extern derived d;
struct base {
    virtual ~base() = default;
    operator derived&() { return d; }
};

struct derived : base {};

derived d;
} // namespace

struct Thing {};
beman::optional::optional<Thing&> process() {
    static Thing t;
    return t;
}

beman::optional::optional<Thing&> processEmpty() { return beman::optional::nullopt; }

TEST(OptionalRefTest, BaseDerivedCastConstruction) {
    base                                b;
    derived&                            dref(b); // ok
    beman::optional::optional<derived&> dopt1(b);
    beman::optional::optional<derived&> dopt2(dref);
    EXPECT_TRUE(dopt1.has_value());
    EXPECT_TRUE(dopt2.has_value());
}

TEST(OptionalRefTest, BaseDerivedCastEmplace) {
    base                                b;
    derived&                            dref(b); // ok
    beman::optional::optional<derived&> dopt1;
    dopt1.emplace(b);
    beman::optional::optional<derived&> dopt2;
    dopt2.emplace(dref);
    EXPECT_TRUE(dopt1.has_value());
    EXPECT_TRUE(dopt2.has_value());
}

TEST(OptionalRefTest, Assignment) {
    beman::optional::optional<int&> i1;
    EXPECT_FALSE(i1);
    int i = 5;
    i1    = i;
    i     = 7;
    EXPECT_TRUE(i1);
    EXPECT_TRUE(*i1 = 7);
    EXPECT_EQ(i1, 7);

    double d;
    // i1 = d;  // ill-formed by mandate
    beman::optional::optional<double&> d1 = d;
    // i1 = d1; // ill-formed by mandate
    beman::optional::optional<int&> i2 = i1;
    EXPECT_TRUE(i2);
    EXPECT_TRUE(*i2 = 7);

    beman::optional::optional<int&> emptyInt;
    EXPECT_FALSE(emptyInt);
    i2 = emptyInt;
    EXPECT_FALSE(i2);
    int  eight  = 8;
    int& result = emptyInt.emplace(eight);
    EXPECT_TRUE(emptyInt);
    EXPECT_EQ(emptyInt, 8);
    EXPECT_EQ(&result, &eight);

    beman::optional::optional<const Thing&> o;
    EXPECT_FALSE(o);
    o = process(); // well-formed
    EXPECT_TRUE(o);

    o = processEmpty(); // well-formed
    EXPECT_FALSE(o);

    beman::optional::optional<const int&> o2;
    EXPECT_FALSE(o2);
    o2 = [&]() { return i1; }();

    EXPECT_EQ(*o2, 7);
}

TEST(OptionalRefTest, NullOptAssignment) {
    beman::optional::optional<int&> i1;
    EXPECT_FALSE(i1);
    int i = 5;
    i1    = i;

    EXPECT_TRUE(i1);
    i1 = beman::optional::nullopt;
    EXPECT_FALSE(i1);
    i1 = i;
    EXPECT_TRUE(i1);
}

TEST(OptionalRefTest, ConstRefAssignment) {
    int                                   i = 7;
    beman::optional::optional<int&>       i1{i};
    const beman::optional::optional<int&> i2 = i1;

    beman::optional::optional<const int&> c1;
    c1 = i2;
    EXPECT_TRUE(c1);
    EXPECT_EQ(*c1, 7);

    i = 5;
    EXPECT_EQ(*c1, 5);
    const beman::optional::optional<int&> emptyInt(beman::optional::nullopt);
    c1 = emptyInt;
    EXPECT_FALSE(c1);
}

TEST(OptionalRefTest, ConvertingConstRvalRef) {
    int                                   i = 7;
    beman::optional::optional<int&>       i1{i};
    const beman::optional::optional<int&> i2 = i1;

    beman::optional::optional<const int&> c1;
    c1 = std::move(i2);
    EXPECT_TRUE(c1);
    EXPECT_EQ(*c1, 7);

    i = 5;
    EXPECT_EQ(*c1, 5);
    const beman::optional::optional<int&> emptyInt(beman::optional::nullopt);
    c1 = std::move(emptyInt);
    EXPECT_FALSE(c1);
}

TEST(OptionalRefTest, NullOptConstruction) {
    beman::optional::optional<int&> i1(beman::optional::nullopt);
    EXPECT_FALSE(i1);
    int i = 5;
    i1    = i;

    EXPECT_TRUE(i1);
    i1 = beman::optional::nullopt;
    EXPECT_FALSE(i1);
    i1 = i;
    EXPECT_TRUE(i1);
}

TEST(OptionalRefTest, RelationalOps) {
    int                             i1 = 4;
    int                             i2 = 42;
    beman::optional::optional<int&> o1{i1};
    beman::optional::optional<int&> o2{i2};
    beman::optional::optional<int&> o3{};

    //  SECTION("self simple")
    {
        EXPECT_TRUE(!(o1 == o2));
        EXPECT_TRUE(o1 == o1);
        EXPECT_TRUE(o1 != o2);
        EXPECT_TRUE(!(o1 != o1));
        EXPECT_TRUE(o1 < o2);
        EXPECT_TRUE(!(o1 < o1));
        EXPECT_TRUE(!(o1 > o2));
        EXPECT_TRUE(!(o1 > o1));
        EXPECT_TRUE(o1 <= o2);
        EXPECT_TRUE(o1 <= o1);
        EXPECT_TRUE(!(o1 >= o2));
        EXPECT_TRUE(o1 >= o1);
    }
    //  SECTION("nullopt simple")
    {
        EXPECT_TRUE(!(o1 == beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt == o1));
        EXPECT_TRUE(o1 != beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt != o1);
        EXPECT_TRUE(!(o1 < beman::optional::nullopt));
        EXPECT_TRUE(beman::optional::nullopt < o1);
        EXPECT_TRUE(o1 > beman::optional::nullopt);
        EXPECT_TRUE(!(beman::optional::nullopt > o1));
        EXPECT_TRUE(!(o1 <= beman::optional::nullopt));
        EXPECT_TRUE(beman::optional::nullopt <= o1);
        EXPECT_TRUE(o1 >= beman::optional::nullopt);
        EXPECT_TRUE(!(beman::optional::nullopt >= o1));

        EXPECT_TRUE(o3 == beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt == o3);
        EXPECT_TRUE(!(o3 != beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt != o3));
        EXPECT_TRUE(!(o3 < beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt < o3));
        EXPECT_TRUE(!(o3 > beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt > o3));
        EXPECT_TRUE(o3 <= beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt <= o3);
        EXPECT_TRUE(o3 >= beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt >= o3);
    }
    //  SECTION("with T simple")
    {
        EXPECT_TRUE(!(o1 == 1));
        EXPECT_TRUE(!(1 == o1));
        EXPECT_TRUE(o1 != 1);
        EXPECT_TRUE(1 != o1);
        EXPECT_TRUE(!(o1 < 1));
        EXPECT_TRUE(1 < o1);
        EXPECT_TRUE(o1 > 1);
        EXPECT_TRUE(!(1 > o1));
        EXPECT_TRUE(!(o1 <= 1));
        EXPECT_TRUE(1 <= o1);
        EXPECT_TRUE(o1 >= 1);
        EXPECT_TRUE(!(1 >= o1));

        EXPECT_TRUE(o1 == 4);
        EXPECT_TRUE(4 == o1);
        EXPECT_TRUE(!(o1 != 4));
        EXPECT_TRUE(!(4 != o1));
        EXPECT_TRUE(!(o1 < 4));
        EXPECT_TRUE(!(4 < o1));
        EXPECT_TRUE(!(o1 > 4));
        EXPECT_TRUE(!(4 > o1));
        EXPECT_TRUE(o1 <= 4);
        EXPECT_TRUE(4 <= o1);
        EXPECT_TRUE(o1 >= 4);
        EXPECT_TRUE(4 >= o1);
    }
    std::string                             s4 = "hello";
    std::string                             s5 = "xyz";
    beman::optional::optional<std::string&> o4{s4};
    beman::optional::optional<std::string&> o5{s5};

    //  SECTION("self complex")
    {
        EXPECT_TRUE(!(o4 == o5));
        EXPECT_TRUE(o4 == o4);
        EXPECT_TRUE(o4 != o5);
        EXPECT_TRUE(!(o4 != o4));
        EXPECT_TRUE(o4 < o5);
        EXPECT_TRUE(!(o4 < o4));
        EXPECT_TRUE(!(o4 > o5));
        EXPECT_TRUE(!(o4 > o4));
        EXPECT_TRUE(o4 <= o5);
        EXPECT_TRUE(o4 <= o4);
        EXPECT_TRUE(!(o4 >= o5));
        EXPECT_TRUE(o4 >= o4);
    }
    //  SECTION("nullopt complex")
    {
        EXPECT_TRUE(!(o4 == beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt == o4));
        EXPECT_TRUE(o4 != beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt != o4);
        EXPECT_TRUE(!(o4 < beman::optional::nullopt));
        EXPECT_TRUE(beman::optional::nullopt < o4);
        EXPECT_TRUE(o4 > beman::optional::nullopt);
        EXPECT_TRUE(!(beman::optional::nullopt > o4));
        EXPECT_TRUE(!(o4 <= beman::optional::nullopt));
        EXPECT_TRUE(beman::optional::nullopt <= o4);
        EXPECT_TRUE(o4 >= beman::optional::nullopt);
        EXPECT_TRUE(!(beman::optional::nullopt >= o4));

        EXPECT_TRUE(o3 == beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt == o3);
        EXPECT_TRUE(!(o3 != beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt != o3));
        EXPECT_TRUE(!(o3 < beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt < o3));
        EXPECT_TRUE(!(o3 > beman::optional::nullopt));
        EXPECT_TRUE(!(beman::optional::nullopt > o3));
        EXPECT_TRUE(o3 <= beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt <= o3);
        EXPECT_TRUE(o3 >= beman::optional::nullopt);
        EXPECT_TRUE(beman::optional::nullopt >= o3);
    }

    //  SECTION("with T complex")
    {
        EXPECT_TRUE(!(o4 == "a"));
        EXPECT_TRUE(!("a" == o4));
        EXPECT_TRUE(o4 != "a");
        EXPECT_TRUE("a" != o4);
        EXPECT_TRUE(!(o4 < "a"));
        EXPECT_TRUE("a" < o4);
        EXPECT_TRUE(o4 > "a");
        EXPECT_TRUE(!("a" > o4));
        EXPECT_TRUE(!(o4 <= "a"));
        EXPECT_TRUE("a" <= o4);
        EXPECT_TRUE(o4 >= "a");
        EXPECT_TRUE(!("a" >= o4));

        EXPECT_TRUE(o4 == "hello");
        EXPECT_TRUE("hello" == o4);
        EXPECT_TRUE(!(o4 != "hello"));
        EXPECT_TRUE(!("hello" != o4));
        EXPECT_TRUE(!(o4 < "hello"));
        EXPECT_TRUE(!("hello" < o4));
        EXPECT_TRUE(!(o4 > "hello"));
        EXPECT_TRUE(!("hello" > o4));
        EXPECT_TRUE(o4 <= "hello");
        EXPECT_TRUE("hello" <= o4);
        EXPECT_TRUE(o4 >= "hello");
        EXPECT_TRUE("hello" >= o4);
    }
}

TEST(OptionalRefTest, Triviality) {
    EXPECT_TRUE(std::is_trivially_copy_constructible<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_trivially_copy_assignable<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_trivially_move_constructible<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_trivially_move_assignable<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_trivially_destructible<beman::optional::optional<int&>>::value);

    {
        struct T {
            T(const T&)            = default;
            T(T&&)                 = default;
            T& operator=(const T&) = default;
            T& operator=(T&&)      = default;
            ~T()                   = default;
        };
        EXPECT_TRUE(std::is_trivially_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_copy_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_move_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_move_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_destructible<beman::optional::optional<T&>>::value);
    }

    {
        struct T {
            T(const T&) {}
            T(T&&) {};
            T& operator=(const T&) { return *this; }
            T& operator=(T&&) { return *this; };
            ~T() {}
        };
        EXPECT_TRUE(std::is_trivially_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_copy_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_move_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_move_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_trivially_destructible<beman::optional::optional<T&>>::value);

        EXPECT_TRUE(
            (std::is_trivially_constructible<beman::optional::optional<T&>, beman::optional::optional<T&>&>::value));
    }
}

TEST(OptionalRefTest, Deletion) {
    EXPECT_TRUE(std::is_copy_constructible<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_copy_assignable<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_move_constructible<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_move_assignable<beman::optional::optional<int&>>::value);
    EXPECT_TRUE(std::is_destructible<beman::optional::optional<int&>>::value);

    {
        struct T {
            T(const T&)            = default;
            T(T&&)                 = default;
            T& operator=(const T&) = default;
            T& operator=(T&&)      = default;
            ~T()                   = default;
        };
        EXPECT_TRUE(std::is_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_copy_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_destructible<beman::optional::optional<T&>>::value);
    }

    {
        struct T {
            T(const T&)            = delete;
            T(T&&)                 = delete;
            T& operator=(const T&) = delete;
            T& operator=(T&&)      = delete;
        };
        EXPECT_TRUE(std::is_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_copy_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_assignable<beman::optional::optional<T&>>::value);
    }

    {
        struct T {
            T(const T&)            = delete;
            T(T&&)                 = default;
            T& operator=(const T&) = delete;
            T& operator=(T&&)      = default;
        };
        EXPECT_TRUE(std::is_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_copy_assignable<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_move_assignable<beman::optional::optional<T&>>::value);
    }

    {
        struct T {
            T(const T&)            = default;
            T(T&&)                 = delete;
            T& operator=(const T&) = default;
            T& operator=(T&&)      = delete;
        };
        EXPECT_TRUE(std::is_copy_constructible<beman::optional::optional<T&>>::value);
        EXPECT_TRUE(std::is_copy_assignable<beman::optional::optional<T&>>::value);
    }
}

struct takes_init_and_variadic {
    std::vector<int>     v;
    std::tuple<int, int> t;
    template <class... Args>
    takes_init_and_variadic(std::initializer_list<int> l, Args&&... args) : v(l), t(std::forward<Args>(args)...) {}
};

TEST(OptionalRefTest, Nullopt) {
    beman::optional::optional<int&> o1 = beman::optional::nullopt;
    beman::optional::optional<int&> o2{beman::optional::nullopt};
    beman::optional::optional<int&> o3(beman::optional::nullopt);
    beman::optional::optional<int&> o4 = {beman::optional::nullopt};

    EXPECT_TRUE(!o1);
    EXPECT_TRUE(!o2);
    EXPECT_TRUE(!o3);
    EXPECT_TRUE(!o4);

    EXPECT_TRUE(!std::is_default_constructible<beman::optional::nullopt_t>::value);
}

TEST(OptionalRefTest, Observers) {
    int                                   var = 42;
    beman::optional::optional<int&>       o1  = var;
    beman::optional::optional<int&>       o2;
    const beman::optional::optional<int&> o3 = var;
    const beman::optional::optional<int&> o4;
    int                                   var2 = 42;
    int                                   var3 = 6 * 9;
    EXPECT_TRUE(*o1 == 42);
    EXPECT_TRUE(*o1 == o1.value());
    EXPECT_TRUE(o2.value_or(var2) == 42);
    EXPECT_TRUE(o3.value() == 42);
    EXPECT_TRUE(o3.value_or(var3) == 42);
    EXPECT_TRUE(o4.value_or(var3) == 54);
    int j = 99;
    EXPECT_TRUE(o4.value_or(j) == 99);
    // o4.value_or(j) = 88;
    // EXPECT_TRUE(j == 88);
    int var99 = 99;
    j         = 88;
    EXPECT_TRUE([&]() {
        beman::optional::optional<int&> o(j);
        return o;
    }()
                    .value_or(var99) == 88);

    EXPECT_TRUE([&]() {
        beman::optional::optional<int&> o;
        return o;
    }()
                    .value_or(var99) == 99);

    EXPECT_TRUE(o3.value_or([&]() -> int& { return var3; }()) == 42);
    EXPECT_TRUE(o4.value_or([&]() -> int& { return var3; }()) == 54);

    std::string                             meow{"meow"};
    std::string                             bark{"bark"};
    beman::optional::optional<std::string&> so1;
    beman::optional::optional<std::string&> so2{meow};
    auto                                    t1 = so1.value_or(bark);
    auto                                    t2 = so2.value_or(bark);
    // auto t3 = so1.value_or("bark");
    // auto t4 = so2.value_or("bark");
    // std::tuple<const std::string&> t("meow");

    auto t5 = so1.value_or({});
    auto t6 = so2.value_or({});
    EXPECT_EQ(t5, std::string());
    EXPECT_EQ(t6, meow);

    auto success = std::is_same<decltype(o1.value()), int&>::value;
    static_assert(std::is_same<decltype(o1.value()), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(o2.value()), int&>::value;
    static_assert(std::is_same<decltype(o2.value()), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(o3.value()), int&>::value;
    static_assert(std::is_same<decltype(o3.value()), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(std::move(o1).value()), int&>::value;
    static_assert(std::is_same<decltype(std::move(o1).value()), int&>::value);
    EXPECT_TRUE(success);

    success = std::is_same<decltype(*o1), int&>::value;
    static_assert(std::is_same<decltype(*o1), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(*o2), int&>::value;
    static_assert(std::is_same<decltype(*o2), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(*o3), int&>::value;
    static_assert(std::is_same<decltype(*o3), int&>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(*std::move(o1)), int&>::value;
    static_assert(std::is_same<decltype(*std::move(o1)), int&>::value);
    EXPECT_TRUE(success);

    success = std::is_same<decltype(o1.operator->()), int*>::value;
    static_assert(std::is_same<decltype(o1.operator->()), int*>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(o2.operator->()), int*>::value;
    static_assert(std::is_same<decltype(o2.operator->()), int*>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(o3.operator->()), int*>::value;
    static_assert(std::is_same<decltype(o3.operator->()), int*>::value);
    EXPECT_TRUE(success);
    success = std::is_same<decltype(std::move(o1).operator->()), int*>::value;
    static_assert(std::is_same<decltype(std::move(o1).operator->()), int*>::value);
    EXPECT_TRUE(success);

    struct int_box {
        int i_;
    };
    int_box                                   i1{3};
    beman::optional::optional<int_box&>       ob1 = i1;
    beman::optional::optional<int_box&>       ob2;
    const beman::optional::optional<int_box&> ob3 = i1;

    EXPECT_EQ(ob1->i_, 3);
    EXPECT_EQ(ob3->i_, 3);

    success = std::is_same_v<decltype(ob1->i_), int>;
    static_assert(std::is_same_v<decltype(ob1->i_), int>);
    EXPECT_TRUE(success);
    success = std::is_same_v<decltype(ob2->i_), int>;
    static_assert(std::is_same_v<decltype(ob2->i_), int>);
    EXPECT_TRUE(success);
    success = std::is_same_v<decltype(ob3->i_), int>;
    static_assert(std::is_same_v<decltype(ob3->i_), int>);
    EXPECT_TRUE(success);
    success = std::is_same_v<decltype(std::move(ob1)->i_), int>;
    static_assert(std::is_same_v<decltype(std::move(ob1)->i_), int>);
    EXPECT_TRUE(success);
}

TEST(OptionalRefTest, AmbiguousConversion) {
    struct TypedInt {
        int c;
        TypedInt(int i) : c(i) {}
        operator int() const { return c; }
    };

    TypedInt c{42};

    auto x1 = beman::optional::optional<int>{}.value_or(c);
    auto x2 = beman::optional::optional<TypedInt>{}.value_or(7);

    auto x3 = beman::optional::optional<int&>{}.value_or(c);
    auto x4 = beman::optional::optional<TypedInt&>{}.value_or(7);
    EXPECT_EQ(x1, 42);
    EXPECT_EQ(x2, 7);
    EXPECT_EQ(x3, 42);
    EXPECT_EQ(x4, 7);
}

TEST(OptionalRefTest, MoveCheck) {
    int  x = 0;
    int& y = std::move(beman::optional::optional<int&>(x)).value();
    EXPECT_EQ(&y, &x);

    int& z = *std::move(beman::optional::optional<int&>(x));
    EXPECT_EQ(&z, &x);
}

TEST(OptionalRefTest, SwapValue) {
    int                             var    = 42;
    int                             twelve = 12;
    beman::optional::optional<int&> o1     = var;
    beman::optional::optional<int&> o2     = twelve;
    o1.swap(o2);
    EXPECT_EQ(o1.value(), 12);
    EXPECT_EQ(o2.value(), 42);
}

TEST(OptionalRefTest, SwapWNull) {
    int var = 42;

    beman::optional::optional<int&> o1 = var;
    beman::optional::optional<int&> o2 = beman::optional::nullopt;
    o1.swap(o2);
    EXPECT_TRUE(!o1.has_value());
    EXPECT_EQ(o2.value(), 42);
}

TEST(OptionalRefTest, SwapNullIntializedWithValue) {
    int                             var = 42;
    beman::optional::optional<int&> o1  = beman::optional::nullopt;
    beman::optional::optional<int&> o2  = var;
    o1.swap(o2);
    EXPECT_EQ(o1.value(), 42);
    EXPECT_TRUE(!o2.has_value());
}

TEST(OptionalRefTest, AssignFromOptional) {
    int                             var = 42;
    beman::optional::optional<int&> o1  = beman::optional::nullopt;
    beman::optional::optional<int&> o2  = var;

    o2 = o1;

    using beman::optional::tests::base;
    using beman::optional::tests::derived;

    base                             b{1};
    derived                          d(1, 2);
    beman::optional::optional<base&> empty_base;
    beman::optional::optional<base&> engaged_base{b};

    beman::optional::optional<derived&> empty_derived_ref;
    beman::optional::optional<derived&> engaged_derived_ref{d};

    beman::optional::optional<base&> optional_base_ref;
    optional_base_ref = empty_base;
    EXPECT_FALSE(optional_base_ref.has_value());
    optional_base_ref = engaged_base;
    EXPECT_TRUE(optional_base_ref.has_value());

    optional_base_ref = empty_derived_ref;
    EXPECT_FALSE(optional_base_ref.has_value());

    optional_base_ref = engaged_derived_ref;
    EXPECT_TRUE(optional_base_ref.has_value());

    beman::optional::optional<derived> empty_derived;
    beman::optional::optional<derived> engaged_derived{d};

    static_assert(std::is_constructible_v<const base&, derived>);

    beman::optional::optional<const base&> optional_base_const_ref;
    optional_base_const_ref = empty_derived;
    EXPECT_FALSE(optional_base_const_ref.has_value());

    optional_base_const_ref = engaged_derived;
    EXPECT_TRUE(optional_base_const_ref.has_value());

    if (empty_derived) {
        optional_base_ref = empty_derived.value();
    } else {
        optional_base_ref.reset();
    }
    EXPECT_FALSE(optional_base_ref.has_value());

    if (engaged_derived) {
        optional_base_ref = engaged_derived.value();
    } else {
        optional_base_ref.reset();
    }
    EXPECT_TRUE(optional_base_ref.has_value());

    derived d2(2, 2);
    engaged_derived = d2;
    EXPECT_EQ(optional_base_ref.value().m_i, static_cast<base>(d2).m_i);

    // deleted the rvalue ref overload
    //     template <class U>
    //        constexpr optional& operator=(optional<U>&& rhs) = delete;
    // -- force failures for
    // optional_base_const_ref = beman::optional::optional<derived>(derived(3, 4));
    // and
    // optional_base_const_ref = [](){return beman::optional::optional<derived>(derived(3, 4));}();
    // TODO: Add to "fail-to-compile" tests when they exist
}

TEST(OptionalRefTest, ConstructFromOptional) {
    int                             var = 42;
    beman::optional::optional<int&> o1  = beman::optional::nullopt;
    beman::optional::optional<int&> o2{var};
    EXPECT_FALSE(o1.has_value());
    EXPECT_TRUE(o2.has_value());

    using beman::optional::tests::base;
    using beman::optional::tests::derived;

    base                             b{1};
    derived                          d(1, 2);
    beman::optional::optional<base&> disengaged_base;
    beman::optional::optional<base&> engaged_base{b};
    EXPECT_FALSE(disengaged_base.has_value());
    EXPECT_TRUE(engaged_base.has_value());

    beman::optional::optional<derived&> disengaged_derived_ref;
    beman::optional::optional<derived&> engaged_derived_ref{d};

    beman::optional::optional<base&> optional_base_ref{disengaged_derived_ref};
    EXPECT_FALSE(optional_base_ref.has_value());

    beman::optional::optional<base&> optional_base_ref2{engaged_derived_ref};
    EXPECT_TRUE(optional_base_ref2.has_value());

    beman::optional::optional<derived> disengaged_derived;
    beman::optional::optional<derived> engaged_derived{d};

    static_assert(std::is_constructible_v<const base&, derived>);

    beman::optional::optional<const base&> optional_base_const_ref{disengaged_derived};
    EXPECT_FALSE(optional_base_const_ref.has_value());

    beman::optional::optional<const base&> optional_base_const_ref2{engaged_derived};
    EXPECT_TRUE(optional_base_const_ref2.has_value());
}

TEST(OptionalRefTest, InPlace) {
    int one      = 1;
    int two      = 2;
    int fortytwo = 42;

    beman::optional::optional<int&> o1{beman::optional::in_place, one};
    beman::optional::optional<int&> o2(beman::optional::in_place, two);
    EXPECT_TRUE(o1);
    EXPECT_TRUE(o1 == 1);
    EXPECT_TRUE(o2);
    EXPECT_TRUE(o2 == 2);

    beman::optional::optional<const int&> o3(beman::optional::in_place, fortytwo);
    EXPECT_TRUE(o3 == 42);

    // beman::optional::optional<std::vector<int>&> o5(beman::optional::in_place, {0, 1});
    // EXPECT_TRUE(o5);
    // EXPECT_TRUE((*o5)[0] == 0);
    // EXPECT_TRUE((*o5)[1] == 1);

    // beman::optional::optional<std::tuple<int, int> const&> o4(beman::optional::in_place, zero, one);
    // EXPECT_TRUE(o4);
    // EXPECT_TRUE(std::get<0>(*o4) == 0);
    // EXPECT_TRUE(std::get<1>(*o4) == 1);
}

TEST(OptionalRefTest, OptionalOfOptional) {
    using O = beman::optional::optional<int>;
    O                             o;
    beman::optional::optional<O&> oo1a(o);
    beman::optional::optional<O&> oo1{o};
    beman::optional::optional<O&> oo1b = o;
    EXPECT_TRUE(oo1.has_value());
    oo1 = o;
    EXPECT_TRUE(oo1.has_value());
    EXPECT_TRUE(&oo1.value() == &o);
    oo1.emplace(o); // emplace, like assignment, binds the reference
    EXPECT_TRUE(oo1.has_value());
    EXPECT_TRUE(&oo1.value() == &o);

    beman::optional::optional<const O&> oo2 = o;
    EXPECT_TRUE(oo2.has_value());
    oo2 = o;
    EXPECT_TRUE(oo2.has_value());
    EXPECT_TRUE(&oo2.value() == &o);
    oo2.emplace(o);
    EXPECT_TRUE(oo2.has_value());
    EXPECT_TRUE(&oo2.value() == &o);
}

TEST(OptionalRefTest, ConstructFromReferenceWrapper) {
    using O = beman::optional::optional<int>;
    O o;

    beman::optional::optional<O&> oo1 = std::ref(o);
    EXPECT_TRUE(oo1.has_value());
    oo1 = std::ref(o);
    EXPECT_TRUE(oo1.has_value());
    EXPECT_TRUE(&oo1.value() == &o);

    auto                          lvalue_refwrapper = std::ref(o);
    beman::optional::optional<O&> oo2               = lvalue_refwrapper;
    EXPECT_TRUE(oo2.has_value());
    oo2 = lvalue_refwrapper;
    EXPECT_TRUE(oo2.has_value());
    EXPECT_TRUE(&oo2.value() == &o);

    beman::optional::optional<const O&> oo3 = std::ref(o);
    EXPECT_TRUE(oo3.has_value());
    oo3 = std::ref(o);
    EXPECT_TRUE(oo3.has_value());
    EXPECT_TRUE(&oo3.value() == &o);

    beman::optional::optional<const O&> oo4 = lvalue_refwrapper;
    EXPECT_TRUE(oo4.has_value());
    oo4 = lvalue_refwrapper;
    EXPECT_TRUE(oo4.has_value());
    EXPECT_TRUE(&oo4.value() == &o);
}

TEST(OptionalRefTest, OverloadResolutionChecksDangling) {
    extern int  check_dangling(beman::optional::optional<const std::string&>);
    extern void check_dangling(beman::optional::optional<const char*>);
    std::string lvalue_string = "abc";
    static_assert(std::is_same_v<decltype(check_dangling(lvalue_string)), int>);
    //    static_assert(std::is_same_v<decltype(check_dangling("abc")), void>);
}

namespace {
int int_func(void) { return 7; }
} // namespace

TEST(OptionalRefTest, NonReturnableRef) {
    using IntArray5 = int[5];
    beman::optional::optional<IntArray5&> o1;
    IntArray5                             array;
    beman::optional::optional<IntArray5&> o2{array};
    EXPECT_FALSE(o1.has_value());
    EXPECT_TRUE(o2.has_value());
    // value_or removed for array types
    // IntArray5 array2;
    // auto t1 = o1.value_or(array2);
    // auto t2 = o2.value_or(array2);

    using IntFunc = int(void);
    beman::optional::optional<IntFunc&> o3;
    beman::optional::optional<IntFunc&> o4{int_func};
    EXPECT_FALSE(o3.has_value());
    EXPECT_TRUE(o4.has_value());
}

// beman::optional::optional<int const&> foo() {
//     beman::optional::optional<int> o(10);
//     return o; // Thanks to a simpler implicit move.
//     /* error: use of deleted function ‘constexpr
//     beman::optional::optional<T&>::optional(beman::optional::optional<U>&&) [with U = int; T = const int]’
//      */
// }

// TEST(OptionalRefTest, iff) {
//     beman::optional::optional<int const&> o =
//         beman::optional::optional<int>(o);
//     // error: use of deleted function ‘constexpr
//     // beman::optional::optional<T&>::optional(beman::optional::optional<U>&&)
//     // [with U = int; T = const int]’
// }

// TEST(OptionalRefTest, dangle) {
//     extern int check_dangling(
//         beman::optional::optional<std::string const&>); // #1
//     extern void check_dangling(
//         beman::optional::optional<char const*&>); // #2
//     beman::optional::optional<std::string> optional_string  = "abc";
//     beman::optional::optional<char const*> optional_pointer = "abc";
//     static_assert(std::is_same_v<decltype(check_dangling(optional_string)),
//                                  int>); // unambiguously calls #1
//     static_assert(std::is_same_v<decltype(check_dangling(
//                                      beman::optional::optional<char
//                                      const*&>(
//                                          optional_pointer))),
//                                  void>); // unambiguously calls #2
//     static_assert(std::is_same_v<decltype(check_dangling(optional_pointer)),
//                   void>); // ambiguous
//     // error: call of overloaded
//     // ‘check_dangling(beman::optional::optional<const char*>&)’ is
//     ambiguous
// }
// namespace {
// void process(beman::optional::optional<std::string const&>) {}
// void process(beman::optional::optional<char const* const&>) {}
// }
// TEST(OptionalRefTest, more_dangle){

//     char const* cstr = "Text";
//     std::string s       = cstr;
//     process(s);    // Picks, `optional<std::string const&>` overload
//     // process(cstr); // Ambiguous, but only std::optional<char const* const&>
// }
