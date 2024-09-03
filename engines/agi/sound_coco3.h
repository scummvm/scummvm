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

#ifndef AGI_SOUND_COCO3_H
#define AGI_SOUND_COCO3_H

#include "audio/audiostream.h"
#include "audio/softsynth/pcspk.h"

namespace Agi {

class SoundGenCoCo3 : public SoundGen, public Audio::AudioStream {
public:
	SoundGenCoCo3(AgiBase *vm, Audio::Mixer *pMixer);
	~SoundGenCoCo3() override;

	void play(int resnum) override;
	void stop() override;

	int readBuffer(int16 *buffer, const int numSamples) override;

	bool isStereo() const override {
		return false;
	}

	bool endOfData() const override {
		return false;
	}

	int getRate() const override {
		return _speaker.getRate();
	}

private:
	Common::Mutex _mutex;
	bool _isPlaying;
	Audio::PCSpeaker _speaker;
};

} // End of namespace Agi

#endif /* AGI_SOUND_COCO3_H */
