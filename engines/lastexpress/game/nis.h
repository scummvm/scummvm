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

#ifndef LASTEXPRESS_NIS_H
#define LASTEXPRESS_NIS_H

#include "lastexpress/lastexpress.h"

#include "lastexpress/data/archive.h"
#include "lastexpress/sound/slot.h"

namespace LastExpress {

class LastExpressEngine;
class Slot;

struct HPF;
struct Event;
struct NisSprite;
struct NisEvents;
struct Extent;

#define NIS_SOUND_CHUNK_SIZE  739

class NISManager {

public:
	NISManager(LastExpressEngine *engine);
	~NISManager();
	
	void clearBounds();
	void addBounds(Extent extent);
	void convertNSPR16(byte *spriteData, NisSprite *outSprite);
	void getStream(byte *data, int32 size);
	void loadSnd(int32 size);
	int loadChunk(int32 size);
	bool initNIS(const char *filename, int32 flags);
	void endNIS();
	void abortNIS();
	void nisMouse(Event *event);
	void nisTimer(Event *event);
	void drawSprite(NisSprite *sprite);
	void waxSprite(NisSprite *sprite);
	void waneSprite(NisSprite *sprite);
	void showNISStatus(int32 status);
	bool doNIS(const char *name, int32 flags);
	void processNIS(NisEvents *event);
	void drawBK(int type);
	void getNISSlot();

	Slot *getChainedSound();
	void setChainedSound(Slot *slot);
	int32 getNISFlag();

private:
	LastExpressEngine *_engine = nullptr;

	int32 _flags = 0;

	Common::Rect _nisRect = Common::Rect(0, 0, 0, 0);

	int32 _eventsCount = 0;
	NisEvents *_events = nullptr;
	byte *_eventsByteStream = nullptr;

	int32 _numOfOverlays = 0;

	HPF *_archive = nullptr;
	Slot *_currentNISSound = nullptr;
	Slot *_chainedSoundSlot = nullptr;

	int32 _streamCurrentPosition = 0;
	int32 _currentStreamPage = 0;
	int32 _totalStreamPages = 0;
	int32 _remainingStreamBytes = 0;
	int32 _streamBufferSize = 0;
	int32 _totalBackgroundPages = 0;
	Extent _spriteExtent;

	PixMap *_backgroundSurface = nullptr;
	int32 _nisStatus = 0;
	bool _decompressToBackBuffer = true;
	bool _firstNISBackgroundDraw = true;
	bool _waxWaneToggleFlag = false;

	NisSprite *_background1 = nullptr;
	NisSprite *_background2 = nullptr;
	NisSprite *_waneSprite = nullptr;
	NisSprite *_waxSprite = nullptr;

	byte *_background1ByteStream = nullptr;
	byte *_background2ByteStream = nullptr;
	byte *_waneSpriteByteStream = nullptr;
	byte *_waxSpriteByteStream = nullptr;

	int32 _cumulativeEventSize = 0;
	bool _backgroundFlag = false;
	int32 _backgroundType = 0;
	int32 _selectBackgroundType = 0;
	int32 _background1Offset = 0;
	int32 _waneSpriteOffset = 0;

	PixMap *_originalBackgroundSurface = nullptr;


	void (LastExpressEngine::*_savedTimerEventHandle)(Event *) = nullptr;
	void (LastExpressEngine::*_savedMouseEventHandle)(Event *) = nullptr;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_NIS_H
