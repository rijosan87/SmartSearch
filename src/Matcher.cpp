#include "Matcher.h"
#include "Utils.h"
#include "IntentModel/IntentModel.h"
#include <algorithm>
#include <chrono>
#include <unordered_map>

namespace Matcher {
    void AddField(std::vector<std::string>& fields, const char* text) {
        if (text && text[0] != '\0') {
            fields.emplace_back(text);
        }
    }

    void AddField(std::vector<std::string>& fields, std::string_view text) {
        if (!text.empty()) {
            fields.emplace_back(std::string(text));
        }
    }

    // Flora/Tree/Furniture aren't in vanilla inventory menus, but housing/crafting
    // mods (Hearthfire Extended, Campfire) show them in custom build menus.
    void AddFormTypeAliasFields(std::vector<std::string>& fields, RE::FormType formType) {
        switch (formType) {
        case RE::FormType::Weapon:
            fields.emplace_back("weapon");
            break;
        case RE::FormType::Armor:
            fields.emplace_back("armor");
            break;
        case RE::FormType::Book:
            fields.emplace_back("book");
            break;
        case RE::FormType::Ingredient:
            fields.emplace_back("ingredient");
            break;
        case RE::FormType::AlchemyItem:
            fields.emplace_back("alchemy");
            break;
        case RE::FormType::Ammo:
            fields.emplace_back("ammo");
            break;
        case RE::FormType::KeyMaster:
            fields.emplace_back("key");
            break;
        case RE::FormType::SoulGem:
            fields.emplace_back("soul gem");
            break;
        case RE::FormType::Scroll:
            fields.emplace_back("scroll");
            break;
        case RE::FormType::Misc:
            fields.emplace_back("misc");
            break;
        case RE::FormType::Light:
            fields.emplace_back("torch");
            fields.emplace_back("light");
            break;
        case RE::FormType::Spell:
            fields.emplace_back("spell");
            break;
        case RE::FormType::Shout:
            fields.emplace_back("shout");
            fields.emplace_back("shouts");  // plural is useful
            break;
        case RE::FormType::Enchantment:
            fields.emplace_back("enchantment");
            break;
        case RE::FormType::Note:
            fields.emplace_back("note");
            break;
        case RE::FormType::MagicEffect:
            fields.emplace_back("effect");
            break;
        case RE::FormType::ConstructibleObject:
            fields.emplace_back("recipe");
            break;
        case RE::FormType::Flora:
        case RE::FormType::Tree:
            fields.emplace_back("plant");
            fields.emplace_back("flora");
            fields.emplace_back("tree");
            break;
        case RE::FormType::Apparatus:
            fields.emplace_back("apparatus");
            break;
        case RE::FormType::Activator:
            fields.emplace_back("activator");
            break;
        case RE::FormType::Furniture:
            fields.emplace_back("furniture");
            break;
        default:
            break;
        }
    }

    // PartySheet EffectCategories
    static const char* ActorValueName(RE::ActorValue av) {
        switch (av) {
        case RE::ActorValue::kHealth:               return "Health";
        case RE::ActorValue::kMagicka:              return "Magicka";
        case RE::ActorValue::kStamina:              return "Stamina";
        case RE::ActorValue::kHealRate:             return "Health Regen";
        case RE::ActorValue::kMagickaRate:          return "Magicka Regen";
        case RE::ActorValue::kStaminaRate:          return "Stamina Regen";
        case RE::ActorValue::kHealRateMult:         return "Health Regen Mult";
        case RE::ActorValue::kMagickaRateMult:      return "Magicka Regen Mult";
        case RE::ActorValue::kStaminaRateMult:      return "Stamina Regen Mult";
        case RE::ActorValue::kResistFire:           return "Fire Resist";
        case RE::ActorValue::kResistFrost:          return "Frost Resist";
        case RE::ActorValue::kResistShock:          return "Shock Resist";
        case RE::ActorValue::kResistMagic:          return "Magic Resist";
        case RE::ActorValue::kPoisonResist:         return "Poison Resist";
        case RE::ActorValue::kResistDisease:        return "Disease Resist";
        case RE::ActorValue::kDamageResist:         return "Armor";
        case RE::ActorValue::kSpeedMult:            return "Speed";
        case RE::ActorValue::kCarryWeight:          return "Carry Weight";
        case RE::ActorValue::kOneHanded:            return "One-Handed";
        case RE::ActorValue::kTwoHanded:            return "Two-Handed";
        case RE::ActorValue::kArchery:              return "Archery";
        case RE::ActorValue::kBlock:                return "Block";
        case RE::ActorValue::kSmithing:             return "Smithing";
        case RE::ActorValue::kHeavyArmor:           return "Heavy Armor";
        case RE::ActorValue::kLightArmor:           return "Light Armor";
        case RE::ActorValue::kPickpocket:           return "Pickpocket";
        case RE::ActorValue::kLockpicking:          return "Lockpicking";
        case RE::ActorValue::kSneak:                return "Sneak";
        case RE::ActorValue::kAlchemy:              return "Alchemy";
        case RE::ActorValue::kSpeech:               return "Speech";
        case RE::ActorValue::kAlteration:           return "Alteration";
        case RE::ActorValue::kConjuration:          return "Conjuration";
        case RE::ActorValue::kDestruction:          return "Destruction";
        case RE::ActorValue::kIllusion:             return "Illusion";
        case RE::ActorValue::kRestoration:          return "Restoration";
        case RE::ActorValue::kEnchanting:           return "Enchanting";
        case RE::ActorValue::kAttackDamageMult:     return "Attack Damage";
        case RE::ActorValue::kMeleeDamage:          return "Melee Damage";
        case RE::ActorValue::kUnarmedDamage:        return "Unarmed Damage";
        case RE::ActorValue::kCriticalChance:       return "Critical Chance";
        case RE::ActorValue::kAbsorbChance:         return "Absorb";
        case RE::ActorValue::kReflectDamage:        return "Reflect";
        case RE::ActorValue::kWardPower:            return "Ward";
        case RE::ActorValue::kWaterBreathing:       return "Water Breathing";
        case RE::ActorValue::kWaterWalking:         return "Water Walking";
        case RE::ActorValue::kParalysis:            return "Paralysis";
        case RE::ActorValue::kInvisibility:         return "Invisibility";
        case RE::ActorValue::kNightEye:             return "Night Eye";
        default:                                    return nullptr;
        }
    }

    // Archetype to clean name
    static const char* ArchetypeName(RE::EffectArchetypes::ArchetypeID archetype) {
        using A = RE::EffectArchetypes::ArchetypeID;
        switch (archetype) {
        case A::kValueModifier:       return "Value Modifier";
        case A::kScript:              return "Script";
        case A::kDispel:              return "Dispel";
        case A::kCureDisease:         return "Cure Disease";
        case A::kAbsorb:              return "Absorb";
        case A::kDualValueModifier:   return "Dual Value Modifier";
        case A::kCalm:                return "Calm";
        case A::kDemoralize:          return "Demoralize";
        case A::kFrenzy:              return "Frenzy";
        case A::kDisarm:              return "Disarm";
        case A::kCommandSummoned:     return "Command Summoned";
        case A::kInvisibility:        return "Invisibility";
        case A::kLight:               return "Light";
        case A::kDarkness:            return "Darkness";
        case A::kNightEye:            return "Night Eye";
        case A::kLock:                return "Lock";
        case A::kOpen:                return "Open";
        case A::kBoundWeapon:         return "Bound Weapon";
        case A::kSummonCreature:      return "Summon Creature";
        case A::kDetectLife:          return "Detect Life";
        case A::kTelekinesis:         return "Telekinesis";
        case A::kParalysis:           return "Paralysis";
        case A::kReanimate:           return "Reanimate";
        case A::kSoulTrap:            return "Soul Trap";
        case A::kTurnUndead:          return "Turn Undead";
        case A::kGuide:               return "Guide";
        case A::kWerewolfFeed:        return "Werewolf Feed";
        case A::kCureParalysis:       return "Cure Paralysis";
        case A::kCureAddiction:       return "Cure Addiction";
        case A::kCurePoison:          return "Cure Poison";
        case A::kConcussion:          return "Concussion";
        case A::kValueAndParts:       return "Value and Parts";
        case A::kAccumulateMagnitude: return "Accumulate Magnitude";
        case A::kStagger:             return "Stagger";
        case A::kPeakValueModifier:   return "Peak Value Modifier";
        case A::kCloak:               return "Cloak";
        case A::kWerewolf:            return "Werewolf";
        case A::kSlowTime:            return "Slow Time";
        case A::kRally:               return "Rally";
        case A::kEnhanceWeapon:       return "Enhance Weapon";
        case A::kSpawnHazard:         return "Spawn Hazard";
        case A::kEtherealize:         return "Etherealize";
        case A::kBanish:              return "Banish";
        case A::kDisguise:            return "Disguise";
        case A::kGrabActor:           return "Grab Actor";
        case A::kVampireLord:         return "Vampire Lord";
        default:                      return nullptr;
        }
    }

    // Spell type -> searchable words (power, ability, etc.)
    // Source PartySheet
    static void AddSpellTypeFields(std::vector<std::string>& fields, RE::MagicSystem::SpellType type) {
        using ST = RE::MagicSystem::SpellType;
        switch (type) {
        case ST::kSpell:
        case ST::kLeveledSpell:
            fields.emplace_back("spell");
            break;
        case ST::kPower:
            fields.emplace_back("power");
            break;
        case ST::kLesserPower:
            fields.emplace_back("power");
            fields.emplace_back("lesser power");
            break;
        case ST::kAbility:
            fields.emplace_back("ability");
            break;
        case ST::kVoicePower:
            fields.emplace_back("shout");
            fields.emplace_back("shouts");
            fields.emplace_back("voice");
            break;
        case ST::kScroll:
            fields.emplace_back("scroll");
            break;
        case ST::kStaffEnchantment:
            fields.emplace_back("staff");
            break;
        case ST::kDisease:
            fields.emplace_back("disease");
            break;
        case ST::kPoison:
            fields.emplace_back("poison");
            break;
        default:
            break;
        }
    }

    // Map vanilla keyword editor ID to a clean searchable word, or nullptr to drop it.
    static const char* KeywordSearchWord(const char* editorID) {
        if (!editorID) return nullptr;

        struct Entry { const char* editorID; const char* word; };
        static constexpr Entry kTable[] = {
            // Elemental damage. e:fire / frost / shock
            { "MagicDamageFire",        "fire"        },
            { "MagicDamageFrost",       "frost"       },
            { "MagicDamageShock",       "shock"       },
            { "MagicDamagePoison",      "poison"      },
            // Influence / illusion family.
            { "MagicInfluence",         "influence"   },
            { "MagicInfluenceFear",     "fear"        },
            { "MagicInfluenceCharm",    "charm"       },
            { "MagicInfluenceFrenzy",   "frenzy"      },
            // Other vanilla concept keywords a player might type.
            { "MagicParalysis",         "paralysis"   },
            { "MagicInvisibility",      "invisibility"},
            { "MagicNightEye",          "night eye"   },
            { "MagicTelekinesis",       "telekinesis" },
            { "MagicCloak",             "cloak"       },
            { "MagicWard",              "ward"        },
            { "MagicRune",              "rune"        },
            { "MagicTurnUndead",        "turn undead" },
            { "MagicVampireDrain",      "drain"       },
            { "MagicRestoreHealth",     "restore health" },
            { "MagicSlow",              "slow"        },
            { "MagicArmorSpell",        "armor"       },
        };

        for (const auto& e : kTable) {
            if (std::strcmp(editorID, e.editorID) == 0) {
                return e.word;
            }
        }
        return nullptr;
    }

    // Raw editor IDs are never indexed
    void AddKeywordSearchFields(std::vector<std::string>& fields, const RE::BGSKeyword* keyword) {
        if (!keyword) {
            return;
        }
        if (const auto* word = KeywordSearchWord(keyword->GetFormEditorID())) {
            fields.emplace_back(word);
        }
        AddField(fields, keyword->GetName());
    }

    // True for the five magic-school ActorValues. 
    static bool IsMagicSchoolAV(RE::ActorValue av) {
        switch (av) {
        case RE::ActorValue::kAlteration:
        case RE::ActorValue::kConjuration:
        case RE::ActorValue::kDestruction:
        case RE::ActorValue::kIllusion:
        case RE::ActorValue::kRestoration:
            return true;
        default:
            return false;
        }
    }

    static const char* ElementResistWord(RE::ActorValue av) {
        switch (av) {
        case RE::ActorValue::kResistFire:   return "fire";
        case RE::ActorValue::kResistFrost:  return "frost";
        case RE::ActorValue::kResistShock:  return "shock";
        case RE::ActorValue::kPoisonResist: return "poison";
        default:                            return nullptr;
        }
    }

    void AddBaseEffectFields(std::vector<std::string>& fields, const RE::EffectSetting* baseEffect) {
        if (!baseEffect) {
            return;
        }

        AddField(fields, baseEffect->GetFullName());
        AddField(fields, baseEffect->GetFormEditorID());

        auto pushAV = [&](RE::ActorValue av) {
            if (IsMagicSchoolAV(av)) {
                return;
            }
            if (const auto* avName = ActorValueName(av)) {
                fields.emplace_back("av:" + Utils::ToLowerAscii(avName));
            }
        };
        pushAV(baseEffect->data.primaryAV);
        pushAV(baseEffect->data.secondaryAV);

        if (const char* el = ElementResistWord(baseEffect->data.resistVariable)) {
            fields.emplace_back(std::string("av:") + el);
        } else {
            pushAV(baseEffect->data.resistVariable);
        }
        if (const auto* archName = ArchetypeName(baseEffect->GetArchetype())) {
            fields.emplace_back(archName);
        }

        // Effect keywords live on the effect, not the form.
        for (const auto* keyword : baseEffect->GetKeywords()) {
            AddKeywordSearchFields(fields, keyword);
        }
    }

    void AddEnchantmentFields(std::vector<std::string>& fields, const RE::TESForm& form) {
        const auto* enchantable = skyrim_cast<const RE::TESEnchantableForm*>(&form);
        if (!enchantable) {
            return;
        }

        const auto* enchant = enchantable->formEnchanting;
        if (!enchant) {
            return;
        }

        AddField(fields, enchant->GetName());
        AddField(fields, enchant->GetFormEditorID());

        for (const auto* effect : enchant->effects) {
            if (effect) {
                AddBaseEffectFields(fields, effect->baseEffect);
            }
        }
    }

    // Keyword-derived facets

    void AddKeywordFields(std::vector<std::string>& fields, RE::TESForm& form) {
        const auto* keywordForm = skyrim_cast<RE::BGSKeywordForm*>(&form);
        if (!keywordForm) {
            return;
        }

        for (const auto* keyword : keywordForm->GetKeywords()) {
            AddKeywordSearchFields(fields, keyword);
        }
    }

    void AddFormFields(std::vector<std::string>& fields, std::uint32_t formID) {
        if (formID == 0) {
            return;
        }

        fields.emplace_back(std::format("{:08X}", formID));

        const auto* form = RE::TESForm::LookupByID(formID);
        if (!form) {
            return;
        }

        AddField(fields, form->GetName());
        // Form editor ID is NOT indexed.
        AddField(fields, form->GetObjectTypeName());
        AddField(fields, RE::FormTypeToString(form->GetFormType()));
        AddFormTypeAliasFields(fields, form->GetFormType());
        AddKeywordFields(fields, *const_cast<RE::TESForm*>(form));
        IntentModel::AddKeywordDerivedFacets(fields, *form);
        IntentModel::AddWeightClassFields(fields, *form);
        AddEnchantmentFields(fields, *form);

        if (const auto* magic = form->As<RE::MagicItem>()) {
            const auto skill = magic->GetAssociatedSkill();
            if (const auto* skillName = ActorValueName(skill)) {
                fields.emplace_back(skillName);
            }
            if (const auto* spell = magic->As<RE::SpellItem>()) {
                AddSpellTypeFields(fields, spell->GetSpellType());
            }

            IntentModel::AddSpellIntentFields(fields, magic);

            // Alchemy discovery and index known effects only
            const auto* ingredient = form->As<RE::IngredientItem>();
            const std::uint16_t knownFlags = ingredient ? ingredient->gamedata.knownEffectFlags : 0xFFFF;

            for (std::uint32_t i = 0; i < magic->effects.size(); ++i) {
                const auto* effect = magic->effects[i];
                if (!effect) {
                    continue;
                }
                // For ingredients, only index discovered effects
                const bool known = !ingredient || (knownFlags & (1u << i)) != 0;
                if (known) {
                    AddBaseEffectFields(fields, effect->baseEffect);
                    IntentModel::AddEffectIntentFields(fields, effect, magic->GetDelivery());
                }
            }
        }

        if (const auto* shout = form->As<RE::TESShout>()) {
            for (const auto& var : shout->variations) {
                if (var.word) {
                    AddField(fields, var.word->GetName());
                    if (!var.word->translation.empty()) {
                        AddField(fields, var.word->translation.c_str());
                    }
                }
                if (var.spell) {
                    for (const auto* effect : var.spell->effects) {
                        if (effect) {
                            AddBaseEffectFields(fields, effect->baseEffect);
                        }
                    }
                }
            }
        }

        if (const auto* alch = form->As<RE::AlchemyItem>()) {
            if (alch->IsFood()) {
                // Only food flags no drink flags.
                fields.emplace_back("food");
            }
        }
    }

    // Word-start substring match. It still matches the start of a longer word, so fire and FIREball
    // conjur and CONJURation, dragon and DRAGONplate all keep working.
    // Inputs are already lowercased
    static bool WordStartMatch(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) {
            return true;
        }

        auto isWordChar = [](unsigned char c) {
            return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        };

        std::size_t pos = 0;
        while ((pos = haystack.find(needle, pos)) != std::string::npos) {
            const bool atBoundary = pos == 0 || !isWordChar(static_cast<unsigned char>(haystack[pos - 1]));
            if (atBoundary) {
                return true;
            }
            pos += 1;  // advance past this false hit and keep looking for a later word-start occurrence
        }
        return false;
    }

    // Whole-word match: "heal" does NOT match av:health but health does match.
    static bool WholeWordMatch(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) {
            return true;
        }
        auto isWordChar = [](unsigned char c) {
            return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        };
        std::size_t i = 0;
        const std::size_t n = haystack.size();
        while (i < n) {
            while (i < n && !isWordChar(static_cast<unsigned char>(haystack[i]))) ++i;  // skip separators
            const std::size_t start = i;
            while (i < n && isWordChar(static_cast<unsigned char>(haystack[i]))) ++i;    // consume word
            if (i - start == needle.size() && haystack.compare(start, needle.size(), needle) == 0) {
                return true;
            }
        }
        return false;
    }

    // Match token against item properties, splitting UI fields and cached form fields for performance.
    bool TokenMatchesFields(const Utils::Token& token, const std::vector<std::string>& live,
                            const std::vector<std::string>& cached, const RE::TESForm* form, bool isEnchanted) {
        auto anyField = [&](auto&& pred) {
            return std::ranges::any_of(live, pred) || std::ranges::any_of(cached, pred);
        };

        // Enforce strict tags if present
        if (!token.tag.empty() && form) {
            const auto type = form->GetFormType();

            if (token.tag == "w") {
                if (type != RE::FormType::Weapon) return false;
            } else if (token.tag == "a") {
                if (type != RE::FormType::Armor) return false;
            } else if (token.tag == "b") {
                if (type != RE::FormType::Book && type != RE::FormType::Note) return false;
            } else if (token.tag == "p") {
                // Strict: potions + poisons only. AlchemyItem covers potions/poisons/food, food is f: now,
                // so exclude IsFood() here to keep p: and f: split.
                if (type != RE::FormType::AlchemyItem) return false;
                const auto* alch = form->As<RE::AlchemyItem>();
                if (alch && alch->IsFood()) return false;
            } else if (token.tag == "f") {
                // Food/drink only. The engine has no drink flag 
                const auto* alch = form->As<RE::AlchemyItem>();
                if (!alch || !alch->IsFood()) return false;
            } else if (token.tag == "i") {
                // Strict: raw ingredients only.
                if (type != RE::FormType::Ingredient) return false;
            } else if (token.tag == "e") {
                if (!isEnchanted && type != RE::FormType::Spell && type != RE::FormType::Enchantment && type != RE::FormType::Scroll) return false;
            } else if (token.tag == "m") {
                // m: is material-only. m:steel catches material:steelplate
                static constexpr std::string_view prefix = "material:";
                if (token.value.empty()) {
                    return anyField([](const std::string& f) { return f.starts_with(prefix); });
                }
                return anyField([&](const std::string& f) {
                    return f.starts_with(prefix) && WordStartMatch(f, token.value);
                });
            }
        }

        // If no value provided
        if (token.value.empty()) {
            return true;
        }

        // av: fields has whole-word matching
        auto matchWord = [&](const std::string& w) {
            return anyField([&](const std::string& field) {
                if (field.starts_with("av:")) {
                    return WholeWordMatch(field, w);
                }
                return WordStartMatch(field, w);
            });
        };

        if (!token.values.empty()) {
            return std::ranges::any_of(token.values, matchWord);
        }
        return matchWord(token.value);
    }

    // Matches tokens and against two pre-normalized field lists.
    static bool MatchNormalized(const std::vector<std::string>& live, const std::vector<std::string>& cached,
                                const std::vector<Utils::Token>& tokens, const RE::TESForm* form, bool isEnchanted) {
        if (tokens.empty()) {
            return true;
        }
        if (live.empty() && cached.empty()) {
            return false;
        }
        for (const auto& token : tokens) {
            if (!TokenMatchesFields(token, live, cached, form, isEnchanted)) {
                return false;
            }
        }
        return true;
    }

    static void NormalizeFields(const std::vector<std::string>& fields, std::vector<std::string>& out) {
        out.reserve(out.size() + fields.size());
        for (const auto& field : fields) {
            if (!field.empty()) {
                out.push_back(Utils::ToLowerAscii(field));
            }
        }
    }

    // Whole-list path kept for MatchRawArguments: normalize everything into one list, match.
    bool FieldsMatchTokens(const std::vector<std::string>& fields, const std::vector<Utils::Token>& tokens, const RE::TESForm* form, bool isEnchanted) {
        if (tokens.empty()) {
            return true;
        }
        std::vector<std::string> normalized;
        NormalizeFields(fields, normalized);
        static const std::vector<std::string> kEmpty;
        return MatchNormalized(normalized, kEmpty, tokens, form, isEnchanted);
    }

    static std::unordered_map<std::uint32_t, std::vector<std::string>> s_formFieldCache;

    // RE
    struct RowBlob {
        std::uint32_t gen;
        std::vector<std::string> live;  // already-lowercased live fields (AddSkyUIEntryFields output)
    };
    static std::unordered_map<std::uint32_t, RowBlob> s_rowBlobCache;
    static std::uint32_t s_blobGen = 1;     // current generation; a blob is valid only if blob.gen == this
    static std::uint32_t s_nextRowId = 1;   // monotonic id stamped onto entries (_smartRowId), 0 = unset

    // Bump the generation so every cached row blob misses and rebuilds,
    // without freeing the map. Memory is bounded by ResetRowBlobs on menu open, not here.
    void InvalidateRowBlobs() {
        ++s_blobGen;
    }

    void ResetRowBlobs() {
        s_rowBlobCache.clear();
        s_blobGen = 1;
        s_nextRowId = 1;
    }

    std::uint32_t NextRowId() {
        return s_nextRowId++;
    }

    void ClearFieldCache() {
        s_formFieldCache.clear();
        s_rowBlobCache.clear();
        s_blobGen = 1;  // keep 0 as never valid
        s_nextRowId = 1;
    }

    PerfSplit& PerfSplitAccum() {
        thread_local PerfSplit s_split{};
        return s_split;
    }

    // Return the normalized form fields for formID, building and caching on a miss.
    static const std::vector<std::string>& CachedFormFields(std::uint32_t formID) {
        auto it = s_formFieldCache.find(formID);
        if (it == s_formFieldCache.end()) {
            std::vector<std::string> raw;
            AddFormFields(raw, formID);
            std::vector<std::string> normalized;
            NormalizeFields(raw, normalized);
            it = s_formFieldCache.emplace(formID, std::move(normalized)).first;
        }
        return it->second;
    }

    // Only read UI fields relevant to this items type to avoid slow Flash queries. 
    // Strings are appended pre lowercased to save allocations.
    void AddSkyUIEntryFields(const RE::GFxValue& entry, std::string_view nameAttribute,
                             const RE::TESForm* form, std::vector<std::string>& fields) {
        const bool nameIsText = nameAttribute == "text";
        if (nameIsText) {
            Utils::AppendLoweredStringMember(entry, "text", fields);
        } else {
            const std::string nameAttr(nameAttribute);
            Utils::AppendLoweredStringMember(entry, nameAttr.c_str(), fields);
        }

        auto readField = [&](const char* fieldName) {
            if (!nameIsText && nameAttribute == fieldName) {
                return;  // already read above via nameAttribute
            }
            if (nameIsText && std::strcmp(fieldName, "text") == 0) {
                return;
            }
            Utils::AppendLoweredStringMember(entry, fieldName, fields);
        };

        // Always-on, item-type agnostic.
        if (auto subType = Utils::TryGetStringMember(entry, "subTypeDisplay")) {
            std::string lowered = std::move(*subType);
            Utils::ToLowerAsciiInPlace(lowered);
            // Group circlet under head so broad searches include circlets
            if (lowered == "circlet") {
                fields.emplace_back("head");
            }
            fields.emplace_back(std::move(lowered));
        }
        readField("status");

        // Equipped, favorite.
        if (Utils::TryGetUIntMember(entry, "equipState").value_or(0) >= 1) {
            fields.emplace_back("equipped");
        }
        if (Utils::TryGetUIntMember(entry, "favorite").value_or(0) >= 1) {
            fields.emplace_back("favorite");
        }

        // Decide which type-specific families to read. Null form => read both.
        const bool isMagic = !form || form->As<RE::MagicItem>() || form->Is(RE::FormType::Shout);
        const bool isItem  = !form || form->IsWeapon() || form->IsArmor();

        if (isItem) {
            readField("materialDisplay");
            readField("weightClassDisplay");
        }
        if (isMagic) {
            readField("infoSchoolName");
            readField("infoCastLevel");
            readField("word0");
            readField("word1");
            readField("word2");
        }
    }

    bool MatchSkyUIEntry(const RE::GFxValue& entry, std::string_view query, std::string_view nameAttribute) {
        const auto tokens = Utils::TokenizeQuery(query);
        if (tokens.empty()) {
            return true;
        }
        return MatchSkyUIEntry(entry, tokens, nameAttribute);
    }

    // Match only the item name, serving as a base comparison.
    bool MatchNameOnly(const RE::GFxValue& entry, std::string_view query, std::string_view nameAttribute) {
        const std::string attr = nameAttribute.empty() ? "text" : std::string(nameAttribute);
        auto name = Utils::TryGetStringMember(entry, attr.c_str());
        if (!name) {
            return false;
        }
        Utils::ToLowerAsciiInPlace(*name);
        return name->find(query) != std::string::npos;
    }

    bool MatchSkyUIEntry(const RE::GFxValue& entry, const std::vector<Utils::Token>& tokens,
                         std::string_view nameAttribute, std::uint32_t rowId) {
        if (tokens.empty()) {
            return true;
        }

        // Perf diagnostics
        const bool perfDbg = spdlog::default_logger_raw()->level() <= spdlog::level::debug;
        const auto splitReadT0 = perfDbg ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};

        // Resolve the form first
        const auto formID = Utils::TryGetUIntMember(entry, "formId").or_else([&entry]() {
            return Utils::TryGetUIntMember(entry, "baseId");
        });
        const auto* form = formID ? RE::TESForm::LookupByID(*formID) : nullptr;

        // Retrieve UI fields from row cache, querying GFx on a miss or if rowId is 0.
        thread_local std::vector<std::string> scratchLive;  // backs the uncached (rowId==0) path
        const std::vector<std::string>* liveFieldsPtr = nullptr;
        if (rowId != 0) {
            auto& slot = s_rowBlobCache[rowId];
            if (slot.gen != s_blobGen) {            // miss (new id, or invalidated generation)
                slot.gen = s_blobGen;
                slot.live.clear();
                AddSkyUIEntryFields(entry, nameAttribute, form, slot.live);
            }
            liveFieldsPtr = &slot.live;
        } else {
            scratchLive.clear();
            AddSkyUIEntryFields(entry, nameAttribute, form, scratchLive);
            liveFieldsPtr = &scratchLive;
        }
        const std::vector<std::string>& liveFields = *liveFieldsPtr;

        // Point to cached form fields, or build them fresh if the item is an ingredient.
        thread_local std::vector<std::string> uncachedFormFields;
        uncachedFormFields.clear();
        const std::vector<std::string>* formFields = &uncachedFormFields;
        if (formID && *formID != 0) {
            const bool cacheable = form && form->GetFormType() != RE::FormType::Ingredient;
            if (cacheable) {
                formFields = &CachedFormFields(*formID);
            } else {
                thread_local std::vector<std::string> raw;
                raw.clear();
                AddFormFields(raw, *formID);
                NormalizeFields(raw, uncachedFormFields);
            }
        }

        bool isEnchanted = false;
        if (const auto ench = Utils::TryGetBoolMember(entry, "isEnchanted")) {
            isEnchanted = *ench;
        } else if (form && form->GetFormType() == RE::FormType::Spell) {
            isEnchanted = true;
        }

        // Perf-split: read half ended above, match half is the MatchNormalized call. Only timed in debug.
        if (perfDbg) {
            auto& split = PerfSplitAccum();
            const auto splitMatchT0 = std::chrono::steady_clock::now();
            split.readUs += std::chrono::duration<double, std::micro>(splitMatchT0 - splitReadT0).count();
            const bool matched = MatchNormalized(liveFields, *formFields, tokens, form, isEnchanted);
            split.matchUs += std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - splitMatchT0).count();
            return matched;
        }

        return MatchNormalized(liveFields, *formFields, tokens, form, isEnchanted);
    }

    bool MatchRawArguments(std::uint32_t formID, std::string_view query, std::string_view entryText) {
        const auto tokens = Utils::TokenizeQuery(query);
        if (tokens.empty()) {
            return true;
        }

        std::vector<std::string> fields;
        AddField(fields, entryText);
        AddFormFields(fields, formID);
        
        const auto* form = formID ? RE::TESForm::LookupByID(formID) : nullptr;
        
        return FieldsMatchTokens(fields, tokens, form, false);
    }
}
