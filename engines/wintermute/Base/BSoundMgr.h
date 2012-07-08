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

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/Base/BBase.h"

namespace WinterMute {
class CBSoundBuffer;
class CBSoundMgr : public CBBase {
public:
	float posToPan(int x, int y);
	HRESULT resumeAll();
	HRESULT pauseAll(bool includingMusic = true);
	HRESULT cleanup();
	//DECLARE_PERSISTENT(CBSoundMgr, CBBase);
	byte getMasterVolumePercent();
	HRESULT setMasterVolumePercent(byte percent);
	byte getVolumePercent(TSoundType type);
	HRESULT setVolumePercent(TSoundType type, byte percent);
	HRESULT setVolume(TSoundType type, int volume);
	uint32 _volumeOriginal;
	int _volumeMaster;
	int _volumeMusic;
	int _volumeSpeech;
	int _volumeSFX;
	HRESULT removeSound(CBSoundBuffer *sound);
	CBSoundBuffer *addSound(const char *filename, TSoundType type = SOUND_SFX, bool streamed = false);
	HRESULT addSound(CBSoundBuffer *sound, TSoundType type = SOUND_SFX);
	HRESULT initLoop();
	HRESULT initialize();
	bool _soundAvailable;
	CBSoundMgr(CBGame *inGame);
	virtual ~CBSoundMgr();
	CBArray<CBSoundBuffer *, CBSoundBuffer *> _sounds;
	void saveSettings();
};

} // end of namespace WinterMute

#endif
