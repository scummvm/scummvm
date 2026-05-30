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

	void adlibReadDeltaTime();

	void OnTimer();

	StreamHandler *adlibSeekStream(StreamHandler *inHandler, uint16 seekDelta);

	void adlibPlaySong(StreamHandler *song);

	uint16 adlibTickHandler();

	uint16 adlibStopMusic();

	uint8 adlibGetOperator(uint8 arg1);

	// Writes a value to the target register
	void adlibWriteReg(byte registerIndex, byte value);
	void adlibWriteRegr(byte value, byte registerIndex) {
		adlibWriteReg(registerIndex, value);
	}

	void adlibSetInstrument();

	void adlibSetFrequency(uint8 bpp0A, StreamHandler *sh);

	void adlibSetupChannel(uint16 bppA, uint8 bpp8, uint16 bpp6);

	// [0036h] - Set to 1 after successful initialization
	uint8 _isInitialized = 0;

	// Array accessed as [di + 37h] - volume lookup table
	Common::Array<uint8> gArray37;

	// Array accessed as [di + 8dh] - OPL operator 2 mapping per voice
	Common::Array<uint8> gArray8d;

	// Array accesed as [di + 57] - percussion operator mapping
	Common::Array<uint8> gArray57;

	// Array accessed as [di + 5C]
	Common::Array<uint8> gArray5C;

	// Array accessed as [di + 69]
	// Used for looking up data in 27E4, loaded from the executable
	Common::Array<uint8> gArray69;

	// Array accesed as [di + 96] - OPL operator 1 mapping per voice
	Common::Array<uint8> gArray96;

	// Array accessed as [di+9Fh] - from 0x0001B69F
	Common::Array<uint8> gArray9F;

	// Array accessed as [di+11Fh] - from 0x0001B71F
	Common::Array<uint8> gArray11F;

	// [223Eh] - Current MIDI status byte (high byte of running status)
	uint16 _currentEventStatus = 0;

	// Memory being pointed to by [2244] and [2246] - song start pointer
	StreamHandler *shMem2244 = nullptr;

	// Memory being pointed to by [2248] and [224A] - instrument data pointer
	StreamHandler *shMem2248 = nullptr;

	// [2240h] - Current MIDI status high (always 0 for single-byte status)
	uint16 _currentEventStatusHi = 0;

	// [2242h] - Playback ready flag: 1 when song has looped/ended
	uint8 _playbackReady = 1;

	// [224Eh] - Timer frequency parameter from song header
	uint16 _timerFrequency = 0;

	// Memory being pointed to by [2250] and [2252] - current playback position
	StreamHandler *shMem2250 = nullptr;

	// [2254h] and [2256h] - delta time until next event (32-bit)
	uint32 _nextEventTimer = 0;

	// [2258h] - Status/control flags byte
	// bit0: unused, bit1: stop requested, bit4: busy/processing
	// bit5: loop event pending, bit6: stop acknowledged
	uint8 _statusFlags = 0;

	// [2259h] - Loop count from controller 0x66, reset on song end
	uint8 _loopCount = 0;

	// [225Eh] - Master volume offset (added to OPL volume calculations)
	uint8 _masterVolume = 0;

	// [226Fh] - Pitch bend value per MIDI channel (16 entries, init 0)
	Common::Array<uint8> _channelPitchBend;

	// [2291h] - Number of active OPL channels (9=melodic, 6=percussion)
	uint16 _numOplChannels = 0;

	// [2296h] - Timer subdivision counter (counts up to _timerSubdivThreshold)
	uint16 _timerSubdivCounter = 0;

	// [2298h] - Timer subdivision threshold (ISR fires music event every N ticks)
	uint16 _timerSubdivThreshold = 5;

	// [225Ah] - Stream bytes consumed counter (low word)
	uint16 _streamBytesConsumed = 0;

	// [225Ch] - Stream bytes consumed counter (high word)
	uint16 _streamBytesConsumedHi = 0;

	// [222Ch] - Voice allocation age (9 entries, 0=active, >0=idle ticks since release)
	Common::Array<uint8> _voiceAge;

	// [225Fh] - MIDI program/instrument assigned per channel (16 entries, init 0)
	Common::Array<uint8> _channelPrograms;

	// [227Fh] - Which MIDI channel each OPL voice is assigned to (9 entries, init 0xFF=none)
	Common::Array<uint8> _voiceMidiChannel;

	// [2288h] - Current instrument loaded per OPL voice (9 entries, init 0xFF=none)
	Common::Array<uint8> _voiceInstrument;

	// [229Ch] - Shadow of OPL register values written
	Common::Array<uint8> gArray229C;

	// [2235h] - Current note playing per OPL voice (9 entries, init 0xFF=none)
	Common::Array<uint8> _voiceNote;

	// [229Ah] - Set to true when timer subdivision fires (ISR tick flag)
	bool _isTimerTick = false;

	// [229Bh] - Current MIDI running status byte for event parsing
	uint8 _currentMidiStatus = 0;

	// fn0017_2A80: 0017:2A80
	void adlibProcessEvent(uint8 bpp6, uint8 bpp8, uint8 reg_base);

public:
	void Init();

	void Deinit();

	void SetSong(Macs2::StreamHandler *sh);
	void PlaySongData(const Common::Array<uint8> &data);
	void StopMusic();
	bool isPlaybackReady() const { return _playbackReady != 0; }

	void SetVolume(uint16 volume);

	void ReadDataFromExecutable(Common::MemoryReadStream *fileStream);

	void LoadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target);

private:
	Common::Array<uint8> _activeSongData;
	StreamHandler *_activeSongStream = nullptr;
};

} // namespace Macs2

#endif
