#pragma once

#include "functional_functor.hpp"

namespace functional
{

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
concept Applicative = Functor<T> && requires(T<detail::MoveOnlyFunctionObject> func, T<detail::MoveOnlyData> t) {
    {fapply(std::move(func), std::move(t))} -> std::same_as<T<detail::MoveOnlyResult>>;
};

template<typename Lhs, typename Rhs>
constexpr auto operator*(Lhs &&lhs, Rhs &&rhs)
{
    return fapply(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
}

} // namespace functional
