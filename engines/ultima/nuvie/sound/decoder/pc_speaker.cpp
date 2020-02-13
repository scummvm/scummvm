/*
 *  PCSpeaker.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Feb 13 2011.
 *  Copyright (c) 2011. All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/decoder/pc_speaker.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Nuvie {

#ifndef PI
#define PI 3.14159265358979323846
#endif


#define SPKR_VOLUME 5000
//#define SPKR_SHIFT 8
#define SPKR_SPEED (float)((SPKR_VOLUME*2)/0.070f)

#define PIT_TICK_RATE 1193182

void PCSpeaker::SetOn() {

	wav_length = 0;
	/*
	    //PCSPEAKER_SetType(3);
	    dataFile.open("/Users/efry/pcspeaker.wav");
	    dataFile.writeBuf((const unsigned char*)"RIFF", 4);
	    dataFile.write4(36 + wav_length * 2); //length of RIFF chunk
	    dataFile.writeBuf((const unsigned char*)"WAVE", 4);
	    dataFile.writeBuf((const unsigned char*)"fmt ", 4);
	    dataFile.write4(16); // length of format chunk
	    dataFile.write2(1); // PCM encoding
	    dataFile.write2(1); // mono
	    dataFile.write4(rate); // sample frequency 16KHz
	    dataFile.write4(rate * 2); // sample rate
	    dataFile.write2(2); // BlockAlign
	    dataFile.write2(16); // Bits per sample

	    dataFile.writeBuf((const unsigned char*)"data", 4);
	    dataFile.write4(wav_length * 2); // length of data chunk
	*/
	time_left = 0.0;
	want_vol = -SPKR_VOLUME;
}

void PCSpeaker::SetOff() {
	want_vol = 0;
	time_left = 0.0;
	/*
	    //PCSPEAKER_SetType(0);
	    dataFile.seek(4);
	    dataFile.write4(36 + wav_length * 2); //length of RIFF chunk
	    dataFile.seek(40);
	    dataFile.write4(wav_length * 2); // length of data chunk

	    dataFile.close();
	*/
}

void PCSpeaker::SetFrequency(uint16 freq, float offset) {
	//PCSPEAKER_SetCounter(PIT_TICK_RATE/freq, PIT_MODE_3_SQUAREWAVE);

	if (frequency == freq)
		return;

	frequency = freq;
	osc_length = rate / frequency;

	osc_samples = 0;
	half_period = ((float)rate / (float)frequency) / 2;
	//want_vol = SPKR_VOLUME;

	//DEBUG(0, LEVEL_DEBUGGING, "new_freq = %d half_period = %f time_left = %f", freq, half_period, time_left);

	//time_left = offset; //half_period;

}

PCSpeaker::PCSpeaker(uint32 mixer_rate) {
	rate = mixer_rate;
	cur_vol = 0.0f;
	want_vol = 0.0f; //SPKR_VOLUME;
	frequency = 0;
}


void PCSpeaker::PCSPEAKER_CallBack(sint16 *stream, const uint32 len) {
	uint32 i;

	for (i = 0; i < len; i++) {
		//float new_time_left = time_left - 1;

		if (cur_vol != want_vol) {
			if (time_left < 1.0f)
				cur_vol = cur_vol + ((want_vol * 8.3502) * time_left) / 2;
			else
				cur_vol = cur_vol + ((want_vol * 8.3502) / 2);

			if (cur_vol > SPKR_VOLUME || cur_vol < -SPKR_VOLUME) { //limit the volume to our max speaker range.
				cur_vol = want_vol;
			}
		}

		if (time_left <= 1.0f) { //we change wave direction in this time slice.
			//change wave edge.
			if (want_vol < 0)
				want_vol = SPKR_VOLUME;
			else
				want_vol = -SPKR_VOLUME;

			float remainder = (1.0f - time_left);

			if (remainder != 0.0f) { //calculate new current volume position
				cur_vol = cur_vol + ((want_vol * 8.3502) * remainder) / 2;

				if (cur_vol > SPKR_VOLUME || cur_vol < -SPKR_VOLUME) { //limit the volume to our max speaker range.
					cur_vol = want_vol;
				}
			}

			time_left = half_period - remainder;
			//DEBUG(0, LEVEL_DEBUGGING, "remainder = %f, time_left = %f\n", remainder, time_left);
		} else
			time_left = time_left - 1.0f;

		stream[i] = (sint16)cur_vol;
		//dataFile.write2(stream[i]);
		//wav_length++;
	}

	return;
}

} // End of namespace Nuvie
} // End of namespace Ultima

