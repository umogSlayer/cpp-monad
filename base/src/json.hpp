#pragma once

#include "functional_applicative.hpp"
#include "functional_alternative.hpp"
#include "functional_traits.hpp"

#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <unordered_map>
#include <vector>
#include <string>

namespace json
{
struct JsonValue;

using JsonString = std::string;
using JsonNumber = double;
using JsonList = std::vector<JsonValue>;
using JsonObject = std::vector<std::pair<JsonString, JsonValue>>;

struct JsonValue final
{
    std::variant<JsonObject, JsonString, JsonNumber, JsonList> value;
};

template<typename Visitor>
constexpr auto visit(Visitor &&visitor, JsonValue &&json_value)
{
    return std::visit(std::forward<Visitor>(visitor), std::move(json_value.value));
}

template<typename Visitor>
constexpr auto visit(Visitor &&visitor, const JsonValue &json_value)
{
    return std::visit(std::forward<Visitor>(visitor), json_value.value);
}

struct ParseError final
{
    std::string error_message;
};
using NotParsed = std::monostate;

template<typename ...Args>
struct overloaded_t : Args...
{
    explicit overloaded_t(Args ...args)
        : Args(std::move(args))...
    {
    }
    using Args::operator()...;
};

template<typename ...Args>
constexpr auto overloaded(Args &&...args)
{
    return overloaded_t<std::remove_cvref_t<Args>...>{std::forward<Args>(args)...};
}

template<typename Type>
struct Parser final
{
    explicit constexpr Parser() = default;
    explicit constexpr Parser(Type &&pure_value)
        : value(std::move(pure_value))
    {
    }
    explicit constexpr Parser(const Type &pure_value)
        : value(pure_value)
    {
    }
    explicit constexpr Parser(ParseError &&parse_error)
        : value(std::move(parse_error))
    {
    }

    explicit constexpr Parser(
        std::variant<NotParsed, Type, ParseError> value_,
        std::string error_prefix_,
        std::string error_suffix_)
        : value(std::move(value_))
        , error_prefix(std::move(error_prefix_))
        , error_suffix(std::move(error_suffix_))
    {
    }

    std::variant<NotParsed, Type, ParseError> value;
    std::string error_prefix;
    std::string error_suffix;
};

template<typename Func>
    requires (std::is_invocable_v<const Func &, const JsonObject &>
              && functional::is_instance_v<Parser, std::invoke_result_t<const Func &, const JsonObject &>>)
consteval auto with_object(std::string_view class_name_sv, Func &&func)
{
    using RetVal = std::remove_cvref_t<std::invoke_result_t<const Func &, const JsonObject &>>;
    return [forwarded_func = std::forward<Func>(func), class_name = class_name_sv] (const JsonValue &json_value) noexcept(std::is_nothrow_invocable_v<const Func &, const JsonObject &>) {
        return visit(
                overloaded([&class_name, &forwarded_func] (const JsonObject &json_object) {
                               auto func_result = forwarded_func(json_object);
                               using namespace std::literals;
                               func_result.error_prefix = "When parsing JSON object for "s + std::string(class_name) + ": " + std::move(func_result.error_prefix);
                               return func_result;
                           },
                           [&class_name] (const auto &) {
                               return RetVal{ParseError{
                                   "Expected JSON object for " + std::string(class_name)
                               }};
                           }),
                json_value);
    };
}

template<typename Func>
    requires (std::is_invocable_v<const Func &, const JsonString &>
              && functional::is_instance_v<Parser, std::invoke_result_t<const Func &, const JsonString &>>)
consteval auto with_string(std::string_view class_name_sv, Func &&func)
{
    using RetVal = std::remove_cvref_t<std::invoke_result_t<const Func &, const JsonString &>>;
    return [forwarded_func = std::forward<Func>(func), class_name = class_name_sv] (const JsonValue &json_value) noexcept(std::is_nothrow_invocable_v<const Func &, const JsonString &>) {
        return visit(
                overloaded([&class_name, &forwarded_func] (const JsonString &json_string) {
                               auto func_result = forwarded_func(json_string);
                               using namespace std::literals;
                               func_result.error_prefix = "When parsing JSON string for "s + std::string(class_name) + ": " + std::move(func_result.error_prefix);
                               return func_result;
                           },
                           [&class_name] (const auto &) {
                               return RetVal{ParseError{
                                   "Expected JSON string for " + std::string(class_name)
                               }};
                           }),
                json_value);
    };
}

template<typename Func>
    requires (std::is_invocable_v<const Func &, const JsonList &>
              && functional::is_instance_v<Parser, std::invoke_result_t<const Func &, const JsonList &>>)
consteval auto with_list(std::string_view class_name_sv, Func &&func)
{
    using RetVal = std::remove_cvref_t<std::invoke_result_t<const Func &, const JsonList &>>;
    return [forwarded_func = std::forward<Func>(func), class_name = class_name_sv] (const JsonValue &json_value) noexcept(std::is_nothrow_invocable_v<const Func &, const JsonList &>) {
        return std::visit(
                overloaded([&class_name, &forwarded_func] (const JsonList &json_list) {
                               auto func_result = forwarded_func(json_list);
                               using namespace std::literals;
                               func_result.error_prefix = "When parsing JSON list for "s + std::string(class_name) + ": " + std::move(func_result.error_prefix);
                               return func_result;
                           },
                           [&class_name] (const auto &) {
                               return RetVal{ParseError{
                                   "Expected JSON list for " + std::string(class_name)
                               }};
                           }),
                json_value);
    };
}

template<typename Func>
    requires (std::is_invocable_v<const Func &, const JsonNumber &>
              && functional::is_instance_v<Parser, std::invoke_result_t<const Func &, JsonNumber>>)
consteval auto with_number(std::string_view class_name_sv, Func &&func)
{
    using RetVal = std::remove_cvref_t<std::invoke_result_t<const Func &, JsonNumber>>;
    return [forwarded_func = std::forward<Func>(func), class_name = class_name_sv] (const JsonValue &json_value) noexcept(std::is_nothrow_invocable_v<const Func &, JsonNumber>) {
        return visit(
                overloaded([&class_name, &forwarded_func] (const JsonNumber json_number) {
                               auto func_result = forwarded_func(json_number);
                               using namespace std::literals;
                               func_result.error_prefix = "When parsing JSON number for "s + std::string(class_name) + ": " + std::move(func_result.error_prefix);
                               return func_result;
                           },
                           [&class_name] (const auto &) {
                               return RetVal{ParseError{
                                   "Expected JSON number for " + std::string(class_name)
                               }};
                           }),
                json_value);
    };
}

template<typename JsonType>
constexpr auto json_type_parser()
{
    using namespace std::literals;
    if constexpr (std::is_same_v<JsonType, JsonObject>)
    {
        return with_object("JsonObject"sv, [] (const JsonObject &value) {return functional::fpure<Parser>(value);});
    }
    else if constexpr (std::is_same_v<JsonType, JsonString>)
    {
        return with_string("JsonString"sv, [] (const JsonString &value) {return functional::fpure<Parser>(value);});
    }
    else if constexpr (std::is_same_v<JsonType, JsonList>)
    {
        return with_list("JsonList"sv, [] (const JsonList &value) {return functional::fpure<Parser>(value);});
    }
    else if constexpr (std::is_same_v<JsonType, JsonNumber>)
    {
        return with_number("JsonNumber"sv, [] (const JsonNumber value) {return functional::fpure<Parser>(value);});
    }
    else
    {
        struct InvalidType {};
        return InvalidType{};
    }
}

template<typename FieldType>
constexpr Parser<FieldType> parse_field(const JsonObject &object, std::string_view field_name)
{
    using namespace std::literals;
    const auto found_element = std::find_if(begin(object), end(object), [field_name] (const auto &field) {
                                                return field.first == field_name;
                                            });
    if (found_element != end(object))
    {
        constexpr auto parser_func = json_type_parser<FieldType>();
        auto func_result = parser_func(found_element->second);
        func_result.error_prefix = "When parsing JSON object field \""s + std::string(field_name) + "\": " + std::move(func_result.error_prefix);
        return func_result;
    }
    return Parser<FieldType>{ParseError{
        "Expected JSON object field \""s + std::string(field_name) + "\""
    }};
}

template<typename Func, typename T>
constexpr auto fmap(Func &&func, Parser<T> &&value)
{
    using OutputType = std::remove_cvref_t<decltype(std::forward<Func>(func)(std::declval<T &&>()))>;
    return std::visit(
            overloaded([&func] (T &&wrapped_value) {
                           return functional::fpure<Parser>(std::forward<Func>(func)(std::move(wrapped_value)));
                       },
                       [] (NotParsed) {
                           return functional::fempty<Parser, OutputType>();
                       },
                       [&error_prefix = value.error_prefix, &error_suffix = value.error_suffix] (ParseError &&parse_error) {
                           return Parser<OutputType>{
                               ParseError{
                                   .error_message = std::move(error_prefix)
                                       + " " + std::move(parse_error.error_message)
                                       + " " + std::move(error_suffix)
                               }
                           };
                       }),
            std::move(value.value));
}

template<typename Func, typename T>
constexpr auto fmap(Func &&func, const Parser<T> &value)
{
    using OutputType = std::remove_cvref_t<decltype(std::forward<Func>(func)(std::declval<const T &>()))>;
    return std::visit(
            overloaded([&func] (const T &wrapped_value) {
                           return functional::fpure<Parser>(std::forward<Func>(func)(wrapped_value));
                       },
                       [] (NotParsed) {
                           return functional::fempty<Parser, OutputType>();
                       },
                       [&error_prefix = value.error_prefix, &error_suffix = value.error_suffix] (const ParseError &parse_error) {
                           return Parser<OutputType>{
                               ParseError{
                                   .error_message = error_prefix
                                       + " " + parse_error.error_message
                                       + " " + error_suffix
                               }
                           };
                       }),
            value.value);
}

template<typename Func, typename ParserT>
    requires (functional::is_instance_v<Parser, ParserT>)
constexpr auto fapply(Parser<Func> &&func, ParserT &&value)
{
    using OutputType = std::remove_cvref_t<decltype(fmap(std::declval<Func &&>(), std::forward<ParserT>(value)))>;
    return std::visit(
            overloaded([&value] (Func &&wrapped_func) {
                           return fmap(std::move(wrapped_func), std::forward<ParserT>(value));
                       },
                       [] (NotParsed) {
                           return OutputType{};
                       },
                       [&error_prefix = func.error_prefix, &error_suffix = func.error_suffix] (ParseError &&parse_error) {
                           return OutputType{
                               ParseError{
                                   .error_message = std::move(error_prefix)
                                       + " " + std::move(parse_error.error_message)
                                       + " " + std::move(error_suffix)
                               }
                           };
                       }),
            std::move(func.value));
}

template<typename Func, typename ParserT>
    requires (functional::is_instance_v<Parser, ParserT>)
constexpr auto fapply(const Parser<Func> &func, ParserT &&value)
{
    using OutputType = std::remove_cvref_t<decltype(fmap(std::declval<const Func &>(), std::forward<ParserT>(value)))>;
    return std::visit(
            overloaded([&value] (const Func &wrapped_func) {
                           return fmap(wrapped_func, std::forward<ParserT>(value));
                       },
                       [] (NotParsed) {
                           return OutputType{};
                       },
                       [&error_prefix = func.error_prefix, &error_suffix = func.error_suffix] (const ParseError &parse_error) {
                           return OutputType{
                               ParseError{
                                   .error_message = error_prefix
                                       + " " + parse_error.error_message
                                       + " " + error_suffix
                               }
                           };
                       }),
            func.value);
}

template<typename InputValue, typename InputWrapped>
    requires std::is_same_v<Parser<InputValue>, std::remove_cvref_t<InputWrapped>>
constexpr auto falternate(Parser<InputValue> &&lhs, InputWrapped &&rhs)
{
    return std::visit(
            overloaded([&lhs] (const InputValue &) {
                           return std::move(lhs);
                       },
                       [&rhs] (NotParsed) {
                           return std::forward<InputWrapped>(rhs);
                       },
                       [&error_prefix = lhs.error_prefix, &error_suffix = lhs.error_suffix, &rhs] (ParseError &&parse_error) {
                           std::string lhs_error_message = std::move(error_prefix)
                               + " " + std::move(parse_error.error_message)
                               + " " + std::move(error_suffix);
                           return Parser<InputValue>{
                               std::forward<InputWrapped>(rhs).value,
                               "alternative: [" + std::move(lhs_error_message) + " | " + std::forward<InputWrapped>(rhs).error_prefix + "]",
                               std::forward<InputWrapped>(rhs).error_suffix,
                           };
                       }),
            std::move(lhs.value));
}

template<typename InputValue, typename InputWrapped>
    requires std::is_same_v<Parser<InputValue>, std::remove_cvref_t<InputWrapped>>
constexpr auto falternate(const Parser<InputValue> &lhs, InputWrapped &&rhs)
{
    return std::visit(
            overloaded([&lhs] (const InputValue &) {
                           return lhs;
                       },
                       [&rhs] (NotParsed) {
                           return std::forward<InputWrapped>(rhs);
                       },
                       [&error_prefix = lhs.error_prefix, &error_suffix = lhs.error_suffix, &rhs] (const ParseError &parse_error) {
                           std::string lhs_error_message = error_prefix
                               + " " + parse_error.error_message
                               + " " + error_suffix;
                           return Parser<InputValue>{
                               std::forward<InputWrapped>(rhs).value,
                               "alternative: [" + std::move(lhs_error_message) + " | " + std::forward<InputWrapped>(rhs).error_prefix + "]",
                               std::forward<InputWrapped>(rhs).error_suffix,
                           };
                       }),
            lhs.value);
}

inline namespace debug
{

template<typename T>
std::ostream &operator<<(std::ostream &stream, const json::Parser<T> &value)
{
    std::visit(
            json::overloaded([&] (const T &v) {
                                 stream << "Parser{" << v << "}";
                             },
                             [&] (json::NotParsed) {
                                 stream << "Parser{NotParsed}";
                             },
                             [&] (const json::ParseError &parse_error) {
                                 stream << "Parser{ParseError{\"" << value.error_prefix << " " << parse_error.error_message << " " + value.error_suffix << "}}";
                             }),
            value.value);
    return stream;
}

} // namespace debug

} // namespace json
