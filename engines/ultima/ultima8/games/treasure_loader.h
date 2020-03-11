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

#ifndef ULTIMA8_GAMES_TREASURELOADER_H
#define ULTIMA8_GAMES_TREASURELOADER_H

#include "ultima/ultima8/world/actors/treasure_info.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

typedef Std::map<istring, TreasureInfo, Common::IgnoreCase_Hash> TreasureMap;

class TreasureLoader {
public:
	TreasureLoader();
	~TreasureLoader();

	//! load defaults from 'game' ini section
	void loadDefaults();

	//! parse treasure string into vector of TreasureInfo objects
	bool parse(const Std::string &, Std::vector<TreasureInfo> &treasure);

private:
	TreasureMap _defaultTreasure;

	bool internalParse(const Std::string &desc, TreasureInfo &ti, bool loadingDefault);

	bool parseUInt32Vector(const Std::string &val, Std::vector<uint32> &vec);
	bool parseUIntRange(const Std::string &val, unsigned int &min, unsigned int &max);
	bool parseDouble(const Std::string &val, double &d);
	bool parseInt(const Std::string &val, int &i);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
