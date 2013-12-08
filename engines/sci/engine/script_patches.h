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
 */

#ifndef SCI_ENGINE_SCRIPT_PATCHES_H
#define SCI_ENGINE_SCRIPT_PATCHES_H

#include "sci/sci.h"

namespace Sci {

#define SIG_END               0xFFFF
#define SIG_MISMATCH          0xFFFE
#define SIG_COMMANDMASK       0xF000
#define SIG_VALUEMASK         0x0FFF
#define SIG_BYTEMASK          0x00FF
#define SIG_MAGICDWORD        0xF000
#define SIG_ADDTOOFFSET       0xE000
#define SIG_SELECTOR16        0x9000
#define SIG_SELECTOR8         0x8000
#define SIG_UINT16            0x1000
#define SIG_BYTE              0x0000

#define PATCH_END                   SIG_END
#define PATCH_COMMANDMASK           SIG_COMMANDMASK
#define PATCH_VALUEMASK             SIG_VALUEMASK
#define PATCH_BYTEMASK              SIG_BYTEMASK
#define PATCH_ADDTOOFFSET           SIG_ADDTOOFFSET
#define PATCH_GETORIGINALBYTE       0xD000
#define PATCH_GETORIGINALBYTEADJUST 0xC000
#define PATCH_SELECTOR16            SIG_SELECTOR16
#define PATCH_SELECTOR8             SIG_SELECTOR8
#define PATCH_UINT16                SIG_UINT16
#define PATCH_BYTE                  SIG_BYTE

// defines maximum scratch area for getting original bytes from unpatched script data
#define PATCH_VALUELIMIT      4096

struct SciScriptPatcherEntry {
	bool defaultActive;
//	bool active;
	uint16 scriptNr;
	const char *description;
	int16 applyCount;
//	uint32 magicDWord;
//	int magicOffset;
	const uint16 *signatureData;
	const uint16 *patchData;
};

//#define SCI_SIGNATUREENTRY_TERMINATOR { false, 0, NULL, 0, 0, 0, NULL, NULL }
#define SCI_SIGNATUREENTRY_TERMINATOR { false, 0, NULL, 0, NULL, NULL }

struct SciScriptPatcherRuntimeEntry {
	bool active;
	uint32 magicDWord;
	int magicOffset;
};

/**
 * ScriptPatcher class, handles on-the-fly patching of script data
 */
class ScriptPatcher {
public:
	ScriptPatcher();
	~ScriptPatcher();

	void processScript(uint16 scriptNr, byte *scriptData, const uint32 scriptSize);

private:
	void initSignature(const SciScriptPatcherEntry *patchTable, bool isMacSci11);
	void enablePatch(const SciScriptPatcherEntry *patchTable, const char *searchDescription);
	int32 findSignature(const SciScriptPatcherEntry *patchEntry, SciScriptPatcherRuntimeEntry *runtimeEntry, const byte *scriptData, const uint32 scriptSize, bool isMacSci11);
	void applyPatch(const SciScriptPatcherEntry *patchEntry, byte *scriptData, const uint32 scriptSize, int32 signatureOffset, bool isMacSci11);
	
	Selector *_selectorIdTable;
	SciScriptPatcherRuntimeEntry *_runtimeTable;
};

} // End of namespace Sci

#endif // SCI_ENGINE_WORKAROUNDS_H
