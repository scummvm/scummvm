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

#ifndef MACS2_ADLIB_H
#define MACS2_ADLIB_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/stream.h"

namespace Common {
class MemoryReadStream;
class MemorySeekableReadWriteStream;
} // namespace Common

namespace OPL {
class OPL;
}

namespace Macs2 {

class StreamHandler : public Common::SeekableReadStream {
private:
	Common::MemorySeekableReadWriteStream *_stream = nullptr;
	int64 _pos = 0;

public:
	StreamHandler(Common::MemorySeekableReadWriteStream *s);
	StreamHandler(Common::Array<uint8> *data);

	bool eos() const override;
	uint32 read(void *dataPtr, uint32 dataSize) override;
	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offset, int whence) override;
	byte peekByte();
	byte peekByteAtOffset(int64 offset, int whence);
	uint16 peekWord();
};

class Adlib {
private:
	OPL::OPL *_opl;

	// --- Binary functions (segment 1000) ---
	void adlibWriteReg(byte registerIndex, byte value);      // 1000:2792
	uint8 adlibGetOperator(uint8 reg);                       // 1000:2779
	void adlibSetInstrument();                               // 1000:27e4
	void adlibSetFrequency(uint8 voiceIndex, StreamHandler *sh); // 1000:2839
	void adlibSetupChannel(uint16 voiceIndex, uint8 note, uint16 pitchBend); // 1000:294e
	void adlibProcessEvent(uint8 pitchBend, uint8 note, uint8 voiceIndex);   // 1000:2a80
	uint16 adlibTickHandler();                               // 1000:24fd (init)
	void adlibReadDeltaTime();                               // reads variable-length delta
	StreamHandler *adlibSeekStream(StreamHandler *inHandler, uint16 seekDelta);

	void OnTimer(); // ISR handler (1000:1a9f)

	// --- State variables (matching binary addresses) ---

	// [0036h] - initialized flag
	uint8 _isInitialized = 0;

	// [222Ch] - Voice age (9 entries, 0=active, >0=idle ticks)
	Common::Array<uint8> _voiceAge;

	// [2235h] - Current note per voice (9 entries, 0xFF=none)
	Common::Array<uint8> _voiceNote;

	// [223Eh] - Current MIDI event status word
	uint16 _currentEventStatus = 0;

	// [2240h] - Current MIDI event status high word
	uint16 _currentEventStatusHi = 0;

	// [2242h] - Playback ready flag (1=song ended/looped)
	uint8 _playbackReady = 1;

	// [2244h] - Song start pointer
	StreamHandler *_songStartPtr = nullptr;

	// [2248h] - Instrument data pointer
	StreamHandler *_instrumentDataPtr = nullptr;

	// [224Eh] - Timer frequency from song header
	uint16 _timerFrequency = 0;

	// [2250h] - Current playback position
	StreamHandler *_playbackPos = nullptr;

	// [2254h] - Delta time until next event (32-bit)
	uint32 _nextEventTimer = 0;

	// [2258h] - Status/control flags
	uint8 _statusFlags = 0;

	// [2259h] - Loop count (from controller 0x66)
	uint8 _loopCount = 0;

	// [225Ah] - Stream bytes consumed (low)
	uint16 _streamBytesConsumed = 0;

	// [225Ch] - Stream bytes consumed (high)
	uint16 _streamBytesConsumedHi = 0;

	// [225Eh] - Master volume (0-63, added to OPL volume calcs)
	uint8 _masterVolume = 0;

	// [225Fh] - Channel program/instrument (16 entries)
	Common::Array<uint8> _channelPrograms;

	// [226Fh] - Channel pitch bend (16 entries)
	Common::Array<uint8> _channelPitchBend;

	// [227Fh] - Voice MIDI channel assignment (9 entries, 0xFF=none)
	Common::Array<uint8> _voiceMidiChannel;

	// [2288h] - Voice instrument loaded (9 entries, 0xFF=none)
	Common::Array<uint8> _voiceInstrument;

	// [2291h] - Number of OPL channels (9=melodic, 6=percussion)
	uint16 _numOplChannels = 0;

	// [2296h] - Timer subdivision counter
	uint16 _timerSubdivCounter = 0;

	// [2298h] - Timer subdivision threshold
	uint16 _timerSubdivThreshold = 5;

	// [229Ah] - Timer tick flag
	bool _isTimerTick = false;

	// [229Bh] - Current MIDI running status byte
	uint8 _currentMidiStatus = 0;

	// [229Ch] - OPL register shadow (256 entries)
	Common::Array<uint8> _regShadow;

	// --- Lookup tables (loaded from executable) ---
	Common::Array<uint8> _opSlotTable;       // [69h] - operator slot mapping for adlibSetInstrument
	Common::Array<uint8> _opMap1;            // [8Dh] - OPL operator 1 per voice
	Common::Array<uint8> _opMap2;            // [96h] - OPL operator 2 per voice
	Common::Array<uint8> _freqTableLo;       // [9Fh] - frequency table low bytes
	Common::Array<uint8> _freqTableHi;       // [11Fh] - frequency table high bytes
	Common::Array<uint8> _percVolTable;      // [37h] - percussion volume lookup
	Common::Array<uint8> _percOpMap;         // [57h] - percussion operator indices
	Common::Array<uint8> _percFreqChannel;   // [5Ch] - percussion frequency channel

	// --- Song data management ---
	Common::Array<uint8> _activeSongData;
	StreamHandler *_activeSongStream = nullptr;

public:
	void Init();
	void Deinit();
	void PlaySongData(const Common::Array<uint8> &data);
	void StopMusic();
	void SetVolume(uint16 volume);
	bool isPlaybackReady() const { return _playbackReady != 0; }
	void ReadDataFromExecutable(Common::MemoryReadStream *fileStream);

private:
	void LoadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target);
};

} // namespace Macs2

#endif
