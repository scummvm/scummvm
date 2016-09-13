/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_MUSIC_H
#define XEEN_MUSIC_H

#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/mutex.h"
#include "common/queue.h"

#define ADLIB_CHANNEL_COUNT 9

namespace OPL {
	class OPL;
}

namespace Xeen {

class Music;

typedef bool (Music::*CommandFn)(const byte *&srcP, byte nextByte);

struct RegisterValue {
	uint8 _regNum;
	uint8 _value;

	RegisterValue(int regNum, int value) {
		_regNum = regNum; _value = value;
	}
};

class Music {
	struct Channel {
		byte _outputLevel;
		byte _scalingValue;

		Channel() : _outputLevel(0), _scalingValue(0) {}
	};
private:
	static const byte OPERATOR1_INDEXES[ADLIB_CHANNEL_COUNT];
	static const byte OPERATOR2_INDEXES[ADLIB_CHANNEL_COUNT];
	static const CommandFn COMMAND_TABLE1[16];
	static const CommandFn COMMAND_TABLE2[16];
private:
	OPL::OPL *_opl;
	Common::Mutex _driverMutex;
	Common::Array<Channel> _channels;
	Common::Queue<RegisterValue> _queue;
	const byte *_effectsData;
	Common::Array<uint16> _effectsOffsets;
	const byte *_musicPtr1, *_musicPtr2;
	const byte *_dataPtr;
	bool _fieldF;
	bool _field1C;
	bool _field1E;
	uint _fieldFB[7];
	int _field109;
	int _field10B;
	byte _field10D[7];
	int _field114;
	int _field115;
	int _field116;
	int _field117;
	bool _lowMusicIgnored;
private:
	/**
	 * Initializes the state of the Adlib OPL driver
	 */
	void initialize();

	/**
	 * Loads effects data that was embedded in the music driver
	 */
	void loadEffectsData();

	/**
	 * Adds a register write to the pending queue that will be flushed
	 * out to the OPL on the next timer call
	 */
	void write(int reg, int val);

	/**
	 * Timer function for OPL
	 */
	void onTimer();

	/**
	 * Flushes any pending writes to the OPL
	 */
	void flush();

	/**
	 * Updates any playing music
	 */
	void update();

	/**
	 * Does a reset
	 */
	void reset();

	/**
	 * Resets all the output frequencies
	 */
	void resetFrequencies();

	/**
	 * Sets the frequency for an operator
	 */
	void setFrequency(byte operatorNum, uint frequency);

	/**
	 * Sets the output level for a channel
	 */
	void setOutputLevel(byte channelNum, uint level);

	/**
	 * Post-process
	 */
	void postProcess();

	/**
	 * Update command methods
	 */
	bool cmd1(const byte *&srcP, byte nextByte);
	bool cmd2(const byte *&srcP, byte nextByte);
	bool cmd3(const byte *&srcP, byte nextByte);
	bool cmd4(const byte *&srcP, byte nextByte);
	bool cmd5(const byte *&srcP, byte nextByte);
	bool cmd6(const byte *&srcP, byte nextByte);
	bool cmd7(const byte *&srcP, byte nextByte);
	bool cmd8(const byte *&srcP, byte nextByte);
	bool cmd9(const byte *&srcP, byte nextByte);
	bool cmd10(const byte *&srcP, byte nextByte);
	bool cmd11(const byte *&srcP, byte nextByte);
	bool cmd12(const byte *&srcP, byte nextByte);
	bool cmd13(const byte *&srcP, byte nextByte);
	bool cmd14(const byte *&srcP, byte nextByte);
	bool cmd15(const byte *&srcP, byte nextByte);
	bool cmd16(const byte *&srcP, byte nextByte);
	bool cmd17(const byte *&srcP, byte nextByte);
	bool cmd18(const byte *&srcP, byte nextByte);
	bool cmd19(const byte *&srcP, byte nextByte);
	bool cmd20(const byte *&srcP, byte nextByte);
	bool cmd21(const byte *&srcP, byte nextByte);
	bool cmd22(const byte *&srcP, byte nextByte);
	bool cmd23(const byte *&srcP, byte nextByte);
	bool cmd24(const byte *&srcP, byte nextByte);
protected:
	Audio::Mixer *_mixer;
public:
	Music(Audio::Mixer *mixer);
	~Music();

	/**
	 * Starts an effect playing
	 */
	void playEffect(uint effectId);
};

} // End of namespace Xeen

#endif /* XEEN_MUSIC_H */
