#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Localization {
    // a bad/absent file = empty map.
    void Load(const std::string& lang);

    [[nodiscard]] const std::vector<std::string>* Rewrite(const std::string& loweredWord);
}
