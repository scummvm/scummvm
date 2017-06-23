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

#ifndef SCI_ENGINE_SCRIPT_PATCHES_H
#define SCI_ENGINE_SCRIPT_PATCHES_H

#include "sci/sci.h"

namespace Sci {

// Please do not use the #defines, that are called SIG_CODE_* / PATCH_CODE_* inside signature/patch-tables
#define SIG_END                      0xFFFF
#define SIG_MISMATCH                 0xFFFE
#define SIG_COMMANDMASK              0xF000
#define SIG_VALUEMASK                0x0FFF
#define SIG_BYTEMASK                 0x00FF
#define SIG_MAGICDWORD               0xF000
#define SIG_CODE_ADDTOOFFSET         0xE000
#define SIG_ADDTOOFFSET(_offset_)    SIG_CODE_ADDTOOFFSET | (_offset_)
#define SIG_CODE_SELECTOR16          0x9000
#define SIG_SELECTOR16(_selectorID_) SIG_CODE_SELECTOR16 | SELECTOR_##_selectorID_
#define SIG_CODE_SELECTOR8           0x8000
#define SIG_SELECTOR8(_selectorID_)  SIG_CODE_SELECTOR8 | SELECTOR_##_selectorID_
#define SIG_CODE_UINT16              0x1000
#define SIG_UINT16(_value_)          SIG_CODE_UINT16 | ((_value_) & 0xFF), ((_value_) >> 8)
#define SIG_CODE_BYTE                0x0000

#define PATCH_END                                              SIG_END
#define PATCH_COMMANDMASK                                      SIG_COMMANDMASK
#define PATCH_VALUEMASK                                        SIG_VALUEMASK
#define PATCH_BYTEMASK                                         SIG_BYTEMASK
#define PATCH_CODE_ADDTOOFFSET                                 SIG_CODE_ADDTOOFFSET
#define PATCH_ADDTOOFFSET(_offset_)                            SIG_CODE_ADDTOOFFSET | (_offset_)
#define PATCH_CODE_GETORIGINALBYTE                             0xC000
#define PATCH_GETORIGINALBYTE(_offset_)                        PATCH_CODE_GETORIGINALBYTE | (_offset_), 0
#define PATCH_GETORIGINALBYTEADJUST(_offset_, _adjustValue_)   PATCH_CODE_GETORIGINALBYTE | (_offset_), (uint16)(_adjustValue_)
#define PATCH_CODE_GETORIGINALUINT16                           0xD000
#define PATCH_GETORIGINALUINT16(_offset_)                      PATCH_CODE_GETORIGINALUINT16 | (_offset_), 0
#define PATCH_GETORIGINALUINT16ADJUST(_offset_, _adjustValue_) PATCH_CODE_GETORIGINALUINT16 | (_offset_), (uint16)(_adjustValue_)
#define PATCH_CODE_SELECTOR16                                  SIG_CODE_SELECTOR16
#define PATCH_SELECTOR16(_selectorID_)                         SIG_CODE_SELECTOR16 | SELECTOR_##_selectorID_
#define PATCH_CODE_SELECTOR8                                   SIG_CODE_SELECTOR8
#define PATCH_SELECTOR8(_selectorID_)                          SIG_CODE_SELECTOR8 | SELECTOR_##_selectorID_
#define PATCH_CODE_UINT16                                      SIG_CODE_UINT16
#define PATCH_UINT16(_value_)                                  SIG_CODE_UINT16 | ((_value_) & 0xFF), ((_value_) >> 8)
#define PATCH_CODE_BYTE                                        SIG_CODE_BYTE

// defines maximum scratch area for getting original bytes from unpatched script data
#define PATCH_VALUELIMIT      4096

struct SciScriptPatcherEntry {
	bool defaultActive;
	uint16 scriptNr;
	const char *description;
	int16 applyCount;
	const uint16 *signatureData;
	const uint16 *patchData;
};

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

	// Calculates the magic DWord for fast search and verifies signature/patch data
	// Returns the magic DWord in platform-specific byte-order. This is done on purpose for performance.
	void calculateMagicDWordAndVerify(const char *signatureDescription, const uint16 *signatureData, bool magicDWordIncluded, uint32 &calculatedMagicDWord, int &calculatedMagicDWordOffset);

	// Called when a script is loaded to check for signature matches and apply patches in such cases
	void processScript(uint16 scriptNr, SciSpan<byte> scriptData);

	// Verifies, if a given signature matches the given script data (pointed to by additional byte offset)
	bool verifySignature(uint32 byteOffset, const uint16 *signatureData, const char *signatureDescription, const SciSpan<const byte> &scriptData);

	// searches for a given signature inside script data
	// returns -1 in case it was not found or an offset to the matching data
	int32 findSignature(uint32 magicDWord, int magicOffset, const uint16 *signatureData, const char *patchDescription, const SciSpan<const byte> &scriptData);

private:
	// Initializes a patch table and creates run time information for it (for enabling/disabling), also calculates magic DWORD)
	void initSignature(const SciScriptPatcherEntry *patchTable);

	// Enables a patch inside the patch table (used for optional patches like CD+Text support for KQ6 & LB2)
	void enablePatch(const SciScriptPatcherEntry *patchTable, const char *searchDescription);

	// Searches for a given signature entry inside script data
	// returns -1 in case it was not found or an offset to the matching data
	int32 findSignature(const SciScriptPatcherEntry *patchEntry, const SciScriptPatcherRuntimeEntry *runtimeEntry, const SciSpan<const byte> &scriptData);

	// Applies a patch to a given script + offset (overwrites parts)
	void applyPatch(const SciScriptPatcherEntry *patchEntry, SciSpan<byte> scriptData, int32 signatureOffset);

	Selector *_selectorIdTable;
	SciScriptPatcherRuntimeEntry *_runtimeTable;
	bool _isMacSci11;
};

} // End of namespace Sci

#endif // SCI_ENGINE_WORKAROUNDS_H
