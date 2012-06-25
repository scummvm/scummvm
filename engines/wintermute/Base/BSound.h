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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BSOUND_H
#define WINTERMUTE_BSOUND_H

#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/Base/BSoundBuffer.h"
#include "engines/wintermute/persistent.h"

namespace WinterMute {

class CBSound : public CBBase {
public:
	HRESULT setPan(float pan);
	int _soundPrivateVolume;
	int getVolume();
	HRESULT setVolume(int volume);
	HRESULT setPrivateVolume(int volume);
	HRESULT setLoopStart(uint32 pos);
	uint32 getPositionTime();
	HRESULT setPositionTime(uint32 time);
	bool _soundPaused;
	bool _soundFreezePaused;
	bool isPlaying();
	bool isPaused();
	bool _soundPlaying;
	bool _soundLooping;
	uint32 _soundLoopStart;
	uint32 _soundPosition;
	DECLARE_PERSISTENT(CBSound, CBBase)
	HRESULT resume();
	HRESULT pause(bool freezePaused = false);
	HRESULT stop();
	HRESULT play(bool looping = false);
	uint32 getLength();
	bool _soundStreamed;
	TSoundType _soundType;
	char *_soundFilename;
	HRESULT setSoundSimple();
	HRESULT setSound(const char *filename, TSoundType type = SOUND_SFX, bool streamed = false);
	CBSound(CBGame *inGame);
	virtual ~CBSound();

	HRESULT ApplyFX(TSFXType type = SFX_NONE, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0);

private:
	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;
	CBSoundBuffer *_sound;

};

} // end of namespace WinterMute

#endif
