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

#ifndef SKY_MUSIC_ADLIBMUSIC_H
#define SKY_MUSIC_ADLIBMUSIC_H

#include "sky/music/musicbase.h"

namespace Audio {
class Mixer;
}

namespace OPL {
class OPL;
}

namespace Sky {

class AdLibMusic : public MusicBase {
public:
	AdLibMusic(Audio::Mixer *pMixer, Disk *pDisk);
	~AdLibMusic() override;

	// AudioStream API
	void setVolume(uint16 param) override;

private:
	OPL::OPL *_opl;
	uint8 *_initSequence;
	uint32 _sampleRate;
	void setupPointers() override;
	void setupChannels(uint8 *channelData) override;
	void startDriver() override;

	void premixerCall(int16 *buf, uint len);

	void onTimer();
};

} // End of namespace Sky

#endif //ADLIBMUSIC_H
