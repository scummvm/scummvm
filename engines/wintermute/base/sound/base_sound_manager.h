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

#ifndef WINTERMUTE_BASE_SOUNDMGR_H
#define WINTERMUTE_BASE_SOUNDMGR_H

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/base/base.h"
#include "audio/mixer.h"
#include "common/array.h"

namespace Wintermute {
class BaseSoundBuffer;
class BaseSoundMgr : public BaseClass {
public:
	float posToPan(int x, int y);
	bool resumeAll();
	bool pauseAll(bool includingMusic = true);
	bool cleanup();
	//DECLARE_PERSISTENT(BaseSoundMgr, BaseClass)
	byte getMasterVolumePercent();
	byte getMasterVolume();
	bool setMasterVolumePercent(byte percent);
	byte getVolumePercent(TSoundType type);
	bool setVolumePercent(TSoundType type, byte percent);
	bool setVolume(TSoundType type, int volume);
	int32 _volumeMaster;
	bool removeSound(BaseSoundBuffer *sound);
	BaseSoundBuffer *addSound(const char *filename, TSoundType type = TSoundType::SOUND_SFX, bool streamed = false, uint32 initialPrivateVolume = 100);
	bool addSound(BaseSoundBuffer *sound, TSoundType type = TSoundType::SOUND_SFX);
	bool initLoop();
	bool initialize();
	bool _soundAvailable;
	BaseSoundMgr(BaseGame *inGame);
	~BaseSoundMgr() override;
	BaseArray<BaseSoundBuffer *> _sounds;
	void saveSettings();
};

} // End of namespace Wintermute

#endif
