#pragma once

#include <string>
#include <vector>

#include "RE/Skyrim.h"

namespace IntentModel {
    void AddKeywordDerivedFacets(std::vector<std::string>& fields, const RE::TESForm& form);

    void AddWeightClassFields(std::vector<std::string>& fields, const RE::TESForm& form);

    void AddSpellIntentFields(std::vector<std::string>& fields, const RE::MagicItem* magic);

    void AddEffectIntentFields(std::vector<std::string>& fields, const RE::Effect* effect,
                               RE::MagicSystem::Delivery delivery);
}
