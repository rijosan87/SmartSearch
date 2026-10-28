#pragma once
#include "PCH.h"
#include <cstdint>

namespace Hooks {
    [[nodiscard]] bool InstallNameFilterHook(std::string_view menuName);
    void ScheduleHookInstall(std::string_view menuName, std::uint32_t attemptsRemaining);

    class MenuOpenSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static MenuOpenSink* GetSingleton();

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
    };
}
