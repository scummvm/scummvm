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

/*
 * This code is based on IBXM mod player
 *
 * Copyright (c) 2015, Martin Cameron
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 * * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 *
 * * Redistributions in binary form must reproduce the
 * above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * * Neither the name of the organization nor the names of
 *  its contributors may be used to endorse or promote
 *  products derived from this software without specific
 *  prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"

#include "audio/audiostream.h"
#include "audio/mods/mod_xm_s3m.h"
#include "audio/mods/module_mod_xm_s3m.h"

namespace Modules {

class ModXmS3mStream : public Audio::AudioStream {
private:
	struct Channel {
		Instrument *instrument;
		Sample *sample;
		Note note;
		int keyOn, randomSeed, plRow;
		int sampleOff, sampleIdx, sampleFra, freq, ampl, pann;
		int volume, panning, fadeoutVol, volEnvTick, panEnvTick;
		int period, portaPeriod, retrigCount, fxCount, avCount;
		int portaUpParam, portaDownParam, tonePortaParam, offsetParam;
		int finePortaUpParam, finePortaDownParam, xfinePortaParam;
		int arpeggioParam, volSlideParam, gvolSlideParam, panSlideParam;
		int fineVslideUpParam, fineVslideDownParam;
		int retrigVolume, retrigTicks, tremorOnTicks, tremorOffTicks;
		int vibratoType, vibratoPhase, vibratoSpeed, vibratoDepth;
		int tremoloType, tremoloPhase, tremoloSpeed, tremoloDepth;
		int tremoloAdd, vibratoAdd, arpeggioAdd;
	};

	ModuleModXmS3m _module;
	bool _loadSuccess;
	int _sampleRate, _interpolation, _globalVol;
	int _seqPos, _breakPos, _row, _nextRow, _tick;
	int _speed, _tempo, _plCount, _plChan;
	int *_rampBuf;
	int8 **_playCount;
	Channel *_channels;
	int _dataLeft;

	// mix buffer to keep a partially consumed decoded tick.
	int *_mixBuffer;
	int _mixBufferSamples;	// number of samples kept in _mixBuffer

	static const int FP_SHIFT;
	static const int FP_ONE;
	static const int FP_MASK;
	static const int16 sinetable[];

	int calculateDuration();
	int calculateTickLength() { return (_sampleRate * 5) / (_tempo * 2); }
	int calculateMixBufLength() { return (calculateTickLength() + 65) * 4; }

	int initPlayCount(int8 **playCount);
	void setSequencePos(int pos);
	int tick();
	void updateRow();
	int seek(int samplePos);

	// Sample
	void downsample(int *buf, int count);
	void resample(Channel &channel, int *mixBuf, int offset, int count, int sampleRate);
	void updateSampleIdx(Channel &channel, int count, int sampleRate);

	// Channel
	void initChannel(int idx);
	void tickChannel(Channel &channel);
	void updateChannelRow(Channel &channel, Note note);

	// Effects
	int waveform(Channel &channel, int phase, int type);
	void vibrato(Channel &channel, int fine);
	void autoVibrato(Channel &channel);
	void portaUp(Channel &channel, int param);
	void portaDown(Channel &channel, int param);
	void tonePorta(Channel &channel);
	void volumeSlide(Channel &channel);
	void retrigVolSlide(Channel &channel);
	void tremolo(Channel &channel);
	void tremor(Channel &channel);
	void trigger(Channel &channel);
	void calculateFreq(Channel &channel);
	void calculateAmpl(Channel &channel);

	// Envelopes
	void updateEnvelopes(Channel &channel);
	int envelopeNextTick(Envelope &envelope, int tick, int keyOn);
	int calculateAmpl(Envelope &envelope, int tick);

	// Read stream
	int getAudio(int *mixBuf);
	void volumeRamp(int *mixBuf, int tickLen);

public:
	// Check if module loading succeeds
	bool loadSuccess() const { return _loadSuccess; }

	// Implement virtual functions
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return true; }
	virtual int getRate() const { return _sampleRate; }
	virtual bool endOfData() const { return _dataLeft <= 0; }

	ModXmS3mStream(Common::SeekableReadStream *stream, int rate, int interpolation);
	~ModXmS3mStream();
};

const int ModXmS3mStream::FP_SHIFT = 0xF;
const int ModXmS3mStream::FP_ONE = 0x8000;
const int ModXmS3mStream::FP_MASK = 0x7FFF;
const short ModXmS3mStream::sinetable[] = {
		  0,  24,  49,  74,  97, 120, 141, 161, 180, 197, 212, 224, 235, 244, 250, 253,
		255, 253, 250, 244, 235, 224, 212, 197, 180, 161, 141, 120,  97,  74,  49,  24
	};

ModXmS3mStream::ModXmS3mStream(Common::SeekableReadStream *stream, int rate, int interpolation) {
	_rampBuf = nullptr;
	_playCount = nullptr;
	_channels = nullptr;

	if (!_module.load(*stream)) {
		warning("It's not a valid Mod/S3m/Xm sound file");
		_loadSuccess = false;
		return;
	}

	// assign values
	_loadSuccess = true;
	_mixBufferSamples = 0;
	_sampleRate = rate;
	_interpolation = interpolation;
	_rampBuf = new int[128];
	_channels = new Channel[_module.numChannels];
	_dataLeft = calculateDuration() * 4; // stereo and uint16
	_mixBuffer = nullptr;
}

ModXmS3mStream::~ModXmS3mStream() {
	if (_rampBuf) {
		delete[] _rampBuf;
		_rampBuf = nullptr;
	}

	if (_playCount) {
		delete[] _playCount;
		_playCount = nullptr;
	}

	if (_channels) {
		delete[] _channels;
		_channels = nullptr;
	}

	if (_mixBuffer) {
		delete []_mixBuffer;
		_mixBuffer = nullptr;
	}
}

int ModXmS3mStream::initPlayCount(int8 **playCount) {
	int len = 0;
	for (int idx = 0; idx < _module.sequenceLen; ++idx) {
		int pat = _module.sequence[idx];
		int rows = (pat < _module.numPatterns) ? _module.patterns[pat].numRows : 0;
		if (playCount) {
			playCount[idx] = playCount[0] ? &playCount[0][len] : nullptr;
		}
		len += rows;
	}
	return len;
}

void ModXmS3mStream::initChannel(int idx) {
	memset(&_channels[idx], 0, sizeof(Channel));
	_channels[idx].panning = _module.defaultPanning[idx];
	_channels[idx].instrument = &_module.instruments[0];
	_channels[idx].sample = &_module.instruments[0].samples[0];
	_channels[idx].randomSeed = (idx + 1) * 0xABCDEF;
}

void ModXmS3mStream::tickChannel(Channel &channel) {
	channel.vibratoAdd = 0;
	channel.fxCount++;
	channel.retrigCount++;
	if (!(channel.note.effect == 0x7D && channel.fxCount <= channel.note.param)) {
		switch (channel.note.volume & 0xF0) {
			case 0x60: /* Vol Slide Down.*/
				channel.volume -= channel.note.volume & 0xF;
				if (channel.volume < 0) {
					channel.volume = 0;
				}
				break;
			case 0x70: /* Vol Slide Up.*/
				channel.volume += channel.note.volume & 0xF;
				if (channel.volume > 64) {
					channel.volume = 64;
				}
				break;
			case 0xB0: /* Vibrato.*/
				channel.vibratoPhase += channel.vibratoSpeed;
				vibrato(channel, 0);
				break;
			case 0xD0: /* Pan Slide Left.*/
				channel.panning -= channel.note.volume & 0xF;
				if (channel.panning < 0) {
					channel.panning = 0;
				}
				break;
			case 0xE0: /* Pan Slide Right.*/
				channel.panning += channel.note.volume & 0xF;
				if (channel.panning > 255) {
					channel.panning = 255;
				}
				break;
			case 0xF0: /* Tone Porta.*/
				tonePorta(channel);
				break;
			default:
				break;
		}
	}
	switch (channel.note.effect) {
		case 0x01:
		case 0x86: /* Porta Up. */
			portaUp(channel, channel.portaUpParam);
			break;
		case 0x02:
		case 0x85: /* Porta Down. */
			portaDown(channel, channel.portaDownParam);
			break;
		case 0x03:
		case 0x87: /* Tone Porta. */
			tonePorta(channel);
			break;
		case 0x04:
		case 0x88: /* Vibrato. */
			channel.vibratoPhase += channel.vibratoSpeed;
			vibrato(channel, 0);
			break;
		case 0x05:
		case 0x8C: /* Tone Porta + Vol Slide. */
			tonePorta(channel);
			volumeSlide(channel);
			break;
		case 0x06:
		case 0x8B: /* Vibrato + Vol Slide. */
			channel.vibratoPhase += channel.vibratoSpeed;
			vibrato(channel, 0);
			volumeSlide(channel);
			break;
		case 0x07:
		case 0x92: /* Tremolo. */
			channel.tremoloPhase += channel.tremoloSpeed;
			tremolo(channel);
			break;
		case 0x0A:
		case 0x84: /* Vol Slide. */
			volumeSlide(channel);
			break;
		case 0x11: /* Global Volume Slide. */
			_globalVol = _globalVol + (channel.gvolSlideParam >> 4) - (channel.gvolSlideParam & 0xF);
			if (_globalVol < 0) {
				_globalVol = 0;
			}
			if (_globalVol > 64) {
				_globalVol = 64;
			}
			break;
		case 0x19: /* Panning Slide. */
			channel.panning = channel.panning + (channel.panSlideParam >> 4) - (channel.panSlideParam & 0xF);
			if (channel.panning < 0) {
				channel.panning = 0;
			}
			if (channel.panning > 255) {
				channel.panning = 255;
			}
			break;
		case 0x1B:
		case 0x91: /* Retrig + Vol Slide. */
			retrigVolSlide(channel);
			break;
		case 0x1D:
		case 0x89: /* Tremor. */
			tremor(channel);
			break;
		case 0x79: /* Retrig. */
			if (channel.fxCount >= channel.note.param) {
				channel.fxCount = 0;
				channel.sampleIdx = channel.sampleFra = 0;
			}
			break;
		case 0x7C:
		case 0xFC: /* Note Cut. */
			if (channel.note.param == channel.fxCount) {
				channel.volume = 0;
			}
			break;
		case 0x7D:
		case 0xFD: /* Note Delay. */
			if (channel.note.param == channel.fxCount) {
				trigger(channel);
			}
			break;
		case 0x8A: /* Arpeggio. */
			if (channel.fxCount == 1) {
				channel.arpeggioAdd = channel.arpeggioParam >> 4;
			} else if (channel.fxCount == 2) {
				channel.arpeggioAdd = channel.arpeggioParam & 0xF;
			} else {
				channel.arpeggioAdd = channel.fxCount = 0;
			}
			break;
		case 0x95: /* Fine Vibrato. */
			channel.vibratoPhase += channel.vibratoSpeed;
			vibrato(channel, 1);
			break;
		default:
			break;
	}
	autoVibrato(channel);
	calculateFreq(channel);
	calculateAmpl(channel);
	updateEnvelopes(channel);
}

void ModXmS3mStream::volumeSlide(Channel &channel) {
	int up = channel.volSlideParam >> 4;
	int down = channel.volSlideParam & 0xF;
	if (down == 0xF && up > 0) {
		/* Fine slide up.*/
		if (channel.fxCount == 0) {
			channel.volume += up;
		}
	} else if (up == 0xF && down > 0) {
		/* Fine slide down.*/
		if (channel.fxCount == 0) {
			channel.volume -= down;
		}
	} else if (channel.fxCount > 0 || _module.fastVolSlides) {
		/* Normal.*/
		channel.volume += up - down;
	}
	if (channel.volume > 64) {
		channel.volume = 64;
	}
	if (channel.volume < 0) {
		channel.volume = 0;
	}
}

void ModXmS3mStream::portaUp(Channel &channel, int param) {
	switch (param & 0xF0) {
		case 0xE0: /* Extra-fine porta.*/
			if (channel.fxCount == 0) {
				channel.period -= param & 0xF;
			}
			break;
		case 0xF0: /* Fine porta.*/
			if (channel.fxCount == 0) {
				channel.period -= (param & 0xF) << 2;
			}
			break;
		default:/* Normal porta.*/
			if (channel.fxCount > 0) {
				channel.period -= param << 2;
			}
			break;
	}
	if (channel.period < 0) {
		channel.period = 0;
	}
}

void ModXmS3mStream::portaDown(Channel &channel, int param) {
	if (channel.period > 0) {
		switch (param & 0xF0) {
			case 0xE0: /* Extra-fine porta.*/
				if (channel.fxCount == 0) {
					channel.period += param & 0xF;
				}
				break;
			case 0xF0: /* Fine porta.*/
				if (channel.fxCount == 0) {
					channel.period += (param & 0xF) << 2;
				}
				break;
			default:/* Normal porta.*/
				if (channel.fxCount > 0) {
					channel.period += param << 2;
				}
				break;
		}
		if (channel.period > 65535) {
			channel.period = 65535;
		}
	}
}

void ModXmS3mStream::tonePorta(Channel &channel) {
	if (channel.period > 0) {
		if (channel.period < channel.portaPeriod) {
			channel.period += channel.tonePortaParam << 2;
			if (channel.period > channel.portaPeriod) {
				channel.period = channel.portaPeriod;
			}
		} else {
			channel.period -= channel.tonePortaParam << 2;
			if (channel.period < channel.portaPeriod) {
				channel.period = channel.portaPeriod;
			}
		}
	}
}

int ModXmS3mStream::waveform(Channel &channel, int phase, int type) {
	int amplitude = 0;
	switch (type) {
		default: /* Sine. */
			amplitude = sinetable[phase & 0x1F];
			if ((phase & 0x20) > 0) {
				amplitude = -amplitude;
			}
			break;
		case 6: /* Saw Up.*/
			amplitude = (((phase + 0x20) & 0x3F) << 3) - 255;
			break;
		case 1:
		case 7: /* Saw Down. */
			amplitude = 255 - (((phase + 0x20) & 0x3F) << 3);
			break;
		case 2:
		case 5: /* Square. */
			amplitude = (phase & 0x20) > 0 ? 255 : -255;
			break;
		case 3:
		case 8: /* Random. */
			amplitude = (channel.randomSeed >> 20) - 255;
			channel.randomSeed = (channel.randomSeed * 65 + 17) & 0x1FFFFFFF;
			break;
	}
	return amplitude;
}

void ModXmS3mStream::vibrato(Channel &channel, int fine) {
	int wave = waveform(channel, channel.vibratoPhase, channel.vibratoType & 0x3);
	channel.vibratoAdd = wave * channel.vibratoDepth >> (fine ? 7 : 5);
}

void ModXmS3mStream::tremolo(Channel &channel) {
	int wave = waveform(channel, channel.tremoloPhase, channel.tremoloType & 0x3);
	channel.tremoloAdd = wave * channel.tremoloDepth >> 6;
}

void ModXmS3mStream::tremor(Channel &channel) {
	if (channel.retrigCount >= channel.tremorOnTicks) {
		channel.tremoloAdd = -64;
	}
	if (channel.retrigCount >= (channel.tremorOnTicks + channel.tremorOffTicks)) {
		channel.tremoloAdd = channel.retrigCount = 0;
	}
}

void ModXmS3mStream::retrigVolSlide(Channel &channel) {
	if (channel.retrigCount >= channel.retrigTicks) {
		channel.retrigCount = channel.sampleIdx = channel.sampleFra = 0;
		switch (channel.retrigVolume) {
			case 0x1:
				channel.volume = channel.volume - 1;
				break;
			case 0x2:
				channel.volume = channel.volume - 2;
				break;
			case 0x3:
				channel.volume = channel.volume - 4;
				break;
			case 0x4:
				channel.volume = channel.volume - 8;
				break;
			case 0x5:
				channel.volume = channel.volume - 16;
				break;
			case 0x6:
				channel.volume = channel.volume * 2 / 3;
				break;
			case 0x7:
				channel.volume = channel.volume >> 1;
				break;
			case 0x8: /* ? */
				break;
			case 0x9:
				channel.volume = channel.volume + 1;
				break;
			case 0xA:
				channel.volume = channel.volume + 2;
				break;
			case 0xB:
				channel.volume = channel.volume + 4;
				break;
			case 0xC:
				channel.volume = channel.volume + 8;
				break;
			case 0xD:
				channel.volume = channel.volume + 16;
				break;
			case 0xE:
				channel.volume = channel.volume * 3 / 2;
				break;
			case 0xF:
				channel.volume = channel.volume << 1;
				break;
			default:
				break;
		}
		if (channel.volume < 0) {
			channel.volume = 0;
		}
		if (channel.volume > 64) {
			channel.volume = 64;
		}
	}
}

void ModXmS3mStream::trigger(Channel &channel) {
	int ins = channel.note.instrument;
	if (ins > 0 && ins <= _module.numInstruments) {
		channel.instrument = &_module.instruments[ins];
		int key = channel.note.key < 97 ? channel.note.key : 0;
		int sam = channel.instrument->keyToSample[key];
		Sample *sample = &channel.instrument->samples[sam];
		channel.volume = sample->volume >= 64 ? 64 : sample->volume & 0x3F;
		if (sample->panning > 0) {
			channel.panning = (sample->panning - 1) & 0xFF;
		}
		if (channel.period > 0 && sample->loopLength > 1) {
			/* Amiga trigger.*/
			channel.sample = sample;
		}
		channel.sampleOff = 0;
		channel.volEnvTick = channel.panEnvTick = 0;
		channel.fadeoutVol = 32768;
		channel.keyOn = 1;
	}
	if (channel.note.effect == 0x09 || channel.note.effect == 0x8F) {
		/* Set Sample Offset. */
		if (channel.note.param > 0) {
			channel.offsetParam = channel.note.param;
		}
		channel.sampleOff = channel.offsetParam << 8;
	}
	if (channel.note.volume >= 0x10 && channel.note.volume < 0x60) {
		channel.volume = channel.note.volume < 0x50 ? channel.note.volume - 0x10 : 64;
	}
	switch (channel.note.volume & 0xF0) {
		case 0x80: /* Fine Vol Down.*/
			channel.volume -= channel.note.volume & 0xF;
			if (channel.volume < 0) {
				channel.volume = 0;
			}
			break;
		case 0x90: /* Fine Vol Up.*/
			channel.volume += channel.note.volume & 0xF;
			if (channel.volume > 64) {
				channel.volume = 64;
			}
			break;
		case 0xA0: /* Set Vibrato Speed.*/
			if ((channel.note.volume & 0xF) > 0) {
				channel.vibratoSpeed = channel.note.volume & 0xF;
			}
			break;
		case 0xB0: /* Vibrato.*/
			if ((channel.note.volume & 0xF) > 0) {
				channel.vibratoDepth = channel.note.volume & 0xF;
			}
			vibrato(channel, 0);
			break;
		case 0xC0: /* Set Panning.*/
			channel.panning = (channel.note.volume & 0xF) * 17;
			break;
		case 0xF0: /* Tone Porta.*/
			if ((channel.note.volume & 0xF) > 0) {
				channel.tonePortaParam = channel.note.volume & 0xF;
			}
			break;
		default:
			break;
	}
	if (channel.note.key > 0) {
		if (channel.note.key > 96) {
			channel.keyOn = 0;
		} else {
			int porta = (channel.note.volume & 0xF0) == 0xF0 || channel.note.effect == 0x03 || channel.note.effect == 0x05 || channel.note.effect == 0x87 || channel.note.effect == 0x8C;
			if (!porta) {
				ins = channel.instrument->keyToSample[channel.note.key];
				channel.sample = &channel.instrument->samples[ins];
			}
			int finetune = channel.sample->finetune;
			if (channel.note.effect == 0x75 || channel.note.effect == 0xF2) {
				/* Set Fine Tune. */
				finetune = ((channel.note.param & 0xF) << 4) - 128;
			}
			int key = channel.note.key + channel.sample->relNote;
			if (key < 1) {
				key = 1;
			}
			if (key > 120) {
				key = 120;
			}
			int period = (key << 6) + (finetune >> 1);
			if (_module.linearPeriods) {
				channel.portaPeriod = 7744 - period;
			} else {
				channel.portaPeriod = 29021 * ModuleModXmS3m::moduleExp2((period << FP_SHIFT) / -768) >> FP_SHIFT;
			}
			if (!porta) {
				channel.period = channel.portaPeriod;
				channel.sampleIdx = channel.sampleOff;
				channel.sampleFra = 0;
				if (channel.vibratoType < 4) {
					channel.vibratoPhase = 0;
				}
				if (channel.tremoloType < 4) {
					channel.tremoloPhase = 0;
				}
				channel.retrigCount = channel.avCount = 0;
			}
		}
	}
}

void ModXmS3mStream::updateEnvelopes(Channel &channel) {
	if (channel.instrument->volEnv.enabled) {
		if (!channel.keyOn) {
			channel.fadeoutVol -= channel.instrument->volFadeout;
			if (channel.fadeoutVol < 0) {
				channel.fadeoutVol = 0;
			}
		}
		channel.volEnvTick = envelopeNextTick(channel.instrument->volEnv, channel.volEnvTick, channel.keyOn);
	}
	if (channel.instrument->panEnv.enabled) {
		channel.panEnvTick = envelopeNextTick(channel.instrument->panEnv, channel.panEnvTick, channel.keyOn);
	}
}

void ModXmS3mStream::autoVibrato(Channel &channel) {
	int depth = channel.instrument->vibDepth & 0x7F;
	if (depth > 0) {
		int sweep = channel.instrument->vibSweep & 0x7F;
		int rate = channel.instrument->vibRate & 0x7F;
		int type = channel.instrument->vibType;
		if (channel.avCount < sweep) {
			depth = depth * channel.avCount / sweep;
		}
		int wave = waveform(channel, channel.avCount * rate >> 2, type + 4);
		channel.vibratoAdd += wave * depth >> 8;
		channel.avCount++;
	}
}

void ModXmS3mStream::calculateFreq(Channel &channel) {
	int per = channel.period + channel.vibratoAdd;
	if (_module.linearPeriods) {
		per = per - (channel.arpeggioAdd << 6);
		if (per < 28 || per > 7680) {
			per = 7680;
		}
		channel.freq = ((_module.c2Rate >> 4) * ModuleModXmS3m::moduleExp2(((4608 - per) << FP_SHIFT) / 768)) >> (FP_SHIFT - 4);
	} else {
		if (per > 29021) {
			per = 29021;
		}
		per = (per << FP_SHIFT) / ModuleModXmS3m::moduleExp2((channel.arpeggioAdd << FP_SHIFT) / 12);
		if (per < 28) {
			per = 29021;
		}
		channel.freq = _module.c2Rate * 1712 / per;
	}
}

void ModXmS3mStream::calculateAmpl(Channel &channel) {
	int envPan = 32, envVol = channel.keyOn ? 64 : 0;
	if (channel.instrument->volEnv.enabled) {
		envVol = calculateAmpl(channel.instrument->volEnv, channel.volEnvTick);
	}
	int vol = channel.volume + channel.tremoloAdd;
	if (vol > 64) {
		vol = 64;
	}
	if (vol < 0) {
		vol = 0;
	}
	vol = (vol * _module.gain * FP_ONE) >> 13;
	vol = (vol * channel.fadeoutVol) >> 15;
	channel.ampl = (vol * _globalVol * envVol) >> 12;
	if (channel.instrument->panEnv.enabled) {
		envPan = calculateAmpl(channel.instrument->panEnv, channel.panEnvTick);
	}
	int range = (channel.panning < 128) ? channel.panning : (255 - channel.panning);
	channel.pann = channel.panning + (range * (envPan - 32) >> 5);
}

void ModXmS3mStream::updateChannelRow(Channel &channel, Note note) {
	channel.note = note;
	channel.retrigCount++;
	channel.vibratoAdd = channel.tremoloAdd = channel.arpeggioAdd = channel.fxCount = 0;
	if (!((note.effect == 0x7D || note.effect == 0xFD) && note.param > 0)) {
		/* Not note delay.*/
		trigger(channel);
	}
	switch (channel.note.effect) {
		case 0x01:
		case 0x86: /* Porta Up. */
			if (channel.note.param > 0) {
				channel.portaUpParam = channel.note.param;
			}
			portaUp(channel, channel.portaUpParam);
			break;
		case 0x02:
		case 0x85: /* Porta Down. */
			if (channel.note.param > 0) {
				channel.portaDownParam = channel.note.param;
			}
			portaDown(channel, channel.portaDownParam);
			break;
		case 0x03:
		case 0x87: /* Tone Porta. */
			if (channel.note.param > 0) {
				channel.tonePortaParam = channel.note.param;
			}
			break;
		case 0x04:
		case 0x88: /* Vibrato. */
			if ((channel.note.param >> 4) > 0) {
				channel.vibratoSpeed = channel.note.param >> 4;
			}
			if ((channel.note.param & 0xF) > 0) {
				channel.vibratoDepth = channel.note.param & 0xF;
			}
			vibrato(channel, 0);
			break;
		case 0x05:
		case 0x8C: /* Tone Porta + Vol Slide. */
			if (channel.note.param > 0) {
				channel.volSlideParam = channel.note.param;
			}
			volumeSlide(channel);
			break;
		case 0x06:
		case 0x8B: /* Vibrato + Vol Slide. */
			if (channel.note.param > 0) {
				channel.volSlideParam = channel.note.param;
			}
			vibrato(channel, 0);
			volumeSlide(channel);
			break;
		case 0x07:
		case 0x92: /* Tremolo. */
			if ((channel.note.param >> 4) > 0) {
				channel.tremoloSpeed = channel.note.param >> 4;
			}
			if ((channel.note.param & 0xF) > 0) {
				channel.tremoloDepth = channel.note.param & 0xF;
			}
			tremolo(channel);
			break;
		case 0x08: /* Set Panning.*/
			channel.panning = channel.note.param & 0xFF;
			break;
		case 0x0A:
		case 0x84: /* Vol Slide. */
			if (channel.note.param > 0) {
				channel.volSlideParam = channel.note.param;
			}
			volumeSlide(channel);
			break;
		case 0x0C: /* Set Volume. */
			channel.volume = channel.note.param >= 64 ? 64 : channel.note.param & 0x3F;
			break;
		case 0x10:
		case 0x96: /* Set Global Volume. */
			_globalVol = channel.note.param >= 64 ? 64 : channel.note.param & 0x3F;
			break;
		case 0x11: /* Global Volume Slide. */
			if (channel.note.param > 0) {
				channel.gvolSlideParam = channel.note.param;
			}
			break;
		case 0x14: /* Key Off. */
			channel.keyOn = 0;
			break;
		case 0x15: /* Set Envelope Tick. */
			channel.volEnvTick = channel.panEnvTick = channel.note.param & 0xFF;
			break;
		case 0x19: /* Panning Slide. */
			if (channel.note.param > 0) {
				channel.panSlideParam = channel.note.param;
			}
			break;
		case 0x1B:
		case 0x91: /* Retrig + Vol Slide. */
			if ((channel.note.param >> 4) > 0) {
				channel.retrigVolume = channel.note.param >> 4;
			}
			if ((channel.note.param & 0xF) > 0) {
				channel.retrigTicks = channel.note.param & 0xF;
			}
			retrigVolSlide(channel);
			break;
		case 0x1D:
		case 0x89: /* Tremor. */
			if ((channel.note.param >> 4) > 0) {
				channel.tremorOnTicks = channel.note.param >> 4;
			}
			if ((channel.note.param & 0xF) > 0) {
				channel.tremorOffTicks = channel.note.param & 0xF;
			}
			tremor(channel);
			break;
		case 0x21: /* Extra Fine Porta. */
			if (channel.note.param > 0) {
				channel.xfinePortaParam = channel.note.param;
			}
			switch (channel.xfinePortaParam & 0xF0) {
				case 0x10:
					portaUp(channel, 0xE0 | (channel.xfinePortaParam & 0xF));
					break;
				case 0x20:
					portaDown(channel, 0xE0 | (channel.xfinePortaParam & 0xF));
					break;
				default:
					break;
			}
			break;
		case 0x71: /* Fine Porta Up. */
			if (channel.note.param > 0) {
				channel.finePortaUpParam = channel.note.param;
			}
			portaUp(channel, 0xF0 | (channel.finePortaUpParam & 0xF));
			break;
		case 0x72: /* Fine Porta Down. */
			if (channel.note.param > 0) {
				channel.finePortaDownParam = channel.note.param;
			}
			portaDown(channel, 0xF0 | (channel.finePortaDownParam & 0xF));
			break;
		case 0x74:
		case 0xF3: /* Set Vibrato Waveform. */
			if (channel.note.param < 8) {
				channel.vibratoType = channel.note.param;
			}
			break;
		case 0x77:
		case 0xF4: /* Set Tremolo Waveform. */
			if (channel.note.param < 8) {
				channel.tremoloType = channel.note.param;
			}
			break;
		case 0x7A: /* Fine Vol Slide Up. */
			if (channel.note.param > 0) {
				channel.fineVslideUpParam = channel.note.param;
			}
			channel.volume += channel.fineVslideUpParam;
			if (channel.volume > 64) {
				channel.volume = 64;
			}
			break;
		case 0x7B: /* Fine Vol Slide Down. */
			if (channel.note.param > 0) {
				channel.fineVslideDownParam = channel.note.param;
			}
			channel.volume -= channel.fineVslideDownParam;
			if (channel.volume < 0) {
				channel.volume = 0;
			}
			break;
		case 0x7C:
		case 0xFC: /* Note Cut. */
			if (channel.note.param <= 0) {
				channel.volume = 0;
			}
			break;
		case 0x8A: /* Arpeggio. */
			if (channel.note.param > 0) {
				channel.arpeggioParam = channel.note.param;
			}
			break;
		case 0x95: /* Fine Vibrato.*/
			if ((channel.note.param >> 4) > 0) {
				channel.vibratoSpeed = channel.note.param >> 4;
			}
			if ((channel.note.param & 0xF) > 0) {
				channel.vibratoDepth = channel.note.param & 0xF;
			}
			vibrato(channel, 1);
			break;
		case 0xF8: /* Set Panning. */
			channel.panning = channel.note.param * 17;
			break;
		default:
			break;
	}
	autoVibrato(channel);
	calculateFreq(channel);
	calculateAmpl(channel);
	updateEnvelopes(channel);
}

int ModXmS3mStream::tick() {
	int count = 1;
	if (--_tick <= 0) {
		_tick = _speed;
		updateRow();
	} else {
		for (int idx = 0; idx < _module.numChannels; idx++) {
			tickChannel(_channels[idx]);
		}
	}
	if (_playCount && _playCount[0]) {
		count = _playCount[_seqPos][_row] - 1;
	}
	return count;
}

void ModXmS3mStream::updateRow() {
	if (_nextRow < 0) {
		_breakPos = _seqPos + 1;
		_nextRow = 0;
	}
	if (_breakPos >= 0) {
		if (_breakPos >= _module.sequenceLen) {
			_breakPos = _nextRow = 0;
		}
		while (_module.sequence[_breakPos] >= _module.numPatterns) {
			_breakPos++;
			if (_breakPos >= _module.sequenceLen) {
				_breakPos = _nextRow = 0;
			}
		}
		_seqPos = _breakPos;
		for (int idx = 0; idx < _module.numChannels; idx++) {
			_channels[idx].plRow = 0;
		}
		_breakPos = -1;
	}
	Pattern &pattern = _module.patterns[_module.sequence[_seqPos]];
	_row = _nextRow;
	if (_row >= pattern.numRows) {
		_row = 0;
	}
	if (_playCount && _playCount[0]) {
		int count = _playCount[_seqPos][_row];
		if (_plCount < 0 && count < 127) {
			_playCount[_seqPos][_row] = count + 1;
		}
	}
	_nextRow = _row + 1;
	if (_nextRow >= pattern.numRows) {
		_nextRow = -1;
	}
	for (int idx = 0; idx < _module.numChannels; ++idx) {
		Note note = pattern.getNote(_row, idx);
		if (note.effect == 0xE) {
			note.effect = 0x70 | (note.param >> 4);
			note.param &= 0xF;
		}
		if (note.effect == 0x93) {
			note.effect = 0xF0 | (note.param >> 4);
			note.param &= 0xF;
		}
		if (note.effect == 0 && note.param > 0) {
			note.effect = 0x8A;
		}

		Channel &channel = _channels[idx];
		updateChannelRow(channel, note);
		switch (note.effect) {
			case 0x81: /* Set Speed. */
				if (note.param > 0) {
					_tick = _speed = note.param;
				}
				break;
			case 0xB:
			case 0x82: /* Pattern Jump.*/
				if (_plCount < 0) {
					_breakPos = note.param;
					_nextRow = 0;
				}
				break;
			case 0xD:
			case 0x83: /* Pattern Break.*/
				if (_plCount < 0) {
					if (_breakPos < 0) {
						_breakPos = _seqPos + 1;
					}
					_nextRow = (note.param >> 4) * 10 + (note.param & 0xF);
				}
				break;
			case 0xF: /* Set Speed/Tempo.*/
				if (note.param > 0) {
					if (note.param < 32) {
						_tick = _speed = note.param;
					} else {
						_tempo = note.param;
					}
				}
				break;
			case 0x94: /* Set Tempo.*/
				if (note.param > 32) {
					_tempo = note.param;
				}
				break;
			case 0x76:
			case 0xFB: /* Pattern Loop.*/
				if (note.param == 0) {
					/* Set loop marker on this channel. */
					channel.plRow = _row;
				}
				if (channel.plRow < _row && _breakPos < 0) {
					/* Marker valid. */
					if (_plCount < 0) {
						/* Not already looping, begin. */
						_plCount = note.param;
						_plChan = idx;
					}
					if (_plChan == idx) {
						/* Next Loop.*/
						if (_plCount == 0) {
							/* Loop finished. Invalidate current marker. */
							channel.plRow = _row + 1;
						} else {
							/* Loop. */
							_nextRow = channel.plRow;
						}
						_plCount--;
					}
				}
				break;
			case 0x7E:
			case 0xFE: /* Pattern Delay.*/
				_tick = _speed + _speed * note.param;
				break;
			default:
				break;
		}
	}
}

int ModXmS3mStream::envelopeNextTick(Envelope &envelope, int currentTick, int keyOn) {
	int nextTick = currentTick + 1;
	if (envelope.looped && nextTick >= envelope.loopEndTick) {
		nextTick = envelope.loopStartTick;
	}
	if (envelope.sustain && keyOn && nextTick >= envelope.sustainTick) {
		nextTick = envelope.sustainTick;
	}
	return nextTick;
}

int ModXmS3mStream::calculateAmpl(Envelope &envelope, int currentTick) {
	int idx, point, dt, da;
	int ampl = envelope.pointsAmpl[envelope.numPoints - 1];
	if (currentTick < envelope.pointsTick[envelope.numPoints - 1]) {
		point = 0;
		for (idx = 1; idx < envelope.numPoints; idx++) {
			if (envelope.pointsTick[idx] <= currentTick) {
				point = idx;
			}
		}
		dt = envelope.pointsTick[point + 1] - envelope.pointsTick[point];
		da = envelope.pointsAmpl[point + 1] - envelope.pointsAmpl[point];
		ampl = envelope.pointsAmpl[point];
		ampl += ((da << 24) / dt) * (currentTick - envelope.pointsTick[point]) >> 24;
	}
	return ampl;
}

int ModXmS3mStream::calculateDuration() {
	int count = 0, duration = 0;
	setSequencePos(0);
	while (count < 1) {
		duration += calculateTickLength();
		count = tick();
	}
	setSequencePos(0);
	return duration;
}

/* Seek to approximately the specified sample position.
 The actual sample position reached is returned. */
int ModXmS3mStream::seek(int samplePos) {
	int currentPos = 0;
	setSequencePos(0);
	int tickLen = calculateTickLength();
	while ((samplePos - currentPos) >= tickLen) {
		for (int idx = 0; idx < _module.numChannels; ++idx) {
			updateSampleIdx(_channels[idx], tickLen * 2, _sampleRate * 2);
		}
		currentPos += tickLen;
		tick();
		tickLen = calculateTickLength();
	}
	return currentPos;
}

void ModXmS3mStream::resample(Channel &channel, int *mixBuf, int offset, int count, int sampleRate) {
	Sample *sample = channel.sample;
	int lGain = 0, rGain = 0, samIdx = 0, samFra = 0, step = 0;
	int loopLen = 0, loopEnd = 0, outIdx = 0, outEnd = 0, y = 0, m = 0, c = 0;
	int16 *sampleData = channel.sample->data;
	if (channel.ampl > 0) {
		lGain = channel.ampl * (255 - channel.pann) >> 8;
		rGain = channel.ampl * channel.pann >> 8;
		samIdx = channel.sampleIdx;
		samFra = channel.sampleFra;
		step = (channel.freq << (FP_SHIFT - 3)) / (sampleRate >> 3);
		loopLen = sample->loopLength;
		loopEnd = sample->loopStart + loopLen;
		outIdx = offset * 2;
		outEnd = (offset + count) * 2;
		if (_interpolation) {
			while (outIdx < outEnd) {
				if (samIdx >= loopEnd) {
					if (loopLen > 1) {
						while (samIdx >= loopEnd) {
							samIdx -= loopLen;
						}
					} else {
						break;
					}
				}
				c = sampleData[samIdx];
				m = sampleData[samIdx + 1] - c;
				y = ((m * samFra) >> FP_SHIFT) + c;
				mixBuf[outIdx++] += (y * lGain) >> FP_SHIFT;
				mixBuf[outIdx++] += (y * rGain) >> FP_SHIFT;
				samFra += step;
				samIdx += samFra >> FP_SHIFT;
				samFra &= FP_MASK;
			}
		} else {
			while (outIdx < outEnd) {
				if (samIdx >= loopEnd) {
					if (loopLen > 1) {
						while (samIdx >= loopEnd) {
							samIdx -= loopLen;
						}
					} else {
						break;
					}
				}
				if (samIdx < 0)
					samIdx = 0;
				y = sampleData[samIdx];
				mixBuf[outIdx++] += (y * lGain) >> FP_SHIFT;
				mixBuf[outIdx++] += (y * rGain) >> FP_SHIFT;
				samFra += step;
				samIdx += samFra >> FP_SHIFT;
				samFra &= FP_MASK;
			}
		}
	}
}

void ModXmS3mStream::updateSampleIdx(Channel &channel, int count, int sampleRate) {
	Sample *sample = channel.sample;
	int step = (channel.freq << (FP_SHIFT - 3)) / (sampleRate >> 3);
	channel.sampleFra += step * count;
	channel.sampleIdx += channel.sampleFra >> FP_SHIFT;
	if (channel.sampleIdx > (int)sample->loopStart) {
		if (sample->loopLength > 1) {
			channel.sampleIdx = sample->loopStart + (channel.sampleIdx - sample->loopStart) % sample->loopLength;
		} else {
			channel.sampleIdx = sample->loopStart;
		}
	}
	channel.sampleFra &= FP_MASK;
}

void ModXmS3mStream::volumeRamp(int *mixBuf, int tickLen) {
	int rampRate = 256 * 2048 / _sampleRate;
	for (int idx = 0, a1 = 0; a1 < 256; idx += 2, a1 += rampRate) {
		int a2 = 256 - a1;
		mixBuf[idx] = (mixBuf[idx] * a1 + _rampBuf[idx] * a2) >> 8;
		mixBuf[idx + 1] = (mixBuf[idx + 1] * a1 + _rampBuf[idx + 1] * a2) >> 8;
	}
	memcpy(_rampBuf, &mixBuf[tickLen * 2], 128 * sizeof(int));
}

/* 2:1 downsampling with simple but effective anti-aliasing. Buf must contain count * 2 + 1 stereo samples. */
void ModXmS3mStream::downsample(int *buf, int count) {
	int outLen = count * 2;
	for (int idx = 0, outIdx = 0; outIdx < outLen; idx += 4, outIdx += 2) {
		buf[outIdx] = (buf[idx] >> 2) + (buf[idx + 2] >> 1) + (buf[idx + 4] >> 2);
		buf[outIdx + 1] = (buf[idx + 1] >> 2) + (buf[idx + 3] >> 1) + (buf[idx + 5] >> 2);
	}
}

/* Generates audio and returns the number of stereo samples written into mixBuf. */
int ModXmS3mStream::getAudio(int *mixBuf) {
	if (_mixBuffer) {
		memcpy(mixBuf, _mixBuffer, _mixBufferSamples * sizeof(int));
		delete []_mixBuffer;
		_mixBuffer = nullptr;
		return _mixBufferSamples;
	}

	int tickLen = calculateTickLength();
	/* Clear output buffer. */
	memset(mixBuf, 0, (tickLen + 65) * 4 * sizeof(int));
	/* Resample. */
	for (int idx = 0; idx < _module.numChannels; idx++) {
		Channel &channel = _channels[idx];
		resample(channel, mixBuf, 0, (tickLen + 65) * 2, _sampleRate * 2);
		updateSampleIdx(channel, tickLen * 2, _sampleRate * 2);
	}
	downsample(mixBuf, tickLen + 64);
	volumeRamp(mixBuf, tickLen);
	tick();
	return tickLen * 2;  // stereo samples
}

int ModXmS3mStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = 0;
	while (samplesRead < numSamples && _dataLeft >= 0) {
		int *mixBuf = new int[calculateMixBufLength()];
		int samples = getAudio(mixBuf);
		if (samplesRead + samples > numSamples) {
			_mixBufferSamples = samplesRead + samples - numSamples;
			samples -= _mixBufferSamples;
			_mixBuffer = new int[_mixBufferSamples];
			memcpy(_mixBuffer, mixBuf + samples, _mixBufferSamples * sizeof(int));
		}
		for (int idx = 0; idx < samples; ++idx) {
			int ampl = mixBuf[idx];
			if (ampl > 32767) {
				ampl = 32767;
			}
			if (ampl < -32768) {
				ampl = -32768;
			}
			*buffer++ = ampl;
		}
		samplesRead += samples;
		delete []mixBuf; // free
	}
	_dataLeft -= samplesRead * 2;

	return samplesRead;
}

void ModXmS3mStream::setSequencePos(int pos) {
	if (pos >= _module.sequenceLen) {
		pos = 0;
	}
	_breakPos = pos;
	_nextRow = 0;
	_tick = 1;
	_globalVol = _module.defaultGvol;
	_speed = _module.defaultSpeed > 0 ? _module.defaultSpeed : 6;
	_tempo = _module.defaultTempo > 0 ? _module.defaultTempo : 125;
	_plCount = _plChan = -1;

	// play count
	if (_playCount) {
		delete[] _playCount[0];
		delete[] _playCount;
	}
	_playCount = new int8 *[_module.sequenceLen];
	memset(_playCount, 0, _module.sequenceLen * sizeof(int8 *));
	int len = initPlayCount(_playCount);
	_playCount[0] = new int8[len];
	memset(_playCount[0], 0, len * sizeof(int8));
	initPlayCount(_playCount);

	for (int idx = 0; idx < _module.numChannels; ++idx) {
		initChannel(idx);
	}
	memset(_rampBuf, 0, 128 * sizeof(int));
	tick();
}

} // End of namespace Modules

namespace Audio {

AudioStream *makeModXmS3mStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int interpolation) {
	Modules::ModXmS3mStream *soundStream = new Modules::ModXmS3mStream(stream, rate, interpolation);

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	if (!soundStream->loadSuccess()) {
		delete soundStream;
		return nullptr;
	}

	return (AudioStream *)soundStream;
}

} // End of namespace Audio
