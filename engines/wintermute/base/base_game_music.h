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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_GAME_MUSIC_H
#define WINTERMUTE_BASE_GAME_MUSIC_H

#include "common/scummsys.h"


namespace Wintermute {

#define NUM_MUSIC_CHANNELS 5
class BasePersistenceManager;
class BaseSound;
class ScStack;
class ScScript;
class BaseGame;
class BaseGameMusic {
public:
	BaseGameMusic(BaseGame *gameRef);
	void cleanup();

	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);

	bool resumeMusic(int channel);
	bool setMusicStartTime(int channel, uint32 time);
	bool pauseMusic(int channel);
	bool stopMusic(int channel);
	bool playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	bool updateMusicCrossfade();

	bool persistChannels(BasePersistenceManager *persistMgr);
	bool persistCrossfadeSettings(BasePersistenceManager *persistMgr);
private:
	BaseGame *_gameRef;
	BaseSound *_music[NUM_MUSIC_CHANNELS];
	uint32 _musicStartTime[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int32 _musicCrossfadeChannel1;
	int32 _musicCrossfadeChannel2;
	int32 _musicCrossfadeVolume1;
	int32 _musicCrossfadeVolume2;
};

} // End of namespace Wintermute

#endif
