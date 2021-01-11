#pragma once

#include <functional>

namespace functional
{

template<typename T, typename ...Args>
struct PartiallyApplicable : T
{
    constexpr PartiallyApplicable(T t, Args ...args) noexcept
        : T(std::move(t))
        , saved_args_{std::move(args)...}
    {
    }

    template<typename ...NonSavedArgs>
        requires std::is_invocable_v<const T &, const Args &..., NonSavedArgs &&...>
    constexpr auto operator()(NonSavedArgs &&...args) const &
    {
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return std::invoke(static_cast<const T &>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...);
        }, std::index_sequence_for<Args...>{});
    }

    template<typename ...NonSavedArgs>
        requires std::is_invocable_v<T &, Args &..., NonSavedArgs &&...>
    constexpr auto operator()(NonSavedArgs &&...args) &
    {
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return std::invoke(static_cast<T &>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...);
        }, std::index_sequence_for<Args...>{});
    }

    template<typename ...NonSavedArgs>
        requires std::is_invocable_v<T &&, Args &&..., NonSavedArgs &&...>
    constexpr auto operator()(NonSavedArgs &&...args) &&
    {
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return std::invoke(static_cast<T &&>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...);
        }, std::index_sequence_for<Args...>{});
    }

    template<typename ...NonSavedArgs>
        requires (!std::is_invocable_v<const T &, const Args &..., NonSavedArgs &&...>)
    constexpr auto operator()(NonSavedArgs &&...args) const &
    {
        using RetType = PartiallyApplicable<T, Args..., std::remove_cvref_t<NonSavedArgs>...>;
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return RetType{static_cast<const T &>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...};
        }, std::index_sequence_for<Args...>{});
    }

    template<typename ...NonSavedArgs>
        requires (!std::is_invocable_v<T &, Args &..., NonSavedArgs &&...>)
    constexpr auto operator()(NonSavedArgs &&...args) &
    {
        using RetType = PartiallyApplicable<T, Args..., std::remove_cvref_t<NonSavedArgs>...>;
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return RetType{static_cast<T &>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...};
        }, std::index_sequence_for<Args...>{});
    }

    template<typename ...NonSavedArgs>
        requires (!std::is_invocable_v<T &&, Args &&..., NonSavedArgs &&...>)
    constexpr auto operator()(NonSavedArgs &&...args) &&
    {
        using RetType = PartiallyApplicable<T, Args..., std::remove_cvref_t<NonSavedArgs>...>;
        return unpack_tuple([this, &args...] <typename ...TupleArgs> (TupleArgs &&...tuple_args) {
            return RetType{static_cast<T &&>(*this), std::forward<TupleArgs>(tuple_args)..., std::forward<NonSavedArgs>(args)...};
        }, std::index_sequence_for<Args...>{});
    }

private:
    template<typename Callable, size_t ...Idx>
    constexpr auto unpack_tuple(Callable &&callable, std::index_sequence<Idx...>) const &
    {
        return std::forward<Callable>(callable)(std::get<Idx>(saved_args_)...);
    }

    template<typename Callable, size_t ...Idx>
    constexpr auto unpack_tuple(Callable &&callable, std::index_sequence<Idx...>) &
    {
        return std::forward<Callable>(callable)(std::get<Idx>(saved_args_)...);
    }

    template<typename Callable, size_t ...Idx>
    constexpr auto unpack_tuple(Callable &&callable, std::index_sequence<Idx...>) &&
    {
        return std::forward<Callable>(callable)(std::get<Idx>(std::move(saved_args_))...);
    }

private:
    std::tuple<Args...> saved_args_;
};

template<typename T>
constexpr PartiallyApplicable<std::remove_cvref_t<T>> partially_applicable(T &&t)
    noexcept(noexcept(PartiallyApplicable<std::remove_cvref_t<T>>{std::forward<T>(t)}))
{
    return {std::forward<T>(t)};
}

} // namespace functional
