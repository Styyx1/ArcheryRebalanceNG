#include "equipmentAdjuster.h"

namespace AdjustWeapons {
	bool AdjustWeapons() {
		if (ArrowAdjuster::GetSingleton()->Adjust()) {
			logger::info("Finished adjusting arrows.");
		}
		else {
            logger::error("Encountered error while adjusting arrows.");
            return false;
        }

		if (BoltAdjuster::GetSingleton()->Adjust()) {
            logger::info("Finished adjusting bolts.");
		}
		else {
            logger::error("Encountered error while adjusting bolts.");
            return false;
        }

		if (BowAdjuster::GetSingleton()->Adjust()) {
            logger::info("Finished adjusting bows.");
		}
		else {
            logger::error("Encountered error while adjusting bows.");
            return false;
        }

        return true;
    }

    void ArrowAdjuster::UpdateArrowDamageSettings(bool a_adjustArrowDamage, double a_additionalDamage)
    {
        this->bBuffArrowDamage = a_adjustArrowDamage;

        if (a_additionalDamage >= 0.0f) {
            if (a_additionalDamage > 100.0f) {
                this->fAdditionalArrowDamage = 100.f;
            }
            else {
                this->fAdditionalArrowDamage = a_additionalDamage;
            }
        }
        else {
            this->fAdditionalArrowDamage = 0.0f;
        }
    }

    void BoltAdjuster::UpdateBoltDamageSettings(bool a_boltsPenetrateArmor, bool a_adjustBoltDamage, double a_additionalDamage)
    {
        this->bBuffBoltDamage      = a_adjustBoltDamage;
        this->bBoltsPenetrateArmor = a_boltsPenetrateArmor;

        if (a_additionalDamage >= 0.0f) {
            if (a_additionalDamage > 100.0f) {
                this->fAdditionalBoltDamage = 100.f;
            }
            else {
                this->fAdditionalBoltDamage = a_additionalDamage;
            }
        }
        else {
            this->fAdditionalBoltDamage = 0.0f;
        }
    }

    void ArrowAdjuster::UpdateArrowSpeedSettings(bool a_adjustArrowSpeed, double a_newArrowSpeed)
    {
        this->bIncreaseArrowSpeed = a_adjustArrowSpeed;

        if (a_newArrowSpeed > 1000.0f) {
            if (a_newArrowSpeed > 15000.0f) {
                this->fNewArrowSpeed = 15000.0f;
            }
            else {
                this->fNewArrowSpeed = a_newArrowSpeed;
            }
        }
        else {
            this->bIncreaseArrowSpeed = false;
            this->fNewArrowSpeed      = 0.0f;
        }
    }

    void BoltAdjuster::UpdateBoltSpeedSettings(bool a_adjustBoltSpeed, double a_newBoltSpeed)
    {
        this->bIncreaseBoltSpeed = a_adjustBoltSpeed;

        if (a_newBoltSpeed > 1000.0f) {
            if (a_newBoltSpeed > 15000.0f) {
                this->fNewBoltSpeed = 15000.0f;
            }
            else {
                this->fNewBoltSpeed = a_newBoltSpeed;
            }
        }
        else {
            this->bIncreaseBoltSpeed = false;
            this->fNewBoltSpeed      = 0.0f;
        }
    }

	bool ArrowAdjuster::Adjust()
    {
        if (!bBuffArrowDamage && !bIncreaseArrowSpeed) return true; 
		const auto& dataHandler = RE::TESDataHandler::GetSingleton();
		std::vector<std::pair<std::string, std::pair<bool, bool>>> adjustedArrows;

        if (!dataHandler) {
            logger::error("Failed to get the data handler.");
            return false;
        }

		const auto& ammoArray = dataHandler->GetFormArray<RE::TESAmmo>();
        for (RE::TESAmmo* ammo : ammoArray) {
            auto& ammoData = ammo->GetRuntimeData().data;
            if (!(ammoData.flags & RE::AMMO_DATA::Flag::kNonBolt)) continue;            
            if (ammoData.flags & RE::AMMO_DATA::Flag::kNonPlayable) continue;
            if (ammoData.damage < 1.0f) continue;            
			std::string ammoName = ammo->GetName();
			bool bAdjustedSpeed = false;
			bool bAdjustedDamage = false;

			if (this->bIncreaseArrowSpeed) {
				auto& ammoProjectile = ammoData.projectile;
				if (ammoProjectile) {
                    ammoProjectile->data.speed = this->fNewArrowSpeed;
					bAdjustedSpeed = true;
				}
			}

			if (this->bBuffArrowDamage) {
                ammoData.damage += this->fAdditionalArrowDamage;
                logger::info("adjusted arrow damage to {}", ammoData.damage);
				bAdjustedDamage = true;
			}

			if (!ammoName.empty() && (bAdjustedDamage || bAdjustedSpeed)) {
				std::pair<std::string, std::pair<bool, bool>> newPair;
				newPair.first = ammoName;
				newPair.second.first = bAdjustedSpeed;
				newPair.second.second = bAdjustedDamage;
				adjustedArrows.push_back(newPair);
			}
		}        

        if (!adjustedArrows.empty()) {
            logger::info("");
            logger::info("================================================================");
            logger::info("Arrow Adjustment Report:");
            logger::info("New speed: {}.", this->fNewArrowSpeed);
            logger::info("Additional damage: {}.", this->fAdditionalArrowDamage);
            for (auto& arrowInfo : adjustedArrows) {
                logger::info("    >{}:", arrowInfo.first);
                logger::info("        Adjusted Speed: {}, Adjusted Damage: {}.", arrowInfo.second.first, arrowInfo.second.second);
            }
            logger::info("================================================================");
        }
        return true;
    }

	bool BoltAdjuster::Adjust() {
        if (!bBuffBoltDamage && !bIncreaseBoltSpeed && !bBoltsPenetrateArmor) return true;        

        const auto&                                                dataHandler = RE::TESDataHandler::GetSingleton();
        std::vector<std::pair<std::string, std::pair<bool, bool>>> adjustedBolts;

        if (!dataHandler) {
            logger::error("Failed to get the data handler.");
            return false;
        }

		const auto& ammoArray = dataHandler->GetFormArray<RE::TESAmmo>();
        for (RE::TESAmmo* ammo : ammoArray) {
            auto& ammoData = ammo->GetRuntimeData().data;
			if ((ammoData.flags & RE::AMMO_DATA::Flag::kNonBolt)) continue;
			if (ammoData.flags & RE::AMMO_DATA::Flag::kNonPlayable) continue;
			if (ammoData.damage < 1.0f) continue;
			std::string ammoName = ammo->GetName();
			bool bAdjustedSpeed = false;
			bool bAdjustedDamage = false;

			if (this->bIncreaseBoltSpeed) {
				auto& ammoProjectile = ammoData.projectile;
				if (ammoProjectile) {
					ammoProjectile->data.speed = this->fNewBoltSpeed;
					bAdjustedSpeed = true;
				}
			}

            if (this->bBuffBoltDamage) {
                ammoData.damage += this->fAdditionalBoltDamage;
                bAdjustedDamage = true;
            }

            if (this->bBoltsPenetrateArmor) {
                ammo->data.flags.set(RE::AMMO_DATA::Flag::kIgnoresNormalWeaponResistance);
            }

            if (!ammoName.empty() && (bAdjustedDamage || bAdjustedSpeed)) {
                std::pair<std::string, std::pair<bool, bool>> newPair;
                newPair.first         = ammoName;
                newPair.second.first  = bAdjustedSpeed;
                newPair.second.second = bAdjustedDamage;
                adjustedBolts.push_back(newPair);
            }
        }

        logger::info("");
        logger::info("================================================================");
        logger::info("Bolt Adjustment Report:");
        logger::info("New speed: {}.", this->fNewBoltSpeed);
        logger::info("Additional damage: {}.", this->fAdditionalBoltDamage);
        logger::info("Penetrate armor: {}.", this->bBoltsPenetrateArmor);

        for (auto& boltInfo : adjustedBolts) {
            logger::info("    >{}:", boltInfo.first);
            logger::info("        Adjusted Speed: {}, Adjusted Damage: {}.", boltInfo.second.first, boltInfo.second.second);
        }

        logger::info("================================================================");
        return true;
    }

	bool BowAdjuster::Adjust() {
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		std::vector<std::string> adjustedBows;

        if (!dataHandler) {
            logger::error("Failed to get the data handler.");
            return false;
        }

        const auto& weaponArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();
        for (auto* weapon : weaponArray) {
            if (!weapon->GetPlayable())
                continue;
            if (!weapon->IsBow())
                continue;
            std::string bowName = weapon->GetName();

            weapon->weaponData.speed = 1.0f;
            if (!bowName.empty()) {
                adjustedBows.push_back(bowName);
            }
        }

        std::sort(adjustedBows.begin(), adjustedBows.end());
        logger::info("");
        logger::info("================================================================");
        logger::info("Bow Adjustment Report:");
        for (auto& bow : adjustedBows) {
            logger::info("    >{}", bow);
        }
        logger::info("================================================================");
        return true;
    }
} // namespace AdjustWeapons
