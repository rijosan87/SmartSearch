#include "PCH.h"
#include "Localization.h"
#include "Utils.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

    std::unordered_map<std::string, std::vector<std::string>> g_aliasToKeywords;

    bool IsValidUTF8(const std::string& str) {
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.c_str());
        const size_t len = str.length();

        for (size_t i = 0; i < len;) {
            if (bytes[i] <= 0x7F) {
                i++;  // ASCII
            } else if ((bytes[i] & 0xE0) == 0xC0) {
                if (i + 1 >= len || (bytes[i + 1] & 0xC0) != 0x80) return false;
                i += 2;
            } else if ((bytes[i] & 0xF0) == 0xE0) {
                if (i + 2 >= len || (bytes[i + 1] & 0xC0) != 0x80 || (bytes[i + 2] & 0xC0) != 0x80) return false;
                i += 3;
            } else if ((bytes[i] & 0xF8) == 0xF0) {
                if (i + 3 >= len || (bytes[i + 1] & 0xC0) != 0x80 || (bytes[i + 2] & 0xC0) != 0x80 ||
                    (bytes[i + 3] & 0xC0) != 0x80)
                    return false;
                i += 4;
            } else {
                return false;  // invalid lead byte
            }
        }
        return true;
    }

    void Trim(std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        const auto last = s.find_last_not_of(" \t\r\n");
        if (last == std::string::npos) {
            s.clear();
        } else {
            s.erase(last + 1);
        }
    }

    // Split the comma-separated typed-word list and record each alias -> keyword.
    void Ingest(const std::string& keyword, const std::string& csv) {
        std::stringstream ss(csv);
        std::string alias;
        while (std::getline(ss, alias, ',')) {
            Trim(alias);
            if (alias.empty()) {
                continue;
            }
            g_aliasToKeywords[Utils::ToLowerAscii(alias)].push_back(keyword);
        }
    }
}

void Localization::Load(const std::string& lang) {
    g_aliasToKeywords.clear();

    const auto path = std::filesystem::path("Data/SKSE/Plugins/SmartSearch") /
                      ("SmartSearch_" + lang + ".ini");

    if (!std::filesystem::exists(path)) {
        // No file is fine
        logger::info("No localization file '{}', facet keywords are English-only", path.string());
        return;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        logger::warn("Could not open localization file '{}', facet keywords are English-only", path.string());
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Skip UTF-8 BOM if present.
    if (content.size() >= 3 && static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB && static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    if (!IsValidUTF8(content)) {
        logger::error("Localization file '{}' is not valid UTF-8 (save it as UTF-8). English-only fallback.",
                      path.string());
        g_aliasToKeywords.clear();
        return;
    }

    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        Trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[') {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string keyword = line.substr(0, eq);
        std::string csv = line.substr(eq + 1);
        Trim(keyword);
        if (keyword.empty()) {
            continue;
        }
        // Keyword is the internal English word. Lowercase it anyway
        // so a translator's stray capital still maps.
        Ingest(Utils::ToLowerAscii(keyword), csv);
    }

    logger::info("Loaded localization '{}': {} typed-word aliases", lang, g_aliasToKeywords.size());
}

const std::vector<std::string>* Localization::Rewrite(const std::string& loweredWord) {
    const auto it = g_aliasToKeywords.find(loweredWord);
    return it == g_aliasToKeywords.end() ? nullptr : &it->second;
}
