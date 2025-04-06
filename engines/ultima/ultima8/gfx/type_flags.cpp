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

#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/gfx/type_flags.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/games/treasure_loader.h"

namespace Ultima {
namespace Ultima8 {

TypeFlags::TypeFlags() {
}


TypeFlags::~TypeFlags() {
}

ShapeInfo *TypeFlags::getShapeInfo(uint32 shapenum) {
	if (shapenum < _shapeInfo.size())
		return &(_shapeInfo[shapenum]);
	else
		return nullptr;
}


void TypeFlags::load(Common::SeekableReadStream *rs) {
	unsigned int blocksize = 8;
	if (GAME_IS_CRUSADER) {
		blocksize = 9;
	}

	uint32 size = rs->size();
	uint32 count = size / blocksize;

	_shapeInfo.clear();
	_shapeInfo.resize(count);

	for (uint32 i = 0; i < count; ++i) {
		uint8 data[9];
		rs->read(data, blocksize);

		ShapeInfo si;
		si._flags = 0;

		if (GAME_IS_U8) {

			if (data[0] & 0x01) si._flags |= ShapeInfo::SI_FIXED;
			if (data[0] & 0x02) si._flags |= ShapeInfo::SI_SOLID;
			if (data[0] & 0x04) si._flags |= ShapeInfo::SI_SEA;
			if (data[0] & 0x08) si._flags |= ShapeInfo::SI_LAND;
			if (data[0] & 0x10) si._flags |= ShapeInfo::SI_OCCL;
			if (data[0] & 0x20) si._flags |= ShapeInfo::SI_BAG;
			if (data[0] & 0x40) si._flags |= ShapeInfo::SI_DAMAGING;
			if (data[0] & 0x80) si._flags |= ShapeInfo::SI_NOISY;

			if (data[1] & 0x01) si._flags |= ShapeInfo::SI_DRAW;
			if (data[1] & 0x02) si._flags |= ShapeInfo::SI_IGNORE;
			if (data[1] & 0x04) si._flags |= ShapeInfo::SI_ROOF;
			if (data[1] & 0x08) si._flags |= ShapeInfo::SI_TRANSL;
			si._family = data[1] >> 4;

			si._equipType = data[2] & 0x0F;
			si._x = data[2] >> 4;

			si._y = data[3] & 0x0F;
			si._z = data[3] >> 4;

			si._animType = data[4] & 0x0F;
			si._animData = data[4] >> 4;
			si._animSpeed = data[5] & 0x0F;

			if (data[5] & 0x10) si._flags |= ShapeInfo::SI_EDITOR;
			if (data[5] & 0x20) si._flags |= ShapeInfo::SI_U8_EXPLODE;
			if (data[5] & 0x40) si._flags |= ShapeInfo::SI_UNKNOWN46;
			if (data[5] & 0x80) si._flags |= ShapeInfo::SI_UNKNOWN47;

			si._weight = data[6];
			si._volume = data[7];

		} else if (GAME_IS_CRUSADER) {
			// Changes from U8 to Crusader:
			// * SI_OCCL seems to be used more like "target" in Cru
			// * SI_BAG bit seems to have a different meaning in Cru
			//   (multi-panel?), but we don't use it anyway
			// * Family/x/y/z are all now 5 bits
			// * There are more and different flags in the last byte

			if (data[0] & 0x01) si._flags |= ShapeInfo::SI_FIXED;
			if (data[0] & 0x02) si._flags |= ShapeInfo::SI_SOLID;
			if (data[0] & 0x04) si._flags |= ShapeInfo::SI_SEA;
			if (data[0] & 0x08) si._flags |= ShapeInfo::SI_LAND;
			if (data[0] & 0x10) si._flags |= ShapeInfo::SI_OCCL;
			if (data[0] & 0x20) si._flags |= ShapeInfo::SI_BAG;
			if (data[0] & 0x40) si._flags |= ShapeInfo::SI_DAMAGING;
			if (data[0] & 0x80) si._flags |= ShapeInfo::SI_NOISY;

			if (data[1] & 0x01) si._flags |= ShapeInfo::SI_DRAW;
			if (data[1] & 0x02) si._flags |= ShapeInfo::SI_IGNORE;
			if (data[1] & 0x04) si._flags |= ShapeInfo::SI_ROOF;
			if (data[1] & 0x08) si._flags |= ShapeInfo::SI_TRANSL;
			si._family = data[1] >> 4;
			si._family += (data[2] & 1) << 4;

			si._equipType = (data[2] >> 1) & 0xF;

			si._x = ((data[3] << 3) | (data[2] >> 5)) & 0x1F;
			si._y = (data[3] >> 2) & 0x1F;
			si._z = ((data[4] << 1) | (data[3] >> 7)) & 0x1F;

			si._animType = data[4] >> 4;
			si._animData = data[5] & 0x0F;
			si._animSpeed = data[5] >> 4;

			if (si._animType != 0 && si._animSpeed == 0) {
				// avoid invalid speeds.
				warning("fixing anim speed 0 for shape %d", i);
				si._animSpeed = 1;
			}

			if (data[6] & 0x01) si._flags |= ShapeInfo::SI_EDITOR;
			if (data[6] & 0x02) si._flags |= ShapeInfo::SI_CRU_SELECTABLE;
			if (data[6] & 0x04) si._flags |= ShapeInfo::SI_CRU_PRELOAD;
			if (data[6] & 0x08) si._flags |= ShapeInfo::SI_CRU_SOUND;
			if (data[6] & 0x10) si._flags |= ShapeInfo::SI_CRU_TARGETABLE;
			if (data[6] & 0x20) si._flags |= ShapeInfo::SI_CRU_NPC;
			if (data[6] & 0x40) si._flags |= ShapeInfo::SI_CRU_UNK66;
			if (data[6] & 0x80) si._flags |= ShapeInfo::SI_CRU_UNK67;

			si._weight = data[7];
			si._volume = data[8];
		} else {
			error("unknown game type in type flags");
		}

		si._weaponInfo = nullptr;
		si._armourInfo = nullptr;

		_shapeInfo[i] = si;
	}

	if (GAME_IS_U8) {
		// Workaround for incorrectly set solid flags on some "moss
		// curtains" in the catacombs. See also docs/u8bugs.txt
		for (uint32 i = 459; i <= 464; ++i) {
			_shapeInfo[i]._flags &= ~ShapeInfo::SI_SOLID;
		}
	}

	loadWeaponInfo();
	loadArmourInfo();
	loadMonsterInfo();
}


// load weapon info from the 'weapons' config root
void TypeFlags::loadWeaponInfo() {
	ConfigFileManager *config = ConfigFileManager::get_instance();

	// load weapons
	Std::vector<Std::string> weaponkeys;
	Std::string category = "weapons";
	weaponkeys = config->listSections(category);
	for (const auto &section : weaponkeys) {
		WeaponInfo *wi = new WeaponInfo;

		int val = 0;

		wi->_name = section;

		config->get(category, section, "shape", val);
		wi->_shape = static_cast<uint32>(val);

		config->get(category, section, "overlay", val);
		wi->_overlayType = static_cast<uint8>(val);

		config->get(category, section, "overlay_shape", val);
		wi->_overlayShape = static_cast<uint32>(val);

		config->get(category, section, "damage_mod", val);
		wi->_damageModifier = static_cast<uint8>(val);

		config->get(category, section, "base_damage", val);
		wi->_baseDamage = static_cast<uint8>(val);

		if (config->get(category, section, "attack_dex", val))
			wi->_dexAttackBonus = static_cast<uint8>(val);
		else
			wi->_dexAttackBonus = 0;

		if (config->get(category, section, "defend_dex", val))
			wi->_dexDefendBonus = static_cast<uint8>(val);
		else
			wi->_dexDefendBonus = 0;

		if (config->get(category, section, "armour", val))
			wi->_armourBonus = static_cast<uint8>(val);
		else
			wi->_armourBonus = 0;

		config->get(category, section, "damage_type", val);
		wi->_damageType = static_cast<uint16>(val);

		if (config->get(category, section, "treasure_chance", val))
			wi->_treasureChance = static_cast<uint16>(val);
		else
			wi->_treasureChance = 0;

		// Crusader-specific fields:

		if (config->get(category, section, "ammo_type", val))
			wi->_ammoType = static_cast<uint16>(val);
		else
			wi->_ammoType = 0;

		if (config->get(category, section, "ammo_shape", val))
			wi->_ammoShape = static_cast<uint16>(val);
		else
			wi->_ammoShape = 0;

		if (config->get(category, section, "sound", val))
			wi->_sound = static_cast<uint16>(val);
		else
			wi->_sound = 0;

		if (config->get(category, section, "reload_sound", val))
			wi->_reloadSound = static_cast<uint16>(val);
		else
			wi->_reloadSound = 0;

		if (config->get(category, section, "display_frame", val))
			wi->_displayGumpFrame = static_cast<uint16>(val);
		else
			wi->_displayGumpFrame = 0;

		if (config->get(category, section, "display_shape", val))
			wi->_displayGumpShape = static_cast<uint16>(val);
		else
			wi->_displayGumpShape = 3;

		if (config->get(category, section, "small", val))
			wi->_small = static_cast<uint8>(val);
		else
			wi->_small = 0;

		if (config->get(category, section, "clip_size", val))
			wi->_clipSize = static_cast<uint16>(val);
		else
			wi->_clipSize = 10;

		if (config->get(category, section, "energy", val))
			wi->_energyUse = static_cast<uint16>(val);
		else
			wi->_energyUse = 0;

		if (config->get(category, section, "shot_delay", val))
			wi->_shotDelay = static_cast<uint16>(val);
		else
			wi->_shotDelay = 0;

		// TODO: this should be 1, 2, or 3 depending on weapon.
		// It's used in the AttackProcess
		wi->_field8 = 1;

		if (wi->_shape > _shapeInfo.size()) {
			warning("ignoring weapon info for shape %d beyond size %d.",
					wi->_shape, _shapeInfo.size());
			delete wi;
			continue;
		}
		_shapeInfo[wi->_shape]._weaponInfo = wi;
	}
}


void TypeFlags::loadArmourInfo() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	MainShapeArchive *msf = GameData::get_instance()->getMainShapes();

	// load armour
	Std::vector<Std::string> armourkeys;
	Std::string category = "armour";
	armourkeys = config->listSections(category);
	for (const auto &section : armourkeys) {
		ArmourInfo ai;

		int val;

		config->get(category, section, "shape", val);
		ai._shape = static_cast<uint32>(val);

		assert(ai._shape < _shapeInfo.size());
		assert(msf->getShape(ai._shape));
		unsigned int framecount = msf->getShape(ai._shape)->frameCount();
		ArmourInfo *aia = _shapeInfo[ai._shape]._armourInfo;
		if (!aia) {
			aia = new ArmourInfo[framecount];
			_shapeInfo[ai._shape]._armourInfo = aia;
			for (unsigned int i = 0; i < framecount; ++i) {
				aia[i]._shape = 0;
				aia[i]._frame = 0;
				aia[i]._armourClass = 0;
				aia[i]._defenseType = 0;
				aia[i]._kickAttackBonus = 0;
			}
		}

		config->get(category, section, "frame", val);
		ai._frame = static_cast<uint32>(val);

		assert(ai._frame < framecount);

		config->get(category, section, "armour", val);
		ai._armourClass = static_cast<uint16>(val);

		if (config->get(category, section, "type", val))
			ai._defenseType = static_cast<uint16>(val);
		else
			ai._defenseType = 0;

		if (config->get(category, section, "kick_bonus", val))
			ai._kickAttackBonus = static_cast<uint16>(val);
		else
			ai._kickAttackBonus = 0;

		aia[ai._frame] = ai;
	}
}

void TypeFlags::loadMonsterInfo() {
	ConfigFileManager *config = ConfigFileManager::get_instance();

	TreasureLoader treasureLoader;
	treasureLoader.loadDefaults();

	// load monsters
	Std::vector<Std::string> monsterkeys;
	Std::string category = "monsters";
	monsterkeys = config->listSections(category);
	for (const auto &section : monsterkeys) {
		MonsterInfo *mi = new MonsterInfo;

		int val;

		config->get(category, section, "shape", val);
		mi->_shape = static_cast<uint32>(val);

		config->get(category, section, "hp_min", val);
		mi->_minHp = static_cast<uint16>(val);

		config->get(category, section, "hp_max", val);
		mi->_maxHp = static_cast<uint16>(val);

		config->get(category, section, "dex_min", val);
		mi->_minDex = static_cast<uint16>(val);

		config->get(category, section, "dex_max", val);
		mi->_maxDex = static_cast<uint16>(val);

		config->get(category, section, "damage_min", val);
		mi->_minDmg = static_cast<uint16>(val);

		config->get(category, section, "damage_max", val);
		mi->_maxDmg = static_cast<uint16>(val);

		config->get(category, section, "armour", val);
		mi->_armourClass = static_cast<uint16>(val);

		config->get(category, section, "alignment", val);
		mi->_alignment = static_cast<uint8>(val);

		config->get(category, section, "unk", val);
		mi->_unk = (val != 0);

		config->get(category, section, "damage_type", val);
		mi->_damageType = static_cast<uint16>(val);

		config->get(category, section, "defense_type", val);
		mi->_defenseType = static_cast<uint16>(val);

		if (config->get(category, section, "resurrection", val))
			mi->_resurrection = (val != 0);
		else
			mi->_resurrection = false;

		if (config->get(category, section, "ranged", val))
			mi->_ranged = (val != 0);
		else
			mi->_ranged = false;

		if (config->get(category, section, "shifter", val))
			mi->_shifter = (val != 0);
		else
			mi->_shifter = false;

		if (config->get(category, section, "explode", val))
			mi->_explode = val;
		else
			mi->_explode = 0;

		Std::string treasure;
		if (config->get(category, section, "treasure", treasure)) {
			bool ok = treasureLoader.parse(treasure, mi->_treasure);
			if (!ok) {
				warning("failed to parse treasure info for monster '%s;", section.c_str());
				mi->_treasure.clear();
			}
		} else {
			mi->_treasure.clear();
		}

		assert(mi->_shape < _shapeInfo.size());
		_shapeInfo[mi->_shape]._monsterInfo = mi;
	}
}

void TypeFlags::loadDamageDat(Common::SeekableReadStream *rs) {
	uint32 count = rs->size() / 6;
	if (_shapeInfo.size() < count) {
		warning("more damage info than shape info");
		return;
	}
	for (uint32 i = 0; i < count; i++) {
		byte damagedata[6];
		rs->read(damagedata, 6);
		if (damagedata[0] == 0)
			continue;

		if (GAME_IS_REGRET && damagedata[0] == 1 && !damagedata[1] &&
			!damagedata[2] && !damagedata[3] &&
			!damagedata[4] && !damagedata[5]) {
			// WORKAROUND: No Regret has 3 shapes with this data pattern
			// which doesn't seem to be correct - eg, the elevator buttons
			// can be destroyed by gunshots which breaks the game.
			// Just ignore this pattern.
			// In No Remorse these maybe this pattern should be ignored too,
			// but there on some shapes that don't break the game.
			debug("Ignoring weird damage dat, shape %d (1 flag and rest 0s)", i);
			continue;
		}

		DamageInfo *di = new DamageInfo(damagedata);
		_shapeInfo[i]._damageInfo = di;
	}
}


} // End of namespace Ultima8
} // End of namespace Ultima
