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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_SOUND_H
#define WINTERMUTE_BASE_SOUND_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/persistent.h"
#include "audio/mixer.h"

namespace Wintermute {

class BaseSoundBuffer;
class BaseSound : public BaseClass {
public:
	bool setPan(float pan);
	int32 _soundPrivateVolume;
	int getVolume();
	int getVolumePercent();
	bool setVolumePercent(int percent);
	bool setVolume(int volume);
	bool setPrivateVolume(int volume);
	bool setLoopStart(uint32 pos);
	uint32 getPositionTime();
	bool setPositionTime(uint32 time);
	bool _soundPaused;
	bool _soundFreezePaused;
	bool isPlaying();
	bool isPaused();
	bool _soundPlaying;
	bool _soundLooping;
	uint32 _soundLoopStart;
	uint32 _soundPosition;
	DECLARE_PERSISTENT(BaseSound, BaseClass)
	bool resume();
	bool pause(bool freezePaused = false);
	bool stop();
	bool play(bool looping = false);
	uint32 getLength();
	bool _soundStreamed;
	TSoundType _soundType;
	char *_soundFilename;
	bool setSoundSimple();
	bool setSound(const char *filename, TSoundType type = TSoundType::SOUND_SFX, bool streamed = false);
	BaseSound(BaseGame *inGame);
	~BaseSound() override;

	bool applyFX(TSFXType type = SFX_NONE, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0);

	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;

private:
	BaseSoundBuffer *_sound;
};

} // End of namespace Wintermute

#endif
