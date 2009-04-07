/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 * $URL$
 * $Id$
 */

#ifndef	SWORD2_HEADER_H
#define	SWORD2_HEADER_H

#include "common/stream.h"
#include "common/endian.h"

namespace Sword2 {

//----------------------------------------------------------
// SYSTEM FILE & FRAME HEADERS
//----------------------------------------------------------

//----------------------------------------------------------
// ALL FILES
//----------------------------------------------------------

// Standard File Header

#define	NAME_LEN 34

struct ResHeader {
	uint8 fileType;			// Byte to define file type (see below)
	uint8 compType;			// Type of file compression used ie.
					// on whole file (see below)
	uint32 compSize;		// Length of compressed file (ie.
					// length on disk)
	uint32 decompSize;		// Length of decompressed file held in
					// memory (NB. frames still held
					// compressed)
	byte name[NAME_LEN];		// Name of object

	static int size() {
		return 44;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		fileType = readS.readByte();
		compType = readS.readByte();
		compSize = readS.readUint32LE();
		decompSize = readS.readUint32LE();
		readS.read(name, NAME_LEN);
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeByte(fileType);
		writeS.writeByte(compType);
		writeS.writeUint32LE(compSize);
		writeS.writeUint32LE(decompSize);
		writeS.write(name, NAME_LEN);
	}
};

// fileType

enum {
	// 0 something's wrong!
	ANIMATION_FILE		= 1,	// All normal animations & sprites
					// including mega-sets & font files
					// which are the same format (but all
					// frames always uncompressed)
	SCREEN_FILE		= 2,	// Each contains background, palette,
					// layer sprites, parallax layers &
					// shading mask
	GAME_OBJECT		= 3,	// Each contains object hub +
					// structures + script data
	WALK_GRID_FILE		= 4,	// Walk-grid data
	GLOBAL_VAR_FILE		= 5,	// All the global script variables in
					// one file; "there can be only one"
	PARALLAX_FILE_null	= 6,	// NOT USED
	RUN_LIST		= 7,	// Each contains a list of object
					// resource id's
	TEXT_FILE		= 8,	// Each contains all the lines of text
					// for a location or a character's
					// conversation script
	SCREEN_MANAGER		= 9,	// One for each location; this contains
					// special startup scripts
	MOUSE_FILE		= 10,	// Mouse pointers and luggage icons
					// (sprites in General / Mouse pointers
					// & Luggage icons)
	WAV_FILE		= 11,	// WAV file
	ICON_FILE		= 12,	// Menu icon (sprites in General / Menu
					// icons)
	PALETTE_FILE		= 13	// separate palette file (see also
					// _paletteHeader)
};

// compType

enum {
	NO_COMPRESSION		= 0,
	FILE_COMPRESSION	= 1	// standard whole-file compression
					// (not yet devised!)
};

//----------------------------------------------------------
// (1) ANIMATION FILES
//----------------------------------------------------------

// an animation file consists of:
//
// standard file header
// animation header
// a string of CDT entries (one per frame of the anim)
// a 16-byte colour table ONLY if (runTimeComp==RLE16)
// a string of groups of (frame header + frame data)

// Animation Header

struct AnimHeader {
	uint8 runTimeComp;	// Type of runtime compression used for the
				// frame data (see below)
	uint16 noAnimFrames;	// Number of frames in the anim (ie. no. of
				// CDT entries)
	uint16 feetStartX;	// Start coords for mega to walk to, before
	uint16 feetStartY;	// running anim
	uint8 feetStartDir;	// Direction to start in before running anim
	uint16 feetEndX;	// End coords for mega to stand at after
	uint16 feetEndY;	// running anim (vital if anim starts from an
				// off-screen position, or ends in a different
				// place from the start)
	uint8 feetEndDir;	// Direction to start in after running anim
	uint16 blend;

	static int size() {
		return 15;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		if (Sword2Engine::isPsx()) {
			noAnimFrames = readS.readUint16LE();
			feetStartX = readS.readUint16LE();
			feetStartY = readS.readUint16LE();
			feetEndX = readS.readUint16LE();
			feetEndY = readS.readUint16LE();
			blend = readS.readUint16LE();
			runTimeComp = readS.readByte();
			feetStartDir = readS.readByte();
			feetEndDir = readS.readByte();	
		} else {
			runTimeComp = readS.readByte();
			noAnimFrames = readS.readUint16LE();
			feetStartX = readS.readUint16LE();
			feetStartY = readS.readUint16LE();
			feetStartDir = readS.readByte();
			feetEndX = readS.readUint16LE();
			feetEndY = readS.readUint16LE();
			feetEndDir = readS.readByte();
			blend = readS.readUint16LE();
		}
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeByte(runTimeComp);
		writeS.writeUint16LE(noAnimFrames);
		writeS.writeUint16LE(feetStartX);
		writeS.writeUint16LE(feetStartY);
		writeS.writeByte(feetStartDir);
		writeS.writeUint16LE(feetEndX);
		writeS.writeUint16LE(feetEndY);
		writeS.writeByte(feetEndDir);
		writeS.writeUint16LE(blend);
	}

};

// runtimeComp - compression used on each frame of the anim

enum {
	NONE	= 0,		// No frame compression
	RLE256	= 1,		// James's RLE for 256-colour sprites
	RLE16	= 2		// James's RLE for 16- or 17-colour sprites
				// (raw blocks have max 16 colours for 2 pixels
				// per byte, so '0's are encoded only as FLAT
				// for 17-colour sprites eg. George's mega-set)
};

// CDT Entry

struct CdtEntry {
	int16 x;		// sprite x-coord OR offset to add to mega's
				// feet x-coord to calc sprite y-coord
	int16 y;		// sprite y-coord OR offset to add to mega's
				// feet y-coord to calc sprite y-coord
	uint32 frameOffset;	// points to start of frame header (from start
				// of file header)
	uint8 frameType;	// 0 = print sprite normally with top-left
				// corner at (x,y), otherwise see below...

	static int size() {
		if (Sword2Engine::isPsx())
			return 12;
		else
			return 9;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		if (Sword2Engine::isPsx()) {
			readS.readByte(); // Skip a byte in psx version
			x = readS.readUint16LE();
			y = readS.readUint16LE();
			frameOffset = readS.readUint32LE();
			frameType = readS.readByte();
		} else {
			x = readS.readUint16LE();
			y = readS.readUint16LE();
			frameOffset = readS.readUint32LE();
			frameType = readS.readByte();
		}
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint16LE(x);
		writeS.writeUint16LE(y);
		writeS.writeUint32LE(frameOffset);
		writeS.writeByte(frameType);
	}
};

// 'frameType' bit values

enum {
	FRAME_OFFSET	= 1,	// Print at (feetX + x, feetY + y), with
				// scaling according to feetY
	FRAME_FLIPPED	= 2,	// Print the frame flipped Left->Right
	FRAME_256_FAST	= 4	// Frame has been compressed using Pauls fast
				// RLE 256 compression.
};

// Frame Header

struct FrameHeader {
	uint32 compSize;	// Compressed size of frame - NB. compression
				// type is now in Anim Header
	uint16 width;		// Dimensions of frame
	uint16 height;

	static int size() {
		return 8;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		compSize = readS.readUint32LE();
		width = readS.readUint16LE();
		height = readS.readUint16LE();

		if (Sword2Engine::isPsx()) { // In PSX version, frames are half height
			height *= 2;
			width = (width % 2) ? width + 1 : width;
		}
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint32LE(compSize);
		writeS.writeUint16LE(width);
		writeS.writeUint16LE(height);
	}
};

//----------------------------------------------------------
// (2) SCREEN FILES
//----------------------------------------------------------
// a screen file consists of:
//
// standard file header
// multi screen header
// 4 * 256 bytes of palette data
// 256k palette match table
// 2 background parallax layers
// 1 background layer with screen header
// 2 foreground parallax layers
// a string of layer headers
// a string of layer masks

// Multi screen header
// Goes at the beginning of a screen file after the standard header.
// Gives offsets from start of table of each of the components

struct MultiScreenHeader {
	uint32 palette;
	uint32 bg_parallax[2];
	uint32 screen;
	uint32 fg_parallax[2];
	uint32 layers;
	uint32 paletteTable;
	uint32 maskOffset;

	static int size() {
		return 36;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		palette = readS.readUint32LE();
		bg_parallax[0] = readS.readUint32LE();
		bg_parallax[1] = readS.readUint32LE();
		screen = readS.readUint32LE();
		fg_parallax[0] = readS.readUint32LE();
		fg_parallax[1] = readS.readUint32LE();
		layers = readS.readUint32LE();
		paletteTable = readS.readUint32LE();
		maskOffset = readS.readUint32LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint32LE(palette);
		writeS.writeUint32LE(bg_parallax[0]);
		writeS.writeUint32LE(bg_parallax[1]);
		writeS.writeUint32LE(screen);
		writeS.writeUint32LE(fg_parallax[0]);
		writeS.writeUint32LE(fg_parallax[1]);
		writeS.writeUint32LE(layers);
		writeS.writeUint32LE(paletteTable);
		writeS.writeUint32LE(maskOffset);
	}
};

// Screen Header

struct ScreenHeader {
	uint16 width;		// dimensions of the background screen
	uint16 height;
	uint16 noLayers;	// number of layer areas

	static int size() {
		return 6;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		width = readS.readUint16LE();
		height = readS.readUint16LE();
		noLayers = readS.readUint16LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint16LE(width);
		writeS.writeUint16LE(height);
		writeS.writeUint16LE(noLayers);
	}
};

// Layer Header

// Note that all the layer headers are kept together, rather than being placed
// before each layer mask, in order to simplify the sort routine.

struct LayerHeader {
	uint16 x;		// coordinates of top-left pixel of area
	uint16 y;
	uint16 width;
	uint16 height;
	uint32 maskSize;
	uint32 offset;		// where to find mask data (from start of
				// standard file header)

	static int size() {
		return 16;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		x = readS.readUint16LE();
		y = readS.readUint16LE();
		width = readS.readUint16LE();
		height = readS.readUint16LE();
		maskSize = readS.readUint32LE();
		offset = readS.readUint32LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint16LE(x);
		writeS.writeUint16LE(y);
		writeS.writeUint16LE(width);
		writeS.writeUint16LE(height);
		writeS.writeUint32LE(maskSize);
		writeS.writeUint32LE(offset);
	}
};

//----------------------------------------------------------
// (3) SCRIPT OBJECT FILES
//----------------------------------------------------------
// a script object file consists of:
//
// standard file header
// script object header
// script object data

//----------------------------------------------------------
// (5) PALETTE FILES
//----------------------------------------------------------
// a palette file consists of:
//
// standard file header
// 4 * 256 bytes of palette data
// 256k palette match table

// an object hub - which represents all that remains of the compact concept

class ObjectHub {
	// int32 type;		// type of object
	// uint32 logic_level;	// what level?
	// uint32 logic[3]	// NOT USED
	// uint32 script_id[3]	// need this if script
	// uint32 script_pc[3]	// need this also

private:
	byte *_addr;

public:
	ObjectHub() {
		_addr = NULL;
	}

	static int size() {
		return 44;
	}

	byte *data() {
		return _addr;
	}

	void setAddress(byte *addr) {
		_addr = addr;
	}

	byte *getScriptPcPtr(int level) {
		return _addr + 32 + 4 * level;
	}

	uint32 getLogicLevel() {
		return READ_LE_UINT32(_addr + 4);
	}
	uint32 getScriptId(int level) {
		return READ_LE_UINT32(_addr + 20 + 4 * level);
	}
	uint32 getScriptPc(int level) {
		return READ_LE_UINT32(_addr + 32 + 4 * level);
	}

	void setLogicLevel(uint32 x) {
		WRITE_LE_UINT32(_addr + 4, x);
	}
	void setScriptId(int level, uint32 x) {
		WRITE_LE_UINT32(_addr + 20 + 4 * level, x);
	}
	void setScriptPc(int level, uint32 x) {
		WRITE_LE_UINT32(_addr + 32 + 4 * level, x);
	}
};

// (6) text module header

struct TextHeader {
	uint32 noOfLines;	// how many lines of text are there in this
				// module

	static int size() {
		return 4;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		noOfLines = readS.readUint32LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint32LE(noOfLines);
	}
};

// a text file has:
//
//	ResHeader
//	TextHeader
//	look up table, to
//	line of text,0
//	line of text,0

//----------------------------------------------------------
// SCREENS.CLU file
//----------------------------------------------------------
// This file is present in PSX version of the game only.
// It keeps parallax and background images, aligned at 1024 bytes
// for faster access by the psx cd drive.
//
// SCREENS.CLU structure:
// In first 2048 Bytes there's an offset table. Each entry is an
// 32bit offset for a background/parallax group. If entry is 0, screen
// does not exist.
// To find matching screen for the location, you must count LOCATION_NO
// words and then go to the corresponding offset indicated by last 32bit
// word.
// Each screen then begins with a PSXScreensEntry entry:

struct PSXScreensEntry {
	uint16 fgPlxXres; // If these values are 0, subsequent fgPlx* values must be
	uint16 fgPlxYres; // ignored, as this means foreground parallax is not present.
	uint32 fgPlxOffset; // This offset is relative, counting from the beginning of Resource Header
	uint32 fgPlxSize; // Size of parallax, the size is aligned at 1024 bytes.
					  // fgPlxSize/1024 gives number of sector the parallax is divided into.
	uint16 bgXres;
	uint16 bgYres;
	uint32 bgOffset; // relative
	uint32 bgSize;
	uint16 bgPlxXres; // same considerations for fg parallaxes apply
	uint16 bgPlxYres;
	uint32 bgPlxOffset; // relative
	uint32 bgPlxSize;

	static int size() {
		return 36;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		bgPlxXres = readS.readUint16LE();
		bgPlxYres = readS.readUint16LE();
		bgPlxOffset = readS.readUint32LE();
		bgPlxSize = readS.readUint32LE();
		bgXres = readS.readUint16LE();
		bgYres = readS.readUint16LE();
		bgOffset = readS.readUint32LE();
		bgSize = readS.readUint32LE();
		fgPlxXres = readS.readUint16LE();
		fgPlxYres = readS.readUint16LE();
		fgPlxOffset = readS.readUint32LE();
		fgPlxSize = readS.readUint32LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint16LE(bgPlxXres);
		writeS.writeUint16LE(bgPlxYres);
		writeS.writeUint32LE(bgPlxOffset);
		writeS.writeUint32LE(bgPlxSize);
		writeS.writeUint16LE(bgXres);
		writeS.writeUint16LE(bgYres);
		writeS.writeUint32LE(bgOffset);
		writeS.writeUint32LE(bgSize);
		writeS.writeUint16LE(fgPlxXres);
		writeS.writeUint16LE(fgPlxYres);
		writeS.writeUint32LE(fgPlxOffset);
		writeS.writeUint32LE(fgPlxSize);
	}
};

// PSXFontEntry is present in font resource file, it is used
// to address a single char in the character atlas image.

struct PSXFontEntry {
	uint16 offset;
	uint16 skipLines;
	uint16 charWidth;
	uint16 charHeight;

	static int size() {
		return 8;
	}

	void read(byte *addr) {
		Common::MemoryReadStream readS(addr, size());

		offset = readS.readUint16LE() / 2;
		skipLines = readS.readUint16LE();
		charWidth = readS.readUint16LE() / 2;
		charHeight = readS.readUint16LE();
	}

	void write(byte *addr) {
		Common::MemoryWriteStream writeS(addr, size());

		writeS.writeUint16LE(offset);
		writeS.writeUint16LE(skipLines);
		writeS.writeUint16LE(charWidth);
		writeS.writeUint16LE(charHeight);
	}	
};

} // End of namespace Sword2

#endif
