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
 * $URL$
 * $Id$
 *
 */

#include "common/md5.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/str-array.h"

#include "agi/agi.h"

#include "agi/sound_2gs.h"

namespace Agi {

#define USE_INTERPOLATION

//
// TODO: add support for variable sampling rate in the output device
//

AgiSound *AgiSound::createFromRawResource(uint8 *data, uint32 len, int resnum, SoundMgr &manager) {
	if (data == NULL || len < 2) // Check for too small resource or no resource at all
		return NULL;
	uint16 type = READ_LE_UINT16(data);

	switch (type) { // Create a sound object based on the type
	case AGI_SOUND_SAMPLE:
		return new IIgsSample(data, len, resnum, manager);
	case AGI_SOUND_MIDI:
		return new IIgsMidi  (data, len, resnum, manager);
	case AGI_SOUND_4CHN:
		return new PCjrSound (data, len, resnum, manager);
	}

	warning("Sound resource (%d) has unknown type (0x%04x). Not using the sound", resnum, type);
	return NULL;
}

PCjrSound::PCjrSound(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_isValid = (_type == AGI_SOUND_4CHN) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating PCjr 4-channel sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

const uint8 *PCjrSound::getVoicePointer(uint voiceNum) {
	assert(voiceNum < 4);
	uint16 voiceStartOffset = READ_LE_UINT16(_data + voiceNum * 2);

	return _data + voiceStartOffset;
}

static const int16 waveformRamp[WAVEFORM_SIZE] = {
	0, 8, 16, 24, 32, 40, 48, 56,
	64, 72, 80, 88, 96, 104, 112, 120,
	128, 136, 144, 152, 160, 168, 176, 184,
	192, 200, 208, 216, 224, 232, 240, 255,
	0, -248, -240, -232, -224, -216, -208, -200,
	-192, -184, -176, -168, -160, -152, -144, -136,
	-128, -120, -112, -104, -96, -88, -80, -72,
	-64, -56, -48, -40, -32, -24, -16, -8	// Ramp up
};

static const int16 waveformSquare[WAVEFORM_SIZE] = {
	255, 230, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 110,
	-255, -230, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -110, 0, 0, 0, 0	// Square
};

static const int16 waveformMac[WAVEFORM_SIZE] = {
	45, 110, 135, 161, 167, 173, 175, 176,
	156, 137, 123, 110, 91, 72, 35, -2,
	-60, -118, -142, -165, -170, -176, -177, -179,
	-177, -176, -164, -152, -117, -82, -17, 47,
	92, 137, 151, 166, 170, 173, 171, 169,
	151, 133, 116, 100, 72, 43, -7, -57,
	-99, -141, -156, -170, -174, -177, -178, -179,
	-175, -172, -165, -159, -137, -114, -67, -19
};

static const uint16 period[] = {
	1024, 1085, 1149, 1218, 1290, 1367,
	1448, 1534, 1625, 1722, 1825, 1933
};

#if 0
static int noteToPeriod(int note) {
	return 10 * (period[note % 12] >> (note / 12 - 3));
}
#endif

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum]->isPlaying()) {
			_vm->_game.sounds[resnum]->stop();
		}

		// Release the sound resource's data
		delete _vm->_game.sounds[resnum];
		_vm->_game.sounds[resnum] = NULL;
		_vm->_game.dirSound[resnum].flags &= ~RES_LOADED;
	}
}

void SoundMgr::startSound(int resnum, int flag) {
	int i;
	AgiSoundType type;

	if (_vm->_game.sounds[resnum] != NULL && _vm->_game.sounds[resnum]->isPlaying())
		return;

	stopSound();

	if (_vm->_game.sounds[resnum] == NULL) // Is this needed at all?
		return;

	type = (AgiSoundType)_vm->_game.sounds[resnum]->type();

	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;

	_vm->_game.sounds[resnum]->play();
	_playingSound = resnum;

	debugC(3, kDebugLevelSound, "startSound(resnum = %d, flag = %d) type = %d", resnum, flag, type);

	switch (type) {
	case AGI_SOUND_SAMPLE: {
		IIgsSample *sampleRes = (IIgsSample *) _vm->_game.sounds[_playingSound];
		_gsSound->playSampleSound(sampleRes->getHeader(), sampleRes->getSample());
		break;
	}
	case AGI_SOUND_MIDI:
		((IIgsMidi *) _vm->_game.sounds[_playingSound])->rewind();
		break;
	case AGI_SOUND_4CHN:
		PCjrSound *pcjrSound = (PCjrSound *) _vm->_game.sounds[resnum];

		// Initialize channel info
		for (i = 0; i < NUM_CHANNELS; i++) {
			_chn[i].type = type;
			_chn[i].flags = AGI_SOUND_LOOP;

			if (_env) {
				_chn[i].flags |= AGI_SOUND_ENVELOPE;
				_chn[i].adsr = AGI_SOUND_ENV_ATTACK;
			}

			_chn[i].ins = _waveform;
			_chn[i].size = WAVEFORM_SIZE;
			_chn[i].ptr = pcjrSound->getVoicePointer(i % 4);
			_chn[i].timer = 0;
			_chn[i].vol = 0;
			_chn[i].end = 0;
		}
		break;
	}

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
	_endflag = flag;

	// Nat Budin reports that the flag should be reset when sound starts
	_vm->setflag(_endflag, false);
}

void SoundMgr::stopSound() {
	int i;

	debugC(3, kDebugLevelSound, "stopSound() --> %d", _playingSound);

	_endflag = -1;
	if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
		for (i = 0; i < NUM_CHANNELS; i++)
			stopNote(i);
	}

	if (_playingSound != -1) {
		if (_vm->_game.sounds[_playingSound]) // sanity checking
			_vm->_game.sounds[_playingSound]->stop();

		if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
			_gsSound->stopSounds();
		}

		_playingSound = -1;
	}
}

int SoundMgr::initSound() {
	int r = -1;

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
	_env = false;

	switch (_vm->_soundemu) {
	case SOUND_EMU_NONE:
		_waveform = waveformRamp;
		_env = true;
		break;
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_PC:
		_waveform = waveformSquare;
		break;
	case SOUND_EMU_MAC:
		_waveform = waveformMac;
		break;
	case SOUND_EMU_APPLE2GS:
		_disabledMidi = !loadInstruments();
		break;
	case SOUND_EMU_COCO3:
		break;
	}

	report("Initializing sound:\n");

	report("sound: envelopes ");
	if (_env) {
		report("enabled (decay=%d, sustain=%d)\n", ENV_DECAY, ENV_SUSTAIN);
	} else {
		report("disabled\n");
	}

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return r;
}

void SoundMgr::deinitSound() {
	debugC(3, kDebugLevelSound, "()");

	_mixer->stopHandle(_soundHandle);
}

void SoundMgr::stopNote(int i) {
	_chn[i].adsr = AGI_SOUND_ENV_RELEASE;

	if (_useChorus) {
		// Stop chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
			stopNote(i + 4);
		}
	}
}

void SoundMgr::playNote(int i, int freq, int vol) {
	if (!_vm->getflag(fSoundOn))
		vol = 0;
	else if (vol && _vm->_soundemu == SOUND_EMU_PC)
		vol = 160;

	_chn[i].phase = 0;
	_chn[i].freq = freq;
	_chn[i].vol = vol;
	_chn[i].env = 0x10000;
	_chn[i].adsr = AGI_SOUND_ENV_ATTACK;

	if (_useChorus) {
		// Add chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {

			int newfreq = freq * 1007 / 1000;

			if (freq == newfreq)
				newfreq++;

			playNote(i + 4, newfreq, vol * 2 / 3);
		}
	}
}

static int cocoFrequencies[] = {
	 130,  138,  146,  155,  164,  174,  184,  195,  207,  220,  233,  246,
	 261,  277,  293,  311,  329,  349,  369,  391,  415,  440,  466,  493,
	 523,  554,  587,  622,  659,  698,  739,  783,  830,  880,  932,  987,
	1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
	2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951
};

void SoundMgr::playCoCoSound() {
	int i = 0;
	CoCoNote note;

	do {
		note.read(_chn[i].ptr);

		if (note.freq != 0xff) {
			playNote(0, cocoFrequencies[note.freq], note.volume);

			uint32 start_time = _vm->_system->getMillis();

			while (_vm->_system->getMillis() < start_time + note.duration) {
                _vm->_system->updateScreen();

                _vm->_system->delayMillis(10);
			}
		}
	} while (note.freq != 0xff);
}

void SoundMgr::playAgiSound() {
	int i;
	AgiNote note;

	_playing = false;
	for (i = 0; i < (_vm->_soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		_playing |= !_chn[i].end;
		note.read(_chn[i].ptr); // Read a single note (Doesn't advance the pointer)

		if (_chn[i].end)
			continue;

		if ((--_chn[i].timer) <= 0) {
			stopNote(i);

			if (note.freqDiv != 0) {
				int volume = (note.attenuation == 0x0F) ? 0 : (0xFF - note.attenuation * 2);
				playNote(i, note.freqDiv * 10, volume);
			}

			_chn[i].timer = note.duration;

			if (_chn[i].timer == 0xffff) {
				_chn[i].end = 1;
				_chn[i].vol = 0;
				_chn[i].env = 0;

				if (_useChorus) {
					// chorus
					if (_chn[i].type == AGI_SOUND_4CHN && _vm->_soundemu == SOUND_EMU_NONE && i < 3) {
						_chn[i + 4].vol = 0;
						_chn[i + 4].env = 0;
					}
				}
			}
			_chn[i].ptr += 5; // Advance the pointer to the next note data (5 bytes per note)
		}
	}
}

void SoundMgr::playSound() {
	int i;

	if (_endflag == -1)
		return;

	if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
		if (_playingSound != -1) {
			if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_MIDI) {
				playMidiSound();
				//warning("playSound: Trying to play an Apple IIGS MIDI sound. Not yet implemented");
			} else if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_SAMPLE) {
				//debugC(3, kDebugLevelSound, "playSound: Trying to play an Apple IIGS sample");
				playSampleSound();
			}
		}
	} else if (_vm->_soundemu == SOUND_EMU_COCO3) {
		playCoCoSound();
	} else {
		//debugC(3, kDebugLevelSound, "playSound: Trying to play a PCjr 4-channel sound");
		playAgiSound();
	}

	if (!_playing) {
		if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
			for (i = 0; i < NUM_CHANNELS; _chn[i++].vol = 0)
				;
		}

		if (_endflag != -1)
			_vm->setflag(_endflag, true);

		if (_playingSound != -1)
			_vm->_game.sounds[_playingSound]->stop();
		_playingSound = -1;
		_endflag = -1;
	}
}

uint32 SoundMgr::mixSound() {
	register int i, p;
	const int16 *src;
	int c, b, m;

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);

	if (!_playing || _playingSound == -1)
		return BUFFER_SIZE;

	// Handle Apple IIGS sound mixing here
	// TODO: Implement playing both waves in an oscillator
	// TODO: Implement swap-mode in an oscillator
	if (_vm->_soundemu == SOUND_EMU_APPLE2GS) {
		for (uint midiChan = 0; midiChan < _gsSound->_midiChannels.size(); midiChan++) {
			for (uint gsChan = 0; gsChan < _gsSound->_midiChannels[midiChan]._gsChannels.size(); gsChan++) {
				IIgsChannelInfo &channel = _gsSound->_midiChannels[midiChan]._gsChannels[gsChan];
				if (channel.playing()) { // Only mix in actively playing channels
					// Frequency multiplier was 1076.0 based on tests made with MESS 0.117.
					// Tests made with KEGS32 averaged the multiplier to around 1045.
					// So this is a guess but maybe it's 1046.5... i.e. C6's frequency?
					double hertz = C6_FREQ * pow(SEMITONE, fracToDouble(channel.note));
					channel.posAdd = doubleToFrac(hertz / getRate());
					channel.vol = doubleToFrac(fracToDouble(channel.envVol) * fracToDouble(channel.chanVol) / 127.0);
					double tempVol = fracToDouble(channel.vol)/127.0;
					for (i = 0; i < IIGS_BUFFER_SIZE; i++) {
						b = channel.relocatedSample[fracToInt(channel.pos)];
						// TODO: Find out what volume/amplification setting is loud enough
						//       but still doesn't clip when playing many channels on it.
						_sndBuffer[i] += (int16) (b * tempVol * 256/4);
						channel.pos += channel.posAdd;

						if (channel.pos >= intToFrac(channel.size)) {
							if (channel.loop) {
								// Don't divide by zero on zero length samples
								channel.pos %= intToFrac(channel.size + (channel.size == 0));
								// Probably we should loop the envelope too
								channel.envSeg = 0;
								channel.envVol = channel.startEnvVol;
							} else {
								channel.pos = channel.chanVol = 0;
								channel.end = true;
								break;
							}
						}
					}

					if (channel.envSeg < ENVELOPE_SEGMENT_COUNT) {
						const IIgsEnvelopeSegment &seg = channel.ins->env.seg[channel.envSeg];
						// I currently assume enveloping works with the same speed as the MIDI
						// (i.e. with 1/60ths of a second ticks).
						// TODO: Check if enveloping really works with the same speed as MIDI
						frac_t envVolDelta = doubleToFrac(seg.inc/256.0);
						if (intToFrac(seg.bp) >= channel.envVol) {
							channel.envVol += envVolDelta;
							if (channel.envVol >= intToFrac(seg.bp)) {
								channel.envVol = intToFrac(seg.bp);
								channel.envSeg += 1;
							}
						} else {
							channel.envVol -= envVolDelta;
							if (channel.envVol <= intToFrac(seg.bp)) {
								channel.envVol = intToFrac(seg.bp);
								channel.envSeg += 1;
							}
						}
					}
				}
			}
		}
		_gsSound->removeStoppedSounds();
		return IIGS_BUFFER_SIZE;
	} // else ...

	// Handle PCjr 4-channel sound mixing here
	for (c = 0; c < NUM_CHANNELS; c++) {
		if (!_chn[c].vol)
			continue;

		m = _chn[c].flags & AGI_SOUND_ENVELOPE ?
		    _chn[c].vol * _chn[c].env >> 16 : _chn[c].vol;

		if (_chn[c].type != AGI_SOUND_4CHN || c != 3) {
			src = _chn[c].ins;

			p = _chn[c].phase;
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = src[p >> 8];
#ifdef USE_INTERPOLATION
				b += ((src[((p >> 8) + 1) % _chn[c].size] - src[p >> 8]) * (p & 0xff)) >> 8;
#endif
				_sndBuffer[i] += (b * m) >> 4;

				p += (uint32) 118600 *4 / _chn[c].freq;

				// FIXME: Fingolfin asks: why is there a FIXME here? Please either clarify what
				// needs fixing, or remove it!
				// FIXME
				if (_chn[c].flags & AGI_SOUND_LOOP) {
					p %= _chn[c].size << 8;
				} else {
					if (p >= _chn[c].size << 8) {
						p = _chn[c].vol = 0;
						_chn[c].end = 1;
						break;
					}
				}

			}
			_chn[c].phase = p;
		} else {
			// Add white noise
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = _vm->_rnd->getRandomNumber(255) - 128;
				_sndBuffer[i] += (b * m) >> 4;
			}
		}

		switch (_chn[c].adsr) {
		case AGI_SOUND_ENV_ATTACK:
			// not implemented
			_chn[c].adsr = AGI_SOUND_ENV_DECAY;
			break;
		case AGI_SOUND_ENV_DECAY:
			if (_chn[c].env > _chn[c].vol * ENV_SUSTAIN + ENV_DECAY) {
				_chn[c].env -= ENV_DECAY;
			} else {
				_chn[c].env = _chn[c].vol * ENV_SUSTAIN;
				_chn[c].adsr = AGI_SOUND_ENV_SUSTAIN;
			}
			break;
		case AGI_SOUND_ENV_SUSTAIN:
			break;
		case AGI_SOUND_ENV_RELEASE:
			if (_chn[c].env >= ENV_RELEASE) {
				_chn[c].env -= ENV_RELEASE;
			} else {
				_chn[c].env = 0;
			}
		}
	}

	return BUFFER_SIZE;
}

/**
 * Convert sample from 8-bit unsigned to 8-bit signed format.
 * @param source  Source stream containing the 8-bit unsigned sample data.
 * @param dest  Destination buffer for the 8-bit signed sample data.
 * @param length  Length of the sample data to be converted.
 */
bool SoundMgr::convertWave(Common::SeekableReadStream &source, int8 *dest, uint length) {
	// Convert the wave from 8-bit unsigned to 8-bit signed format
	for (uint i = 0; i < length; i++)
		dest[i] = (int8) ((int) source.readByte() - 128);
	return !(source.eos() || source.err());
}

void SoundMgr::fillAudio(void *udata, int16 *stream, uint len) {
	SoundMgr *soundMgr = (SoundMgr *)udata;
	uint32 p = 0;

	// current number of audio bytes in _sndBuffer
	static uint32 data_available = 0;
	// offset of start of audio bytes in _sndBuffer
	static uint32 data_offset = 0;

	len <<= 2;

	debugC(5, kDebugLevelSound, "(%p, %p, %d)", (void *)udata, (void *)stream, len);

	while (len > data_available) {
		memcpy((uint8 *)stream + p, (uint8*)_sndBuffer + data_offset, data_available);
		p += data_available;
		len -= data_available;

		soundMgr->playSound();
		data_available = soundMgr->mixSound() << 1;
		data_offset = 0;
	}

	memcpy((uint8 *)stream + p, (uint8*)_sndBuffer + data_offset, len);
	data_offset += len;
	data_available -= len;
}

SoundMgr::SoundMgr(AgiBase *agi, Audio::Mixer *pMixer) : _chn() {
	_vm = agi;
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	_endflag = -1;
	_playingSound = -1;
	_env = false;
	_playing = false;
	_sndBuffer = (int16 *)calloc(2, BUFFER_SIZE);
	_waveform = 0;
	_disabledMidi = false;
	_useChorus = true;	// FIXME: Currently always true?

	_gsSound = new IIgsSoundMgr;
}

void SoundMgr::premixerCall(int16 *data, uint len) {
	fillAudio(this, data, len);
}

void SoundMgr::setVolume(uint8 volume) {
	// TODO
}

SoundMgr::~SoundMgr() {
	free(_sndBuffer);
	delete _gsSound;
}

} // End of namespace Agi
