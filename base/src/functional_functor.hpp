#pragma once

#include "functional_traits.hpp"

namespace functional
{

namespace detail
{

struct MoveOnlyResult final
{
    constexpr MoveOnlyResult() noexcept = default;
    constexpr ~MoveOnlyResult() noexcept = default;

    constexpr MoveOnlyResult(MoveOnlyResult &&) noexcept = default;
    constexpr MoveOnlyResult(const MoveOnlyResult &) noexcept = delete;

    constexpr MoveOnlyResult &operator=(MoveOnlyResult &&) noexcept = default;
    constexpr MoveOnlyResult &operator=(const MoveOnlyResult &) noexcept = delete;
};

struct MoveOnlyData final
{
    constexpr MoveOnlyData() noexcept = default;
    constexpr ~MoveOnlyData() noexcept = default;

    constexpr MoveOnlyData(MoveOnlyData &&) noexcept = default;
    constexpr MoveOnlyData(const MoveOnlyData &) noexcept = delete;

    constexpr MoveOnlyData &operator=(MoveOnlyData &&) noexcept = default;
    constexpr MoveOnlyData &operator=(const MoveOnlyData &) noexcept = delete;
};

struct MoveOnlyFunctionObject final
{
    constexpr MoveOnlyFunctionObject() noexcept = default;
    constexpr ~MoveOnlyFunctionObject() noexcept = default;

    constexpr MoveOnlyFunctionObject(MoveOnlyFunctionObject &&) noexcept = default;
    constexpr MoveOnlyFunctionObject(const MoveOnlyFunctionObject &) noexcept = delete;

    constexpr MoveOnlyFunctionObject &operator=(MoveOnlyFunctionObject &&) noexcept = default;
    constexpr MoveOnlyFunctionObject &operator=(const MoveOnlyFunctionObject &) noexcept = delete;

    constexpr MoveOnlyResult operator()(MoveOnlyData &&) const noexcept
    {
        return {};
    }
};

} // namespace detail

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
concept Functor = requires(detail::MoveOnlyFunctionObject func, T<detail::MoveOnlyData> t) {
    {fmap(std::move(func), std::move(t))} -> std::same_as<T<detail::MoveOnlyResult>>;
    {fpure<T>(detail::MoveOnlyData{})} -> std::same_as<T<detail::MoveOnlyData>>;
};

} // namespace functional
