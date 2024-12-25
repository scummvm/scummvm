/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WORLD_ACTORS_MAINACTOR_H
#define WORLD_ACTORS_MAINACTOR_H

#include "ultima/ultima8/world/actors/actor.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;
struct WeaponOverlayFrame;

class MainActor : public Actor {
	friend class Debugger;
public:
	enum CruBatteryType {
		NoBattery = 0,
		ChemicalBattery = 1,
		FissionBattery = 2,
		FusionBattery = 3
	};

	MainActor();
	~MainActor() override;

	bool CanAddItem(Item *item, bool checkwghtvol = false) override;
	bool addItem(Item *item, bool checkwghtvol = false) override;

	//! Get the ShapeInfo object for this MainActor.  Overridden because it changes
	//! when Crusader is kneeling.
	const ShapeInfo *getShapeInfoFromGameInstance() const override;

	void move(int32 X, int32 Y, int32 Z) override;

	//! Add item to avatar's inventory, but with some extra logic to do things like combine
	//! ammo and credits, use batteries, etc.
	int16 addItemCru(Item *item, bool showtoast);

	//! Remove a single item - only called from an intrinsic
	bool removeItemCru(Item *item);

	//! teleport to the given location on the given map
	void teleport(int mapNum, int32 x, int32 y, int32 z) override;

	//! teleport to a teleport-destination egg
	//! \param mapnum The map to teleport to
	//! \param teleport_id The ID of the egg to teleport to
	void teleport(int mapNum, int teleport_id); // to teleportegg

	bool hasJustTeleported() const {
		return _justTeleported;
	}
	void setJustTeleported(bool t) {
		_justTeleported = t;
	}

	//! accumulate a little bit of strength. When you reach 650 you gain
	//! one strength point. (There's a chance you gain strength sooner)
	void accumulateStr(int n);

	//! accumulate a little bit of dexterity. When you reach 650 you gain
	//! one dex. point. (There's a chance you gain dex. sooner)
	void accumulateDex(int n);

	//! accumulate a little bit of intelligence. When you reach 650 you gain
	//! one int. point. (There's a chance you gain int. sooner)
	void accumulateInt(int n);

	//! Get the GravityProcess of this Item, creating it if necessary
	GravityProcess *ensureGravityProcess() override;

	uint32 getArmourClass() const override;
	uint16 getDefenseType() const override;
	int16 getAttackingDex() const override;
	int16 getDefendingDex() const override;

	uint16 getDamageType() const override;
	int getDamageAmount() const override;

	void toggleInCombat() {
		if (isInCombat())
			clearInCombat();
		else
			setInCombat(0);
	}

	// Note: activity num parameter is ignored for Avatar.
	void setInCombat(int activity) override;
	void clearInCombat() override;

	ProcId die(uint16 damageType, uint16 damagePts, Direction srcDir) override;

	const Std::string &getName() const {
		return _name;
	}
	void setName(const Std::string &name) {
		_name = name;
	}

	int16 getMaxEnergy();

	CruBatteryType getBatteryType() const {
		return _cruBatteryType;
	}
	void setBatteryType(CruBatteryType newbattery) {
		_cruBatteryType = newbattery;
		setMana(getMaxEnergy());
	}

	void setShieldType(uint16 shieldtype) {
		_shieldType = shieldtype;
	}

	uint16 getShieldType() {
		return _shieldType;
	}

	bool hasKeycard(int num) const;
	void addKeycard(int bitno);

	void clrKeycards() {
		_keycards = 0;
	}

	uint16 getActiveInvItem() const {
		return _activeInvItem;
	}

	//! Swap to the next active weapon (Crusader)
	void nextWeapon();

	//! Swap to the next inventory item (Crusader)
	void nextInvItem();

	//! Drop the current weapon (Crusader)
	void dropWeapon();

	//! Check if we can absorb a hit with the shield. Returns the modified damage value.
	int receiveShieldHit(int damage, uint16 damage_type) override;

	//! Detonate used bomb (Crusader)
	void detonateBomb();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	ENABLE_RUNTIME_CLASSTYPE()

	INTRINSIC(I_teleportToEgg);
	INTRINSIC(I_accumulateStrength);
	INTRINSIC(I_accumulateDexterity);
	INTRINSIC(I_accumulateIntelligence);
	INTRINSIC(I_clrAvatarInCombat);
	INTRINSIC(I_setAvatarInCombat);
	INTRINSIC(I_isAvatarInCombat);
	INTRINSIC(I_getMaxEnergy);
	INTRINSIC(I_hasKeycard);
	INTRINSIC(I_clrKeycards);
	INTRINSIC(I_addItemCru);
	INTRINSIC(I_getNumberOfCredits);
	INTRINSIC(I_switchMap);
	INTRINSIC(I_removeItemCru);

	void getWeaponOverlay(const WeaponOverlayFrame *&frame, uint32 &shape);


protected:
	void useInventoryItem(uint32 shapenum);
	void useInventoryItem(Item *item);

	bool _justTeleported;

	int _accumStr;
	int _accumDex;
	int _accumInt;

	uint32 _keycards;
	CruBatteryType _cruBatteryType;
	uint16 _activeInvItem;

	Std::string _name;

	//! Process for a shield zap animation sprite
	uint16 _shieldSpriteProc;
	//! Type of shield (only used in Crusader)
	uint16 _shieldType;

	static ShapeInfo *_kneelingShapeInfo;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
