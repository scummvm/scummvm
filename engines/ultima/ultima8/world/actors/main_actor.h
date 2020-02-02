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

struct WeaponOverlayFrame;

class MainActor : public Actor {
public:
	MainActor();
	virtual ~MainActor();

	virtual bool CanAddItem(Item *item, bool checkwghtvol = false) override;
	virtual bool addItem(Item *item, bool checkwghtvol = false) override;

	//! teleport to the given location on the given map
	virtual void teleport(int mapNum_, int32 x_, int32 y_, int32 z_) override;

	//! teleport to a teleport-destination egg
	//! \param mapnum The map to teleport to
	//! \param teleport_id The ID of the egg to teleport to
	void teleport(int mapNum_, int teleport_id); // to teleportegg

	bool hasJustTeleported() const {
		return justTeleported;
	}
	void setJustTeleported(bool t) {
		justTeleported = t;
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
	virtual GravityProcess *ensureGravityProcess() override;

	virtual uint32 getArmourClass() override;
	virtual uint16 getDefenseType() override;
	virtual int16 getAttackingDex() override;
	virtual int16 getDefendingDex() override;

	virtual uint16 getDamageType() override;
	virtual int getDamageAmount() override;

	virtual void setInCombat() override;
	virtual void clearInCombat() override;

	virtual ProcId die(uint16 DamageType) override;

	Std::string getName() {
		return name;
	}
	void setName(Std::string name_) {
		name = name_;
	}

	bool loadData(IDataSource *ids, uint32 version);

	//! "teleport" console command
	static void ConCmd_teleport(const Console::ArgvType &argv);
	//! "mark" console command
	static void ConCmd_mark(const Console::ArgvType &argv);
	//! "recall" console command
	static void ConCmd_recall(const Console::ArgvType &argv);
	//! "listmarks" console command
	static void ConCmd_listmarks(const Console::ArgvType &argv);
	//! "Name" console command
	static void ConCmd_name(const Console::ArgvType &argv);

	//! "maxstats" console command
	static void ConCmd_maxstats(const Console::ArgvType &argv);
	//! "heal" console command
	static void ConCmd_heal(const Console::ArgvType &argv);
	//! "toggleInvincibility" console command
	static void ConCmd_toggleInvincibility(const Console::ArgvType &argv);


	//! "useBackpack" console command
	static void ConCmd_useBackpack(const Console::ArgvType &argv);
	//! "useInventory" console command
	static void ConCmd_useInventory(const Console::ArgvType &argv);
	//! "useRecall" console command
	static void ConCmd_useRecall(const Console::ArgvType &argv);
	//! "useBedroll" console command
	static void ConCmd_useBedroll(const Console::ArgvType &argv);
	//! "useKeyring" console command
	static void ConCmd_useKeyring(const Console::ArgvType &argv);

	//! "toggleCombat" console command
	static void ConCmd_toggleCombat(const Console::ArgvType &argv);

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
	virtual void saveData(ODataSource *ods) override;

	void useInventoryItem(uint32 shapenum);

	bool justTeleported;

	int accumStr;
	int accumDex;
	int accumInt;

	Std::string name;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
