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

#include "scumm/players/player_ad.h"
#include "scumm/imuse/imuse.h"
#include "scumm/scumm.h"
#include "scumm/resource.h"

#include "audio/fmopl.h"

#include "common/textconsole.h"
#include "common/config-manager.h"

namespace Scumm {

#define AD_CALLBACK_FREQUENCY 472

Player_AD::Player_AD(ScummEngine *scumm, Audio::Mixer *mixer)
	: _vm(scumm), _mixer(mixer), _rate(mixer->getOutputRate()) {
	_opl2 = OPL::Config::create();
	if (!_opl2->init(_rate)) {
		error("Could not initialize OPL2 emulator");
	}

	_samplesPerCallback = _rate / AD_CALLBACK_FREQUENCY;
	_samplesPerCallbackRemainder = _rate % AD_CALLBACK_FREQUENCY;
	_samplesTillCallback = 0;
	_samplesTillCallbackRemainder = 0;

	memset(_registerBackUpTable, 0, sizeof(_registerBackUpTable));
	writeReg(0x01, 0x00);
	writeReg(0xBD, 0x00);
	writeReg(0x08, 0x00);
	writeReg(0x01, 0x20);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_engineMusicTimer = 0;
	_soundPlaying = -1;

	_curOffset = 0;

	_sfxTimer = 4;
	_rndSeed = 1;

	memset(_channels, 0, sizeof(_channels));
	memset(_sfxResource, 0, sizeof(_sfxResource));
	memset(_sfxPriority, 0, sizeof(_sfxPriority));
}

Player_AD::~Player_AD() {
	_mixer->stopHandle(_soundHandle);

	stopAllSounds();
	Common::StackLock lock(_mutex);
	delete _opl2;
	_opl2 = 0;
}

void Player_AD::setMusicVolume(int vol) {
	// HACK: We ignore the parameter and set up the volume specified in the
	// config manager. This allows us to differentiate between music and sfx
	// volume changes.
	setupVolume();
}

void Player_AD::startSound(int sound) {
	Common::StackLock lock(_mutex);

	// Query the sound resource
	const byte *res = _vm->getResourceAddress(rtSound, sound);

	if (res[2] == 0x80) {
		// Stop the current sounds
		stopAllSounds();

		// Lock the new music resource
		_soundPlaying = sound;
		_vm->_res->lock(rtSound, _soundPlaying);

		// Start the new music resource
		_resource = res;
		startMusic();
	} else {
		// Only try to start a sfx when no music is playing.
		if (_soundPlaying == -1) {
			const byte priority = res[0];
			const byte channel  = res[1];

			// Check for out of bounds access
			if (channel >= 3) {
				warning("AdLib sfx resource %d uses channel %d", sound, channel);
				return;
			}

			// Check whether the channel is free or the priority of the new
			// sfx resource is above the old one.
			if (_channels[channel * 3 + 0].state
			    || _channels[channel * 3 + 1].state
			    || _channels[channel * 3 + 2].state) {
				if (_sfxPriority[channel] > priority) {
					return;
				}
			}

			// Lock the new resource
			_sfxResource[channel] = sound;
			_sfxPriority[channel] = priority;
			_vm->_res->lock(rtSound, sound);

			// Start the actual sfx resource
			_resource = res;
			startSfx();
		}
	}

	// Setup the sound volume
	setupVolume();
}

void Player_AD::stopSound(int sound) {
	Common::StackLock lock(_mutex);

	if (sound == _soundPlaying) {
		stopAllSounds();
	} else {
		for (int i = 0; i < 3; ++i) {
			if (_sfxResource[i] == sound) {
				if (_channels[i * 3 + 0].state
				    || _channels[i * 3 + 1].state
				    || _channels[i * 3 + 2].state) {
					// Unlock the sound resource
					_vm->_res->unlock(rtSound, sound);

					// Stop the actual sfx playback
					_channels[i * 3 + 0].state = 0;
					_channels[i * 3 + 1].state = 0;
					_channels[i * 3 + 2].state = 0;
					clearChannel(i * 3 + 0);
					clearChannel(i * 3 + 1);
					clearChannel(i * 3 + 2);
				}
			}
		}
	}
}

void Player_AD::stopAllSounds() {
	Common::StackLock lock(_mutex);

	// Unlock the music resource if present
	if (_soundPlaying != -1) {
		_vm->_res->unlock(rtSound, _soundPlaying);
		_soundPlaying = -1;
	}

	// Stop the music playback
	_curOffset = 0;

	// Unloack all used sfx resources
	for (int i = 0; i < 3; ++i) {
		if (_channels[i * 3 + 0].state || _channels[i * 3 + 1].state || _channels[i * 3 + 2].state) {
			_vm->_res->unlock(rtSound, _sfxResource[i]);
		}
	}

	// Reset all the sfx channels
	for (int i = 0; i < 9; ++i) {
		_channels[i].state = 0;
		clearChannel(i);
	}

	writeReg(0xBD, 0x00);
}

int Player_AD::getMusicTimer() {
	return _engineMusicTimer;
}

int Player_AD::getSoundStatus(int sound) const {
	return (sound == _soundPlaying);
}

void Player_AD::saveLoadWithSerializer(Serializer *ser) {
	Common::StackLock lock(_mutex);

	if (ser->getVersion() < VER(95)) {
		IMuse *dummyImuse = IMuse::create(_vm->_system, NULL, NULL);
		dummyImuse->save_or_load(ser, _vm, false);
		delete dummyImuse;
		return;
	}

	// TODO: Be nicer than the original and save the data to continue the
	// currently played sound resources on load?
}

int Player_AD::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	int len = numSamples;

	while (len > 0) {
		if (!_samplesTillCallback) {
			// Run the update callback for music or sfx depending on which is
			// active.
			if (_curOffset) {
				updateMusic();
			} else {
				updateSfx();
			}

			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= AD_CALLBACK_FREQUENCY) {
				++_samplesTillCallback;
				_samplesTillCallbackRemainder -= AD_CALLBACK_FREQUENCY;
			}
		}

		const int samplesToRead = MIN(len, _samplesTillCallback);
		_opl2->readBuffer(buffer, samplesToRead);

		buffer += samplesToRead;
		len -= samplesToRead;
		_samplesTillCallback -= samplesToRead;
	}

	return numSamples;
}

void Player_AD::setupVolume() {
	// Setup the correct volume
	int soundVolumeMusic = CLIP<int>(ConfMan.getInt("music_volume"), 0, Audio::Mixer::kMaxChannelVolume);
	int soundVolumeSfx = CLIP<int>(ConfMan.getInt("sfx_volume"), 0, Audio::Mixer::kMaxChannelVolume);
	if (ConfMan.hasKey("mute")) {
		if (ConfMan.getBool("mute")) {
			soundVolumeMusic = 0;
			soundVolumeSfx = 0;
		}
	}

	// In case a music is being played set the music volume. Set the sfx
	// volume otherwise. This is safe because in the latter case either
	// sfx are playing or there is no sound being played at all.
	if (_soundPlaying != -1) {
		_mixer->setChannelVolume(_soundHandle, soundVolumeMusic);
	} else {
		_mixer->setChannelVolume(_soundHandle, soundVolumeSfx);
	}
}

void Player_AD::writeReg(int r, int v) {
	if (r >= 0 && r < ARRAYSIZE(_registerBackUpTable)) {
		_registerBackUpTable[r] = v;
	}
	_opl2->writeReg(r, v);
}

uint8 Player_AD::readReg(int r) const {
	if (r >= 0 && r < ARRAYSIZE(_registerBackUpTable)) {
		return _registerBackUpTable[r];
	} else {
		return 0;
	}
}

void Player_AD::setupChannel(const uint channel, const byte *instrOffset) {
	instrOffset += 2;
	writeReg(0xC0 + channel, *instrOffset++);
	setupOperator(_operatorOffsetTable[channel * 2 + 0], instrOffset);
	setupOperator(_operatorOffsetTable[channel * 2 + 1], instrOffset);
}

void Player_AD::setupOperator(const uint opr, const byte *&instrOffset) {
	writeReg(0x20 + opr, *instrOffset++);
	writeReg(0x40 + opr, *instrOffset++);
	writeReg(0x60 + opr, *instrOffset++);
	writeReg(0x80 + opr, *instrOffset++);
	writeReg(0xE0 + opr, *instrOffset++);
}

const int Player_AD::_operatorOffsetTable[18] = {
	 0,  3,  1,  4,
	 2,  5,  8, 11,
	 9, 12, 10, 13,
	16, 19, 17, 20,
	18, 21
};

// Music

void Player_AD::startMusic() {
	memset(_instrumentOffset, 0, sizeof(_instrumentOffset));
	memset(_channelLastEvent, 0, sizeof(_channelLastEvent));
	memset(_channelFrequency, 0, sizeof(_channelFrequency));
	memset(_channelB0Reg, 0, sizeof(_channelB0Reg));

	_voiceChannels = 0;
	uint instruments = _resource[10];
	for (uint i = 0; i < instruments; ++i) {
		const int instrIndex = _resource[11 + i] - 1;
		if (0 <= instrIndex && instrIndex < 16) {
			_instrumentOffset[instrIndex] = i * 16 + 16 + 3;
			_voiceChannels |= _resource[_instrumentOffset[instrIndex] + 13];
		}
	}

	if (_voiceChannels) {
		_mdvdrState = 0x20;
		_voiceChannels = 6;
	} else {
		_mdvdrState = 0;
		_voiceChannels = 9;
	}

	_curOffset = 0x93;
	// TODO: is this the same for Loom?
	_nextEventTimer = 40;
	_engineMusicTimer = 0;
	_internalMusicTimer = 0;
	_musicTimer = 0;

	writeReg(0xBD, _mdvdrState);

	const bool isLoom = (_vm->_game.id == GID_LOOM);
	_timerLimit = isLoom ? 473 : 256;
	_musicTicks = _resource[3] * (isLoom ? 2 : 1);
	_loopFlag = (_resource[4] == 0);
	_musicLoopStart = READ_LE_UINT16(_resource + 5);
}

void Player_AD::updateMusic() {
	_musicTimer += _musicTicks;
	if (_musicTimer < _timerLimit) {
		return;
	}
	_musicTimer -= _timerLimit;

	++_internalMusicTimer;
	if (_internalMusicTimer > 120) {
		_internalMusicTimer = 0;
		++_engineMusicTimer;
	}

	--_nextEventTimer;
	if (_nextEventTimer) {
		return;
	}

	while (true) {
		uint command = _resource[_curOffset++];
		if (command == 0xFF) {
			// META EVENT
			// Get the command number.
			command = _resource[_curOffset++];
			if (command == 47) {
				// End of track
				if (_loopFlag) {
					// In case the track is looping jump to the start.
					_curOffset = _musicLoopStart;
					_nextEventTimer = 0;
				} else {
					// Otherwise completely stop playback.
					stopAllSounds();
				}
				return;
			} else if (command == 88) {
				// This is proposedly a debug information insertion. The CMS
				// player code handles this differently, but is still using
				// the same resources...
				_curOffset += 5;
			} else if (command == 81) {
				// Change tempo. This is used exclusively in Loom.
				const uint timing = _resource[_curOffset + 2] | (_resource[_curOffset + 1] << 8);
				_musicTicks = 0x73000 / timing;
				command = _resource[_curOffset++];
				_curOffset += command;
			} else {
				// In case an unknown meta event occurs just skip over the
				// data by using the length supplied.
				command = _resource[_curOffset++];
				_curOffset += command;
			}
		} else {
			if (command >= 0x90) {
				// NOTE ON
				// Extract the channel number and save it in command.
				command -= 0x90;

				const uint instrOffset = _instrumentOffset[command];
				if (instrOffset) {
					if (_resource[instrOffset + 13] != 0) {
						setupRhythm(_resource[instrOffset + 13], instrOffset);
					} else {
						int channel = findFreeChannel();
						if (channel != -1) {
							noteOff(channel);
							setupChannel(channel, instrOffset);
							_channelLastEvent[channel] = command + 0x90;
							_channelFrequency[channel] = _resource[_curOffset];
							setupFrequency(channel, _resource[_curOffset]);
						}
					}
				}
			} else {
				// NOTE OFF
				const uint note = _resource[_curOffset];
				command += 0x10;

				// Find the output channel which plays the note.
				uint channel = 0xFF;
				for (uint i = 0; i < _voiceChannels; ++i) {
					if (_channelFrequency[i] == note && _channelLastEvent[i] == command) {
						channel = i;
						break;
					}
				}

				if (channel != 0xFF) {
					// In case a output channel playing the note was found,
					// stop it.
					noteOff(channel);
				} else {
					// In case there is no such note this will disable the
					// rhythm instrument played on the channel.
					command -= 0x90;
					const uint instrOffset = _instrumentOffset[command];
					if (instrOffset && _resource[instrOffset + 13] != 0) {
						const uint rhythmInstr = _resource[instrOffset + 13];
						if (rhythmInstr < 6) {
							_mdvdrState &= _mdvdrTable[rhythmInstr] ^ 0xFF;
							writeReg(0xBD, _mdvdrState);
						}
					}
				}
			}

			_curOffset += 2;
		}

		// In case there is a delay till the next event stop handling.
		if (_resource[_curOffset] != 0) {
			break;
		}
		++_curOffset;
	}

	_nextEventTimer = _resource[_curOffset++];
	if (_nextEventTimer & 0x80) {
		_nextEventTimer -= 0x80;
		_nextEventTimer <<= 7;
		_nextEventTimer |= _resource[_curOffset++];
	}

	_nextEventTimer >>= (_vm->_game.id == GID_LOOM) ? 2 : 1;
	if (!_nextEventTimer) {
		_nextEventTimer = 1;
	}
}

void Player_AD::noteOff(uint channel) {
	_channelLastEvent[channel] = 0;
	writeReg(0xB0 + channel, _channelB0Reg[channel] & 0xDF);
}

int Player_AD::findFreeChannel() {
	for (uint i = 0; i < _voiceChannels; ++i) {
		if (!_channelLastEvent[i]) {
			return i;
		}
	}

	return -1;
}

void Player_AD::setupFrequency(uint channel, int8 frequency) {
	frequency -= 31;
	if (frequency < 0) {
		frequency = 0;
	}

	uint octave = 0;
	while (frequency >= 12) {
		frequency -= 12;
		++octave;
	}

	const uint noteFrequency = _noteFrequencies[frequency];
	octave <<= 2;
	octave |= noteFrequency >> 8;
	octave |= 0x20;
	writeReg(0xA0 + channel, noteFrequency & 0xFF);
	_channelB0Reg[channel] = octave;
	writeReg(0xB0 + channel, octave);
}

void Player_AD::setupRhythm(uint rhythmInstr, uint instrOffset) {
	if (rhythmInstr == 1) {
		setupChannel(6, instrOffset);
		writeReg(0xA6, _resource[instrOffset++]);
		writeReg(0xB6, _resource[instrOffset] & 0xDF);
		_mdvdrState |= 0x10;
		writeReg(0xBD, _mdvdrState);
	} else if (rhythmInstr < 6) {
		const byte *secondOperatorOffset = _resource + instrOffset + 8;
		setupOperator(_rhythmOperatorTable[rhythmInstr], secondOperatorOffset);
		writeReg(0xA0 + _rhythmChannelTable[rhythmInstr], _resource[instrOffset++]);
		writeReg(0xB0 + _rhythmChannelTable[rhythmInstr], _resource[instrOffset++] & 0xDF);
		writeReg(0xC0 + _rhythmChannelTable[rhythmInstr], _resource[instrOffset]);
		_mdvdrState |= _mdvdrTable[rhythmInstr];
		writeReg(0xBD, _mdvdrState);
	}
}

const uint Player_AD::_noteFrequencies[12] = {
	0x200, 0x21E, 0x23F, 0x261,
	0x285, 0x2AB, 0x2D4, 0x300,
	0x32E, 0x35E, 0x390, 0x3C7
};

const uint Player_AD::_mdvdrTable[6] = {
	0x00, 0x10, 0x08, 0x04, 0x02, 0x01
};

const uint Player_AD::_rhythmOperatorTable[6] = {
	0x00, 0x00, 0x14, 0x12, 0x15, 0x11
};

const uint Player_AD::_rhythmChannelTable[6] = {
	0x00, 0x00, 0x07, 0x08, 0x08, 0x07
};

// SFX

void Player_AD::startSfx() {
	writeReg(0xBD, 0x00);

	// The second byte of the resource defines the logical channel where
	// the sound effect should be played.
	const int startChannel = _resource[1] * 3;

	// Clear the channel.
	_channels[startChannel + 0].state = 0;
	_channels[startChannel + 1].state = 0;
	_channels[startChannel + 2].state = 0;

	clearChannel(startChannel + 0);
	clearChannel(startChannel + 1);
	clearChannel(startChannel + 2);

	// Set up the first channel to pick up playback.
	_channels[startChannel].currentOffset = _channels[startChannel].startOffset = _resource + 2;
	_channels[startChannel].state = 1;

	// Scan for the start of the other channels and set them up if required.
	int curChannel = startChannel + 1;
	const byte *bufferPosition = _resource + 2;
	uint8 command = 0;
	while ((command = *bufferPosition) != 0xFF) {
		switch (command) {
		case 1:
			// INSTRUMENT DEFINITION
			bufferPosition += 15;
			break;

		case 2:
			// NOTE DEFINITION
			bufferPosition += 11;
			break;

		case 0x80:
			// LOOP
			bufferPosition += 1;
			break;

		default:
			// START OF CHANNEL
			bufferPosition += 1;
			_channels[curChannel].currentOffset = bufferPosition;
			_channels[curChannel].startOffset = bufferPosition;
			_channels[curChannel].state = 1;
			++curChannel;
			break;
		}
	}
}

void Player_AD::updateSfx() {
	if (--_sfxTimer) {
		return;
	}
	_sfxTimer = 4;

	for (int i = 0; i <= 9; ++i) {
		if (!_channels[i].state) {
			continue;
		}

		updateChannel(i);
	}
}

void Player_AD::clearChannel(int channel) {
	writeReg(0xA0 + channel, 0x00);
	writeReg(0xB0 + channel, 0x00);
}

void Player_AD::updateChannel(int channel) {
	if (_channels[channel].state == 1) {
		parseSlot(channel);
	} else {
		updateSlot(channel);
	}
}

void Player_AD::parseSlot(int channel) {
	while (true) {
		const byte *curOffset = _channels[channel].currentOffset;

		switch (*curOffset) {
		case 1:
			// INSTRUMENT DEFINITION
			++curOffset;
			_channels[channel].instrumentData[0] = *(curOffset + 0);
			_channels[channel].instrumentData[1] = *(curOffset + 2);
			_channels[channel].instrumentData[2] = *(curOffset + 9);
			_channels[channel].instrumentData[3] = *(curOffset + 8);
			_channels[channel].instrumentData[4] = *(curOffset + 4);
			_channels[channel].instrumentData[5] = *(curOffset + 3);
			_channels[channel].instrumentData[6] = 0;

			setupChannel(channel, curOffset);

			writeReg(0xA0 + channel, *(curOffset + 0));
			writeReg(0xB0 + channel, *(curOffset + 1) & 0xDF);

			_channels[channel].currentOffset += 15;
			break;

		case 2:
			// NOTE DEFINITION
			++curOffset;
			_channels[channel].state = 2;
			noteOffOn(channel);
			parseNote(channel, 0, curOffset);
			parseNote(channel, 1, curOffset);
			return;

		case 0x80:
			// LOOP
			_channels[channel].currentOffset = _channels[channel].startOffset;
			break;

		default:
			// START OF CHANNEL
			// When we encounter a start of another channel while playback
			// it means that the current channel is finished. Thus, we will
			// stop it.
			clearChannel(channel);
			_channels[channel].state = 0;

			// If no channel of the sound effect is playing anymore, unlock
			// the resource.
			channel /= 3;
			if (!_channels[channel + 0].state
			    && !_channels[channel + 1].state
			    && !_channels[channel + 2].state) {
				_vm->_res->unlock(rtSound, _sfxResource[channel]);
			}
			return;
		}
	}
}

void Player_AD::updateSlot(int channel) {
	const byte *curOffset = _channels[channel].currentOffset + 1;

	for (int num = 0; num <= 1; ++num, curOffset += 5) {
		if (!(*curOffset & 0x80)) {
			continue;
		}

		const int note = channel * 2 + num;
		bool updateNote = false;

		if (_notes[note].state == 2) {
			if (!--_notes[note].sustainTimer) {
				updateNote = true;
			}
		} else {
			updateNote = processNoteEnvelope(note, _notes[note].instrumentValue);

			if (_notes[note].bias) {
				writeRegisterSpecial(note, _notes[note].bias - _notes[note].instrumentValue, *curOffset & 0x07);
			} else {
				writeRegisterSpecial(note, _notes[note].instrumentValue, *curOffset & 0x07);
			}
		}

		if (updateNote) {
			if (processNote(note, curOffset)) {
				if (!(*curOffset & 0x08)) {
					_channels[channel].currentOffset += 11;
					_channels[channel].state = 1;
					continue;
				} else if (*curOffset & 0x10) {
					noteOffOn(channel);
				}

				_notes[note].state = -1;
				processNote(note, curOffset);
			}
		}

		if ((*curOffset & 0x20) && !--_notes[note].playTime) {
			_channels[channel].currentOffset += 11;
			_channels[channel].state = 1;
		}
	}
}

void Player_AD::parseNote(int channel, int num, const byte *offset) {
	if (num) {
		offset += 5;
	}

	if (*offset & 0x80) {
		const int note = channel * 2 + num;
		_notes[note].state = -1;
		processNote(note, offset);
		_notes[note].playTime = 0;

		if (*offset & 0x20) {
			_notes[note].playTime = (*(offset + 4) >> 4) * 118;
			_notes[note].playTime += (*(offset + 4) & 0x0F) * 8;
		}
	}
}

bool Player_AD::processNote(int note, const byte *offset) {
	if (++_notes[note].state == 4) {
		return true;
	}

	const int instrumentDataOffset = *offset & 0x07;
	_notes[note].bias = _noteBiasTable[instrumentDataOffset];

	uint8 instrumentDataValue = 0;
	if (_notes[note].state == 0) {
		instrumentDataValue = _channels[note / 2].instrumentData[instrumentDataOffset];
	}

	uint8 noteInstrumentValue = readRegisterSpecial(note, instrumentDataValue, instrumentDataOffset);
	if (_notes[note].bias) {
		noteInstrumentValue = _notes[note].bias - noteInstrumentValue;
	}
	_notes[note].instrumentValue = noteInstrumentValue;

	if (_notes[note].state == 2) {
		_notes[note].sustainTimer = _numStepsTable[*(offset + 3) >> 4];

		if (*offset & 0x40) {
			_notes[note].sustainTimer = (((getRnd() << 8) * _notes[note].sustainTimer) >> 16) + 1;
		}
	} else {
		int timer1, timer2;
		if (_notes[note].state == 3) {
			timer1 = *(offset + 3) & 0x0F;
			timer2 = 0;
		} else {
			timer1 = *(offset + _notes[note].state + 1) >> 4;
			timer2 = *(offset + _notes[note].state + 1) & 0x0F;
		}

		int adjustValue = ((_noteAdjustTable[timer2] * _noteAdjustScaleTable[instrumentDataOffset]) >> 16) - noteInstrumentValue;
		setupNoteEnvelopeState(note, _numStepsTable[timer1], adjustValue);
	}

	return false;
}

void Player_AD::noteOffOn(int channel) {
	const uint8 regValue = readReg(0xB0 | channel);
	writeReg(0xB0 | channel, regValue & 0xDF);
	writeReg(0xB0 | channel, regValue | 0x20);
}

void Player_AD::writeRegisterSpecial(int note, uint8 value, int offset) {
	if (offset == 6) {
		return;
	}

	// Division by 2 extracts the channel number out of the note.
	note /= 2;

	uint8 regNum;
	if (_useOperatorTable[offset]) {
		regNum = _operatorOffsetTable[_channelOperatorOffsetTable[offset] + note * 2];
	} else {
		regNum = _channelOffsetTable[note];
	}

	regNum += _baseRegisterTable[offset];

	uint8 regValue = readReg(regNum) & (~_registerMaskTable[offset]);
	regValue |= value << _registerShiftTable[offset];

	writeReg(regNum, regValue);
}

uint8 Player_AD::readRegisterSpecial(int note, uint8 defaultValue, int offset) {
	if (offset == 6) {
		return 0;
	}

	// Division by 2 extracts the channel number out of the note.
	note /= 2;

	uint8 regNum;
	if (_useOperatorTable[offset]) {
		regNum = _operatorOffsetTable[_channelOperatorOffsetTable[offset] + note * 2];
	} else {
		regNum = _channelOffsetTable[note];
	}

	regNum += _baseRegisterTable[offset];

	uint8 regValue;
	if (defaultValue) {
		regValue = defaultValue;
	} else {
		regValue = readReg(regNum);
	}

	regValue &= _registerMaskTable[offset];
	regValue >>= _registerShiftTable[offset];

	return regValue;
}

void Player_AD::setupNoteEnvelopeState(int note, int steps, int adjust) {
	_notes[note].preIncrease = 0;
	if (ABS(adjust) > steps) {
		_notes[note].preIncrease = 1;
		_notes[note].adjust = adjust / steps;
		_notes[note].envelope.stepIncrease = ABS(adjust % steps);
	} else {
		_notes[note].adjust = adjust;
		_notes[note].envelope.stepIncrease = ABS(adjust);
	}

	_notes[note].envelope.step = steps;
	_notes[note].envelope.stepCounter = 0;
	_notes[note].envelope.timer = steps;
}

bool Player_AD::processNoteEnvelope(int note, int &instrumentValue) {
	if (_notes[note].preIncrease) {
		instrumentValue += _notes[note].adjust;
	}

	_notes[note].envelope.stepCounter += _notes[note].envelope.stepIncrease;
	if (_notes[note].envelope.stepCounter >= _notes[note].envelope.step) {
		_notes[note].envelope.stepCounter -= _notes[note].envelope.step;

		if (_notes[note].adjust < 0) {
			--instrumentValue;
		} else {
			++instrumentValue;
		}
	}

	if (--_notes[note].envelope.timer) {
		return false;
	} else {
		return true;
	}
}

uint8 Player_AD::getRnd() {
	if (_rndSeed & 1) {
		_rndSeed >>= 1;
		_rndSeed ^= 0xB8;
	} else {
		_rndSeed >>= 1;
	}

	return _rndSeed;
}

const uint Player_AD::_noteBiasTable[7] = {
	0x00, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x00
};

const uint Player_AD::_numStepsTable[16] = {
	    1,    4,    6,    8,
	   10,   14,   18,   24,
	   36,   64,  100,  160,
	  240,  340,  600, 1200
};

const uint Player_AD::_noteAdjustScaleTable[7] = {
	255,   7,  63,  15,  63,  15,  63
};

const uint Player_AD::_noteAdjustTable[16] = {
	    0,  4369,  8738, 13107,
	17476, 21845, 26214, 30583,
	34952, 39321, 43690, 48059,
	52428, 46797, 61166, 65535
};

const bool Player_AD::_useOperatorTable[7] = {
	false, false, true, true, true, true, false
};

const uint Player_AD::_channelOffsetTable[11] = {
	 0,  1,  2,  3,
	 4,  5,  6,  7,
	 8,  8,  7
};

const uint Player_AD::_channelOperatorOffsetTable[7] = {
	0, 0, 1, 1, 0, 0, 0
};

const uint Player_AD::_baseRegisterTable[7] = {
	0xA0, 0xC0, 0x40, 0x20, 0x40, 0x20, 0x00
};

const uint Player_AD::_registerMaskTable[7] = {
	0xFF, 0x0E, 0x3F, 0x0F, 0x3F, 0x0F, 0x00
};

const uint Player_AD::_registerShiftTable[7] = {
	0, 1, 0, 0, 0, 0, 0
};

} // End of namespace Scumm
