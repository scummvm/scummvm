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

#include "BBase.h"
#include "dctypes.h"    // Added by ClassView
#include "BSoundBuffer.h"
#include "persistent.h"

namespace WinterMute {

class CBSound : public CBBase {
public:
	HRESULT SetPan(float Pan);
	int _soundPrivateVolume;
	int GetVolume();
	HRESULT SetVolume(int Volume);
	HRESULT SetPrivateVolume(int Volume);
	HRESULT SetLoopStart(uint32 Pos);
	uint32 GetPositionTime();
	HRESULT SetPositionTime(uint32 Time);
	bool _soundPaused;
	bool _soundFreezePaused;
	bool IsPlaying();
	bool IsPaused();
	bool _soundPlaying;
	bool _soundLooping;
	uint32 _soundLoopStart;
	uint32 _soundPosition;
	DECLARE_PERSISTENT(CBSound, CBBase)
	HRESULT Resume();
	HRESULT Pause(bool FreezePaused = false);
	HRESULT Stop();
	HRESULT Play(bool Looping = false);
	uint32 GetLength();
	bool _soundStreamed;
	TSoundType _soundType;
	char *_soundFilename;
	HRESULT SetSoundSimple();
	HRESULT SetSound(const char *Filename, TSoundType Type = SOUND_SFX, bool Streamed = false);
	CBSound(CBGame *inGame);
	virtual ~CBSound();

	HRESULT ApplyFX(TSFXType Type = SFX_NONE, float Param1 = 0, float Param2 = 0, float Param3 = 0, float Param4 = 0);

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
