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

#include "audio/softsynth/fmtowns_pc98/pc98_audio.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"
#include "common/mutex.h"

class PC98AudioCoreInternal final : public TownsPC98_FmSynth {
private:
	PC98AudioCoreInternal(Audio::Mixer *mixer, PC98AudioCore *owner, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type);
public:
	~PC98AudioCoreInternal();

	static PC98AudioCoreInternal *addNewRef(Audio::Mixer *mixer, PC98AudioCore *owner, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type);
	static void releaseRef(PC98AudioCore *owner);

	bool init();

	void writePort(uint16 port, uint8 value);
	uint8 readPort(uint16 port);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);
	// Defines the channels used as sound effect channels for the purpose of ScummVM GUI volume control.
	// The first 6 bits are 6 fm channels. The next 3 bits are ssg channels. The next bit is the rhythm channel.
	void setSoundEffectChanMask(int mask);

	void ssgSetVolume(int volume);

	Common::Mutex &mutex();
	int mixerThreadLockCounter() const;

private:
	bool assignPluginDriver(PC98AudioCore *owner, PC98AudioPluginDriver *driver, bool externalMutexHandling = false);
	void removePluginDriver(PC98AudioCore *owner);

	void timerCallbackA();
	void timerCallbackB();

	uint16 _musicVolume;
	uint16 _sfxVolume;

	const uint16 _port1, _port2, _port3, _port4;
	uint8 _address[2];

	PC98AudioPluginDriver *_drv;
	void *_drvOwner;
	bool _ready;

	static PC98AudioCoreInternal *_refInstance;
	static int _refCount;
};

PC98AudioCoreInternal::PC98AudioCoreInternal(Audio::Mixer *mixer, PC98AudioCore *owner, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type) :
	TownsPC98_FmSynth(mixer, (TownsPC98_FmSynth::EmuType)type),
	_drv(driver), _drvOwner(owner),
	_musicVolume(Audio::Mixer::kMaxMixerVolume), _sfxVolume(Audio::Mixer::kMaxMixerVolume),
	_port1(type == PC98AudioPluginDriver::kTypeTowns ? 0x4D8 : 0x188), _port2(type == PC98AudioPluginDriver::kTypeTowns ? 0x4DA : 0x18A),
	_port3(type == PC98AudioPluginDriver::kTypeTowns ? 0x4DC : 0x18C), _port4(type == PC98AudioPluginDriver::kTypeTowns ? 0x4DE : 0x18E),
	_ready(false) {
		_address[0] = _address[1] = 0xFF;
}

PC98AudioCoreInternal::~PC98AudioCoreInternal() {
	deinit();
	Common::StackLock lock(_mutex);
	_ready = false;

	/*

	*/
}

PC98AudioCoreInternal *PC98AudioCoreInternal::addNewRef(Audio::Mixer *mixer, PC98AudioCore *owner, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type) {
	_refCount++;
	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new PC98AudioCoreInternal(mixer, owner, driver, type);
	else if (_refCount < 2 || _refInstance == 0)
		error("PC98AudioCoreInternal::addNewRef(): Internal reference management failure");
	else if (!_refInstance->assignPluginDriver(owner, driver))
		error("PC98AudioCoreInternal::addNewRef(): Plugin driver conflict");

	return _refInstance;
}

void PC98AudioCoreInternal::releaseRef(PC98AudioCore *owner) {
	if (!_refCount)
		return;

	_refCount--;

	if (_refCount) {
		if (_refInstance)
			_refInstance->removePluginDriver(owner);
	} else {
		delete _refInstance;
		_refInstance = 0;
	}
}

bool PC98AudioCoreInternal::init() {
	if (_ready)
		return true;

	if (!TownsPC98_FmSynth::init())
		return false;

	reset();

	writeReg(0, 0x26, 0xDD);
	writeReg(0, 0x25, 0x01);
	writeReg(0, 0x24, 0x00);
	writeReg(0, 0x27, 0x30);

	setVolumeChannelMasks(-1, 0);
	ssgSetVolume(0x60);

	_ready = true;

	return true;
}

void PC98AudioCoreInternal::writePort(uint16 port, uint8 value) {
	Common::StackLock lock(_mutex);
	if (port == _port1)
		_address[0] = value;
	else if (port == _port2 && _address[0] < 0xc0) {
		writeReg(0, _address[0], value);
		_address[0] = 0xFF;
	} else if (port == _port3)
		_address[1] = value;
	else if (port == _port4 && _address[1] < 0xc0) {
		writeReg(1, _address[1], value);
		_address[1] = 0xFF;
	}
}

uint8 PC98AudioCoreInternal::readPort(uint16 port) {
	Common::StackLock lock(_mutex);
	uint8 val = 0;
	if (port == _port2 && _address[0] < 0xc0) {
		val = readReg(0, _address[0]);
		_address[0] = 0xFF;
	} else if (port == _port4 && _address[1] < 0xc0) {
		val = readReg(1, _address[1]);
		_address[1] = 0xFF;
	}
	return val;
}

void PC98AudioCoreInternal::setMusicVolume(int volume) {
	Common::StackLock lock(_mutex);
	_musicVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void PC98AudioCoreInternal::setSoundEffectVolume(int volume) {
	Common::StackLock lock(_mutex);
	_sfxVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void PC98AudioCoreInternal::setSoundEffectChanMask(int mask) {
	Common::StackLock lock(_mutex);
	setVolumeChannelMasks(~mask, mask);
}

void PC98AudioCoreInternal::ssgSetVolume(int volume) {
	Common::StackLock lock(_mutex);
	setLevelSSG(volume);
}

Common::Mutex &PC98AudioCoreInternal::mutex() {
	return _mutex;
}

int PC98AudioCoreInternal::mixerThreadLockCounter() const {
	return _mixerThreadLockCounter;
}

bool PC98AudioCoreInternal::assignPluginDriver(PC98AudioCore *owner, PC98AudioPluginDriver *driver, bool externalMutexHandling) {
	Common::StackLock lock(_mutex);
	if (_refCount <= 1)
		return true;

	if (_drv) {
		if (driver && driver != _drv)
			return false;
	} else {
		_drv = driver;
		_drvOwner = owner;
	}

	return true;
}

void PC98AudioCoreInternal::removePluginDriver(PC98AudioCore *owner) {
	Common::StackLock lock(_mutex);
	if (_drvOwner == owner)
		_drv = 0;
}

void PC98AudioCoreInternal::timerCallbackA() {
	if (_drv && _ready)
		_drv->timerCallbackA();
}

void PC98AudioCoreInternal::timerCallbackB() {
	if (_drv && _ready)
		_drv->timerCallbackB();
}

PC98AudioCoreInternal *PC98AudioCoreInternal::_refInstance = 0;

int PC98AudioCoreInternal::_refCount = 0;

PC98AudioCore::PC98AudioCore(Audio::Mixer *mixer, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type) {
	_internal = PC98AudioCoreInternal::addNewRef(mixer, this, driver, type);
}

PC98AudioCore::~PC98AudioCore() {
	PC98AudioCoreInternal::releaseRef(this);
	_internal = 0;
}

bool PC98AudioCore::init() {
	return _internal->init();
}

void PC98AudioCore::reset() {
	_internal->reset();
}

void PC98AudioCore::writeReg(uint8 part, uint8 regAddress, uint8 value) {
	_internal->writeReg(part, regAddress, value);
}

uint8 PC98AudioCore::readReg(uint8 part, uint8 regAddress) {
	return _internal->readReg(part, regAddress);
}

void PC98AudioCore::writePort(uint16 port, uint8 value) {
	_internal->writePort(port, value);
}

uint8 PC98AudioCore::readPort(uint16 port) {
	return _internal->readPort(port);
}

void PC98AudioCore::setMusicVolume(int volume) {
	_internal->setMusicVolume(volume);
}

void PC98AudioCore::setSoundEffectVolume(int volume) {
	_internal->setSoundEffectVolume(volume);
}

void PC98AudioCore::setSoundEffectChanMask(int mask) {
	_internal->setSoundEffectChanMask(mask);
}

void PC98AudioCore::ssgSetVolume(int volume) {
	_internal->ssgSetVolume(volume);
}

PC98AudioCore::MutexLock PC98AudioCore::stackLockMutex() {
	return MutexLock(_internal);
}

PC98AudioCore::MutexLock PC98AudioCore::stackUnlockMutex() {
	return MutexLock(_internal, _internal->mixerThreadLockCounter());
}

PC98AudioCore::MutexLock::MutexLock(PC98AudioCoreInternal *pc98int, int reverse) : _pc98int(pc98int), _count(reverse) {
	if (!_pc98int)
		return;

	if (!reverse) {
		_pc98int->mutex().lock();
		return;
	}

	while (reverse--)
		_pc98int->mutex().unlock();
}

PC98AudioCore::MutexLock::~MutexLock() {
	if (!_pc98int)
		return;

	if (!_count)
		_pc98int->mutex().unlock();

	while (_count--)
		_pc98int->mutex().lock();
}
