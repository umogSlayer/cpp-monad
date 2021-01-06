#include <iostream>

#include <functional>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <ranges>
#include <concepts>

namespace functional
{

template<template<typename> typename T, typename T2>
struct is_instance final : public std::false_type {};

template<template<typename> typename T, typename T2>
struct is_instance<T, T<T2>> final : public std::true_type {};

template<template<typename> typename T, typename T2>
constexpr bool is_instance_v = is_instance<T, std::remove_cvref_t<T2>>::value;

template<template<typename> typename T>
struct FunctionalTraits final
{
    template<typename Func, typename Input>
    static constexpr auto map(Func &&func, T<Input> &&input)
    {
        return std::move(input).map(std::forward<Func>(func));
    }

    template<typename Func, typename Input>
    static constexpr auto map(Func &&func, const T<Input> &input)
    {
        return input.map(std::forward<Func>(func));
    }

    template<typename Func, typename Input>
        requires is_instance_v<T, Input>
    static constexpr auto apply(T<Func> &&func, Input &&input)
    {
        return std::forward<Input>(input).apply(std::move(func));
    }

    template<typename Func, typename Input>
        requires is_instance_v<T, Input>
    static constexpr auto apply(const T<Func> &func, Input &&input)
    {
        return std::forward<Input>(input).apply(func);
    }

    template<typename Input>
        requires is_instance_v<T, Input>
    static constexpr auto join(T<Input> &&input)
    {
        return std::move(input).join();
    }

    template<typename Input>
        requires is_instance_v<T, Input>
    static constexpr auto join(const T<Input> &input)
    {
        return input.join();
    }
};


template<template<typename> typename T, typename Input>
constexpr auto fpure(Input &&input)
{
    return T<std::remove_cvref_t<Input>>{std::forward<Input>(input)};
}

template<template<typename> typename T, typename Func, typename Input>
constexpr auto fmap(Func &&func, T<Input> &&input)
{
    return FunctionalTraits<T>::map(std::forward<Func>(func), std::move(input));
}

template<template<typename> typename T, typename Func, typename Input>
constexpr auto fmap(Func &&func, const T<Input> &input)
{
    return FunctionalTraits<T>::map(std::forward<Func>(func), input);
}

template<template<typename> typename T>
concept Functor = requires(double (*func)(int), T<int> t) {
    {fmap(func, t)} -> std::same_as<T<double>>;
    {fpure<T>(0)} -> std::same_as<T<int>>;
};

template<template<typename> typename T, typename Func, typename Input>
    requires is_instance_v<T, Input>
constexpr auto fapply(T<Func> &&func, Input &&input)
{
    return FunctionalTraits<T>::apply(std::move(func), std::forward<Input>(input));
}

template<template<typename> typename T, typename Func, typename Input>
    requires is_instance_v<T, Input>
constexpr auto fapply(const T<Func> &func, Input &&input)
{
    return FunctionalTraits<T>::apply(func, std::forward<Input>(input));
}

template<template<typename> typename T>
concept Applicative = Functor<T> && requires(T<double (*)(int)> func, T<int> t) {
    {fapply(func, t)} -> std::same_as<T<double>>;
};

template<template<typename> typename T, typename Input>
    requires is_instance_v<T, Input>
constexpr auto fjoin(T<Input> &&input)
{
    return FunctionalTraits<T>::join(std::move(input));
}

template<template<typename> typename T, typename Input>
    requires is_instance_v<T, Input>
constexpr auto fjoin(const T<Input> &input)
{
    return FunctionalTraits<T>::join(input);
}

template<template<typename> typename T>
concept Monad = Applicative<T> && requires(T<T<int>> t) {
    {fjoin(t)} -> std::same_as<T<int>>;
};

template<template<typename> typename T, typename Func, typename Input>
    requires is_instance_v<T, decltype(fmap(std::declval<Func>(), std::declval<T<Input> &&>()))>
constexpr auto fbind(Func &&func, T<Input> &&input)
{
    return fjoin(fmap(std::forward<Func>(func), std::move(input)));
}

template<template<typename> typename T, typename Func, typename Input>
    requires is_instance_v<T, decltype(fmap(std::declval<Func>(), std::declval<const T<Input> &>()))>
constexpr auto fbind(Func &&func, const T<Input> &input)
{
    return fjoin(fmap(std::forward<Func>(func), input));
}

template<typename T, typename Func>
constexpr auto operator>>=(T &&value, Func &&func)
{
    return fbind(std::forward<Func>(func), std::forward<T>(value));
}

template<typename ...Args>
constexpr auto chain_m(Args &&...args)
{
    return (... >>= std::forward<Args>(args));
}

} // namespace functional

namespace test_functional {

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
void functor_test() {
    using functional::fmap;
    using functional::fpure;
    std::cout << "Functor (" << typeid(Functor<int>).name() << ")\n";
    std::cout << "\tpure: " << fpure<Functor>(15) << '\n';
    std::cout << "\tmap: " << fmap([] (int val) { return val * 0.5; }, fpure<Functor>(15)) << '\n';
}

template<template<typename> typename Applicative>
    requires Printable<Applicative<int>> && functional::Applicative<Applicative>
void applicative_test() {
    using functional::fapply;
    using functional::fpure;
    std::cout << "Applicative (" << typeid(Applicative<int>).name() << ")\n";
    std::cout << "\tapply: " << fapply(Applicative{[] (int val) { return val * 0.5; }}, fpure<Applicative>(15)) << '\n';
}

template<template<typename> typename Monad>
    requires Printable<Monad<int>> && functional::Monad<Monad>
void monad_test() {
    using functional::fjoin;
    using functional::fbind;
    using functional::fpure;
    using functional::operator>>=;
    using functional::chain_m;
    std::cout << "Monad (" << typeid(Monad<int>).name() << ")\n";
    std::cout << "\tjoin: " << fpure<Monad>(fpure<Monad>(15)) << " -> " << fjoin(fpure<Monad>(fpure<Monad>(15))) << '\n';
    std::cout << "\tbind: " << fbind([] (int val) { return Monad{val * 0.5}; }, fpure<Monad>(15)) << '\n';
    std::cout << "\tbind(operator): " << (fpure<Monad>(15) >>= [] (int val) { return Monad{val * 0.5}; }) << '\n';
    const auto chain_result = chain_m(
            fpure<Monad>(15),
            [] (auto val) { return fpure<Monad>(val * 0.5); },
            [] (auto val) { return fpure<Monad>(static_cast<int>(val)); });
    std::cout << "\tchain: " << chain_result << '\n';
}

template<typename T>
static std::ostream &operator<<(std::ostream &stream, const std::optional<T> &value) {
    if (value) {
        stream << "std::optional{" << *value << "}";
    } else {
        stream << "std::nullopt";
    }
    return stream;
}

namespace functional
{

template<>
struct FunctionalTraits<std::optional> final
{
    template<typename Func, typename Input>
    static constexpr auto map(Func &&func, std::optional<Input> &&input)
    {
        using functional::fpure;
        using FuncRet = std::remove_cvref_t<decltype(std::forward<Func>(func)(std::move(*input)))>;
        if (!input)
        {
            return std::optional<FuncRet>{};
        }
        return fpure<std::optional>(std::forward<Func>(func)(std::move(*input)));
    }

    template<typename Func, typename Input>
    static constexpr auto map(Func &&func, const std::optional<Input> &input)
    {
        using functional::fpure;
        using FuncRet = std::remove_cvref_t<decltype(std::forward<Func>(func)(*input))>;
        if (!input)
        {
            return std::optional<FuncRet>{};
        }
        return fpure<std::optional>(std::forward<Func>(func)(*input));
    }

    template<typename Func, typename Input>
        requires is_instance_v<std::optional, Input>
    static constexpr auto apply(std::optional<Func> &&func, Input &&input)
    {
        using FuncRet = std::remove_cvref_t<decltype(map(std::move(*func), std::forward<Input>(input)))>;
        if (!func)
        {
            return FuncRet{};
        }
        return map(std::move(*func), std::forward<Input>(input));
    }

    template<typename Func, typename Input>
        requires is_instance_v<std::optional, Input>
    static constexpr auto apply(const std::optional<Func> &func, Input &&input)
    {
        using FuncRet = std::remove_cvref_t<decltype(map(*func, std::forward<Input>(input)))>;
        if (!func)
        {
            return FuncRet{};
        }
        return map(*func, std::forward<Input>(input));
    }

    template<typename Input>
        requires is_instance_v<std::optional, Input>
    static constexpr auto join(std::optional<Input> &&input)
    {
        if (!input)
        {
            return Input{};
        }
        return std::move(*input);
    }

    template<typename Input>
        requires is_instance_v<std::optional, Input>
    static constexpr auto join(const std::optional<Input> &input)
    {
        if (!input)
        {
            return Input{};
        }
        return *input;
    }
};

} // namespace functional

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

    //map(convertt, a);
    return EXIT_SUCCESS;
}
