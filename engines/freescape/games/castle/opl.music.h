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

#ifndef FREESCAPE_CASTLE_OPL_MUSIC_H
#define FREESCAPE_CASTLE_OPL_MUSIC_H

#include "audio/fmopl.h"
#include "freescape/music.h"

namespace Freescape {

/**
 * OPL2/AdLib music player for Castle Master DOS.
 *
 * Adapts the Castle Master C64 title music to the OPL2 FM chip by:
 * - Reusing compact note, rest, and instrument pattern events
 * - Converting SID note numbers to OPL F-number/block pairs
 * - Mapping the original voice changes to OPL FM instrument patches
 */
class CastleOPLMusicPlayer : public MusicPlayer {
public:
	CastleOPLMusicPlayer();
	~CastleOPLMusicPlayer() override;

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
		uint16 orderPosition;
		byte patternIndex;
		uint16 patternDataOffset;
		uint16 patternOffset;
		uint16 delay;
		byte instrument;
		int8 transpose;
		byte currentNote;
		uint16 baseSIDFrequency;
		int16 sidFrequencyOffset;
		uint16 baseFrequencyFnum;
		byte baseFrequencyBlock;
		uint16 frequencyFnum;
		byte frequencyBlock;
		byte vibratoStep;
		bool vibratoReverse;
		bool keyOn;
		bool gateReleased;

		void reset(const byte *channelOrderList);
	};

	OPL::OPL *_opl;
	bool _musicActive;
	uint32 _tick;
	ChannelState _channels[kChannelCount];

	void onTimer();
	void setupSong();
	void silenceAll();
	void loadNextPattern(int channel);
	byte readPatternByte(int channel);
	void parseCommands(int channel);
	void setOPLInstrument(int channel, byte instrument);
	void noteOn(int channel, byte note);
	void noteOff(int channel);
	void gateOff(int channel);
	void applyFrameEffects(int channel);
	void setFrequency(int channel, uint16 fnum, byte block);
	void writeFrequency(int channel, uint16 fnum, byte block);
	void noteToFnumBlock(int note, uint16 &fnum, byte &block) const;
};

} // namespace Freescape

#endif
