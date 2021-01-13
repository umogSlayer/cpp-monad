#pragma once

#include "functional_applicative.hpp"

namespace functional
{

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
concept Monad = Applicative<T> && requires(T<T<detail::MoveOnlyData>> t) {
    {fjoin(std::move(t))} -> std::same_as<T<detail::MoveOnlyData>>;
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
constexpr auto operator>>(T &&value, Func &&func)
{
    return fbind(std::forward<Func>(func), std::forward<T>(value));
}

} // namespace functional
