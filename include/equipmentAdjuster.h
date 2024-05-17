#pragma once

namespace AdjustWeapons {
	/*
	Here for convinience. Adjusts weapons based on settings.  Should be called after settings are initialized
	since that function fills in the classes' attributes.
	*/
	bool AdjustWeapons();

	class BowAdjuster : public Singleton<BowAdjuster> {
	public:
		bool Adjust();
	};

	class ArrowAdjuster : public Singleton<ArrowAdjuster> {
	public:
        bool Adjust();
        void UpdateArrowDamageSettings(bool a_adjustArrowDamage, double a_additionalDamage);
        void UpdateArrowSpeedSettings(bool a_adjustArrowSpeed, double a_newArrowSpeed);

	private:
		bool   bBuffArrowDamage;
		bool   bIncreaseArrowSpeed;
		double fAdditionalArrowDamage;
		double fNewArrowSpeed;
	};

	class BoltAdjuster : public Singleton<BoltAdjuster> {
	public:
		bool Adjust();
		void UpdateBoltDamageSettings(bool a_boltsPenetrateArmor, bool a_adjustBoltDamage, double a_additionalDamage);
		void UpdateBoltSpeedSettings(bool a_adjustBoltSpeed, double a_newBoltSpeed);

	private:
		bool   bBuffBoltDamage;
		bool   bIncreaseBoltSpeed;
		bool   bBoltsPenetrateArmor;
		double fAdditionalBoltDamage;
		double fNewBoltSpeed;
	};
}
