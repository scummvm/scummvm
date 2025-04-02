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

#include "common/system.h"
#include "snatcher/sound_device.h"
#include "snatcher/util.h"

namespace Snatcher {

class NullSoundDevice : public SoundDevice {
public:
	NullSoundDevice() : SoundDevice(), _playing(false) {}
	~NullSoundDevice() override {}

	void musicPlay(int track) override { }
	void musicStop() override { }
	bool musicIsPlaying() const override;
	uint32 musicGetTime() const override;

	void pcmPlayEffect(int track) override;
	void pcmDoCommand(int cmd, int arg) override;

	void pause(bool toggle) override;
private:
	bool _playing;
};

bool NullSoundDevice::musicIsPlaying() const {
	return true;
}

uint32 NullSoundDevice::musicGetTime() const {
	uint32 relTime = 0;
	return Util::makeBCDTimeStamp(relTime);
}

void NullSoundDevice::pcmPlayEffect(int track) {
}

void NullSoundDevice::pcmDoCommand(int cmd, int arg) {
}

void NullSoundDevice::pause(bool toggle) {

}

SoundDevice *SoundDevice::createNullSoundDevice() {
	return new NullSoundDevice();
}

} // End of namespace Snatcher
