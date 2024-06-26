#pragma once

namespace EventHandler
{
#define continueEvent RE::BSEventNotifyControl::kContinue

    /*
	Listens for actors equipping (bows and arrows) or (crossbows and bolts) and reacts appropriately.
	*/
    class OnEquip : public RE::BSTEventSink<RE::TESEquipEvent>, public Singleton<OnEquip>
    {
    public:
        // Registers the listener, so it can actually respond to game events.
        bool NPCGoodToFire();
        bool RegisterListener();
        void UpdateDrawSpeedSetting(bool a_enableDynamicDraw, bool a_playerOnly, bool a_enableConjurationFactor, double a_conjurationWeight);

    private:
        RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;

        bool   bAdjustBowDrawSpeed;
        bool   bAccountConjuration;
        bool   bPlayerOnly;
        double fConjurationWeight;
    };

    class OnLoad : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public Singleton<OnLoad>
    {
    public:
        bool RegisterListener();
        void UpdateDrawSpeedSetting(bool a_enableDynamicDraw, bool a_playerOnly, bool a_enableConjurationFactor, double a_conjurationWeight);

    private:
        RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) override;

        bool   bAdjustBowDrawSpeed;
        bool   bAccountConjuration;
        bool   bPlayerOnly;
        double fConjurationWeight;
    };
} // namespace EventHandler
