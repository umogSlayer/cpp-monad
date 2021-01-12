#pragma once

#include "functional_applicative.hpp"
#include <utility>

namespace functional
{

template<template<typename> typename T, typename Input>
    requires is_instance_v<T, Input>
constexpr auto fempty() noexcept
{
    return FunctionalTraits<T>::empty();
}

template<template<typename> typename T, typename Input1, typename Input2>
    requires is_instance_v<T, Input2>
constexpr auto falternate(T<Input1> &&lhs, Input2 &&rhs)
{
    return FunctionalTraits<T>::alternate(std::move(lhs), std::forward<Input2>(rhs));
}

template<template<typename> typename T, typename Input1, typename Input2>
    requires is_instance_v<T, Input2>
constexpr auto falternate(const T<Input1> &lhs, Input2 &&rhs)
{
    return FunctionalTraits<T>::alternate(lhs, std::forward<Input2>(rhs));
}

template<template<typename> typename T>
concept Monad = Applicative<T> && requires(T<int> t1, T<int> t2) {
    {fempty<T, int>()} noexcept -> std::same_as<T<int>>;
    {falternate(t1, t2)} -> std::same_as<T<int>>;
};

template<typename Lhs, typename Rhs>
constexpr auto operator|(Lhs &&lhs, Rhs &&rhs)
{
    return falternate(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
}

} // namespace functional
