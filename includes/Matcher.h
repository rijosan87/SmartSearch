#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "PCH.h"
#include "Utils.h"  // Utils::Token, for the FieldsMatchTokens declaration below

namespace Matcher {
    void AddField(std::vector<std::string>& fields, const char* text);
    void AddField(std::vector<std::string>& fields, std::string_view text);
    void AddFormTypeAliasFields(std::vector<std::string>& fields, RE::FormType formType);
    void AddKeywordFields(std::vector<std::string>& fields, RE::TESForm& form);
    void AddEnchantmentFields(std::vector<std::string>& fields, const RE::TESForm& form);
    void AddFormFields(std::vector<std::string>& fields, std::uint32_t formID);

    [[nodiscard]] bool FieldsMatchQuery(const std::vector<std::string>& fields, std::string_view query);
    [[nodiscard]] bool FieldsMatchTokens(const std::vector<std::string>& fields,
                                         const std::vector<Utils::Token>& tokens,
                                         const RE::TESForm* form, bool isEnchanted);
    void AddSkyUIEntryFields(const RE::GFxValue& entry, std::string_view nameAttribute,
                             const RE::TESForm* form, std::vector<std::string>& fields);

    [[nodiscard]] bool MatchSkyUIEntry(const RE::GFxValue& entry, std::string_view query, std::string_view nameAttribute = "text");
    [[nodiscard]] bool MatchSkyUIEntry(const RE::GFxValue& entry, const std::vector<Utils::Token>& tokens,
                                       std::string_view nameAttribute = "text", std::uint32_t rowId = 0);
    [[nodiscard]] bool MatchRawArguments(std::uint32_t formID, std::string_view query, std::string_view entryText);
    [[nodiscard]] bool MatchNameOnly(const RE::GFxValue& entry, std::string_view query,
                                     std::string_view nameAttribute = "text");

    void ClearFieldCache();

    // Per-row live-field cache control
    void InvalidateRowBlobs();
    void ResetRowBlobs();
    [[nodiscard]] std::uint32_t NextRowId();

    // TEMP Diagnostic.
    struct PerfSplit { double readUs; double matchUs; };
    PerfSplit& PerfSplitAccum();
}
