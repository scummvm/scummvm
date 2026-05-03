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

#ifndef FREESCAPE_DARK_C64_MUSIC_H
#define FREESCAPE_DARK_C64_MUSIC_H

#include "audio/sid.h"
#include "freescape/music.h"

namespace Freescape {

// 3-channel SID music player for Dark Side C64.
// Implements the Wally Beben byte-stream sequencer from darkside.prg ($0901).
class DarkSideC64MusicPlayer : public MusicPlayer {
public:
	DarkSideC64MusicPlayer();
	~DarkSideC64MusicPlayer();

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

private:
	SID::SID *_sid;

	void sidWrite(int reg, uint8 data);
	void onTimer();

	// Song setup
	void setupSong();
	void silenceAll();
	void loadNextPattern(int ch);
	void buildEffectArpeggio(int ch);
	void loadCurrentFrequency(int ch);
	void finalizeChannel(int ch);

	// Per-tick processing
	void processChannel(int ch, bool newBeat);
	void parseCommands(int ch);
	void applyNote(int ch, uint8 note);
	void applyFrameEffects(int ch);
	bool applySpecialAttack(int ch);
	bool applyEnvelopeSequence(int ch);
	bool applyInstrumentVibrato(int ch);
	void applyEffectArpeggio(int ch);
	void applyTimedSlide(int ch);
	void applyPWModulation(int ch);

	uint8 readPatByte(int ch);

	void initSID();
	void destroySID();

	// Global state
	bool _musicActive;
	uint8 _speedDiv;       // $15A1: speed divider
	uint8 _speedCounter;   // $15A2: frames until next beat

	// Per-channel state
	struct ChannelState {
		const uint8 *orderData;
		uint8 orderPos;        // $15A3

		const uint8 *patData;
		int patOffset;

		uint8 instIdx;         // $15F6: instrument# * 8
		uint8 curNote;         // $159C: note index (with transpose)
		uint8 transpose;       // $15C4

		uint8 freqLo;          // $15A6
		uint8 freqHi;          // $15A9
		uint8 pwLo;            // $15AC
		uint8 pwHi;            // $15AF

		uint8 durReload;       // $15B8
		uint8 durCounter;      // $15BB

		uint8 effectMode;      // $15BE: mode 2 keeps effectParam alive across note loads
		uint8 effectParam;     // $15C1

		uint8 arpPattern;      // $15C7
		uint8 arpParam2;       // $15CA
		uint8 arpSeqPos;       // $15D9
		uint8 arpSeqData[20];  // $160C
		uint8 arpSeqLen;

		uint8 noteStepCommand; // $15D0: self-modified INC/DEC opcode for beat step
		uint8 stepDownCounter; // $15D3: short downward slide counter

		uint8 vibPhase;        // $15DE
		uint8 vibCounter;      // $15E1

		uint8 pwDirection;     // $15D6: 0=up, 1=down

		uint8 delayValue;      // $15E4
		uint8 delayCounter;    // $15E7

		uint8 envCounter;      // $15FC
		bool gateOffDisabled;  // $1606: set when SR release nibble is $F
		bool gateModeControl;  // $1600: flags bit 7
		bool specialAttack;    // flags bit 3
		bool attackDone;       // $1603
		uint8 waveform;        // current instrument ctrl byte
		uint8 instFlags;       // current instrument flags byte

		void reset();
	};

	ChannelState _ch[3];
};

} // namespace Freescape

#endif // FREESCAPE_DARK_C64_MUSIC_H
