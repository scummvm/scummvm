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

#ifndef XEEN_DETECTION_H
#define XEEN_DETECTION_H

#include "engines/advancedDetector.h"

namespace Xeen {

enum {
	GType_Clouds = 1,
	GType_DarkSide = 2,
	GType_WorldOfXeen = 3,
	GType_Swords = 4
};

struct XeenGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

#define GAMEOPTION_SHOW_ITEM_COSTS	GUIO_GAMEOPTIONS1
#define GAMEOPTION_DURABLE_ARMOR	GUIO_GAMEOPTIONS2

} // End of namespace Xeen

#endif // XEEN_DETECTION_H
