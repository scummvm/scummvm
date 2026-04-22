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
#include "common/memstream.h"
#include "mads/core/sound.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/sound_nebular.h"

namespace Audio {
class Mixer;
}

namespace MADS {

SoundManager::SoundManager(Audio::Mixer *mixer, bool &soundFlag) : _mixer(mixer), _soundFlag(soundFlag) {
	_opl = OPL::Config::create();
	_opl->init();
}

SoundManager::~SoundManager() {
	if (_driver) {
		_driver->stop();
		delete _driver;
	}

	delete _opl;
}

void SoundManager::init(int sectionNumber) {
	assert(sectionNumber > 0 && sectionNumber < 10);

	if (_driver != nullptr)
		delete _driver;

	// Load the correct driver for the section
	loadDriver(sectionNumber);

	// Set volume for newly loaded driver
	_driver->setVolume(_masterVolume);
}

void SoundManager::closeDriver() {
	if (_driver) {
		command(0);
		setEnabled(false);
		stop();

		removeDriver();
	}
}

void SoundManager::removeDriver() {
	delete _driver;
	_driver = nullptr;
}

void SoundManager::setEnabled(bool flag) {
	_pollSoundEnabled = flag;
	_soundPollFlag = false;
}

void SoundManager::pauseNewCommands() {
	_newSoundsPaused = true;
}

void SoundManager::startQueuedCommands() {
	_newSoundsPaused = false;

	while (!_queuedCommands.empty()) {
		int commandId = _queuedCommands.pop();
		command(commandId);
	}
}

void SoundManager::setVolume(int volume) {
	_masterVolume = volume;

	if (_driver)
		_driver->setVolume(volume);
}

int SoundManager::command(int commandId, int param) {
	if (_newSoundsPaused) {
		if (_queuedCommands.size() < 8)
			_queuedCommands.push(commandId);
		return _queuedCommands.size() - 1;
	} else if (_driver) {
		// Note: I don't know any way to identify music commands versus sfx
		// commands, so if sfx is mute, then so is music
		if (_soundFlag)
			_driver->command(commandId, param);
	}

	return 0;
}

void SoundManager::stop() {
	if (_driver)
		_driver->stop();
}

void SoundManager::noise() {
	if (_driver)
		_driver->noise();
}

/*-----------------------------------------------------------------------*/

bool AdlibChannel::_channelsEnabled;

AdlibChannel::AdlibChannel() {
	_owner = nullptr;
	_activeCount = 0;
	_field1 = 0;
	_field2 = 0;
	_field3 = 0;
	_field4 = 0;
	_sampleIndex = 0;
	_volume = 0;
	_volumeOffset = 0;
	_field7 = 0;
	_field8 = 0;
	_field9 = 0;
	_fieldA = 0;
	_fieldB = 0;
	_fieldC = 0;
	_fieldD = 0;
	_fieldE = 0;
	_ptr1 = nullptr;
	_pSrc = nullptr;
	_ptr3 = nullptr;
	_ptr4 = nullptr;
	_ptrEnd = nullptr;
	_field17 = 0;
	_field19 = 0;
	_soundData = nullptr;
	_field1D = 0;
	_field1F = 0;

	_field20 = 0;
	_field26 = 0;
	_field28 = 0;
	_field2A = 0;
	_field2B = 0;
	_field2C = 0;
}

void AdlibChannel::reset() {
	_activeCount = 0;
	_field1 = 0;
	_field2 = 0;
	_field3 = 0;
}

void AdlibChannel::enable(int flag) {
	if (_activeCount) {
		_fieldE = flag;

		// WORKAROUND: Original set _soundData pointer to flag. Since this seems
		// just intended to invalidate any prior pointer, I've replaced it with
		// a simple null pointer
		_soundData = nullptr;
	}

	_channelsEnabled = true;
}

void AdlibChannel::setPtr2(byte *pData) {
	_pSrc = pData;
	_field2 = 0xFF;
	_fieldA = 1;
	_field9 = 1;
}

void AdlibChannel::load(byte *pData) {
	_ptr1 = _pSrc = _ptr3 = pData;
	_ptr4 = _soundData = pData;
	_volumeOffset = 0;
	_fieldA = 0xFF;
	_activeCount = 1;
	_fieldD = 64;
	_field1 = 0;
	_field1F = 0;
	_field2 = _field3 = 0;
	_volume = _field7 = 0;
	_field1D = 0;
	_fieldE = 0;
	_field9 = 0;
	_fieldB = 0;
	_field17 = 0;
	_field19 = 0;

	CachedDataEntry &cacheEntry = _owner->getCachedData(pData);
	_ptrEnd = cacheEntry._dataEnd;
}

void AdlibChannel::check(byte *nullPtr) {
	if (_activeCount && _fieldE) {
		if (!_volumeOffset) {
			_pSrc = nullPtr;
			_fieldE = 0;
		} else {
			_field2 = 0xFF;
			_fieldA = 4;
			if (!_field9)
				_field9 = 1;
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
	_fieldE = s.readByte();
	s.skip(1);
	_freqMask = s.readUint16LE();
	_freqBase = s.readUint16LE();
	_field14 = s.readUint16LE();
}

/*-----------------------------------------------------------------------*/

ASound::ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename, int dataOffset) {
	// Open up the appropriate sound file
	if (!_soundFile.open(filename))
		error("Could not open file - %s", filename.toString().c_str());

	// Initialize fields
	_commandParam = 0;
	_activeChannelPtr = nullptr;
	_samplePtr = nullptr;
	_frameCounter = 0;
	_isDisabled = false;
	_masterVolume = 255;
	_v1 = 0;
	_v2 = 0;
	_activeChannelNumber = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_channelNum1 = _channelNum2 = 0;
	_v7 = 0;
	_v8 = 0;
	_v9 = 0;
	_v10 = 0;
	_pollResult = 0;
	_resultFlag = 0;
	_nullData[0] = _nullData[1] = 0;
	Common::fill(&_ports[0], &_ports[256], 0);
	_stateFlag = false;
	_activeChannelReg = 0;
	_v11 = 0;
	_randomSeed = 1234;
	_amDep = _vibDep = _splitPoint = true;

	for (int i = 0; i < 11; ++i) {
		_channelData[i]._field0 = 0;
		_channelData[i]._freqMask = 0;
		_channelData[i]._freqBase = 0;
		_channelData[i]._field6 = 0;
	}

	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i]._owner = this;

	AdlibChannel::_channelsEnabled = false;

	// Store passed parameters, and setup OPL
	_dataOffset = dataOffset;
	_mixer = mixer;
	_opl = opl;

	// Initialize the Adlib
	adlibInit();

	// Reset the adlib
	command0();

	_opl->start(new Common::Functor0Mem<void, ASound>(this, &ASound::onTimer), CALLBACKS_PER_SECOND);
}

ASound::~ASound() {
	_opl->stop();

	Common::List<CachedDataEntry>::iterator i;
	for (i = _dataCache.begin(); i != _dataCache.end(); ++i)
		delete[](*i)._data;
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

	if (_v1) {
		setFrequency(_channelNum1, ((randomVal ^ 0xFFFF) & _freqMask1) + _freqBase1);
	}

	if (_v2) {
		setFrequency(_channelNum2, (randomVal & _freqMask2) + _freqBase2);
	}
}

byte *ASound::getDataPtr(int nearPtr) {
	for (auto i = _dataCache.begin(); i != _dataCache.end(); ++i) {
		CachedDataEntry &e = *i;
		int entrySize = (int)(e._dataEnd - e._data) + 1;
		if (nearPtr >= e._offset && nearPtr < e._offset + entrySize)
			return e._data + (nearPtr - e._offset);
	}
	return nullptr;
}

CachedDataEntry &ASound::getCachedData(byte *pData) {
	Common::List<CachedDataEntry>::iterator i;
	for (i = _dataCache.begin(); i != _dataCache.end(); ++i) {
		CachedDataEntry &e = *i;
		if (e._data == pData)
			return e;
	}

	error("Could not find previously loaded data");
}

void ASound::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
}

int ASound::write2(int state, int reg, int val) {
	// TODO: Original has a state parameter, not used when in Adlib mode?
	_ports[reg] = val;
	write(reg, val);
	return state;
}

void ASound::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
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

byte *ASound::loadData(int offset, int size) {
	// First scan for an existing copy
	Common::List<CachedDataEntry>::iterator i;
	for (i = _dataCache.begin(); i != _dataCache.end(); ++i) {
		CachedDataEntry &e = *i;
		if (e._offset == offset)
			return e._data;
	}

	// No existing entry found, so load up data and store as a new entry
	CachedDataEntry rec;
	rec._offset = offset;
	rec._data = new byte[size];
	rec._dataEnd = rec._data + size - 1;
	_soundFile.seek(_dataOffset + offset);
	_soundFile.read(rec._data, size);
	_dataCache.push_back(rec);

	// Return the data
	return rec._data;
}

void ASound::playSound(int offset, int size) {
	// Load the specified data block
	playSoundData(loadData(offset, size));
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
		if (_channels[i]._fieldE == 0xFF) {
			_channels[i].load(pData);
			return;
		}
	}
}

bool ASound::isSoundActive(byte *pData) {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i) {
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

	++_frameCounter;
	pollChannels();
	checkChannels();

	if (_v1 == _v2) {
		if (_resultFlag != -1) {
			_resultFlag = -1;
			_pollResult = -1;
		}
	} else {
		if (_v1) {
			_freqBase1 += _v7;
			if (!--_v1) {
				if (!_v2 || _channelNum1 != _channelNum2) {
					write2(8, 0xA0 + _channelNum1, 0);
					write2(8, 0xB0 + _channelNum1, 0);
				}
			}
		}

		if (_v2) {
			_freqBase2 += _v8;
			if (!--_v2) {
				if (!_v1 || _channelNum2 != _channelNum1) {
					write2(8, 0xA0 + _channelNum2, 0);
					write2(8, 0xB0 + _channelNum2, 0);
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
		if (chan->_field8 > 0 && --chan->_field8 == 0)
			updateOctave();

		bool updateFlag = true;
		if (--_activeChannelPtr->_activeCount <= 0) {
			for (;;) {
				byte *pSrc = chan->_pSrc;
				if (!chan->_ptr1) {
					warning("pollActiveChannel(): No data found for sound channel");
					break;
				}
				if (pSrc > chan->_ptrEnd) {
					warning("Read beyond end of loaded sound data");
					return;
				}

				if (!(*pSrc & 0x80) || (*pSrc <= (0xff - _chanCommandCount))) {
					if (updateFlag)
						updateActiveChannel();

					chan->_field4 = *pSrc++;
					chan->_activeCount = *pSrc++;
					chan->_pSrc += 2;

					if (!chan->_field4 || !chan->_activeCount) {
						updateOctave();
					} else {
						chan->_field8 = chan->_activeCount - chan->_field7;
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

		if (chan->_field1)
			updateFNumber();

		updateFlag = false;
		if (chan->_field9 || chan->_fieldB) {
			if (!--chan->_field9) {
				chan->_field9 = chan->_fieldA;
				if (chan->_field2) {
					int8 newVal = (int8)chan->_field2 + (int8)chan->_volumeOffset;
					if (newVal < 0) {
						chan->_field9 = 0;
						newVal = 0;
					} else if (newVal > 63) {
						chan->_field9 = 0;
						newVal = 63;
					}

					chan->_volumeOffset = newVal;
					updateFlag = true;
				}
			}

			if (!--chan->_fieldB) {
				chan->_fieldB = chan->_fieldC;
				if (chan->_field3) {
					chan->_fieldD = chan->_field3;
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

	if (_channelData[_activeChannelNumber]._field0) {
		if (_channelNum1 == _activeChannelNumber)
			_stateFlag = 0;
		if (_channelNum2 == _activeChannelNumber)
			_stateFlag = 1;

		if (!_stateFlag) {
			_stateFlag = 1;
			if (_v1)
				write2(8, 0xB0 + _channelNum1, _ports[0xB0 + _channelNum1] & 0xDF);

			_channelNum1 = _activeChannelNumber;
			_v1 = _channelData[_channelNum1]._field0;
			_freqMask1 = _channelData[_channelNum1]._freqMask;
			_freqBase1 = _channelData[_channelNum1]._freqBase;
			_v7 = _channelData[_channelNum1]._field6;
		} else {
			_stateFlag = 0;
			if (_v2)
				write2(8, 0xB0 + _channelNum2, _ports[0xB0 + _channelNum2] & 0xDF);

			_channelNum2 = _activeChannelNumber;
			_v2 = _channelData[_channelNum2]._field0;
			_freqMask2 = _channelData[_channelNum2]._freqMask;
			_freqBase2 = _channelData[_channelNum2]._freqBase;
			_v8 = _channelData[_channelNum2]._field6;
		}

		resultCheck();
	} else {
		int reg = 0xA0 + _activeChannelNumber;
		int vTimes = (byte)(_activeChannelPtr->_field4 + _activeChannelPtr->_field1F) / 12;
		int vOffset = (byte)(_activeChannelPtr->_field4 + _activeChannelPtr->_field1F) % 12;
		int val = _vList1[vOffset] + _activeChannelPtr->_field1D;
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
	int newVolume = CLIP(_activeChannelPtr->_volume + _activeChannelPtr->_volumeOffset, 0, 63);
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
	_v11 = outputChannels[outputIndexes[_activeChannelReg * 2]];
	processSample();

	AdlibChannelData &cd = _channelData[_activeChannelNumber];
	cd._field6 = _samplePtr->_field14;
	cd._freqBase = _samplePtr->_freqBase;
	cd._freqMask = _samplePtr->_freqMask;
	cd._field0 = _samplePtr->_fieldE;

	_samplePtr = &_samples[sampleIndex * 2 + 1];
	_v11 = outputChannels[outputIndexes[_activeChannelReg * 2 + 1]];
	processSample();
}

void ASound::processSample() {
	// Write out vib flags and split point
	write2(8, 0x40 + _v11, 0x3F);
	int depthRhythm = (_ports[0xBD] & 0x3F) | (_amDep ? 0x80 : 0) |
		(_vibDep ? 0x40 : 0);
	write2(8, 0xBD, depthRhythm);
	write2(8, 8, _splitPoint ? 0x40 : 0);

	// Write out feedback & Alg
	int val = (_samplePtr->_feedback << 1) | (1 - _samplePtr->_alg);
	write2(8, 0xC0 + _activeChannelReg, val);

	// Write out attack/decay rate
	val = (_samplePtr->_attackRate << 4) | (_samplePtr->_decayRate & 0xF);
	write2(8, 0x60 + _v11, val);

	// Write out sustain level/release rate
	val = (_samplePtr->_sustainLevel << 4) | (_samplePtr->_releaseRate & 0xF);
	write2(8, 0x80 + _v11, val);

	// Write out misc flags
	val = (_samplePtr->_ampMod ? 0x80 : 0) | (_samplePtr->_vib ? 0x40 : 0)
		| (_samplePtr->_egTyp ? 0x20 : 0) | (_samplePtr->_ksr ? 0x10 : 0)
		| (_samplePtr->_freqMultiple & 0xF);
	write2(8, 0x20 + _v11, val);

	// Write out waveform select
	write2(8, 0xE0 + _v11, _samplePtr->_waveformSelect & 3);

	// Write out total level & scaling level
	val = -((_samplePtr->_totalLevel & 0x3F) - 0x3F) | (_samplePtr->_scalingLevel << 6);
	write2(8, 0x40 + _v11, val);
}

void ASound::updateFNumber() {
	int loReg = 0xA0 + _activeChannelNumber;
	int hiReg = 0xB0 + _activeChannelNumber;
	int val1 = (_ports[hiReg] & 0x1F) << 8;
	val1 += _ports[loReg] + _activeChannelPtr->_field1;
	write2(8, loReg, val1);

	int val2 = (_ports[hiReg] & 0x20) | (val1 >> 8);
	write2(8, hiReg, val2);
}

void ASound::onTimer() {
	Common::StackLock slock(_driverMutex);
	poll();
	flush();
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

	_v1 = 0;
	_v2 = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_v7 = 0;
	_v8 = 0;

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
	_v9 = _v1;
	_v1 = 0;
	_v10 = _v2;
	_v2 = 0;

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

	_v1 = _v9;
	_v2 = _v10;

	if (_v9 != _v10)
		resultCheck();

	_isDisabled = 0;
	return _v10;
}

int ASound::command8() {
	int result = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;

	return result;
}

} // namespace MADS
