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
	int m_SoundPrivateVolume;
	int GetVolume();
	HRESULT SetVolume(int Volume);
	HRESULT SetPrivateVolume(int Volume);
	HRESULT SetLoopStart(uint32 Pos);
	uint32 GetPositionTime();
	HRESULT SetPositionTime(uint32 Time);
	bool m_SoundPaused;
	bool m_SoundFreezePaused;
	bool IsPlaying();
	bool IsPaused();
	bool m_SoundPlaying;
	bool m_SoundLooping;
	uint32 m_SoundLoopStart;
	uint32 m_SoundPosition;
	DECLARE_PERSISTENT(CBSound, CBBase)
	HRESULT Resume();
	HRESULT Pause(bool FreezePaused = false);
	HRESULT Stop();
	HRESULT Play(bool Looping = false);
	uint32 GetLength();
	bool m_SoundStreamed;
	TSoundType m_SoundType;
	char *m_SoundFilename;
	HRESULT SetSoundSimple();
	HRESULT SetSound(char *Filename, TSoundType Type = SOUND_SFX, bool Streamed = false);
	CBSound(CBGame *inGame);
	virtual ~CBSound();

	HRESULT ApplyFX(TSFXType Type = SFX_NONE, float Param1 = 0, float Param2 = 0, float Param3 = 0, float Param4 = 0);

private:
	TSFXType m_SFXType;
	float m_SFXParam1;
	float m_SFXParam2;
	float m_SFXParam3;
	float m_SFXParam4;
	CBSoundBuffer *m_Sound;

};

} // end of namespace WinterMute

#endif
