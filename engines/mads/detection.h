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
 *
 */

#ifndef MADS_DETECTION_H
#define MADS_DETECTION_H

#include "engines/advancedDetector.h"

namespace MADS {

enum {
	GType_RexNebular = 0,
	GType_Dragonsphere = 1,
	GType_Phantom = 2,
	GType_Forest = 3
};

enum {
	GF_INSTALLER = 1
};

struct MADSGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

#define GAMEOPTION_EASY_MOUSE          GUIO_GAMEOPTIONS1
#define GAMEOPTION_ANIMATED_INVENTORY  GUIO_GAMEOPTIONS2
#define GAMEOPTION_ANIMATED_INTERFACE  GUIO_GAMEOPTIONS3
#define GAMEOPTION_NAUGHTY_MODE        GUIO_GAMEOPTIONS4
//#define GAMEOPTION_GRAPHICS_DITHERING  GUIO_GAMEOPTIONS5

#ifdef USE_TTS
#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS5
#endif

} // End of namespace MADS

#endif // MADS_DETECTION_H
