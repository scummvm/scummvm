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

// Code Segment Addresses
// Intro function : 0x024c
const uint16 csAddr_intro = 0x024c;
// Display Message function : 0xa055
const uint16 csAddr_displayMsg = 0xa055;
// Reject Message function : 0xa4d6
const uint16 csAddr_rejectMsg = 0xa4d6;

// Data Segment Addresses
// Reject Messages Address Pointers : (4 * 2-byte) = 0x339e to 0x33a5
const uint16 dsAddr_rejectMsg = 0x339e;

// Save State Region : 0x6478 to 0xdbf1
const uint16 dsAddr_saveState = 0x6478;
const uint16 saveStateSize = 0x777a;

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
