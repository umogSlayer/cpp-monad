#include <iostream>

#include <functional>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <ranges>
#include <concepts>

#include "functional_monad.hpp"
#include "functional_optional.hpp"

namespace functional
{

#if 0
template<typename T>
class PartiallyApplicable : public T
{
public:
    constexpr PartiallyApplicable(T &&t)
        : T(std::move(t))
    {
    }

    constexpr PartiallyApplicable(const T &t)
        : T(t)
    {
    }

    template<typename Arg>
    constexpr auto operator()(Arg &&arg) const
    {
        return [this, forwarded_arg = std::forward<Arg>(arg)] <typename ...Args> (Args &&...args) mutable {
            return T::operator()(std::move(forwarded_arg), std::forward<Args>(args)...);
        };
    }

    template<typename Arg>
    constexpr auto operator()(Arg &&arg)
    {
        return [this, forwarded_arg = std::forward<Arg>(arg)] <typename ...Args> (Args &&...args) mutable {
            return T::operator()(std::move(forwarded_arg), std::forward<Args>(args)...);
        };
    }
};

template<typename T>
PartiallyApplicable(T &&t) -> PartiallyApplicable<std::remove_cvref_t<T>>;
#endif

} // namespace functional

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
    constexpr auto map(Func &&func) const {
        return MyMonadMethods<decltype(std::forward<Func>(func)(value))>{std::forward<Func>(func)(value)};
    }

    template<typename Func>
    constexpr auto apply(const MyMonadMethods<Func> &func) const {
        return MyMonadMethods<decltype(func.value(value))>{func.value(value)};
    }

    constexpr auto join() const {
        return value;
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
    using functional::fmap;
    using functional::fpure;
    std::cout << "Functor (" << typeid(Functor<int>).name() << ")\n";
    std::cout << "\tpure: " << fpure<Functor>(15) << '\n';
    std::cout << "\tmap: " << fmap([] (int val) { return val * 0.5; }, fpure<Functor>(15)) << '\n';
}

template<template<typename> typename Applicative>
    requires Printable<Applicative<int>> && functional::Applicative<Applicative>
static void applicative_test() {
    using functional::fapply;
    using functional::fpure;
    std::cout << "Applicative (" << typeid(Applicative<int>).name() << ")\n";
    std::cout << "\tapply: " << fapply(Applicative{[] (int val) { return val * 0.5; }}, fpure<Applicative>(15)) << '\n';
}

template<template<typename> typename Monad>
    requires Printable<Monad<int>> && functional::Monad<Monad>
static void monad_test()
{
    using functional::fjoin;
    using functional::fbind;
    using functional::fpure;
    using functional::operator>>;
    std::cout << "Monad (" << typeid(Monad<int>).name() << ")\n";
    std::cout << "\tjoin: " << fpure<Monad>(fpure<Monad>(15)) << " -> " << fjoin(fpure<Monad>(fpure<Monad>(15))) << '\n';
    std::cout << "\tbind: " << fbind([] (int val) { return Monad{val * 0.5}; }, fpure<Monad>(15)) << '\n';
    std::cout << "\tbind(operator): " << (fpure<Monad>(15) >> [] (int val) { return Monad{val * 0.5}; }) << '\n';
    const auto chain_result = fpure<Monad>(15)
            >> [] (auto val) { return fpure<Monad>(val * 0.5); }
            >> [] (auto val) { return fpure<Monad>(static_cast<int>(val)); };
    std::cout << "\tchain: " << chain_result << '\n';
}

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

} // anonymous namespace

int main()
{
    const std::optional a{0};
    const std::optional aa{0.0};
    const auto aaa = std::optional{1ull};
    const auto convertt = [] (int i) noexcept -> double { return i; };
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

    auto partial_test = [] (int a, int b) mutable {
        return a * b;
    };

#if 0
    auto partial_wrapped = functional::PartiallyApplicable(partial_test)(15);
    std::cout << "partial_wrapped -> " << partial_wrapped(5) << '\n';
#endif

    //map(convertt, a);
    return EXIT_SUCCESS;
}
