/* ScummVM - Scumm Interpreter
 * Copyright (C) 2007 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"

#include "sound/mods/paula.h"
#include "sound/mods/soundfx.h"
#include "sound/audiostream.h"

namespace Audio {

struct SoundFxInstrument {
	char name[23];
	uint16 len;
	uint8 finetune;
	uint8 volume;
	uint16 repeatPos;
	uint16 repeatLen;
	int8 *data;
};

class SoundFx : public Paula {
public:

	enum {
		NUM_CHANNELS = 4,
		NUM_INSTRUMENTS = 15,
		CIA_FREQ = 715909
	};

	SoundFx(int rate, bool stereo);
	virtual ~SoundFx();

	bool load(Common::SeekableReadStream *data, LoadSoundFxInstrumentCallback loadCb);
	void play();

protected:

	void handlePattern(int ch, uint32 pat);
	void updateEffects(int ch);
	void handleTick();

	void startPaula();
	void stopPaula();
	void setPaulaChannelPeriod(uint8 channel, int16 period);
	void setPaulaChannelVolume(uint8 channel, uint8 volume);
	void enablePaulaChannel(uint8 channel);
	void disablePaulaChannel(uint8 channel);
	void setupPaulaChannel(uint8 channel, const int8 *data, uint16 len, uint16 repeatPos, uint16 repeatLen);

	virtual void interrupt();

	uint8 _ticks;
	uint16 _delay;
	SoundFxInstrument _instruments[NUM_INSTRUMENTS];
	uint8 _numOrders;
	uint8 _curOrder;
	uint16 _curPos;
	uint8 _ordersTable[128];
	uint8 *_patternData;
	int _eventsFreq;
	uint16 _effects[NUM_CHANNELS];
};

SoundFx::SoundFx(int rate, bool stereo)
	: Paula(stereo, rate) {
	_ticks = 0;
	_delay = 0;
	memset(_instruments, 0, sizeof(_instruments));
	_numOrders = 0;
	_curOrder = 0;
	_curPos = 0;
	memset(_ordersTable, 0, sizeof(_ordersTable));
	_patternData = 0;
	_eventsFreq = 0;
	memset(_effects, 0, sizeof(_effects));
}

SoundFx::~SoundFx() {
	free(_patternData);
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		free(_instruments[i].data);
	}
}

bool SoundFx::load(Common::SeekableReadStream *data, LoadSoundFxInstrumentCallback loadCb) {
	int instrumentsSize[15];
	if (!loadCb) {
		for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
			instrumentsSize[i] = data->readUint32BE();
		}
	}
	uint8 tag[4];
	data->read(tag, 4);
	if (memcmp(tag, "SONG", 4) != 0) {
		return false;
	}
	_delay = data->readUint16BE();
	data->skip(7 * 2);
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		SoundFxInstrument *ins = &_instruments[i];
		data->read(ins->name, 22); ins->name[22] = 0;
		ins->len = data->readUint16BE();
		ins->finetune = data->readByte();
		ins->volume = data->readByte();
		ins->repeatPos = data->readUint16BE();
		ins->repeatLen = data->readUint16BE();
	}
	_numOrders = data->readByte();
	data->skip(1);
	data->read(_ordersTable, 128);
	int maxOrder = 0;
	for (int i = 0; i < _numOrders; ++i) {
		if (_ordersTable[i] > maxOrder) {
			maxOrder = _ordersTable[i];
		}
	}
	int patternSize = (maxOrder + 1) * 4 * 4 * 64;
	_patternData = (uint8 *)malloc(patternSize);
	if (!_patternData) {
		return false;
	}
	data->read(_patternData, patternSize);
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		SoundFxInstrument *ins = &_instruments[i];
		if (!loadCb) {
			if (instrumentsSize[i] != 0) {
				assert(ins->len <= 1 || ins->len * 2 <= instrumentsSize[i]);
				assert(ins->repeatLen <= 1 || (ins->repeatPos + ins->repeatLen) * 2 <= instrumentsSize[i]);
				ins->data = (int8 *)malloc(instrumentsSize[i]);
				if (!ins->data) {
					return false;
				}
				data->read(ins->data, instrumentsSize[i]);
			}
		} else {
			if (ins->name[0]) {
				ins->name[8] = '\0';
				ins->data = (int8 *)(*loadCb)(ins->name, 0);
				if (!ins->data) {
					return false;
				}
			}
		}
	}
	return true;
}

void SoundFx::play() {
	_curPos = 0;
	_curOrder = 0;
	_ticks = 0;
	_eventsFreq = CIA_FREQ / _delay;
	setInterruptFreq(_rate / _eventsFreq);
	startPaula();
}

void SoundFx::handlePattern(int ch, uint32 pat) {
	uint16 note1 = pat >> 16;
	uint16 note2 = pat & 0xFFFF;
	if (note1 != 0xFFFD) {
		int ins = (note2 & 0xF000) >> 12;
		if (ins != 0) {
			SoundFxInstrument *i = &_instruments[ins - 1];
			setupPaulaChannel(ch, i->data, i->len, i->repeatPos, i->repeatLen);
			int effect = (note2 & 0xF00) >> 8;
			int volume = i->volume;
			switch (effect) {
			case 5: // volume up
				volume += (note2 & 0xFF);
				if (volume > 63) {
					volume = 63;
				}
				break;
			case 6: // volume down
				volume -= (note2 & 0xFF);
				if (volume < 0) {
					volume = 0;
				}
				break;
			}
			setPaulaChannelVolume(ch, volume);
		}
	}
	_effects[ch] = note2;
	if (note1 == 0xFFFD) { // PIC
		_effects[ch] = 0;
	} else if (note1 == 0xFFFE) { // STP
		disablePaulaChannel(ch);
	} else if (note1 != 0) {
		setPaulaChannelPeriod(ch, note1);
		enablePaulaChannel(ch);
	}
}

void SoundFx::updateEffects(int ch) {
	// updateEffects() is a no-op in all Delphine Software games using SoundFx : FW,OS,Cruise,AW
	if (_effects[ch] != 0) {
		switch (_effects[ch]) {
		case 1: // appreggiato
		case 2: // pitchbend
		case 3: // ledon, enable low-pass filter
		case 4: // ledoff, disable low-pass filter
		case 7: // set step up
		case 8: // set step down
			warning("Unhandled effect %d\n", _effects[ch]);
			break;
		}
	}
}

void SoundFx::handleTick() {
	++_ticks;
	if (_ticks != 6) {
		for (int ch = 0; ch < 4; ++ch) {
			updateEffects(ch);
		}
	} else {
		_ticks = 0;
		const uint8 *patternData = _patternData + _ordersTable[_curOrder] * 1024 + _curPos;
		for (int ch = 0; ch < 4; ++ch) {
			handlePattern(ch, READ_BE_UINT32(patternData));
			patternData += 4;
		}
		_curPos += 4 * 4;
		if (_curPos >= 1024) {
			_curPos = 0;
			++_curOrder;
			if (_curOrder == _numOrders) {
				stopPaula();
			}
		}
	}
}

void SoundFx::startPaula() {
	_playing = true;
	_end = false;
}

void SoundFx::stopPaula() {
	_playing = false;
	_end = true;
}

void SoundFx::setPaulaChannelPeriod(uint8 channel, int16 period) {
	_voice[channel].period = period;
}

void SoundFx::setPaulaChannelVolume(uint8 channel, uint8 volume) {
	_voice[channel].volume = volume;
}

void SoundFx::enablePaulaChannel(uint8 channel) {
}

void SoundFx::disablePaulaChannel(uint8 channel) {
	_voice[channel].period = 0;
}

void SoundFx::setupPaulaChannel(uint8 channel, const int8 *data, uint16 len, uint16 repeatPos, uint16 repeatLen) {
	if (data && len > 1) {
		Channel *ch = &_voice[channel];
		ch->data = data;
		ch->dataRepeat = data + repeatPos * 2;
		ch->length = len * 2;
		ch->lengthRepeat = repeatLen * 2;
		ch->offset = 0;
	}
}

void SoundFx::interrupt() {
	handleTick();
}

AudioStream *makeSoundFxStream(Common::SeekableReadStream *data, LoadSoundFxInstrumentCallback loadCb, int rate, bool stereo) {
	SoundFx *stream = new SoundFx(rate, stereo);
	if (stream->load(data, loadCb)) {
		stream->play();
		return stream;
	}
	delete stream;
	return 0;
}

} // End of namespace Audio
