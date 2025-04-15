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


#include "backends/audiocd/audiocd.h"
#include "common/system.h"

#include "snatcher/sound_device.h"
#include "snatcher/util.h"

namespace Snatcher {

class SegaSoundDevice : public SoundDevice {
public:
	SegaSoundDevice();
	~SegaSoundDevice() override {}

	void musicPlay(int track) override;
	void musicStop() override;
	bool musicIsPlaying() const override;
	uint32 musicGetTime() const override;

	void fmStartSound(int track) override;

	void pcmPlayEffect(int track) override;
	void pcmDoCommand(int cmd, int arg) override;

	void pause(bool toggle) override;

private:
	int _lastTrack;
	uint32 _pauseStartTime;
};

SegaSoundDevice::SegaSoundDevice() : SoundDevice(), _lastTrack(-1), _pauseStartTime(0) {

}

void SegaSoundDevice::musicPlay(int track) {
	assert(track > 0);
	g_system->getAudioCDManager()->play(track - 1, 1, 0, 0);
	_lastTrack = track;
	_musicStartTime = g_system->getMillis();
}

void SegaSoundDevice::musicStop() {
	g_system->getAudioCDManager()->stop();
	_lastTrack = -1;
}

bool SegaSoundDevice::musicIsPlaying() const {
	return g_system->getAudioCDManager()->isPlaying();
}

uint32 SegaSoundDevice::musicGetTime() const {
	uint32 relTime = musicIsPlaying() ? g_system->getMillis() - _musicStartTime : 0;
	return Util::makeBCDTimeStamp(relTime);
}

void SegaSoundDevice::fmStartSound(int track) {

}

void SegaSoundDevice::pcmPlayEffect(int track) {
}

void SegaSoundDevice::pcmDoCommand(int cmd, int arg) {
}


void SegaSoundDevice::pause(bool toggle) {
	if (toggle) {
		if (_pauseStartTime == 0) {
			if (musicIsPlaying()) {
				_pauseStartTime = g_system->getMillis();
				g_system->getAudioCDManager()->stop();
			} else {
				_lastTrack = -1;
				_pauseStartTime = 0;
			}
		}
	} else {
		if (_pauseStartTime != 0) {
			if (_lastTrack != -1)
				g_system->getAudioCDManager()->play(_lastTrack - 1, 1, (_pauseStartTime - _musicStartTime) / (1000 / 75), 0);
			int pauseDuration = g_system->getMillis() - _pauseStartTime;
			_musicStartTime += (pauseDuration - (pauseDuration % (1000 / 75)));
			_pauseStartTime = 0;
		}
	}
}

SoundDevice *SoundDevice::createSegaSoundDevice() {
	return new SegaSoundDevice();
}

} // End of namespace Snatcher
