/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	MainActor();
	~MainActor() override;

	bool CanAddItem(Item *item, bool checkwghtvol = false) override;
	bool addItem(Item *item, bool checkwghtvol = false) override;

	//! teleport to the given location on the given map
	void teleport(int mapNum_, int32 x_, int32 y_, int32 z_) override;

	//! teleport to a teleport-destination egg
	//! \param mapnum The map to teleport to
	//! \param teleport_id The ID of the egg to teleport to
	void teleport(int mapNum_, int teleport_id); // to teleportegg

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

	void setInCombat() override;
	void clearInCombat() override;

	ProcId die(uint16 DamageType) override;

	const Std::string &getName() const {
		return _name;
	}
	void setName(const Std::string &name) {
		_name = name;
	}

	bool loadData(IDataSource *ids, uint32 version);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	INTRINSIC(I_teleportToEgg);
	INTRINSIC(I_accumulateStrength);
	INTRINSIC(I_accumulateDexterity);
	INTRINSIC(I_accumulateIntelligence);
	INTRINSIC(I_clrAvatarInCombat);
	INTRINSIC(I_setAvatarInCombat);
	INTRINSIC(I_isAvatarInCombat);

	void getWeaponOverlay(const WeaponOverlayFrame *&frame_, uint32 &shape_);


protected:
	void saveData(ODataSource *ods) override;

	void useInventoryItem(uint32 shapenum);

	bool _justTeleported;

	int _accumStr;
	int _accumDex;
	int _accumInt;

	Std::string _name;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
