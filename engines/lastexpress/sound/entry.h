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

#ifndef LASTEXPRESS_SOUND_ENTRY_H
#define LASTEXPRESS_SOUND_ENTRY_H

/*
	Sound entry: 68 bytes (this is what appears in savegames)
	    uint32 {4}      - status
	    uint32 {4}      - tag
	    uint32 {4}      - time left (_blockCount - _time)
	    uint32 {4}      - time in sound ticks (30Hz timer)
	    uint32 {4}      - LastExpress_ADPCMStream::_volumeHoldBlocks
	                      (useless since the target volume is not saved)
	    uint32 {4}      - ?? [no references except for save/load]
	    uint32 {4}      - entity
	    uint32 {4}      - activate delay in sound ticks (30Hz timer)
	    uint32 {4}      - priority
	    char {16}       - name of linked-after sound
	                      (always empty because only NIS entries
	                       have linked-after sounds, and NIS entries are not saved)
	    char {16}       - name

	Sound queue entry: 120 bytes
	    uint32 {4}      - status (combination of flags from SoundFlag)
	    uint32 {4}      - tag (enum SoundTag for special sounds,
	                           unique value for normal ones)
	    uint32 {4}      - pointer to the beginning of the buffer for compressed sound data
	    uint32 {4}      - pointer to the end of the buffer for compressed sound data
	    uint32 {4}      - decoder pointer inside the buffer
	    uint32 {4}      - pointer to the sound buffer [always same as the third field]
	    uint32 {4}      - reader pointer inside the buffer
	    uint32 {4}      - time left (_blockCount - time)
	    uint32 {4}      - time in sound ticks (30Hz timer)
	    uint32 {4}      - buffer size
	    uint32 {4}      - union:
	                       if data stream is open: position in the stream
	                       if data stream is closed: close reason, purely informational
	    uint32 {4}      - archive structure pointer
	    uint32 {4}      - _linkAfter, pointer to the entry for linked-after sound
	                      (xxx.LNK for sound entry corresponding to xxx.NIS)
	    uint32 {4}      - LastExpress_ADPCMStream::_volumeHoldBlocks
	                      (used for smooth change of volume)
	    uint32 {4}      - ?? [no references except for save/load]
	    uint32 {4}      - target value for smooth change of volume
	    uint32 {4}      - base volume if NIS is playing
	                      (the actual volume is reduced in half for non-NIS sounds;
	                       this is used to restore the volume after NIS ends)
	    uint32 {4}      - entity
	    uint32 {4}      - activate time in sound ticks (30Hz timer)
	    uint32 {4}      - priority
	    char {16}       - name of linked-after sound, used to save/load _linkAfter
	    char {16}       - name
	    uint32 {4}      - pointer to next entry in the queue
	    uint32 {4}      - subtitle data pointer
*/

#include "lastexpress/data/snd.h"
#include "lastexpress/data/subtitle.h"

#include "lastexpress/shared.h"

#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class SubtitleEntry;

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
class SoundEntry : Common::Serializable {
public:
	SoundEntry(LastExpressEngine *engine);
	~SoundEntry() override;

	void open(Common::String name, SoundFlag flag, int priority);
	void close();
	// startTime is measured in sound ticks, 30Hz timer
	// [used for restoring the entry from savefile]
	void play(uint32 startTime = 0);
	void kill() {
		_entity = kEntityPlayer; // no kActionEndSound notifications
		close();
	}
	void setVolumeSmoothly(SoundFlag newVolume);
	// setVolumeSmoothly() treats kVolumeNone in a special way;
	// fade() terminates the stream after the transition
	void fade() { setVolumeSmoothly(kVolumeNone); }
	bool update();
	void adjustVolumeIfNISPlaying();
	void setVolume(SoundFlag newVolume);
	// activateDelay is measured in main ticks, 15Hz timer
	void initDelayedActivate(unsigned activateDelay);

	// Subtitles
	void setSubtitles(Common::String filename);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser) override;

	// Accessors
	void setEntity(EntityIndex entity) { _entity = entity; }
	bool needSaving() const {
		return (_name != "NISSND?" && (_status & kSoundTypeMask) != kSoundTypeMenu);
	}

	uint32           getStatus()   { return _status; }
	int32            getTag()      { return _tag; }
	uint32           getTime()     { return _soundStream ? (_soundStream->getTimeMS() * 30 / 1000) + _startTime : 0; }
	EntityIndex      getEntity()   { return _entity; }
	uint32           getPriority() { return _priority; }
	const Common::String& getName(){ return _name; }

	// Streams
	SimpleSound *getSoundStream() { return _soundStream; }

private:
	LastExpressEngine *_engine;

	// _status field is a combination of bits from SoundFlag; writing
	// _status = (SoundFlag)(_status | kSoundFlagXxx) instead of _status |= kSoundFlagXxx
	// is irksome, so let's keep the type as uint32
	uint32 _status;
	int32 _tag; // member of SoundTag for special sounds, unique value for normal sounds
	//byte *_bufferStart, *_bufferEnd, *_decodePointer, *_buffer, *_readPointer;
	// the original game uses uint32 _blocksLeft, _time instead of _blockCount
	// we ask the backend for sound time
	uint32 _blockCount;
	uint32 _startTime;
	//uint32 _bufferSize;
	//union { uint32 _streamPos; enum StreamCloseReason _streamCloseReason; };
	Common::SeekableReadStream *_stream;    // The file stream
	//SoundEntry* _linkAfter;
	//uint32 _volumeHoldBlocks; // the related logic is in LastExpress_ADPCMStream
	//uint32 _unused;
	//uint32 _smoothChangeTarget; // the related logic is in LastExpress_ADPCMStream
	uint32 _volumeWithoutNIS;
	EntityIndex _entity;
	// The original game uses one variable _activateTime = _initTime + _activateDelay
	// and measures everything in sound ticks (30Hz timer).
	// We use milliseconds and two variables to deal with possible overflow
	// (probably paranoid, but nothing really complicated).
	uint32 _initTimeMS, _activateDelayMS;
	uint32 _priority;
	// char _linkAfterName[16];
	Common::String _name;    //char[16];
	// original has pointer to the next structure in the list (not used)
	SubtitleEntry *_subtitle;

	// Sound buffer & stream
	StreamedSound *_soundStream;    // the filtered sound stream

	void setupTag(SoundFlag flag);
	void setupStatus(SoundFlag flag);
	void loadStream(Common::String name);
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
	void close();
	void setupAndDraw();
	void drawOnScreen();

	// Accessors
	uint32 getStatus() { return _status; }
	SoundEntry *getSoundEntry()  { return _sound; }

private:
	LastExpressEngine *_engine;

	Common::String    _filename;
	uint32            _status;
	SoundEntry       *_sound;
	SubtitleManager  *_data;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_ENTRY_H
