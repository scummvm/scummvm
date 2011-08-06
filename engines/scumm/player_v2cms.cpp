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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "scumm/player_v2cms.h"
#include "scumm/scumm.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"
#include "audio/softsynth/cms.h"

namespace Scumm {

#define PROCESS_ATTACK 1
#define PROCESS_RELEASE 2
#define PROCESS_SUSTAIN 3
#define PROCESS_DECAY 4
#define PROCESS_VIBRATO 5

#define CMS_RATE 22050

static const byte freqTable[] = {
	  3,  10,  17,  24,  31,  38,  45,  51,
	 58,  65,  71,  77,  83,  90,  96, 102,
	107, 113, 119, 125, 130, 136, 141, 146,
	151, 157, 162, 167, 172, 177, 181, 186,
	191, 195, 200, 204, 209, 213, 217, 221,
	226, 230, 234, 238, 242, 246, 249, 253
};

/*static const byte amplTable[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0 %
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 10 %
	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10,	// 20 %
	0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x20,	// 30%
	0x20, 0x20, 0x30, 0x30, 0x30, 0x30, 0x40, 0x40,
	0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x20,	// 40 %
	0x30, 0x30, 0x40, 0x40, 0x40, 0x50, 0x50, 0x60,
	0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x30, 0x30,	// 50%
	0x40, 0x40, 0x50, 0x50, 0x60, 0x60, 0x70, 0x70,
	0x00, 0x00, 0x10, 0x10, 0x20, 0x30, 0x30, 0x40,	// 60 %
	0x40, 0x50, 0x60, 0x60, 0x70, 0x70, 0x80, 0x90,
	0x00, 0x00, 0x10, 0x20, 0x20, 0x30, 0x40, 0x40,	// 70 %
	0x50, 0x60, 0x70, 0x70, 0x80, 0x90, 0x90, 0xA0,
	0x00, 0x00, 0x10, 0x20, 0x30, 0x40, 0x40, 0x50,	// 80 %
	0x60, 0x70, 0x80, 0x80, 0x90, 0xA0, 0xB0, 0xC0,
	0x00, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60,	// 90 %
	0x70, 0x80, 0x90, 0x90, 0xA0, 0xB0, 0xC0, 0xD0,
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,	// 100 %
	0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
};*/

static const uint16 noteTable[] = {
	0x000, 0x100, 0x200, 0x300,
	0x400, 0x500, 0x600, 0x700,
	0x800, 0x900, 0xA00, 0xB00,
	0x001, 0x101, 0x201, 0x301,
	0x401, 0x501, 0x601, 0x701,
	0x801, 0x901, 0xA01, 0xB01,
	0x002, 0x102, 0x202, 0x302,
	0x402, 0x502, 0x602, 0x702,
	0x802, 0x902, 0xA02, 0xB02,
	0x003, 0x103, 0x203, 0x303,
	0x403, 0x503, 0x603, 0x703,
	0x803, 0x903, 0xA03, 0xB03,
	0x004, 0x104, 0x204, 0x304,
	0x404, 0x504, 0x604, 0x704,
	0x804, 0x904, 0xA04, 0xB04,
	0x005, 0x105, 0x205, 0x305,
	0x405, 0x505, 0x605, 0x705,
	0x805, 0x905, 0xA05, 0xB05,
	0x006, 0x106, 0x206, 0x306,
	0x406, 0x506, 0x606, 0x706,
	0x806, 0x906, 0xA06, 0xB06,
	0x007, 0x107, 0x207, 0x307,
	0x407, 0x507, 0x607, 0x707,
	0x807, 0x907, 0xA07, 0xB07,
	0x008, 0x108, 0x208, 0x308,
	0x408, 0x508, 0x608, 0x708,
	0x808, 0x908, 0xA08, 0xB08,
	0x009, 0x109, 0x209, 0x309,
	0x409, 0x509, 0x609, 0x709,
	0x809, 0x909, 0xA09, 0xB09,
	0x00A, 0x10A, 0x20A, 0x30A,
	0x40A, 0x50A, 0x60A, 0x70A,
	0x80A, 0x90A, 0xA0A, 0xB0A
};

static const byte attackRate[] = {
	  0,   2,   4,   7,  14,  26,  48,  82,
	128, 144, 160, 176, 192, 208, 224, 255
};

static const byte decayRate[] = {
	  0,   1,   2,   3,   4,   6,  12,  24,
	 48,  96, 192, 215, 255, 255, 255, 255
};

static const byte sustainRate[] = {
	255, 180, 128,  96,  80,  64,  56,  48,
	 42,  36,  32,  28,  24,  20,  16,   0
};

static const byte releaseRate[] = {
	  0,   1,   2,   4,   6,   9,  14,  22,
	 36,  56,  80, 100, 120, 140, 160, 255
};

static const byte volumeTable[] = {
	0x00, 0x10, 0x10, 0x11, 0x11, 0x21, 0x22, 0x22,
	0x33, 0x44, 0x55, 0x66, 0x88, 0xAA, 0xCC, 0xFF
};

static const byte cmsInitData[13*2] = {
	0x1C, 0x02,
	0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00,
	0x14, 0x3F, 0x15, 0x00, 0x16, 0x00, 0x18, 0x00, 0x19, 0x00, 0x1C, 0x01
};

Player_V2CMS::Player_V2CMS(ScummEngine *scumm, Audio::Mixer *mixer)
	: Player_V2Base(scumm, mixer, true) {
	int i;

	setMusicVolume(255);

	memset(_cmsVoicesBase, 0, sizeof(Voice)*16);
	memset(_cmsVoices, 0, sizeof(Voice2)*8);
	memset(_cmsChips, 0, sizeof(MusicChip)*2);
	_midiDelay = _octaveMask = _looping = _tempo = _tempoSum = 0;
	_midiData = _midiSongBegin = 0;
	_musicTimer = _musicTimerTicks = 0;
	_clkFrequenz = 0;
	_loadedMidiSong = 0;
	_outputTableReady = 0;
	memset(_midiChannel, 0, sizeof(Voice2*)*16);
	memset(_midiChannelUse, 0, sizeof(byte)*16);

	_cmsVoices[0].amplitudeOutput = &(_cmsChips[0].ampl[0]);
	_cmsVoices[0].freqOutput = &(_cmsChips[0].freq[0]);
	_cmsVoices[0].octaveOutput = &(_cmsChips[0].octave[0]);
	_cmsVoices[1].amplitudeOutput = &(_cmsChips[0].ampl[1]);
	_cmsVoices[1].freqOutput = &(_cmsChips[0].freq[1]);
	_cmsVoices[1].octaveOutput = &(_cmsChips[0].octave[0]);
	_cmsVoices[2].amplitudeOutput = &(_cmsChips[0].ampl[2]);
	_cmsVoices[2].freqOutput = &(_cmsChips[0].freq[2]);
	_cmsVoices[2].octaveOutput = &(_cmsChips[0].octave[1]);
	_cmsVoices[3].amplitudeOutput = &(_cmsChips[0].ampl[3]);
	_cmsVoices[3].freqOutput = &(_cmsChips[0].freq[3]);
	_cmsVoices[3].octaveOutput = &(_cmsChips[0].octave[1]);
	_cmsVoices[4].amplitudeOutput = &(_cmsChips[1].ampl[0]);
	_cmsVoices[4].freqOutput = &(_cmsChips[1].freq[0]);
	_cmsVoices[4].octaveOutput = &(_cmsChips[1].octave[0]);
	_cmsVoices[5].amplitudeOutput = &(_cmsChips[1].ampl[1]);
	_cmsVoices[5].freqOutput = &(_cmsChips[1].freq[1]);
	_cmsVoices[5].octaveOutput = &(_cmsChips[1].octave[0]);
	_cmsVoices[6].amplitudeOutput = &(_cmsChips[1].ampl[2]);
	_cmsVoices[6].freqOutput = &(_cmsChips[1].freq[2]);
	_cmsVoices[6].octaveOutput = &(_cmsChips[1].octave[1]);
	_cmsVoices[7].amplitudeOutput = &(_cmsChips[1].ampl[3]);
	_cmsVoices[7].freqOutput = &(_cmsChips[1].freq[3]);
	_cmsVoices[7].octaveOutput = &(_cmsChips[1].octave[1]);

	// inits the CMS Emulator like in the original
	_cmsEmu = new CMSEmulator(_sampleRate);
	i = 0;
	for (int cmsPort = 0x220; i < 2; cmsPort += 2, ++i) {
		for (int off = 0; off < 13; ++off) {
			_cmsEmu->portWrite(cmsPort+1, cmsInitData[off*2]);
			_cmsEmu->portWrite(cmsPort, cmsInitData[off*2+1]);
		}
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_V2CMS::~Player_V2CMS() {
	Common::StackLock lock(_mutex);

	_mixer->stopHandle(_soundHandle);
	delete _cmsEmu;
}

void Player_V2CMS::setMusicVolume(int vol) {
}

int Player_V2CMS::getMusicTimer() {
	return _midiData ? _musicTimer : Player_V2Base::getMusicTimer();
}

void Player_V2CMS::stopAllSounds() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < 4; i++) {
		clear_channel(i);
	}
	_next_nr = _current_nr = 0;
	_next_data = _current_data = 0;
	_midiData = 0;
	_midiSongBegin = 0;
	_midiDelay = 0;
	_musicTimer = _musicTimerTicks = 0;
	offAllChannels();
}

void Player_V2CMS::stopSound(int nr) {
	Common::StackLock lock(_mutex);

	if (_next_nr == nr) {
		_next_nr = 0;
		_next_data = 0;
	}
	if (_current_nr == nr) {
		for (int i = 0; i < 4; i++) {
			clear_channel(i);
		}
		_current_nr = 0;
		_current_data = 0;
		chainNextSound();
	}
	if (_loadedMidiSong == nr) {
		_midiData = 0;
		_midiSongBegin = 0;
		_midiDelay = 0;
		offAllChannels();
	}
}

void Player_V2CMS::startSound(int nr) {
	Common::StackLock lock(_mutex);

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	if (data[6] == 0x80) {
		_musicTimer = _musicTimerTicks = 0;
		loadMidiData(data, nr);
	} else {
		int cprio = _current_data ? *(_current_data + _header_len) : 0;
		int prio  = *(data + _header_len);
		int nprio = _next_data ? *(_next_data + _header_len) : 0;

		int restartable = *(data + _header_len + 1);

		if (!_current_nr || cprio <= prio) {
			int tnr = _current_nr;
			int tprio = cprio;
			byte *tdata  = _current_data;

			chainSound(nr, data);
			nr   = tnr;
			prio = tprio;
			data = tdata;
			restartable = data ? *(data + _header_len + 1) : 0;
		}

		if (!_current_nr) {
			nr = 0;
			_next_nr = 0;
			_next_data = 0;
		}

		if (nr != _current_nr
			&& restartable
			&& (!_next_nr
			|| nprio <= prio)) {

			_next_nr = nr;
			_next_data = data;
		}
	}
}

void Player_V2CMS::loadMidiData(byte *data, int sound) {
	memset(_midiChannelUse, 0, sizeof(byte)*16);

	_tempo = data[7];
	_looping = data[8];

	byte channels = data[14];
	byte curChannel = 0;
	byte *voice2 = data + 23;

	for (; channels != 0; ++curChannel, --channels, voice2 += 16) {
		if (*(data + 15 + curChannel)) {
			byte channel = *(data + 15 + curChannel) - 1;
			_midiChannelUse[channel] = 1;

			Voice *voiceDef = &_cmsVoicesBase[channel];

			byte attackDecay = voice2[10];
			voiceDef->attack = attackRate[attackDecay >> 4];
			voiceDef->decay = decayRate[attackDecay & 0x0F];
			byte sustainRelease = voice2[11];
			voiceDef->sustain = sustainRate[sustainRelease >> 4];
			voiceDef->release = releaseRate[sustainRelease & 0x0F];

			if (voice2[3] & 0x40) {
				voiceDef->vibrato = 0x0301;
				if (voice2[13] & 0x40) {
					voiceDef->vibrato = 0x0601;
				}
			} else {
				voiceDef->vibrato = 0;
			}

			if (voice2[8] & 0x80) {
				voiceDef->vibrato2 = 0x0506;
				if (voice2[13] & 0x80) {
					voiceDef->vibrato2 = 0x050C;
				}
			} else {
				voiceDef->vibrato2 = 0;
			}

			if ((voice2[8] & 0x0F) > 1) {
				voiceDef->octadd = 0x01;
			} else {
				voiceDef->octadd = 0x00;
			}
		}
	}

	for (int i = 0, channel = 0; i < 8; ++i, channel += 2) {
		_cmsVoices[i].chanNumber = 0xFF;
		_cmsVoices[i].curVolume = 0;
		_cmsVoices[i].nextVoice = 0;

		_midiChannel[channel] = 0;
	}

	_midiDelay = 0;
	memset(_cmsChips, 0, sizeof(MusicChip)*2);
	_midiData = data + 151;
	_midiSongBegin = _midiData + data[9];

	_loadedMidiSong = sound;
}

int Player_V2CMS::getSoundStatus(int nr) const {
	return _current_nr == nr || _next_nr == nr || _loadedMidiSong == nr;
}

void Player_V2CMS::processMidiData() {
	byte *currentData = _midiData;
	byte command = 0x00;
	int16 temp = 0;

	++_musicTimerTicks;
	if (_musicTimerTicks > 60) {
		_musicTimerTicks = 0;
		++_musicTimer;
	}

	if (!_midiDelay) {
		while (true) {
			if ((command = *currentData++) == 0xFF) {
				if ((command = *currentData++) == 0x2F) {
					if (_looping == 0) {
						currentData = _midiData = _midiSongBegin;
						continue;
					}
					_midiData = _midiSongBegin = 0;
					_midiDelay = 0;
					_loadedMidiSong = 0;
					offAllChannels();
					return;
				} else {
					if (command == 0x58) {
						currentData += 6;
					}
				}
			} else {
				_lastMidiCommand = command;
				if (command < 0x90) {
					clearNote(currentData);
				} else {
					playNote(currentData);
				}
			}

			temp = command = *currentData++;
			if (command & 0x80) {
				temp = (command & 0x7F) << 8;
				command = *currentData++;
				temp |= (command << 1);
				temp >>= 1;
			}
			temp >>= 1;
			int lastBit = temp & 1;
			temp >>= 1;
			temp += lastBit;

			if (temp)
				break;
		}
		_midiData = currentData;
		_midiDelay = temp;
	}

	--_midiDelay;
	if (_midiDelay < 0)
		_midiDelay = 0;

	return;
}

int Player_V2CMS::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	uint step = 1;
	int len = numSamples / 2;

	// maybe this needs a complete rewrite
	do {
		if (!(_next_tick >> FIXP_SHIFT)) {
			if (_midiData) {
				--_clkFrequenz;
				if (!(_clkFrequenz & 0x01))
					playVoice();

				int newTempoSum = _tempo + _tempoSum;
				_tempoSum = newTempoSum & 0xFF;
				if (newTempoSum > 0xFF)
					processMidiData();
			} else {
				nextTick();
				play();
			}
			_next_tick += _tick_len;
		}

		step = len;
		if (step > (_next_tick >> FIXP_SHIFT))
			step = (_next_tick >> FIXP_SHIFT);
		_cmsEmu->readBuffer(buffer, step);
		buffer += 2 * step;
		_next_tick -= step << FIXP_SHIFT;
	} while (len -= step);

	return numSamples;
}

void Player_V2CMS::playVoice() {
	if (_outputTableReady) {
		playMusicChips(_cmsChips);
		_outputTableReady = 0;
	}

	_octaveMask = 0xF0;
	Voice2 *voice = 0;
	for (int i = 0; i < 8; ++i) {
		voice = &_cmsVoices[i];
		_octaveMask = ~_octaveMask;

		if (voice->chanNumber != 0xFF) {
			processChannel(voice);
		} else {
			if (!voice->curVolume) {
				*(voice->amplitudeOutput) = 0;
			}

			int volume = voice->curVolume - voice->releaseRate;
			if (volume < 0)
				volume = 0;

			voice->curVolume = volume;
			*(voice->amplitudeOutput) = ((volume >> 4) | (volume & 0xF0)) & voice->channel;
			++_outputTableReady;
		}
	}
}

void Player_V2CMS::processChannel(Voice2 *channel) {
	++_outputTableReady;
	switch (channel->nextProcessState) {
	case PROCESS_RELEASE:
		processRelease(channel);
		break;

	case PROCESS_ATTACK:
		processAttack(channel);
		break;

	case PROCESS_DECAY:
		processDecay(channel);
		break;

	case PROCESS_SUSTAIN:
		processSustain(channel);
		break;

	case PROCESS_VIBRATO:
		processVibrato(channel);
		break;

	default:
		break;
	}
}

void Player_V2CMS::processRelease(Voice2 *channel) {
	int newVolume = channel->curVolume - channel->releaseRate;
	if (newVolume < 0)
		newVolume = 0;

	channel->curVolume = newVolume;
	processVibrato(channel);
}

void Player_V2CMS::processAttack(Voice2 *channel) {
	int newVolume = channel->curVolume + channel->attackRate;
	if (newVolume > channel->maxAmpl) {
		channel->curVolume = channel->maxAmpl;
		channel->nextProcessState = PROCESS_DECAY;
	} else {
		channel->curVolume = newVolume;
	}

	processVibrato(channel);
}

void Player_V2CMS::processDecay(Voice2 *channel) {
	int newVolume = channel->curVolume - channel->decayRate;
	if (newVolume <= channel->sustainRate) {
		channel->curVolume = channel->sustainRate;
		channel->nextProcessState = PROCESS_SUSTAIN;
	} else {
		channel->curVolume = newVolume;
	}

	processVibrato(channel);
}

void Player_V2CMS::processSustain(Voice2 *channel) {
	if (channel->unkVibratoRate) {
		int16 volume = channel->curVolume + channel->unkRate;
		if (volume & 0xFF00) {
			volume = int8(volume >> 8);
			volume = -volume;
		}

		channel->curVolume = volume;
		--channel->unkCount;
		if (!channel->unkCount) {
			channel->unkRate = -channel->unkRate;
			channel->unkCount = (channel->unkVibratoDepth & 0x0F) << 1;
		}
	}
	processVibrato(channel);
}

void Player_V2CMS::processVibrato(Voice2 *channel) {
	if (channel->vibratoRate) {
		int16 temp = channel->curFreq + channel->curVibratoRate;
		channel->curOctave += (temp & 0xFF00) >> 8;
		channel->curFreq = temp & 0xFF;

		--channel->curVibratoUnk;
		if (!channel->curVibratoUnk) {
			channel->curVibratoRate = -channel->curVibratoRate;
			channel->curVibratoUnk = (channel->vibratoDepth & 0x0F) << 1;
		}
	}

	byte *output = channel->amplitudeOutput;
	*output = ((channel->curVolume >> 4) | (channel->curVolume & 0xF0)) & channel->channel;
	output = channel->freqOutput;
	*output = channel->curFreq;
	output = channel->octaveOutput;
	*output = ((((channel->curOctave << 4) | (channel->curOctave & 0x0F)) & _octaveMask) | ((~_octaveMask) & *output));
}

void Player_V2CMS::offAllChannels() {
	for (int cmsPort = 0x220, i = 0; i < 2; cmsPort += 2, ++i) {
		for (int off = 1; off <= 10; ++off) {
			_cmsEmu->portWrite(cmsPort+1, cmsInitData[off*2]);
			_cmsEmu->portWrite(cmsPort, cmsInitData[off*2+1]);
		}
	}
}

Player_V2CMS::Voice2 *Player_V2CMS::getFreeVoice() {
	Voice2 *curVoice = 0;
	Voice2 *selected = 0;
	uint8 volume = 0xFF;

	for (int i = 0; i < 8; ++i) {
		curVoice = &_cmsVoices[i];

		if (curVoice->chanNumber == 0xFF) {
			if (!curVoice->curVolume) {
				selected = curVoice;
				break;
			}

			if (curVoice->curVolume < volume) {
				selected = curVoice;
				volume = selected->curVolume;
			}
		}
	}

	if (selected) {
		selected->chanNumber = _lastMidiCommand & 0x0F;

		uint8 channel = selected->chanNumber;
		Voice2 *oldChannel = _midiChannel[channel];
		_midiChannel[channel] = selected;
		selected->nextVoice = oldChannel;
	}

	return selected;
}

void Player_V2CMS::playNote(byte *&data) {
	byte channel = _lastMidiCommand & 0x0F;
	if (_midiChannelUse[channel]) {
		Voice2 *freeVoice = getFreeVoice();
		if (freeVoice) {
			Voice *voice = &_cmsVoicesBase[freeVoice->chanNumber];
			freeVoice->attackRate = voice->attack;
			freeVoice->decayRate = voice->decay;
			freeVoice->sustainRate = voice->sustain;
			freeVoice->releaseRate = voice->release;
			freeVoice->octaveAdd = voice->octadd;
			freeVoice->vibratoRate = freeVoice->curVibratoRate = voice->vibrato & 0xFF;
			freeVoice->vibratoDepth = freeVoice->curVibratoUnk = voice->vibrato >> 8;
			freeVoice->unkVibratoRate = freeVoice->unkRate = voice->vibrato2 & 0xFF;
			freeVoice->unkVibratoDepth = freeVoice->unkCount = voice->vibrato2 >> 8;
			freeVoice->maxAmpl = 0xFF;

			uint8 rate = freeVoice->attackRate;
			uint8 volume = freeVoice->curVolume >> 1;

			if (rate < volume)
				rate = volume;

			rate -= freeVoice->attackRate;
			freeVoice->curVolume = rate;
			freeVoice->playingNote = *data;

			uint16 note = noteTable[*data + 3];

			int octave = int8(note & 0xFF) + freeVoice->octaveAdd - 3;
			if (octave < 0)
				octave = 0;
			if (octave > 7)
				octave = 7;
			if (!octave)
				++octave;
			freeVoice->curOctave = octave;
			freeVoice->curFreq = freqTable[(note >> 8) << 2];
			freeVoice->curVolume = 0;
			freeVoice->nextProcessState = PROCESS_ATTACK;
			if (!(_lastMidiCommand & 1))
				freeVoice->channel = 0xF0;
			else
				freeVoice->channel = 0x0F;
		}
	}
	data += 2;
}

Player_V2CMS::Voice2 *Player_V2CMS::getPlayVoice(byte param) {
	byte channelNum = _lastMidiCommand & 0x0F;
	Voice2 *curVoice = _midiChannel[channelNum];

	if (curVoice) {
		Voice2 *prevVoice = 0;
		while (true) {
			if (curVoice->playingNote == param)
				break;

			prevVoice = curVoice;
			curVoice = curVoice->nextVoice;
			if (!curVoice)
				return 0;
		}

		if (prevVoice)
			prevVoice->nextVoice = curVoice->nextVoice;
		else
			_midiChannel[channelNum] = curVoice->nextVoice;
	}

	return curVoice;
}

void Player_V2CMS::clearNote(byte *&data) {
	Voice2 *voice = getPlayVoice(*data);
	if (voice) {
		voice->chanNumber = 0xFF;
		voice->nextVoice = 0;
		voice->nextProcessState = PROCESS_RELEASE;
	}
	data += 2;
}

void Player_V2CMS::play() {
	_octaveMask = 0xF0;
	channel_data *chan = &(_channels[0].d);

	static byte volumeReg[4] = { 0x00, 0x00, 0x00, 0x00 };
	static byte octaveReg[4] = { 0x66, 0x66, 0x66, 0x66 };
	static byte freqReg[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	static byte freqEnable = 0x3E;
	static byte noiseEnable = 0x01;
	static byte noiseGen = 0x02;
	for (int i = 1; i <= 4; ++i) {
		if (chan->time_left) {
			uint16 freq = chan->freq;

			if (i == 4) {
				if ((freq >> 8) & 0x40) {
					noiseGen = freq & 0xFF;
				} else {
					noiseGen = 3;
					freqReg[0] = freqReg[3];
					octaveReg[0] = (octaveReg[0] & 0xF0) | ((octaveReg[1] & 0xF0) >> 4);
				}
			} else {
				if (freq == 0) {
					freq = 0xFFC0;
				}

				int cmsOct = 2;
				int freqOct = 0x8000;

				while (true) {
					if (freq >= freqOct) {
						break;
					}
					freqOct >>= 1;
					++cmsOct;
					if (cmsOct == 8) {
						--cmsOct;
						freq = 1024;
						break;
					}
				}
				byte oct = cmsOct << 4;
				oct |= cmsOct;

				oct &= _octaveMask;
				oct |= ((~_octaveMask) & octaveReg[((i & 3) >> 1)]);
				octaveReg[((i & 3) >> 1)] = oct;

				freq >>= -(cmsOct-9);
				freqReg[(i&3)] = (-(freq-511)) & 0xFF;
			}
			volumeReg[i & 3] = volumeTable[chan->volume >> 12];
		} else {
			volumeReg[i & 3] = 0;
		}
		chan = &(_channels[i].d);
		_octaveMask ^= 0xFF;
	}

	// with the high nibble of the volumeReg value
	// the right channels amplitude is set
	// with the low value the left channels amplitude
	_cmsEmu->portWrite(0x221, 0);
	_cmsEmu->portWrite(0x220, volumeReg[0]);
	_cmsEmu->portWrite(0x221, 1);
	_cmsEmu->portWrite(0x220, volumeReg[1]);
	_cmsEmu->portWrite(0x221, 2);
	_cmsEmu->portWrite(0x220, volumeReg[2]);
	_cmsEmu->portWrite(0x221, 3);
	_cmsEmu->portWrite(0x220, volumeReg[3]);
	_cmsEmu->portWrite(0x221, 8);
	_cmsEmu->portWrite(0x220, freqReg[0]);
	_cmsEmu->portWrite(0x221, 9);
	_cmsEmu->portWrite(0x220, freqReg[1]);
	_cmsEmu->portWrite(0x221, 10);
	_cmsEmu->portWrite(0x220, freqReg[2]);
	_cmsEmu->portWrite(0x221, 11);
	_cmsEmu->portWrite(0x220, freqReg[3]);
	_cmsEmu->portWrite(0x221, 0x10);
	_cmsEmu->portWrite(0x220, octaveReg[0]);
	_cmsEmu->portWrite(0x221, 0x11);
	_cmsEmu->portWrite(0x220, octaveReg[1]);
	_cmsEmu->portWrite(0x221, 0x14);
	_cmsEmu->portWrite(0x220, freqEnable);
	_cmsEmu->portWrite(0x221, 0x15);
	_cmsEmu->portWrite(0x220, noiseEnable);
	_cmsEmu->portWrite(0x221, 0x16);
	_cmsEmu->portWrite(0x220, noiseGen);
}

void Player_V2CMS::playMusicChips(const MusicChip *table) {
	int cmsPort = 0x21E;

	do {
		cmsPort += 2;
		_cmsEmu->portWrite(cmsPort+1, 0);
		_cmsEmu->portWrite(cmsPort, table->ampl[0]);
		_cmsEmu->portWrite(cmsPort+1, 1);
		_cmsEmu->portWrite(cmsPort, table->ampl[1]);
		_cmsEmu->portWrite(cmsPort+1, 2);
		_cmsEmu->portWrite(cmsPort, table->ampl[2]);
		_cmsEmu->portWrite(cmsPort+1, 3);
		_cmsEmu->portWrite(cmsPort, table->ampl[3]);
		_cmsEmu->portWrite(cmsPort+1, 8);
		_cmsEmu->portWrite(cmsPort, table->freq[0]);
		_cmsEmu->portWrite(cmsPort+1, 9);
		_cmsEmu->portWrite(cmsPort, table->freq[1]);
		_cmsEmu->portWrite(cmsPort+1, 10);
		_cmsEmu->portWrite(cmsPort, table->freq[2]);
		_cmsEmu->portWrite(cmsPort+1, 11);
		_cmsEmu->portWrite(cmsPort, table->freq[3]);
		_cmsEmu->portWrite(cmsPort+1, 0x10);
		_cmsEmu->portWrite(cmsPort, table->octave[0]);
		_cmsEmu->portWrite(cmsPort+1, 0x11);
		_cmsEmu->portWrite(cmsPort, table->octave[1]);
		_cmsEmu->portWrite(cmsPort+1, 0x14);
		_cmsEmu->portWrite(cmsPort, 0x3F);
		_cmsEmu->portWrite(cmsPort+1, 0x15);
		_cmsEmu->portWrite(cmsPort, 0x00);
		++table;
	} while ((cmsPort & 2) == 0);
}

} // End of namespace Scumm
