#include "Hooks.h"
#include "Scaleform.h"
#include "Matcher.h"
#include <array>

namespace Hooks {
    constexpr std::array kSupportedMenus = {
        "InventoryMenu"sv,
        "MagicMenu"sv,
        "ContainerMenu"sv,
        "BarterMenu"sv,
        "GiftMenu"sv,
        // Crafting Menu (forge/smelter/workbench/tanning/cooking/enchanting) uses the same
        // skyui.filter.NameFilter and a searchWidget
        // NOTE: the menu name has a space "Crafting Menu".
        "Crafting Menu"sv
    };

    bool InstallNameFilterHook(std::string_view menuName) {
        const auto ui = RE::UI::GetSingleton();
        if (!ui) {
            return false;
        }

        const auto view = ui->GetMovieView(menuName);
        if (!view) {
            return false;
        }

        static constexpr std::array prototypePaths{
            "skyui.filter.NameFilter.prototype",
            "_global.skyui.filter.NameFilter.prototype",
        };

        for (const auto* path : prototypePaths) {
            RE::GFxValue prototype;
            if (!view->GetVariable(&prototype, path) || (!prototype.IsObject() && !prototype.IsDisplayObject())) {
                continue;
            }

            RE::GFxValue installed;
            if (prototype.GetMember("_smartSearchInstalled", &installed) && installed.IsBool() && installed.GetBool()) {
                return true;
            }

            // SkyUI 6.x inlined its match into applyFilter compared to 5.2
            RE::GFxValue applyFilterFunction;
            view->CreateFunction(&applyFilterFunction, Scaleform::SkyUIApplyFilterHandler().get());

            if (!prototype.SetMember("applyFilter", applyFilterFunction)) {
                logger::warn("Failed to replace SkyUI NameFilter.applyFilter at {} for {}", path, menuName);
                continue;
            }

            if (!prototype.SetMember("_smartSearchInstalled", RE::GFxValue(true))) {
                logger::debug("Failed to mark SkyUI NameFilter hook as installed");
            }
            logger::info("Installed SkyUI {} search hook at {}", menuName, path);
            return true;
        }

        logger::debug("SkyUI NameFilter prototype was not ready yet for {}", menuName);
        return false;
    }

    void ScheduleHookInstall(std::string_view menuName, std::uint32_t attemptsRemaining) {
        const auto taskInterface = SKSE::GetTaskInterface();
        if (!taskInterface) {
            [[maybe_unused]] const auto installed = InstallNameFilterHook(menuName);
            return;
        }

        // Copy menuName into a std::string to capture it safely by value
        taskInterface->AddUITask([menuName = std::string(menuName), attemptsRemaining]() {
            if (InstallNameFilterHook(menuName) || attemptsRemaining == 0) {
                return;
            }

            ScheduleHookInstall(menuName, attemptsRemaining - 1);
        });
    }

    MenuOpenSink* MenuOpenSink::GetSingleton() {
        static MenuOpenSink singleton;
        return std::addressof(singleton);
    }

    RE::BSEventNotifyControl MenuOpenSink::ProcessEvent(const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
        if (event && event->opening) {
            for (const auto& supportedMenu : kSupportedMenus) {
                if (event->menuName == supportedMenu) {
                    // Drop every cached row blob and free the map, a new menu builds fresh entry objects,
                    // and their live fields (status/temper/etc.) 
                    Matcher::ResetRowBlobs();
                    ScheduleHookInstall(event->menuName.c_str(), 120);
                    break;
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
