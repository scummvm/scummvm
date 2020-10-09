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

#ifndef AGI_DETECTION_H
#define AGI_DETECTION_H

#include "engines/advancedDetector.h"

namespace Agi {

enum AgiGameType {
	GType_PreAGI = 0,
	GType_V1 = 1,
	GType_V2 = 2,
	GType_V3 = 3
};

enum AgiGameID {
	GID_AGIDEMO,
	GID_BC,
	GID_DDP,
	GID_GOLDRUSH,
	GID_KQ1,
	GID_KQ2,
	GID_KQ3,
	GID_KQ4,
	GID_LSL1,
	GID_MH1,
	GID_MH2,
	GID_MIXEDUP,
	GID_PQ1,
	GID_SQ1,
	GID_SQ2,
	GID_XMASCARD,
	GID_FANMADE,
	GID_GETOUTTASQ, // Fanmade
	GID_MICKEY,     // PreAGI
	GID_WINNIE,     // PreAGI
	GID_TROLL       // PreAGI
};

//
// GF_OLDAMIGAV20 means that the interpreter is an old Amiga AGI interpreter that
// uses value 20 for the computer type (v20 i.e. vComputer) rather than the usual value 5.
//
enum AgiGameFeatures {
	GF_AGIMOUSE    = (1 << 0), // this disables "Click-to-walk mouse interface"
	GF_AGDS        = (1 << 1),
	GF_AGI256      = (1 << 2), // marks fanmade AGI-256 games
	GF_MACGOLDRUSH = (1 << 3), // use "grdir" instead of "dir" for volume loading
	GF_FANMADE     = (1 << 4), // marks fanmade games
	GF_OLDAMIGAV20 = (1 << 5),
	GF_2GSOLDSOUND = (1 << 6)
};

enum BooterDisks {
	BooterDisk1 = 0,
	BooterDisk2 = 1
};

struct AGIGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

} // End of namespace Agi

#endif // AGI_DETECTION_H
