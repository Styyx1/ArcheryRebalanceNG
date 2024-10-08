#include "eventHandler.h"

namespace
{

    static RE::TESObjectWEAP* GetWieldingWeapon(RE::Actor* a_actor)
    {
        auto weapon = a_actor->GetAttackingWeapon();
        if (weapon) {
            return weapon->object->As<RE::TESObjectWEAP>();
        }
        auto rhs = a_actor->GetEquippedObject(false);
        if (rhs && rhs->IsWeapon()) {
            return rhs->As<RE::TESObjectWEAP>();
        }
        auto lhs = a_actor->GetEquippedObject(true);
        if (lhs && lhs->IsWeapon()) {
            return lhs->As<RE::TESObjectWEAP>();
        }
        return nullptr;
    }

    void HandleActorStateChanged(RE::Actor* eventActor, RE::InventoryEntryData* a_data, bool bAccountConjuration, float fConjurationWeight, bool a_playerOnly)
    { // Added in 2.1.0
        // Player exclusivity for speed settings.
        if (!eventActor->IsPlayerRef() && a_playerOnly)
            return;
        // Make sure that the actor has equipped ammo and either a bow or crossbow equipped.
        auto  rightEquipObject = a_data->object;
        auto* rightEquipWeapon = rightEquipObject->As<RE::TESObjectWEAP>();

        auto* equippedWeap = GetWieldingWeapon(eventActor);

        bool  isBow       = equippedWeap ? equippedWeap->IsBow() : false;
        auto* currentAmmo = isBow ? eventActor->GetCurrentAmmo() : nullptr;
        if (!currentAmmo)
            return;
        // Make sure that the ammo matches the ranged weapon equipped.
        if (isBow && currentAmmo->IsBolt() || (!isBow && !currentAmmo->IsBolt())) {
            return;
        }

        // Beyond this point, the actor has a bow or crossbow with matching ammo.
        float actorArchery = eventActor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);
        float weaponWeight = equippedWeap->GetWeight();

        if (weaponWeight < 5.0f) {
            weaponWeight = 5.0f;
        }
        else if (weaponWeight > 20.0f) {
            weaponWeight = 20.0f;
        }

        if (bAccountConjuration && equippedWeap->IsBound()) {
            float actorConjuration = eventActor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kConjuration);
            actorArchery           = (1.0f - fConjurationWeight) * actorArchery + fConjurationWeight * actorConjuration;
        }

        // Formula: Skill / (5 * Weapon Weight)
        // Weapon weight is between 5 and 20.
        float weaponSpeedFactor = actorArchery / (3.0f * weaponWeight);

        if (weaponSpeedFactor > 1.2f) {
            weaponSpeedFactor = 1.2f;
        }
        else if (weaponSpeedFactor < 0.4f) {
            weaponSpeedFactor = 0.4f;
        }
        equippedWeap->weaponData.speed = weaponSpeedFactor;
    }
} // namespace

namespace EventHandler
{

    bool OnEquip::RegisterListener()
    {
        if (!this->bAdjustBowDrawSpeed)
            return true;

        auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        if (!eventHolder)
            return false;

        eventHolder->AddEventSink(this);
        return true;
    }

    bool OnEquip::NPCGoodToFire()
    {
        return !this->bPlayerOnly;
    }

    void OnEquip::UpdateDrawSpeedSetting(bool a_enableDynamicDraw, bool a_playerOnly, bool a_enableConjurationFactor, double a_conjurationWeight)
    {
        this->bAdjustBowDrawSpeed = a_enableDynamicDraw;
        this->bAccountConjuration = a_enableConjurationFactor;

        if (a_conjurationWeight >= 0.0f && a_conjurationWeight <= 1.0f) {
            this->fConjurationWeight = a_conjurationWeight;
        }
        else {
            this->fConjurationWeight = 0.6f;
        }
    }

    RE::BSEventNotifyControl OnEquip::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource)
    {
        if (!a_event || !a_eventSource)
            return continueEvent;
        if (!a_event->equipped)
            return continueEvent;

        // Make sure an actor is the event's actor.
        auto* eventReference = a_event->actor.get();
        auto* eventActor     = eventReference ? eventReference->As<RE::Actor>() : nullptr;
        if (!eventActor)
            return continueEvent;

        // Make sure that a weapon or ammo piece was equipped while a weapon is in the right hand.
        auto  eventID        = a_event->baseObject;
        auto* eventForm      = RE::TESForm::LookupByID(eventID);
        auto* eventWeapon    = eventForm ? eventForm->As<RE::TESObjectWEAP>() : nullptr;
        auto* eventAmmo      = (eventForm && !eventWeapon) ? eventForm->As<RE::TESAmmo>() : nullptr;
        auto* rightEquipData = (eventAmmo || eventWeapon) ? eventActor->GetEquippedEntryData(false) : nullptr;
        if (!rightEquipData)
            return continueEvent;

        HandleActorStateChanged(eventActor, rightEquipData, this->bAccountConjuration, this->fConjurationWeight, this->bPlayerOnly);
        return continueEvent;
    }

    bool OnLoad::RegisterListener()
    {
        if (!this->bAdjustBowDrawSpeed)
            return true;

        auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        if (!eventHolder)
            return false;

        eventHolder->AddEventSink(this);
        return true;
    }

    void OnLoad::UpdateDrawSpeedSetting(bool a_enableDynamicDraw, bool a_enableConjurationFactor, bool a_playerOnly, double a_conjurationWeight)
    {
        this->bAdjustBowDrawSpeed = a_enableDynamicDraw;
        this->bAccountConjuration = a_enableConjurationFactor;
        this->bPlayerOnly         = a_playerOnly;

        if (a_conjurationWeight >= 0.0f && a_conjurationWeight <= 1.0f) {
            this->fConjurationWeight = a_conjurationWeight;
        }
        else {
            this->fConjurationWeight = 0.6f;
        }
    }

    RE::BSEventNotifyControl OnLoad::ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource)
    {
        if (!a_event || !a_eventSource)
            return continueEvent;
        if (!a_event->loaded)
            return continueEvent;

        // Make sure an actor is the event's actor.
        auto  eventID    = a_event->formID;
        auto* eventForm  = RE::TESForm::LookupByID(eventID);
        auto* eventActor = eventForm ? eventForm->As<RE::Actor>() : nullptr;
        if (!eventActor)
            return continueEvent;

        // Make sure that a weapon or ammo piece was equipped while a weapon is in the right hand.
        auto* rightEquipData = eventActor->GetEquippedEntryData(false);
        if (!rightEquipData)
            return continueEvent;

        HandleActorStateChanged(eventActor, rightEquipData, this->bAccountConjuration, this->fConjurationWeight, this->bPlayerOnly);
        return continueEvent;
    }
} // namespace EventHandler
