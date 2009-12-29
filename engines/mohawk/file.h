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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"

#include "mohawk/sound.h"

#ifndef MOHAWK_FILE_H
#define MOHAWK_FILE_H

namespace Mohawk {

// Main FourCC's
#define ID_MHWK MKID_BE('MHWK') // Main FourCC
#define ID_RSRC MKID_BE('RSRC') // Resource Directory Tag

// Myst Resource FourCC's
#define ID_CLRC MKID_BE('CLRC') // Cursor Hotspots
#define ID_EXIT MKID_BE('EXIT') // Card Exit Scripts
#define ID_HINT MKID_BE('HINT') // Specifies Cursors in What Area
#define ID_INIT MKID_BE('INIT') // Card Entrance Scripts
#define ID_MSND MKID_BE('MSND') // Standard Mohawk Sound
#define ID_RLST MKID_BE('RLST') // Resource List, Specifies HotSpots
#define ID_RSFL MKID_BE('RSFL') // ??? (system.dat only)
#define ID_VIEW MKID_BE('VIEW') // Card Details
#define ID_WDIB MKID_BE('WDIB') // LZ-Compressed Windows Bitmap

// Myst Masterpiece Edition Resource FourCC's (In addition to Myst FourCC's)
#define ID_HELP MKID_BE('HELP') // Help Chunk
#define ID_MJMP MKID_BE('MJMP') // MSND Jumps (To reduce MSND duplication)
#define ID_PICT MKID_BE('PICT') // JPEG/PICT Image

// Riven Resource FourCC's
#define ID_BLST MKID_BE('BLST') // Card Hotspot Enabling Lists
#define ID_CARD MKID_BE('CARD') // Card Scripts
#define ID_FLST MKID_BE('FLST') // Card SFXE Lists
#define ID_HSPT MKID_BE('HSPT') // Card Hotspots
#define ID_MLST MKID_BE('MLST') // Card Movie Lists
#define ID_NAME MKID_BE('NAME') // Object Names
#define ID_PLST MKID_BE('PLST') // Card Picture Lists
#define ID_RMAP MKID_BE('RMAP') // Card Code
#define ID_SFXE MKID_BE('SFXE') // Water Effect Animations
#define ID_SLST MKID_BE('SLST') // Card Ambient Sound Lists
#define ID_TMOV MKID_BE('tMOV') // Game Movie

// Riven Saved Game FourCC's
#define ID_VARS MKID_BE('VARS') // Saved Game Variable Values
#define ID_VERS MKID_BE('VERS') // Version Info
#define ID_ZIPS MKID_BE('ZIPS') // Zip Mode Status

// Zoombini Resource FourCC's
#define ID_SND  MKID_BE('\0SND') // Standard Mohawk Sound
#define ID_CURS MKID_BE('CURS') // Cursor?
#define ID_SCRB MKID_BE('SCRB') // ???
#define ID_SCRS MKID_BE('SCRS') // ???
#define ID_NODE MKID_BE('NODE') // ???
#define ID_PATH MKID_BE('PATH') // ???
#define ID_SHPL MKID_BE('SHPL') // ???

// Living Books Resource FourCC's
#define ID_TCUR MKID_BE('tCUR') // Cursor
#define ID_BITL MKID_BE('BITL') // ???
#define ID_CTBL MKID_BE('CTBL') // Color Table?
#define ID_SCRP MKID_BE('SCRP') // Script?
#define ID_SPR  MKID_BE('SPR#') // Sprites?
#define ID_VRSN MKID_BE('VRSN') // Version
#define ID_ANI  MKID_BE('ANI ') // Animation?
#define ID_SHP  MKID_BE('SHP#') // ???

// JamesMath Resource FourCC's
#define ID_TANM MKID_BE('tANM') // Animation?
#define ID_TMFO MKID_BE('tMFO') // ???

// Mohawk Wave Tags
#define ID_WAVE MKID_BE('WAVE') // Game Sound (Third Tag)
#define ID_ADPC MKID_BE('ADPC') // Game Sound Chunk
#define ID_DATA MKID_BE('Data') // Game Sound Chunk
#define ID_CUE  MKID_BE('Cue#') // Game Sound Chunk

// Mohawk MIDI Tags
#define ID_MIDI MKID_BE('MIDI') // Game Sound (Third Tag), instead of WAVE
#define ID_PRG  MKID_BE('Prg#') // Midi Program?

// Old Mohawk Resource FourCC's
#define ID_WAV  MKID_BE('WAV ') // Old Sound Resource
#define ID_BMAP MKID_BE('BMAP') // Standard Mohawk Bitmap

// Common Resource FourCC's
#define ID_TBMP MKID_BE('tBMP') // Standard Mohawk Bitmap
#define ID_TWAV MKID_BE('tWAV') // Standard Mohawk Sound
#define ID_TPAL MKID_BE('tPAL') // Standard Mohawk Palette
#define ID_TCNT MKID_BE('tCNT') // ??? (CSWorld, CSAmtrak, JamesMath)
#define ID_TSCR MKID_BE('tSCR') // Script? Screen? (CSWorld, CSAmtrak, Treehouse)
#define ID_STRL MKID_BE('STRL') // String List (Zoombini, CSWorld, CSAmtrak)
#define ID_TBMH MKID_BE('tBMH') // Standard Mohawk Bitmap
#define ID_TMID MKID_BE('tMID') // Standard Mohawk MIDI
#define ID_REGS MKID_BE('REGS') // ??? (Zoombini, Treehouse)
#define ID_BYTS MKID_BE('BYTS') // Database Entry (CSWorld, CSAmtrak)
#define ID_INTS MKID_BE('INTS') // ??? (CSWorld, CSAmtrak)
#define ID_BBOX MKID_BE('BBOX') // Boxes? (CSWorld, CSAmtrak)
#define ID_SYSX MKID_BE('SYSX') // MIDI Sysex

struct FileTable {
	uint32 offset;
	uint32 dataSize; // Really 27 bits
	byte flags; // Mostly useless except for the bottom 3 bits which are part of the size
	uint16 unk; // Always 0
};

struct Type {
	Type() { resTable.entries = NULL; nameTable.entries = NULL; }
	~Type() { delete[] resTable.entries; delete[] nameTable.entries; }

	//Type Table
	uint32 tag;
	uint16 resource_table_offset;
	uint16 name_table_offset;
		
	struct ResourceTable {
		uint16 resources;
		struct Entries {
			uint16 id;
			uint16 index;
		} *entries;
	} resTable;
		
	struct NameTable {
		uint16 num;
		struct Entries {
			uint16 offset;
			uint16 index;
			// Name List
			Common::String name;
		} *entries;
	} nameTable;
};

struct TypeTable {
	uint16 name_offset;
	uint16 resource_types;
};

struct RSRC_Header {
	uint32 size;
	uint32 filesize;
	uint32 abs_offset;
	uint16 file_table_offset;
	uint16 file_table_size;
};

class MohawkFile {
public:
	MohawkFile();
	virtual ~MohawkFile() { close(); }
	
	void open(Common::String filename);
	virtual void open(Common::SeekableReadStream *stream);
	void close();
	
	bool hasResource(uint32 tag, uint16 id);
	virtual Common::SeekableReadStream *getRawData(uint32 tag, uint16 id);

protected:
	Common::SeekableReadStream *_mhk;
	TypeTable _typeTable;
	Common::String _curFile;
	
private:
	bool _hasData;
	uint32 _fileSize;
	RSRC_Header _rsrc;
	Type *_types;
	FileTable *_fileTable;
	uint16 _nameTableAmount;
	uint16 _resourceTableAmount;
	uint16 _fileTableAmount;

	virtual int16 getTypeIndex(uint32 tag) {
		for (uint16 i = 0; i < _typeTable.resource_types; i++)
			if (_types[i].tag == tag)
				return i;
		return -1;	// not found
	}

	virtual int16 getIdIndex(int16 typeIndex, uint16 id) {
		for (uint16 i = 0; i < _types[typeIndex].resTable.resources; i++)
			if (_types[typeIndex].resTable.entries[i].id == id)
				return i;
		return -1;	// not found
	}
};

class OldMohawkFile : public MohawkFile {
public:
	OldMohawkFile() : MohawkFile() {}
	~OldMohawkFile() {}
	
	void open(Common::SeekableReadStream *stream);
	Common::SeekableReadStream *getRawData(uint32 tag, uint16 id);
	
private:
	struct OldType {
		uint32 tag;
		uint16 resource_table_offset;
		struct ResourceTable {
			uint16 resources;
			struct Entries {
				uint16 id;
				uint32 offset;
				uint32 size;
			} *entries;
		} resTable;
	} *_types;
	
	int16 getTypeIndex(uint32 tag) {
		for (uint16 i = 0; i < _typeTable.resource_types; i++)
			if (_types[i].tag == tag)
				return i;
		return -1;	// not found
	}
	
	int16 getIdIndex(int16 typeIndex, uint16 id) {
		for (uint16 i = 0; i < _types[typeIndex].resTable.resources; i++)
			if (_types[typeIndex].resTable.entries[i].id == id)
				return i;
		return -1;	// not found
	}
};

} // End of namespace Mohawk

#endif
