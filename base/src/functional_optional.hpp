#pragma once

#include <optional>

#include "functional_monad.hpp"

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
