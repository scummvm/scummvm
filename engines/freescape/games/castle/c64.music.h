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
#include "freescape/music.h"

namespace Freescape {

class CastleC64MusicPlayer : public MusicPlayer {
public:
	CastleC64MusicPlayer();
	~CastleC64MusicPlayer() override;

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

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
	bool _musicActive;
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
