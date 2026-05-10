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

#ifndef FREESCAPE_ECLIPSE_C64_MUSIC_H
#define FREESCAPE_ECLIPSE_C64_MUSIC_H

#include "audio/sid.h"
#include "common/array.h"
#include "freescape/music.h"

namespace Freescape {

class EclipseC64MusicPlayer : public MusicPlayer {
public:
	EclipseC64MusicPlayer(const Common::Array<byte> &musicData);
	~EclipseC64MusicPlayer();

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

private:
	static const uint16 kLoadAddress = 0x0410;
	static const uint16 kOrderPointerTable = 0x1041;
	static const uint16 kInstrumentTable = 0x1047;
	static const uint16 kPatternPointerLowTable = 0x10A7;
	static const uint16 kPatternPointerHighTable = 0x10C6;
	static const uint16 kFrequencyHighTable = 0x0F5F;
	static const uint16 kFrequencyLowTable = 0x0FBE;
	static const uint16 kArpeggioIntervalTable = 0x148B;

	static const byte kChannelCount = 3;
	static const byte kInstrumentCount = 12;
	static const byte kPatternCount = 31;
	static const byte kMaxNote = 94;

	struct ChannelState {
		uint16 orderAddr;
		byte orderPos;

		uint16 patternAddr;
		byte patternOffset;

		byte instrumentOffset;
		byte currentNote;
		byte transpose;

		byte frequencyLow;
		byte frequencyHigh;
		byte pulseWidthLow;
		byte pulseWidthHigh;

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
		byte pulseWidthDirection;

		byte delayValue;
		byte delayCounter;

		byte waveform;
		byte instrumentFlags;
		bool gateOffDisabled;

		void reset();
	};

	SID::SID *_sid;
	Common::Array<byte> _musicData;
	byte _arpeggioIntervals[8];
	bool _musicActive;
	byte _speedDivider;
	byte _speedCounter;
	ChannelState _channels[kChannelCount];

	byte readByte(uint16 address) const;
	uint16 readWordLE(uint16 address) const;
	uint16 readPatternPointer(byte index) const;
	byte readInstrumentByte(byte instrumentOffset, byte field) const;
	byte readPatternByte(int channel);
	byte clampNote(byte note) const;

	void sidWrite(int reg, byte data);
	void onTimer();
	void silenceAll();
	void setupSong();
	void loadNextPattern(int channel);
	void buildEffectArpeggio(int channel);
	void loadCurrentFrequency(int channel);
	void finalizeChannel(int channel);
	void processChannel(int channel, bool newBeat);
	void parseCommands(int channel);
	void applyNote(int channel, byte note);
	void applyFrameEffects(int channel);
	bool applyInstrumentVibrato(int channel);
	void applyEffectArpeggio(int channel);
	void applyTimedSlide(int channel);
	void applyPulseWidthModulation(int channel);

	void initSID();
	void destroySID();
};

} // namespace Freescape

#endif
