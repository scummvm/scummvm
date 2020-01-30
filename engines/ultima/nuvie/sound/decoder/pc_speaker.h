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

#ifndef NUVIE_SOUND_MIXER_DECODER_PC_SPEAKER_H
#define NUVIE_SOUND_MIXER_DECODER_PC_SPEAKER_H

#include "ultima/nuvie/files/nuvie_io_file.h"

namespace Ultima {
namespace Nuvie {

#define SPKR_OUTPUT_RATE 22050 //11025 //FIXME may need to fiddle with this.

class PCSpeaker {
private:
	uint32 rate;
	uint16 frequency;
	float half_period;
	float cur_vol;
	float want_vol;
	float time_left;

	uint32 osc_length;
	uint32 osc_samples;

private:

	NuvieIOFileWrite dataFile;
	uint32 wav_length;

public:
	PCSpeaker(uint32 mixer_rate);
	~PCSpeaker() { }
	void SetOn();
	void SetOff();
	void SetFrequency(uint16 freq, float offset = 0.0f);

	void PCSPEAKER_CallBack(sint16 *stream, const uint32 len);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
