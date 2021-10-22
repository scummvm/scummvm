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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/gui_options.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"
#include "common/file.h"
#include "audio/mididrv.h"
#include "audio/musicplugin.h"

const byte MidiDriver::_mt32ToGm[128] = {
//	  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

const byte MidiDriver::_gmToMt32[128] = {
//	  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  5,   1,   2,   7,   3,   5,  16,  21,  22, 101, 101,  97, 104, 103, 102,  20, // 0x
	  8,   9,  11,  12,  14,  15,  87,  15,  59,  60,  61,  62,  67,  44,  79,  23, // 1x
	 64,  67,  66,  70,  68,  69,  28,  31,  52,  54,  55,  56,  49,  51,  57, 112, // 2x
	 48,  50,  45,  26,  34,  35,  45, 122,  89,  90,  94,  81,  92,  95,  24,  25, // 3x
	 80,  78,  79,  78,  84,  85,  86,  82,  74,  72,  76,  77, 110, 107, 108,  76, // 4x
	 47,  44, 111,  45,  44,  34,  44,  30,  32,  33,  88,  34,  35,  35,  38,  33, // 5x
	 41,  36, 100,  37,  40,  34,  43,  40,  63,  21,  99, 105, 103,  86,  55,  84, // 6x
	101, 103, 100, 120, 117, 113,  99, 128, 128, 128, 128, 124, 123, 128, 128, 128, // 7x
};

static const struct {
	uint32      type;
	const char *guio;
} GUIOMapping[] = {
	{ MT_PCSPK,		GUIO_MIDIPCSPK },
	{ MT_CMS,		GUIO_MIDICMS },
	{ MT_PCJR,		GUIO_MIDIPCJR },
	{ MT_ADLIB,		GUIO_MIDIADLIB },
	{ MT_C64,		GUIO_MIDIC64 },
	{ MT_AMIGA,	    GUIO_MIDIAMIGA },
	{ MT_APPLEIIGS,	GUIO_MIDIAPPLEIIGS },
	{ MT_TOWNS,		GUIO_MIDITOWNS },
	{ MT_PC98,		GUIO_MIDIPC98 },
	{ MT_SEGACD,	GUIO_MIDISEGACD },
	{ MT_GM,		GUIO_MIDIGM },
	{ MT_MT32,		GUIO_MIDIMT32 },
	{ 0,			0 },
};

Common::String musicType2GUIO(uint32 musicType) {
	Common::String res;

	for (int i = 0; GUIOMapping[i].guio; i++) {
		if (musicType == GUIOMapping[i].type || musicType == (uint32)-1)
			res += GUIOMapping[i].guio;
	}

	return res;
}

void MidiDriver::sendMT32Reset() {
	static const byte resetSysEx[] = { 0x41, 0x10, 0x16, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x00 };
	sysEx(resetSysEx, sizeof(resetSysEx));
	g_system->delayMillis(100);
}

void MidiDriver::sendGMReset() {
	static const byte gmResetSysEx[] = { 0x7E, 0x7F, 0x09, 0x01 };
	sysEx(gmResetSysEx, sizeof(gmResetSysEx));
	g_system->delayMillis(100);

	// Send a Roland GS reset. This will be ignored by pure GM units,
	// but will enable certain GS features on units that support them.
	// This is especially useful for some Yamaha units, which are put
	// in XG mode after a GM reset, which has some compatibility
	// problems with GS features like instrument banks and
	// GS-exclusive drum sounds.
	static const byte gsResetSysEx[] = { 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41 };
	sysEx(gsResetSysEx, sizeof(gsResetSysEx));
	g_system->delayMillis(100);
}

void MidiDriver_BASE::midiDumpInit() {
	g_system->displayMessageOnOSD(_("Starting MIDI dump"));
	_midiDumpCache.clear();
	_prevMillis = g_system->getMillis(true);
}

int MidiDriver_BASE::midiDumpVarLength(const uint32 &delta) {
	// MIDI file format has a very strange representation - "Variable Length Values"
	// we're using only *7* bits of each byte for the data
	// the MSB bit is 1 for all bytes, except the last one
	if (delta <= 127) {
		// "Variable Length Values" of 1 byte
		debugN("0x%02x", delta);
		_midiDumpCache.push_back(delta);
		return 1;
	} else {
		// "Variable Length Values" of 2 bytes
		// theoretically, "Variable Length Values" can have more than 2 bytes, but it won't happen in our use case
		byte msb = delta / 128;
		msb |= 0x80;
		byte lsb = delta % 128;
		debugN("0x%02x,0x%02x", msb, lsb);
		_midiDumpCache.push_back(msb);
		_midiDumpCache.push_back(lsb);
		return 2;
	}
}

void MidiDriver_BASE::midiDumpDelta() {
	uint32 millis = g_system->getMillis(true);
	uint32 delta = millis - _prevMillis;
	_prevMillis = millis;

	debugN("MIDI : delta(");
	int varLength = midiDumpVarLength(delta);
	if (varLength == 1)
		debugN("),\t ");
	else
		debugN("), ");
}

void MidiDriver_BASE::midiDumpDo(uint32 b) {
	const byte status = b & 0xff;
	const byte firstOp = (b >> 8) & 0xff;
	const byte secondOp = (b >> 16) & 0xff;

	midiDumpDelta();
	debugN("message(0x%02x 0x%02x", status, firstOp);

	_midiDumpCache.push_back(status);
	_midiDumpCache.push_back(firstOp);

	if (status < 0xc0 || status > 0xdf) {
		_midiDumpCache.push_back(secondOp);
		debug(" 0x%02x)", secondOp);
	} else
		debug(")");
}

void MidiDriver_BASE::midiDumpSysEx(const byte *msg, uint16 length) {
	midiDumpDelta();
	_midiDumpCache.push_back(0xf0);
	debugN("0xf0, length(");
	midiDumpVarLength(length + 1);		// +1 because of closing 0xf7
	debugN("), sysex[");
	for (int i = 0; i < length; i++) {
		debugN("0x%x, ", msg[i]);
		_midiDumpCache.push_back(msg[i]);
	}
	debug("0xf7]\t\t");
	_midiDumpCache.push_back(0xf7);
}


void MidiDriver_BASE::midiDumpFinish() {
	Common::DumpFile *midiDumpFile = new Common::DumpFile();
	midiDumpFile->open("dump.mid");
	midiDumpFile->write("MThd\0\0\0\x6\0\x1\0\x2", 12);		// standard MIDI file header, with two tracks
	midiDumpFile->write("\x1\xf4", 2);						// division - 500 ticks per beat, i.e. a quarter note. Each tick is 1ms
	midiDumpFile->write("MTrk", 4);							// start of first track - doesn't contain real data, it's just common practice to use two tracks
	midiDumpFile->writeUint32BE(4);							// first track size
	midiDumpFile->write("\0\xff\x2f\0", 4);			    	// meta event - end of track
	midiDumpFile->write("MTrk", 4);							// start of second track
	midiDumpFile->writeUint32BE(_midiDumpCache.size() + 4);	// track size (+4 because of the 'end of track' event)
	midiDumpFile->write(_midiDumpCache.data(), _midiDumpCache.size());
	midiDumpFile->write("\0\xff\x2f\0", 4);			    	// meta event - end of track
	midiDumpFile->finalize();
	midiDumpFile->close();
	const char msg[] = "Ending MIDI dump, created 'dump.mid'";
	g_system->displayMessageOnOSD(_(msg));		//TODO: why it doesn't appear?
	debug("%s", msg);
}

MidiDriver_BASE::MidiDriver_BASE() {
	_midiDumpEnable = ConfMan.getBool("dump_midi");
	if (_midiDumpEnable) {
		midiDumpInit();
	}
}

MidiDriver_BASE::~MidiDriver_BASE() {
	if (_midiDumpEnable && !_midiDumpCache.empty()) {
		midiDumpFinish();
	}
}

void MidiDriver_BASE::send(byte status, byte firstOp, byte secondOp) {
	send(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
}

void MidiDriver_BASE::send(int8 source, byte status, byte firstOp, byte secondOp) {
	send(source, status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
}

void MidiDriver_BASE::stopAllNotes(bool stopSustainedNotes) {
	for (int i = 0; i < 16; ++i) {
		send(0xB0 | i, MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		if (stopSustainedNotes)
			send(0xB0 | i, MIDI_CONTROLLER_SUSTAIN, 0); // Also send a sustain off event (bug #5524)
	}
}

void MidiDriver::midiDriverCommonSend(uint32 b) {
	if (_midiDumpEnable) {
		midiDumpDo(b);
	}
}

void MidiDriver::midiDriverCommonSysEx(const byte *msg, uint16 length) {
	if (_midiDumpEnable) {
		midiDumpSysEx(msg, length);
	}
}
