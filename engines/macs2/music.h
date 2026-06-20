/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MACS2_MUSIC_H
#define MACS2_MUSIC_H

#include "audio/mididrv.h"
#include "common/array.h"
#include "common/scummsys.h"

class MidiParser;

namespace Common {
class MemoryReadStream;
}

namespace OPL {
class OPL;
}

namespace Macs2 {

/**
 * Music player for the macs2 engine.
 *
 * Uses MidiParser_Macs2 for event parsing and drives OPL hardware directly,
 * preserving the original game's register-write behavior for authentic sound.
 * Implements MidiDriver_BASE to receive events from the MidiParser.
 */
class Music : public MidiDriver_BASE {
public:
	Music();
	~Music();

	void init();
	void deinit();

	void playSongData(const Common::Array<uint8> &data);
	void stopMusic();
	void setVolume(uint16 volume);
	bool isPlaybackReady() const { return _adlibPlaybackReady; }

	void readDataFromExecutable(Common::MemoryReadStream *fileStream);

	// MidiDriver_BASE interface
	void send(uint32 b) override;
	void metaEvent(byte type, const byte *data, uint16 length) override;

	// Debug state for ImGui visualization
	static constexpr int kDebugRingSize = 512;
	struct VoiceDebugState {
		uint8 note = 0xFF;
		uint8 channel = 0xFF;
		uint8 volume = 0;
		bool active = false;
	};
	struct DebugState {
		VoiceDebugState voices[9];
		uint8 masterVolume = 0;
		uint16 activeMusicSlot = 0;
		uint8 statusFlags = 0;
		uint32 nextEventTimer = 0;
		uint16 numOplChannels = 0;
		float regHistory[9][kDebugRingSize] = {};
		int ringPos = 0;
	};
	DebugState _debug;

private:
	void onTimer();
	void writeReg(byte reg, byte value);
	uint8 readReg(byte reg) const { return _regShadow[reg]; }

	void noteOn(byte channel, byte note, byte velocity);
	void noteOff(byte channel, byte note);
	void programChange(byte channel, byte program);
	void controlChange(byte channel, byte control, byte value);

	void loadInstrument(uint8 voice, uint8 program);
	void setFrequency(uint8 voice, uint8 note, uint8 pitchBend);
	void silenceAll();
	void updateDebugState();

	OPL::OPL *_opl;
	MidiParser *_parser;
	bool _playing;
	bool _adlibPlaybackReady = true;

	// OPL state
	byte _regShadow[256];
	uint8 _masterVolume;
	uint8 _numOplChannels;

	// Voice allocation (age-based, matching original)
	uint8 _voiceAge[9];
	uint8 _voiceMidiChannel[9];
	uint8 _voiceInstrument[9];
	uint8 _voiceNote[9];

	// Channel state
	uint8 _channelPrograms[16];
	uint8 _channelPitchBend[16];

	// Song data (kept alive for MidiParser which doesn't copy)
	Common::Array<uint8> _songData;

	// Instrument data from EXE (16 bytes per instrument, 11 used)
	Common::Array<uint8> _instrumentData;
	uint16 _instrumentDataOffset;

	// Lookup tables from EXE
	Common::Array<uint8> _opSlotTable;
	Common::Array<uint8> _opMap1;
	Common::Array<uint8> _opMap2;
	Common::Array<uint8> _freqTableLo;
	Common::Array<uint8> _freqTableHi;
	Common::Array<uint8> _percVolTable;
	Common::Array<uint8> _percOpMap;
	Common::Array<uint8> _percFreqChannel;

	void loadData(Common::MemoryReadStream *stream, int64 pos, uint16 size, void *target);
};

} // End of namespace Macs2

#endif
