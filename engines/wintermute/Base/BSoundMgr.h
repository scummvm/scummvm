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
#include "audio/mixer.h"
#include "common/array.h"

namespace WinterMute {
class CBSoundBuffer;
class CBSoundMgr : public CBBase {
public:
	float posToPan(int x, int y);
	bool resumeAll();
	bool pauseAll(bool includingMusic = true);
	bool cleanup();
	//DECLARE_PERSISTENT(CBSoundMgr, CBBase);
	byte getMasterVolumePercent();
	byte getMasterVolume();
	bool setMasterVolume(byte percent);
	bool setMasterVolumePercent(byte percent);
	byte getVolumePercent(Audio::Mixer::SoundType type);
	bool setVolumePercent(Audio::Mixer::SoundType type, byte percent);
	bool setVolume(Audio::Mixer::SoundType type, int volume);
	uint32 _volumeOriginal;
	int _volumeMaster;
	bool removeSound(CBSoundBuffer *sound);
	CBSoundBuffer *addSound(const char *filename, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType, bool streamed = false);
	bool addSound(CBSoundBuffer *sound, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType);
	bool initLoop();
	bool initialize();
	bool _soundAvailable;
	CBSoundMgr(CBGame *inGame);
	virtual ~CBSoundMgr();
	Common::Array<CBSoundBuffer *> _sounds;
	void saveSettings();
};

} // end of namespace WinterMute

#endif
