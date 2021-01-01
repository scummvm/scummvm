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

#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/map/tile.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima4 {

Armors *g_armors;

Armors::Armors() : _confLoaded(false) {
	g_armors = this;
}

Armors::~Armors() {
	g_armors = nullptr;
}

const Armor *Armors::get(ArmorType a) {
	// Load in XML if it hasn't been already
	loadConf();

	if (static_cast<unsigned>(a) >= size())
		return nullptr;
	return (*this)[a];
}

const Armor *Armors::get(const Common::String &name) {
	// Load in XML if it hasn't been already
	loadConf();

	for (unsigned i = 0; i < size(); i++) {
		if (scumm_stricmp(name.c_str(), (*this)[i]->_name.c_str()) == 0)
			return (*this)[i];
	}
	return nullptr;
}


void Armors::loadConf() {
	if (!_confLoaded)
		_confLoaded = true;
	else
		return;

	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> armorConfs = config->getElement("armors").getChildren();
	for (Std::vector<ConfigElement>::iterator i = armorConfs.begin(); i != armorConfs.end(); i++) {
		if (i->getName() != "armor")
			continue;

		ArmorType armorType = static_cast<ArmorType>(size());
		push_back(new Armor(armorType, *i));
	}
}

/*-------------------------------------------------------------------*/

Armor::Armor(ArmorType armorType, const ConfigElement &conf) :
		_type(armorType), _canUse(0xff) /*, _mask(0) */ {
	_name = conf.getString("name");
	_defense = conf.getInt("defense");

	Std::vector<ConfigElement> contraintConfs = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = contraintConfs.begin(); i != contraintConfs.end(); i++) {
		byte useMask = 0;

		if (i->getName() != "constraint")
			continue;

		for (int cl = 0; cl < 8; cl++) {
			if (scumm_stricmp(i->getString("class").c_str(), getClassName(static_cast<ClassType>(cl))) == 0)
				useMask = (1 << cl);
		}
		if (useMask == 0 && scumm_stricmp(i->getString("class").c_str(), "all") == 0)
			useMask = 0xFF;
		if (useMask == 0) {
			error("malformed armor.xml file: constraint has unknown class %s",
			           i->getString("class").c_str());
		}
		if (i->getBool("canuse"))
			_canUse |= useMask;
		else
			_canUse &= ~useMask;
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
