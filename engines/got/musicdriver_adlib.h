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

#ifndef GOT_MUSICDRIVER_ADLIB_H
#define GOT_MUSICDRIVER_ADLIB_H

#include "got/musicdriver.h"

#include "common/scummsys.h"
#include "common/timer.h"
#include "audio/fmopl.h"

namespace Got {

class MusicDriver_Got_AdLib : public MusicDriver_Got {
public:
	/**
	 * The number of available channels on the OPL2 chip.
	 */
	static const uint8 OPL2_NUM_CHANNELS = 9;

	/**
	 * OPL operator base registers.
	 */
	static const uint8 OPL_REGISTER_BASE_FREQMULT_MISC = 0x20;
	static const uint8 OPL_REGISTER_BASE_LEVEL = 0x40;
	static const uint8 OPL_REGISTER_BASE_DECAY_ATTACK = 0x60;
	static const uint8 OPL_REGISTER_BASE_RELEASE_SUSTAIN = 0x80;
	static const uint8 OPL_REGISTER_BASE_WAVEFORMSELECT = 0xE0;

	/**
	 * OPL channel base registers.
	 */
	static const uint8 OPL_REGISTER_BASE_FNUMLOW = 0xA0;
	static const uint8 OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON = 0xB0;
	static const uint8 OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING = 0xC0;

	MusicDriver_Got_AdLib(uint8 timerFrequency);
	~MusicDriver_Got_AdLib();

	int open() override;
	void close() override;

	void syncSoundSettings() override;

	void send(uint16 b) override;
	void stopAllNotes() override;

	void setTimerFrequency(uint8 timerFrequency) override;

protected:
	/**
	 * Initializes the OPL registers to their default values.
	 */
	void initOpl();

	void recalculateVolumes();
	uint8 calculateVolume(uint8 channel, uint8 operatorNum);
	/**
	 * Determines if volume settings should be applied to the operator level.
	 * This depends on the type of the operator (carrier or modulator), which
	 * depends on the type of connection specified on the channel.
	 *
	 * @param oplChannel The OPL channel
	 * @param operatorNum The number of the operator (0-1)
	 * @return True if volume should be applied, false otherwise
	 */
	bool isVolumeApplicableToOperator(uint8 oplChannel, uint8 operatorNum);
	/**
	 * Determines the offset from a base register for the specified operator of
	 * the specified OPL channel.
	 * Add the offset to the base register to get the correct register for this
	 * operator and channel.
	 *
	 * @param oplChannel The OPL channel for which to determine the offset.
	 * @param operatorNum The operator for which to determine the offset (0-1).
	 * @return The offset to the base register for this operator.
	 */
	uint16 determineOperatorRegisterOffset(uint8 oplChannel, uint8 operatorNum);
	void writeRegister(uint8 oplRegister, uint8 value);

	// The OPL emulator / hardware interface.
	OPL::OPL *_opl;

	// Last written value to the operator 0 level register for each channel.
	uint8 _channelOp0LevelRegisterValues[OPL2_NUM_CHANNELS];
	// Last written value to the operator 1 level register for each channel.
	uint8 _channelOp1LevelRegisterValues[OPL2_NUM_CHANNELS];
	// Last written value to the F-num high / block / key on register for each
	// channel, with the key on bit cleared.
	uint8 _channelBxRegisterValues[OPL2_NUM_CHANNELS];
	// Last written value of the connection bit for each channel.
	uint8 _channelConnectionValues[OPL2_NUM_CHANNELS];

	// User volume settings.
	uint16 _userMusicVolume;
	bool _userMute;
};

} // namespace Got

#endif
