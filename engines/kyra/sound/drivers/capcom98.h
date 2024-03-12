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

#ifdef ENABLE_EOB

#ifndef KYRA_SOUND_CAPCOM98_H
#define KYRA_SOUND_CAPCOM98_H

#include "audio/mididrv.h"
#include "common/scummsys.h"

namespace Audio {
	class Mixer;
}

namespace Kyra {

class CapcomPC98AudioDriverInternal;

class CapcomPC98AudioDriver {
public:
	CapcomPC98AudioDriver(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev);
	~CapcomPC98AudioDriver();

	bool isUsable() const;

	// All data passed to the following functions has to be maintained by the caller.
	void reset();
	void loadFMInstruments(const uint8 *data);
	void startSong(const uint8 *data, uint8 volume, bool loop);
	void stopSong();
	void startSoundEffect(const uint8 *data, uint8 volume);
	void stopSoundEffect();

	int checkSoundMarker() const;
	bool songIsPlaying() const;
	bool soundEffectIsPlaying() const;

	void fadeOut();
	void allNotesOff();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	CapcomPC98AudioDriverInternal *_drv;
};

} // End of namespace Kyra

#endif

#endif
