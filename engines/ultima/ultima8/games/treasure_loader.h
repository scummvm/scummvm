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

#ifndef ULTIMA8_GAMES_TREASURELOADER_H
#define ULTIMA8_GAMES_TREASURELOADER_H

#include "ultima/ultima8/world/actors/treasure_info.h"
#include "ultima/ultima8/misc/istring.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

typedef Common::HashMap<istring, TreasureInfo, Common::IgnoreCase_Hash> TreasureMap;

class TreasureLoader {
public:
	TreasureLoader();
	~TreasureLoader();

	//! load defaults from 'game' ini section
	void loadDefaults();

	//! parse treasure string into vector of TreasureInfo objects
	bool parse(const Std::string &, Std::vector<TreasureInfo> &treasure) const;

private:
	TreasureMap _defaultTreasure;

	bool internalParse(const Std::string &desc, TreasureInfo &ti, bool loadingDefault) const;

	bool parseUInt32Vector(const Std::string &val, Std::vector<uint32> &vec) const;
	bool parseUIntRange(const Std::string &val, unsigned int &min, unsigned int &max) const;
	bool parseDouble(const Std::string &val, double &d) const;
	bool parseInt(const Std::string &val, int &i) const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
