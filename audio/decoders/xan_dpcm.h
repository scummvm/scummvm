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

/**
 * @file
 * Sound decoder used in engines:
 *  - ultima8 (Crusader: No Regret)
 */

#ifndef AUDIO_XAN_DPCM_H
#define AUDIO_XAN_DPCM_H

#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class StatelessPacketizedAudioStream;

/**
 * Implements the Xan DPCM decoder used in Crusader: No Regret and Wing
 * Commander IV movies.  Implementation based on the description on the
 * MultiMedia Wiki:
 * https://wiki.multimedia.cx/index.php/Xan_DPCM
 */
class XanDPCMStream : public StatelessPacketizedAudioStream {
public:
	/**
	 * Create a Xan DPCM stream
	 * @param rate              sampling rate (samples per second)
	 * @param channels  	number of channels to decode
	 * @return   a new XanDPCMStream, or NULL, if an error occurred
	 */
	 XanDPCMStream(int rate, int channels);

protected:
	AudioStream *makeStream(Common::SeekableReadStream *data) override;
};


} // End of namespace Audio

#endif
