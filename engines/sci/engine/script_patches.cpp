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

#include "sci/sci.h"
#include "sci/engine/script.h"

#include "common/util.h"

namespace Sci {

#define PATCH_END             0xFFFF
#define PATCH_ADDTOOFFSET     0x8000
#define PATCH_GETORIGINALBYTE 0x4000
#define PATCH_MAGICDWORD(a, b, c, d) CONSTANT_LE_32(a | (b << 8) | (c << 16) | (d << 24))

struct SciScriptSignature {
	uint16 scriptNr;
	const char *description;
	uint32 magicDWord;
	int magicOffset;
	const byte *data;
	const uint16 *patch;
};

// signatures are built like this:
//  - first a counter of the bytes that follow
//  - then the actual bytes that need to get matched
//  - then another counter of bytes (0 for EOS)
//  - if not EOS, an adjust offset and the actual bytes
//  - rinse and repeat


// daySixBeignet::changeState (4) is called when the cop goes out and sets cycles to 220.
//  this is not enough time to get to the door, so we patch that to 23 seconds
const byte gk1SignatureDay6PoliceBeignet[] = {
	4,
	0x35, 0x04,        // ldi 04
	0x1a,              // eq?
	0x30,              // bnt [next state check]
	+2, 5,             // [skip 2 bytes, offset of bnt]
	0x38, 0x93, 0x00,  // pushi 93 (selector dispose)
	0x76,              // push0
	0x72,              // lofsa deskSarg
	+2, 9,             // [skip 2 bytes, offset of lofsa]
	0x4a, 0x04, 0x00,  // send 04
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceBeignet[] = {
	PATCH_ADDTOOFFSET | +16,
	0x34, 0x17, 0x00,  // ldi 23
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// sargSleeping::changeState (8) is called when the cop falls asleep and sets cycles to 220.
//  this is not enough time to get to the door, so we patch it to 42 seconds
const byte gk1SignatureDay6PoliceSleep[] = {
	4,
	0x35, 0x08,        // ldi 08
	0x1a,              // eq?
	0x31,              // bnt [next state check]
	+1, 6,             // [skip 1 byte, offset of bnt]
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceSleep[] = {
	PATCH_ADDTOOFFSET | +5,
	0x34, 0x2a, 0x00,  // ldi 42
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// startOfDay5::changeState (20h) - when gabriel goes to the phone the script will hang
const byte gk1SignatureDay5PhoneFreeze[] = {
	5,
	0x35, 0x03,        // ldi 03
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	+2, 3,             // [skip 2 bytes, offset of jmp]
	0x3c,              // dup
	0x35, 0x21,        // ldi 21
	0
};

const uint16 gk1PatchDay5PhoneFreeze[] = {
	0x35, 0x06,        // ldi 06
	0x65, 0x20,        // aTop ticks
	PATCH_END
};

//    script, description,                                   magic DWORD,                                 adjust
const SciScriptSignature gk1Signatures[] = {
    {    212, "day 5 phone freeze",                          PATCH_MAGICDWORD(0x35, 0x03, 0x65, 0x1a),     0, gk1SignatureDay5PhoneFreeze, gk1PatchDay5PhoneFreeze },
    {    230, "day 6 police beignet timer issue",            PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),   -16, gk1SignatureDay6PoliceBeignet, gk1PatchDay6PoliceBeignet },
    {    230, "day 6 police sleep timer issue",              PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),    -5, gk1SignatureDay6PoliceSleep, gk1PatchDay6PoliceSleep },
    {      0, NULL,                                          0,                                            0, NULL,                          NULL }
};

// this here gets called on entry and when going out of game windows
//  uEvt::port will not get changed after kDisposeWindow but a bit later, so
//  we would get an invalid port handle to a kSetPort call. We just patch in
//  resetting of the port selector. We destroy the stop/fade code in there,
//  it seems it isn't used at all in the game.
const byte hoyle4SignaturePortFix[] = {
	28,
	0x39, 0x09,        // pushi 09
	0x89, 0x0b,        // lsg 0b
	0x39, 0x64,        // pushi 64
	0x38, 0xc8, 0x00,  // pushi 00c8
	0x38, 0x2c, 0x01,  // pushi 012c
	0x38, 0x90, 0x01,  // pushi 0190
	0x38, 0xf4, 0x01,  // pushi 01f4
	0x38, 0x58, 0x02,  // pushi 0258
	0x38, 0xbc, 0x02,  // pushi 02bc
	0x38, 0x20, 0x03,  // pushi 0320
	0x46,              // calle [xxxx] [xxxx] [xx]
	+5, 43,            // [skip 5 bytes]
	0x30, 0x27, 0x00,  // bnt 0027 -> end of routine
	0x87, 0x00,        // lap 00
	0x30, 0x19, 0x00,  // bnt 0019 -> fade out
	0x87, 0x01,        // lap 01
	0x30, 0x14, 0x00,  // bnt 0014 -> fade out
	0x38, 0xa7, 0x00,  // pushi 00a7
	0x76,              // push0
	0x80, 0x29, 0x01,  // lag 0129
	0x4a, 0x04,        // send 04 (song::stop)
	0x39, 0x27,        // pushi 27
	0x78,              // push1
	0x8f, 0x01,        // lsp 01
	0x51, 0x54,        // class 54
	0x4a, 0x06,        // send 06 (PlaySong::play)
	0x33, 0x09,        // jmp 09 -> end of routine
	0x38, 0xaa, 0x00,  // pushi 00aa
	0x76,              // push0
	0x80, 0x29, 0x01,  // lag 0129
	0x4a, 0x04,        // send 04
	0x48,              // ret
	0
};

const uint16 hoyle4PatchPortFix[] = {
	PATCH_ADDTOOFFSET | +33,
	0x38, 0x31, 0x01,  // pushi 0131 (selector curEvent)
	0x76,              // push0
	0x80, 0x50, 0x00,  // lag 0050 (global var 80h, "User")
	0x4a, 0x04,        // send 04 (read User::curEvent)

	0x38, 0x93, 0x00,  // pushi 0093 (selector port)
	0x78,              // push1
	0x76,              // push0
	0x4a, 0x06,        // send 06 (write 0 to that object::port)
	0x48,              // ret
	PATCH_END
};

//    script, description,                                   magic DWORD,                                 adjust
const SciScriptSignature hoyle4Signatures[] = {
    {      0, "port fix when disposing windows",             PATCH_MAGICDWORD(0x64, 0x38, 0xC8, 0x00),    -5, hoyle4SignaturePortFix,   hoyle4PatchPortFix },
    {      0, NULL,                                          0,                                            0, NULL,                     NULL }
};


// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
const byte larry6SignatureDeathDialog[] = {
	7,
	0x3e, 0x33, 0x01,             // link 0133 (offset 0x20)
	0x35, 0xff,                   // ldi ff
	0xa3, 0x00,                   // sal 00
	+255, 0,
	+255, 0,
	+170, 12,                     // [skip 680 bytes]
	0x8f, 0x01,                   // lsp 01 (offset 0x2cf)
	0x7a,                         // push2
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	0x43, 0x7c, 0x0e,             // kMessage[7c] 0e
	+90, 10,                      // [skip 90 bytes]
	0x38, 0xd6, 0x00,             // pushi 00d6 (offset 0x335)
	0x78,                         // push1
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	+76, 11,                      // [skip 76 bytes]
	0x38, 0xcd, 0x00,             // pushi 00cd (offset 0x38b)
	0x39, 0x03,                   // pushi 03
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,
	0
};

const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,             // link 0200
	PATCH_ADDTOOFFSET | +687,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +98,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +82,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_END
};

//    script, description,                                   magic DWORD,                                  adjust
const SciScriptSignature larry6Signatures[] = {
    {     82, "death dialog memory corruption",              PATCH_MAGICDWORD(0x3e, 0x33, 0x01, 0x35),     0, larry6SignatureDeathDialog, larry6PatchDeathDialog },
    {      0, NULL,                                          0,                                            0, NULL,                       NULL }
};

// It seems to scripts warp ego outside the screen somehow (or maybe kDoBresen?)
//  ego::mover is set to 0 and rm119::doit will crash in that case. This here
//  fixes part of the problem and actually checks ego::mover to be 0 and skips
//  TODO: this should get further investigated by waltervn and maybe properly
//   patched. For now ego will shortly disappear and reappear a bit after
//   this isn't good, but sierra sci also "crashed" (endless looped) so this
//   is at least better than the original code
const byte sq5SignatureScrubbing[] = {
	19,
	0x18,             // not
	0x31, 0x37,       // bnt 37
	0x78,             // push1 (selector x)
	0x76,             // push0
	0x39, 0x38,       // pushi 38 (selector mover)
	0x76,             // push0
	0x81, 0x00,       // lag 00
	0x4a, 0x04,       // send 04 (read ego::mover)
	0x4a, 0x04,       // send 04 (read ego::mover::x)
	0x36,             // push
	0x34, 0xa0, 0x00, // ldi 00a0
	0x1c,             // ne?
	0
};

const uint16 sq5PatchScrubbing[] = {
	0x18,             // not
	0x31, 0x37,       // bnt 37
//	0x2f, 0x38,       // bt 37 (would save another byte, isn't needed
	0x39, 0x38,       // pushi 38 (selector mover)
	0x76,             // push0
	0x81, 0x00,       // lag 00
	0x4a, 0x04,       // send 04 (read ego::mover)
	0x31, 0x2e,       // bnt 2e (jump if ego::mover is 0)
	0x78,             // push1 (selector x)
	0x76,             // push0
	0x4a, 0x04,       // send 04 (read ego::mover::x)
	0x39, 0xa0,       // pushi a0 (saving 2 bytes)
	0x1c,             // ne?
	PATCH_END
};

//    script, description,                                   magic DWORD,                                  adjust
const SciScriptSignature sq5Signatures[] = {
    {    119, "scrubbing send crash",                        PATCH_MAGICDWORD(0x18, 0x31, 0x37, 0x78),     0, sq5SignatureScrubbing, sq5PatchScrubbing },
    {      0, NULL,                                          0,                                            0, NULL,                  NULL }
};


// will actually patch previously found signature area
void Script::applyPatch(const uint16 *patch, byte *scriptData, const uint32 scriptSize, int32 signatureOffset) {
	int32 offset = signatureOffset;
	uint16 patchWord = *patch;

	while (patchWord != PATCH_END) {
		if (patchWord & PATCH_ADDTOOFFSET) {
			offset += patchWord & ~PATCH_ADDTOOFFSET;
		} else if (patchWord & PATCH_GETORIGINALBYTE) {
			// TODO: implement this
		} else {
			scriptData[offset] = patchWord & 0xFF;
			offset++;
		}
		patch++;
		patchWord = *patch;
	}	
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 Script::findSignature(const SciScriptSignature *signature, const byte *scriptData, const uint32 scriptSize) {
	if (scriptSize < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	const uint32 magicDWord = signature->magicDWord; // is platform-specific BE/LE form, so that the later match will work
	const uint32 searchLimit = scriptSize - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == *(const uint32 *)(scriptData + DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + signature->magicOffset;
			uint32 byteOffset = offset;
			const byte *signatureData = signature->data;
			byte matchAdjust = 1;
			while (matchAdjust) {
				byte matchBytesCount = *signatureData++;
				if ((byteOffset + matchBytesCount) > scriptSize) // Out-Of-Bounds?
					break;
				if (memcmp(signatureData, &scriptData[byteOffset], matchBytesCount)) // Byte-Mismatch?
					break;
				// those bytes matched, adjust offsets accordingly
				signatureData += matchBytesCount;
				byteOffset += matchBytesCount;
				// get offset...
				matchAdjust = *signatureData++;
				byteOffset += matchAdjust;
			}
			if (!matchAdjust) // all matches worked?
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

void Script::matchSignatureAndPatch(uint16 scriptNr, byte *scriptData, const uint32 scriptSize) {
	const SciScriptSignature *signatureTable = NULL;
	if (g_sci->getGameId() == GID_GK1)
		signatureTable = gk1Signatures;
// hoyle4 now works due workaround inside GfxPorts
//	if (g_sci->getGameId() == GID_HOYLE4)
//		signatureTable = hoyle4Signatures;
	if (g_sci->getGameId() == GID_LSL6)
		signatureTable = larry6Signatures;
	if (g_sci->getGameId() == GID_SQ5)
		signatureTable = sq5Signatures;

	if (signatureTable) {
		while (signatureTable->data) {
			if (scriptNr == signatureTable->scriptNr) {
				int32 foundOffset = findSignature(signatureTable, scriptData, scriptSize);
				if (foundOffset != -1) {
					// found, so apply the patch
					warning("matched and patched %s on script %d offset %d", signatureTable->description, scriptNr, foundOffset);
					applyPatch(signatureTable->patch, scriptData, scriptSize, foundOffset);
				}
			}
			signatureTable++;
		}
	}
}

} // End of namespace Sci
