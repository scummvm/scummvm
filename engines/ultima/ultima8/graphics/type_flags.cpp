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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/type_flags.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/games/treasure_loader.h"
#include "ultima/ultima8/games/game_info.h"

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


void TypeFlags::load(IDataSource *ds) {
	// TODO: detect U8/crusader format somehow?!
	// (Or probably pass it as parameter)
	// The 'parsing' below is only for U8

	unsigned int blocksize = 8;
	if (GAME_IS_CRUSADER) {
		blocksize = 9;
	}

	uint32 size = ds->getSize();
	uint32 count = size / blocksize;

	_shapeInfo.clear();
	_shapeInfo.resize(count);

	for (uint32 i = 0; i < count; ++i) {
		uint8 data[9];
		ds->read(data, blocksize);

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

			si._unknown = data[5] & 0x0F;
			if (data[5] & 0x10) si._flags |= ShapeInfo::SI_EDITOR;
			if (data[5] & 0x20) si._flags |= ShapeInfo::SI_EXPLODE;
			if (data[5] & 0x40) si._flags |= ShapeInfo::SI_UNKNOWN46;
			if (data[5] & 0x80) si._flags |= ShapeInfo::SI_UNKNOWN47;

			si._weight = data[6];

			si._volume = data[7];

		} else if (GAME_IS_CRUSADER) {

			// might have to split up remorse/regret at some point

			// unchecked
			if (data[0] & 0x01) si._flags |= ShapeInfo::SI_FIXED;
			if (data[0] & 0x02) si._flags |= ShapeInfo::SI_SOLID;
			if (data[0] & 0x04) si._flags |= ShapeInfo::SI_SEA;
			if (data[0] & 0x08) si._flags |= ShapeInfo::SI_LAND;
			if (data[0] & 0x10) si._flags |= ShapeInfo::SI_OCCL;
			if (data[0] & 0x20) si._flags |= ShapeInfo::SI_BAG;
			if (data[0] & 0x40) si._flags |= ShapeInfo::SI_DAMAGING;
			if (data[0] & 0x80) si._flags |= ShapeInfo::SI_NOISY;

			// unchecked
			if (data[1] & 0x01) si._flags |= ShapeInfo::SI_DRAW;
			if (data[1] & 0x02) si._flags |= ShapeInfo::SI_IGNORE;
			if (data[1] & 0x04) si._flags |= ShapeInfo::SI_ROOF;
			if (data[1] & 0x08) si._flags |= ShapeInfo::SI_TRANSL;
			si._family = data[1] >> 4;
			si._family += (data[2] & 1) << 4;

			// (copied from old/viewer/ShapeManager.h)
			si._x = ((data[3] << 3) | (data[2] >> 5)) & 0x1F;
			si._y = (data[3] >> 2) & 0x1F;
			si._z = ((data[4] << 1) | (data[3] >> 7)) & 0x1F;

			if (data[6] & 0x01) si._flags |= ShapeInfo::SI_EDITOR;
			if (data[6] & 0x02) si._flags |= ShapeInfo::SI_CRUSUNK61;
			if (data[6] & 0x04) si._flags |= ShapeInfo::SI_CRUSUNK62;
			if (data[6] & 0x08) si._flags |= ShapeInfo::SI_CRUSUNK63;
			if (data[6] & 0x10) si._flags |= ShapeInfo::SI_CRUSUNK64;
			if (data[6] & 0x20) si._flags |= ShapeInfo::SI_CRUS_NPC;
			if (data[6] & 0x40) si._flags |= ShapeInfo::SI_CRUSUNK66;
			if (data[6] & 0x80) si._flags |= ShapeInfo::SI_CRUSUNK67;

			si._animType = 0;

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
	Std::vector<istring> weaponkeys;
	weaponkeys = config->listSections("weapons", true);
	for (Std::vector<istring>::const_iterator iter = weaponkeys.begin();
	        iter != weaponkeys.end(); ++iter) {
		const istring &k = *iter;
		WeaponInfo *wi = new WeaponInfo;

		int val;

		config->get(k + "/shape", val);
		wi->_shape = static_cast<uint32>(val);

		config->get(k + "/overlay", val);
		wi->_overlayType = static_cast<uint8>(val);

		config->get(k + "/overlay_shape", val);
		wi->_overlayShape = static_cast<uint32>(val);

		config->get(k + "/damage_mod", val);
		wi->_damageModifier = static_cast<uint8>(val);

		config->get(k + "/base_damage", val);
		wi->_baseDamage = static_cast<uint8>(val);

		config->get(k + "/attack_dex", val);
		wi->_dexAttackBonus = static_cast<uint8>(val);

		config->get(k + "/defend_dex", val);
		wi->_dexDefendBonus = static_cast<uint8>(val);

		config->get(k + "/armour", val);
		wi->_armourBonus = static_cast<uint8>(val);

		config->get(k + "/damage_type", val);
		wi->_damageType = static_cast<uint16>(val);

		if (config->get(k + "/treasure_chance", val))
			wi->_treasureChance = static_cast<uint16>(val);
		else
			wi->_treasureChance = 0;

		assert(wi->_shape < _shapeInfo.size());
		_shapeInfo[wi->_shape]._weaponInfo = wi;
	}
}


void TypeFlags::loadArmourInfo() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	MainShapeArchive *msf = GameData::get_instance()->getMainShapes();

	// load armour
	Std::vector<istring> armourkeys;
	armourkeys = config->listSections("armour", true);
	for (Std::vector<istring>::const_iterator iter = armourkeys.begin();
	        iter != armourkeys.end(); ++iter) {
		const istring &k = *iter;
		ArmourInfo ai;

		int val;

		config->get(k + "/shape", val);
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

		config->get(k + "/frame", val);
		ai._frame = static_cast<uint32>(val);

		assert(ai._frame < framecount);

		config->get(k + "/armour", val);
		ai._armourClass = static_cast<uint16>(val);

		if (config->get(k + "/type", val))
			ai._defenseType = static_cast<uint16>(val);
		else
			ai._defenseType = 0;

		if (config->get(k + "/kick_bonus", val))
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
	Std::vector<istring> monsterkeys;
	monsterkeys = config->listSections("monsters", true);
	for (Std::vector<istring>::const_iterator iter = monsterkeys.begin();
	        iter != monsterkeys.end(); ++iter) {
		const istring k = *iter;
		MonsterInfo *mi = new MonsterInfo;

		int val;

		config->get(k + "/shape", val);
		mi->_shape = static_cast<uint32>(val);

		config->get(k + "/hp_min", val);
		mi->_minHp = static_cast<uint16>(val);

		config->get(k + "/hp_max", val);
		mi->_maxHp = static_cast<uint16>(val);

		config->get(k + "/dex_min", val);
		mi->_minDex = static_cast<uint16>(val);

		config->get(k + "/dex_max", val);
		mi->_maxDex = static_cast<uint16>(val);

		config->get(k + "/damage_min", val);
		mi->_minDmg = static_cast<uint16>(val);

		config->get(k + "/damage_max", val);
		mi->_maxDmg = static_cast<uint16>(val);

		config->get(k + "/armour", val);
		mi->_armourClass = static_cast<uint16>(val);

		config->get(k + "/alignment", val);
		mi->_alignment = static_cast<uint8>(val);

		config->get(k + "/unk", val);
		mi->_unk = (val != 0);

		config->get(k + "/damage_type", val);
		mi->_damageType = static_cast<uint16>(val);

		config->get(k + "/defense_type", val);
		mi->_defenseType = static_cast<uint16>(val);

		if (config->get(k + "/resurrection", val))
			mi->_resurrection = (val != 0);
		else
			mi->_resurrection = false;

		if (config->get(k + "/ranged", val))
			mi->_ranged = (val != 0);
		else
			mi->_ranged = false;

		if (config->get(k + "/shifter", val))
			mi->_shifter = (val != 0);
		else
			mi->_shifter = false;

		if (config->get(k + "/explode", val))
			mi->_explode = val;
		else
			mi->_explode = 0;

		Std::string treasure;
		if (config->get(k + "/treasure", treasure)) {
			bool ok = treasureLoader.parse(treasure, mi->_treasure);
			if (!ok) {
				perr << "failed to parse treasure info for monster '" << k
				     << "'"  << Std::endl;
				mi->_treasure.clear();
			}
		} else {
			mi->_treasure.clear();
		}

		assert(mi->_shape < _shapeInfo.size());
		_shapeInfo[mi->_shape]._monsterInfo = mi;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
