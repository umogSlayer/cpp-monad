#pragma once

#include <type_traits>
#include <utility>

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

    template<typename Input>
    static constexpr auto empty() noexcept
    {
        return T<Input>::empty();
    }
};

} // namespace functional
