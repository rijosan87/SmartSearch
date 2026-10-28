#include "Scaleform.h"
#include "Utils.h"
#include "Matcher.h"
#include "Config.h"
#include <chrono>

namespace Scaleform {
    void SmartSearchMatchFunction::Call(Params& params) {
        bool result = false;

        if (params.argCount >= 2 && params.args[0].IsObject()) {
            const auto query = Utils::StringFromGFxValue(params.args[1]);
            logger::debug("SmartSearch evaluating object query: {}", query);
            result = Matcher::MatchSkyUIEntry(params.args[0], query);
        } else if (params.argCount >= 2) {
            const auto formID = Utils::UIntFromGFxValue(params.args[0]).value_or(0);
            const auto query = Utils::StringFromGFxValue(params.args[1]);
            const auto entryText = params.argCount >= 3 ? Utils::StringFromGFxValue(params.args[2]) : std::string{};
            logger::debug("SmartSearch evaluating raw query: {}", query);
            result = Matcher::MatchRawArguments(formID, query, entryText);
        }

        if (params.retVal) {
            params.retVal->SetBoolean(result);
        }
    }

    // Hook for SkyUI's NameFilter.applyFilter. 
    void SkyUIApplyFilterFunction::Call(Params& params) {
        if (params.argCount < 1 || !params.args[0].IsArray() || !params.thisPtr) {
            return;  // Not the shape we expect; leave the list untouched (SkyUI shows all).
        }

        // _filterText is the raw typed query (lowercased by SkyUI's setter)
        const auto query = Utils::TryGetStringMember(*params.thisPtr, "_filterText")
                               .or_else([&params]() {
                                   return Utils::TryGetStringMember(*params.thisPtr, "filterText");
                               })
                               .value_or("");
        if (query.empty()) {
            return;
        }

        const auto nameAttribute = Utils::TryGetStringMember(*params.thisPtr, "nameAttribute").value_or("text"s);

        // Tokenize ONCE
        const auto tokens = Utils::TokenizeQuery(query);
        if (tokens.empty()) {
            return;
        }

        auto& list = params.args[0];
        const std::uint32_t count = list.GetArraySize();
        std::uint32_t kept = 0;

        // Debug performance timing and metrics
        const bool perfDbg = spdlog::default_logger_raw()->level() <= spdlog::level::debug;
        if (perfDbg) {
            logger::debug("SkyUIApplyFilter query='{}' nameAttr='{}' entries={}", query, nameAttribute, count);
        }
        const auto passT0 = perfDbg ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
        if (perfDbg) {
            Matcher::PerfSplitAccum() = {};  // reset the read/match accumulators before the loop
        }

        // stock mode is the bench baseline for SKYUI
        const bool stock = Config::stockMode;

        static std::uint32_t s_lastCount = 0;
        if (!stock && count != s_lastCount) {
            Matcher::InvalidateRowBlobs();
            s_lastCount = count;
        }

        for (std::uint32_t i = 0; i < count; ++i) {
            RE::GFxValue entry;
            if (!list.GetElement(i, &entry)) {
                continue;
            }

            bool match;
            if (stock) {
                match = Matcher::MatchNameOnly(entry, query, nameAttribute);
            } else {
                std::uint32_t rowId = Utils::TryGetUIntMember(entry, "_smartRowId").value_or(0);
                if (rowId == 0) {
                    rowId = Matcher::NextRowId();
                    entry.SetMember("_smartRowId", RE::GFxValue(static_cast<double>(rowId)));
                }
                match = Matcher::MatchSkyUIEntry(entry, tokens, nameAttribute, rowId);
            }

            if (match) {
                if (kept != i) {
                    list.SetElement(kept, entry);
                }
                ++kept;
            }
        }

        // Shrink to the survivors. Matches SkyUI 6.x a_filteredList.length = n.
        list.SetArraySize(kept);

        if (perfDbg) {
            const double passMs = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - passT0).count();
            const auto& split = Matcher::PerfSplitAccum();
            logger::debug("{} pass: {} rows, {} kept, {:.1f}ms (read={:.1f}ms match={:.1f}ms) (query='{}')",
                stock ? "SkyUI-stock" : "SmartSearch",
                count, kept, passMs, split.readUs / 1000.0, split.matchUs / 1000.0, query);
        }
    }

    RE::GPtr<SmartSearchMatchFunction>& SmartSearchMatchHandler() {
        static auto handler = RE::make_gptr<SmartSearchMatchFunction>();
        return handler;
    }

    RE::GPtr<SkyUIApplyFilterFunction>& SkyUIApplyFilterHandler() {
        static auto handler = RE::make_gptr<SkyUIApplyFilterFunction>();
        return handler;
    }

    bool RegisterScaleform(RE::GFxMovieView* view, RE::GFxValue* root) {
        if (!view || !root) {
            return false;
        }

        RE::GFxValue matchFunction;
        view->CreateFunction(&matchFunction, SmartSearchMatchHandler().get());

        const auto registered = root->SetMember("Match", matchFunction);
        logger::debug("SmartSearch Scaleform API registration {}", registered ? "succeeded" : "failed");
        return registered;
    }
}
