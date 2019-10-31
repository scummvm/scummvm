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

#ifndef SCI_ENGINE_WORKAROUNDS_H
#define SCI_ENGINE_WORKAROUNDS_H

#include "sci/engine/vm_types.h"
#include "sci/engine/state.h"
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

/**
 * A structure describing a 'workaround' for a SCI script bug.
 *
 * Arrays of SciWorkaroundEntry instances are terminated by
 * a fake entry in which "objectName" is NULL.
 */
struct SciWorkaroundEntry {
	SciGameId gameId;
	int roomNr;
	int scriptNr;
	int16 inheritanceLevel;
	const char *objectName;
	const char *methodName;
	const uint16 *localCallSignature;
	int fromIndex;
	int toIndex;
	SciWorkaroundSolution newValue;
};

extern const SciWorkaroundEntry arithmeticWorkarounds[];
extern const SciWorkaroundEntry uninitializedReadWorkarounds[];
extern const SciWorkaroundEntry uninitializedReadForParamWorkarounds[];
extern const SciWorkaroundEntry kAbs_workarounds[];
extern const SciWorkaroundEntry kCelHigh_workarounds[];
extern const SciWorkaroundEntry kCelWide_workarounds[];
extern const SciWorkaroundEntry kDeviceInfo_workarounds[];
extern const SciWorkaroundEntry kDisplay_workarounds[];
extern const SciWorkaroundEntry kDirLoop_workarounds[];
extern const SciWorkaroundEntry kDisposeScript_workarounds[];
extern const SciWorkaroundEntry kDoAudioResume_workarounds[];
extern const SciWorkaroundEntry kDoSoundPlay_workarounds[];
extern const SciWorkaroundEntry kDoSoundFade_workarounds[];
extern const SciWorkaroundEntry kFileIOOpen_workarounds[];
extern const SciWorkaroundEntry kFileIOCheckFreeSpace_workarounds[];
extern const SciWorkaroundEntry kFileIOReadString_workarounds[];
extern const SciWorkaroundEntry kFindKey_workarounds[];
extern const SciWorkaroundEntry kFrameOut_workarounds[];
extern const SciWorkaroundEntry kDeleteKey_workarounds[];
extern const SciWorkaroundEntry kGetAngle_workarounds[];
extern const SciWorkaroundEntry kGetCWD_workarounds[];
extern const SciWorkaroundEntry kGraphDrawLine_workarounds[];
extern const SciWorkaroundEntry kGraphSaveBox_workarounds[];
extern const SciWorkaroundEntry kGraphRestoreBox_workarounds[];
extern const SciWorkaroundEntry kGraphUpdateBox_workarounds[];
extern const SciWorkaroundEntry kGraphFillBoxForeground_workarounds[];
extern const SciWorkaroundEntry kGraphFillBoxAny_workarounds[];
extern const SciWorkaroundEntry kGraphRedrawBox_workarounds[];
extern const SciWorkaroundEntry kIsObject_workarounds[];
extern const SciWorkaroundEntry kListAt_workarounds[];
extern const SciWorkaroundEntry kLock_workarounds[];
extern const SciWorkaroundEntry kMemory_workarounds[];
extern const SciWorkaroundEntry kMoveCursor_workarounds[];
extern const SciWorkaroundEntry kNewWindow_workarounds[];
extern const SciWorkaroundEntry kPalVarySetVary_workarounds[];
extern const SciWorkaroundEntry kPalVarySetPercent_workarounds[];
extern const SciWorkaroundEntry kPalVarySetStart_workarounds[];
extern const SciWorkaroundEntry kPalVaryMergeStart_workarounds[];
extern const SciWorkaroundEntry kPlatform32_workarounds[];
extern const SciWorkaroundEntry kRandom_workarounds[];
extern const SciWorkaroundEntry kReadNumber_workarounds[];
extern const SciWorkaroundEntry kResCheck_workarounds[];
extern const SciWorkaroundEntry kPaletteUnsetFlag_workarounds[];
extern const SciWorkaroundEntry kSetCursor_workarounds[];
extern const SciWorkaroundEntry kArraySetElements_workarounds[];
extern const SciWorkaroundEntry kArrayFill_workarounds[];
extern const SciWorkaroundEntry kSetPort_workarounds[];
extern const SciWorkaroundEntry kStrAt_workarounds[];
extern const SciWorkaroundEntry kStrCpy_workarounds[];
extern const SciWorkaroundEntry kStrLen_workarounds[];
extern const SciWorkaroundEntry kUnLoad_workarounds[];
extern const SciWorkaroundEntry kStringNew_workarounds[];
extern const SciWorkaroundEntry kScrollWindowAdd_workarounds[];

extern SciWorkaroundSolution trackOriginAndFindWorkaround(int index, const SciWorkaroundEntry *workaroundList, SciCallOrigin *trackOrigin);

enum SciMessageWorkaroundType {
	MSG_WORKAROUND_NONE,        // only used by terminator or when no workaround was found
	MSG_WORKAROUND_REMAP,       // use a different tuple instead
	MSG_WORKAROUND_FAKE,        // use a hard-coded response
	MSG_WORKAROUND_EXTRACT      // use text from a different record, optionally a substring
};

enum SciMedia {
	SCI_MEDIA_ALL,
	SCI_MEDIA_FLOPPY,
	SCI_MEDIA_CD
};

struct SciMessageWorkaroundSolution {
	SciMessageWorkaroundType type;
	int module;
	byte noun;
	byte verb;
	byte cond;
	byte seq;
	byte talker;
	uint32 substringIndex;
	uint32 substringLength;
	const char *text;
};

struct SciMessageWorkaroundEntry {
	SciGameId gameId;
	SciMedia media;
	kLanguage language;
	int roomNumber;
	int module;
	byte noun;
	byte verb;
	byte cond;
	byte seq;
	SciMessageWorkaroundSolution solution;
};

extern SciMessageWorkaroundSolution findMessageWorkaround(int module, byte noun, byte verb, byte cond, byte seq);
extern ResourceId remapAudio36ResourceId(const ResourceId &resourceId);
extern ResourceId remapSync36ResourceId(const ResourceId &resourceId);

} // End of namespace Sci

#endif // SCI_ENGINE_WORKAROUNDS_H
