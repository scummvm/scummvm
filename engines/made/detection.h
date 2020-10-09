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

#ifndef MADE_DETECTION_H
#define MADE_DETECTION_H

#include "engines/advancedDetector.h"

namespace Made {

enum MadeGameID {
	GID_RTZ		= 0,
	GID_MANHOLE	= 1,
	GID_LGOP2	= 2,
	GID_RODNEY	= 3
};

enum MadeGameFeatures {
	GF_DEMO				= 1 << 0,
	GF_CD				= 1 << 1,
	GF_CD_COMPRESSED	= 1 << 2,
	GF_FLOPPY			= 1 << 3
};

struct MadeGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

} // End of namespace Made

#endif // MADE_DETECTION_H
