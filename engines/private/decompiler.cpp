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
// Heavily based on code by jdieguez

#include "private/decompiler.h"

namespace Private {

const char *kHeader = "Precompiled Game Matrix";
const uint kHeaderSize = 23;

const byte kCodeString = 0x01;
const byte kCodeShortLiteral = 0x02;
const byte kCodeBraceClose = 0x04;
const byte kCodeRect = 0x2e;
const byte kCodeRects = 0x4f;
const byte kCodeShortId = 0x50;

const static char *kCodeTable[] = {"",                   //
								   "",                   // 0x01  (string)
								   "",                   // 0x02  (short literal)
								   " {\n",               // 0x03
								   "}\n",                // 0x04
								   "(",                  // 0x05
								   ")",                  // 0x06
								   "",                   //
								   "",                   //
								   "",                   //
								   ",",                  // 0x0a
								   "",                   //
								   "%",                  // 0x0c
								   "",                   //
								   ";\n",                // 0x0e
								   "!",                  // 0x0f
								   "-",                  // 0x10
								   "+",                  // 0x11
								   "=",                  // 0x12
								   ">",                  // 0x13
								   "<",                  // 0x14
								   "if ",                // 0x15
								   "else ",              // 0x16
								   "Exit",               // 0x17
								   "goto ",              // 0x18
								   "Mask",               // 0x19
								   "MaskDrawn",          // 0x1a
								   "Movie",              // 0x1b
								   "Transition",         // 0x1c
								   "ThumbnailMovie",     // 0x1d
								   "BustMovie",          // 0x1e
								   "ViewScreen",         // 0x1f
								   "VSPicture",          // 0x20
								   "Bitmap",             // 0x21
								   "Timer",              // 0x22
								   "SoundArea",          // 0x23
								   "Sound",              // 0x24
								   "SoundEffect",        // 0x25
								   "SyncSound",          // 0x26
								   "LoopedSound",        // 0x27
								   "NoStopSounds",       // 0x28
								   "Resume",             // 0x29
								   "Inventory",          // 0x2a
								   "SetFlag",            // 0x2b
								   "ChgMode",            // 0x2c
								   "PoliceBust",         // 0x2d
								   "CRect",              // 0x2e   overridden with "RECT" if in "define rects" block
								   "",                   //
								   "Random",             // 0x30
								   "SafeDigit",          // 0x31
								   "LoseInventory",      // 0x32
								   "",                   //
								   "PaperShuffleSound",  // 0x34
								   "Quit",               // 0x35
								   "DossierAdd",         // 0x36
								   "DossierBitmap",      // 0x37
								   "DossierPrevSuspect", // 0x38
								   "DossierNextSuspect", // 0x39
								   "DossierChgSheet",    // 0x3a
								   "DiaryLocList",       // 0x3b
								   "DiaryPage",          // 0x3c
								   "DiaryInvList",       // 0x3d
								   "DiaryPageTurn",      // 0x3e
								   "DiaryGoLoc",         // 0x3f
								   "SaveGame",           // 0x40
								   "LoadGame",           // 0x41
								   "RestartGame",        // 0x42
								   "AskSave",            // 0x43
								   "SetModifiedFlag",    // 0x44
								   "PhoneClip",          // 0x45
								   "PoliceClip",         // 0x46
								   "AMRadioClip",        // 0x47
								   "\nsetting ",         // 0x48
								   "debug ",             // 0x49
								   "\ndefine ",          // 0x4a
								   "",                   //
								   "variables",          // 0x4c
								   "",                   //
								   "",                   //
								   "rects",              // 0x4f
								   ""};                  // 0x50  (short id)

Decompiler::Decompiler(char *buf, uint32 fileSize, bool mac) {

	Common::Array<byte> array;
	uint32 i = 0;
	while (i < fileSize) {
		array.push_back(buf[i]);
		i++;
	}

	Common::String firstBytes((const char *)array.begin(), (const char *)array.begin() + kHeaderSize);

	if (firstBytes != kHeader) {
		_result = Common::String(buf);
		return;
	}

	decompile(array, mac);
}

void Decompiler::decompile(Common::Array<byte> &buffer, bool mac) {
	Common::Array<byte>::iterator it = buffer.begin();

	Common::String ss;
	bool inDefineRects = false;
	for (it += kHeaderSize; it != buffer.end();) {
		byte b = *it++;
		if (b == kCodeString) {
			byte len = *it++;
			Common::String s((const char *)it, (const char *)it + len);
			it += len;
			ss += Common::String::format("\"%s\"", s.c_str());
		} else if (b == kCodeShortLiteral || b == kCodeShortId) {
			byte b1 = *it++;
			byte b2 = *it++;
			uint number = mac ? b2 + (b1 << 8) : b1 + (b2 << 8);
			if (b == kCodeShortId)
				ss += "k";
			ss += Common::String::format("%d", number);
		} else if (b == kCodeRect && inDefineRects) {
			ss += "RECT"; // override CRect
		} else if (b <= kCodeShortId && strlen(kCodeTable[b]) > 0) {
			ss += kCodeTable[b];
		} else {
			error("decompile(): Unknown byte code (%d %c)", b, b);
		}

		if (b == kCodeRects) {
			inDefineRects = true;
		} else if (b == kCodeBraceClose && inDefineRects) {
			inDefineRects = false;
		}
	}
	ss += "\ndefine locations { k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k13, k14 }";
	_result = ss;
}

Common::String Decompiler::getResult() const {
	return _result;
}

} // namespace Private
