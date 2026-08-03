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

#include "audio/fmopl.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "mads/nebular/sound/asound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

constexpr int CHAN_COMMAND_COUNT = 15;

bool AdlibChannel::_channelsEnabled;

void AdlibChannel::reset() {
	_activeCount = 0;
	_pitchBend = 0;
	_volumeFadeStep = 0;
	_attenFadeStep = 0;
}

void AdlibChannel::enable(int flag) {
	if (_activeCount) {
		_pendingStop = flag;

		// WORKAROUND: Original set _soundData pointer to flag. Since this seems
		// just intended to invalidate any prior pointer, I've replaced it with
		// a simple null pointer
		_soundData = nullptr;
	}

	_channelsEnabled = true;
}

void AdlibChannel::setPtr2(byte *pData) {
	_pSrc = pData;
	_volumeFadeStep = 0xFF;
	_volumeFadeReload = 1;
	_volumeFadeCounter = 1;
}

void AdlibChannel::load(byte *pData) {
	_ptr1 = _pSrc = _innerLoopPtr = pData;
	_outerLoopPtr = _soundData = pData;
	_volumeOffset = 0;
	_volumeFadeReload = 0xFF;
	_activeCount = 1;
	_patchAttenuation = 64;
	_pitchBend = 0;
	_octaveTranspose = 0;
	_volumeFadeStep = _attenFadeStep = 0;
	_volume = _noteOffset = 0;
	_transpose = 0;
	_pendingStop = 0;
	_volumeFadeCounter = 0;
	_attenFadeCounter = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
}

void AdlibChannel::check(byte *nullPtr) {
	if (_activeCount && _pendingStop) {
		if (!_volumeOffset) {
			_pSrc = nullPtr;
			_pendingStop = 0;
		} else {
			_volumeFadeStep = 0xFF;
			_volumeFadeReload = 4;
			if (!_volumeFadeCounter)
				_volumeFadeCounter = 1;
		}
	}
}

/*-----------------------------------------------------------------------*/

AdlibSample::AdlibSample(Common::SeekableReadStream &s) {
	_attackRate = s.readByte();
	_decayRate = s.readByte();
	_sustainLevel = s.readByte();
	_releaseRate = s.readByte();
	_egTyp = s.readByte() != 0;
	_ksr = s.readByte() != 0;
	_totalLevel = s.readByte();
	_scalingLevel = s.readByte();
	_waveformSelect = s.readByte();
	_freqMultiple = s.readByte();
	_feedback = s.readByte();
	_ampMod = s.readByte() != 0;
	_vib = s.readByte();
	_alg = s.readByte();
	_noiseMode = s.readByte();
	s.skip(1);
	_freqMask = s.readUint16LE();
	_freqBase = s.readUint16LE();
	_freqStep = s.readUint16LE();
}

/*-----------------------------------------------------------------------*/

ASound::ASound(Audio::Mixer *mixer, const Common::Path &filename, int dataOffset, int dataSize) :
		SoundDriver(mixer, filename, dataOffset, dataSize) {
	// Initialize fields
	_commandParam = 0;
	_activeChannelPtr = nullptr;
	_samplePtr = nullptr;
	_frameCounter = 0;
	_isDisabled = false;
	_masterVolume = 255;
	_noiseTicks1 = 0;
	_noiseTicks2 = 0;
	_activeChannelNumber = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_noiseChannel1 = _noiseChannel2 = 0;
	_noiseFreqStep1 = 0;
	_noiseFreqStep2 = 0;
	_savedNoiseTicks1 = 0;
	_savedNoiseTicks2 = 0;
	_pollResult = 0;
	_resultFlag = 0;
	_nullData[0] = _nullData[1] = 0;
	Common::fill(&_ports[0], &_ports[256], 0);
	_stateFlag = false;
	_activeChannelReg = 0;
	_outputReg = 0;
	_randomSeed = 1234;
	_amDep = _vibDep = _splitPoint = true;

	for (int i = 0; i < 11; ++i) {
		_channelData[i]._hasNoiseMode = 0;
		_channelData[i]._freqMask = 0;
		_channelData[i]._freqBase = 0;
		_channelData[i]._freqStep = 0;
	}

	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i]._owner = this;

	AdlibChannel::_channelsEnabled = false;

	// Initialize the OPL instance
	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, ASound>(this, &ASound::onTimer), CALLBACKS_PER_SECOND);

	// Initialize the Adlib
	adlibInit();

	// Reset the adlib
	command0();
}

ASound::~ASound() {
	_opl->stop();
	delete _opl;
}

void ASound::validate(bool isDemo) {
	Common::File f;
	static const char *const MD5[] = {
		"205398468de2c8873b7d4d73d5be8ddc",
		"f9b2d944a2fb782b1af5c0ad592306d3",
		"7431f8dad77d6ddfc24e6f3c0c4ac7df",
		"eb1f3f5a4673d3e73d8ac1818c957cf4",
		"f936dd853073fa44f3daac512e91c476",
		"3dc139d3e02437a6d9b732072407c366",
		"af0edab2934947982e9a405476702e03",
		"8cbc25570b50ba41c9b5361cad4fbedc",
		"a31e4783e098f633cbb6689adb41dd4f"
	};

	static const char *const MD5_DEMO[] = {
		"11aa27a511abdfa905fd290b4d96e9d3",
		"057c16af3b84b019bf088cb0ef8e6c4f"
	};

	for (int i = 1; i <= 9; ++i) {
		if (isDemo && i != 1 && i != 9)
			continue;
		const char *const EXPECTED = isDemo ? MD5_DEMO[i == 1 ? 0 : 1] : MD5[i - 1];

		Common::Path filename(Common::String::format("ASOUND.00%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != EXPECTED)
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void ASound::adlibInit() {
	write(4, 0x60);
	write(4, 0x80);
	write(2, 0xff);
	write(4, 0x21);
	write(4, 0x60);
	write(4, 0x80);
}

int ASound::stop() {
	command0();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

int ASound::poll() {
	// Update any playing sounds
	update();

	// Return result
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

void ASound::noise() {
	int randomVal = getRandomNumber();

	if (_noiseTicks1) {
		setFrequency(_noiseChannel1, ((randomVal ^ 0xFFFF) & _freqMask1) + _freqBase1);
	}

	if (_noiseTicks2) {
		setFrequency(_noiseChannel2, (randomVal & _freqMask2) + _freqBase2);
	}
}

void ASound::write(uint8 reg, uint8 val) {
	_opl->writeReg(reg, val);
}

int ASound::write2(int state, uint8 reg, uint8 val) {
	_ports[reg] = val;
	write(reg, val);
	return state;
}

void ASound::channelOn(int reg, int volume) {
	write2(8, reg, (_ports[reg] & 0xC0) | (volume & 0x3F));
}

void ASound::channelOff(int reg) {
	write2(8, reg, _ports[reg] | 0x3F);
}

void ASound::resultCheck() {
	if (_resultFlag != 1) {
		_resultFlag = 1;
		_pollResult = 1;
	}
}

void ASound::playSound(int offset) {
	// Load the specified data block
	playSoundData(loadData(offset));
}

void ASound::playSoundData(byte *pData, int startingChannel) {
	// Scan for a high level free channel
	for (int i = startingChannel; i < ADLIB_CHANNEL_COUNT; ++i) {
		if (!_channels[i]._activeCount) {
			_channels[i].load(pData);
			return;
		}
	}

	// None found, do a secondary scan for an interruptable channel
	for (int i = ADLIB_CHANNEL_COUNT - 1; i >= startingChannel; --i) {
		if (_channels[i]._pendingStop == 0xFF) {
			_channels[i].load(pData);
			return;
		}
	}
}

bool ASound::isSoundActive(byte *pData) {
	for (int i = 0; i <= ADLIB_CHANNEL_MIDWAY; ++i) {
		if (_channels[i]._activeCount && _channels[i]._soundData == pData)
			return true;
	}

	return false;
}

void ASound::setFrequency(int channel, int freq) {
	write2(8, 0xA0 + channel, freq & 0xFF);
	write2(8, 0xB0 + channel, (freq >> 8) | 0x20);
}

int ASound::getRandomNumber() {
	int v = 0x9248 + (int)_randomSeed;
	_randomSeed = ((v >> 3) | (v << 13)) & 0xFFFF;
	return _randomSeed;
}

void ASound::update() {
	getRandomNumber();
	if (_isDisabled)
		return;

	tickCallback();

	++_frameCounter;
	pollChannels();
	checkChannels();

	if (_noiseTicks1 == _noiseTicks2) {
		if (_resultFlag != -1) {
			_resultFlag = -1;
			_pollResult = -1;
		}
	} else {
		if (_noiseTicks1) {
			_freqBase1 += _noiseFreqStep1;
			if (!--_noiseTicks1) {
				if (!_noiseTicks2 || _noiseChannel1 != _noiseChannel2) {
					write2(8, 0xA0 + _noiseChannel1, 0);
					write2(8, 0xB0 + _noiseChannel1, 0);
				}
			}
		}

		if (_noiseTicks2) {
			_freqBase2 += _noiseFreqStep2;
			if (!--_noiseTicks2) {
				if (!_noiseTicks1 || _noiseChannel2 != _noiseChannel1) {
					write2(8, 0xA0 + _noiseChannel2, 0);
					write2(8, 0xB0 + _noiseChannel2, 0);
				}
			}
		}
	}
}

void ASound::pollChannels() {
	_activeChannelNumber = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i) {
		_activeChannelPtr = &_channels[i];
		pollActiveChannel();
	}
}

void ASound::checkChannels() {
	if (AdlibChannel::_channelsEnabled) {
		for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
			_channels[i].check(_nullData);
	}
}

void ASound::pollActiveChannel() {
	AdlibChannel *chan = _activeChannelPtr;

	if (chan->_activeCount) {
		if (chan->_keyOnDelay > 0 && --chan->_keyOnDelay == 0)
			updateOctave();

		bool updateFlag = true;
		if (--_activeChannelPtr->_activeCount <= 0) {
			for (;;) {
				byte *pSrc = chan->_pSrc;
				if (!chan->_ptr1 || !pSrc) {
					warning("pollActiveChannel(): No data found for sound channel");
					break;
				}

				if (!(*pSrc & 0x80) || (*pSrc <= (0xff - CHAN_COMMAND_COUNT))) {
					if (updateFlag)
						updateActiveChannel();

					chan->_note = *pSrc++;
					chan->_activeCount = *pSrc++;
					chan->_pSrc += 2;

					if (!chan->_note || !chan->_activeCount) {
						updateOctave();
					} else {
						chan->_keyOnDelay = chan->_activeCount - chan->_noteOffset;
						updateChannelState();
					}

					// Break out of processing loop
					break;
				} else {
					updateFlag = false;
					channelCommand(pSrc, updateFlag);
				}
			}
		}

		if (chan->_pitchBend)
			updateFNumber();

		updateFlag = false;
		if (chan->_volumeFadeCounter || chan->_attenFadeCounter) {
			if (!--chan->_volumeFadeCounter) {
				chan->_volumeFadeCounter = chan->_volumeFadeReload;
				if (chan->_volumeFadeStep) {
					int8 newVal = (int8)chan->_volumeFadeStep + (int8)chan->_volumeOffset;
					if (newVal < 0) {
						chan->_volumeFadeCounter = 0;
						newVal = 0;
					} else if (newVal > 63) {
						chan->_volumeFadeCounter = 0;
						newVal = 63;
					}

					chan->_volumeOffset = newVal;
					updateFlag = true;
				}
			}

			if (!--chan->_attenFadeCounter) {
				chan->_attenFadeCounter = chan->_attenFadeReload;
				if (chan->_attenFadeStep) {
					chan->_patchAttenuation += chan->_attenFadeStep;
					updateFlag = true;
				}
			}

			if (updateFlag)
				updateActiveChannel();
		}
	}

	++_activeChannelNumber;
}

void ASound::updateOctave() {
	int reg = 0xB0 + _activeChannelNumber;
	write2(8, reg, _ports[reg] & 0xDF);
}

static int _vList1[] = {
	0x200, 0x21E, 0x23F, 0x261, 0x285, 0x2AB,
	0x2D4, 0x2FF, 0x32D, 0x35D, 0x390, 0x3C7
};

void ASound::updateChannelState() {
	updateActiveChannel();

	if (_channelData[_activeChannelNumber]._hasNoiseMode) {
		if (_noiseChannel1 == _activeChannelNumber)
			_stateFlag = 0;
		if (_noiseChannel2 == _activeChannelNumber)
			_stateFlag = 1;

		if (!_stateFlag) {
			_stateFlag = 1;
			if (_noiseTicks1)
				write2(8, 0xB0 + _noiseChannel1, _ports[0xB0 + _noiseChannel1] & 0xDF);

			_noiseChannel1 = _activeChannelNumber;
			_noiseTicks1 = _channelData[_noiseChannel1]._hasNoiseMode;
			_freqMask1 = _channelData[_noiseChannel1]._freqMask;
			_freqBase1 = _channelData[_noiseChannel1]._freqBase;
			_noiseFreqStep1 = _channelData[_noiseChannel1]._freqStep;
		} else {
			_stateFlag = 0;
			if (_noiseTicks2)
				write2(8, 0xB0 + _noiseChannel2, _ports[0xB0 + _noiseChannel2] & 0xDF);

			_noiseChannel2 = _activeChannelNumber;
			_noiseTicks2 = _channelData[_noiseChannel2]._hasNoiseMode;
			_freqMask2 = _channelData[_noiseChannel2]._freqMask;
			_freqBase2 = _channelData[_noiseChannel2]._freqBase;
			_noiseFreqStep2 = _channelData[_noiseChannel2]._freqStep;
		}

		resultCheck();
	} else {
		int reg = 0xA0 + _activeChannelNumber;
		int vTimes = (byte)(_activeChannelPtr->_note + _activeChannelPtr->_octaveTranspose) / 12;
		int vOffset = (byte)(_activeChannelPtr->_note + _activeChannelPtr->_octaveTranspose) % 12;
		int val = _vList1[vOffset] + _activeChannelPtr->_transpose;
		write2(8, reg, val & 0xFF);

		reg += 0x10;
		write2(8, reg, (_ports[reg] & 0x20) | (vTimes << 2) | (val >> 8));

		write2(8, reg, _ports[reg] | 0x20);
	}
}

static const int outputIndexes[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 13, 16, 14, 17
};
static const int outputChannels[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21, 0
};

void ASound::updateActiveChannel() {
	int reg = 0x40 + outputChannels[outputIndexes[_activeChannelNumber * 2 + 1]];
	int portVal = _ports[reg] & 0xFFC0;
	int newVolume = CLIP(_activeChannelPtr->_volume + _activeChannelPtr->_volumeOffset
		+ (int8)_activeChannelPtr->_channelAttenuation, 0, 63);
	newVolume = newVolume * _masterVolume / 255;

	// Note: Original had a whole block not seeming to be used, since the initialisation
	// sets a variable to 5660h, and doesn't change it, so the branch is never taken
	portVal |= 63 - newVolume;

	write2(8, reg, portVal);
}

void ASound::loadSample(int sampleIndex) {
	_activeChannelReg = 0xB0 + _activeChannelNumber;
	write2(8, _activeChannelReg, _ports[_activeChannelReg] & 0xDF);

	_activeChannelReg = _activeChannelNumber;
	_samplePtr = &_samples[sampleIndex * 2];
	_outputReg = outputChannels[outputIndexes[_activeChannelReg * 2]];
	processSample();

	AdlibChannelData &cd = _channelData[_activeChannelNumber];
	cd._freqStep = _samplePtr->_freqStep;
	cd._freqBase = _samplePtr->_freqBase;
	cd._freqMask = _samplePtr->_freqMask;
	cd._hasNoiseMode = _samplePtr->_noiseMode;

	_samplePtr = &_samples[sampleIndex * 2 + 1];
	_outputReg = outputChannels[outputIndexes[_activeChannelReg * 2 + 1]];
	processSample();
}

void ASound::processSample() {
	// Write out vib flags and split point
	write2(8, 0x40 + _outputReg, 0x3F);
	int depthRhythm = (_ports[0xBD] & 0x3F) | (_amDep ? 0x80 : 0) |
		(_vibDep ? 0x40 : 0);
	write2(8, 0xBD, depthRhythm);
	write2(8, 8, _splitPoint ? 0x40 : 0);

	// Write out feedback & Alg
	int val = (_samplePtr->_feedback << 1) | (1 - _samplePtr->_alg);
	write2(8, 0xC0 + _activeChannelReg, val);

	// Write out attack/decay rate
	val = (_samplePtr->_attackRate << 4) | (_samplePtr->_decayRate & 0xF);
	write2(8, 0x60 + _outputReg, val);

	// Write out sustain level/release rate
	val = (_samplePtr->_sustainLevel << 4) | (_samplePtr->_releaseRate & 0xF);
	write2(8, 0x80 + _outputReg, val);

	// Write out misc flags
	val = (_samplePtr->_ampMod ? 0x80 : 0) | (_samplePtr->_vib ? 0x40 : 0)
		| (_samplePtr->_egTyp ? 0x20 : 0) | (_samplePtr->_ksr ? 0x10 : 0)
		| (_samplePtr->_freqMultiple & 0xF);
	write2(8, 0x20 + _outputReg, val);

	// Write out waveform select
	write2(8, 0xE0 + _outputReg, _samplePtr->_waveformSelect & 3);

	// Write out total level & scaling level
	val = -((_samplePtr->_totalLevel & 0x3F) - 0x3F) | (_samplePtr->_scalingLevel << 6);
	write2(8, 0x40 + _outputReg, val);
}

void ASound::updateFNumber() {
	int loReg = 0xA0 + _activeChannelNumber;
	int hiReg = 0xB0 + _activeChannelNumber;
	int val1 = (_ports[hiReg] & 0x1F) << 8;
	val1 += _ports[loReg] + _activeChannelPtr->_pitchBend;
	write2(8, loReg, val1);

	int val2 = (_ports[hiReg] & 0x20) | (val1 >> 8);
	write2(8, hiReg, val2);
}

void ASound::onTimer() {
	poll();
}

void ASound::setVolume(int volume) {
	_masterVolume = volume;
	if (!volume)
		command0();
}

int ASound::command0() {
	bool isDisabled = _isDisabled;
	_isDisabled = true;

	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].reset();

	_noiseTicks1 = 0;
	_noiseTicks2 = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_noiseFreqStep1 = 0;
	_noiseFreqStep2 = 0;

	// Reset Adlib port registers
	for (int reg = 0x4F; reg >= 0x40; --reg)
		write2(8, reg, 0x3F);
	for (int reg = 0xFF; reg >= 0x60; --reg)
		write2(8, reg, 0);
	for (int reg = 0x3F; reg > 0; --reg)
		write2(8, reg, 0);
	write2(8, 1, 0x20);

	_isDisabled = isDisabled;
	return 0;
}

int ASound::command1() {
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command2() {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i)
		_channels[i].setPtr2(_nullData);
	return 0;
}

int ASound::command3() {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command4() {
	for (int i = ADLIB_CHANNEL_MIDWAY; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].setPtr2(_nullData);
	return 0;
}

int ASound::command5() {
	for (int i = 5; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command6() {
	_savedNoiseTicks1 = _noiseTicks1;
	_noiseTicks1 = 0;
	_savedNoiseTicks2 = _noiseTicks2;
	_noiseTicks2 = 0;

	channelOff(0x43);
	channelOff(0x44);
	channelOff(0x45);
	channelOff(0x4B);
	channelOff(0x4C);
	channelOff(0x4D);
	channelOff(0x53);
	channelOff(0x54);
	channelOff(0x55);

	return 0;
}

int ASound::command7() {
	channelOn(0x43, _channels[0]._volume);
	channelOn(0x44, _channels[1]._volume);
	channelOn(0x45, _channels[2]._volume);
	channelOn(0x4B, _channels[3]._volume);
	channelOn(0x4C, _channels[4]._volume);
	channelOn(0x4D, _channels[5]._volume);

	_noiseTicks1 = _savedNoiseTicks1;
	_noiseTicks2 = _savedNoiseTicks2;

	if (_savedNoiseTicks1 != _savedNoiseTicks2)
		resultCheck();

	_isDisabled = 0;
	return _savedNoiseTicks2;
}

int ASound::command8() {
	int result = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;

	return result;
}

void ASound::channelCommand(byte *&pSrc, bool &updateFlag) {
	AdlibChannel *chan = _activeChannelPtr;
	int cmdNum = 255 - *pSrc;

	switch (cmdNum) {
	case 0:
		if (!chan->_innerLoopCount) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_innerLoopPtr = chan->_pSrc;
				chan->_innerLoopCount = 0;
			} else {
				chan->_innerLoopCount = *pSrc;
				chan->_pSrc = chan->_innerLoopPtr;
			}
		} else if (--chan->_innerLoopCount) {
			chan->_pSrc = chan->_innerLoopPtr;
		} else {
			chan->_pSrc += 2;
			chan->_innerLoopPtr = chan->_pSrc;
		}
		break;

	case 1:
		if (!chan->_outerLoopCount) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_outerLoopPtr = chan->_pSrc;
				chan->_innerLoopPtr = chan->_pSrc;
				chan->_innerLoopCount = 0;
				chan->_outerLoopCount = 0;
			} else {
				chan->_outerLoopCount = *pSrc;
				chan->_pSrc = chan->_outerLoopPtr;
				chan->_innerLoopPtr = chan->_outerLoopPtr;
			}
		} else if (--chan->_outerLoopCount) {
			chan->_outerLoopPtr = chan->_pSrc;
			chan->_innerLoopPtr = chan->_pSrc;
		} else {
			chan->_pSrc += 2;
			chan->_outerLoopPtr = chan->_pSrc;
			chan->_innerLoopPtr = chan->_pSrc;
		}
		break;

	case 2:
		// Loop sound data
		chan->_pitchBend = 0;
		chan->_volumeFadeStep = chan->_attenFadeStep = 0;
		chan->_volume = chan->_noteOffset = 0;
		chan->_transpose = chan->_volumeOffset = 0;
		chan->_keyOnDelay = 0;
		chan->_volumeFadeCounter = 0;
		chan->_attenFadeCounter = 0;
		chan->_innerLoopCount = 0;
		chan->_outerLoopCount = 0;
		chan->_patchAttenuation = 0x40;
		chan->_ptr1 = chan->_soundData;
		chan->_pSrc = chan->_soundData;
		chan->_innerLoopPtr = chan->_soundData;
		chan->_outerLoopPtr = chan->_soundData;

		chan->_pSrc += 2;
		break;

	case 3:
		chan->_sampleIndex = *++pSrc;
		chan->_pSrc += 2;
		loadSample(chan->_sampleIndex);
		break;

	case 4:
		chan->_noteOffset = *++pSrc;
		chan->_pSrc += 2;
		break;

	case 5:
		chan->_pitchBend = (int8)*++pSrc;
		chan->_pSrc += 2;
		break;

	case 6:
		++pSrc;
		if (chan->_pendingStop) {
			chan->_pSrc += 2;
		} else {
			chan->_volume = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 7:
		++pSrc;
		if (!chan->_pendingStop) {
			chan->_volumeFadeReload = *pSrc;
			chan->_volumeFadeStep = *++pSrc;
			chan->_volumeFadeCounter = 1;
		}

		chan->_pSrc += 3;
		break;

	case 8:
		chan->_transpose = (int8) * ++pSrc;
		chan->_pSrc += 2;
		break;

	case 9:
	{
		int v1 = *++pSrc;
		++pSrc;
		int v2 = (v1 - 1) & getRandomNumber();
		int v3 = pSrc[v2];
		int v4 = pSrc[v1];

		pSrc[v4 + v1 + 1] = v3;
		chan->_pSrc += v1 + 3;
		break;
	}

	case 10:
		++pSrc;
		if (chan->_pendingStop) {
			chan->_pSrc += 2;
		} else {
			chan->_volumeOffset = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 11:
		chan->_patchAttenuation = *++pSrc;
		updateFlag = true;
		chan->_pSrc += 2;
		break;

	case 12:
		chan->_attenFadeReload = *++pSrc;
		chan->_attenFadeStep = *++pSrc;
		chan->_attenFadeCounter = 1;
		chan->_pSrc += 2;
		break;

	case 13:
		++pSrc;
		chan->_pSrc += 2;
		break;

	case 14:
		chan->_octaveTranspose = *++pSrc;
		chan->_pSrc += 2;
		break;

	default:
		break;
	}
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
