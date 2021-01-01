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

#ifndef ULTIMA4_GAME_ARMOR_H
#define ULTIMA4_GAME_ARMOR_H

#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Armors;

class Armor {
	friend class Armors;
public:
	// Getters
	ArmorType getType() const {
		return _type;      /**< Returns the ArmorType of the armor */
	}
	const Common::String &getName() const {
		return _name;      /**< Returns the name of the armor */
	}
	int getDefense() const {
		return _defense;   /**< Returns the defense value of the armor */
	}
	/** Returns true if the class given can wear the armor */
	bool canWear(ClassType klass) const {
		return _canUse & (1 << klass);
	}

private:
	Armor(ArmorType armorType, const ConfigElement &conf);

	ArmorType _type;
	Common::String _name;
	byte _canUse;
	int _defense;
	//unsigned short _mask;
};

class Armors : public Std::vector<Armor *> {
private:
	void loadConf();
	bool _confLoaded;
public:
	/**
	 * Constructor
	 */
	Armors();

	/**
	 * Destructor
	 */
	~Armors();

	/**
	 * Returns armor by ArmorType.
	 */
	const Armor *get(ArmorType a);

	/**
	 * Returns armor that has the given name
	 */
	const Armor *get(const Common::String &name);
};

extern Armors *g_armors;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
