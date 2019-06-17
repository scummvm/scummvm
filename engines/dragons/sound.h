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
#ifndef DRAGONS_SOUND_H
#define DRAGONS_SOUND_H

#include "common/scummsys.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"


namespace Dragons {

class DragonsEngine;
struct SpeechLocation;

class Sound {
private:
	DragonsEngine *_vm;
	Audio::SoundHandle _speechHandle;

public:
	Sound(DragonsEngine *vm);
	void playSpeech(uint32 textIndex);
	bool isSpeechPlaying();
private:
	bool getSpeechLocation(uint32 talkId, struct SpeechLocation *location);

private:
	class PSXAudioTrack {
	public:
		PSXAudioTrack(Common::SeekableReadStream *sector, Audio::Mixer::SoundType soundType);
		~PSXAudioTrack();

		void queueAudioFromSector(Common::SeekableReadStream *sector);
		Audio::QueuingAudioStream *getAudioStream() {
			return _audStream;
		}
	private:
		Audio::QueuingAudioStream *_audStream;

		struct ADPCMStatus {
			int16 sample[2];
		} _adpcmStatus[2];
	};
};

} // End of namespace Dragons

#endif //DRAGONS_SOUND_H
