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

#ifndef LASTEXPRESS_SOUND_ENTRY_H
#define LASTEXPRESS_SOUND_ENTRY_H

/*
	Sound entry: 68 bytes (this is what appears in the savegames)
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - entity
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    char {16}       - name 1
	    char {16}       - name 2

	Sound queue entry: 120 bytes
	    uint16 {2}      - status
	    byte {1}        - ??
	    byte {1}        - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - file data pointer
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - archive structure pointer
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - entity
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    char {16}       - name 1
	    char {16}       - name 2
	    uint32 {4}      - pointer to next entry in the queue
	    uint32 {4}      - subtitle data pointer
*/

#include "lastexpress/data/snd.h"
#include "lastexpress/data/subtitle.h"

#include "lastexpress/shared.h"

namespace LastExpress {

class LastExpressEngine;
class SubtitleEntry;

enum SoundStatus {
	kSoundStatus_20       = 0x20,
	kSoundStatus_40       = 0x40,
	kSoundStatus_180      = 0x180,
	kSoundStatusRemoved   = 0x200,
	kSoundStatus_400      = 0x400,

	kSoundStatus_8000     = 0x8000,
	kSoundStatus_20000    = 0x20000,
	kSoundStatus_100000   = 0x100000,
	kSoundStatus_40000000 = 0x40000000,

	kSoundStatusClear0    = 0x10,
	kSoundStatusClear1    = 0x1F,
	kSoundStatusClear2    = 0x80,
	kSoundStatusClear3    = 0x200,
	kSoundStatusClear4    = 0x800,
	kSoundStatusClearAll  = 0xFFFFFFE0
};

union SoundStatusUnion {
	uint32 status;
	byte status1;
	byte status2;
	byte status3;
	byte status4;

	SoundStatusUnion() {
		status = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
class SoundEntry {
public:
	SoundEntry(LastExpressEngine *engine);
	~SoundEntry();

	void setStatus(SoundFlag flag);
	void setInCache();
	void update(uint val);
	void updateState();
	void reset();

	// Subtitles
	void showSubtitle(Common::String filename);

private:
	LastExpressEngine *_engine;

public:
	SoundStatusUnion status;
	SoundType type;    // int
	//int data;
	//int endOffset;
	int currentDataPtr;
	void *soundData;
	//int currentBufferPtr;
	int blockCount;
	uint32 time;
	//int size;
	//int field_28;
	Common::SeekableReadStream *stream;	// int
	//int field_30;
	int field_34;
	int field_38;
	int field_3C;
	int field_40;
	EntityIndex entity;
	int field_48;
	uint32 field_4C;
	Common::String name1; //char[16];
	Common::String name2; //char[16];
	//int next; // offset to the next structure in the list (not used)
	SubtitleEntry *subtitle;

	// Sound stream
	StreamedSound *soundStream;
};

//////////////////////////////////////////////////////////////////////////
// SubtitleEntry
//////////////////////////////////////////////////////////////////////////
class SubtitleEntry {
public:
	SubtitleEntry(LastExpressEngine *engine);
	~SubtitleEntry();

	void load(Common::String filename, SoundEntry *soundEntry);
	void loadData();
	void draw();
	void setupAndDraw();
	void drawOnScreen();

	// Accessors
	SoundStatusUnion getStatus() { return _status; }
	SoundEntry *getSoundEntry() { return _sound; }

private:
	LastExpressEngine *_engine;

	Common::String    _filename;
	SoundStatusUnion  _status;
	SoundEntry       *_sound;
	SubtitleManager  *_data;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_ENTRY_H
