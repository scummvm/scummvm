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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SHARED_H
#define ASYLUM_SHARED_H

namespace Asylum {

enum GameFlag {
	kGameFlag4 = 4,
	kGameFlag12 = 12,
	kGameFlagScriptProcessing     = 183,
	kGameFlagCommentLeavingCell   = 214,
	kGameFlag219                  = 219,
	kGameFlagSolveVCRBlowUpPuzzle = 220,
	kGameFlag279                  = 279,
	kGameFlagFinishGame           = 901
};

typedef int ResourceId;

enum ResourceIdEnum {
	kResourceSpeech_8000050A = 0x8000050A,
	kResourceMusic_80020000  = 0x80020000,
	kResourceSound_80030203  = 0x80030203,
	kResourceSound_80120001  = 0x80120001,
	kResourceSound_80120006  = 0x80120006,
	kResourceSoundIntro      = 0x80120007,
	kResourceMusic_FFFFFD66  = 0xFFFFFD66
};

enum ActionType {
	kActionTypeNone = 0,
	kActionTypeFind = 1,
	kActionTypeTalk = 2,
	kActionType3 = 3,
	kActionTypeGrab = 4,
	kActionType8 = 8,
	kActionType16 = 16
};

//////////////////////////////////////////////////////////////////////////
// Helpers
#define CLEAR_ARRAY(type, name) { \
	for (Common::Array<type *>::iterator it = name.begin(); it != name.end(); it++) {\
		delete *it; \
		*it = NULL; \
	} \
	name.clear(); \
}

} // End of namespace Asylum

#endif // ASYLUM_SHARED_H
