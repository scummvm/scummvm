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

#ifndef FREESCAPE_CASTLE_AY_MUSIC_H
#define FREESCAPE_CASTLE_AY_MUSIC_H

#include "audio/mixer.h"
#include "audio/softsynth/ay8912.h"
#include "freescape/music.h"

namespace Freescape {

/**
 * AY-3-8910 music player for Castle Master CPC.
 *
 * Adapts the Castle Master C64 title music to the CPC AY chip by reusing the
 * Castle order lists and pattern data shared with the AdLib rendition.
 */
class CastleAYMusicPlayer : public MusicPlayer, private Audio::AY8912Stream {
public:
	CastleAYMusicPlayer(Audio::Mixer *mixer);
	~CastleAYMusicPlayer() override;

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool endOfData() const override { return !_musicActive; }
	bool endOfStream() const override { return false; }

private:
	enum {
		kChannelCount = 3,
		kMaxNote = 94
	};

	enum ADSRPhase {
		kPhaseOff,
		kPhaseAttack,
		kPhaseDecay,
		kPhaseSustain,
		kPhaseRelease
	};

	struct ChannelState {
		const byte *orderList;
		uint16 orderPosition;
		byte patternIndex;
		uint16 patternDataOffset;
		uint16 patternOffset;
		uint16 delay;
		byte instrument;
		int8 transpose;
		byte currentNote;
		uint16 baseSIDFrequency;
		uint16 basePeriod;
		uint16 currentPeriod;
		uint16 adsrVolume;
		uint16 attackRate;
		uint16 decayRate;
		byte sustainLevel;
		uint16 releaseRate;
		int16 sidFrequencyOffset;
		byte vibratoStep;
		bool vibratoReverse;
		bool toneEnabled;
		bool noiseEnabled;
		bool active;
		ADSRPhase adsrPhase;

		void reset(const byte *channelOrderList);
	};

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	bool _musicActive;
	byte _mixerRegister;
	int _tickSampleCount;
	uint32 _tick;
	ChannelState _channels[kChannelCount];

	void onTimer();
	void setupSong();
	void silenceAll();
	void loadNextPattern(int channel);
	byte readPatternByte(int channel);
	void parseCommands(int channel);
	void noteOn(int channel, byte note);
	void noteOff(int channel);
	void writeChannelPeriod(int channel, uint16 period);
	void noteToPeriod(int note, uint16 &period) const;
	void applyFrameEffects(int channel);
	void triggerADSR(int channel, byte attackDecay, byte sustainRelease);
	void releaseADSR(int channel);
	void updateADSR(int channel);
	void setChannelOutput(int channel, bool toneEnabled, bool noiseEnabled);
	void writeNoisePeriod(uint16 period);
	byte sidControlForInstrument(byte instrument) const;
	byte instrumentVolumeScale(byte instrument) const;
};

} // namespace Freescape

#endif
