#pragma once

#include "functional_applicative.hpp"
#include <type_traits>
#include <utility>

namespace functional
{

template<template<typename> typename T, typename Input>
    requires requires() {
        {typename FunctionalTraits<T>::empty<Input>()} noexcept -> std::same_as<T<Input>>;
    }
constexpr auto fempty() noexcept
{
    return typename FunctionalTraits<T>::empty<Input>();
}

template<template<typename> typename T, typename Input>
    requires requires() {
        {T<Input>{}} noexcept -> std::same_as<T<Input>>;
    }
constexpr auto fempty() noexcept
{
    return T<Input>{};
}

template<template<typename> typename T, typename InputValue, typename InputWrapped>
    requires std::is_same_v<T<InputValue>, std::remove_cvref_t<InputWrapped>>
constexpr auto falternate(T<InputValue> &&lhs, InputWrapped &&rhs)
{
    return FunctionalTraits<T>::alternate(std::move(lhs), std::forward<InputWrapped>(rhs));
}

template<template<typename> typename T, typename InputValue, typename InputWrapped>
    requires std::is_same_v<T<InputValue>, std::remove_cvref_t<InputWrapped>>
constexpr auto falternate(const T<InputValue> &lhs, InputWrapped &&rhs)
{
    return FunctionalTraits<T>::alternate(lhs, std::forward<InputWrapped>(rhs));
}

template<template<typename> typename T>
concept Alternative = Applicative<T> && requires(T<detail::MoveOnlyData> t1, T<detail::MoveOnlyData> t2) {
    {fempty<T, detail::MoveOnlyData>()} noexcept -> std::same_as<T<detail::MoveOnlyData>>;
    {falternate(std::move(t1), std::move(t2))} -> std::same_as<T<detail::MoveOnlyData>>;
};

template<typename Lhs, typename Rhs>
constexpr auto operator|(Lhs &&lhs, Rhs &&rhs)
{
    return falternate(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
}

} // namespace functional
