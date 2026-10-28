#pragma once
#include "PCH.h"

namespace Scaleform {
    class SmartSearchMatchFunction final : public RE::GFxFunctionHandler {
    public:
        void Call(Params& params) override;
    };

    // Replaces skyui.filter.NameFilter.prototype.applyFilter. Hook applyFilter
    class SkyUIApplyFilterFunction final : public RE::GFxFunctionHandler {
    public:
        void Call(Params& params) override;
    };

    [[nodiscard]] RE::GPtr<SmartSearchMatchFunction>& SmartSearchMatchHandler();
    [[nodiscard]] RE::GPtr<SkyUIApplyFilterFunction>& SkyUIApplyFilterHandler();

    bool RegisterScaleform(RE::GFxMovieView* view, RE::GFxValue* root);
}
