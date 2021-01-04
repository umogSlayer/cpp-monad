#include <iostream>

#include <functional>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <ranges>

namespace functional
{

template<template<typename> typename T, typename T2>
struct is_instance final : public std::false_type {};

template<template<typename> typename T, typename T2>
struct is_instance<T, T<T2>> final : public std::true_type {};

template<template<typename> typename T, typename T2>
constexpr bool is_instance_v = is_instance<T, std::remove_cvref_t<T2>>::value;

template<template<typename> typename T>
struct FunctorTypeclassTraits;

template<template<typename> typename T, typename T1>
auto functional_map(auto &&func, T<T1> &&value) noexcept
{
    return FunctorTypeclassTraits<T>::map(std::forward<decltype(func)>(func), std::move(value));
}

template<template<typename> typename T, typename T1>
auto functional_map(auto &&func, const T<T1> &value) noexcept
{
    return FunctorTypeclassTraits<T>::map(std::forward<decltype(func)>(func), value);
}

template<template<typename> typename T>
concept FunctorTypeclass = requires (T<int> t, double (*simple_func)(int) noexcept) {
    {functional_map(simple_func, t)} -> std::same_as<T<double>>;
    {T<int>{0}};
};

template<typename Function, typename Input, typename Output>
concept FunctionConcept = requires (Function function, Input input) {
    {function(std::forward<Input>(input))} -> std::same_as<Output>;
};

template<template<typename> typename Functor, typename Function, typename Input, typename Output>
concept FunctorConcept = FunctionConcept<Function, Input, Output>
    && requires (Input input, Functor<Input> input_functor, Function function) {
        {functional_map(function, input_functor)} -> std::same_as<Functor<Output>>;
        {Functor<Input>{input}} -> std::same_as<Functor<Input>>;
    };

} // namespace functional


namespace functional
{
template<>
struct FunctorTypeclassTraits<std::optional> final
{
    template<typename T1>
    static auto map(auto &&func, std::optional<T1> &&value) noexcept
    {
        using FuncRetVal = decltype(std::forward<decltype(func)>(func)(std::move(*value)));
        if (value)
        {
            return std::optional<FuncRetVal>{std::forward<decltype(func)>(func)(std::move(*value))};
        }
        return std::optional<FuncRetVal>{};
    }

    template<typename T1>
    static auto map(auto &&func, const std::optional<T1> &value) noexcept
    {
        using FuncRetVal = decltype(std::forward<decltype(func)>(func)(*value));
        if (value)
        {
            return std::optional<FuncRetVal>{std::forward<decltype(func)>(func)(*value)};
        }
        return std::optional<FuncRetVal>{};
    }
};

template<typename T>
using DefaultVector = std::vector<T>;

template<>
struct FunctorTypeclassTraits<DefaultVector> final
{
    template<typename Func, typename T1>
    static auto map(Func &&func, DefaultVector<T1> &&input_vector) noexcept
    {
        using FuncRetVal = std::remove_cvref_t<decltype(std::forward<Func>(func)(std::move(input_vector.front())))>;
        DefaultVector<FuncRetVal> retval;
        retval.reserve(input_vector.size());
        for (auto &val : input_vector)
        {
            retval.push_back(std::move(val));
        }
        return retval;
    }

    template<typename Func, typename T1>
    static auto map(Func &&func, const DefaultVector<T1> &input_vector) noexcept
    {
        return input_vector | std::ranges::views::transform(std::forward<Func>(func));
    }
};
} // namespace functional

namespace functional::test {

template<typename T>
struct MyMonad final {
    T value;
};

template<typename Func, typename T>
auto functional_map(Func &&func, MyMonad<T> &&value) noexcept(noexcept(std::forward<Func>(func)(std::move(value.value)))) {
    return MyMonad{std::forward<Func>(func)(std::move(value.value))};
}

template<typename Func, typename T>
auto functional_map(Func &&func, const MyMonad<T> &value) noexcept(noexcept(std::forward<Func>(func)(value.value))) {
    return MyMonad{std::forward<Func>(func)(value.value)};
}

template<typename T>
    requires requires(T t, std::ostream &stream) {
        { stream << t };
    }
std::ostream &operator<<(std::ostream &stream, const MyMonad<T> &value) {
    return stream << "MyMonad{" << value.value << "}";
}

} // namespace functional::test

const int &get_const_int()
{
    static int a = 5;
    return a;
}

template<typename T>
concept Printable = requires(T val, std::ostream &stream) {
    { stream << val };
};

template<template<typename> typename Functor>
    requires Printable<Functor<int>> && functional::FunctorTypeclass<Functor>
void functor_test() {
    using functional::functional_map;
    std::cout << "Value: " << Functor{15} << '\n';
    std::cout << "Value: " << functional_map([] (int val) { return val * 0.5; }, Functor{15}) << '\n';
}

template<typename T>
static std::ostream &operator<<(std::ostream &stream, const std::vector<T> &range) {
    stream << "[";
    if (!range.empty()) {
        stream << range.front();
    }
    std::ranges::for_each(range, [&stream] (const Printable auto &value) { stream << ", " << value; });
    stream << "]";
    return stream;
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

int main()
{
    const std::optional a{0};
    const std::optional aa{0.0};
    const auto aaa = std::optional{1ull};
    const auto convertt = [] (int i) noexcept -> double { return i; };
    functional::FunctorTypeclassTraits<std::optional>::map(convertt, a);
    using functional::functional_map;
    std::cout << functional_map(convertt, a) << '\n';
    std::cout << functional_map(convertt, aaa) << '\n';
    std::cout << functional_map(convertt, std::optional{1}) << '\n';
    std::cout << functional_map(convertt, std::optional<int>{}) << '\n';
    std::cout << functional_map<functional::DefaultVector>([] (int value) -> double { return value; }, std::vector<int>{3, 5}) << '\n';
    functor_test<functional::test::MyMonad>();
    functor_test<std::optional>();

    //map(convertt, a);
    return EXIT_SUCCESS;
}
