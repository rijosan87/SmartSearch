#pragma once

#include <string>

namespace Config {
    extern bool debugLogging;

    extern bool traceLogging;

    extern bool stockMode;

    extern std::string language;

    void Load();
}
