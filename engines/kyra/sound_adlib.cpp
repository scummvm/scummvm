/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
#include "common/system.h"
#include "common/mutex.h"
#include "common/timer.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "sound/mixer.h"
#include "sound/fmopl.h"
#include "sound/audiostream.h"

// TODO:
//   - check how the sounds are stopped (doesn't work atm whyever)
//   - implement music pausing + stop and fadeing
//   - check why the sfx sounds strange sometimes
//   - implement stateCallback1_1

// Basic Adlib Programming:
// http://www.gamedev.net/reference/articles/article446.asp

namespace Kyra {

class AdlibDriver : public AudioStream {
public:
	AdlibDriver(Audio::Mixer *mixer);
	~AdlibDriver();

	int callback(int opcode, ...);
	void callback();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		memset(buffer, 0, sizeof(int16)*numSamples);
		lock();
		YM3812UpdateOne(_adlib, buffer, numSamples);
		unlock();
		return numSamples;
	}

	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

private:
	struct OpcodeEntry {
		typedef int (AdlibDriver::*DriverOpcode)(va_list &list);
		DriverOpcode function;
		const char *name;
	};

	static const OpcodeEntry _opcodeList[];
	static const int _opcodesEntries;

	int snd_ret0x100(va_list &list);
	int snd_ret0x1983(va_list &list);
	int snd_initDriver(va_list &list);
	int snd_deinitDriver(va_list &list);
	int snd_setSoundData(va_list &list);
	int snd_unkOpcode1(va_list &list);
	int snd_startSong(va_list &list);
	int snd_unkOpcode2(va_list &list);
	int snd_unkOpcode3(va_list &list);
	int snd_readByte(va_list &list);
	int snd_writeByte(va_list &list);
	int snd_setUnk5(va_list &list);
	int snd_unkOpcode4(va_list &list);
	int snd_dummy(va_list &list);
	int snd_getNullvar4(va_list &list);
	int snd_setNullvar3(va_list &list);
	int snd_setFlag(va_list &list);
	int snd_clearFlag(va_list &list);

	struct OutputState {
		uint8 unk27;
		uint8 *dataptr;
		uint8 unk5;
		uint8 repeatCounter;
		uint8 unk10;
		int8 unk2;
		uint8 dataptrStackPos;
		uint8 *dataptrStack[4];
		uint8 unk14;
		uint8 unk29;
		int8 unk31;
		uint16 unk30;
		uint16 unk37;
		uint8 unk33;
		uint8 unk34;
		uint8 unk35;
		uint8 unk36;
		uint8 unk32;
		uint8 unk41;
		uint8 unk38;
		uint8 unk26;
		uint8 unk7;
		uint8 unk15;
		uint8 unk1;
		uint8 unk4;
		uint8 unk17;
		uint8 unkOutputValue1;
		typedef void (AdlibDriver::*Callback)(OutputState&);
		Callback callback1;
		Callback callback2;
		uint8 unk12;
		uint8 unk24;
		uint8 unk25;
		uint8 unk28;
		uint8 unk23;
		uint8 unk39;	
		uint8 unk40;
		uint8 unk3;
		uint8 unk11;
		uint8 unk19;
		uint8 unk18;
		uint8 unk20;
		uint8 unk21;
		uint8 unk22;
		uint16 offset;
		uint8 unk6;
		uint8 unk13;
		int8 unk16;
	};

	void stateCallback1_1(OutputState &state);
	void stateCallback1_2(OutputState &state);
	void stateCallback2_1(OutputState& state);

	void resetAdlibState();
	void writeOPL(byte reg, byte val);
	void initTable(OutputState &table);
	void unkOutput1(OutputState &table);
	void unkOutput2(uint8 num);

	uint16 updateUnk6Value();
	void update1(uint8 unk1, OutputState &state);

	void updateAndOutput1(uint8 unk1, OutputState &state);
	void updateAndOutput2(uint8 unk1, uint8 *dataptr, OutputState &state);
	void updateAndOutput3(OutputState &state);

	void adjustVolume(OutputState &state);

	uint8 calculateLowByte1(OutputState &state);
	uint8 calculateLowByte2(OutputState &state);

	uint16 checkValue(int16 val) {
		if (val < 0)
			val = 0;
		if (val > 0x3F)
			val = 0x3F;
		return val;
	}

	void callbackOutput();
	void callbackProcess();

	struct ParserOpcode {
		typedef int (AdlibDriver::*POpcode)(uint8 *&dataptr, OutputState &state, uint8 value);
		POpcode function;
		const char *name;
	};
	static const ParserOpcode _parserOpcodeTable[];
	static const int _parserOpcodeTableSize;

	int update_setRepeat(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_checkRepeat(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback3(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback4(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_jump(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_jumpToSubroutine(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_returnFromSubroutine(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback8(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback9(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback10(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_writeAdlib(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback12(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback13(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback14(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback15(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback16(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback17(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback18(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback19(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback20(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback21(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback22(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback23(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback24(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback25(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback26(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback27(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback28(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback29(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback30(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback31(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback32(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback33(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback34(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_setAMDepth(uint8 *&dataptr, OutputState &state, uint8 value);
	int update_setVibratoDepth(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback37(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback38(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback39(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback40(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback41(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback42(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback43(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback44(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback45(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback46(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback47(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback48(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback49(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback50(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback51(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback52(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback53(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback54(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback55(uint8 *&dataptr, OutputState &state, uint8 value);
	int updateCallback56(uint8 *&dataptr, OutputState &state, uint8 value);
private:
	int _lastProcessed;
	uint8 _flagTrigger;
	int _curTable;
	uint8 _unk4;
	uint8 _unk5;
	int _soundsPlaying;

	uint16 _unk6;
	uint8 _continueFlag;

	uint8 _unkValue1;
	uint8 _unkValue2;
	uint8 _unkValue3;
	uint8 _unkValue4;
	uint8 _unkValue5;
	uint8 _unkValue6;
	uint8 _unkValue7;
	uint8 _unkValue8;
	uint8 _unkValue9;
	uint8 _unkValue10;
	uint8 _unkValue11;
	uint8 _unkValue12;
	uint8 _unkValue13;
	uint8 _unkValue14;
	uint8 _unkValue15;
	uint8 _unkValue16;
	uint8 _unkValue17;
	uint8 _unkValue18;
	uint8 _unkValue19;
	uint8 _unkValue20;

	int _flags;
	FM_OPL *_adlib;

	uint8 *_soundData;

	uint8 _soundIdTable[0x10];
	OutputState _outputTables[10];

	uint8 _unkOutputByte2;
	uint8 _unkOutputByte1;
	uint8 _unkTableByte1;

	const uint8 *_tablePtr1;
	const uint8 *_tablePtr2;

	static const uint8 _outputTable[];
	static const uint16 _unkTable[];
	static const uint8 *_unkTable2[];
	static const uint8 _unkTable2_1[];
	static const uint8 _unkTable2_2[];
	static const uint8 _unkTable2_3[];
	static const uint8 _unkTables[][32];

	Common::Mutex _mutex;
	Audio::Mixer *_mixer;

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
};

void AdlibTimerCall(void *refCon) {
	AdlibDriver *driver = (AdlibDriver*)refCon;
	driver->callback();
}

AdlibDriver::AdlibDriver(Audio::Mixer *mixer) {
	_mixer = mixer;

	_flags = 0;
	_adlib = makeAdlibOPL(getRate());
	assert(_adlib);

	memset(_outputTables, 0, sizeof(_outputTables));
	_soundData = 0;

	_unkOutputByte2 = _unkOutputByte1 = 0;

	_lastProcessed = _flagTrigger = _curTable = _unk4 = 0;
	_unk6 = 0x1234;
	_continueFlag = 0;

	_unkTableByte1 = 0;

	_unkValue3 = 0xFF;
	_unkValue1 = _unkValue2 = _unkValue4 = _unkValue5 = 0;
	_unkValue6 = _unkValue7 = _unkValue8 = _unkValue9 = _unkValue10 = 0;
	_unkValue11 = _unkValue12 = _unkValue13 = _unkValue14 = _unkValue15 =
	_unkValue16 = _unkValue17 = _unkValue18 = _unkValue19 = _unkValue20 = 0;

	_tablePtr1 = _tablePtr2 = 0;

	_mixer->setupPremix(this);

	Common::g_timer->installTimerProc(&AdlibTimerCall, 13888, this);
}

AdlibDriver::~AdlibDriver() {
	Common::g_timer->removeTimerProc(&AdlibTimerCall);
	_mixer->setupPremix(0);
	OPLDestroy(_adlib);
	_adlib = 0;
}

int AdlibDriver::callback(int opcode, ...) {
	lock();
	if (opcode >= _opcodesEntries || opcode < 0) {
		warning("AdlibDriver: calling unknown opcode '%d'", opcode);
		return 0;
	}

	debugC(9, kDebugLevelSound, "Calling opcode '%s' (%d)", _opcodeList[opcode].name, opcode);

	va_list args;
	va_start(args, opcode);
	int returnValue = (this->*(_opcodeList[opcode].function))(args);
	va_end(args);
	unlock();
	return returnValue;
}

// Opcodes

int AdlibDriver::snd_ret0x100(va_list &list) {
	return 0x100;
}

int AdlibDriver::snd_ret0x1983(va_list &list) {
	return 0x1983;
}

int AdlibDriver::snd_initDriver(va_list &list) {
	_lastProcessed = _soundsPlaying = 0;
	resetAdlibState();
	return 0;
}

int AdlibDriver::snd_deinitDriver(va_list &list) {
	resetAdlibState();
	return 0;
}

int AdlibDriver::snd_setSoundData(va_list &list) {
	if (_soundData) {
		delete [] _soundData;
		_soundData = 0;
	}
	_soundData = va_arg(list, uint8*);
	return 0;
}

int AdlibDriver::snd_unkOpcode1(va_list &list) {
	warning("unimplemented snd_unkOpcode1");
	return 0;
}

int AdlibDriver::snd_startSong(va_list &list) {
	int songId = va_arg(list, int);
	_flags |= 8;
	_flagTrigger = 1;
	uint16 offset = READ_LE_UINT16(&_soundData[songId << 1]);
	uint8 firstByte = *(_soundData + offset);

	if ((songId << 1) != 0) {
		if (firstByte == 9) {
			if (_flags & 2)
				return 0;
		} else {
			if (_flags & 1)
				return 0;
		}
	}

	_soundIdTable[_soundsPlaying] = songId;
	++_soundsPlaying;
	_soundsPlaying &= 0x0F;

	return 0;
}

int AdlibDriver::snd_unkOpcode2(va_list &list) {
	warning("unimplemented snd_unkOpcode2");
	return 0;
}

int AdlibDriver::snd_unkOpcode3(va_list &list) {
	int value = va_arg(list, int);
	int loop = value;
	if (value < 0) {
		value = 0;
		loop = 9;
	}
	loop -= value;
	++loop;

	while (loop--) {
		_curTable = value;
		OutputState &table = _outputTables[_curTable];
		table.unk2 = 0;
		table.dataptr = 0;
		if (value != 9) {
			unkOutput1(table);
		}
		++value;
	}

	return 0;
}

int AdlibDriver::snd_readByte(va_list &list) {
	int a = va_arg(list, int);
	int b = va_arg(list, int);
	uint8 *ptr = _soundData + READ_LE_UINT16(&_soundData[a << 1]) + b;
	return *ptr;
}

int AdlibDriver::snd_writeByte(va_list &list) {
	int a = va_arg(list, int);
	int b = va_arg(list, int);
	int c = va_arg(list, int);
	uint8 *ptr = _soundData + READ_LE_UINT16(&_soundData[a << 1]) + b;
	uint8 oldValue = *ptr;
	*ptr = (uint8)c;
	return oldValue;
}

int AdlibDriver::snd_setUnk5(va_list &list) {
	warning("unimplemented snd_setUnk5");
	return 0;
}

int AdlibDriver::snd_unkOpcode4(va_list &list) {
	warning("unimplemented snd_unkOpcode4");
	return 0;
}

int AdlibDriver::snd_dummy(va_list &list) {
	return 0;
}

int AdlibDriver::snd_getNullvar4(va_list &list) {
	warning("unimplemented snd_getNullvar4");
	return 0;
}

int AdlibDriver::snd_setNullvar3(va_list &list) {
	warning("unimplemented snd_setNullvar3");
	return 0;
}

int AdlibDriver::snd_setFlag(va_list &list) {
	int oldFlags = _flags;
	_flags |= va_arg(list, int);
	return oldFlags;
}

int AdlibDriver::snd_clearFlag(va_list &list) {
	int oldFlags = _flags;
	_flags &= ~(va_arg(list, int));
	return oldFlags;
}

// timer callback

void AdlibDriver::callback() {
	lock();
	--_flagTrigger;
	if ((int8)_flagTrigger < 0)
		_flags &= ~8;
	callbackOutput();
	callbackProcess();

	_unkValue3 += _unkTableByte1;
	if ((int8)_unkValue3 < 0) {
		if (!(--_unkValue2)) {
			_unkValue2 = _unkValue1;
			++_unkValue4;
		}
	}
	unlock();
}

void AdlibDriver::callbackOutput() {
	while (_lastProcessed != _soundsPlaying) {
		uint8 *ptr = _soundData;

		ptr += READ_LE_UINT16(&ptr[_soundIdTable[_lastProcessed] << 1]);
		uint8 index = *ptr++;
		OutputState &table = _outputTables[index];

		int8 unk2 = *ptr++;
		if (unk2 >= table.unk2) {
			initTable(table);
			table.unk2 = unk2;
			table.dataptr = ptr;
			table.unk1 = 0xFF;
			table.unk4 = 0xFF;
			table.unk5 = 1;
			if (index != 9) {
				unkOutput2(index);
			}
		}

		++_lastProcessed;
		_lastProcessed &= 0x0F;
	}
}

void AdlibDriver::callbackProcess() {
	for (_curTable = 9; _curTable >= 0; --_curTable) {
		if (!_outputTables[_curTable].dataptr) {
			continue;
		}
	
		OutputState &table = _outputTables[_curTable];
		_unkOutputByte1 = _outputTable[_curTable];

		if (table.unk6) {
			table.unk1 = _unkTableByte1;
		}

		uint16 temp = table.unk4 + table.unk1;
		table.unk4 += table.unk1;
		if (temp > 0xFF) {
			if (--table.unk5) {
				if (table.unk5 == table.unk7)
					unkOutput1(table);
				if (table.unk5 == table.unk3 && _curTable != 9)
					unkOutput1(table);
			} else {
				int8 opcode = 0;
				while (table.dataptr) {
					uint16 command = READ_LE_UINT16(table.dataptr);
					table.dataptr += 2;
					if (command & 0x0080) {
						opcode = command & 0x7F;
						if (opcode > 0x4A)
							opcode = 0x4A;
						debugC(9, kDebugLevelSound, "Calling opcode '%s' (%d) (channel: %d)", _parserOpcodeTable[opcode].name, opcode, _curTable);
						opcode = (this->*(_parserOpcodeTable[opcode].function))(table.dataptr, table, (command & 0xFF00) >> 8);
						--opcode;
						if (opcode >= 0)
							break;
						continue;
					} else {
						opcode = 0;
						updateAndOutput1(command & 0xFF, table);
						updateAndOutput3(table);
						update1((command & 0xFF00) >> 8, table);
						if (!_continueFlag)
							continue;
						break;
					}
				}
				if (opcode)
					continue;
			}
		}

		if (table.callback1)
			(this->*(table.callback1))(table);
		if (table.callback2)
			(this->*(table.callback2))(table);
	}
}

// 

void AdlibDriver::resetAdlibState() {
	debugC(9, kDebugLevelSound, "resetAdlibState()");
	_unk6 = 0x1234;

	// Authorize the control of the waveforms
	writeOPL(0x01, 0x20);

	// Select FM music mode
	writeOPL(0x08, 0x00);

	// I would guess the main purpose of this is to turn off the rhythm,
	// thus allowing us to use 9 melodic voices instead of 6.
	writeOPL(0xBD, 0x00);

	int loop = 10;
	while (loop--) {
		if (loop != 9) {
			// Silence the channel
			writeOPL(0x40 + _outputTable[loop], 0x3F);
			writeOPL(0x43 + _outputTable[loop], 0x3F);
		}
		initTable(_outputTables[loop]);
	}
}

// Old calling style: output0x388(0xABCD)
// New calling style: writeOPL(0xAB, 0xCD)

void AdlibDriver::writeOPL(byte reg, byte val) {
	OPLWriteReg(_adlib, reg, val);
}

void AdlibDriver::initTable(OutputState &table) {
	debugC(9, kDebugLevelSound, "initTable(%d)", &table - _outputTables);
	memset(&table.dataptr, 0, sizeof(OutputState) - ((char*)&table.dataptr - (char*)&table));

	table.unk1 = 0xFF;
	table.unk2 = 0;
	// normally here are nullfuncs but we set 0 for now
	table.callback1 = 0;
	table.callback2 = 0;
	table.unk3 = 0x01;
}

void AdlibDriver::unkOutput1(OutputState &table) {
	debugC(9, kDebugLevelSound, "unkOutput1(%d)", &table - _outputTables);
	if (_curTable == 9)
		return;
	if (_unk4 && _curTable >= 6)
		return;

	// This means the "Key On" bit will always be 0
	table.unkOutputValue1 &= 0xDF;

	// Octave / F-Number / Key-On
	writeOPL(0xB0 + _curTable, table.unkOutputValue1);
}

void AdlibDriver::unkOutput2(uint8 num) {
	debugC(9, kDebugLevelSound, "unkOutput2(%d)", num);
	if (_unk4)
		if (num >= 6)
			return;

	uint8 value = _outputTable[num];

	// Clear the Attack Rate / Decay Rate for the channel
	writeOPL(0x60 + value, 0xFF);
	writeOPL(0x63 + value, 0xFF);

	// Clear the Sustain Level / Release Rate for the channel
	writeOPL(0x80 + value, 0xFF);
	writeOPL(0x83 + value, 0xFF);

	// Octave / F-Number / Key-On

	// The purpose of this seems to be to first clear everything, and then
	// set the octave.

	writeOPL(0xB0 + num, 0x00);
	writeOPL(0xB0 + num, 0x20);
}

uint16 AdlibDriver::updateUnk6Value() {
	_unk6 += 0x9248;
	uint16 lowBits = _unk6 & 7;
	_unk6 >>= 3;
	_unk6 |= lowBits << 13;
	return _unk6;
}

void AdlibDriver::update1(uint8 unk1, OutputState &state) {
	debugC(9, kDebugLevelSound, "update1(%d, %d)", unk1, &state - _outputTables);
	_continueFlag = unk1;
	if (state.unk11) {
		state.unk5 = unk1 + (updateUnk6Value() & state.unk11 & 0xFF);
		return;
	}
	uint8 value = unk1;
	if (state.unk12) {
		uint8 value2 = 0;
		uint8 add = value >> 3;
		int loops = state.unk12;
		while (loops--) {
			value2 += add;
		}
		state.unk7 = value2;
	}
	state.unk5 = value;
}

void AdlibDriver::updateAndOutput1(uint8 unk1, OutputState &state) {
	debugC(9, kDebugLevelSound, "updateAndOutput1(%d, %d)", unk1, &state - _outputTables);
	state.unk13 = unk1;
	uint8 unk2 = unk1 & 0xF0;
	unk2 += state.unk10;
	unk1 &= 0x0F;
	unk1 += state.unk14;

	if ((int8)unk1 >= 0x0C) {
		unk1 -= 0x0C;
		unk2 += 0x10;
	} else if ((int8)unk1 < 0) {
		unk1 += 0x0C;
		unk2 -= 0x10;
	}

	uint16 value = _unkTable[unk1] + state.unk15;

	unk2 >>= 2;
	unk2 &= 0x1C;
	unk2 |= (value & 0xFF00) >> 8;
	value = (value & 0xFF) | (unk2 << 8);

	if (state.unk16 != 0) {
		if (state.unk16 > 0) {
			const uint8 *table = _unkTables[(state.unk13 & 0x0F) + 2];
			value += table[state.unk16];
		} else {
			const uint8 *table = _unkTables[state.unk13 & 0x0F];
			value -= table[(state.unk16 ^ 0xFF) + 1];
		}
	}

	state.unkOutputValue1 = (state.unkOutputValue1 & 0x20) | ((value & 0xFF00) >> 8);
	state.unk17 = value & 0xFF;

	// Octave / F-Number / Key-On
	writeOPL(0xA0 + _curTable, state.unk17);
	writeOPL(0xB0 + _curTable, state.unkOutputValue1);
}

void AdlibDriver::updateAndOutput2(uint8 unk1, uint8 *dataptr, OutputState &state) {
	debugC(9, kDebugLevelSound, "updateAndOutput2(%d, 0x%X, %d)", unk1, dataptr, &state - _outputTables);
	// Amplitude Modulation / Vibrato / Envelope Generator Type /
	// Keyboard Scaling Rate / Modulator Frequency Multiple
	writeOPL(0x20 + unk1, *dataptr++);
	writeOPL(0x23 + unk1, *dataptr++);

	uint8 temp = *dataptr++;

	// Feedback / Algorithm
	writeOPL(0xC0 + _curTable, temp);

	state.unk23 = temp & 1;

	// Waveform Select
	writeOPL(0xE0 + unk1, *dataptr++);
	writeOPL(0xE3 + unk1, *dataptr++);

	state.unk24 = *dataptr++;
	state.unk25 = *dataptr++;

	// Level Key Scaling / Total Level
	writeOPL(0x40 + unk1, calculateLowByte1(state));
	writeOPL(0x43 + unk1, calculateLowByte2(state));

	// Attack Rate / Decay Rate
	writeOPL(0x60 + unk1, *dataptr++);
	writeOPL(0x63 + unk1, *dataptr++);

	// Sustain Level / Release Rate
	writeOPL(0x80 + unk1, *dataptr++);
	writeOPL(0x83 + unk1, *dataptr++);
}

void AdlibDriver::updateAndOutput3(OutputState &state) {
	debugC(9, kDebugLevelSound, "updateAndOutput3(%d)", &state - _outputTables);
	// This sets a bit in the "Octave" field
	state.unkOutputValue1 |= 0x20;

	// Octave / F-Number / Key-On
	writeOPL(0xB0 + _curTable, state.unkOutputValue1);

	int8 shift = 9 - state.unk33;
	uint16 temp = state.unk17 | (state.unkOutputValue1 << 8);
	state.unk37 = ((temp & 0x3FF) >> shift) & 0xFF;
	state.unk38 = state.unk36;
}

void AdlibDriver::adjustVolume(OutputState &state) {
	debugC(9, kDebugLevelSound, "adjustVolume(%d)", &state - _outputTables);
	uint8 lowByte = calculateLowByte2(state);

	// Level Key Scaling / Total Level
	writeOPL(0x43 + _outputTable[_curTable], lowByte);
	if (state.unk23) {
		lowByte = calculateLowByte1(state);

		// Level Key Scaling / Total Level
		writeOPL(0x40 + _outputTable[_curTable], lowByte);
	}
}

void AdlibDriver::stateCallback1_1(OutputState &state) {
	debugC(9, kDebugLevelSound, "Calling stateCallback1_1 (channel: %d)", _curTable);
	state.unk31 += state.unk29;
	if ((int8)state.unk31 >= 0)
		return;
	uint16 unk1 = ((state.unkOutputValue1 & 3) << 8) | state.unk17;
	uint16 unk2 = ((state.unkOutputValue1 & 0x20) << 8) | (state.unkOutputValue1 & 0x1C);
	int16 unk3 = (int16)state.unk30;

	if (unk3 >= 0) {
		unk1 += unk3;
		if (unk1 >= 734) {
			unk1 >>= 1;
			if (!(unk1 & 0x3FF))
				++unk1;
			unk2 += 4;
			unk2 &= 0xFF1C;
		}
	} else {
		unk1 += unk3;
		if (unk1 < 388) {
			unk1 <<= 1;
			if (!(unk1 & 0x3FF))
				--unk1;
			unk2 -= 4;
			unk2 &= 0xFF1C;
		}
	}
	unk1 &= 0x3FF;

	writeOPL(0xA0 + _curTable, unk1 & 0xFF);
	state.unk17 = unk1 & 0xFF;

	uint8 value = unk1 >> 8;
	value |= (unk2 >> 8) & 0xFF;
	value |= unk2 & 0xFF;

	writeOPL(0xB0 + _curTable, value);
	state.unkOutputValue1 = value;
}

void AdlibDriver::stateCallback1_2(OutputState &state) {
	debugC(9, kDebugLevelSound, "Calling stateCallback1_2 (channel: %d)", _curTable);
	if (state.unk38) {
		--state.unk38;
		return;
	}

	state.unk41 += state.unk32;
	if ((int8)state.unk41 < 0) {
		uint16 temp2 = state.unk37;
		if (!(--state.unk34)) {
			temp2 ^= 0xFFFF;
			++temp2;
			state.unk37 = temp2;
			state.unk34 = state.unk35;
		}

		uint16 temp3 = state.unk17 | (state.unkOutputValue1 << 8);
		temp2 += temp3 & 0x3FF;
		state.unk17 = temp2 & 0xFF;

		state.unkOutputValue1 = (state.unkOutputValue1 & 0xFC) | (temp3 >> 8);

		// Octave / F-Number / Key-On
		writeOPL(0xA0 + _curTable, state.unkOutputValue1);
		writeOPL(0xB0 + _curTable, state.unkOutputValue1);
	}
}

void AdlibDriver::stateCallback2_1(OutputState &state) {
	debugC(9, kDebugLevelSound, "Calling stateCallback2_1 (channel: %d)", _curTable);
	state.unk18 += state.unk19;
	if ((int8)state.unk18 < 0) {
		if ((--state.unk21) & 0x80) {
			state.unk21 = state.unk20;
		}
		writeOPL(state.unk22 + _unkOutputByte1, _soundData[state.offset + state.unk21]);
	}
}

uint8 AdlibDriver::calculateLowByte1(OutputState &state) {
	int8 value = state.unk24 & 0x3F;
	if (state.unk23) {
		value += state.unk26;
		value += state.unk27;
		value += state.unk28;
	}

	if (value > 0x3F) {
		value = 0x3F;
	} else if (value < 0)
		value = 0;

	return value | (state.unk24 & 0xC0);
}

uint8 AdlibDriver::calculateLowByte2(OutputState &state) {
	int8 value = state.unk25 & 0x3F;
	value += state.unk26;
	value += state.unk27;
	value += state.unk28;

	if (value > 0x3F) {
		value = 0x3F;
	} else if (value < 0)
		value = 0;

	return value | (state.unk25 & 0xC0);
}

// parser opcodes

int AdlibDriver::update_setRepeat(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.repeatCounter = value;
	return 0;
}

int AdlibDriver::update_checkRepeat(uint8 *&dataptr, OutputState &state, uint8 value) {
	++dataptr;
	if (--state.repeatCounter) {
		int16 add = READ_LE_UINT16(dataptr - 2);
		dataptr += add;
	}
	return 0;
}

int AdlibDriver::updateCallback3(uint8 *&dataptr, OutputState &state, uint8 value) {
	if (value >= 0xFF)
		return 0;

	uint16 add = value << 1;
	uint8 *ptr = _soundData + READ_LE_UINT16(_soundData + add);
	uint8 table = *ptr++;
	OutputState &state2 = _outputTables[table];
	int8 temp = *((int8*)ptr); ++ptr;
	if (temp >= (int8)state2.unk2) {
		_flagTrigger = 1;
		_flags |= 8;
		initTable(state2);
		state2.unk2 = temp;
		state2.dataptr = ptr;
		state2.unk1 = 0xFF;
		state2.unk4 = 0xFF;
		state2.unk5 = 1;
		unkOutput2(table);
	}
	return 0;
}

int AdlibDriver::updateCallback4(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk3 = value;
	return 0;
}

int AdlibDriver::update_jump(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;
	dataptr += add;
	return 0;
}

int AdlibDriver::update_jumpToSubroutine(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;
	state.dataptrStack[state.dataptrStackPos++] = dataptr;
	dataptr += add;
	return 0;
}

int AdlibDriver::update_returnFromSubroutine(uint8 *&dataptr, OutputState &state, uint8 value) {
	dataptr = state.dataptrStack[--state.dataptrStackPos];
	return 0;
}

int AdlibDriver::updateCallback8(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk10 = value;
	return 0;
}

int AdlibDriver::updateCallback9(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk2 = 0;
	if (_curTable != 9) {
		unkOutput1(state);
	}
	dataptr = 0;
	return 2;
}

int AdlibDriver::updateCallback10(uint8 *&dataptr, OutputState &state, uint8 value) {
	update1(value, state);
	unkOutput1(state);
	return (_continueFlag != 0);
}

int AdlibDriver::update_writeAdlib(uint8 *&dataptr, OutputState &state, uint8 value) {
	writeOPL(value, *dataptr++);
	return 0;
}

int AdlibDriver::updateCallback12(uint8 *&dataptr, OutputState &state, uint8 value) {
	updateAndOutput1(value, state);
	value = *dataptr++;
	update1(value, state);
	return (_continueFlag != 0);
}

int AdlibDriver::updateCallback13(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk14 = value;
	return 0;
}

int AdlibDriver::updateCallback14(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk18 = value;
	state.unk19 = value;
	state.unk20 = state.unk21 = *dataptr++;
	state.unk22 = *dataptr++;
	state.offset = READ_LE_UINT16(dataptr); dataptr += 2;
	state.callback2 = &AdlibDriver::stateCallback2_1;
	return 0;
}

int AdlibDriver::updateCallback15(uint8 *&dataptr, OutputState &state, uint8 value) {
	OutputState &state2 = _outputTables[value];
	state2.unk5 = 0;
	state2.unk2 = 0;
	state2.dataptr = 0;
	return 0;
}

int AdlibDriver::updateCallback16(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint8 *ptr = _soundData;
	ptr += READ_LE_UINT16(&_soundData[value << 1]);
	OutputState &state2 = _outputTables[*ptr];
	if (!state2.dataptr) {
		return 0;
	}
	dataptr -= 2;
	return 2;
}

int AdlibDriver::updateCallback17(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint8 *ptr = _soundData;
	ptr += READ_LE_UINT16(_soundData + (value << 1) + 0x1F4);
	updateAndOutput2(_unkOutputByte1, ptr, state);
	return 0;
}

int AdlibDriver::updateCallback18(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk29 = value;
	state.unk30 = READ_BE_UINT16(dataptr); dataptr += 2;
	state.callback1 = &AdlibDriver::stateCallback1_1;
	state.unk31 = -1;
	return 0;
}

int AdlibDriver::updateCallback19(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	state.callback1 = 0;
	state.unk30 = 0;
	return 0;
}

int AdlibDriver::updateCallback20(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk15 = value;
	return 0;
}

int AdlibDriver::updateCallback21(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk32 = value;
	state.unk33 = *dataptr++;
	uint8 temp = *dataptr++;
	state.unk34 = temp + 1;
	state.unk35 = temp << 1;
	state.unk36 = *dataptr++;
	state.callback1 = &AdlibDriver::stateCallback1_2;
	return 0;
}

int AdlibDriver::updateCallback22(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk2 = value;
	return 0;
}

int AdlibDriver::updateCallback23(uint8 *&dataptr, OutputState &state, uint8 value) {
	value >>= 1;
	_unkValue1 = _unkValue2 = value;
	_unkValue3 = 0xFF;
	_unkValue4 = _unkValue5 = 0;
	return 0;
}

int AdlibDriver::updateCallback24(uint8 *&dataptr, OutputState &state, uint8 value) {
	if (_unkValue5) {
		if (_unkValue4 & value) {
			_unkValue5 = 0;
			return 0;
		}
	}

	if (!(value & _unkValue4)) {
		++_unkValue5;
	}

	dataptr -= 2;
	state.unk5 = 1;
	return 2;
}

int AdlibDriver::updateCallback25(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk26 = value;
	adjustVolume(state);
	return 0;
}

int AdlibDriver::updateCallback26(uint8 *&dataptr, OutputState &state, uint8 value) {
	update1(value, state);
	return (_continueFlag != 0);
}

int AdlibDriver::updateCallback27(uint8 *&dataptr, OutputState &state, uint8 value) {
	update1(value, state);
	updateAndOutput3(state);
	return (_continueFlag != 0);
}

int AdlibDriver::updateCallback28(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk12 = value & 7;
	return 0;
}

int AdlibDriver::updateCallback29(uint8 *&dataptr, OutputState &state, uint8 value) {
	_unkTableByte1 = value;
	return 0;
}

int AdlibDriver::updateCallback30(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	state.callback2 = 0;
	return 0;
}

int AdlibDriver::updateCallback31(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk1 = value;
	return 0;
}

int AdlibDriver::updateCallback32(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk28 = value;
	return 0;
}

int AdlibDriver::updateCallback33(uint8 *&dataptr, OutputState &state, uint8 value) {
	int tableBackup = _curTable;

	_curTable = value;
	OutputState &state2 = _outputTables[value];
	state2.unk27 = *dataptr++;
	adjustVolume(state2);

	_curTable = tableBackup;
	return 0;
}

int AdlibDriver::updateCallback34(uint8 *&dataptr, OutputState &state, uint8 value) {
	int tableBackup = _curTable;

	_curTable = value;
	OutputState &state2 = _outputTables[value];
	state2.unk27 += *dataptr++;
	adjustVolume(state2);

	_curTable = tableBackup;
	return 0;
}

int AdlibDriver::update_setAMDepth(uint8 *&dataptr, OutputState &state, uint8 value) {
	if (value & 1)
		_unkOutputByte2 |= 0x80;
	else
		_unkOutputByte2 &= 0x7F;

	// The AM Depth bit is set or cleared, the others remain unchanged
	writeOPL(0xBD, _unkOutputByte2);
	return 0;
}

int AdlibDriver::update_setVibratoDepth(uint8 *&dataptr, OutputState &state, uint8 value) {
	if (value & 1)
		_unkOutputByte2 |= 0x40;
	else
		_unkOutputByte2 &= 0xBF;

	// The Vibrato Depth bit is set or cleared, the others remain unchanged
	writeOPL(0xBD, _unkOutputByte2);
	return 0;
}

int AdlibDriver::updateCallback37(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk26 += value;
	adjustVolume(state);
	return 0;
}

int AdlibDriver::updateCallback38(uint8 *&dataptr, OutputState &state, uint8 value) {
	int tableBackUp = _curTable;

	_curTable = value;
	OutputState &state2 = _outputTables[value];
	state2.unk5 = state2.unk2 = 0;
	state2.dataptr = 0;
	state2.unk27 = 0;

	if (value != 9) {
		uint8 outValue = _outputTable[value];

		// Feedback strength / Connection type
		writeOPL(0xC0 + _curTable, 0x00);

		// Key scaling level / Operator output level
		writeOPL(0x43 + outValue, 0x3F);

		// Sustain Level / Release Rate
		writeOPL(0x83 + outValue, 0xFF);

		// Key On / Octave / Frequency
		writeOPL(0xB0 + _curTable, 0x00);
	}

	_curTable = tableBackUp;
	return 0;
}

int AdlibDriver::updateCallback39(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint16 unk = *dataptr++;
	unk |= value << 8;
	unk &= updateUnk6Value();

	uint16 unk2 = ((state.unkOutputValue1 & 0x1F) << 8) | state.unk17;
	unk2 += unk;
	unk2 |= ((state.unkOutputValue1 & 0x20) << 8);

	// Frequency
	writeOPL(0xA0 + _curTable, unk2 & 0xFF);

	// Key On / Octave / Frequency
	writeOPL(0xB0 + _curTable, (unk2 & 0xFF00) >> 8);

	return 0;
}

int AdlibDriver::updateCallback40(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	state.callback1 = 0;
	return 0;
}

int AdlibDriver::updateCallback41(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk16 = value;
	int8 unk1 = 0, unk2 = 0;

	unk1 = state.unk13 & 0xF0;
	unk1 += state.unk10;

	unk2 = state.unk13 & 0x0F;
	unk2 += state.unk14;

	if (unk2 >= 12) {
		unk2 -= 12;
		unk1 += 16;
	} else if (unk2 < 0) {
		unk2 += 12;
		unk1 -= 16;
	}

	uint16 unk3 = _unkTable[unk2] + state.unk15;
	unk1 >>= 2; unk1 &= 0x1C;
	unk1 |= (unk3 >> 8);

	uint16 unk4 = (unk1 << 8) | (unk3 & 0xFF);
	if (state.unk16 >= 0) {
		const uint8 *ptr = _unkTables[(state.unk13 & 0x0F) + 2];
		unk4 += ptr[state.unk16];
	} else {
		const uint8 *ptr = _unkTables[state.unk13 & 0x0F];
		unk4 -= ptr[(state.unk16 ^ 0xFF) + 1];
	}

	unk2 = unk4 >> 8;
	unk1 = unk4 & 0xFF;

	state.unkOutputValue1 = unk2 | (state.unkOutputValue1 & 0x20);
	state.unk17 = unk1;

	// Frequency
	writeOPL(0xA0 + _curTable, unk1);

	// Key On / Octave / Frequency
	writeOPL(0xB0 + _curTable, state.unkOutputValue1);

	return 0;
}

int AdlibDriver::updateCallback42(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	state.unk1 = _unkTableByte1;
	return 0;
}

int AdlibDriver::updateCallback43(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	return 0;
}

int AdlibDriver::updateCallback44(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk11 = value;
	return 0;
}

int AdlibDriver::updateCallback45(uint8 *&dataptr, OutputState &state, uint8 value) {
	if (value & 0x80) {
		value += state.unk1;
		if ((int8)value >= (int8)state.unk1)
			value = 1;
	} else {
		value += state.unk1;
		if ((int8)value < 0)
			value = (uint8)-1;
	}
	state.unk1 = value;
	return 0;
}

int AdlibDriver::updateCallback46(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint8 entry = *dataptr++;
	_tablePtr1 = _unkTable2[entry++];
	_tablePtr2 = _unkTable2[entry];
	if (value == 2) {
		// Frequency
		writeOPL(0xA0, _tablePtr2[0]);
	}
	return 0;
}

int AdlibDriver::updateCallback47(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	return 0;
}

int AdlibDriver::updateCallback48(uint8 *&dataptr, OutputState &state, uint8 value) {
	int tableBackUp = _curTable;
	int outputByteBackUp = _unkOutputByte1;

	uint8 entry = value * 2;

	uint8 *ptr = _soundData;
	_curTable = 6;
	_unkOutputByte1 = _outputTable[6];

	ptr += READ_LE_UINT16(_soundData + entry + 0x1F4);

	_unkValue6 = *(ptr + 6);
	updateAndOutput2(_unkOutputByte1, ptr, state);

	entry = *ptr++ * 2;

	ptr = _soundData;
	_curTable = 7;
	_unkOutputByte1 = _outputTable[7];

	_unkValue7 = entry = *(ptr + 5);
	_unkValue8 = entry = *(ptr + 6);
	updateAndOutput2(_unkOutputByte1, ptr, state);

	entry = *ptr++ * 2;

	ptr = _soundData;
	_curTable = 8;
	_unkOutputByte1 = _outputTable[8];

	_unkValue9 = entry = *(ptr + 5);
	_unkValue10 = entry = *(ptr + 6);
	updateAndOutput2(_unkOutputByte1, ptr, state);

	// Octave / F-Number / Key-On for channels 6, 7 and 8

	_outputTables[6].unkOutputValue1 = *ptr++ & 0x2F;
	writeOPL(0xB6, _outputTables[6].unkOutputValue1);
	writeOPL(0xA6, *ptr++);

	_outputTables[7].unkOutputValue1 = *ptr++ & 0x2F;
	writeOPL(0xB7, _outputTables[7].unkOutputValue1);
	writeOPL(0xA7, *ptr++);

	_outputTables[8].unkOutputValue1 = *ptr++ & 0x2F;
	writeOPL(0xB8, _outputTables[8].unkOutputValue1);
	writeOPL(0xA8, *ptr++);

	_unk4 = 0x20;

	_unkOutputByte1 = outputByteBackUp;
	_curTable = tableBackUp;
	return 0;
}

int AdlibDriver::updateCallback49(uint8 *&dataptr, OutputState &state, uint8 value) {
	// Amplitude Modulation Depth / Vibrato Depth / Rhythm
	writeOPL(0xBD, (((value & 0x1F) ^ 0xFF) & _unk4) | 0x20);

	value |= _unk4;
	_unk4 = value;

	value |= _unkOutputByte2;
	value |= 0x20;

	// FIXME: This could probably be replaced with writeOPL(0xBD, value),
	//        but to make it easier to compare the output to DOSbox, write
	//        directly to the data port and do the probably unnecessary
	//        delay loop.

	OPLWrite(_adlib, 0x389, value);

	for (int i = 0; i < 23;  i++)
		OPLRead(_adlib, 0x388);

	return 0;
}

int AdlibDriver::updateCallback50(uint8 *&dataptr, OutputState &state, uint8 value) {
	--dataptr;
	_unk4 = 0;

	// Amplitude Modulation Depth / Vibrato Depth / Rhythm
	writeOPL(0xBD, value & _unkOutputByte2);

	return 0;
}

int AdlibDriver::updateCallback51(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint16 temp = (value << 8) | *dataptr++;

	if (value & 1) {
		uint8 val = temp & 0xFF;
		_unkValue12 = val;
		val += _unkValue7;
		val += _unkValue11;
		val += _unkValue12;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(val));
	}

	if (value & 2) {
		uint8 val = temp & 0xFF;
		_unkValue14 = val;
		val += _unkValue10;
		val += _unkValue13;
		val += _unkValue14;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(val));
	}

	if (value & 4) {
		uint8 val = temp & 0xFF;
		_unkValue15 = val;
		val += _unkValue9;
		val += _unkValue16;
		val += _unkValue15;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(val));
	}

	if (value & 8) {
		uint8 val = temp & 0xFF;
		_unkValue18 = val;
		val += _unkValue8;
		val += _unkValue17;
		val += _unkValue18;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(val));
	}

	if (value & 16) {
		uint8 val = temp & 0xFF;
		_unkValue20 = val;
		val += _unkValue6;
		val += _unkValue19;
		val += _unkValue20;

		// Channel 1: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(val));
	}

	return 0;
}

int AdlibDriver::updateCallback52(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint16 temp = (value << 8) | *dataptr++;

	if (value & 1) {
		uint8 val = temp & 0xFF;
		val += _unkValue7;
		val += _unkValue11;
		val += _unkValue12;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(val));
	}

	if (value & 2) {
		uint8 val = temp & 0xFF;
		val += _unkValue10;
		val += _unkValue13;
		val += _unkValue14;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(val));
	}

	if (value & 4) {
		uint8 val = temp & 0xFF;
		val += _unkValue9;
		val += _unkValue16;
		val += _unkValue15;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(val));
	}

	if (value & 8) {
		uint8 val = temp & 0xFF;
		val += _unkValue8;
		val += _unkValue17;
		val += _unkValue18;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(val));
	}

	if (value & 16) {
		uint8 val = temp & 0xFF;
		val += _unkValue6;
		val += _unkValue19;
		val += _unkValue20;

		// Channel 1: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(val));
	}

	return 0;
}

int AdlibDriver::updateCallback53(uint8 *&dataptr, OutputState &state, uint8 value) {
	uint16 temp = (value << 8) | *dataptr++;

	if (value & 1) {
		uint8 val = temp & 0xFF;
		_unkValue11 = val;
		val += _unkValue7;
		val += _unkValue12;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(val));
	}

	if (value & 2) {
		uint8 val = temp & 0xFF;
		_unkValue13 = val;
		val += _unkValue10;
		val += _unkValue14;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(val));
	}

	if (value & 4) {
		uint8 val = temp & 0xFF;
		_unkValue16 = val;
		val += _unkValue9;
		val += _unkValue15;

		// Channel 3: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(val));
	}

	if (value & 8) {
		uint8 val = temp & 0xFF;
		_unkValue17 = val;
		val += _unkValue8;
		val += _unkValue18;

		// Channel 2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(val));
	}

	if (value & 16) {
		uint8 val = temp & 0xFF;
		_unkValue19 = val;
		val += _unkValue6;
		val += _unkValue20;

		// Channel 1: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(val));
	}

	return 0;
}

int AdlibDriver::updateCallback54(uint8 *&dataptr, OutputState &state, uint8 value) {
	_unk5 = value;
	return 0;
}

int AdlibDriver::updateCallback55(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk6 = value;
	return 0;
}

int AdlibDriver::updateCallback56(uint8 *&dataptr, OutputState &state, uint8 value) {
	state.unk39 = value;
	state.unk40 = *dataptr++;
	return 0;
}

// static res

#define COMMAND(x) { &AdlibDriver::x, #x }
const AdlibDriver::OpcodeEntry AdlibDriver::_opcodeList[] = {
	COMMAND(snd_ret0x100),
	COMMAND(snd_ret0x1983),
	COMMAND(snd_initDriver),
	COMMAND(snd_deinitDriver),
	COMMAND(snd_setSoundData),
	COMMAND(snd_unkOpcode1),
	COMMAND(snd_startSong),
	COMMAND(snd_unkOpcode2),
	COMMAND(snd_unkOpcode3),
	COMMAND(snd_readByte),
	COMMAND(snd_writeByte),
	COMMAND(snd_setUnk5),
	COMMAND(snd_unkOpcode4),
	COMMAND(snd_dummy),
	COMMAND(snd_getNullvar4),
	COMMAND(snd_setNullvar3),
	COMMAND(snd_setFlag),
	COMMAND(snd_clearFlag)
};

const AdlibDriver::ParserOpcode AdlibDriver::_parserOpcodeTable[] = {
	// 0
	COMMAND(update_setRepeat),
	COMMAND(update_checkRepeat),
	COMMAND(updateCallback3),
	COMMAND(updateCallback4),

	// 4
	COMMAND(update_jump),
	COMMAND(update_jumpToSubroutine),
	COMMAND(update_returnFromSubroutine),
	COMMAND(updateCallback8),

	// 8
	COMMAND(updateCallback9),
	COMMAND(updateCallback10),
	COMMAND(update_writeAdlib),
	COMMAND(updateCallback12),

	// 12
	COMMAND(updateCallback13),
	COMMAND(updateCallback14),
	COMMAND(updateCallback15),
	COMMAND(updateCallback16),

	// 16
	COMMAND(updateCallback17),
	COMMAND(updateCallback18),
	COMMAND(updateCallback19),
	COMMAND(updateCallback20),

	// 20
	COMMAND(updateCallback9),
	COMMAND(updateCallback21),
	COMMAND(updateCallback9),
	COMMAND(updateCallback9),

	// 24
	COMMAND(updateCallback9),
	COMMAND(updateCallback9),
	COMMAND(updateCallback22),
	COMMAND(updateCallback9),

	// 28
	COMMAND(updateCallback23),
	COMMAND(updateCallback24),
	COMMAND(updateCallback25),
	COMMAND(updateCallback9),

	// 32
	COMMAND(updateCallback26),
	COMMAND(updateCallback27),
	COMMAND(updateCallback9),
	COMMAND(updateCallback9),

	// 36
	COMMAND(updateCallback28),
	COMMAND(updateCallback9),
	COMMAND(updateCallback29),
	COMMAND(updateCallback30),

	// 40
	COMMAND(updateCallback9),
	COMMAND(updateCallback31),
	COMMAND(updateCallback9),
	COMMAND(updateCallback32),

	// 44
	COMMAND(updateCallback33),
	COMMAND(updateCallback34),
	COMMAND(update_setAMDepth),
	COMMAND(update_setVibratoDepth),

	// 48
	COMMAND(updateCallback37),
	COMMAND(updateCallback9),
	COMMAND(updateCallback9),
	COMMAND(updateCallback38),

	// 52
	COMMAND(updateCallback9),
	COMMAND(updateCallback39),
	COMMAND(updateCallback40),
	COMMAND(updateCallback9),

	// 56
	COMMAND(updateCallback9),
	COMMAND(updateCallback41),
	COMMAND(updateCallback42),
	COMMAND(updateCallback43),

	// 60
	COMMAND(updateCallback44),
	COMMAND(updateCallback45),
	COMMAND(updateCallback9),
	COMMAND(updateCallback46),

	// 64
	COMMAND(updateCallback47),
	COMMAND(updateCallback48),
	COMMAND(updateCallback49),
	COMMAND(updateCallback50),

	// 68
	COMMAND(updateCallback51),
	COMMAND(updateCallback52),
	COMMAND(updateCallback53),
	COMMAND(updateCallback54),

	// 72
	COMMAND(updateCallback55),
	COMMAND(updateCallback56),
	COMMAND(updateCallback9)
};
#undef COMMAND

const int AdlibDriver::_opcodesEntries = ARRAYSIZE(AdlibDriver::_opcodeList);
const int AdlibDriver::_parserOpcodeTableSize = ARRAYSIZE(AdlibDriver::_parserOpcodeTable);

const uint8 AdlibDriver::_outputTable[] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11,
	0x12
};

const uint16 AdlibDriver::_unkTable[] = {
	0x0134, 0x0147, 0x015A, 0x016F, 0x0184, 0x019C, 0x01B4, 0x01CE, 0x01E9,
	0x0207, 0x0225, 0x0246
};

const uint8 *AdlibDriver::_unkTable2[] = {
	AdlibDriver::_unkTable2_1,
	AdlibDriver::_unkTable2_2,
	AdlibDriver::_unkTable2_1,
	AdlibDriver::_unkTable2_2,
	AdlibDriver::_unkTable2_3,
	AdlibDriver::_unkTable2_2
};

const uint8 AdlibDriver::_unkTable2_1[] = {
	0x50, 0x50, 0x4F, 0x4F, 0x4E, 0x4E, 0x4D, 0x4D,
	0x4C, 0x4C, 0x4B, 0x4B, 0x4A, 0x4A, 0x49, 0x49,
	0x48, 0x48, 0x47, 0x47, 0x46, 0x46, 0x45, 0x45,
	0x44, 0x44, 0x43, 0x43, 0x42, 0x42, 0x41, 0x41,
	0x40, 0x40, 0x3F, 0x3F, 0x3E, 0x3E, 0x3D, 0x3D,
	0x3C, 0x3C, 0x3B, 0x3B, 0x3A, 0x3A, 0x39, 0x39,
	0x38, 0x38, 0x37, 0x37, 0x36, 0x36, 0x35, 0x35,
	0x34, 0x34, 0x33, 0x33, 0x32, 0x32, 0x31, 0x31,
	0x30, 0x30, 0x2F, 0x2F, 0x2E, 0x2E, 0x2D, 0x2D,
	0x2C, 0x2C, 0x2B, 0x2B, 0x2A, 0x2A, 0x29, 0x29,
	0x28, 0x28, 0x27, 0x27, 0x26, 0x26, 0x25, 0x25,
	0x24, 0x24, 0x23, 0x23, 0x22, 0x22, 0x21, 0x21,
	0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1D, 0x1D,
	0x1C, 0x1C, 0x1B, 0x1B, 0x1A, 0x1A, 0x19, 0x19,
	0x18, 0x18, 0x17, 0x17, 0x16, 0x16, 0x15, 0x15,
	0x14, 0x14, 0x13, 0x13, 0x12, 0x12, 0x11, 0x11,
	0x10, 0x10
};

// no don't ask me WHY this table exsits!
const uint8 AdlibDriver::_unkTable2_2[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x6F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
};

const uint8 AdlibDriver::_unkTable2_3[] = {
	0x40, 0x40, 0x40, 0x3F, 0x3F, 0x3F, 0x3E, 0x3E,
	0x3E, 0x3D, 0x3D, 0x3D, 0x3C, 0x3C, 0x3C, 0x3B,
	0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x39, 0x39, 0x39,
	0x38, 0x38, 0x38, 0x37, 0x37, 0x37, 0x36, 0x36,
	0x36, 0x35, 0x35, 0x35, 0x34, 0x34, 0x34, 0x33,
	0x33, 0x33, 0x32, 0x32, 0x32, 0x31, 0x31, 0x31,
	0x30, 0x30, 0x30, 0x2F, 0x2F, 0x2F, 0x2E, 0x2E,
	0x2E, 0x2D, 0x2D, 0x2D, 0x2C, 0x2C, 0x2C, 0x2B,
	0x2B, 0x2B, 0x2A, 0x2A, 0x2A, 0x29, 0x29, 0x29,
	0x28, 0x28, 0x28, 0x27, 0x27, 0x27, 0x26, 0x26,
	0x26, 0x25, 0x25, 0x25, 0x24, 0x24, 0x24, 0x23,
	0x23, 0x23, 0x22, 0x22, 0x22, 0x21, 0x21, 0x21,
	0x20, 0x20, 0x20, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E,
	0x1E, 0x1D, 0x1D, 0x1D, 0x1C, 0x1C, 0x1C, 0x1B,
	0x1B, 0x1B, 0x1A, 0x1A, 0x1A, 0x19, 0x19, 0x19,
	0x18, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16,
	0x16, 0x15
};

// TODO: format this
const uint8 AdlibDriver::_unkTables[][32] = {
	{    0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x08,
	     0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,  0x10,
	     0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x19,
	     0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,  0x20,  0x21 },
	{    0x00,  0x01,  0x02,  0x03,  0x04,  0x06,  0x07,  0x09,
	     0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,  0x10,  0x11,
	     0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x1A,
	     0x1B,  0x1C,  0x1D,  0x1E,  0x1F,  0x20,  0x22,  0x24 },
	{    0x00,  0x01,  0x02,  0x03,  0x04,  0x06,  0x08,  0x09,
	     0x0A,  0x0C,  0x0D,  0x0E,  0x0F,  0x11,  0x12,  0x13,
	     0x14,  0x15,  0x16,  0x17,  0x19,  0x1A,  0x1C,  0x1D,
	     0x1E,  0x1F,  0x20,  0x21,  0x22,  0x24,  0x25,  0x26 },
	{    0x00,  0x01,  0x02,  0x03,  0x04,  0x06,  0x08,  0x0A,
	     0x0B,  0x0C,  0x0D,  0x0E,  0x0F,  0x11,  0x12,  0x13,
	     0x14,  0x15,  0x16,  0x17,  0x18,  0x1A,  0x1C,  0x1D,
	     0x1E,  0x1F,  0x20,  0x21,  0x23,  0x25,  0x27,  0x28 },
	{    0x00,  0x01,  0x02,  0x03,  0x04,  0x06,  0x08,  0x0A,
	     0x0B,  0x0C,  0x0D,  0x0E,  0x0F,  0x11,  0x13,  0x15,
	     0x16,  0x17,  0x18,  0x19,  0x1B,  0x1D,  0x1F,  0x20,
	     0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x28,  0x2A },
	{    0x00,  0x01,  0x02,  0x03,  0x05,  0x07,  0x09,  0x0B,
	     0x0C,  0x0D,  0x0E,  0x0F,  0x10,  0x11,  0x13,  0x15,
	     0x16,  0x17,  0x18,  0x19,  0x1B,  0x1D,  0x1F,  0x20,
	     0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x28,  0x2A },
	{    0x00,  0x01,  0x02,  0x03,  0x05,  0x07,  0x09,  0x0B,
	     0x0C,  0x0D,  0x0E,  0x0F,  0x10,  0x11,  0x13,  0x15,
	     0x16,  0x17,  0x18,  0x19,  0x1B,  0x1D,  0x1F,  0x20,
	     0x21,  0x22,  0x23,  0x25,  0x27,  0x29,  0x2B,  0x2D },
	{    0x00,  0x01,  0x02,  0x03,  0x05,  0x07,  0x09,  0x0B,
	     0x0C,  0x0D,  0x0E,  0x0F,  0x10,  0x11,  0x13,  0x15,
	     0x16,  0x17,  0x18,  0x1A,  0x1C,  0x1E,  0x21,  0x24,
	     0x25,  0x26,  0x27,  0x29,  0x2B,  0x2D,  0x2F,  0x30 },
	{    0x00,  0x01,  0x02,  0x04,  0x06,  0x08,  0x0A,  0x0C,
	     0x0D,  0x0E,  0x0F,  0x10,  0x11,  0x13,  0x15,  0x18,
	     0x19,  0x1A,  0x1C,  0x1D,  0x1F,  0x21,  0x23,  0x25,
	     0x26,  0x27,  0x29,  0x2B,  0x2D,  0x2F,  0x30,  0x32 },
	{    0x00,  0x01,  0x02,  0x04,  0x06,  0x08,  0x0A,  0x0D,
	     0x0E,  0x0F,  0x10,  0x11,  0x12,  0x14,  0x17,  0x1A,
	     0x19,  0x1A,  0x1C,  0x1E,  0x20,  0x22,  0x25,  0x28,
	     0x29,  0x2A,  0x2B,  0x2D,  0x2F,  0x31,  0x33,  0x35 },
	{    0x00,  0x01,  0x03,  0x05,  0x07,  0x09,  0x0B,  0x0E,
	     0x0F,  0x10,  0x12,  0x14,  0x16,  0x18,  0x1A,  0x1B,
	     0x1C,  0x1D,  0x1E,  0x20,  0x22,  0x24,  0x26,  0x29,
	     0x2A,  0x2C,  0x2E,  0x30,  0x32,  0x34,  0x36,  0x39 },
	{    0x00,  0x01,  0x03,  0x05,  0x07,  0x09,  0x0B,  0x0E,
	     0x0F,  0x10,  0x12,  0x14,  0x16,  0x19,  0x1B,  0x1E,
	     0x1F,  0x21,  0x23,  0x25,  0x27,  0x29,  0x2B,  0x2D,
	     0x2E,  0x2F,  0x31,  0x32,  0x34,  0x36,  0x39,  0x3C },
	{    0x00,  0x01,  0x03,  0x05,  0x07,  0x0A,  0x0C,  0x0F,
	     0x10,  0x11,  0x13,  0x15,  0x17,  0x19,  0x1B,  0x1E,
	     0x1F,  0x20,  0x22,  0x24,  0x26,  0x28,  0x2B,  0x2E,
	     0x2F,  0x30,  0x32,  0x34,  0x36,  0x39,  0x3C,  0x3F },
	{    0x00,  0x02,  0x04,  0x06,  0x08,  0x0B,  0x0D,  0x10,
	     0x11,  0x12,  0x14,  0x16,  0x18,  0x1B,  0x1E,  0x21,
	     0x22,  0x23,  0x25,  0x27,  0x29,  0x2C,  0x2F,  0x32,
	     0x33,  0x34,  0x36,  0x38,  0x3B,  0x34,  0x41,  0x44 },
	{    0x00,  0x02,  0x04,  0x06,  0x08,  0x0B,  0x0D,  0x11,
	     0x12,  0x13,  0x15,  0x17,  0x1A,  0x1D,  0x20,  0x23,
	     0x24,  0x25,  0x27,  0x29,  0x2C,  0x2F,  0x32,  0x35,
	     0x36,  0x37,  0x39,  0x3B,  0x3E,  0x41,  0x44,  0x47 }
};

#pragma mark -

SoundAdlibPC::SoundAdlibPC(Audio::Mixer *mixer, KyraEngine *engine)
	: Sound(engine, mixer), _driver(0), _trackEntries(), _soundDataPtr(0) {
	memset(_trackEntries, 0, sizeof(_trackEntries));
	_driver = new AdlibDriver(mixer);
	assert(_driver);

	_sfxPlayingSound = -1;
	_soundFileLoaded = "";
}

SoundAdlibPC::~SoundAdlibPC() {
	delete [] _soundDataPtr;
	delete _driver;
}

bool SoundAdlibPC::init() {
	_driver->callback(2);
	_driver->callback(16, int(4));
	return true;
}

void SoundAdlibPC::setVolume(int volume) {
}

int SoundAdlibPC::getVolume() {
	return 0;
}

void SoundAdlibPC::playMusic(const char *file) {
	loadSoundFile(file);
}

void SoundAdlibPC::stopMusic() {
	playSoundEffect(0);
}

void SoundAdlibPC::playTrack(uint8 track, bool looping) {
	// snd_stopSound();
	// snd_unk1();
	playSoundEffect(track);
}

void SoundAdlibPC::haltTrack() {
	playSoundEffect(0);
}

void SoundAdlibPC::startTrack() {
}

void SoundAdlibPC::loadSoundEffectFile(const char *file) {
	loadSoundFile(file);
}

void SoundAdlibPC::stopSoundEffect() {
}

void SoundAdlibPC::playSoundEffect(uint8 track) {
	uint8 soundId = _trackEntries[track];
	if ((int8)soundId == -1 || !_soundDataPtr)
		return;
	soundId &= 0xFF;
	while ((_driver->callback(16, 0) & 8)) {
		_engine->delay(10);
	}
	if (_sfxPlayingSound != -1) {
		_driver->callback(10, _sfxPlayingSound, int(1), int(_sfxSecondByteOfSong));
		_driver->callback(10, _sfxPlayingSound, int(3), int(_sfxFourthByteOfSong));
		_sfxPlayingSound = -1;
	}

	int firstByteOfSong = _driver->callback(9, soundId, int(0));

	if (firstByteOfSong != 9) {
		_sfxPlayingSound = soundId;
		_sfxSecondByteOfSong = _driver->callback(9, soundId, int(1));
		_sfxFourthByteOfSong = _driver->callback(9, soundId, int(3));

		int newVal = ((((-_sfxFourthByteOfSong) + 63) * 0xFF) >> 8) & 0xFF;
		newVal = -newVal + 63;
		_driver->callback(10, soundId, int(3), newVal);
		newVal = ((_sfxSecondByteOfSong * 0xFF) >> 8) & 0xFF;
		_driver->callback(10, soundId, int(1), newVal);
	}

	_driver->callback(6, soundId);
}

void SoundAdlibPC::beginFadeOut() {
}

bool SoundAdlibPC::fadeOut() {
	return false;
}

void SoundAdlibPC::loadSoundFile(const char *file) {
	if (_soundFileLoaded == file)
		return;

	uint8 *file_data = 0; uint32 file_size = 0;

	char filename[25];
	sprintf(filename, "%s.ADL", file);

	file_data = _engine->resource()->fileData(filename, &file_size);
	if (!file_data) {
		warning("Couldn't find music file: '%s'", filename);
		return;
	}

	// snd_stopSound();
	// snd_unk1();

	_driver->callback(8, int(-1));
	_soundDataPtr = 0;

	uint8 *p = file_data;
	memcpy(_trackEntries, p, 120*sizeof(uint8));
	p += 120;

	int soundDataSize = file_size - 120;

	_soundDataPtr = new uint8[soundDataSize];
	assert(_soundDataPtr);

	memcpy(_soundDataPtr, p, soundDataSize*sizeof(uint8));

	delete [] file_data;
	file_data = p = 0;
	file_size = 0;

	_driver->callback(4, _soundDataPtr);

	_soundFileLoaded = file;
}

} // end of namespace Kyra

