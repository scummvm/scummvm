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

#ifndef LASTEXPRESS_SOUND_H
#define LASTEXPRESS_SOUND_H

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

#include "lastexpress/shared.h"

#include "lastexpress/helpers.h"

#include "common/list.h"
#include "common/system.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class StreamedSound;
class SubtitleManager;

class SoundManager : Common::Serializable {
public:
	enum SoundType {
		kSoundTypeNone = 0,
		kSoundType1,
		kSoundType2,
		kSoundType3,
		kSoundType4,
		kSoundType5,
		kSoundType6,
		kSoundType7,
		kSoundType8,
		kSoundType9,
		kSoundType10,
		kSoundType11,
		kSoundType12,
		kSoundType13,
		kSoundType14,
		kSoundType15,
		kSoundType16
	};

	enum FlagType {
		kFlagInvalid     = -1,
		kFlagNone        = 0x0,
		kFlag2           = 0x2,
		kFlag3           = 0x3,
		kFlag4           = 0x4,
		kFlag5           = 0x5,
		kFlag6           = 0x6,
		kFlag7           = 0x7,
		kFlag8           = 0x8,
		kFlag9           = 0x9,
		kFlag10          = 0xA,
		kFlag11          = 0xB,
		kFlag12          = 0xC,
		kFlag13          = 0xD,
		kFlag14          = 0xE,
		kFlag15          = 0xF,
		kFlagDefault     = 0x10,

		kFlagType1_2     = 0x1000000,
		kFlagSteam       = 0x1001007,
		kFlagType13      = 0x3000000,
		kFlagMenuClock   = 0x3080010,
		kFlagType7       = 0x4000000,
		kFlagType11      = 0x5000000,
		kFlagMusic       = 0x5000010,
		kFlagType3       = 0x6000000,
		kFlagLoop        = 0x6001008,
		kFlagType9       = 0x7000000
	};

	SoundManager(LastExpressEngine *engine);
	~SoundManager();

	// Timer
	void handleTimer();

	// State
	void resetState() { _state |= kSoundType1; }

	// Sound queue
	void updateQueue();
	void resetQueue(SoundType type1, SoundType type2 = kSoundTypeNone);
	void clearQueue();

	// Subtitles
	void updateSubtitles();

	// Entry
	bool isBuffered(Common::String filename, bool testForEntity = false);
	bool isBuffered(EntityIndex entity);
	void setupEntry(SoundType type, EntityIndex index);
	void processEntry(EntityIndex entity);
	void processEntry(SoundType type);
	void processEntry(Common::String filename);
	void processEntries();
	void removeFromQueue(Common::String filename);
	void removeFromQueue(EntityIndex entity);
	uint32 getEntryTime(EntityIndex index);

	// Misc
	void unknownFunction4();
	void clearStatus();

	// Sound playing
	void playSound(EntityIndex entity, Common::String filename, FlagType flag = kFlagInvalid, byte a4 = 0);
	SoundType playSoundWithSubtitles(Common::String filename, FlagType flag, EntityIndex entity, byte a4 = 0);
	void playSoundEvent(EntityIndex entity, byte action, byte a3 = 0);
	void playDialog(EntityIndex entity, EntityIndex entityDialog, FlagType flag, byte a4);
	void playSteam(CityIndex index);
	void playFightSound(byte action, byte a4);
	void playLocomotiveSound();
	void playWarningCompartment(EntityIndex entity, ObjectIndex compartment);

	// Dialog & Letters
	void readText(int id);
	const char *getDialogName(EntityIndex entity) const;

	// Sound bites
	void excuseMe(EntityIndex entity, EntityIndex entity2 = kEntityPlayer, FlagType flag = kFlagNone);
	void excuseMeCath();
	const char *justCheckingCath() const;
	const char *wrongDoorCath() const;
	const char *justAMinuteCath() const;

	// FLags
	SoundManager::FlagType getSoundFlag(EntityIndex index) const;

	// Debug
	void stopAllSound() const;

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser);
	uint32 count();

private:
	typedef int32 *SoundBuffer;

	enum SoundStatus {
		kSoundStatus_20       = 0x20,
		kSoundStatusRemoved   = 0x200,

		kSoundStatus_8000     = 0x8000,
		kSoundStatus_100000   = 0x100000,
		kSoundStatus_40000000 = 0x40000000,

		kSoundStatusClear0    = 0x10,
		kSoundStatusClear1    = 0x1F,
		kSoundStatusClear2    = 0x80,
		kSoundStatusClear3    = 0x200,
		kSoundStatusClear4    = 0x800,
		kSoundStatusClearAll  = 0xFFFFFFE0
	};

	enum SoundState {
		kSoundState0 = 0,
		kSoundState1 = 1,
		kSoundState2 = 2
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

	struct SoundEntry {
		SoundStatusUnion status;
		SoundType type;    // int
		//int field_8;
		//int field_C;
		//int field_10;
		//int fileData;
		//int field_18;
		int field_1C;
		uint32 time;
		//int field_24;
		//int field_28;
		Common::SeekableReadStream *stream;	// int
		//int field_30;
		int field_34;
		int field_38;
		int field_3C;
		int field_40;
		EntityIndex entity;
		int field_48;
		int field_4C;
		Common::String name1; //char[16];
		Common::String name2; //char[16];
		//int next; // offset to the next structure in the list (not used)
		SubtitleManager *subtitle;

		bool isStreamed; // TEMPORARY

		SoundEntry() {
			status.status = 0;
			type = kSoundTypeNone;

			field_1C = 0;
			time = 0;

			stream = NULL;

			field_34 = 0;
			field_38 = 0;
			field_3C = 0;
			field_40 = 0;
			entity = kEntityPlayer;
			field_48 = 0;
			field_4C = 0;

			subtitle = NULL;

			isStreamed = false;
		}

		~SoundEntry() {
			// Entries that have been queued would have their streamed disposed automatically
			if (!isStreamed)
				SAFE_DELETE(stream);

			//delete subtitle;
		}
	};

	// Engine
	LastExpressEngine *_engine;

	// State flag
	int _state;
	SoundType _currentType;

	// Sound stream
	StreamedSound *_soundStream;

	// Unknown data
	uint32 _data0;
	uint32 _data1;
	uint32 _data2;
	uint32 _flag;

	// Filters

	int32 _buffer[2940];    ///< Static sound buffer

	// Compartment warnings by Mertens or Coudert
	uint32 _lastWarning[12];

	// Looping sound
	void playLoopingSound();

	// Sound cache
	Common::List<SoundEntry *> _cache;

	SoundEntry *getEntry(EntityIndex index);
	SoundEntry *getEntry(Common::String name);
	SoundEntry *getEntry(SoundType type);

	void setupEntry(SoundEntry *entry, Common::String name, FlagType flag, int a4);
	void setEntryType(SoundEntry *entry, FlagType flag);
	void setEntryStatus(SoundEntry *entry, FlagType flag) const;
	bool setupCache(SoundEntry *entry);
	void loadSoundData(SoundEntry *entry, Common::String name);

	void updateEntry(SoundEntry *entry, uint value) const;
	void updateEntryState(SoundEntry *entry) const ;
	void resetEntry(SoundEntry *entry) const;
	void removeEntry(SoundEntry *entry);

	// Subtitles
	void showSubtitles(SoundEntry *entry, Common::String filename);
	void drawSubtitles(SubtitleManager *subtitle);

	// Sound filter
	void applyFilter(SoundEntry *entry, SoundBuffer buffer);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_H
