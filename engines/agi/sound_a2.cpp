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

#include "audio/mixer.h"

#include "agi/agi.h"
#include "agi/sound_a2.h"

namespace Agi {

// SoundGenA2 plays Apple II sounds.
//
// Apple II AGI sounds are a series of monotonic notes. They sound similar to
// PC speaker versions, but they use a different resource format, and sound
// playback is a blocking operation.
//
// The sound resource's values are based on the number of 6502 CPU cycles
// consumed by AGI's play-note routine and the speed of the CPU. Playback was
// driven by the engine's own inner loops instead of a timer, so games are
// blocked until a sound is completed or interrupted by a key press.
//
// Common::PCSpeaker is used for sound generation. It produces significantly
// louder volumes than the other AGI sound generators, so I've lowered the
// mixer volume for consistency.

#define A2_MIXER_VOLUME 20

static void calculateNote(uint16 clickCount, uint16 delayCount, float &freq, uint32 &duration_usec);
static uint32 calculateDelayCycles(uint16 delayCount);
static uint32 calculateTotalCycles(uint32 delayCycles, uint16 delayCount, uint16 clickCount);

SoundGenA2::SoundGenA2(AgiBase *vm, Audio::Mixer *pMixer) :
	_isPlaying(false),
	SoundGen(vm, pMixer) {
	
	_mixer->playStream(Audio::Mixer::kMusicSoundType, _soundHandle, this, -1, A2_MIXER_VOLUME, 0, DisposeAfterUse::NO, true);
}

SoundGenA2::~SoundGenA2() {
	_mixer->stopHandle(*_soundHandle);
}

void SoundGenA2::play(int resnum) {
	Common::StackLock lock(_mutex);

	if (_vm->_game.sounds[resnum] == nullptr ||
		_vm->_game.sounds[resnum]->type() != AGI_SOUND_APPLE2) {
		error("Apple II sound %d not loaded", resnum);
	}

	_speaker.stop();

	// parse and enqueue all notes
	AgiSound *sound = _vm->_game.sounds[resnum];
	byte *data = sound->getData();
	uint32 dataLength = sound->getLength();
	for (uint32 i = 0; i + 4 < dataLength; i += 4) {
		uint16 clickCount = READ_LE_UINT16(&data[i]);
		uint16 delayCount = READ_LE_UINT16(&data[i + 2]);
		if (clickCount == 0xffff) {
			break;
		}
		
		float freq;
		uint32 duration_usec;
		calculateNote(clickCount, delayCount, freq, duration_usec);

		if (delayCount != 0) {
			_speaker.playQueue(Audio::PCSpeaker::kWaveFormSquare, freq, duration_usec);
		} else {
			_speaker.playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, duration_usec);
		}
	}
	
	_isPlaying = true;
}

void SoundGenA2::stop() {
	Common::StackLock lock(_mutex);

	_speaker.stop();
	_isPlaying = false;
}

int SoundGenA2::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	// if not playing then there are no samples
	if (!_isPlaying) {
		return 0;
	}

	// fill the buffer with PCSpeaker samples
	int result = _speaker.readBuffer(buffer, numSamples);

	// if PCSpeaker is no longer playing then sound is finished
	if (!_speaker.isPlaying()) {
		_isPlaying = false;
		_vm->_sound->soundIsFinished();
	}

	return result;
}

// Apple II note calculations
// 
// Each note is four bytes. Each byte controls how many iterations a loop makes
// in AGI's play-note routine. If the last two bytes are zero then the "click"
// instruction (LDA $C030) is skipped.
//
// The four bytes are conceptually two 16-bit little-endian values: the number
// of clicks to perform and the delay before each click.
//
// Calculating a note's frequency and duration requires calculating the number
// of CPU cycles spent delaying before each click and the number of CPU cycles
// spent in the play-note routine, and then applying the CPU speed.
//
// play-note routine from Black Cauldron:
//
// 6583:A5 12                LDA VALTYP+1   0012 ; a = delayCount[0]
// 6585:05 13                ORA GARFLG     0013 ; a |= delayCount[1]
// 6587:85 14                STA SUBFLG     0014 ; playNote = delayCount != 0
// ----------- begin counting cycles -----------
// 6589:A6 13                LDX GARFLG     0013 ; x = delayCount[1]
// 658B:A4 12                LDY VALTYP+1   0012 ; y = delayCount[0]
// 658D:88                   DEY                 ; x--
// 658E:D0 FD                BNE $658D      658D
// 6590:CA                   DEX                 ; y--
// 6591:10 FA                BPL $658D      658D
// 6593:A5 14                LDA SUBFLG     0014 ; a = playNote
// 6595:F0 03                BEQ $659A      659A ; skip click if !playNote
// 6597:AD 30 C0             LDA SPKR       C030 ; *click*
// 659A:C6 10                DEC DIMFLG     0010 ; clickCount[0]--
// 659C:D0 EB                BNE $6589      6589
// 659E:C6 11                DEC VALTYP     0011 ; clickCount[1]--
// 65A0:10 E7                BPL $6589      6589
// ----------- end counting cycles -------------
// 65A2:60                   RTS

static void calculateNote(uint16 clickCount, uint16 delayCount, float &freq, uint32 &duration_usec) {
	// calculate CPU cycles
	uint32 delayCycles = calculateDelayCycles(delayCount);
	uint32 totalCycles = calculateTotalCycles(delayCycles, delayCount, clickCount);

	// frequency is half the time spent delaying before a click,
	// because each click only toggles the speaker's state.
	// the average 6502 CPU cycle at 1.023 MHz is 0.98 microseconds.
	freq = 0.5f / (delayCycles * 0.00000098f);
	duration_usec = (uint32)(totalCycles * 0.98f);
}

static uint32 calculateDelayCycles(uint16 delayCount) {
	bool playNote = (delayCount != 0);
	uint32 delayHighByte = delayCount >> 8;

	uint32 cycles = 0;
	cycles += 3; // LDX
	cycles += 3; // LDY
	if (playNote) {
		cycles += (2 * delayCount); // DEY
		int bneNoBranchCount = (delayCount / 256) + 1;
		cycles += (3 * (delayCount - bneNoBranchCount)) + // BNE
			      (2 * bneNoBranchCount);
	} else {
		cycles += ((2 + 3) * 256) - 1; // DEY, BNE - 1 for last 2-cycle BNE
	}
	cycles += 2 * (delayHighByte + 1); // DEX
	cycles += (3 * delayHighByte) + 2; // BPL (3 cycles, 2 cycles on last iteration)
	cycles += 3;                       // LDA playNote
	cycles += playNote ? 2 : 3;        // BEQ (playNote)

	return cycles;
}

static uint32 calculateTotalCycles(uint32 delayCycles, uint16 delayCount, uint16 clickCount) {
	bool playNote = (delayCount != 0);
	uint32 clickHighByte = clickCount >> 8;

	// click count should never be zero, but if it were, then the low byte
	// would wrap around and produce 256 clicks while the high byte would
	// be correctly interpreted as zero.
	if (clickCount == 0) {
		clickCount = 256;
	}

	uint32 cycles = 0;
	cycles += delayCycles * clickCount; // every click incurs delayCycles
	if (playNote) {
		cycles += (4 * clickCount); // every click incurs LDA SPKR (the click!)
	}

	cycles += 5 * clickCount; // DEC
	int bneNoBranchCount = (clickCount / 256) + 1;
	cycles += (3 * (clickCount - bneNoBranchCount)) + // BNE
	          (2 * bneNoBranchCount);
	cycles += 5 * (clickHighByte + 1); // DEC
	cycles += (3 * clickHighByte) + 2; // BPL (3 cycles, 2 cycles on last iteration)

	return cycles;
}

} // End of namespace Agi
