/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BSSOUND_H
#define BSSOUND_H

#include "object.h"
#include "sworddefs.h"
#include "common/file.h"
#include "sound/mixer.h"
#include "common/util.h"

#define	TOTAL_FX_PER_ROOM	7		// total loop & random fx per room (see fx_list.c)
#define	MAX_ROOMS_PER_FX	7		// max no. of rooms in the fx's room,vol list
#define	MAX_FXQ_LENGTH		32		// max length of sound queue - ie. max number of fx that can be stored up/playing together

#define FX_SPOT 1
#define FX_LOOP 2
#define FX_RANDOM 3

struct QueueElement {
	uint32 id, delay;
	PlayingSoundHandle handle;
};

struct RoomVol {
	int32 roomNo, leftVol, rightVol;
};

struct FxDef {
	uint32 sampleId, type, delay;
	RoomVol roomVolList[MAX_ROOMS_PER_FX];
};

class SoundMixer;
class ResMan;

class SwordSound {
public:
	SwordSound(const char *searchPath, SoundMixer *mixer, ResMan *pResMan);
	~SwordSound(void);
	void newScreen(uint32 screen);
	void quitScreen(void);

	bool startSpeech(uint16 roomNo, uint16 localNo); // this should work more or less.
													 // Maybe we'll need a delay of 3 gameCycles.
	bool speechFinished(void);
	void stopSpeech();
	bool amISpeaking(void); // this is supposed to return if the sounddata is near the ending or very silent...

	void fnStopFx(int32 fxNo);
	int addToQueue(int32 fxNo);

	void engine(void);

private:
	void playSample(QueueElement *elem);
	void initCowSystem(void);
	void closeCowSystem(void);

	int16 *uncompressSpeech(uint32 index, uint32 cSize, uint32 *size);
	File		 _cowFile;
	uint32		 *_cowHeader;
	uint32		 _cowHeaderSize;
	uint8		 _currentCowFile;
	PlayingSoundHandle _speechHandle, _fxHandle;
	Common::RandomSource _rnd;
	
	QueueElement _fxQueue[MAX_FXQ_LENGTH];
	uint8		 _endOfQueue;
	SoundMixer *_mixer;
	ResMan *_resMan;
	char _filePath[100];
	static const char _musicList[270];
	static const uint16 _roomsFixedFx[TOTAL_ROOMS][TOTAL_FX_PER_ROOM];
	static const FxDef _fxList[312];
};

#endif //BSSOUND_H
