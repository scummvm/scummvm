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

#ifndef MM_DETECTION_H
#define MM_DETECTION_H

#include "engines/advancedDetector.h"

namespace MM {

enum {
	GType_MightAndMagic1,
	GType_Clouds,
	GType_DarkSide,
	GType_WorldOfXeen,
	GType_Swords
};

enum GameFeature {
	GF_NONE = 0,
	GF_ENHANCED = 1,
	GF_GFX_PACK = 2
};

struct MightAndMagicGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

#define GAMEOPTION_SHOW_ITEM_COSTS	GUIO_GAMEOPTIONS1
#define GAMEOPTION_DURABLE_ARMOR	GUIO_GAMEOPTIONS2
#define GAMEOPTION_SHOW_HP_SP_BARS	GUIO_GAMEOPTIONS3

} // namespace MM

#endif // MM_DETECTION_H
