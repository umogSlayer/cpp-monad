#pragma once

#include "functional_traits.hpp"

namespace functional
{

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

} // namespace functional
