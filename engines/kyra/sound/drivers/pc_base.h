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
	PCSoundDriver() {}
	virtual ~PCSoundDriver() {}

	virtual void initDriver() = 0;
	virtual void setSoundData(uint8 *data, uint32 size) = 0;
	virtual void queueTrack(int track, int volume) = 0;
	virtual bool isChannelPlaying(int channel) const = 0;
	virtual void stopAllChannels() = 0;
	virtual int getSoundTrigger() const = 0;
	virtual void resetSoundTrigger() = 0;

	virtual void callback() = 0;

	// AdLiB specific
	virtual void setSyncJumpMask(uint16) {}

	virtual void setMusicVolume(uint8 volume) = 0;
	virtual void setSfxVolume(uint8 volume) = 0;

	static PCSoundDriver *createAdLib(Audio::Mixer *mixer, int version);
#ifdef ENABLE_EOB
	static PCSoundDriver *createPCSpk(Audio::Mixer *mixer);
#endif
};

} // End of namespace Kyra

#endif
