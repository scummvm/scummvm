/* Created by Eric Fry
 * Copyright (C) 2011 The Nuvie Team
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
#include "ultima/nuvie/sound/decoder/pc_speaker_stream.h"
#include "ultima/nuvie/core/game.h"

namespace Ultima {
namespace Nuvie {

PCSpeakerFreqStream::PCSpeakerFreqStream(uint freq, uint16 d) {
	frequency = freq;

	duration = d * (SPKR_OUTPUT_RATE / 1255);
	if (freq != 0) {
		pcspkr->SetOn();
		pcspkr->SetFrequency(frequency);
	}

	total_samples_played = 0;
}


PCSpeakerFreqStream::~PCSpeakerFreqStream() {

}

uint32 PCSpeakerFreqStream::getLengthInMsec() {
	return (uint32)(duration / (getRate() / 1000.0f));
}

int PCSpeakerFreqStream::readBuffer(sint16 *buffer, const int numSamples) {
	uint32 samples = (uint32)numSamples;

	if (total_samples_played >= duration)
		return 0;

	if (total_samples_played + samples > duration)
		samples = duration - total_samples_played;

	if (frequency != 0)
		pcspkr->PCSPEAKER_CallBack(buffer, samples);
	else
		memset(buffer, 0, sizeof(sint16)*numSamples);

	total_samples_played += samples;

	if (total_samples_played >= duration) {
		finished = true;
		pcspkr->SetOff();
	}

	return samples;
}

//******** PCSpeakerSweepFreqStream

PCSpeakerSweepFreqStream::PCSpeakerSweepFreqStream(uint start, uint end, uint16 d, uint16 s) {
	start_freq = start;
	finish_freq = end;
	cur_freq = start_freq;

	num_steps = d / s;
	freq_step = ((finish_freq - start_freq) * s) / d;
	stepping = s;
	duration = d * (SPKR_OUTPUT_RATE / 1255);
	samples_per_step = (float)s * (SPKR_OUTPUT_RATE * 0.000879533f); //(2 * (uint32)(SPKR_OUTPUT_RATE / start_freq)); //duration / num_steps;
	sample_pos = 0.0f;
	pcspkr->SetOn();
	pcspkr->SetFrequency(start_freq);


	total_samples_played = 0;
	cur_step = 0;
	DEBUG(0, LEVEL_DEBUGGING, "num_steps = %d freq_step = %d samples_per_step = %f\n", num_steps, freq_step, samples_per_step);
}


PCSpeakerSweepFreqStream::~PCSpeakerSweepFreqStream() {

}

uint32 PCSpeakerSweepFreqStream::getLengthInMsec() {
	return (uint32)((num_steps * samples_per_step) / (getRate() / 1000.0f));
}

int PCSpeakerSweepFreqStream::readBuffer(sint16 *buffer, const int numSamples) {
	uint32 samples = (uint32)numSamples;
	uint32 i;
	//if(total_samples_played >= duration)
	//  return 0;

	//if(total_samples_played + samples > duration)
	//  samples = duration - total_samples_played;

	for (i = 0; i < samples && cur_step < num_steps;) {
		//DEBUG(0, LEVEL_DEBUGGING, "sample_pos = %f\n", sample_pos);
		float n = samples_per_step - sample_pos;
		if ((float)i + n > (float)samples)
			n = (float)(samples - i);

		float remainder = n - floor(n);
		n = floor(n);
		pcspkr->PCSPEAKER_CallBack(&buffer[i], (uint32)n);
		sample_pos += n;

		i += (uint32)n;
		//DEBUG(0, LEVEL_DEBUGGING, "sample_pos = %f remainder = %f\n", sample_pos, remainder);
		if (sample_pos + remainder >= samples_per_step) {
			cur_freq += freq_step;

			pcspkr->SetFrequency(cur_freq, remainder);

			if (remainder != 0.0f) {
				sample_pos = 1.0f - remainder;
				pcspkr->PCSPEAKER_CallBack(&buffer[i], 1);
				i++;
			} else {
				sample_pos = 0;
			}

			cur_step++;
		}

	}

	total_samples_played += i;

	if (cur_step >= num_steps) { //total_samples_played >= duration)
		DEBUG(0, LEVEL_DEBUGGING, "total_samples_played = %d cur_freq = %d\n", total_samples_played, cur_freq);
		finished = true;
		pcspkr->SetOff();
	}

	return i;
}


//**************** PCSpeakerRandomStream

PCSpeakerRandomStream::PCSpeakerRandomStream(uint freq, uint16 d, uint16 s) {
	rand_value = 0x7664;
	base_val = freq;
	/*
	frequency = freq;

	duration = d * (SPKR_OUTPUT_RATE / 1255);

	pcspkr->SetFrequency(frequency);
	pcspkr->SetOn();

	total_samples_played = 0;
	*/

	pcspkr->SetOn();
	pcspkr->SetFrequency(getNextFreqValue());

	cur_step = 0;
	sample_pos = 0;
	num_steps = d / s;
	samples_per_step = s * (SPKR_OUTPUT_RATE / 20 / 800); //1255);
	total_samples_played = 0;
	DEBUG(0, LEVEL_DEBUGGING, "num_steps = %d samples_per_step = %d\n", num_steps, samples_per_step);

}


PCSpeakerRandomStream::~PCSpeakerRandomStream() {

}

uint32 PCSpeakerRandomStream::getLengthInMsec() {
	return (uint32)((num_steps * samples_per_step) / (getRate() / 1000.0f));
}

uint16 PCSpeakerRandomStream::getNextFreqValue() {
	rand_value += 0x9248;
	rand_value = rand_value & 0xffff; //clamp_max(rand_value, 65535);
	uint16 bits = rand_value & 0x7;
	rand_value = (rand_value >> 3) + (bits << 13); //rotate rand_value right (ror) by 3 bits
	rand_value = rand_value ^ 0x9248;
	rand_value += 0x11;
	rand_value = rand_value & 0xffff; //clamp_max(rand_value, 65535);

	uint16 freq = base_val - 0x64 + 1;
	uint16 tmp = rand_value;
	freq = tmp - floor(tmp / freq) * freq;
	freq += 0x64;

	return freq;
}

int PCSpeakerRandomStream::readBuffer(sint16 *buffer, const int numSamples) {
	uint32 samples = (uint32)numSamples;
	uint32 s = 0;
	//if(total_samples_played >= duration)
	//  return 0;

	//if(total_samples_played + samples > duration)
	//  samples = duration - total_samples_played;

	for (uint32 i = 0; i < samples && cur_step <= num_steps;) {
		uint32 n = samples_per_step - sample_pos;
		if (i + n > samples)
			n = samples - i;

		pcspkr->PCSPEAKER_CallBack(&buffer[i], n);
		sample_pos += n;
		i += n;
//		DEBUG(0, LEVEL_DEBUGGING, "n = %d\n", n);
		if (sample_pos >= samples_per_step) {
			//DEBUG(0, LEVEL_DEBUGGING, "samples_per_step = %d period = %d\n", samples_per_step, period);
			pcspkr->SetFrequency(getNextFreqValue());
			sample_pos = 0;
			cur_step++;
		}

		s += n;

	}

	total_samples_played += s;

	if (cur_step >= num_steps) { //total_samples_played >= duration)
		DEBUG(0, LEVEL_DEBUGGING, "total_samples_played = %d\n", total_samples_played);
		finished = true;
		pcspkr->SetOff();
	}

	return s;
}

//**************** PCSpeakerStutterStream

PCSpeakerStutterStream::PCSpeakerStutterStream(sint16 a0, uint16 a2, uint16 a4, uint16 a6, uint16 a8) {
	arg_0 = a0;
	arg_2 = a2;
	arg_4 = a4;
	arg_6 = a6;
	arg_8 = a8;

	cx = arg_4;
	dx = 0;

	pcspkr->SetOn();
	pcspkr->SetFrequency(22096);

	delay = (SPKR_OUTPUT_RATE / 22050) * arg_6; //FIXME this needs to be refined.
	delay_remaining = 0;

	//samples_per_step = s * (SPKR_OUTPUT_RATE / 20 / 800); //1255);
	//total_samples_played = 0;
	//DEBUG(0, LEVEL_DEBUGGING, "num_steps = %d samples_per_step = %d\n", num_steps, samples_per_step);

}


PCSpeakerStutterStream::~PCSpeakerStutterStream() {

}

uint32 PCSpeakerStutterStream::getLengthInMsec() {
	return (uint32)((arg_4 * delay) / (getRate() / 1000.0f));
}


int PCSpeakerStutterStream::readBuffer(sint16 *buffer, const int numSamples) {
	uint32 s = 0;

	for (; cx > 0 && s < (uint32)numSamples; cx--) {
		uint32 n = (uint32)floor(delay_remaining);
		if (n > 0) {
			pcspkr->PCSPEAKER_CallBack(&buffer[s], n);
			delay_remaining -= n;
			s += n;
		}

		dx = (dx + arg_8) & 0xffff;

		if (dx > arg_2) {
			pcspkr->SetOn();
		} else {
			pcspkr->SetOff();
		}

		arg_2 += arg_0;
		/*
		       for(int i = arg_6; i > 0 ; i--)
		       {
		          for(int j = counter;j > 0;)
		          {
		             j--;
		          }
		       }
		*/
		n = (uint32)floor(delay);
		if (s + n > (uint32)numSamples)
			n = numSamples - s;

		pcspkr->PCSPEAKER_CallBack(&buffer[s], n);
		delay_remaining = delay - n;
		s += n;
	}

	if (cx <= 0) {
		//DEBUG(0, LEVEL_DEBUGGING, "total_samples_played = %d\n", total_samples_played);
		finished = true;
		pcspkr->SetOff();
	}

	return s;
}

Audio::AudioStream *makePCSpeakerSlugDissolveSfxStream(uint /*rate*/) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);
	for (uint16 i = 0; i < 20; i++) {
		stream->queueAudioStream(new PCSpeakerRandomStream((NUVIE_RAND() % 0x1068) + 0x258, 0x15e, 1), DisposeAfterUse::YES);
	}

	return stream;
}

Audio::AudioStream *makePCSpeakerGlassSfxStream(uint /*rate*/) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);
	for (uint16 i = 0x7d0; i < 0x4e20; i += 0x3e8) {
		stream->queueAudioStream(new PCSpeakerRandomStream(i, 0x78, 0x28), DisposeAfterUse::YES);
	}

	return stream;
}


Audio::AudioStream *makePCSpeakerMagicCastingP1SfxStream(uint /*rate*/, uint8 magic_circle) {
	//Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(rate, false);

	return new PCSpeakerRandomStream(0x2bc, 0x640 * magic_circle + 0x1f40, 0x320);

	//return stream;
}

Audio::AudioStream *makePCSpeakerMagicCastingP2SfxStream(uint /*rate*/, uint8 magic_circle) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);

	const sint16 word_30188[] = {3, 2, 2, 2, 1, 1, 1, 1, 1};

	const uint16 word_30164[] = {0xA8C, 0xBB8, 0x3E8, 0x64, 0x1388, 0xFA0, 0x9C4, 0x3E8, 1};
	const uint16 word_30176[] = {0x7FBC, 0x7918, 0x9088, 0xAFC8, 0x7918, 0x84D0, 0x8E94, 0x9858, 0xA410};

	const uint16 word_30152[] = {0x226A, 0x1E96, 0x1B94, 0x1996, 0x173E, 0x15C2, 0x143C, 0x12D4, 0x1180};

	stream->queueAudioStream(new PCSpeakerStutterStream(word_30188[magic_circle], word_30164[magic_circle], (magic_circle * 0xfa0) + 0x2710, 1, word_30152[magic_circle]));
	stream->queueAudioStream(new PCSpeakerStutterStream(-word_30188[magic_circle], word_30176[magic_circle], (magic_circle * 0xfa0) + 0x2710, 1, word_30152[magic_circle]));

	return stream;
}

Audio::AudioStream *makePCSpeakerAvatarDeathSfxStream(uint /*rate*/) {
	const uint16 avatar_death_tune[] = {0x12C, 0x119, 0x12C, 0xFA, 0x119, 0xDE, 0xFA, 0xFA};

	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);
	for (uint8 i = 0; i < 8; i++) {
		stream->queueAudioStream(new PCSpeakerStutterStream(3, 1, 0x4e20, 1, avatar_death_tune[i]));
	}

	return stream;
}

Audio::AudioStream *makePCSpeakerKalLorSfxStream(uint /*rate*/) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);
	for (uint8 i = 0; i < 0x32; i++) {
		stream->queueAudioStream(new PCSpeakerStutterStream((0x32 - i) << 2, 0x2710 - (i << 6), 0x3e8, 1, (i << 4) + 0x320));
	}

	stream->queueAudioStream(new PCSpeakerStutterStream(8, 0, 0x1f40, 1, 0x640));

	return stream;
}

Audio::AudioStream *makePCSpeakerHailStoneSfxStream(uint /*rate*/) {
	//FIXME This doesn't sound right. It should probably use a single
	// pcspkr object. The original also plays the hailstones
	// individually, not all at once like we do. :(
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);

	for (uint16 i = 0; i < 0x28; i++) {
		stream->queueAudioStream(new PCSpeakerFreqStream((NUVIE_RAND() % 0x28) + 0x20, 8), DisposeAfterUse::YES);
	}

	/* The original logic looks something like this. But this doesn't sound right.
	uint16 base_freq = (NUVIE_RAND()%0x64)+0x190;
	for(uint16 i=0;i<0x28;i++)
	{
	    if(NUVIE_RAND()%7==0)
	        stream->queueAudioStream(new PCSpeakerFreqStream(base_freq + (NUVIE_RAND()%0x28), 8), DisposeAfterUse::YES);
	    else
	        stream->queueAudioStream(new PCSpeakerFreqStream(0, 8), DisposeAfterUse::YES);
	}
	 */

	return stream;
}

Audio::AudioStream *makePCSpeakerEarthQuakeSfxStream(uint /*rate*/) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SPKR_OUTPUT_RATE, false);

	for (uint16 i = 0; i < 0x28; i++) {
		stream->queueAudioStream(new PCSpeakerFreqStream((NUVIE_RAND() % 0xb5) + 0x13, 8), DisposeAfterUse::YES);
	}

	return stream;
}

} // End of namespace Nuvie
} // End of namespace Ultima
