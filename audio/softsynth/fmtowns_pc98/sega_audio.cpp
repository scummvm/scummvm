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

#include "audio/softsynth/fmtowns_pc98/sega_audio.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"
#include "audio/softsynth/fmtowns_pc98/pcm_common.h"
#include "common/mutex.h"

class SegaPCMChannel final : public PCMChannel_Base {
public:
	SegaPCMChannel() : PCMChannel_Base(), _playing(false) {}
	~SegaPCMChannel() override {}

	void play(const int8 *data, uint16 dataSize, uint16 startAddress, uint16 loopStart, uint16 loopLen, uint16 pitch, uint8 pan, uint8 vol);
	void stop();
	bool isPlaying() const override;

private:
	void stopInternal() override;
	bool _playing;
};

class SegaPSG {
public:
	SegaPSG(int samplingRate, int deviceVolume);
	~SegaPSG() {}

	void write(uint8 val);

	void setMusicVolume(uint16 vol);
	void setSfxVolume(uint16 vol);
	void setSfxChanMask(int mask);

	void readBuffer(int32 *buffer, uint32 bufferSize);

private:
	struct Channel {
		Channel() : freq(0), curSample(0), counter(0), out(0) {}
		uint16 freq;
		int16 curSample;
		int16 out;
		uint16 counter;
	};

	Channel _channels[3];
	uint8 _nfb;
	uint8 _nfs;
	uint8 _nat;
	int _cr;

	int16 _attnTable[16];

	uint16 _musicVolume;
	uint16 _sfxVolume;
	int _sfxChanMask;

	const uint32 _extRate;
	const uint32 _intRate;
	uint32 _timer;
	const uint16 _deviceVolume;
	const uint8 _numChannels;
};

class SegaAudioInterfaceInternal final : public TownsPC98_FmSynth {
private:
	SegaAudioInterfaceInternal(Audio::Mixer *mixer, SegaAudioInterface *owner, SegaAudioPluginDriver *driver);
public:
	~SegaAudioInterfaceInternal();

	static SegaAudioInterfaceInternal *addNewRef(Audio::Mixer *mixer, SegaAudioInterface *owner, SegaAudioPluginDriver *driver);
	static void releaseRef(SegaAudioInterface *owner);

	bool init() override;

	void loadPCMData(uint16 address, const uint8 *data, uint16 dataLen);
	void playPCMChannel(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 pitch, uint8 pan, uint8 vol);
	void stopPCMChannel(uint8 channel);

	void psgWrite(uint8 data);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);
	// Defines the channels used as sound effect channels for the purpose of ScummVM GUI volume control.
	// The first 6 bits are 6 fm channels. The next 3 bits are psg channels. The bits that follow represent pcm channels.
	void setSoundEffectChanMask(int mask);

	Common::Mutex &mutex();
	int mixerThreadLockCounter() const;

private:
	bool assignPluginDriver(SegaAudioInterface *owner, SegaAudioPluginDriver *driver, bool externalMutexHandling = false);
	void removePluginDriver(SegaAudioInterface *owner);

	void nextTickEx(int32 *buffer, uint32 bufferSize) override;

	void timerCallbackA() override;
	void timerCallbackB() override;

	uint16 pcmCountSamples(uint16 address) const;

	int8 *_pcmBanks;

	uint16 _musicVolume;
	uint16 _sfxVolume;

	SegaPCMChannel **_pcmChan;
	SegaPSG *_psgDev;
	PCMDevice_Base *_pcmDev;

	SegaAudioPluginDriver *_drv;
	void *_drvOwner;
	bool _ready;

	static SegaAudioInterfaceInternal *_refInstance;
	static int _refCount;
};

SegaAudioInterfaceInternal *SegaAudioInterfaceInternal::_refInstance = 0;
int SegaAudioInterfaceInternal::_refCount = 0;

SegaAudioInterfaceInternal::SegaAudioInterfaceInternal(Audio::Mixer *mixer, SegaAudioInterface *owner, SegaAudioPluginDriver *driver) :TownsPC98_FmSynth(mixer, TownsPC98_FmSynth::kTypeTowns),
	_drv(driver), _drvOwner(owner),	_musicVolume(Audio::Mixer::kMaxMixerVolume), _sfxVolume(Audio::Mixer::kMaxMixerVolume), _pcmBanks(0), _pcmDev(0), _psgDev(0), _pcmChan(0), _ready(false) {
}

SegaAudioInterfaceInternal::~SegaAudioInterfaceInternal() {
	deinit();
	Common::StackLock lock(_mutex);
	_ready = false;

	if (_pcmChan) {
		for (int i = 0; i < 8; ++i)
			delete _pcmChan[i];
		delete[] _pcmChan;
	}
	delete _pcmDev;
	delete _psgDev;
	delete[] _pcmBanks;
}

SegaAudioInterfaceInternal *SegaAudioInterfaceInternal::addNewRef(Audio::Mixer *mixer, SegaAudioInterface *owner, SegaAudioPluginDriver *driver) {
	_refCount++;
	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new SegaAudioInterfaceInternal(mixer, owner, driver);
	else if (_refCount < 2 || _refInstance == 0)
		error("SegaAudioInterfaceInternal::addNewRef(): Internal reference management failure");
	else if (!_refInstance->assignPluginDriver(owner, driver))
		error("SegaAudioInterfaceInternal::addNewRef(): Plugin driver conflict");

	return _refInstance;
}

void SegaAudioInterfaceInternal::releaseRef(SegaAudioInterface *owner) {
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

bool SegaAudioInterfaceInternal::init() {
	if (_ready)
		return true;

	if (!TownsPC98_FmSynth::init())
		return false;

	_pcmBanks = new int8[0x10000];
	memset(_pcmBanks, 0, 0x10000);
	_pcmChan = new SegaPCMChannel*[8];
	_psgDev = new SegaPSG(7670454 / 72, 16);
	_pcmDev = new PCMDevice_Base(33300, 16, 8);
	for (int i = 0; i < 8; ++i) {
		_pcmChan[i] = new SegaPCMChannel();
		_pcmDev->assignChannel(i, _pcmChan[i]);
	}

	reset();

	writeReg(0, 0x26, 0xC6);
	writeReg(0, 0x25, 0x62);
	writeReg(0, 0x24, 0x00);
	writeReg(0, 0x27, 0x30);

	// Declare FM channels as music channels and PCM channels as sound effect channels.
	setSoundEffectChanMask(~0x1FF);

	_ready = true;

	return true;
}

void SegaAudioInterfaceInternal::loadPCMData(uint16 address, const uint8 *data, uint16 dataSize) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	while (dataSize--)
		_pcmBanks[address++] = (*data & 0x80) ? (*data++ & 0x7F) : -*data++;
}

void SegaAudioInterfaceInternal::playPCMChannel(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 vol) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	assert(channel < 8);
	_pcmChan[channel]->play(_pcmBanks, pcmCountSamples(dataStart << 8), dataStart << 8, loopStart, pcmCountSamples(loopStart), rate, pan, vol);
}

void SegaAudioInterfaceInternal::stopPCMChannel(uint8 channel) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	assert(channel < 8);
	_pcmChan[channel]->stop();
}

void SegaAudioInterfaceInternal::psgWrite(uint8 data) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	_psgDev->write(data);
}

void SegaAudioInterfaceInternal::setMusicVolume(int volume) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	_musicVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	_pcmDev->setMusicVolume(_musicVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void SegaAudioInterfaceInternal::setSoundEffectVolume(int volume) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	_sfxVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	_pcmDev->setSfxVolume(_sfxVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void SegaAudioInterfaceInternal::setSoundEffectChanMask(int mask) {
	if (!_ready)
		return;
	Common::StackLock lock(_mutex);
	_psgDev->setSfxChanMask((mask >> 6) & 7);
	_pcmDev->setSfxChanMask(mask >> 9);
	mask &= 0x3f;
	setVolumeChannelMasks(~mask, mask);
}

Common::Mutex &SegaAudioInterfaceInternal::mutex() {
	return _mutex;
}

int SegaAudioInterfaceInternal::mixerThreadLockCounter() const {
	return _mixerThreadLockCounter;
}

bool SegaAudioInterfaceInternal::assignPluginDriver(SegaAudioInterface *owner, SegaAudioPluginDriver *driver, bool externalMutexHandling) {
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

void SegaAudioInterfaceInternal::removePluginDriver(SegaAudioInterface *owner) {
	Common::StackLock lock(_mutex);
	if (_drvOwner == owner)
		_drv = 0;
}

void SegaAudioInterfaceInternal::nextTickEx(int32 *buffer, uint32 bufferSize) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	_pcmDev->readBuffer(buffer, bufferSize);
	_psgDev->readBuffer(buffer, bufferSize);
}

void SegaAudioInterfaceInternal::timerCallbackA() {
	if (_drv && _ready)
		_drv->timerCallbackA();
}

void SegaAudioInterfaceInternal::timerCallbackB() {
	if (_drv && _ready)
		_drv->timerCallbackB();
}

uint16 SegaAudioInterfaceInternal::pcmCountSamples(uint16 address) const {
	const int8 *start = &_pcmBanks[address];
	const int8 *end = &_pcmBanks[0xFFFF];
	const int8 *pos = start;
	for (; pos <= end; ++pos) {
		if (*pos == 0x7F)
			break;
	}
	return pos - start;
}

void SegaPCMChannel::play(const int8 *data, uint16 dataSize, uint16 startAddress, uint16 loopStart, uint16 loopLen, uint16 rate, uint8 pan, uint8 vol) {
	setData(data, (startAddress + dataSize) << 11, startAddress << 11);
	setupLoop(loopLen ? loopStart : (startAddress + dataSize), loopLen);
	setRate(rate);
	setPanPos(pan);
	setVolume(vol);
	activate();
	_playing = true;
}

void SegaPCMChannel::stop() {
	stopInternal();
}

bool SegaPCMChannel::isPlaying() const {
	return _playing;
}

void SegaPCMChannel::stopInternal() {
	_playing = false;
}

SegaPSG::SegaPSG(int samplingRate, int deviceVolume) : _intRate(3579545), _extRate(samplingRate), _deviceVolume(deviceVolume), _numChannels(3), _cr(-1),
	_musicVolume(Audio::Mixer::kMaxMixerVolume), _sfxVolume(Audio::Mixer::kMaxMixerVolume), _sfxChanMask(0), _nfb(0), _nfs(0), _timer(0) {
	memset(_attnTable, 0, sizeof(_attnTable));
	for (int i = 0; i < 15; ++i)
		_attnTable[i] =  (32767.0 / (double)(_numChannels + 1)) / pow(2.0, (double)(i << 1) / 6.0);
}

void SegaPSG::write(uint8 val) {
	if (val & 0x80) {
		uint8 reg = (val >> 4) & 7;
		val &= 0x0F;
		_cr = -1;
		// The noise generator is not implemented, since we don't have a single test case for it.
		if (reg == 7) {
			_nat = val;
		} else if (reg & 1) {
			_channels[reg >> 1].curSample = _attnTable[val];
		} else if (reg == 6) {
			_nfb = val >> 2;
			_nfs = val & 3;
		} else {
			_channels[reg >> 1].freq = (_channels[reg >> 1].freq & 0x3F0) | val;
			_cr = reg >> 1;
		}
	} else if (_cr != -1) {
		_channels[_cr].freq = (_channels[_cr].freq & 0x0F) | (val << 4);
	}
}

void SegaPSG::setMusicVolume(uint16 vol) {
	_musicVolume = vol;
}

void SegaPSG::setSfxVolume(uint16 vol) {
	_sfxVolume = vol;
}

void SegaPSG::setSfxChanMask(int mask) {
	_sfxChanMask = mask;
}

void SegaPSG::readBuffer(int32 *buffer, uint32 bufferSize) {
	while (bufferSize--) {
		_timer += _intRate;
		while (_timer >= _extRate) {
			_timer -= _extRate;
			// The noise generator is not implemented, since we don't have a single test case for it.
			for (int i = 0; i < _numChannels; ++i) {
				Channel *c = &_channels[i];
				if (c->counter)
					c->counter--;
				if (!c->counter) {
					c->counter = c->freq << 4;
					c->out = c->curSample;
					c->curSample = ~c->curSample;
					if (c->curSample < 0)
						c->curSample++;
				}
			}
		}

		int32 smp = 0;
		for (int i = 0; i < _numChannels; ++i)
			smp += ((_channels[i].out * (((1 << i) & _sfxChanMask) ? _sfxVolume : _musicVolume)) / Audio::Mixer::kMaxMixerVolume);

		smp = (smp * _deviceVolume) >> 7;
		*buffer++ += smp;
		*buffer++ += smp;
	}
}

SegaAudioInterface::SegaAudioInterface(Audio::Mixer *mixer, SegaAudioPluginDriver *driver) {
	_internal = SegaAudioInterfaceInternal::addNewRef(mixer, this, driver);
}

SegaAudioInterface::~SegaAudioInterface() {
	SegaAudioInterfaceInternal::releaseRef(this);
	_internal = 0;
}

bool SegaAudioInterface::init() {
	return _internal->init();
}

void SegaAudioInterface::reset() {
	_internal->reset();
}

void SegaAudioInterface::loadPCMData(uint16 address, const uint8 *data, uint16 dataSize) {
	_internal->loadPCMData(address, data, dataSize);
}

void SegaAudioInterface::playPCMChannel(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 vol) {
	_internal->playPCMChannel(channel, dataStart, loopStart, rate, pan, vol);
}

void SegaAudioInterface::stopPCMChannel(uint8 channel) {
	_internal->stopPCMChannel(channel);
}

void SegaAudioInterface::writeReg(uint8 part, uint8 regAddress, uint8 value) {
	_internal->writeReg(part, regAddress, value);
}

uint8 SegaAudioInterface::readReg(uint8 part, uint8 regAddress) {
	return _internal->readReg(part, regAddress);
}

void SegaAudioInterface::psgWrite(uint8 data) {
	_internal->psgWrite(data);
}

void SegaAudioInterface::setMusicVolume(int volume) {
	_internal->setMusicVolume(volume);
}

void SegaAudioInterface::setSoundEffectVolume(int volume) {
	_internal->setSoundEffectVolume(volume);
}

void SegaAudioInterface::setSoundEffectChanMask(int mask) {
	_internal->setSoundEffectChanMask(mask);
}

SegaAudioInterface::MutexLock SegaAudioInterface::stackLockMutex() {
	return MutexLock(_internal);
}

SegaAudioInterface::MutexLock SegaAudioInterface::stackUnlockMutex() {
	return MutexLock(_internal, _internal->mixerThreadLockCounter());
}

SegaAudioInterface::MutexLock::MutexLock(SegaAudioInterfaceInternal *saii, int reverse) : _saii(saii), _count(reverse) {
	if (!_saii)
		return;

	if (!reverse) {
		_saii->mutex().lock();
		return;
	}

	while (reverse--)
		_saii->mutex().unlock();
}

SegaAudioInterface::MutexLock::~MutexLock() {
	if (!_saii)
		return;

	if (!_count)
		_saii->mutex().unlock();

	while (_count--)
		_saii->mutex().lock();
}
