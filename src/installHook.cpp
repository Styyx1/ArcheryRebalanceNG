#include "installHook.h"
#include "eventHandler.h"

namespace Hooks
{
    struct UpdateCombatThreat
    {
        static void thunk(RE::CombatThreatMap* a_threatMap, RE::Projectile* a_projectile)
        {
            if (a_projectile) {
                auto* actorCause = a_projectile->GetProjectileRuntimeData().actorCause.get();
                auto* actor      = actorCause ? actorCause->actor.get().get() : nullptr;
                if (actor && (actor->IsPlayerRef() || EventHandler::OnEquip::GetSingleton()->NPCGoodToFire())) {
                    const auto* base           = a_projectile->GetBaseObject();
                    const auto* projectileBase = base ? base->As<RE::BGSProjectile>() : nullptr;
                    const auto* weaponSource   = projectileBase ? a_projectile->GetProjectileRuntimeData().weaponSource : nullptr;
                    if (weaponSource && weaponSource->IsBow()) {
                        float boundMultiplier  = 1.0f;
                        float weightMultiplier = 1.0f;
                        if (weaponSource && weaponSource->IsBow()) {
                            float boundMultiplier  = 1.0f;
                            float weightMultiplier = 1.0f;
                            if (weaponSource->IsBound() && weaponSource->GetWeight() < 10.0f) {
                                boundMultiplier = 2.0f;
                            }
                            // Bows are split into 4 categories. Light, medium, fluffy, and DAMN.
                            // Light: Weight of 5 or under. Multiplier: 0.3.
                            // Medium: Weight of 10 or under. Multiplier: 0.6.
                            // Fluffy: Weight of 15 or under. Multiplier: 1.0.
                            // DAMN: Weight of 20 or under. Multiplier: 1.3.
                            float weaponWeight = weaponSource->GetWeight();
                            if (weaponWeight <= 5.0f) {
                                weightMultiplier = 0.5f;
                            }
                            else if (weaponWeight <= 10.0f) {
                                weightMultiplier = 0.8f;
                            }
                            else if (weaponWeight <= 15.0f) {
                                weightMultiplier = 1.0f;
                            }
                            else {
                                weightMultiplier = 1.3f;
                            }
                            weightMultiplier *= boundMultiplier;
                            a_projectile->GetProjectileRuntimeData().linearVelocity *= weightMultiplier;
                        }
                    }
                }
            }
            func(a_threatMap, a_projectile);
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };

    struct CombatHit
    {
        static bool Install() { return true; }

        static void thunk(RE::Actor* target, RE::HitData* hitData)
        {
            if (auto* hitWeapon = hitData->weapon) {
                if (hitWeapon->IsCrossbow()) {
                    hitData->totalDamage = hitData->physicalDamage * hitData->bonusHealthDamageMult;
                }
            }
            return func(target, hitData);
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };

    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(43030, 44222), REL::Relocate(0x3CB, 0x79D, 0x3A8) };
        stl::write_thunk_call<UpdateCombatThreat>(target.address());
        logger::info("Installed hook.");
    }

    void InstallDamageResistPatch() // ID SE : 37673
    {
        logger::info("install damage resist patch");
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::Relocate(0x3c0, 0x4A8) };
        stl::write_thunk_call<CombatHit>(target.address());
    }

} // namespace Hooks
