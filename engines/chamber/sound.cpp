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

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/sound.h"

namespace Chamber {

typedef struct pcsample_t {
	unsigned short repeat;
	unsigned short delay1;
	unsigned short delay1sweep;
	unsigned short delay2;
	unsigned short delay2sweep;
	unsigned short freq;
	unsigned short freqsweep;
} pcsample_t;

#define N(x) (0xF000|x)

pcsample_t pc_samples[] = {
	{ 64,   32,      0,   32,      0,    256,      0},
	{  1, 2560,      0, 2560,      0,      0,      0},
	{  8, 2048,      0,  256,    256,      0,      0},
	{ 11, 2816, N(256), 2816, N(256),    256,    256},
	{  1,  768,      0,  768,      0,    256,      0},
	{128,   48,      1,   48,      0, 61440u,      1},
	{128,   80,      1,   80,      0,      1,      1},
	{128,  128,      1,  128,      1,     16,      1},
	{128,  128,      1,  128,      0,    256,     16},
	{128,  128,      1,  128,      1, 61440u,  N(16)},
	{ 16, 1536,      0, 1536,      0,      0,      0},
	{  3,  768,      0,  768, N(256),    256, N(256)},
};

#undef N

static void SpeakerPlay(pcsample_t *sample) {
	uint16 rep, freq, delay1, delay2;

	freq = sample->freq;
	delay1 = sample->delay1;
	delay2 = sample->delay2;

	for (rep = 0; rep < sample->repeat; rep++) {
		uint32 frequency = 1193180 / freq;
		uint32 delayOff = delay1 * 300;
		uint32 delayOn = delay2 * 300;

		warning("freq: %d delayOff: %d delayOn: %d", frequency, delayOff, delayOn);

		g_vm->_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, 1, delayOff);
		g_vm->_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, frequency, delayOn);

		if (sample->delay1sweep & 0xF000)
			delay1 -= sample->delay1sweep & 0xFFF;
		else
			delay1 += sample->delay1sweep;

		if (sample->delay2sweep & 0xF000)
			delay2 -= sample->delay2sweep & 0xFFF;
		else
			delay2 += sample->delay2sweep;

		if (sample->freqsweep & 0xF000)
			freq -= sample->freqsweep & 0xFFF;
		else
			freq += sample->freqsweep;
	}
}

#define kMaxSounds 12

unsigned char sounds_table[kMaxSounds][3] = {
	{20, 0, 0},
	{19, 0, 0},
	{176, 0, 0},
	{144, 145, 146},
	{243, 0, 0},
	{18, 0, 0},
	{149, 21, 0},
	{27, 0, 0},
	{241, 25, 151},
	{22, 0, 0},
	{224, 0, 0},
	{31, 0, 0}
};

void PlaySound(unsigned char index) {
	SpeakerPlay(&pc_samples[index]);

#if 0
	int i;
	for (i = 0; i < kMaxSounds; i++) {
		if (sounds_table[i][0] == index
		        || sounds_table[i][1] == index
		        || sounds_table[i][2] == index) {
			SpeakerPlay(&pc_samples[i]);
			break;
		}
	}
#endif
}

void ChamberEngine::initSound() {
	// Setup mixer
	syncSoundSettings();

	_speakerHandle = new Audio::SoundHandle();
	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, _speakerHandle,
		_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

void ChamberEngine::deinitSound() {
	_mixer->stopHandle(*_speakerHandle);
	delete g_vm->_speakerHandle;
	delete g_vm->_speakerStream;
}

} // End of namespace Chamber
