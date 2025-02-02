#include "player.h"

#include "equippable.h"
#include "gear.h"
#include "shouts.h"
#include "utility.h"

#include "helpers.h"
#include "offset.h"

#include "lib.rs.h"

namespace player
{
	bool isInCombat() { return RE::PlayerCharacter::GetSingleton()->IsInCombat(); }

	bool weaponsAreDrawn() { return RE::PlayerCharacter::GetSingleton()->AsActorState()->IsWeaponDrawn(); }

	bool isVampireLord()
	{
		// DLC1VampireBeastRace; form id 0x0200283a
		const auto* race = RE::PlayerCharacter::GetSingleton()->GetRace();
		if (!race) { return false; }
		if (race->GetFormID() == 0x0200283a) { return true; }
		const auto* editorID = race->GetFormEditorID();
		if (editorID && std::strcmp(editorID, "DLC1VampireBeastRace") == 0) { return true; }
		return false;
	}

	bool isWerewolf()
	{
		// WerewolfBeastRace; form id 0x000cdd84
		const auto* race = RE::PlayerCharacter::GetSingleton()->GetRace();
		if (!race) { return false; }
		if (race->GetFormID() == 0x000cdd84) { return true; }
		const auto* editorID = race->GetFormEditorID();
		if (editorID && std::strcmp(editorID, "WerewolfBeastRace") == 0) { return true; }
		return false;
	}

	bool useCGOAltGrip()
	{
		bool useAltGrip = false;
		RE::PlayerCharacter::GetSingleton()->GetGraphVariableBool("bUseAltGrip", useAltGrip);
		return useAltGrip;
	}

	rust::String specEquippedLeft()
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		// I think this is a form already????
		const auto obj = player->GetActorRuntimeData().currentProcess->GetEquippedLeftHand();
		if (!obj) return std::string("unarmed_proxy");

		auto* form = RE::TESForm::LookupByID(obj->formID);
		if (!form) return std::string("unarmed_proxy");

		RE::TESBoundObject* bound    = nullptr;
		RE::ExtraDataList* extraData = nullptr;
		gear::boundObjectForWornItem(form, gear::WornWhere::kLeftOnly, bound, extraData);

		if (bound) { return helpers::makeFormSpecString(bound); }
		else { return helpers::makeFormSpecString(form); }
	}

	rust::String specEquippedRight()
	{
		auto* player   = RE::PlayerCharacter::GetSingleton();
		const auto obj = player->GetActorRuntimeData().currentProcess->GetEquippedRightHand();
		if (!obj) return std::string("unarmed_proxy");

		auto* form = RE::TESForm::LookupByID(obj->formID);
		if (!form) return std::string("unarmed_proxy");

		RE::TESBoundObject* bound    = nullptr;
		RE::ExtraDataList* extraData = nullptr;
		gear::boundObjectForWornItem(form, gear::WornWhere::kRightOnly, bound, extraData);

		if (bound) { return helpers::makeFormSpecString(bound); }
		else { return helpers::makeFormSpecString(form); }
	}

	rust::String specEquippedPower()
	{
		auto* player    = RE::PlayerCharacter::GetSingleton();
		const auto* obj = player->GetActorRuntimeData().selectedPower;
		if (!obj) return std::string("");
		auto* item_form = RE::TESForm::LookupByID(obj->formID);
		if (!item_form) return std::string("");
		return helpers::makeFormSpecString(item_form);
	}

	rust::String specEquippedAmmo()
	{
		auto player        = RE::PlayerCharacter::GetSingleton();
		auto* current_ammo = player->GetCurrentAmmo();
		if (!current_ammo || !current_ammo->IsAmmo()) { return std::string(""); }

		const auto formspec = helpers::makeFormSpecString(current_ammo);
		return formspec;
	}

	bool compare(RE::TESAmmo* left, RE::TESAmmo* right) { return (left->data.damage < right->data.damage); }

	rust::Vec<rust::String> getAmmoInventory()
	{
		auto player     = RE::PlayerCharacter::GetSingleton();
		auto* rightItem = player->GetActorRuntimeData().currentProcess->GetEquippedRightHand();
		bool useBolts   = false;
		if (rightItem->IsWeapon())
		{
			auto* weapon = rightItem->As<RE::TESObjectWEAP>();
			useBolts     = weapon->IsCrossbow();
		}
		else
		{
			// filter for the same type that we have equipped
			auto* currentAmmo = player->GetCurrentAmmo();
			useBolts          = currentAmmo->IsBolt();
		}

		auto ammoTypes = getInventoryForType(player, RE::FormType::Ammo);
		auto sorted    = new std::vector<RE::TESAmmo*>();
		for (const auto& [item, inv_data] : ammoTypes)
		{
			const auto& [num_items, entry] = inv_data;
			auto* new_ammo                 = item->As<RE::TESAmmo>();
			if ((num_items > 0) && (new_ammo->IsBolt() == useBolts)) { sorted->push_back(new_ammo); }
		}
		sort(sorted->begin(), sorted->end(), compare);

		auto specs = new rust::Vec<rust::String>();
		for (auto* ammo : *sorted)
		{
			auto spec = helpers::makeFormSpecString(ammo->As<RE::TESForm>());
			specs->push_back(rust::String(spec));
		}
		delete sorted;

		return std::move(*specs);
	}

	bool hasRangedEquipped()
	{
		auto player    = RE::PlayerCharacter::GetSingleton();
		const auto obj = player->GetActorRuntimeData().currentProcess->GetEquippedRightHand();
		if (!obj) return false;
		if (!obj->IsWeapon()) return false;
		const auto* weapon = obj->As<RE::TESObjectWEAP>();
		if (!weapon) return false;

		return weapon->IsBow() || weapon->IsCrossbow();
	}

	void unequipSlot(Action which)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();

		if (which == Action::Power) { shouts::unequipShoutSlot(player); }
		else if (which == Action::Right || which == Action::Left) { gear::unequipHand(player, which); }
		else { rlog::debug("somebody called unequipSlot() with slot={};"sv, static_cast<uint8_t>(which)); }
	}

	void unequipShout()
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		shouts::unequipShoutSlot(player);
	}

	void equipShout(const std::string& form_spec)
	{
		auto* shout_form = helpers::formSpecToFormItem(form_spec);
		if (!shout_form) { return; }
		auto* player = RE::PlayerCharacter::GetSingleton();
		shouts::equipShoutByForm(shout_form, player);
	}

	void equipMagic(const std::string& form_spec, Action slot)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player     = RE::PlayerCharacter::GetSingleton();
		auto* equip_slot = (slot == Action::Right ? gear::right_hand_equip_slot() : gear::left_hand_equip_slot());
		gear::equipSpellByFormAndSlot(form, equip_slot, player);
	}

	void equipWeapon(const std::string& form_spec, Action slot, const std::string& nameToMatch)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player     = RE::PlayerCharacter::GetSingleton();
		auto* equip_slot = (slot == Action::Left ? gear::left_hand_equip_slot() : gear::right_hand_equip_slot());
		gear::equipItemByFormAndSlot(form, equip_slot, player, nameToMatch);
	}

	void toggleArmor(const std::string& form_spec, const std::string& nameToMatch)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player = RE::PlayerCharacter::GetSingleton();
		utility::toggleArmorByForm(form, player, nameToMatch);
	}

	void equipArmor(const std::string& form_spec, const std::string& nameToMatch)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player = RE::PlayerCharacter::GetSingleton();
		utility::equipArmorByForm(form, player, nameToMatch);
	}

	void equipAmmo(const std::string& form_spec)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player = RE::PlayerCharacter::GetSingleton();
		utility::equipAmmoByForm(form, player);
	}

	void consumePotion(const std::string& form_spec)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }
		auto* player = RE::PlayerCharacter::GetSingleton();
		utility::consumePotion(form, player);
	}

	std::map<RE::TESBoundObject*, std::pair<int, std::unique_ptr<RE::InventoryEntryData>>>
		getInventoryForType(RE::PlayerCharacter*& a_player, RE::FormType a_type)
	{
		return a_player->GetInventory([a_type](const RE::TESBoundObject& a_object) { return a_object.Is(a_type); });
	}

	uint32_t itemCount(const std::string& form_spec)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return 0; }
		return getInventoryCountByForm(form);
	}

	uint32_t getInventoryCountByForm(const RE::TESForm* form)
	{
		if (!form) { return 0; }

		auto* player   = RE::PlayerCharacter::GetSingleton();
		uint32_t count = inventoryCount(form, form->GetFormType(), player);
		// rlog::trace("item='{}'; count={};"sv, helpers::nameAsUtf8(form), count);

		return count;
	}

	bool hasItemOrSpell(const std::string& form_spec)
	{
		if (form_spec.find(std::string("_proxy")) != std::string::npos) { return true; }
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form)
		{
			rlog::warn("unable to turn string formspec into valid form in-game: {}"sv, form_spec);
			return false;
		}

		auto* thePlayer = RE::PlayerCharacter::GetSingleton();
		auto has_it     = false;
		auto formType   = form->GetFormType();
		if (RE::FormType::Spell == formType)
		{
			auto* spell = form->As<RE::SpellItem>();
			has_it      = thePlayer->HasSpell(spell);
		}
		else if (RE::FormType::Shout == formType)
		{
			const auto shout = form->As<RE::TESShout>();
			has_it           = shouts::has_shout(thePlayer, shout);
		}
		else { has_it = inventoryCount(form, formType, thePlayer) > 0; }

		rlog::trace("player has: {}; name='{}'; formID={};"sv, has_it, helpers::nameAsUtf8(form), form_spec);

		return has_it;
	}

	void reequipHand(Action which, const std::string& form_spec, const std::string& nameToMatch)
	{
		auto* form = helpers::formSpecToFormItem(form_spec);
		if (!form) { return; }

		auto* thePlayer = RE::PlayerCharacter::GetSingleton();
		auto* slot      = which == Action::Left ? gear::left_hand_equip_slot() : gear::right_hand_equip_slot();
		gear::equipItemByFormAndSlot(form, slot, thePlayer, nameToMatch);
	}

	uint32_t inventoryCount(const RE::TESForm* form, RE::FormType formType, RE::PlayerCharacter*& thePlayer)
	{
		if (!form) { return 0; }
		auto count     = 0;
		auto inventory = getInventoryForType(thePlayer, formType);
		for (const auto& [item, inv_data] : inventory)
		{
			if (const auto& [num_items, entry] = inv_data; entry->object->formID == form->formID)
			{
				count = num_items;
				break;
			}
		}

		return count;
	}

	// ---------- counting potions for display in the HUD

	uint32_t potionCountByActorValue(RE::ActorValue vital_stat)
	{
		auto* the_player = RE::PlayerCharacter::GetSingleton();
		if (!the_player) return 0;

		uint32_t count = 0;

		for (auto candidates = player::getInventoryForType(the_player, RE::FormType::AlchemyItem);
			 const auto& [item, inv_data] : candidates)
		{
			const auto& [num_items, entry] = inv_data;

			auto* alchemy_item = item->As<RE::AlchemyItem>();
			if (alchemy_item->IsPoison() || alchemy_item->IsFood()) { continue; }
			auto actor_value = equippable::getPotionEffect(item, true);
			if (actor_value == vital_stat) count += num_items;
		}

		return count;
	}

	uint32_t staminaPotionCount() { return potionCountByActorValue(RE::ActorValue::kStamina); }
	uint32_t healthPotionCount() { return potionCountByActorValue(RE::ActorValue::kHealth); }
	uint32_t magickaPotionCount() { return potionCountByActorValue(RE::ActorValue::kMagicka); }

	void chooseStaminaPotion() { utility::consumeBestOption(RE::ActorValue::kStamina); }
	void chooseHealthPotion() { utility::consumeBestOption(RE::ActorValue::kHealth); }
	void chooseMagickaPotion() { utility::consumeBestOption(RE::ActorValue::kMagicka); }

	rust::Box<EquippedData> getEquippedItems()
	{
		auto specs = new rust::Vec<rust::String>();
		auto empty = new rust::Vec<uint8_t>();

		auto* the_player = RE::PlayerCharacter::GetSingleton();
		if (!the_player)
		{
			auto data = equipped_data(*specs, *empty);
			return std::move(data);
		}

		for (uint8_t shift = 0; shift < 32; shift++)
		{
			auto slot  = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(1 << shift);
			auto* item = the_player->GetWornArmor(slot);
			if (item)
			{
				std::string formSpec = helpers::makeFormSpecString(item);
				specs->push_back(formSpec);
			}
			else { empty->push_back(shift); };
		}

		auto data = equipped_data(*specs, *empty);
		return std::move(data);
	}

	void unequipSlotByShift(uint8_t shift)
	{
		auto slot        = static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(1 << shift);
		auto* the_player = RE::PlayerCharacter::GetSingleton();

		auto* item = the_player->GetWornArmor(slot);
		if (item)
		{
			auto* equip_manager = RE::ActorEquipManager::GetSingleton();
			auto* task          = SKSE::GetTaskInterface();
			task->AddTask([=]() { equip_manager->UnequipObject(the_player, item, nullptr); });
		}
	}

}  // player
