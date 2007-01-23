/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Original ADL-Player source Copyright (C) 2004 by Patrick Combet aka Dorian Gray
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"
#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/music.h"
#include "gob/gob.h"
#include "gob/game.h"
#include "gob/util.h"

namespace Gob {

Paula::Paula(GobEngine *vm, bool stereo, int intFreq) : _vm(vm) {
	_playing = false;

	_stereo = stereo;
	_rate = _vm->_mixer->getOutputRate();
	_vm->_mixer->setupPremix(this, Audio::Mixer::kMusicSoundType);
	_intFreq = intFreq;

	clearVoices();
	_voice[0].panning = 0;
	_voice[1].panning = 1;
	_voice[2].panning = 1;
	_voice[3].panning = 0;

	if (_intFreq <= 0)
		_intFreq = _rate;

	_curInt = _intFreq;
	_end = true;
}

Paula::~Paula() {
	_vm->_mixer->setupPremix(0);
}

void Paula::clearVoice(int voice) {
	_voice[voice].data = 0;
	_voice[voice].dataRepeat = 0;
	_voice[voice].length = 0;
	_voice[voice].lengthRepeat = 0;
	_voice[voice].period = 0;
	_voice[voice].volume = 0;
	_voice[voice].offset = 0;
}

int Paula::readBuffer(int16 *buffer, const int numSamples) {
	int voice;
	int samples;
	int nSamples;
	int sLen;
	double frequency;
	double rate;
	double offset;
	int16 *p;
	int8 *data;

	memset(buffer, 0, numSamples * 2);
	if (!_playing)
		return numSamples;

	samples = _stereo ? numSamples / 2 : numSamples;
	while (samples > 0) {
		if (_curInt == _intFreq) {
			interrupt();
			_curInt = 0;
		}
		nSamples = MIN(samples, _intFreq - _curInt);
		for (voice = 0; voice < 4; voice++) {
			if (!_voice[voice].data || (_voice[voice].period <= 0))
				continue;

			frequency = (7093789.2 / 2.0) / _voice[voice].period;
			rate = frequency / _rate;
			offset = _voice[voice].offset;
			sLen = _voice[voice].length;
			data = _voice[voice].data;
			p = buffer;

			_voice[voice].volume &= 0x3F;
			if ((_voice[voice].lengthRepeat > 2) && ((int)(offset + nSamples * rate) >= sLen)) {
				int neededSamples = nSamples;

				int end = (int)((sLen - offset) / rate);

				for (int i = 0; i < end; i++)
					mix(p, data[(int)(offset + rate * i)], voice);

				_voice[voice].length = sLen = _voice[voice].lengthRepeat;
				_voice[voice].data = data = _voice[voice].dataRepeat;
				_voice[voice].offset = offset = 0;
				neededSamples -= end;

				while (neededSamples > 0) {
					if (neededSamples >= (int) ((sLen - offset) / rate)) {
						end = (int)((sLen - offset) / rate);

						for (int i = 0; i < end; i++)
							mix(p, data[(int)(offset + rate * i)], voice);

						_voice[voice].data = data = _voice[voice].dataRepeat;
						_voice[voice].length = sLen = _voice[voice].lengthRepeat;
						_voice[voice].offset = offset = 0;

						neededSamples -= end;
					} else {
						for (int i = 0; i < neededSamples; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset += rate * neededSamples;
						if (ceil(_voice[voice].offset) >= sLen) {
							_voice[voice].data = data = _voice[voice].dataRepeat;
							_voice[voice].length = sLen = _voice[voice].lengthRepeat;
							_voice[voice].offset = offset = 0;
						}
						neededSamples = 0;
					}
				}
			} else {
				if (offset < sLen) {
					if ((int)(offset + nSamples * rate) >= sLen) {
						// The end of the sample is the limiting factor

						int end = (int)((sLen - offset) / rate);
						for (int i = 0; i < end; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset = sLen;
					} else {
						// The requested number of samples is the limiting factor, not the sample

						for (int i = 0; i < nSamples; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset += rate * nSamples;
					}
				}
			}
		}
		buffer += _stereo ? nSamples * 2 : nSamples;
		_curInt += nSamples;
		samples -= nSamples;
	}
	return numSamples;
}

Infogrames::Instruments::Instruments() {
	int i;

	for (i = 0; i < 32; i++) {
		_samples[i].data = 0;
		_samples[i].dataRepeat = 0;
		_samples[i].length = 0;
		_samples[i].lengthRepeat = 0;
	}
	_count = 0;
	_sampleData = 0;
}

Infogrames::Instruments::~Instruments() {
	if (_sampleData)
		delete[] _sampleData;
}

bool Infogrames::Instruments::load(Common::SeekableReadStream &ins) {
	int i;
	uint32 fsize;
	uint32 offset[32];
	uint32 offsetRepeat[32];
	uint32 dataOffset;

	fsize = ins.readUint32BE();
	dataOffset = fsize;
	for (i = 0; (i < 32) && !ins.eos(); i++) {
		offset[i] = ins.readUint32BE();
		offsetRepeat[i] = ins.readUint32BE();
		if ((offset[i] > fsize) || (offsetRepeat[i] > fsize) ||
				(offset[i] < (ins.pos() + 4)) || (offsetRepeat[i] < (ins.pos() + 4))) {
			// Definitely no real entry anymore
			ins.seek(-8, SEEK_CUR);
			break;
		}

		dataOffset = MIN(dataOffset, MIN(offset[i], offsetRepeat[i]));
		ins.skip(4); // Unknown
		_samples[i].length = ins.readUint16BE() * 2;
		_samples[i].lengthRepeat = ins.readUint16BE() * 2;
	}

	if (dataOffset >= fsize)
		return false;

	_count = i;
	_sampleData = new int8[fsize - dataOffset];
	ins.seek(dataOffset + 4);
	ins.read(_sampleData, fsize - dataOffset);

	for (i--; i >= 0; i--) {
		_samples[i].data = _sampleData + (offset[i] - dataOffset);
		_samples[i].dataRepeat = _sampleData + (offsetRepeat[i] - dataOffset);
	}

	return true;
}

const uint16 Infogrames::periods[] =
	{0x6ACC, 0x64CC, 0x5F25, 0x59CE, 0x54C3, 0x5003, 0x4B86, 0x4747, 0x4346,
		0x3F8B, 0x3BF3, 0x3892, 0x3568, 0x3269, 0x2F93, 0x2CEA, 0x2A66, 0x2801,
		0x2566, 0x23A5, 0x21AF, 0x1FC4, 0x1DFE, 0x1C4E, 0x1ABC, 0x1936, 0x17CC,
		0x1676, 0x1533, 0x1401, 0x12E4, 0x11D5, 0x10D4, 0xFE3, 0xEFE, 0xE26, 
		0xD5B, 0xC9B, 0xBE5, 0xB3B, 0xA9B, 0xA02, 0x972, 0x8E9, 0x869, 0x7F1,
		0x77F, 0x713, 0x6AD, 0x64D, 0x5F2, 0x59D, 0x54D, 0x500, 0x4B8, 0x475,
		0x435, 0x3F8, 0x3BF, 0x38A, 0x356, 0x326, 0x2F9, 0x2CF, 0x2A6, 0x280,
		0x25C, 0x23A, 0x21A, 0x1FC, 0x1E0, 0x1C5, 0x1AB, 0x193, 0x17D, 0x167,
		0x153, 0x140, 0x12E, 0x11D, 0x10D, 0xFE, 0xF0, 0xE2, 0xD6, 0xCA, 0xBE,
		0xB4, 0xAA, 0xA0, 0x97, 0x8F, 0x87, 0x7F, 0x78, 0x70, 0x60, 0x50, 0x40,
		0x30, 0x20, 0x10, 0, 0, 0x20, 0x2020, 0x2020, 0x2020, 0x2020, 0x3030,
		0x3030, 0x3020, 0x2020, 0x2020, 0x2020, 0x2020, 0x2020, 0x2020, 0x2020,
		0x2020, 0x2090, 0x4040, 0x4040, 0x4040, 0x4040, 0x4040, 0x4040, 0x4040,
		0x400C, 0xC0C, 0xC0C, 0xC0C, 0xC0C, 0xC40, 0x4040, 0x4040, 0x4040, 0x909,
		0x909, 0x909, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101,
		0x101, 0x4040, 0x4040, 0x4040, 0xA0A, 0xA0A, 0xA0A, 0x202, 0x202, 0x202, 
		0x202, 0x202, 0x202, 0x202, 0x202, 0x202, 0x202, 0x4040, 0x4040, 0x2000};
const uint8 Infogrames::tickCount[] = {2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96};

Infogrames::Infogrames(GobEngine *vm, bool stereo) :
		Paula(vm, stereo, vm->_mixer->getOutputRate()/80) {
	_instruments = 0;
	_data = 0;
	_repCount = -1;

	reset();
}

Infogrames::~Infogrames() {
	if (_data)
		delete[] _data;
}

void Infogrames::init() {
	int i;

	_volume = 0;
	_period = 0;
	_sample = 0;
	_speedCounter = _speed;
	_newVol = 0x3F;

	for (i = 0; i < 4; i++) {
		_chn[i].cmds = 0;
		_chn[i].cmdBlocks = 0;
		_chn[i].volSlide.finetuneNeg = 0;
		_chn[i].volSlide.finetunePos = 0;
		_chn[i].volSlide.data = 0;
		_chn[i].volSlide.amount = 0;
		_chn[i].volSlide.dataOffset = 0;
		_chn[i].volSlide.flags = 0;
		_chn[i].volSlide.curDelay1 = 0;
		_chn[i].volSlide.curDelay2 = 0;
		_chn[i].periodSlide.finetuneNeg = 0;
		_chn[i].periodSlide.finetunePos = 0;
		_chn[i].periodSlide.data = 0;
		_chn[i].periodSlide.amount = 0;
		_chn[i].periodSlide.dataOffset = 0;
		_chn[i].periodSlide.flags = 0;
		_chn[i].periodSlide.curDelay1 = 0;
		_chn[i].periodSlide.curDelay2 = 0;
		_chn[i].curPeriod = 0;
		_chn[i].period = 0;
		_chn[i].curCmdBlock = 0;
		_chn[i].flags = 0;
		_chn[i].ticks = 0;
		_chn[i].tickCount = 0;
		_chn[i].periodMod = 0;
		_chn[i].field_2B = 0;
		_chn[i].field_2C = 0;
		_chn[i].field_2F = 0;
	}

	for (i = 0; i < 4; i++) {
		_chn[i].flags = 0x81;
		_chn[i].field_2B = 0x3F;
		_chn[i].field_2F = 0x3F;
	}
}

void Infogrames::reset() {
	int i;

	init();

	_volSlideBlocks = 0;
	_periodSlideBlocks = 0;
	_subSong = 0;
	_cmdBlocks = 0;
	_speedCounter = 0;
	_speed = 0;
	_newVol = 0;
	_field_1E = 8;

	for (i = 0; i < 4; i++)
		_chn[i].cmdBlockIndices = 0;
}

bool Infogrames::load(Common::SeekableReadStream &dum) {
	int subSong = 0;
	int i;
	uint32 size;

	size = dum.size();
	if (size < 20)
		return false;

	_data = new uint8[size];
	dum.seek(0);
	dum.read(_data, size);

	Common::MemoryReadStream dataStr(_data, size);

	dataStr.seek(subSong * 2);
	dataStr.seek(dataStr.readUint16BE());
	_subSong = _data + dataStr.pos();
	if (_subSong > (_data + size))
		return false;

	_speedCounter = dataStr.readUint16BE();
	_speed = _speedCounter;
	_volSlideBlocks = _subSong + dataStr.readUint16BE();
	_periodSlideBlocks = _subSong + dataStr.readUint16BE();
	for (i = 0; i < 4; i++) {
		_chn[i].cmdBlockIndices = _subSong + dataStr.readUint16BE();
		_chn[i].flags = 0x81;
		_chn[i].field_2B = 0x3F;
		_chn[i].field_2F = 0x3F;
	}
	_cmdBlocks = _data + dataStr.pos() + 2;
	_newVol = 0x3F;

	if ((_volSlideBlocks > (_data + size)) ||
			(_periodSlideBlocks > (_data + size)) ||
			(_chn[0].cmdBlockIndices > (_data + size)) ||
			(_chn[1].cmdBlockIndices > (_data + size)) ||
			(_chn[2].cmdBlockIndices > (_data + size)) ||
			(_chn[3].cmdBlockIndices > (_data + size)) ||
			(_cmdBlocks > (_data + size)))
		return false;

	_end = false;
	return true;
}

void Infogrames::unload(bool destroyInstruments) {
	stopPlay();

	if (_data)
		delete[] _data;
	_data = 0;

	clearVoices();
	reset();

	if (destroyInstruments) {
		if (_instruments)
			delete _instruments;
		_instruments = 0;
	}

	_end = true;
}

void Infogrames::getNextSample(Channel &chn) {
	byte *data;
	byte cmdBlock;
	uint16 cmd;
	bool cont = false;

	if (chn.flags & 64)
		return;

	if (chn.field_2B != chn.field_2F) {
		chn.field_2C++;
		if (chn.field_2C > _field_1E) {
			chn.field_2C = 0;
			if (chn.field_2F <= chn.field_2B)
				chn.field_2B--;
			else
				chn.field_2B++;
		}
	}

	if (chn.flags & 1) {
		chn.flags &= ~1;
		chn.cmdBlocks = chn.cmdBlockIndices;
		chn.curCmdBlock = 0;
	} else {
		chn.flags &= ~1;
		if (_speedCounter == 0)
			chn.ticks--;
		if (chn.ticks != 0) {
			_volume = MAX(0, tune(chn.volSlide, 0) - (0x3F - MAX(chn.field_2B, _newVol)));
			_period = tune(chn.periodSlide, chn.curPeriod);
			return;
		} else {
			chn.ticks = chn.tickCount;
			cont = true;
		}
	}

	while (1) {
		while (cont || ((cmdBlock = *chn.cmdBlocks) != 0xFF)) {
			if (!cont) {
				chn.cmdBlocks++;
				chn.curCmdBlock++;
				chn.cmds = _subSong + READ_BE_UINT16(_cmdBlocks + (cmdBlock * 2));
			} else
				cont = false;
			while ((cmd = *chn.cmds) != 0xFF) {
				chn.cmds++;
				if (cmd & 128)
				{
					switch (cmd & 0xE0) {
					case 0x80: // 100xxxxx - Set ticks
						chn.ticks = tickCount[cmd & 0xF];
						chn.tickCount = tickCount[cmd & 0xF];
						break;
					case 0xA0: // 101xxxxx - Set sample
						_sample = cmd & 0x1F;
						break;
					case 0xC0: // 110xxxxx - Set volume slide/finetune
						data = _volSlideBlocks + (cmd & 0x1F) * 13;
						chn.volSlide.flags = (*data & 0x80) | 1;
						chn.volSlide.amount = *data++ & 0x7F;
						chn.volSlide.data = data;
						chn.volSlide.dataOffset = 0;
						chn.volSlide.finetunePos = 0;
						chn.volSlide.finetuneNeg = 0;
						chn.volSlide.curDelay1 = 0;
						chn.volSlide.curDelay2 = 0;
						break;
					case 0xE0: // 111xxxxx - Extended
						switch(cmd & 0x1F) {
						case 0: // Set period modifier
							chn.periodMod = (int8) *chn.cmds++;
							break;
						case 1: // Set continuous period slide
							chn.periodSlide.data = _periodSlideBlocks + *chn.cmds++ * 13 + 1;
							chn.periodSlide.amount = 0;
							chn.periodSlide.dataOffset = 0;
							chn.periodSlide.finetunePos = 0;
							chn.periodSlide.finetuneNeg = 0;
							chn.periodSlide.curDelay1 = 0;
							chn.periodSlide.curDelay2 = 0;
							chn.periodSlide.flags = 0x81;
							break;
						case 2: // Set non-continuous period slide
							chn.periodSlide.data = _periodSlideBlocks + *chn.cmds++ * 13 + 1;
							chn.periodSlide.amount = 0;
							chn.periodSlide.dataOffset = 0;
							chn.periodSlide.finetunePos = 0;
							chn.periodSlide.finetuneNeg = 0;
							chn.periodSlide.curDelay1 = 0;
							chn.periodSlide.curDelay2 = 0;
							chn.periodSlide.flags = 1;
							break;
						case 3: // NOP
							break;
						default:
							warning("Unknown Infogrames command: %X", cmd);
						}
						break;
					}
				} else { // 0xxxxxxx - Set period
					if (cmd != 0)
						cmd += chn.periodMod;
					chn.curPeriod = periods[cmd];
					chn.period = periods[cmd];
					chn.volSlide.dataOffset = 0;
					chn.volSlide.finetunePos = 0;
					chn.volSlide.finetuneNeg = 0;
					chn.volSlide.curDelay1 = 0;
					chn.volSlide.curDelay2 = 0;
					chn.volSlide.flags |= 1;
					chn.volSlide.flags &= ~4;
					chn.periodSlide.dataOffset = 0;
					chn.periodSlide.finetunePos = 0;
					chn.periodSlide.finetuneNeg = 0;
					chn.periodSlide.curDelay1 = 0;
					chn.periodSlide.curDelay2 = 0;
					chn.periodSlide.flags |= 1;
					chn.periodSlide.flags &= ~4;
					_volume = MAX(0, tune(chn.volSlide, 0) - (0x3F - MAX(chn.field_2B, _newVol)));
					_period = tune(chn.periodSlide, chn.curPeriod);
					return;
				}
			}
		}
		if (chn.flags & 32) {
			chn.cmdBlocks = chn.cmdBlockIndices;
			chn.curCmdBlock = 0;
		} else {
			chn.flags |= 0x40;
			_volume = 0;
			return;
		}
	}
}

int16 Infogrames::tune(Slide &slide, int16 start) const {
	byte *data;
	uint8 off;

	data = slide.data + slide.dataOffset;

	if (slide.flags & 1)
		slide.finetunePos += (int8) data[1];
	slide.flags &= ~1;

	start += slide.finetunePos - slide.finetuneNeg;
	if (start < 0)
		start = 0;

	if (slide.flags & 4)
		return start;

	slide.curDelay1++;
	if (slide.curDelay1 != data[2])
		return start;
	slide.curDelay2++;
	slide.curDelay1 = 0;
	if (slide.curDelay2 == data[0]) {
		slide.curDelay2 = 0;
		off = slide.dataOffset + 3;
		if (off == 12) {
			if (slide.flags == 0) {
				slide.flags |= 4;
				return start;
			} else {
				slide.curDelay2 = 0;
				slide.finetuneNeg += slide.amount;
				off = 3;
			}
		}
		slide.dataOffset = off;
	} 
	slide.flags |= 1;
	return start;
}

void Infogrames::interrupt() {
	int chn;

	if (!_data) {
		clearVoices();
		return;
	}

	_speedCounter--;
	_sample = 0xFF;
	for (chn = 0; chn < 4; chn++) {
		_volume = 0;
		_period = 0;
		getNextSample(_chn[chn]);
		_voice[chn].volume = _volume;
		_voice[chn].period = _period;
		if ((_sample != 0xFF) && (_sample < _instruments->_count)) {
			_voice[chn].data = _instruments->_samples[_sample].data;
			_voice[chn].length = _instruments->_samples[_sample].length;
			_voice[chn].dataRepeat = _instruments->_samples[_sample].dataRepeat;
			_voice[chn].lengthRepeat = _instruments->_samples[_sample].lengthRepeat;
			_voice[chn].offset = 0;
			_sample = 0xFF;
		}
	}
	if (_speedCounter == 0)
		_speedCounter = _speed;

	// End reached?
	if ((_chn[0].flags & 64) && (_chn[1].flags & 64) &&
			(_chn[2].flags & 64) && (_chn[3].flags & 64)) {
		if (_repCount > 0) {
			_repCount--;
			init();
		} else if (_repCount == -1)
			init();
		else
			_end = true;
	}
}

const char *Adlib::_tracks[][2] = {
	{"avt00.tot",  "mine"},
	{"avt001.tot", "nuit"},
	{"avt002.tot", "campagne"},
	{"avt003.tot", "extsor1"},
	{"avt004.tot", "interieure"},
	{"avt005.tot", "zombie"},
	{"avt006.tot", "zombie"},
	{"avt007.tot", "campagne"},
	{"avt008.tot", "campagne"},
	{"avt009.tot", "extsor1"},
	{"avt010.tot", "extsor1"},
	{"avt011.tot", "interieure"},
	{"avt012.tot", "zombie"},
	{"avt014.tot", "nuit"},
	{"avt015.tot", "interieure"},
	{"avt016.tot", "statue"},
	{"avt017.tot", "zombie"},
	{"avt018.tot", "statue"},
	{"avt019.tot", "mine"},
	{"avt020.tot", "statue"},
	{"avt021.tot", "mine"},
	{"avt022.tot", "zombie"}
};

const char *Adlib::_trackFiles[] = {
//	"musmac1.adl", // TODO: This track isn't played correctly at all yet
	"musmac2.adl",
	"musmac3.adl",
	"musmac4.adl",
	"musmac5.adl",
	"musmac6.adl"
};

const unsigned char Adlib::_operators[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const unsigned char Adlib::_volRegNums[] = { 
	3,  4,  5,
	11, 12, 13,
	19, 20, 21
};

Adlib::Adlib(GobEngine *vm) : _vm(vm) {
	int i;

	_index = -1;
	_data = 0;
	_playPos = 0;
	_dataSize = 0;
	_rate = _vm->_mixer->getOutputRate();
	_opl = makeAdlibOPL(_rate);
	_vm->_mixer->setupPremix(this, Audio::Mixer::kMusicSoundType);
	_first = true;
	_ended = false;
	_playing = false;
	_needFree = false;
	_repCount = -1;
	_samplesTillPoll = 0;

	for (i = 0; i < 16; i ++)
		_pollNotes[i] = 0;

	setFreqs();
}

Adlib::~Adlib(void) {
	OPLDestroy(_opl);
	if (_data && _needFree)
		delete[] _data;
	_vm->_mixer->setupPremix(0);
}

void Adlib::premixerCall(int16 *buf, uint len) {
	_mutex.lock();
	if (!_playing) {
		memset(buf, 0, 2 * len * sizeof(int16));
		_mutex.unlock();
		return;
	}
	else {
		if (_first) {
			memset(buf, 0, 2 * len * sizeof(int16));
			pollMusic();
			_mutex.unlock();
			return;
		}
		else {
			uint32 render;
			int16 *data = buf;
			uint datalen = len;
			while (datalen && _playing) {
				if (_samplesTillPoll) {
					render = (datalen > _samplesTillPoll) ? (_samplesTillPoll) : (datalen);
					datalen -= render;
					_samplesTillPoll -= render;
					YM3812UpdateOne(_opl, data, render);
					data += render;
				} else {
					pollMusic();
					if (_ended) {
						memset(data, 0, datalen * sizeof(int16));
						datalen = 0;
					}
				}
			}
		}
		if (_ended) {
			_first = true;
			_ended = false;
			_playPos = _data + 3 + (_data[1] + 1) * 0x38;
			_samplesTillPoll = 0;
			if (_repCount == -1) {
				reset();
				setVoices();
			} else if (_repCount > 0) {
				_repCount--;
				reset();
				setVoices();
			}
			else
				_playing = false;
		}
		// Convert mono data to stereo
		for (int i = (len - 1); i >= 0; i--) {
			buf[2 * i] = buf[2 * i + 1] = buf[i];
		}
	}
	_mutex.unlock();
}

void Adlib::writeOPL(byte reg, byte val) {
	debugC(6, kDebugMusic, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void Adlib::setFreqs(void) {
	byte lin;
	byte col;
	long val = 0;

	// Run through the 11 channels
	for (lin = 0; lin < 11; lin ++) {
		_notes[lin] = 0;
		_notCol[lin] = 0;
		_notLin[lin] = 0;
		_notOn[lin] = false;
	} 
		
	// Run through the 25 lines
	for (lin = 0; lin < 25; lin ++) {
		// Run through the 12 columns
		for (col = 0; col < 12; col ++) {
			if (!col)
				val = (((0x2710L + lin * 0x18) * 0xCB78 / 0x3D090) << 0xE) * 9 / 0x1B503;
			_freqs[lin][col] = (short)((val + 4) >> 3);
			val = val * 0x6A / 0x64;
	//      val = val *  392 / 370;
		} 
	}
}

void Adlib::reset() {
	OPLResetChip(_opl);
	_samplesTillPoll = 0;

	setFreqs();
	// Set frequencies and octave to 0; notes off
	for (int i = 0; i < 9; i++) {
		writeOPL(0xA0 | i, 0);
		writeOPL(0xB0 | i, 0);
		writeOPL(0xE0 | _operators[i]    , 0);
		writeOPL(0xE0 | _operators[i] + 3, 0);
	}

	// Authorize the control of the waveformes
	writeOPL(0x01, 0x20);
}

void Adlib::setVoices() {
	// Definitions of the 9 instruments
	for (int i = 0; i < 9; i++)
		setVoice(i, i, true);
}

void Adlib::setVoice(byte voice, byte instr, bool set) {
	int i;
	int j;
	uint16 strct[27];
	byte channel;
	byte *dataPtr;

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (i = 0; i < 2; i++) {
		dataPtr = _data + 3 + instr * 0x38 + i * 0x1A;
		for (j = 0; j < 27; j++) {
			strct[j] = READ_LE_UINT16(dataPtr);
			dataPtr += 2;
		}
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice  , ((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
		writeOPL(0x60 | channel, ((strct[3] & 0xF) << 4) | (strct[6] & 0xF));
		writeOPL(0x80 | channel, ((strct[4] & 0xF) << 4) | (strct[7] & 0xF));
		writeOPL(0x20 | channel, ((strct[9] & 1) << 7) |
			((strct[10] & 1) << 6) | ((strct[5] & 1) << 5) |
			((strct[11] & 1) << 4) |  (strct[1] & 0xF));
		if (!i)
			writeOPL(0xE0 | channel, (strct[26] & 3));
		else
			writeOPL(0xE0 | channel, (strct[14] & 3));
		if (i && set)
			writeOPL(0x40 | channel, 0);
	}
}

void Adlib::setKey(byte voice, byte note, bool on, bool spec) {
	short freq = 0;
	short octa = 0;

	// Instruction AX
	if (spec) {
		// 0x7F donne 0x16B;
		//     7F
		// <<   7 =  3F80
		// + E000 = 11F80
		// & FFFF =  1F80
		// *   19 = 31380
		// / 2000 =    18 => Ligne 18h, colonne  0 => freq 16B

		// 0x3A donne 0x2AF;
		//     3A
		// <<   7 =  1D00
		// + E000 =  FD00 négatif
		// *   19 = xB500
		// / 2000 =    -2 => Ligne 17h, colonne -1

		//     2E
		// <<   7 =  1700
		// + E000 =  F700 négatif
		// *   19 = x1F00
		// / 2000 =
		short a;
		short lin;
		short col;

		a = (note << 7) + 0xE000; // Volontairement tronqué
		a = (short)((long)a * 25 / 0x2000);
		if (a < 0) {
			col = - ((24 - a) / 25);
			lin = (-a % 25);
			if (lin)
				lin = 25 - lin;
		}
		else {
			col = a / 25;
			lin = a % 25;
		}

		_notCol[voice] = col;
		_notLin[voice] = lin;
		note = _notes[voice];
	}
	// Instructions 0X 9X 8X
	else {
		note -= 12;
		_notOn[voice] = on;
	}

	_notes[voice] = note;
	note += _notCol[voice];
	note = MIN(0x5F, (int)note);
	octa = note / 12;
	freq = _freqs[_notLin[voice]][note - octa * 12];

	writeOPL(0xA0 + voice,  freq & 0xff);
	writeOPL(0xB0 + voice, (freq >> 8) | (octa << 2) | 0x20 * on);

	if (!freq)
		warning("Voice %d, note %02X unknown\n", voice, note);
}

void Adlib::setVolume(byte voice, byte volume) {
	volume = 0x3F - (volume * 0x7E + 0x7F) / 0xFE;
	writeOPL(0x40 + _volRegNums[voice], volume);
}

void Adlib::pollMusic(void) {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint16 tempo;

	if ((_playPos > (_data + _dataSize)) && (_dataSize != (uint32) -1)) {
		_ended = true;
		return;
	}

	// First tempo, we'll ignore it...
	if (_first) {
		tempo = *(_playPos++);
		// Tempo on 2 bytes
		if (tempo & 0x80)
			tempo = ((tempo & 3) << 8) | *(_playPos++);
	}
	_first = false;

	// Instruction
	instr = *(_playPos++);
	channel = instr & 0x0F;

	switch (instr & 0xF0) {
		// Note on + Volume
		case 0x00:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setVolume(channel, *(_playPos++));
			setKey(channel, note, true, false);
			break;
		case 0x10:
			warning("GOB2 Stub! ADL command 0x10");
			break;
		case 0x50:
			warning("GOB2 Stub! ADL command 0x50");
			break;
		// Note on
		case 0x90:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setKey(channel, note, true, false);
			break;
		case 0x60:
			warning("GOB2 Stub! ADL command 0x60");
			break;
		// Last note off
		case 0x80:
			note = _pollNotes[channel];
			setKey(channel, note, false, false);
			break;
		// Frequency on/off
		case 0xA0:
			note = *(_playPos++);
			setKey(channel, note, _notOn[channel], true);
			break;
		// Volume
		case 0xB0:
			volume = *(_playPos++);
			setVolume(channel, volume);
			break;
		// Program change
		case 0xC0:
			setVoice(channel, *(_playPos++), false);
			break;
		// Special
		case 0xF0:
			switch (instr & 0x0F) {
			case 0xF: // End instruction
				_ended = true;
				_samplesTillPoll = 0;
				return;
			default:
				warning("Unknown special command in ADL, stopping playback: %X", instr & 0x0F);
				_repCount = 0;
				_ended = true;
				break;
			}
			break;
		default:
			warning("Unknown command in ADL, stopping playback: %X", instr & 0xF0);
			_repCount = 0;
			_ended = true;
			break;
	}

	// Temporization
	tempo = *(_playPos++);
	// End tempo
	if (tempo == 0xFF) {
		_ended = true;
		return;
	}
	// Tempo on 2 bytes
	if (tempo & 0x80)
		tempo = ((tempo & 3) << 8) | *(_playPos++);
	if (!tempo)
		tempo ++;

	_samplesTillPoll = tempo * (_rate / 1000);
}

void Adlib::startPlay(void) {
	if (!_data)
		return;
	
	_playing = true;
}

void Adlib::playBgMusic(void) {
	for (int i = 0; i < ARRAYSIZE(_tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, _tracks[i][0])) {
			playTrack(_tracks[i][1]);
			break;
		}
}

void Adlib::playTrack(const char *trackname) {
	if (_playing) return;
	
	debugC(1, kDebugMusic, "Adlib::playTrack(%s)", trackname);
	unload();
	load(_trackFiles[_vm->_util->getRandom(ARRAYSIZE(_trackFiles))]);
	startPlay();
}

bool Adlib::load(const char *filename) {
	Common::File song;

	unload();
	song.open(filename);
	if (!song.isOpen())
		return false;

	_needFree = true;
	_dataSize = song.size();
	_data = new byte[_dataSize];
	song.read(_data, _dataSize);
	song.close();

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
	
	return true;
}

void Adlib::load(byte *data, int index) {
	unload();
	_repCount = 0;

	_dataSize = (uint32) -1;
	_data = data;
	_index = index;

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
}

void Adlib::unload(void) {
	_playing = false;
	_index = -1;

	if (_data && _needFree)
		delete[] _data;

	_needFree = false;
}

} // End of namespace Gob
