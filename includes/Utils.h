#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include "PCH.h"

namespace Utils {
    struct Token {
        std::string tag;
        std::string value;
        std::vector<std::string> values;
    };

    [[nodiscard]] std::string ToLowerAscii(std::string_view text);
    void ToLowerAsciiInPlace(std::string& text);
    [[nodiscard]] std::vector<Token> TokenizeQuery(std::string_view text);

    [[nodiscard]] std::string StringFromGFxValue(const RE::GFxValue& value);
    [[nodiscard]] std::optional<std::string> TryGetStringMember(const RE::GFxValue& object, const char* memberName);

    // Read string member, lowercase it. Return true
    // if a non-empty field was appended.
    bool AppendLoweredStringMember(const RE::GFxValue& object, const char* memberName,
                                   std::vector<std::string>& out);

    [[nodiscard]] std::optional<std::uint32_t> UIntFromGFxValue(const RE::GFxValue& value);
    [[nodiscard]] std::optional<std::uint32_t> TryGetUIntMember(const RE::GFxValue& object, const char* memberName);

    [[nodiscard]] std::optional<bool> TryGetBoolMember(const RE::GFxValue& object, const char* memberName);
}
