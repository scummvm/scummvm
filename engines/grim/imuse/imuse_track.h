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

#ifndef GRIM_IMUSE_TRACK_H
#define GRIM_IMUSE_TRACK_H

#include "engines/grim/imuse/imuse_sndmgr.h"

namespace Grim {

enum {
	kFlagUnsigned = 1 << 0,
	kFlag16Bits = 1 << 1,
	kFlagLittleEndian = 1 << 2,
	kFlagStereo = 1 << 3,
	kFlagReverseStereo = 1 << 4
};

struct Track {
	int trackId;

	int32 pan;
	int32 panFadeDest;
	int32 panFadeStep;
	int32 panFadeDelay;
	bool panFadeUsed;
	int32 vol;
	int32 volFadeDest;
	int32 volFadeStep;
	int32 volFadeDelay;
	bool volFadeUsed;

	char soundName[32];
	bool used;
	bool toBeRemoved;
	int32 priority;
	int32 regionOffset;
	int32 dataOffset;
	int32 curRegion;
	int32 curHookId;
	int32 volGroupId;
	int32 feedSize;
	int32 mixerFlags;

	ImuseSndMgr::SoundDesc *soundDesc;
	Audio::SoundHandle handle;
	Audio::QueuingAudioStream *stream;

	Track() {
		clear();
	}

	void clear() {
		trackId = 0;

		pan = 0;
		panFadeDest = 0;
		panFadeStep = 0;
		panFadeDelay = 0;
		panFadeUsed = 0;
		vol = 0;
		volFadeDest = 0;
		volFadeStep = 0;
		volFadeDelay = 0;
		volFadeUsed = 0;

		for (uint i = 0; i < ARRAYSIZE(soundName); i++) {
			soundName[i] = 0;
		}
		used = false;
		toBeRemoved = false;
		priority = 0;
		regionOffset = 0;
		dataOffset = 0;
		curRegion = 0;
		curHookId = 0;
		volGroupId = 0;
		feedSize = 0;
		mixerFlags = 0;

		soundDesc = nullptr;
		// handle not cleared. FIXME: Clear by resetting _val to default (0xFFFFFFFF not 0)?
		stream = nullptr;
	}

	/* getPan() returns -127 ... 127 */
	int getPan() const { return (pan != 64000) ? 2 * (pan / 1000) - 127 : 0; }
	int getVol() const { return vol / 1000; }
	Audio::Mixer::SoundType getType() const {
		Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;
		if (volGroupId == IMUSE_VOLGRP_VOICE)
			type = Audio::Mixer::kSpeechSoundType;
		else if (volGroupId == IMUSE_VOLGRP_SFX)
			type = Audio::Mixer::kSFXSoundType;
		else if (volGroupId == IMUSE_VOLGRP_MUSIC)
			type = Audio::Mixer::kMusicSoundType;
		else if (volGroupId == IMUSE_VOLGRP_BGND)
			type = Audio::Mixer::kPlainSoundType;
		else if (volGroupId == IMUSE_VOLGRP_ACTION)
			type = Audio::Mixer::kPlainSoundType;
		return type;
	}
};

} // end of namespace Grim

#endif
