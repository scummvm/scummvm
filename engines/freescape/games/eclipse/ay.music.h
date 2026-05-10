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

#ifndef FREESCAPE_ECLIPSE_AY_MUSIC_H
#define FREESCAPE_ECLIPSE_AY_MUSIC_H

#include "audio/softsynth/ay8912.h"
#include "audio/mixer.h"
#include "freescape/music.h"

namespace Freescape {

/**
 * AY-3-8910 music player for Total Eclipse CPC.
 *
 * Ports the Wally Beben C64 SID music to the CPC's AY chip by:
 * - Reusing the same sequencer format (order lists, patterns, instruments)
 * - Converting SID frequencies to AY periods
 * - Replacing SID hardware ADSR with a software envelope
 * - Dropping SID-specific features (pulse width, filters)
 *
 */
class EclipseAYMusicPlayer : public MusicPlayer, private Audio::AY8912Stream {
public:
	EclipseAYMusicPlayer(Audio::Mixer *mixer);
	~EclipseAYMusicPlayer() override;

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

	// AudioStream overrides
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool endOfData() const override { return !_musicActive; }
	bool endOfStream() const override { return false; }

private:
	static const byte kChannelCount = 3;
	static const byte kMaxNote = 94;

	enum ADSRPhase {
		kPhaseOff,
		kPhaseAttack,
		kPhaseDecay,
		kPhaseSustain,
		kPhaseRelease
	};

	struct ChannelState {
		const byte *orderList;
		byte orderPos;

		uint16 patternDataOffset;
		uint16 patternOffset;

		byte instrumentOffset;
		byte currentNote;
		byte transpose;

		uint16 currentPeriod;

		byte durationReload;
		byte durationCounter;

		byte effectMode;
		byte effectParam;
		byte arpeggioTarget;
		byte arpeggioParam;
		byte arpeggioSequencePos;
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
		bool gateOffDisabled;

		ADSRPhase adsrPhase;
		uint16 adsrVolume; // 8.8 fixed point (0x0000 - 0x0F00)
		uint16 attackRate;
		uint16 decayRate;
		byte sustainLevel;
		uint16 releaseRate;

		void reset();
	};

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	bool _musicActive;
	byte _speedDivider;
	byte _speedCounter;
	byte _mixerRegister;
	int _tickSampleCount;
	ChannelState _channels[kChannelCount];
	byte _arpeggioIntervals[8];

	void onTimer();
	void setupSong();
	void silenceAll();
	void loadNextPattern(int channel);
	void buildEffectArpeggio(int channel);
	void loadCurrentPeriod(int channel);
	void finalizeChannel(int channel);
	void processChannel(int channel, bool newBeat);
	void parseCommands(int channel);
	void applyNote(int channel, byte note);
	void applyFrameEffects(int channel);
	bool applyInstrumentVibrato(int channel);
	void applyEffectArpeggio(int channel);
	void applyTimedSlide(int channel);

	void triggerADSR(int channel, byte ad, byte sr);
	void releaseADSR(int channel);
	void updateADSR(int channel);

	byte readPatternByte(int channel);
	byte clampNote(byte note) const;
	void writeChannelPeriod(int channel, uint16 period);
};

} // namespace Freescape

#endif
