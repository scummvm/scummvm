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

#ifndef WINTERMUTE_BSOUNDMGR_H
#define WINTERMUTE_BSOUNDMGR_H

#include "coll_templ.h"
#include "BBase.h"
#include "BSoundBuffer.h"
//#include "bass.h"

namespace WinterMute {

class CBSoundMgr : public CBBase {
public:
	float PosToPan(int X, int Y);
	HRESULT ResumeAll();
	HRESULT PauseAll(bool IncludingMusic = true);
	HRESULT Cleanup();
	//DECLARE_PERSISTENT(CBSoundMgr, CBBase);
	byte GetMasterVolumePercent();
	HRESULT SetMasterVolumePercent(byte  Percent);
	byte GetVolumePercent(TSoundType Type);
	HRESULT SetVolumePercent(TSoundType Type, byte Percent);
	HRESULT SetVolume(TSoundType Type, int Volume);
	uint32 _volumeOriginal;
	int _volumeMaster;
	int _volumeMusic;
	int _volumeSpeech;
	int _volumeSFX;
	HRESULT RemoveSound(CBSoundBuffer *Sound);
	CBSoundBuffer *AddSound(const char *Filename, TSoundType Type = SOUND_SFX, bool Streamed = false);
	HRESULT AddSound(CBSoundBuffer *Sound, TSoundType Type = SOUND_SFX);
	HRESULT InitLoop();
	HRESULT Initialize();
	bool _soundAvailable;
	CBSoundMgr(CBGame *inGame);
	virtual ~CBSoundMgr();
	CBArray<CBSoundBuffer *, CBSoundBuffer *> _sounds;
	void SaveSettings();
};

} // end of namespace WinterMute

#endif
