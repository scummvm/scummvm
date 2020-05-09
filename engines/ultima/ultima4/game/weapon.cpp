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

#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/names.h"

namespace Ultima {
namespace Ultima4 {

Weapons *g_weapons;

Weapons::Weapons() : _confLoaded(false) {
	g_weapons = this;
}

Weapons::~Weapons() {
	for (uint idx = 0; idx < size(); ++idx)
		delete (*this)[idx];

	g_weapons = nullptr;
}

const Weapon *Weapons::get(WeaponType w) {
	// Load in XML if it hasn't been already
	loadConf();

	if (static_cast<unsigned>(w) >= size())
		return nullptr;
	return (*this)[w];
}

const Weapon *Weapons::get(const Common::String &name) {
	// Load in XML if it hasn't been already
	loadConf();

	for (uint i = 0; i < size(); i++) {
		if (scumm_stricmp(name.c_str(), (*this)[i]->_name.c_str()) == 0)
			return (*this)[i];
	}

	return nullptr;
}

void Weapons::loadConf() {
	if (_confLoaded)
		return;

	_confLoaded = true;
	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> weaponConfs = config->getElement("weapons").getChildren();
	for (Std::vector<ConfigElement>::iterator i = weaponConfs.begin(); i != weaponConfs.end(); i++) {
		if (i->getName() != "weapon")
			continue;

		WeaponType weaponType = static_cast<WeaponType>(size());
		push_back(new Weapon(weaponType, *i));
	}
}

/*-------------------------------------------------------------------*/

Weapon::Weapon(WeaponType weaponType, const ConfigElement &conf)
	: _type(weaponType)
	, _name(conf.getString("name"))
	, _abbr(conf.getString("abbr"))
	, _canUse(0xFF)
	, _range(0)
	, _damage(conf.getInt("damage"))
	, _hitTile("hit_flash")
	, _missTile("miss_flash")
	, _leaveTile("")
	, _flags(0) {
	static const struct {
		const char *name;
		uint flag;
	} booleanAttributes[] = {
		{ "lose", WEAP_LOSE },
		{ "losewhenranged", WEAP_LOSEWHENRANGED },
		{ "choosedistance", WEAP_CHOOSEDISTANCE },
		{ "alwayshits", WEAP_ALWAYSHITS },
		{ "magic", WEAP_MAGIC },
		{ "attackthroughobjects", WEAP_ATTACKTHROUGHOBJECTS },
		{ "returns", WEAP_RETURNS },
		{ "dontshowtravel", WEAP_DONTSHOWTRAVEL }
	};

	/* Get the range of the weapon, whether it is absolute or normal range */
	Common::String range = conf.getString("range");
	if (range.empty()) {
		range = conf.getString("absolute_range");
		if (!range.empty())
			_flags |= WEAP_ABSOLUTERANGE;
	}
	if (range.empty())
		error("malformed weapons.xml file: range or absolute_range not found for weapon %s", _name.c_str());

	_range = atoi(range.c_str());

	/* Load weapon attributes */
	for (unsigned at = 0; at < sizeof(booleanAttributes) / sizeof(booleanAttributes[0]); at++) {
		if (conf.getBool(booleanAttributes[at].name)) {
			_flags |= booleanAttributes[at].flag;
		}
	}

	/* Load hit tiles */
	if (conf.exists("hittile"))
		_hitTile = conf.getString("hittile");

	/* Load miss tiles */
	if (conf.exists("misstile"))
		_missTile = conf.getString("misstile");

	/* Load leave tiles */
	if (conf.exists("leavetile")) {
		_leaveTile = conf.getString("leavetile");
	}

	Std::vector<ConfigElement> contraintConfs = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = contraintConfs.begin(); i != contraintConfs.end(); i++) {
		byte mask = 0;

		if (i->getName() != "constraint")
			continue;

		for (int cl = 0; cl < 8; cl++) {
			if (scumm_stricmp(i->getString("class").c_str(), getClassName(static_cast<ClassType>(cl))) == 0)
				mask = (1 << cl);
		}
		if (mask == 0 && scumm_stricmp(i->getString("class").c_str(), "all") == 0)
			mask = 0xFF;
		if (mask == 0) {
			error("malformed weapons.xml file: constraint has unknown class %s",
			           i->getString("class").c_str());
		}
		if (i->getBool("canuse"))
			_canUse |= mask;
		else
			_canUse &= ~mask;
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
