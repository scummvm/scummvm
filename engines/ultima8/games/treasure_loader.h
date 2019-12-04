/*
Copyright (C) 2004 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_GAMES_TREASURELOADER_H
#define ULTIMA8_GAMES_TREASURELOADER_H

#include "ultima8/world/actors/treasure_info.h"
#include "ultima8/std/containers.h"
#include "ultima8/std/containers.h"

namespace Ultima8 {

class TreasureLoader {
public:
	TreasureLoader();
	~TreasureLoader();

	//! load defaults from 'game' ini section
	void loadDefaults();

	//! parse treasure string into vector of TreasureInfo objects
	bool parse(std::string, std::vector<TreasureInfo> &treasure);

private:
	std::map<Pentagram::istring, TreasureInfo, Common::IgnoreCase_Hash> defaultTreasure;

	bool internalParse(std::string desc, TreasureInfo &ti, bool loadingDefault);

	bool parseUInt32Vector(std::string val, std::vector<uint32> &vec);
	bool parseUIntRange(std::string val, unsigned int &min, unsigned int &max);
	bool parseDouble(std::string val, double &d);
	bool parseInt(std::string val, int &i);
};

} // End of namespace Ultima8

#endif
