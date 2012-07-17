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
 */

#ifndef TEENAGENT_RESOURCES_H
#define TEENAGENT_RESOURCES_H

#include "teenagent/pack.h"
#include "teenagent/segment.h"
#include "teenagent/font.h"
#include "graphics/surface.h"

struct ADGameDescription;

namespace TeenAgent {

// Code Segment Addresses (Read Only)
// Intro function : 0x024c
const uint16 csAddr_intro = 0x024c;
// Guard Drinking function : 0x5189
const uint16 csAddr_guardDrinking = 0x5189;
// Putting Rock in Hole function : 0x8d57
const uint16 csAddr_putRockInHole = 0x8d57;
// Successfully Got Anchor function : 0x99e0
const uint16 csAddr_gotAnchor = 0x99e0;
// Display Message function : 0xa055
const uint16 csAddr_displayMsg = 0xa055;
// Reject Message function : 0xa4d6
const uint16 csAddr_rejectMsg = 0xa4d6;

// Data Segment Addresses
// Cursor Graphic 8*12px : 0x00da to 0x0139 (Read Only)
const uint16 dsAddr_cursor = 0x00da;
// Reject Messages Address Pointers : (4 * 2-byte) = 0x339e to 0x33a5
const uint16 dsAddr_rejectMsg = 0x339e;

// Save State Region : 0x6478 to 0xdbf1
const uint16 dsAddr_saveState = 0x6478;
const uint16 saveStateSize = 0x777a;

// Save Description String (24 bytes) : 0x6478 to 0x648f

// Ego (Mark) position in scene : 0x64af to 0x64b2
const uint16 dsAddr_egoX = 0x64af; // 2 bytes
const uint16 dsAddr_egoY = 0x64b1; // 2 bytes

// Current Scene Id : 0xb4f3
const uint16 dsAddr_currentScene = 0xb4f3; // 1 byte

// Current Music Id Playing : 0xdb90
const uint16 dsAddr_currentMusic = 0xdb90; // 1 byte
// Counter for Mansion Intrusion Attempts : 0xdbea
const uint16 dsAddr_mansionEntryCount = 0xdbea;// 1 byte

// Intro Credits #1 : 0xe3c2 to 0xe3e5 (Read Only)
const uint16 dsAddr_introCredits1 = 0xe3c2; // "backgrounds ..."
// Intro Credits #2 : 0xe3e6 to 0xe3fe (Read Only)
const uint16 dsAddr_introCredits2 = 0xe3e6; // "music ..."
// Intro Credits #3 : 0xe3ff to 0xe42e (Read Only)
const uint16 dsAddr_introCredits3 = 0xe3ff; // "animation..."
// Intro Credits #4 : 0xe42f to 0xe45b (Read Only)
const uint16 dsAddr_introCredits4 = 0xe42f; // "programming..."
// Credits #5 : 0xe45c to 0xe47b (Read Only)
const uint16 dsAddr_credits5 = 0xe45c; // "after the tiring journey..."
// Final Credits #6 : 0xe47c to 0xe487 (Read Only)
const uint16 dsAddr_finalCredits6 = 0xe47c; // "THE END..."
// Final Credits #7 : 0xe488 to 0xe782 (Read Only)
const uint16 dsAddr_finalCredits7 = 0xe488; // "programming..."
// 0xe783 to 0xe78f: 13 null bytes at end of dseg data - segment alignment padding?

class Resources {
public:
	void deinit();
	bool loadArchives(const ADGameDescription *gd);

	void loadOff(Graphics::Surface &surface, byte *palette, int id);
	Common::SeekableReadStream *loadLan(uint32 id) const;
	Common::SeekableReadStream *loadLan000(uint32 id) const;

	/*
	 * PSP (as the other sony playstation consoles - to be confirmed and 'ifdef'ed here too)
	 * is very sensitive to the number of simultaneously opened files.
	 * This is an attempt to reduce their number to zero.
	 * TransientFilePack does not keep opened file descriptors and reopens it on each request.
	 */
#ifdef __PSP__
	TransientFilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#else
	FilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#endif

	Segment cseg, dseg;
	Font font7, font8;

	//const byte *getDialog(uint16 dialogNum) { return eseg.ptr(dialogOffsets[dialogNum]); }
	uint16 getDialogAddr(uint16 dialogNum) { return dialogOffsets[dialogNum]; }

	Segment eseg;
private:
	void precomputeDialogOffsets();

	Common::Array<uint16> dialogOffsets;
};

} // End of namespace TeenAgent

#endif
