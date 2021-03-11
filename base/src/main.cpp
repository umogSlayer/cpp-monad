#include <iostream>

#include "functional_applicative.hpp"
#include "functional_functor.hpp"
#include "functional_partially_applicable.hpp"
#include "functional_monad.hpp"
#include "functional_optional.hpp"
#include "functional_alternative.hpp"
#include "json.hpp"

#include <functional>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <ranges>
#include <concepts>

inline namespace
{

namespace test_functional
{

template<typename T>
struct MyMonad final {
    T value;
};

template<typename Func, typename T>
#define CALL_TEXT std::forward<Func>(func)(std::move(value.value))
constexpr auto fmap(Func &&func, MyMonad<T> &&value) noexcept(noexcept(CALL_TEXT)) {
    using functional::fpure;
    return fpure<MyMonad>(CALL_TEXT);
#undef CALL_TEXT
}

template<typename Func, typename T>
#define CALL_TEXT std::forward<Func>(func)(value.value)
constexpr auto fmap(Func &&func, const MyMonad<T> &value) noexcept(noexcept(CALL_TEXT)) {
    using functional::fpure;
    return fpure<MyMonad>(CALL_TEXT);
#undef CALL_TEXT
}

template<typename Func, typename T>
    requires functional::is_instance_v<MyMonad, T>
#define CALL_TEXT fmap(std::move(func.value), std::forward<T>(value))
constexpr auto fapply(MyMonad<Func> &&func, T &&value) noexcept(noexcept(CALL_TEXT)) {
    return CALL_TEXT;
#undef CALL_TEXT
}

template<typename Func, typename T>
    requires functional::is_instance_v<MyMonad, T>
#define CALL_TEXT fmap(func.value, std::forward<T>(value))
constexpr auto fapply(const MyMonad<Func> &func, T &&value) noexcept(noexcept(CALL_TEXT)) {
    return CALL_TEXT;
#undef CALL_TEXT
}

template<typename T>
    requires functional::is_instance_v<MyMonad, T>
auto fjoin(MyMonad<T> &&value) {
    return std::move(value.value);
}

template<typename T>
    requires functional::is_instance_v<MyMonad, T>
auto fjoin(const MyMonad<T> &value) {
    return value.value;
}

template<typename T>
    requires requires(T t, std::ostream &stream) {
        { stream << t };
    }
std::ostream &operator<<(std::ostream &stream, const MyMonad<T> &value) {
    return stream << "MyMonad{" << value.value << "}";
}

template<typename T>
struct MyMonadMethods final {
    T value;

    template<typename Func>
    constexpr auto map(Func &&func) const &
    {
        return MyMonadMethods<decltype(std::forward<Func>(func)(value))>{std::forward<Func>(func)(value)};
    }

    template<typename Func>
    constexpr auto map(Func &&func) &&
    {
        return MyMonadMethods<decltype(std::forward<Func>(func)(std::move(value)))>{std::forward<Func>(func)(std::move(value))};
    }

    template<typename Func>
    constexpr auto apply(const MyMonadMethods<Func> &func) const &
    {
        return MyMonadMethods<decltype(func.value(value))>{func.value(value)};
    }

    template<typename Func>
    constexpr auto apply(const MyMonadMethods<Func> &func) &&
    {
        return MyMonadMethods<decltype(func.value(std::move(value)))>{func.value(std::move(value))};
    }

    constexpr auto join() const &
    {
        return value;
    }

    constexpr auto join() &&
    {
        return std::move(value);
    }
};

template<typename T>
    requires requires(T t, std::ostream &stream) {
        { stream << t };
    }
std::ostream &operator<<(std::ostream &stream, const MyMonadMethods<T> &value) {
    return stream << "MyMonadMethods{" << value.value << "}";
}

} // namespace test_functional

template<typename T>
concept Printable = requires(T val, std::ostream &stream) {
    { stream << val };
};

template<template<typename> typename Functor>
    requires Printable<Functor<int>> && functional::Functor<Functor>
static void functor_test() {
    std::cout << __PRETTY_FUNCTION__ << '\n';
    using functional::fmap;
    using functional::fpure;
    std::cout << "\tpure: " << fpure<Functor>(15) << '\n';
    std::cout << "\tmap: " << fmap([] (int val) { return val * 0.5; }, fpure<Functor>(15)) << '\n';
    std::cout << '\n';
}

template<template<typename> typename Applicative>
    requires Printable<Applicative<int>> && functional::Applicative<Applicative>
static void applicative_test() {
    std::cout << __PRETTY_FUNCTION__ << '\n';
    using functional::fapply;
    using functional::fpure;
    using functional::operator*;
    std::cout << "\tapply: " << fapply(fpure<Applicative>([] (int val) { return val * 0.5; }), fpure<Applicative>(15)) << '\n';
    std::cout << "\tapply (operator): " << (fpure<Applicative>([] (int val) { return val * 0.5; }) * fpure<Applicative>(15)) << '\n';
    std::cout << '\n';
}

template<template<typename> typename Monad>
    requires Printable<Monad<int>> && functional::Monad<Monad>
static void monad_test()
{
    std::cout << __PRETTY_FUNCTION__ << '\n';
    using functional::fjoin;
    using functional::fbind;
    using functional::fpure;
    using functional::operator>>;
    std::cout << "\tjoin: " << fpure<Monad>(fpure<Monad>(15)) << " -> " << fjoin(fpure<Monad>(fpure<Monad>(15))) << '\n';
    std::cout << "\tbind: " << fbind([] (int val) { return Monad{val * 0.5}; }, fpure<Monad>(15)) << '\n';
    std::cout << "\tbind(operator): " << (fpure<Monad>(15) >> [] (int val) { return Monad{val * 0.5}; }) << '\n';
    const auto chain_result = fpure<Monad>(15)
            >> [] (auto val) { return fpure<Monad>(val * 0.5); }
            >> [] (auto val) { return fpure<Monad>(static_cast<int>(val)); };
    std::cout << "\tchain: " << chain_result << '\n';
    std::cout << '\n';
}

template<template<typename> typename Alternative>
    requires Printable<Alternative<int>> && functional::Alternative<Alternative>
static void alternative_test()
{
    std::cout << __PRETTY_FUNCTION__ << '\n';
    using functional::fpure;
    using functional::fempty;
    using functional::falternate;
    using functional::operator|;
    std::cout << "Empty: " << fempty<Alternative, int>() << '\n';
    std::cout << "Alternate (left empty): " << falternate(fempty<Alternative, int>(), fpure<Alternative>(15)) << '\n';
    std::cout << "Alternate (right empty): " << falternate(fpure<Alternative>(15), fempty<Alternative, int>()) << '\n';
    std::cout << "Alternate (operator, left empty): " << (fempty<Alternative, int>() | fpure<Alternative>(15)) << '\n';
    std::cout << '\n';
}

struct OptionalResult final
{
    int x;
    double y;

    friend std::ostream &operator<<(std::ostream &stream, const OptionalResult &value)
    {
        return stream << "OptionalResult{" << value.x << ", " << value.y << "}";
    }
};

template<typename T>
static std::ostream &operator<<(std::ostream &stream, const std::optional<T> &value)
{
    if (value) {
        stream << "std::optional{" << *value << "}";
    } else {
        stream << "std::nullopt";
    }
    return stream;
}

static void optional_test()
{
    std::cout << __PRETTY_FUNCTION__ << '\n';
    using functional::fpure;
    using functional::fempty;
    using functional::fmap;
    using functional::operator*;
    using functional::operator|;
    using functional::partially_applicable;
    {
        constexpr auto result =
            fmap(partially_applicable([] (int a, double b) {
                                          return OptionalResult{.x = a, .y = b};
                                      }),
                 fpure<std::optional>(12))
            * fpure<std::optional>(5.0);
        std::cout << result << '\n';
    }
    {
        constexpr auto result =
            fmap(partially_applicable([] (int a, double b) {
                                          return OptionalResult{.x = a, .y = b};
                                      }),
                 fempty<std::optional, int>())
            * fpure<std::optional>(5.0);
        std::cout << result << '\n';
    }
    std::cout << '\n';
}

static void partially_applicable_test()
{
    std::cout << __PRETTY_FUNCTION__ << '\n';
    constexpr auto partial_test = [] <typename A, typename B, typename C> (A a, B b, C c) {
        return a * b + c;
    };

    constexpr auto partial_wrapped = functional::partially_applicable(partial_test)(15);
    std::cout << "partial_wrapped -> " << partial_wrapped(5., 1.f) << '\n';
    std::cout << "partial_wrapped -> " << partial_wrapped(5.)(1.f) << '\n';
    std::cout << '\n';
}

namespace json_test
{

struct MyStruct final
{
    int a;
    float b;
};

std::ostream &operator<<(std::ostream &stream, const MyStruct &val)
{
    std::cout << "MyStruct{" << val.a << ", " << val.b << "}";
    return stream;
}

json::Parser<MyStruct> parse_json(const json::JsonValue &json_value)
{
    using json::with_object;
    using json::parse_field;
    using json::JsonNumber;
    using namespace std::literals;
    constexpr auto parser = json::with_object(
            "MyStruct"sv,
            [] (const json::JsonObject &json_object) {
                using namespace functional;
                return fmap(partially_applicable([] (JsonNumber a, JsonNumber b) {
                                                     return MyStruct{static_cast<int>(a), static_cast<float>(b)};
                                                 }),
                            parse_field<JsonNumber>(json_object, "a"sv))
                    * parse_field<JsonNumber>(json_object, "b"sv);
            });
    return parser(json_value);
}

void test_json()
{
    const json::JsonObject value = {
        {"a", {12.0}},
        {"b", {12.0}},
    };
    std::cout << parse_json(json::JsonValue{value}) << '\n';
}

} // namespace json_test

} // anonymous namespace

int main()
{
    using namespace test_functional;
    functor_test<MyMonad>();
    applicative_test<MyMonad>();
    monad_test<MyMonad>();

    functor_test<MyMonadMethods>();
    applicative_test<MyMonadMethods>();
    monad_test<MyMonadMethods>();

    functor_test<std::optional>();
    applicative_test<std::optional>();
    monad_test<std::optional>();
    alternative_test<std::optional>();

    functor_test<json::Parser>();
    applicative_test<json::Parser>();
    alternative_test<json::Parser>();

    optional_test();
    json_test::test_json();

    return EXIT_SUCCESS;
}
