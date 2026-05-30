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

#ifndef FREESCAPE_DRILLER_OPL_MUSIC_H
#define FREESCAPE_DRILLER_OPL_MUSIC_H

#include "audio/fmopl.h"
#include "freescape/music.h"

namespace Freescape {

class DrillerOPLMusicPlayer : public MusicPlayer {
public:
	DrillerOPLMusicPlayer(int tuneIndex = 1);
	~DrillerOPLMusicPlayer() override;

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

private:
	enum {
		kChannelCount = 3,
		kMaxNote = 94
	};

	enum ContinuousEffectEntry {
		kVoiceDone,
		kFullEffectPath,
		kPortamentoOnlyPath
	};

	struct VoiceState {
		const uint8_t *trackDataPtr;
		uint8_t trackIndex;
		const uint8_t *patternDataPtr;
		uint8_t patternIndex;
		uint8_t instrumentIndex;
		int8_t delayCounter;
		uint8_t noteDuration;
		uint8_t gateMask;
		uint8_t currentNote;
		uint8_t currentNoteSlideTarget;
		uint8_t currentControl;
		uint8_t whatever0;
		uint8_t whatever1;
		uint8_t whatever2;
		uint8_t whatever3;
		uint8_t whatever4;
		uint8_t whatever2_vibDirToggle;
		uint16_t portaStepRaw;
		uint8_t something_else[3];
		uint8_t ctrl0;
		uint8_t arpTableIndex;
		uint8_t arpSpeedHiNibble;
		uint16_t stuff_freq_porta_vib;
		uint16_t stuff_freq_base;
		uint8_t stuff_arp_counter;
		uint8_t things_vib_state;
		uint16_t things_vib_depth;
		uint8_t things_vib_delay_reload;
		uint8_t things_vib_delay_ctr;
		uint8_t glideDownTimer;
		uint16_t baseSIDFrequency;
		uint16_t baseFrequencyFnum;
		uint8_t baseFrequencyBlock;
		uint16_t frequencyFnum;
		uint8_t frequencyBlock;
		bool keyOn;
		bool gateReleased;

		void reset();
	};

	OPL::OPL *_opl;
	bool _musicActive;
	uint8_t _targetTuneIndex;
	uint8_t _globalTempo;
	int8_t _globalTempoCounter;
	uint32 _tick;
	VoiceState _voiceState[kChannelCount];

	void onTimer();
	void setupTune(int tuneIndex);
	void resetVoices();
	void silenceAll();
	void playVoice(int channel);
	void applyNote(int channel, VoiceState &voice, const uint8_t *instA0, const uint8_t *instA1);
	ContinuousEffectEntry postNoteEffectSetup(VoiceState &voice, const uint8_t *instA0, const uint8_t *instA1);
	void applyContinuousEffects(int channel, VoiceState &voice, const uint8_t *instA0, const uint8_t *instA1, bool startAtPortamento);
	void applyHardRestart(int channel, VoiceState &voice, const uint8_t *instA1);
	void setOPLInstrument(int channel, VoiceState &voice);
	void applyPulseWidth(int channel, const VoiceState &voice);
	void noteOn(int channel, VoiceState &voice, uint8_t note);
	void noteOff(int channel);
	void setFrequency(int channel, VoiceState &voice, uint16 fnum, byte block);
	void writeFrequency(int channel, const VoiceState &voice, uint16 fnum, byte block);
	void writeNoteFrequency(int channel, VoiceState &voice, uint8_t note);
	void writeSIDFrequency(int channel, VoiceState &voice, uint16 sidFrequency);
	void noteToFnumBlock(int note, uint16 &fnum, byte &block) const;
};

} // namespace Freescape

#endif
