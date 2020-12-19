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


#ifndef KYRA_SOUND_PCDRIVER_H
#define KYRA_SOUND_PCDRIVER_H

#include "kyra/resource/resource.h"

namespace Audio {
	class Mixer;
}

namespace Kyra {

class PCSoundDriver {
public:
	PCSoundDriver() : _soundData(0), _soundDataSize(0) {}
	virtual ~PCSoundDriver() {}

	virtual void initDriver() = 0;
	virtual void setSoundData(uint8 *data, uint32 size) = 0;
	virtual void startSound(int track, int volume) = 0;
	virtual bool isChannelPlaying(int channel) const = 0;
	virtual void stopAllChannels() = 0;

	virtual int getSoundTrigger() const { return 0; }
	virtual void resetSoundTrigger() {}

	virtual void setMusicVolume(uint8 volume) = 0;
	virtual void setSfxVolume(uint8 volume) = 0;

	// AdLiB (Kyra 1) specific
	virtual void setSyncJumpMask(uint16) {}

protected:
	uint8 *getProgram(int progId) {
		// Safety check: invalid progId would crash.
		if (progId < 0 || progId >= (int32)_soundDataSize / 2)
			return nullptr;

		const uint16 offset = READ_LE_UINT16(_soundData + 2 * progId);

		// In case an invalid offset is specified we return nullptr to
		// indicate an error. 0xFFFF seems to indicate "this is not a valid
		// program/instrument". However, 0 is also invalid because it points
		// inside the offset table itself. We also ignore any offsets outside
		// of the actual data size.
		// The original does not contain any safety checks and will simply
		// read outside of the valid sound data in case an invalid offset is
		// encountered.
		if (offset == 0 || offset >= _soundDataSize) {
			return nullptr;
		} else {
			return _soundData + offset;
		}
	}

	uint8 *_soundData;
	uint32 _soundDataSize;

public:
	static PCSoundDriver *createAdLib(Audio::Mixer *mixer, int version);
#ifdef ENABLE_EOB
	static PCSoundDriver *createPCSpk(Audio::Mixer *mixer, bool pcJRMode);
#endif
};

} // End of namespace Kyra

#endif
