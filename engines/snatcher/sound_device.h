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

#ifndef SNATCHER_SOUND_DEVICE_H
#define SNATCHER_SOUND_DEVICE_H

#include "audio/mididrv.h"
#include "common/platform.h"

namespace Audio {
class Mixer;
}

namespace Snatcher {

class FIO;

class SoundDevice {
public:
	virtual ~SoundDevice() {}

	virtual bool init(Audio::Mixer *mixer) = 0;

	virtual void cdaPlay(int track) = 0;
	virtual void cdaStop() = 0;
	virtual bool cdaIsPlaying() const = 0;
	virtual uint32 cdaGetTime() const = 0;

	virtual void fmSendCommand(int cmd, int arg) = 0;
	virtual uint8 fmGetStatus() const = 0;

	virtual void pcmSendCommand(int cmd, int arg) = 0;
	virtual void pcmInitSound(int sndId) = 0;
	virtual uint8 pcmGetStatus() const = 0;

	virtual void pause(bool toggle) = 0;
	virtual void update() = 0;

	virtual void setMusicVolume(int vol) = 0;
	virtual void setSoundEffectVolume(int vol) = 0;

protected:
	SoundDevice() : _musicStartTime(0) {}

	uint32 _musicStartTime;

private:
	static SoundDevice *createSegaSoundDevice(FIO *fio);

public:
	static SoundDevice *create(FIO *fio, Common::Platform platform, int soundOptions);
};

} // End of namespace Snatcher

#endif // SNATCHER_SOUND_DEVICE_H
