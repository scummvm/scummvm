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

#ifndef LASTEXPRESS_SOUND_SLOT_H
#define LASTEXPRESS_SOUND_SLOT_H

#include "lastexpress/shared.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/sound/sound.h"

namespace LastExpress {

class LastExpressEngine;
class Subtitle;
class SoundManager;
struct HPF;

#include <common/pack-start.h>
typedef struct SaveSlot {
	int32 status;
	int32 tag;
	int32 blockCount;
	int32 time;
	int32 fadeDelayCounter;
	int32 unusedVar;
	int32 character;
	int32 delayTicks;
	int32 priority;
	char name1[16];
	char name2[16];

	SaveSlot() {
		status = 0;
		tag = 0;
		blockCount = 0;
		time = 0;
		fadeDelayCounter = 0;
		unusedVar = 0;
		character = 0;
		delayTicks = 0;
		priority = 0;
		memset(name1, 0, 16);
		memset(name2, 0, 16);
	}
} SaveSlot;
#include <common/pack-end.h>

class Slot {
	friend class Subtitle;
	friend class LastExpressEngine;
	friend class SoundManager;
	friend class LogicManager;
	friend class SubtitleManager;
	friend class Menu;

public:
	Slot(SoundManager *soundMan, const char *filename, int typeFlags, int priority);
	Slot(SoundManager *soundMan, int typeFlags, int priority);
	Slot(SoundManager *soundMan, SaveSlot *soundEntry);
	~Slot();

	void play();
	void setFade(int volume);
	void setVol(int volume);
	void setTag(int typeFlags);
	void setType(int typeFlags);
	void setSub(const char *filename);
	bool getBuffer();
	void releaseBuffer();
	void virtualize();
	void devirtualize();
	bool load(const char *filename);
	void stream();
	bool update();

	// The following getters/setters were not in the original
	// but are needed for thread safety
	int32 getStatusFlags();
	void setStatusFlags(int32 flags); 
	void addStatusFlag(int32 flag);
	void removeStatusFlag(int32 flag);
	int32 getTag();
	bool hasTag(int32 tag);
	void assignDirectTag(int32 tag);
	int getAssociatedCharacter();
	void setAssociatedCharacter(int character);
	int32 getTime();
	Slot *getNext();
	void setNext(Slot *next);
	int getPriority();
	bool closeArchive();
	void assignDirectVolume(int volume);
	int getVolume();
	void setChainedSound(Slot *chainedSound);
	Subtitle *getSubtitle();
	int32 getSize();
	void setSize(int32 size);
	int32 getNumLoadedBytes();
	void setNumLoadedBytes(int32 bytes);
	void advanceLoadedBytesBy(int32 loadedBytes);
	byte *getDataStart();
	void setDataStart(byte *dataStart);
	byte *getDataEnd();
	void setDataEnd(byte *dataEnd);
	void setDataEndByte(int32 pos, byte value);
	void advanceDataEndBy(int32 size);
	byte *getSoundBuffer();
	void setSoundBuffer(byte *bufferPtr);
	byte *getCurrentBufferPtr();
	void advanceCurrentBufferPtrBy(int32 size);
	void setCurrentBufferPtr(byte *bufferPtr);
	byte *getCurrentDataPtr();
	void setCurrentDataPtr(byte *dataPtr);
	int32 getBlockCount();
	void setBlockCount(int32 blockCount);
	void setDelayedStartTime(int32 time);
	char *getName2();

private:
	LastExpressEngine *_engine = nullptr;
	SoundManager *_soundMan = nullptr;

	int32 _statusFlags = 0;
	int32 _tag = 0;
	byte *_dataStart = nullptr;
	byte *_dataEnd = 0;
	byte *_currentDataPtr = nullptr;
	byte *_soundBuffer = nullptr;
	byte *_currentBufferPtr = nullptr;
	int32 _blockCount = 0;
	int32 _time = 0;
	int32 _size = 0;
	int32 _loadedBytes = 0;
	HPF *_archive = nullptr;
	Slot *_chainedSound = nullptr;
	int32 _fadeDelayCounter = 0;
	int32 _unusedVar = 0;
	int32 _fadeTargetVolume = 0;
	int _volume = 0;
	int _character = 0;
	int32 _delayedStartTime = 0;
	int _priority = 0;
	char _name1[16];
	char _name2[16];
	Slot *_next = nullptr;
	Subtitle *_subtitle = nullptr;
};


} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_SLOT_H
