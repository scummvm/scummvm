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

#ifndef MOHAWK_MYST_SOUND_H
#define MOHAWK_MYST_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"

#include "audio/mixer.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Mohawk {

struct CueList;
class MohawkEngine_Myst;

class MystSound {
public:
	explicit MystSound(MohawkEngine_Myst *vm);
	~MystSound();

	// Effect channel
	void playEffect(uint16 id, bool loop = false);
	void stopEffect();
	bool isEffectPlaying();

	// Background channel
	void playBackground(uint16 id, uint16 volume = 0xFFFF);
	void pauseBackground();
	void resumeBackground();
	void stopBackground();
	void changeBackgroundVolume(uint16 volume);

	// Speech channel
	void playSpeech(uint16 id, CueList *cueList = nullptr);
	bool isSpeechPlaying();
	uint getSpeechNumSamplesPlayed();
	void stopSpeech();

private:
	MohawkEngine_Myst *_vm;

	Audio::RewindableAudioStream *makeAudioStream(uint16 id, CueList *cueList = nullptr);
	uint16 convertMystID(uint16 id);

	Audio::SoundHandle _effectHandle;
	int _speechSamplesPerSecond;
	uint16 _effectId;

	Audio::SoundHandle _backgroundHandle;
	uint16 _backgroundId;

	Audio::SoundHandle _speechHandle;
};

} // End of namespace Mohawk

#endif
