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

#include "got/musicdriver_adlib.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "audio/mididrv.h"


namespace Got {

MusicDriver_Got_AdLib::MusicDriver_Got_AdLib(uint8 timerFrequency) : MusicDriver_Got(timerFrequency), 
		_opl(nullptr),
		_userMusicVolume(256),
		_userMute(false) {
	Common::fill(_channelOp0LevelRegisterValues, _channelOp0LevelRegisterValues + ARRAYSIZE(_channelOp0LevelRegisterValues), 0);
	Common::fill(_channelOp1LevelRegisterValues, _channelOp1LevelRegisterValues + ARRAYSIZE(_channelOp1LevelRegisterValues), 0);
	Common::fill(_channelBxRegisterValues, _channelBxRegisterValues + ARRAYSIZE(_channelBxRegisterValues), 0);
	Common::fill(_channelConnectionValues, _channelConnectionValues + ARRAYSIZE(_channelConnectionValues), 0);
}

MusicDriver_Got_AdLib::~MusicDriver_Got_AdLib() {
	close();
}

int MusicDriver_Got_AdLib::open() {
	if (_isOpen)
		return MidiDriver::MERR_ALREADY_OPEN;

	int8 detectResult = OPL::Config::detect(OPL::Config::kOpl2);
	if (detectResult == -1)
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;

	// Create the emulator / hardware interface.
	_opl = OPL::Config::create(OPL::Config::kOpl2);

	if (_opl == nullptr)
		return MidiDriver::MERR_CANNOT_CONNECT;

	// Initialize emulator / hardware interface.
	if (!_opl->init())
		return MidiDriver::MERR_CANNOT_CONNECT;

	_isOpen = true;

	// Set default OPL register values.
	initOpl();

	// Start the emulator / hardware interface. This will also start the timer
	// callbacks.
	_opl->start(new Common::Functor0Mem<void, MusicDriver_Got_AdLib>(this, &MusicDriver_Got_AdLib::onTimer), _timerFrequency);

	return 0;
}

void MusicDriver_Got_AdLib::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	stopAllNotes();

	if (_opl) {
		_opl->stop();
		delete _opl;
		_opl = nullptr;
	}
}

void MusicDriver_Got_AdLib::syncSoundSettings() {
	// Get user volume settings.
	_userMusicVolume = MIN(256, ConfMan.getInt("music_volume"));
	_userMute = ConfMan.getBool("mute") || ConfMan.getBool("music_mute");

	// Apply the user volume.
	recalculateVolumes();
}

void MusicDriver_Got_AdLib::send(uint16 b) {
	uint8 oplRegister = b >> 8;
	uint8 value = b & 0xFF;

	if (oplRegister >= OPL_REGISTER_BASE_LEVEL && oplRegister < OPL_REGISTER_BASE_LEVEL + 0x20) {
		// Write to a level register.

		// Determine the channel and operator from the register number.
		uint8 regOffset = oplRegister - OPL_REGISTER_BASE_LEVEL;
		uint8 oplChannel = ((regOffset / 8) * 3) + ((regOffset % 8) % 3);
		uint8 oplOperator = (regOffset % 8) / 3;
		assert(oplChannel < OPL2_NUM_CHANNELS);
		assert(oplOperator < 2);

		// Store the new register value.
		if (oplOperator == 0) {
			_channelOp0LevelRegisterValues[oplChannel] = value;
		} else {
			_channelOp1LevelRegisterValues[oplChannel] = value;
		}

		// Apply user volume settings to the level.
		uint8 scaledLevel = calculateVolume(oplChannel, oplOperator);
		// Add the KSL bits to the new level value.
		value = (value & 0xC0) | scaledLevel;
	} else if ((oplRegister & 0xF0) == OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON) {
		// Write to an F-num high / block / key on register.

		uint8 oplChannel = oplRegister & 0x0F;
		assert(oplChannel < OPL2_NUM_CHANNELS);

		// Store the value, but clear the key on bit.
		_channelBxRegisterValues[oplChannel] = value & 0x1F;
	} else if ((oplRegister & 0xF0) == OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING) {
		// Write to a connection register.

		uint8 oplChannel = oplRegister & 0x0F;
		assert(oplChannel < OPL2_NUM_CHANNELS);

		// Store the connection bit.
		_channelConnectionValues[oplChannel] = value & 0x01;
	}

	// Write the new register value to the OPL chip.
	writeRegister(oplRegister, value);
}

void MusicDriver_Got_AdLib::stopAllNotes() {
	// Clear the key on bit on all OPL channels.
	for (int i = 0; i < OPL2_NUM_CHANNELS; i++) {
		writeRegister(0xB0 | i, _channelBxRegisterValues[i]);
	}
}

void MusicDriver_Got_AdLib::initOpl() {
	// Clear test flags and enable waveform select.
	writeRegister(0x01, 0x20);

	// Clear, stop and mask the timers and reset the interrupt.
	writeRegister(0x02, 0);
	writeRegister(0x03, 0);
	writeRegister(0x04, 0x60);
	writeRegister(0x04, 0x80);

	// Set note select mode 0 and disable CSM mode.
	writeRegister(0x08, 0);

	// Clear operator registers.
	for (int i = 0; i < 5; i++) {
		uint8 baseReg = 0;
		switch (i) {
		case 0:
			baseReg = OPL_REGISTER_BASE_FREQMULT_MISC;
			break;
		case 1:
			baseReg = OPL_REGISTER_BASE_LEVEL;
			break;
		case 2:
			baseReg = OPL_REGISTER_BASE_DECAY_ATTACK;
			break;
		case 3:
			baseReg = OPL_REGISTER_BASE_RELEASE_SUSTAIN;
			break;
		case 4:
			baseReg = OPL_REGISTER_BASE_WAVEFORMSELECT;
			break;
		}

		for (int j = 0; j < OPL2_NUM_CHANNELS; j++) {
			writeRegister(baseReg + determineOperatorRegisterOffset(j, 0), 0);
			writeRegister(baseReg + determineOperatorRegisterOffset(j, 1), 0);
		}
	}

	// Clear channel registers.
	for (int i = 0; i < 3; i++) {
		uint8 baseReg = 0;
		switch (i) {
		case 0:
			baseReg = OPL_REGISTER_BASE_FNUMLOW;
			break;
		case 1:
			baseReg = OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON;
			break;
		case 2:
			baseReg = OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING;
			break;
		}

		for (int j = 0; j < OPL2_NUM_CHANNELS; j++) {
			writeRegister(baseReg + j, 0);
		}
	}

	// Disable rhythm mode and set modulation and vibrato depth to low.
	writeRegister(0xBD, 0);
}

void MusicDriver_Got_AdLib::recalculateVolumes() {
	// Determine the value for all level registers and write them out.
	for (int i = 0; i < OPL2_NUM_CHANNELS; i++) {
		uint8 oplRegister = OPL_REGISTER_BASE_LEVEL + determineOperatorRegisterOffset(i, 0);
		uint8 value = (_channelOp0LevelRegisterValues[i] & 0xC0) | calculateVolume(i, 0);
		writeRegister(oplRegister, value);

		oplRegister = OPL_REGISTER_BASE_LEVEL + determineOperatorRegisterOffset(i, 1);
		value = (_channelOp1LevelRegisterValues[i] & 0xC0) | calculateVolume(i, 1);
		writeRegister(oplRegister, value);
	}
}

uint8 MusicDriver_Got_AdLib::calculateVolume(uint8 channel, uint8 operatorNum) {
	// Get the last written level for this operator.
	uint8 operatorDefVolume = (operatorNum == 0 ? _channelOp0LevelRegisterValues[channel] : _channelOp1LevelRegisterValues[channel]) & 0x3F;

	// Determine if volume settings should be applied to this operator.
	if (!isVolumeApplicableToOperator(channel, operatorNum))
		// No need to apply volume settings; just use the level as written.
		return operatorDefVolume;

	uint8 invertedVolume = 0x3F - operatorDefVolume;

	// Scale by user volume.
	if (_userMute) {
		invertedVolume = 0;
	} else {
		invertedVolume = (invertedVolume * _userMusicVolume) >> 8;
	}
	uint8 scaledVolume = 0x3F - invertedVolume;

	return scaledVolume;
}

bool MusicDriver_Got_AdLib::isVolumeApplicableToOperator(uint8 channel, uint8 operatorNum) {
	// 2 operator instruments have 2 different operator connections:
	// additive (0x01) or FM (0x00) synthesis.  Carrier operators in FM
	// synthesis and both operators in additive synthesis need to have
	// volume settings applied; modulator operators just use the level
	// as written. In FM synthesis connection, operator 1 is a carrier.
	return _channelConnectionValues[channel] == 0x01 || operatorNum == 1;
}

uint16 MusicDriver_Got_AdLib::determineOperatorRegisterOffset(uint8 oplChannel, uint8 operatorNum) {
	// 2 operator register offset for each channel and operator:
	//
	// Channel  | 0 | 1 | 2 | 0 | 1 | 2 | 3 | 4 | 5 | 3 | 4 | 5 | 6 | 7 | 8 | 6 | 7 | 8 |
	// Operator | 0         | 1         | 0         | 1         | 0         | 1         |
	// Register | 0 | 1 | 2 | 3 | 4 | 5 | 8 | 9 | A | B | C | D |10 |11 |12 |13 |14 |15 |
	return (oplChannel / 3 * 8) + (oplChannel % 3) + (operatorNum * 3);
}

void MusicDriver_Got_AdLib::writeRegister(uint8 oplRegister, uint8 value) {
	//debug("Writing register %X %X", oplRegister, value);

	_opl->writeReg(oplRegister, value);
}

void MusicDriver_Got_AdLib::setTimerFrequency(uint8 timerFrequency) {
	if (timerFrequency == _timerFrequency)
		return;

	MusicDriver_Got::setTimerFrequency(timerFrequency);

	if (isOpen()) {
		// Update OPL timer frequency.
		_opl->stop();
		_opl->start(new Common::Functor0Mem<void, MusicDriver_Got_AdLib>(this, &MusicDriver_Got_AdLib::onTimer), _timerFrequency);
	}
}

} // namespace Got
