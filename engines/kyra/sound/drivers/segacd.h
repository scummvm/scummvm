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

#ifdef ENABLE_EOB

#ifndef KYRA_SOUND_SEGACD_H
#define KYRA_SOUND_SEGACD_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {
	class SeekableReadStream;
}

namespace Audio {
	class Mixer;
}

namespace Kyra {

class SegaAudioDriverInternal;

class SegaAudioDriver {
public:
	SegaAudioDriver(Audio::Mixer *mixer);
	~SegaAudioDriver();

	enum PrioFlags {
		kPrioHigh = 0x10,
		kPrioLow = 0x20
	};

	void startFMSound(const uint8 *trackData, uint8 volume, PrioFlags prioFlags);

	void loadPCMData(uint16 address, const uint8 *data, uint16 dataLen);
	void startPCMSound(uint8 channel, uint8 dataStart, uint16 loopStart = 0xFF00, uint16 rate = 0x300, uint8 pan = 0xFF, uint8 vol = 0xFF);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	SegaAudioDriverInternal *_drv;
};

} // End of namespace Kyra

#endif

#endif
