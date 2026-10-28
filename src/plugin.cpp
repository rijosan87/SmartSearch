#include "PCH.h"
#include <ShlObj.h>
#include <spdlog/sinks/null_sink.h>

#include "Config.h"
#include "Scaleform.h"
#include "Hooks.h"
#include "Matcher.h"

namespace {
    void OnSKSEMessage(SKSE::MessagingInterface::Message* message) {
        if (!message) {
            return;
        }

        switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            if (const auto ui = RE::UI::GetSingleton()) {
                ui->AddEventSink(Hooks::MenuOpenSink::GetSingleton());
                logger::info("Registered Universal Menu open sink");
            } else {
                logger::warn("UI singleton unavailable; InventoryMenu search hook was not registered");
            }
            break;

        // Drop the form-field cache when a different game starts.
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            Matcher::ClearFieldCache();
            logger::debug("Cleared Smart Search field cache on game load");
            break;

        default:
            break;
        }
    }
}

extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    Config::Load();

    std::shared_ptr<spdlog::sinks::sink> sink;
    wchar_t* documentsPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsPath)) && documentsPath) {
        auto logPath = std::filesystem::path(documentsPath);
        CoTaskMemFree(documentsPath);

        logPath /= "My Games/Skyrim Special Edition/SKSE/SmartSearch.log";
        std::filesystem::create_directories(logPath.parent_path());
        sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
    } else {
        if (documentsPath) {
            CoTaskMemFree(documentsPath);
        }
        sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    }

    const auto logLevel = Config::traceLogging ? spdlog::level::trace
                        : Config::debugLogging ? spdlog::level::debug
                                               : spdlog::level::info;
    auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
    log->set_level(logLevel);
    log->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(std::move(log));

    logger::info("Smart Search SKSE loading (LogLevel={})",
        Config::traceLogging ? "trace" : Config::debugLogging ? "debug" : "info");

    if (const auto scaleform = SKSE::GetScaleformInterface()) {
        if (!scaleform->Register(Scaleform::RegisterScaleform, "SmartSearch")) {
            logger::error("Failed to register SmartSearch Scaleform API");
            return false;
        }
    } else {
        logger::error("SKSE Scaleform interface unavailable");
        return false;
    }

    if (const auto messaging = SKSE::GetMessagingInterface()) {
        if (!messaging->RegisterListener(OnSKSEMessage)) {
            logger::error("Failed to register SKSE messaging listener");
            return false;
        }
    } else {
        logger::error("SKSE messaging interface unavailable");
        return false;
    }

    logger::info("Smart Search SKSE loaded");
    return true;
}
