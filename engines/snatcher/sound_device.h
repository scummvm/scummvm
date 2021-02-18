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

#include "audio//mididrv.h"
#include "common/platform.h"

namespace Snatcher {

class SoundDevice {
public:
	virtual ~SoundDevice() {}

protected:
	SoundDevice() {}

private:
	static SoundDevice *createNullSoundDevice();
	static SoundDevice *createSegaSoundDevice();

public:
	static SoundDevice *create(Common::Platform platform, int soundOptions) {
		MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(soundOptions);
		MusicType musicType = MidiDriver::getMusicType(dev);

		if (musicType == MT_INVALID || musicType == MT_NULL)
			return createNullSoundDevice();

		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSegaSoundDevice();
		default:
			break;
		};

		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_SOUND_DEVICE_H
