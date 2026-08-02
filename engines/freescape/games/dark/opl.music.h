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

#ifndef FREESCAPE_DARK_OPL_MUSIC_H
#define FREESCAPE_DARK_OPL_MUSIC_H

#include "audio/fmopl.h"
#include "freescape/music.h"

namespace Freescape {

/**
 * OPL2/AdLib rendition of the Dark Side theme.
 *
 * Runs the C64 sequencer over the song data in dark.musicdata.h and voices it
 * on the OPL2: SID note numbers become F-number/block pairs, the SID ADSR
 * drives the chip's own envelope generator, and the noise waveform is played
 * on the rhythm-mode hi-hat.
 */
class DarkSideOPLMusicPlayer : public MusicPlayer {
public:
	DarkSideOPLMusicPlayer();
	~DarkSideOPLMusicPlayer();

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

private:
	enum {
		kChannelCount = 3,
		kMaxNote = 94
	};

	struct ChannelState {
		const byte *orderList;
		byte orderPos;
		const byte *pattern;
		uint16 patternPos;

		byte instrumentOffset;
		byte currentNote;
		byte transpose;
		uint16 frequencyFnum;
		byte frequencyBlock;

		byte durationReload;
		byte durationCounter;

		byte effectMode;
		byte effectParam;
		byte slideTarget;
		byte slideParam;
		byte arpeggioPos;
		byte arpeggioSequence[9];
		byte arpeggioSequenceLen;

		byte noteStepCommand;
		byte stepDownCounter;

		byte vibratoPhase;
		byte vibratoCounter;

		byte delayValue;
		byte delayCounter;

		byte waveform;
		byte instrumentFlags;
		bool specialAttack;
		bool attackDone;
		byte envCounter;
		bool gateOffDisabled;
		bool keyOn;
		uint16 pulseWidth;
		byte pulseWidthMod;
		byte pulseWidthDirection;
		byte modBaseLevel;
		byte carBaseLevel;
		byte modLevel;
		byte carLevel;
		bool rhythmVoice;

		void reset();
	};

	OPL::OPL *_opl;
	bool _musicActive;
	byte _speedDivider;
	byte _speedCounter;
	byte _rhythmReg;
	ChannelState _channels[kChannelCount];

	void onTimer();
	void setupSong();
	void silenceAll();
	void loadNextPattern(int channel);
	void buildEffectArpeggio(int channel);
	void loadCurrentFrequency(int channel);
	void finalizeChannel(int channel);
	void processChannel(int channel, bool newBeat);
	void parseCommands(int channel);
	void applyNote(int channel, byte note);
	void applyFrameEffects(int channel);
	bool applySpecialAttack(int channel);
	bool applyInstrumentVibrato(int channel);
	void applyEffectArpeggio(int channel);
	void applyTimedSlide(int channel);
	void programEnvelope(byte op, byte attack, byte decay, byte sustain, byte release);
	void updatePulseWidth(int channel, bool advance);
	void applyOperatorLevels(int channel);

	void setOPLInstrument(int channel, byte instrumentOffset);
	void noteOn(int channel);
	void noteOff(int channel);
	void writeFrequency(int channel, uint16 fnum, byte block);
	void setFrequency(int channel, uint16 fnum, byte block);
	void noteToFnumBlock(byte note, uint16 &fnum, byte &block) const;

	byte readPatternByte(int channel);
	byte clampNote(int note) const;
};

} // End of namespace Freescape

#endif
