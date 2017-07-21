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

#include "mohawk/sound.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Mohawk {

class MohawkEngine_Myst;

class MystSound {
public:
	MystSound(MohawkEngine_Myst *vm);
	~MystSound();

	// Generic sound functions
	Audio::SoundHandle *playSound(uint16 id, byte volume = Audio::Mixer::kMaxChannelVolume, bool loop = false, CueList *cueList = NULL);
	void stopSound();
	void stopSound(uint16 id);
	bool isPlaying(uint16 id);
	bool isPlaying();
	uint getNumSamplesPlayed(uint16 id);

	// Myst-specific sound functions
	Audio::SoundHandle *replaceSoundMyst(uint16 id, byte volume = Audio::Mixer::kMaxChannelVolume, bool loop = false);
	void replaceBackgroundMyst(uint16 id, uint16 volume = 0xFFFF);
	void pauseBackgroundMyst();
	void resumeBackgroundMyst();
	void stopBackgroundMyst();
	void changeBackgroundVolumeMyst(uint16 vol);

private:
	MohawkEngine_Myst *_vm;

	Common::Array<SndHandle> _handles;
	SndHandle *getHandle();
	Audio::RewindableAudioStream *makeAudioStream(uint16 id, CueList *cueList = NULL);
	uint16 convertMystID(uint16 id);

	// Myst-specific
	SndHandle _mystBackgroundSound;
};

} // End of namespace Mohawk

#endif
