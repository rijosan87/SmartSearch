#include "Utils.h"
#include "Localization.h"
#include <Windows.h>

namespace Utils {
    [[nodiscard]] constexpr char ToLowerAsciiChar(char ch) {
        return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch + ('a' - 'A')) : ch;
    }

    [[nodiscard]] constexpr bool IsAsciiSpace(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
    }

    std::string ToLowerAscii(std::string_view text) {
        std::string result;
        result.reserve(text.size());

        for (const auto ch : text) {
            result.push_back(ToLowerAsciiChar(ch));
        }

        return result;
    }

    void ToLowerAsciiInPlace(std::string& text) {
        for (auto& ch : text) {
            ch = ToLowerAsciiChar(ch);
        }
    }

    std::vector<Token> TokenizeQuery(std::string_view text) {
        std::vector<Token> tokens;
        std::string current;

        auto flushToken = [&]() {
            if (current.empty()) {
                return;
            }
            
            Token token;
            const auto colonPos = current.find(':');
            if (colonPos != std::string::npos) {
                token.tag = current.substr(0, colonPos);
                token.value = current.substr(colonPos + 1);
            } else {
                token.value = current;
            }
            if (!token.value.empty()) {
                if (const auto* keywords = Localization::Rewrite(token.value)) {
                    token.values = *keywords;
                }
            }
            tokens.push_back(std::move(token));
            current.clear();
        };

        for (const auto ch : text) {
            if (IsAsciiSpace(ch)) {
                flushToken();
                continue;
            }

            current.push_back(ToLowerAsciiChar(ch));
        }

        flushToken();
        return tokens;
    }

    std::string StringFromGFxValue(const RE::GFxValue& value) {
        if (value.IsString()) {
            const auto* text = value.GetString();
            return text ? text : "";
        }

        if (value.IsStringW()) {
            const auto* text = value.GetStringW();
            if (!text || *text == L'\0') {
                return {};
            }

            // GFx wide strings are UTF-16 on Windows. Convert to UTF-8.
            const int needed = ::WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
            if (needed <= 1) {  // 1 = just the null terminator; <=0 = error
                return {};
            }

            std::string result(static_cast<std::size_t>(needed - 1), '\0');  // -1: drop the null terminator
            ::WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), needed, nullptr, nullptr);
            return result;
        }

        return {};
    }

    std::optional<std::string> TryGetStringMember(const RE::GFxValue& object, const char* memberName) {
        if (!object.IsObject() && !object.IsDisplayObject()) {
            return std::nullopt;
        }

        RE::GFxValue value;
        if (!object.GetMember(memberName, &value)) {
            return std::nullopt;
        }

        if (!value.IsString() && !value.IsStringW()) {
            return std::nullopt;
        }

        return StringFromGFxValue(value);
    }

    bool AppendLoweredStringMember(const RE::GFxValue& object, const char* memberName,
                                   std::vector<std::string>& out) {
        if (!object.IsObject() && !object.IsDisplayObject()) {
            return false;
        }

        RE::GFxValue value;
        if (!object.GetMember(memberName, &value)) {
            return false;
        }

        if (!value.IsString() && !value.IsStringW()) {
            return false;
        }

        // Build the UTF-8 string, lowercase it in place
        std::string s = StringFromGFxValue(value);
        if (s.empty()) {
            return false;
        }
        ToLowerAsciiInPlace(s);
        out.push_back(std::move(s));
        return true;
    }

    std::optional<std::uint32_t> UIntFromGFxValue(const RE::GFxValue& value) {
        if (value.IsNumber()) {
            const auto number = value.GetNumber();
            if (number >= 0.0 && number <= 4294967295.0) {
                return static_cast<std::uint32_t>(number);
            }
        }

        if (value.IsString()) {
            const auto text = StringFromGFxValue(value);
            if (text.empty()) {
                return std::nullopt;
            }

            try {
                std::size_t parsed = 0;
                const auto base = text.starts_with("0x") || text.starts_with("0X") ? 16 : 10;
                const auto offset = base == 16 ? 2U : 0U;
                const auto number = std::stoul(text.substr(offset), &parsed, base);
                if (parsed == text.size() - offset && number <= 0xFFFFFFFFu) {
                    return static_cast<std::uint32_t>(number);
                }
            } catch (...) {
                return std::nullopt;
            }
        }

        return std::nullopt;
    }

    std::optional<std::uint32_t> TryGetUIntMember(const RE::GFxValue& object, const char* memberName) {
        if (!object.IsObject() && !object.IsDisplayObject()) {
            return std::nullopt;
        }

        RE::GFxValue value;
        if (!object.GetMember(memberName, &value)) {
            return std::nullopt;
        }

        return UIntFromGFxValue(value);
    }

    std::optional<bool> TryGetBoolMember(const RE::GFxValue& object, const char* memberName) {
        if (!object.IsObject() && !object.IsDisplayObject()) {
            return std::nullopt;
        }

        RE::GFxValue value;
        if (!object.GetMember(memberName, &value)) {
            return std::nullopt;
        }

        if (value.IsBool()) {
            return value.GetBool();
        }

        return std::nullopt;
    }
}
