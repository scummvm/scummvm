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

namespace Freescape {

// 3-channel SID music player for Dark Side C64.
// Implements the Wally Beben byte-stream sequencer from darkside.prg ($0901).
class DarkSideC64MusicPlayer {
public:
	DarkSideC64MusicPlayer();
	~DarkSideC64MusicPlayer();

	void startMusic();
	void stopMusic();
	bool isPlaying() const;
	void initSID();
	void destroySID();

private:
	SID::SID *_sid;

	void sidWrite(int reg, uint8 data);
	void onTimer();

	// Song setup
	void setupSong();
	void silenceAll();
	void loadNextPattern(int ch);
	void unpackArpeggio(int ch);

	// Per-tick processing
	void processChannel(int ch, bool newBeat);
	void parseCommands(int ch);
	void applyNote(int ch, uint8 note);
	void applyContinuousEffects(int ch);
	void applyVibrato(int ch);
	void applyArpeggio(int ch);
	void applyPortamento(int ch);
	void applyPWModulation(int ch);
	void applyEnvelope(int ch);

	uint8 readPatByte(int ch);

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
		uint8 noteActive;      // $1599
		uint8 curNote;         // $159C: note index (with transpose)
		uint8 transpose;       // $15C4

		uint8 freqLo;          // $15A6
		uint8 freqHi;          // $15A9
		uint8 pwLo;            // $15AC
		uint8 pwHi;            // $15AF

		uint8 durReload;       // $15B8
		uint8 durCounter;      // $15BB

		uint8 effectMode;      // $15BE: 0=none, 1=arp/vib1, 2=vib2, 0xDE=portaUp, 0xFE=portaDn
		uint8 effectParam;     // $15C1

		uint8 arpPattern;      // $15C7
		uint8 arpParam2;       // $15CA
		uint8 arpSeqPos;       // $15D9
		uint8 arpSeqData[20];  // $160C
		uint8 arpSeqLen;

		int16 portaDelta;      // $15CD/$15CE: per-tick freq delta
		uint16 portaTarget;    // target frequency for portamento

		uint8 vibPhase;        // $15DE
		uint8 vibCounter;      // $15E1

		uint8 pwDirection;     // $15D6: 0=up, 1=down

		uint8 delayCounter;    // $15E7

		uint8 envCounter;      // $15FC
		uint8 envTable;        // envelope table set (0 or 1)
		bool envSeqActive;     // flags bit 0

		bool sustainMode;      // flags bit 3

		uint8 waveform;        // current waveform ctrl byte (from instrument)

		void reset();
	};

	ChannelState _ch[3];
};

} // namespace Freescape

#endif // FREESCAPE_DARK_C64_MUSIC_H
