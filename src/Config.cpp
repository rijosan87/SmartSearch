#include "PCH.h"
#include "Config.h"
#include "Localization.h"
#include "Utils.h"
#include <Windows.h>

namespace Config {
    bool debugLogging = false;
    bool traceLogging = false;
    bool stockMode = false;
    std::string language = "en";

    void Load() {
        constexpr auto iniPath = "Data/SKSE/Plugins/SmartSearch.ini";
        char buf[32] = "info";
        [[maybe_unused]] const auto logResult =
            ::GetPrivateProfileStringA("Debug", "LogLevel", "info", buf, 32, iniPath);
        const auto level = Utils::ToLowerAscii(buf);
        traceLogging = level == "trace";
        debugLogging = traceLogging || level == "debug";  // trace implies debug

        char modeBuf[32] = "smart";
        [[maybe_unused]] const auto modeResult =
            ::GetPrivateProfileStringA("Debug", "Mode", "smart", modeBuf, 32, iniPath);
        stockMode = Utils::ToLowerAscii(modeBuf) == "stock";

        char langBuf[32] = "en";
        [[maybe_unused]] const auto langResult =
            ::GetPrivateProfileStringA("Localization", "Language", "en", langBuf, 32, iniPath);
        language = Utils::ToLowerAscii(langBuf);

        // Load the active language's typed-word aliases
        Localization::Load(language);
    }
}
