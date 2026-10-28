#include "IntentModel.h"

#include <cstring>

#ifdef INTENTMODEL_DIAG
#include <spdlog/spdlog.h>
namespace logger = SKSE::log;
#endif

namespace IntentModel {

namespace {

    // Material keyword editor ID -> clean name, do nullptr if not material keyword.
    const char* MaterialNameForKeyword(const char* editorID) {
        if (!editorID) return nullptr;

        struct Entry { const char* editorID; const char* material; };
        static constexpr Entry kTable[] = {
            { "ArmorMaterialIron",                  "iron"       },
            { "WeapMaterialIron",                   "iron"       },
            { "ArmorMaterialSteel",                 "steel"      },
            { "WeapMaterialSteel",                  "steel"      },
            { "ArmorMaterialSteelPlate",            "steelplate" },
            { "ArmorMaterialLeather",               "leather"    },
            { "ArmorMaterialHide",                  "hide"       },
            { "ArmorMaterialStudded",               "studded"    },
            { "ArmorMaterialScaled",                "scaled"     },
            { "ArmorMaterialImperialLight",         "imperial"   },
            { "ArmorMaterialImperialStudded",       "imperial"   },
            { "ArmorMaterialImperialHeavy",         "imperial"   },
            { "WeapMaterialImperial",               "imperial"   },
            { "ArmorMaterialDwarven",               "dwarven"    },
            { "WeapMaterialDwarven",                "dwarven"    },
            { "ArmorMaterialOrcish",                "orcish"     },
            { "WeapMaterialOrcish",                 "orcish"     },
            { "ArmorMaterialElven",                 "elven"      },
            { "WeapMaterialElven",                  "elven"      },
            { "ArmorMaterialGlass",                 "glass"      },
            { "WeapMaterialGlass",                  "glass"      },
            { "ArmorMaterialEbony",                 "ebony"      },
            { "WeapMaterialEbony",                  "ebony"      },
            { "ArmorMaterialDragonplate",           "dragonplate"},
            { "ArmorMaterialDragonscale",           "dragonscale"},
            { "DLC1WeapMaterialDragonbone",         "dragonbone" },
            { "ArmorMaterialDaedric",               "daedric"    },
            { "WeapMaterialDaedric",                "daedric"    },
            { "DLC2ArmorMaterialNordicHeavy",       "nordic"     },
            { "DLC2WeaponMaterialNordic",           "nordic"     },
            { "DLC2ArmorMaterialStalhrimHeavy",     "stalhrim"   },
            { "DLC2ArmorMaterialStalhrimLight",     "stalhrim"   },
            { "DLC2WeaponMaterialStalhrim",         "stalhrim"   },
            { "WeapMaterialNordHero",               "nordhero"   },
            { "ccBGSSSE025_ArmorMaterialAmber",     "amber"      },
            { "ccBGSSSE025_WeapMaterialAmber",      "amber"      },
            { "ccBGSSSE025_WeapMaterialDark",       "dark"       },
            { "ccBGSSSE025_WeapMaterialGolden",     "golden"     },
            { "WeapMaterialSilver",                 "silver"     },
            { "WeapMaterialWood",                   "wood"       },
            { "WeapMaterialFalmer",                 "falmer"     },
            { "ArmorMaterialForsworn",              "forsworn"   },
            { "ArmorMaterialBlades",                "blades"     },
        };

        for (const auto& e : kTable) {
            if (std::strcmp(editorID, e.editorID) == 0) {
                return e.material;
            }
        }
        return nullptr;
    }

    // VendorItem keyword editor ID -> category word. 
    void AddCategoryWordsForKeyword(std::vector<std::string>& fields, const char* editorID) {
        if (!editorID) return;

        struct Entry { const char* editorID; const char* word; const char* word2; };
        static constexpr Entry kTable[] = {
            { "VendorItemWeapon",     "weapon",      nullptr      },
            { "VendorItemStaff",      "staff",       nullptr      },
            { "VendorItemArrow",      "arrow",       "ammo"       },
            { "VendorItemArmor",      "armor",       nullptr      },
            { "VendorItemClothing",   "clothing",    nullptr      },
            { "VendorItemJewelry",    "jewelry",     nullptr      },
            { "VendorItemFood",       "food",        nullptr      },
            { "VendorItemFoodRaw",    "food",        nullptr      },
            { "VendorItemPotion",     "potion",      nullptr      },
            { "VendorItemPoison",     "poison",      nullptr      },
            { "VendorItemIngredient", "ingredient",  nullptr      },
            { "VendorItemBook",       "book",        nullptr      },
            { "VendorItemSpellTome",  "tome",        "spell tome" },
            { "VendorItemRecipe",     "recipe",      nullptr      },
            { "VendorItemScroll",     "scroll",      nullptr      },
            { "VendorItemSoulGem",    "soul gem",    nullptr      },
            { "VendorItemOreIngot",   "ore",         "ingot"      },
            { "VendorItemGem",        "gem",         nullptr      },
            { "VendorItemAnimalHide", "hide",        nullptr      },
            { "VendorItemAnimalPart", "animal part", nullptr      },
            { "VendorItemClutter",    "clutter",     "misc"       },
            { "VendorItemTool",       "tool",        nullptr      },
            { "VendorItemFirewood",   "firewood",    "wood"       },
            { "VendorItemKey",        "key",         nullptr      },
        };

        for (const auto& e : kTable) {
            if (std::strcmp(editorID, e.editorID) == 0) {
                fields.emplace_back(std::string("category:") + e.word);
                if (e.word2) fields.emplace_back(std::string("category:") + e.word2);
                return;
            }
        }
    }

    // WeapType keyword editor ID -> weapon-type word.
    const char* WeaponTypeWordForKeyword(const char* editorID) {
        if (!editorID) return nullptr;

        struct Entry { const char* editorID; const char* word; };
        static constexpr Entry kTable[] = {
            { "WeapTypeSword",      "sword"      },
            { "WeapTypeGreatsword", "greatsword" },
            { "WeapTypeDagger",     "dagger"     },
            { "WeapTypeWarAxe",     "war axe"    },
            { "WeapTypeBattleaxe",  "battleaxe"  },
            { "WeapTypeMace",       "mace"       },
            { "WeapTypeWarhammer",  "warhammer"  },
            { "WeapTypeBow",        "bow"        },
            { "WeapTypeStaff",      "staff"      },
        };

        for (const auto& e : kTable) {
            if (std::strcmp(editorID, e.editorID) == 0) {
                return e.word;
            }
        }
        return nullptr;
    }

    // Armor equip-slot keyword editor ID -> slot word.
    void AddEquipSlotWordsForKeyword(std::vector<std::string>& fields, const char* editorID) {
        if (!editorID) return;

        struct Entry { const char* editorID; const char* a; const char* b; const char* c; };
        static constexpr Entry kTable[] = {
            { "ArmorCuirass",   "body",      "chest", "cuirass" },
            { "ArmorHelmet",    "helmet",    "head",  nullptr   },
            { "ArmorGauntlets", "gauntlets", "hands", nullptr   },
            { "ArmorBoots",     "boots",     "feet",  nullptr   },
            { "ArmorShield",    "shield",    nullptr, nullptr   },
        };

        for (const auto& e : kTable) {
            if (std::strcmp(editorID, e.editorID) == 0) {
                fields.emplace_back(std::string("slot:") + e.a);
                if (e.b) fields.emplace_back(std::string("slot:") + e.b);
                if (e.c) fields.emplace_back(std::string("slot:") + e.c);
                return;
            }
        }
    }

    enum class Intent {
        Damage, Heal, Buff, Debuff, Defense, ArmorBuff, Resist,
        CrowdControl, Summon, Bound, Stealth, Escape, Utility,
        Ranged, Self, Channel, Instant
    };

    void PushIntent(std::vector<std::string>& fields, Intent intent) {
        auto add = [&](std::initializer_list<const char*> words) {
            for (const char* w : words) fields.emplace_back(std::string("intent:") + w);
        };
        switch (intent) {
        case Intent::Damage:       add({ "damage", "offense", "attack" }); break;
        case Intent::Heal:         add({ "heal", "healing", "restore", "recovery" }); break;
        case Intent::Buff:         add({ "buff", "fortify", "boost" }); break;
        case Intent::Debuff:       add({ "debuff", "weaken" }); break;
        case Intent::Defense:      add({ "defense", "protection" }); break;
        case Intent::ArmorBuff:    add({ "armor buff" }); break;
        case Intent::Resist:       add({ "resist", "resistance" }); break;
        case Intent::CrowdControl: add({ "control", "crowd control", "disable" }); break;
        case Intent::Summon:       add({ "summon", "ally", "conjure" }); break;
        case Intent::Bound:        add({ "bound", "bound weapon" }); break;
        case Intent::Stealth:      add({ "stealth", "vanish" }); break;
        case Intent::Escape:       add({ "escape" }); break;
        case Intent::Utility:      add({ "utility" }); break;
        case Intent::Ranged:       add({ "ranged" }); break;
        case Intent::Self:         add({ "self" }); break;
        case Intent::Channel:      add({ "channel" }); break;
        case Intent::Instant:      add({ "instant" }); break;
        }
    }

}  // namespace

void AddKeywordDerivedFacets(std::vector<std::string>& fields, const RE::TESForm& form) {
    const auto* keywordForm = skyrim_cast<const RE::BGSKeywordForm*>(&form);
    if (!keywordForm) return;

    for (const auto* keyword : keywordForm->GetKeywords()) {
        if (!keyword) continue;
        const char* id = keyword->GetFormEditorID();
        if (const auto* mat = MaterialNameForKeyword(id)) {
            fields.emplace_back(std::string("material:") + mat);
        }
        AddCategoryWordsForKeyword(fields, id);
        if (const auto* wt = WeaponTypeWordForKeyword(id)) {
            fields.emplace_back(std::string("weapontype:") + wt);
        }
        AddEquipSlotWordsForKeyword(fields, id);
    }
}

// Armor weight class from (kHeavyArmor/kLightArmor/kClothing)
// not the ArmorHeavy/Light/Clothing keywords
void AddWeightClassFields(std::vector<std::string>& fields, const RE::TESForm& form) {
    if (form.GetFormType() != RE::FormType::Armor) return;

    const auto* biped = skyrim_cast<const RE::BGSBipedObjectForm*>(&form);
    if (!biped) return;

    switch (biped->GetArmorType()) {
    case RE::BGSBipedObjectForm::ArmorType::kHeavyArmor:
        fields.emplace_back("weight:heavy");
        break;
    case RE::BGSBipedObjectForm::ArmorType::kLightArmor:
        fields.emplace_back("weight:light");
        break;
    case RE::BGSBipedObjectForm::ArmorType::kClothing:
        fields.emplace_back("weight:clothing");
        break;
    }
}

// Spell-level intent: delivery + casting type. Called once per magic form.
void AddSpellIntentFields(std::vector<std::string>& fields, const RE::MagicItem* magic) {
    if (!magic) return;
    using D = RE::MagicSystem::Delivery;
    using C = RE::MagicSystem::CastingType;
    switch (magic->GetDelivery()) {
    case D::kSelf:                           PushIntent(fields, Intent::Self); break;
    case D::kAimed: case D::kTargetLocation: PushIntent(fields, Intent::Ranged); break;
    default: break;
    }
    switch (magic->GetCastingType()) {
    case C::kConcentration: PushIntent(fields, Intent::Channel); break;
    case C::kFireAndForget: PushIntent(fields, Intent::Instant); break;
    default: break;
    }
}

static const char* ElementWord(RE::ActorValue av) {
    using AV = RE::ActorValue;
    switch (av) {
    case AV::kResistFire:   return "fire";
    case AV::kResistFrost:  return "frost";
    case AV::kResistShock:  return "shock";
    case AV::kPoisonResist: return "poison";
    default:                return nullptr;
    }
}

// Per-effect intent from archetype + AV + hostile flag
void AddEffectIntentFields(std::vector<std::string>& fields, const RE::Effect* effect,
                           RE::MagicSystem::Delivery delivery) {
    if (!effect || !effect->baseEffect) return;
    const auto* be = effect->baseEffect;
    using A    = RE::EffectArchetypes::ArchetypeID;
    using AV   = RE::ActorValue;
    using Flag = RE::EffectSetting::EffectSettingData::Flag;

    const bool hostile    = effect->IsHostile();
    // Detrimental, not Hostile
    const bool detrimental = be->IsDetrimental();
    const bool harmful     = hostile || detrimental;
    const bool beneficial  = !harmful;
    const auto arch       = be->GetArchetype();
    const AV   pAV        = be->data.primaryAV;
    const AV   rAV        = be->data.resistVariable;
    const bool recover    = be->data.flags.any(Flag::kRecover);

    auto isPool = [](AV av) {
        return av == AV::kHealth || av == AV::kMagicka || av == AV::kStamina;
    };

    auto isDamageElement = [](AV av) {
        return av == AV::kResistFire || av == AV::kResistFrost ||
               av == AV::kResistShock || av == AV::kPoisonResist;
    };

    auto isResistAV = [](AV av) {
        return av == AV::kResistFire || av == AV::kResistFrost || av == AV::kResistShock ||
               av == AV::kResistMagic || av == AV::kPoisonResist || av == AV::kResistDisease;
    };

    bool buffFired = false;

    // harmful (hostile OR detrimental)
    if (harmful && arch == A::kValueModifier && isPool(pAV)) PushIntent(fields, Intent::Damage);
    if (hostile && isDamageElement(rAV)) {
        PushIntent(fields, Intent::Damage);
        if (const char* el = ElementWord(rAV)) fields.emplace_back(std::string(el) + " damage");
    }
    if (arch == A::kAbsorb && isPool(pAV))                   PushIntent(fields, Intent::Damage);

    if (detrimental && !hostile && isResistAV(pAV)) PushIntent(fields, Intent::Debuff);

    if (beneficial && arch == A::kValueModifier && isPool(pAV)) {

        const bool isHeal = !recover;
#ifdef INTENTMODEL_DIAG

        if (spdlog::default_logger_raw()->level() <= spdlog::level::trace) {
            logger::trace("INTENTPOLARITY effect '{}' av={} recover={} -> tagged {}",
                be->GetFullName(), static_cast<int>(pAV), recover, isHeal ? "HEAL" : "BUFF/fortify");
        }
#endif
        if (isHeal) PushIntent(fields, Intent::Heal);
        else { PushIntent(fields, Intent::Buff); buffFired = true; }
    }

    if (beneficial && pAV == AV::kDamageResist) {
        PushIntent(fields, Intent::Buff); PushIntent(fields, Intent::ArmorBuff);
        PushIntent(fields, Intent::Defense); buffFired = true;
    }
    if (harmful && pAV == AV::kDamageResist) PushIntent(fields, Intent::Debuff);

    if (beneficial && isResistAV(pAV)) {
        PushIntent(fields, Intent::Buff); PushIntent(fields, Intent::Resist);
        PushIntent(fields, Intent::Defense); buffFired = true;

        if (const char* el = ElementWord(pAV)) fields.emplace_back(std::string(el) + " resist");
    }

    switch (arch) {
    case A::kFrenzy:
        // Frenzy = rage/fury 
        PushIntent(fields, Intent::CrowdControl);
        fields.emplace_back("intent:rage");
        fields.emplace_back("intent:fury");
        break;
    case A::kParalysis: case A::kCalm: case A::kDemoralize:
    case A::kDisarm:    case A::kStagger:
        PushIntent(fields, Intent::CrowdControl); break;
    case A::kSummonCreature: case A::kReanimate: case A::kCommandSummoned:
        PushIntent(fields, Intent::Summon); break;
    case A::kBoundWeapon:
        PushIntent(fields, Intent::Bound); PushIntent(fields, Intent::Summon); break;
    case A::kInvisibility:
        PushIntent(fields, Intent::Stealth); PushIntent(fields, Intent::Escape); break;
    case A::kEtherealize:
        PushIntent(fields, Intent::Escape); PushIntent(fields, Intent::Defense); break;
    case A::kDetectLife: case A::kTelekinesis: case A::kLight: case A::kOpen:
        PushIntent(fields, Intent::Utility); break;
    default: break;
    }
    if (pAV == AV::kWardPower) PushIntent(fields, Intent::Defense);
    if (pAV == AV::kWaterBreathing || pAV == AV::kWaterWalking)
        PushIntent(fields, Intent::Utility);

    if (buffFired && delivery == RE::MagicSystem::Delivery::kSelf) {
        fields.emplace_back("intent:self buff");
    }
}

}  // namespace IntentModel
