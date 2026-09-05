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

#ifndef FREESCAPE_CASTLE_C64_MUSIC_H
#define FREESCAPE_CASTLE_C64_MUSIC_H

#include "audio/sid.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "freescape/music.h"

namespace Freescape {

class Sound;
Sound *createCastleC64Sound(Audio::Mixer *mixer, const Common::Array<byte> &data);
void enableCastleC64Sound(Sound *sound, bool enabled);
void updateCastleC64GhostSound(Sound *sound, bool active);

class CastleC64MusicPlayer : public MusicPlayer, private Audio::AudioStream {
public:
	CastleC64MusicPlayer(Audio::Mixer *mixer);
	~CastleC64MusicPlayer() override;

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;
	int readBuffer(int16 *buffer, int numSamples) override;
	int getRate() const override;
	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }

private:
	enum {
		kChannelCount = 3,
		kMaxNote = 95
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
		uint16 baseFrequency;
		int16 frequencyOffset;
		byte vibratoStep;
		bool vibratoReverse;
		byte control;
		bool active;

		void reset(const byte *channelOrderList);
	};

	SID::SID *_sid;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	bool _musicActive;
	int _samplesUntilTick;
	int _sampleRemainder;
	uint32 _tick;
	ChannelState _channels[kChannelCount];

	void initSID();
	void destroySID();
	void sidWrite(int reg, byte data);
	void silenceAll();
	void setupSong();
	void onTimer();
	void loadNextPattern(int channel);
	byte readPatternByte(int channel);
	void parseCommands(int channel);
	void noteOn(int channel, byte note);
	void rest(int channel);
	void gateOff(int channel);
	void writeFrequency(int channel, uint16 frequency);
	uint16 noteToSIDFrequency(int note) const;
	void applyFrameEffects(int channel);
	byte sidControlForInstrument(byte instrument) const;
};

} // namespace Freescape

#endif
