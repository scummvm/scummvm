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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */


#include "crab/people/personbase.h"

namespace Crab {

using namespace pyrodactyl::stat;

namespace pyrodactyl {
namespace people {
PersonType StringToPersonType(const Common::String &val) {
	if (val == "neutral")
		return PE_NEUTRAL;
	else if (val == "hostile")
		return PE_HOSTILE;
	else if (val == "coward")
		return PE_COWARD;
	else if (val == "immobile")
		return PE_IMMOBILE;

	return PE_NEUTRAL;
}

PersonState StringToPersonState(const Common::String &val) {
	if (val == "ko")
		return PST_KO;
	else if (val == "fight")
		return PST_FIGHT;
	else if (val == "flee")
		return PST_FLEE;
	else if (val == "dying")
		return PST_DYING;

	return PST_NORMAL;
}
} // End of namespace people
} // End of namespace pyrodactyl

} // End of namespace Crab
