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

#ifndef SCI_ENGINE_WORKAROUNDS_H
#define SCI_ENGINE_WORKAROUNDS_H

#include "sci/engine/vm_types.h"
#include "sci/sci.h"

namespace Sci {

enum SciWorkaroundType {
	WORKAROUND_NONE,      // only used by terminator or when no workaround was found
	WORKAROUND_IGNORE,    // ignore kernel call
	WORKAROUND_STILLCALL, // still do kernel call
	WORKAROUND_FAKE       // fake kernel call / replace temp value / fake opcode
};

struct SciWorkaroundSolution {
	SciWorkaroundType type;
	uint16 value;
};

struct SciWorkaroundEntry {
	SciGameId gameId;
	int roomNr;
	int scriptNr;
	int16 inheritanceLevel;
	const char *objectName;
	const char *methodName;
	int localCallOffset;
	int index;
	SciWorkaroundSolution newValue;
};

#define SCI_WORKAROUNDENTRY_TERMINATOR { (SciGameId)0, -1, -1, 0, NULL, NULL, -1, 0, { WORKAROUND_NONE, 0 } }
#define FAKE WORKAROUND_FAKE

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,             workaround
extern const SciWorkaroundEntry opcodeDivWorkarounds[];
extern const SciWorkaroundEntry opcodeDptoaWorkarounds[];
extern const SciWorkaroundEntry uninitializedReadWorkarounds[];
extern const SciWorkaroundEntry kAbs_workarounds[];
extern const SciWorkaroundEntry kDisposeScript_workarounds[];
extern const SciWorkaroundEntry kDoSoundFade_workarounds[];
extern const SciWorkaroundEntry kGraphDrawLine_workarounds[];
extern const SciWorkaroundEntry kGraphRestoreBox_workarounds[];
extern const SciWorkaroundEntry kGraphFillBoxForeground_workarounds[];
extern const SciWorkaroundEntry kGraphFillBoxAny_workarounds[];
extern const SciWorkaroundEntry kSetPort_workarounds[];
extern const SciWorkaroundEntry kUnLoad_workarounds[];
extern const SciWorkaroundEntry kStrCpy_workarounds[];

#undef FAKE

} // End of namespace Sci

#endif // SCI_ENGINE_WORKAROUNDS_H
