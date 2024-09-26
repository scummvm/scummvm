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


#include "scumm/players/player_mac_new.h"
#include "scumm/players/player_mac_intern.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/endian.h"

namespace Scumm {

#define ASC_DEVICE_RATE		0x56EE8BA3
#define PCM_BUFFER_SIZE		1024

extern const uint8 *g_pv2ModTbl;
extern const uint32 g_pv2ModTblSize;

class LegacyMusicDriver : public MacSoundDriver {
public:
	LegacyMusicDriver(uint16 numChannels, Common::Mutex &mutex, bool canInterpolate, bool internal16Bit) :
		MacSoundDriver(mutex, ASC_DEVICE_RATE, internal16Bit ? numChannels : 1, canInterpolate, internal16Bit),
			_numChan(numChannels), _sndType(Audio::Mixer::kMusicSoundType) {}
	virtual void start() = 0;
	virtual void stop() = 0;

	enum SendDataType {
		kDuration = 0,
		kChanRate = 1,
		kChanWaveform = 2,
		kSquareWaveTriplet = 3
	};

	virtual void send(int dataType, ...) = 0;
	uint16 numChannels() const { return _numChan; }

protected:
	void putSample(int8 *&dst, int16 smp, bool expectStereo) {
		if (_smpSize == 2) {
			smp = CLIP<int16>(smp, _smpMin, _smpMax);
			*reinterpret_cast<int16*>(dst) += smp;
			dst += _smpSize;
			if (expectStereo) {
				*reinterpret_cast<int16*>(dst) += smp;
				dst += _smpSize;
			}
		} else {
			smp = CLIP<int16>(smp / _numChan, _smpMin, _smpMax);
			*dst++ += smp;
			if (expectStereo)
				*dst++ += smp;
		}
	}
	const uint16 _numChan;
	const Audio::Mixer::SoundType _sndType;
};

class FourToneSynthDriver final : public LegacyMusicDriver {
public:
	FourToneSynthDriver(Common::Mutex &mutex, bool internal16bit);
	~FourToneSynthDriver() override;

	void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) override;
	void start() override;
	void stop() override;
	void send(int dataType, ...) override;

private:
	void setWaveForm(uint8 chan, const uint8 *data, uint32 dataSize);
	void setDuration(uint16 duration);
	void setRate(uint8 chan, uint16 rate);

	uint32 _pos;
	uint16 _duration;

	struct Channel {
		Channel() : rate(0), phase(0), waveform(nullptr) {}
		uint32 rate;
		uint32 phase;
		const int8 *waveform;
	};

	Channel *_chan;
};

class SquareWaveSynthDriver final : public LegacyMusicDriver {
public:
	SquareWaveSynthDriver(Common::Mutex &mutex, bool internal16Bit);
	~SquareWaveSynthDriver() override {};

	void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) override;
	void start() override;
	void stop() override;
	void send(int dataType, ...) override;
private:
	void addTriplet(uint16 frequency, uint16 amplitude);

	struct Triplet {
		Triplet(uint16 cnt, uint16 ampl, uint16 dur) : count(cnt), amplitude(ampl), duration(dur) {}
		Triplet() : Triplet(0xffff, 0xffff, 1) {}
		Triplet &&fromScumm() { count = (count / 3) * 2; duration = MAX<uint8>(duration, 1); return Common::move(*this); }
		void toHardware(uint32 &dstCount, uint8 &dstAmpl, uint16 &dstDur) { dstCount = count ? 0x58000000 / (5 * count) : 0; dstAmpl = (amplitude & 0xff) >> 1; dstDur = duration; }
		uint16 count;
		uint16 amplitude;
		uint16 duration;
	};

	Common::Array<SquareWaveSynthDriver::Triplet> _tripletsQueue;
	Triplet _lastPara;
	uint16 _pos;
	uint32 _count;
	uint16 _duration;
	uint8 _amplitude;
	uint32 _phase;
};

FourToneSynthDriver::FourToneSynthDriver(Common::Mutex &mutex, bool internal16Bit) : LegacyMusicDriver(4, mutex, false, internal16Bit), _duration(0), _pos(0), _chan(nullptr) {
	_chan = new Channel[_numChan];
}

FourToneSynthDriver::~FourToneSynthDriver() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numChan; ++i)
		setWaveForm(i, 0, 0);
	delete[] _chan;
}

void FourToneSynthDriver::feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr || type != _sndType)
		return;

	const int8 *end = &dst[byteSize];

	while (_duration && dst < end) {
		if (_pos == 0)
			--_duration;

		int16 smp = 0;
		for (int i = 0; i < _numChan; ++i) {
			_chan[i].phase += _chan[i].rate;
			if (_chan[i].waveform != nullptr)
				smp += _chan[i].waveform[(_chan[i].phase >> 16) & 0xff];
		}

		putSample(dst, smp, expectStereo);

		if (++_pos == 370) {
			_pos = 0;
			if (!_duration)
				setFlags(kStatusDone);
		}
	}
}

void FourToneSynthDriver::start() {
	Common::StackLock lock(_mutex);
	stop();
	setDuration(50);
}

void FourToneSynthDriver::stop() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numChan; ++i) {
		_chan[i].phase = 0;
		_chan[i].rate = 0;
	}
	setDuration(0);
}

void FourToneSynthDriver::send(int dataType, ...)  {
	Common::StackLock lock(_mutex);
	va_list arg;
	va_start(arg, dataType);
	int chan = (dataType == kChanRate || dataType == kChanWaveform) ? va_arg(arg, int) : 0;
	const uint8 *ptr = (dataType == kChanWaveform) ? va_arg(arg, const uint8*) : nullptr;

	switch (dataType) {
	case kDuration:
		setDuration((uint16)va_arg(arg, uint));
		break;
	case kChanRate:
		setRate(chan, (uint16)va_arg(arg, uint));
		break;
	case kChanWaveform:
		setWaveForm(chan, ptr, va_arg(arg, uint32));
		break;
	default:
		break;
	}

	setFlags(kStatusOverflow);

	va_end(arg);
}

void FourToneSynthDriver::setWaveForm(uint8 chan, const uint8 *data, uint32 dataSize) {
	assert(chan < _numChan);
	delete[] _chan[chan].waveform;
	_chan[chan].waveform = nullptr;
	if (data == nullptr || dataSize == 0)
		return;
	dataSize = MIN<uint32>(256, dataSize);
	int8 *wf = new int8[256];
	memset(wf, 0, 256);
	for (uint32 i = 0; i < dataSize; ++i)
		wf[i] = data[i] ^ 0x80;
	_chan[chan].waveform = wf;
}

void FourToneSynthDriver::setDuration(uint16 duration) {
	_duration = duration;
	_pos = 0;
	clearFlags(kStatusDone);
}

void FourToneSynthDriver::setRate(uint8 chan, uint16 rate) {
	assert(chan < _numChan);
	_chan[chan].rate = rate ? (0x5060000 / (rate >> ((rate < 1600) ? 4 : 6))) : 0;
}

SquareWaveSynthDriver::SquareWaveSynthDriver(Common::Mutex &mutex, bool internal16Bit) :
	LegacyMusicDriver(1, mutex, false, internal16Bit), _count(0xffff), _duration(0), _amplitude(0), _phase(0), _pos(0) {
}

void SquareWaveSynthDriver::feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr || type != _sndType)
		return;

	Common::Array<Triplet>::iterator t = _tripletsQueue.begin();
	const int8 *end = &dst[byteSize];

	while (dst < end && (_count != 0xffff || t != _tripletsQueue.end())) {
		if (_pos == 0 && (_count == 0xffff || (_duration && !--_duration) || !_duration)) {
			if (t == _tripletsQueue.end()) {
				_count = 0xffff;
				_duration = 1;
				setFlags(kStatusDone);
				break;
			} else {
				t->toHardware(_count, _amplitude, _duration);
				t = _tripletsQueue.erase(t);
				_phase = _count;
			}
		}

		_phase += _count;
		putSample(dst, _phase ? -(int8)((_phase >> 23) & 1) ^ _amplitude : 0, expectStereo);

		if (++_pos == 370)
			_pos = 0;
	}
}

void SquareWaveSynthDriver::start() {
	Common::StackLock lock(_mutex);
	stop();
	setFlags(kStatusPlaying | kStatusStartup);
}

void SquareWaveSynthDriver::stop() {
	Common::StackLock lock(_mutex);
	_lastPara = Triplet();
	_count = 0xffff;
	_duration = 1;
	_tripletsQueue.clear();
	clearFlags(kStatusPlaying);
	setFlags(kStatusDone);
}

void SquareWaveSynthDriver::send(int dataType, ...)  {
	Common::StackLock lock(_mutex);
	va_list arg;
	va_start(arg, dataType);
	uint16 a = (uint16)va_arg(arg, uint);

	switch (dataType) {
	case kSquareWaveTriplet:
		addTriplet(a, (uint16)va_arg(arg, uint));
		break;
	default:
		break;
	}

	va_end(arg);
}

void SquareWaveSynthDriver::addTriplet(uint16 frequency, uint16 amplitude) {
	if ((getStatus().flags & kStatusStartup) && frequency < 3)
		return;

	clearFlags(kStatusStartup);

	if (_lastPara.count == 0xffff)
		_lastPara.count = frequency;
	if (_lastPara.amplitude == 0xffff)
		_lastPara.amplitude = amplitude;

	if ((getStatus().flags & kStatusPlaying) && _tripletsQueue.size() < 176) {
		if (frequency >> 3 != _lastPara.count >> 3 || amplitude != _lastPara.amplitude) {
			_tripletsQueue.push_back(_lastPara.fromScumm());
			_lastPara = Triplet(frequency, amplitude, 0);
			clearFlags(kStatusDone);
		}
		_lastPara.duration++;
	}

	if (!(getStatus().flags & kStatusPlaying) || _tripletsQueue.size() >= 176)
		setFlags(kStatusOverflow);
}

Common::WeakPtr<Indy3MacSnd> *Indy3MacSnd::_inst = nullptr;

Indy3MacSnd::Indy3MacSnd(ScummEngine *vm, Audio::Mixer *mixer) : VblTaskClientDriver(),
	_vm(vm), _mixer(mixer), _musicChannels(nullptr), _curSound(0), _curSong(0), _lastSoundEffectPrio(0), _idRangeMax(86), _soundEffectNumLoops(-1),
	_musicIDTable(nullptr), _macstr(nullptr), _musicIDTableLen(0), _soundUsage(0), _mdrv(nullptr), _sdrv(nullptr), _nextTickProc(this, &VblTaskClientDriver::vblCallback),
	_soundEffectPlaying(false), _songTimer(0), _songTimerInternal(0), _qmode(0), _16bit(false), _qualHi(false),	_mixerThread(false), _activeChanCount(0),
	_songUnfinished(false), _numMusicChannels(8), _numMusicTracks(4), _sfxChan(0) {
	assert(_vm);
	assert(_mixer);

	if (_vm->_game.id == GID_INDY3) {
		static const byte table[] = { 0x1D, 0x23, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x36, 0x3B, 0x42, 0x43, 0x45, 0x46, 0x53 };
		_musicIDTable = table;
		_musicIDTableLen = ARRAYSIZE(table);
	}

	_soundUsage = new uint8[_idRangeMax]();

	_musicChannels = new MusicChannel*[_numMusicChannels];
	assert(_musicChannels);
	for (int i = 0; i < _numMusicChannels; ++i)
		_musicChannels[i] = new MusicChannel(this);
}

Indy3MacSnd::~Indy3MacSnd() {
	_mixer->stopHandle(_soundHandle);
	delete _macstr;
	delete[] _soundUsage;

	_sdrv->disposeChannel(_sfxChan);

	for (Common::Array<MacSoundDriver*>::const_iterator i = _drivers.begin(); i != _drivers.end(); ++i)
		delete *i;
	_drivers.clear();

	if (_musicChannels) {
		for (int i = 0; i < _numMusicChannels; ++i)
			delete _musicChannels[i];
		delete[] _musicChannels;
	}

	delete _inst;
	_inst = nullptr;
}

Common::SharedPtr<Indy3MacSnd> Indy3MacSnd::open(ScummEngine *vm, Audio::Mixer *mixer) {
	Common::SharedPtr<Indy3MacSnd> scp = nullptr;

	if (_inst == nullptr) {
		scp = Common::SharedPtr<Indy3MacSnd>(new Indy3MacSnd(vm, mixer));
		_inst = new Common::WeakPtr<Indy3MacSnd>(scp);
		// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
		// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
		if ((_inst == nullptr) || (mixer == nullptr) || !(scp->startDevices(mixer->getOutputRate(), mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, PCM_BUFFER_SIZE, true, false, true)))
			error("Indy3MacSnd::open(): Failed to start player");
	}

	return _inst->lock();
}

bool Indy3MacSnd::startDevices(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation, bool stereo, bool internal16Bit) {
	_macstr = new MacPlayerAudioStream(this, outputRate, stereo, enableInterpolation, internal16Bit);
	if (!_macstr || !_mixer)
		return false;

	_sdrv = new MacLowLevelPCMDriver(_mixer->mutex(), pcmDeviceRate, internal16Bit);
	FourToneSynthDriver *mdrv = new FourToneSynthDriver(_mixer->mutex(), internal16Bit);
	if (!mdrv || !_sdrv)
		return false;

	for (int i = 0; i < mdrv->numChannels(); ++i)
		mdrv->send(LegacyMusicDriver::kChanWaveform, i, _fourToneSynthWaveForm, sizeof(_fourToneSynthWaveForm));
	_qualHi = true;
	_16bit = internal16Bit;
	_mdrv = mdrv;

	_sfxChan = _sdrv->createChannel(Audio::Mixer::kSFXSoundType, MacLowLevelPCMDriver::kSampledSynth, 0, nullptr);

	_drivers.push_back(_mdrv);
	_drivers.push_back(_sdrv);

	_macstr->initDrivers();
	_macstr->initBuffers(feedBufferSize);
	_macstr->setVblCallback(&_nextTickProc);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void Indy3MacSnd::setMusicVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kMusicSoundType, vol);
}

void Indy3MacSnd::setSfxVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kSFXSoundType, vol);
}

void Indy3MacSnd::startSound(int id) {
	if (id < 0 || id >= _idRangeMax)
		return;

	if (isSong(id))
		startSong(id);
	else
		startSoundEffect(id);
}

void Indy3MacSnd::stopSound(int id) {
	if (id < 0 || id >= _idRangeMax) {
		warning("Indy3MacSnd::stopSound(): sound id '%d' out of range (0 - 85)", id);
		return;
	}

	Common::StackLock lock(_mixer->mutex());
	_soundUsage[id] = 0;

	if (id == _curSound)
		stopActiveSound();
}

void Indy3MacSnd::stopAllSounds() {
	Common::StackLock lock(_mixer->mutex());
	memset(_soundUsage, 0, _idRangeMax);
	stopActiveSound();
}

int Indy3MacSnd::getMusicTimer() {
	Common::StackLock lock(_mixer->mutex());
	return _songTimer;
}

int Indy3MacSnd::getSoundStatus(int id) const {
	if (id < 0 || id >= _idRangeMax) {
		warning("Indy3MacSnd::getSoundStatus(): sound id '%d' out of range (0 - 85)", id);
		return 0;
	}
	Common::StackLock lock(_mixer->mutex());
	return _soundUsage[id];
}

void Indy3MacSnd::setQuality(int qual) {
	if (qual != MacSound::kQualityAuto && qual != MacSound::kQualityLowest && qual != MacSound::kQualityHighest) {
		warning ("Indy3MacSnd::setQuality(): Indy 3 supports only the following quality settings:"
			" kQualityAuto, kQualityLowest and kQualityHighest. Setting is now changed to kQualityAuto");
		qual = MacSound::kQualityAuto;
	}

	while (_qualHi == isHiQuality()) {
		if (_qmode == qual)
			return;
		_qmode = qual;
	}

	Common::StackLock lock(_mixer->mutex());
	Common::Array<MacSoundDriver*>::iterator dr = Common::find(_drivers.begin(), _drivers.end(), _mdrv);
	delete _mdrv;
	_mdrv = nullptr;
	_qmode = qual;

	if (isHiQuality()) {
		FourToneSynthDriver *mdrv = new FourToneSynthDriver(_mixer->mutex(), _16bit);
		assert(mdrv);
		for (int i = 0; i < mdrv->numChannels(); ++i)
			mdrv->send(LegacyMusicDriver::kChanWaveform, i, _fourToneSynthWaveForm, sizeof(_fourToneSynthWaveForm));
		_mdrv = mdrv;
		_qualHi = true;
	} else {
		_mdrv = new SquareWaveSynthDriver(_mixer->mutex(), _16bit);
		_qualHi = false;
		assert(_mdrv);
	}

	if (dr != _drivers.end())
		*dr = _mdrv;
	else if (_drivers.empty())
		_drivers.push_back(_mdrv);
	else
		error("Indy3MacSnd::setQuality(): Invalid usage");

	assert(_macstr);
	_macstr->initDrivers();
}

void Indy3MacSnd::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncBytes(_soundUsage, _idRangeMax, VER(113));
}

void Indy3MacSnd::restoreAfterLoad() {
	stopActiveSound();
	for (int i = 0; i < _idRangeMax; ++i) {
		if (_soundUsage[i] && isSong(i)) {
			--_soundUsage[i];
			startSound(i);
		}
	}
}

void Indy3MacSnd::vblCallback() {
	if (_songTimerInternal++ == 29) {
		_songTimerInternal = 0;
		++_songTimer;
	}

	_mixerThread = true;

	if (!_curSong && (_sdrv->getChannelStatus(_sfxChan) & MacSoundDriver::kStatusDone))
		updateSoundEffect();
	else if (_curSong)
		updateSong();
	else if (_songUnfinished && (_mdrv->getStatus().flags & MacSoundDriver::kStatusDone))
		stopSong();

	_mixerThread = false;
}

void Indy3MacSnd::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	assert(dst);
	memset(dst, 0, len);
	for (Common::Array<MacSoundDriver*>::const_iterator i = _drivers.begin(); i != _drivers.end(); ++i)
		(*i)->feed(dst, len, type, expectStereo);
}

const MacSoundDriver::Status &Indy3MacSnd::getDriverStatus(uint8 drvID, Audio::Mixer::SoundType type) const {
	if (drvID >= _drivers.size())
		error("Indy3MacSnd::getDriverCaps(): Invalid driver id %d", drvID);
	return _drivers[drvID]->getStatus(type);
}

void Indy3MacSnd::startSong(int id) {
	if (_mdrv == nullptr || id < 0 || id >= _idRangeMax) {
		warning("Indy3MacSnd::startSong(): sound id '%d' out of range (0 - 85)", id);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	stopActiveSound();

	uint32 sz = _vm->getResourceSize(rtSound, id);
	const byte *ptr = _vm->getResourceAddress(rtSound, id);
	assert(ptr);
	byte *buff = new byte[sz];
	memcpy(buff, ptr, sz);
	Common::SharedPtr<const byte> sres(buff, Common::ArrayDeleter<const byte>());

	_songTimer = 0;
	++_soundUsage[id];
	if (_curSong > 0)
		--_soundUsage[_curSong];
	_curSong = _curSound = id;

	// This applies if the quality mode is set to kQualAuto
	// and the VAR_SOUNDCARD setting changes.
	if (_qualHi != isHiQuality())
		setQuality(_qmode);

	if (isHiQuality()) {
		_qualHi = true;
		ptr += 14;
	} else {
		_qualHi = false;
		ptr += 6;
	}

	_mdrv->start();

	_activeChanCount = 0;
	for (int i = 0; i < _numMusicTracks; ++i) {
		uint16 offs = READ_LE_UINT16(ptr);
		ptr += 2;
		if (offs)
			++_activeChanCount;
		_musicChannels[i]->start(sres, offs, _qualHi);
	}
}

void Indy3MacSnd::startSoundEffect(int id) {
	if (_sdrv == nullptr || id < 0 || id >= _idRangeMax) {
		warning("Indy3MacSnd::startSoundEffect(): sound id '%d' out of range (0 - 85)", id);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	const uint8 *ptr = _vm->getResourceAddress(rtSound, id);
	assert(ptr);

	if (READ_LE_UINT16(ptr) < 28) {
		warning("Indy3MacSnd::startSoundEffect(%d): invalid resource", id);
		return;
	}

	if (_curSong)
		return;

	uint16 prio = READ_BE_UINT16(ptr + 4);

	if (_curSound) {
		if (prio < _lastSoundEffectPrio)
			return;
		const uint8 *ptr2 = _vm->getResourceAddress(rtSound, _curSound);
		assert(ptr2);
		if (READ_BE_UINT16(ptr2 + 6) == 0)
			_soundUsage[_curSound] = 0;
	}

	stopActiveSound();
	_soundEffectPlaying = true;

	// Two-byte prio always gets through.
	_lastSoundEffectPrio = prio & 0xff;
	_soundEffectNumLoops = (int8)ptr[27];

	int offs = (READ_BE_UINT16(ptr + 14) >= READ_BE_UINT16(ptr + 12)) ? 2 : 0;
	uint16 numSamples = READ_BE_UINT16(ptr + 12 + offs);
	uint16 spos = READ_BE_UINT16(ptr + 8 + offs);
	if (spos <= 20)
		return;

	byte *buff = new byte[numSamples - 22];
	for (uint16 i = 0; i < numSamples - 22; ++i)
		buff[i] = ptr[spos + 22 + i] ^ 0x80;

	_pcmSnd.rate = 0x4E200000 / (READ_BE_UINT16(ptr + 20 + offs) >> 7);
	_pcmSnd.data = Common::SharedPtr<const byte> (buff, Common::ArrayDeleter<const byte>());
	_pcmSnd.len = numSamples - 23;
	_pcmSnd.loopst = numSamples - 2;
	_pcmSnd.loopend = numSamples - 1;
	_pcmSnd.baseFreq = 60;

	_sdrv->playSamples(_sfxChan, MacLowLevelPCMDriver::kImmediate, &_pcmSnd);

	if (READ_BE_UINT16(ptr + 6) || _soundEffectNumLoops == -1) {
		_sdrv->playSamples(_sfxChan, MacLowLevelPCMDriver::kEnqueue, &_pcmSnd);
		_sdrv->playSamples(_sfxChan, MacLowLevelPCMDriver::kEnqueue, &_pcmSnd);
		_sdrv->playSamples(_sfxChan, MacLowLevelPCMDriver::kEnqueue, &_pcmSnd);
	}

	_curSound = id;
	_soundUsage[id]++;
}

void Indy3MacSnd::stopSong() {
	Common::StackLock lock(_mixer->mutex());
	_mdrv->stop();
	finishSong();
	_curSound = 0;
}

void Indy3MacSnd::stopSoundEffect() {
	Common::StackLock lock(_mixer->mutex());
	_sdrv->quiet(_sfxChan, MacLowLevelPCMDriver::kImmediate);
	_sdrv->flush(_sfxChan, MacLowLevelPCMDriver::kImmediate);
	_soundEffectPlaying = false;
	_lastSoundEffectPrio = 0;
	_curSound = 0;
}

void Indy3MacSnd::stopActiveSound() {
	if (_soundEffectPlaying)
		stopSoundEffect();
	else if (_curSong || _songUnfinished)
		stopSong();
	_songUnfinished = false;
}

void Indy3MacSnd::finishSong() {
	if (_soundUsage[_curSong])
		--_soundUsage[_curSong];
	_curSong = 0;
	_songUnfinished = !(_mdrv->getStatus().flags & MacSoundDriver::kStatusDone);
}

void Indy3MacSnd::updateSong() {
	if (_curSong && (_qualHi || (_mdrv->getStatus().flags & MacSoundDriver::kStatusDone))) {
		_mdrv->clearFlags(MacSoundDriver::kStatusOverflow);
		while (_curSong && !(_mdrv->getStatus().flags & MacSoundDriver::kStatusOverflow)) {
			for (int i = 4; i; --i) {
				for (int ii = 0; ii < _numMusicTracks && _curSong; ++ii)
					_musicChannels[ii]->nextTick();
			}

			if (_qualHi) {
				for (int i = 0; i < _mdrv->numChannels(); ++i)
					_mdrv->send(LegacyMusicDriver::kChanRate, i, _curSong ? _musicChannels[i]->checkPeriod() : 0);
				if (_curSong)
					_mdrv->send(LegacyMusicDriver::kDuration, 10);
			} else {
				MusicChannel *ch = nullptr;
				for (int i = 0; i < _numMusicTracks && ch == nullptr && _curSong; ++i) {
					if (_musicChannels[i]->checkPeriod())
						ch = _musicChannels[i];
				}
				_mdrv->send(LegacyMusicDriver::kSquareWaveTriplet, ch ? ch->checkPeriod() : 0, 0xff);
			}
		}
	}
}

void Indy3MacSnd::updateSoundEffect() {
	_sdrv->clearChannelFlags(_sfxChan, MacSoundDriver::kStatusDone);
	bool chkRestart = false;

	if (!_soundEffectPlaying || !_curSound) {
		chkRestart = true;
	} else {
		if (_soundEffectNumLoops > 0)
			--_soundEffectNumLoops;
		if (_soundEffectNumLoops)
			_sdrv->playSamples(_sfxChan, MacLowLevelPCMDriver::kImmediate, &_pcmSnd);
		else
			--_soundUsage[_curSound];
		chkRestart = (_soundEffectNumLoops == 0);
	}

	if (chkRestart) {
		_curSound = 0;
		_lastSoundEffectPrio = 0;
		checkRestartSoundEffects();
	}
}

void Indy3MacSnd::checkRestartSoundEffects() {
	for (int i = 1; i < _idRangeMax; ++i) {
		if (!_soundUsage[i] || isSong(i))
			continue;

		const uint8 *ptr = _vm->getResourceAddress(rtSound, i);
		assert(ptr);
		if (READ_BE_UINT16(ptr + 6) == 0)
			continue;

		_soundUsage[i] = 1;
		startSoundEffect(i);
	}
}

void Indy3MacSnd::endOfTrack() {
	if (!_activeChanCount || !--_activeChanCount)
		finishSong();
}

bool Indy3MacSnd::isSong(int id) const {
	return (Common::find(_musicIDTable, &_musicIDTable[_musicIDTableLen], id) != &_musicIDTable[_musicIDTableLen]);
}

bool Indy3MacSnd::isHiQuality() const {
	return _mixerThread ? _qualHi : (_qmode == MacSound::kQualityAuto && (_vm->VAR_SOUNDCARD == 0xff || _vm->VAR(_vm->VAR_SOUNDCARD) == 11)) || (_qmode == MacSound::kQualityHighest);
}

Indy3MacSnd::MusicChannel *Indy3MacSnd::getMusicChannel(uint8 id) const {
	return (id < _numMusicChannels) ? _musicChannels[id] : 0;
}

uint16 savedOffset = 0;
Indy3MacSnd::MusicChannel *Indy3MacSnd::MusicChannel::_ctrlChan = nullptr;

Indy3MacSnd::MusicChannel::MusicChannel(Indy3MacSnd *pl) : _player(pl), _vars(nullptr), _numVars(0), _ctrlProc(nullptr),
_resSize(0), _savedOffset(savedOffset), _modShapes(g_pv2ModTbl), _modShapesTableSize(g_pv2ModTblSize) {
	static const CtrlProc ctrl[8] {
		&Indy3MacSnd::MusicChannel::ctrl_setShape,
		&Indy3MacSnd::MusicChannel::ctrl_modPara,
		&Indy3MacSnd::MusicChannel::ctrl_init,
		&Indy3MacSnd::MusicChannel::ctrl_returnFromSubroutine,
		&Indy3MacSnd::MusicChannel::ctrl_jumpToSubroutine,
		&Indy3MacSnd::MusicChannel::ctrl_initOther,
		&Indy3MacSnd::MusicChannel::ctrl_decrJumpIf,
		&Indy3MacSnd::MusicChannel::ctrl_writeVar
	};

	const uint16 *mVars[] = {
		/*  0 */	&_frameLen,			&_curPos,			&_freqCur,			&_freqIncr,			&_freqEff,
		/*  5 */	&_envPhase,			&_envRate,			&_tempo,			&_envSust,			(uint16*)&_transpose,
		/* 10 */	&_envAtt,			&_envShape,			&_envStep,			&_envStepLen,		&_modType,
		/* 15 */	&_modState,			&_modStep,			&_modSensitivity,	&_modRange,			&_localVars[0],
		/* 20 */	&_localVars[1],		&_localVars[2],		&_localVars[3],		&_localVars[4]
	};

	_ctrlProc = ctrl;
	_vars = new uint16*[ARRAYSIZE(mVars)];
	memcpy(_vars, mVars, sizeof(mVars));
	_numVars = ARRAYSIZE(mVars);
	_savedOffset = 0;
	_ctrlChan = nullptr;

	clear();
}

Indy3MacSnd::MusicChannel::~MusicChannel() {
	clear();
	delete[] _vars;
	_vars = nullptr;
	_numVars = 0;
}

void Indy3MacSnd::MusicChannel::clear() {
	for (int i = 0; i < _numVars; ++i)
		getMemberRef(i) = 0;
	_resource.reset();
	_resSize = 0;
	_hq = false;
}

void Indy3MacSnd::MusicChannel::start(Common::SharedPtr<const byte> &songRes, uint16 offset, bool hq) {
	clear();
	_resource = songRes;
	_resSize = READ_LE_UINT16(_resource.get());
	_curPos = offset;
	_frameLen = 1;
	_hq = hq;
}

void Indy3MacSnd::MusicChannel::nextTick() {
	if (!_frameLen)
		return;

	_ctrlChan = this;
	_envPhase += _envRate;
	_freqCur += _freqIncr;

	uint16 v = _modState + _modStep;
	uint16 frqAdjust = 0;

	if (v != 0) {
		if (v >= _modRange)
			v -= _modRange;
		_modState = v;
		uint16 ix = (_modType + (v >> 4)) >> 4;
		assert(ix < _modShapesTableSize);
		frqAdjust = (((_modShapes[ix] << 7) * _modSensitivity) >> 16);
	}

	_freqEff = _freqCur + frqAdjust;

	if (_envAtt && !--_envAtt) {
		_envStep = 16;
		_envStepLen = 1;
	}

	if (!--_frameLen)
		parseNextEvents();

	if (!_envStepLen || --_envStepLen)
		return;

	int ix = _envShape + (_envStep >> 2);
	assert(ix < ARRAYSIZE(_envShapes));
	const uint32 *in = &_envShapes[ix];

	for (_envStep += 4; (*in & 0xffff) == 0xffff; ++in) {
		_envPhase = *in >> 16;
		if (_envPhase == 0)
			_envRate = 0;
		_envStep += 4;
	}

	_envStepLen = *in & 0xffff;
	_envRate = *in >> 16;
}

void Indy3MacSnd::MusicChannel::parseNextEvents() {
	if (_resSize && _curPos >= _resSize) {
		warning("Indy3MacSnd::MusicChannel::parseNext(): playback error");
		_frameLen = 0;
		_curPos = 0;
		_player->stopSong();
	}

	if (_curPos == 0)
		return;

	const byte *in = _resource.get() + _curPos;

	for (bool loop = true, loop2 = false; loop; ) {
		uint8 cmd = *in++;

		if (in - _resource.get() >= _resSize)
			break;

		if (cmd >= 0xf8 && !loop2) {
			if (!ctrlProc(cmd - 0xf8, in))
				loop = false;

			if (in - _resource.get() >= _resSize)
				break;

		} else {
			loop2 = true;
			MusicChannel *ch = _player->getMusicChannel(cmd >> 5);
			setFrameLen(cmd);
			cmd = *in++;

			if (in - _resource.get() >= _resSize)
				break;

			if (ch != nullptr && ((cmd & 0x7f) != 0x7f))
				ch->noteOn(_ctrlChan->_frameLen, cmd & 0x7f);

			if (cmd & 0x80)
				loop = false;
		}
	}

	int cp = in - _resource.get();
	if ((cp >= _resSize && _frameLen) || cp & ~0xffff) {
		warning("Indy3MacSnd::MusicChannel::parseNext(): playback error");
		_frameLen = 0;
		_player->stopSong();
	}

	_curPos = _frameLen ? cp : 0;
	if (!_frameLen)
		_player->endOfTrack();
}

void Indy3MacSnd::MusicChannel::noteOn(uint16 duration, uint8 note) {
	static const uint16 noteFreqTable[2][12] = {
		{ 0xFFC0, 0xF140, 0xE3C0, 0xD700, 0xCB40, 0xBF80, 0xB4C0, 0xAA80, 0xA100, 0x9800, 0x8F80, 0x8740 },
		{ 0x8E84, 0x8684, 0x7EF7, 0x77D7, 0x714F, 0x6AC4, 0x64C6, 0x5F1E, 0x59C7, 0x54BD, 0x4FFC, 0x4B7E }
	};
	_envStep = 0;
	_envStepLen = 1;
	_frameLen = duration;
	_envAtt = _frameLen - _envSust;
	int n = note + _transpose;
	while (n < 0)
		n += 12;

	_freqEff = _freqCur = noteFreqTable[_hq ? 0 : 1][n % 12] >> (n / 12);
}

uint16 Indy3MacSnd::MusicChannel::checkPeriod() const {
	return (_frameLen && _envPhase) ? _freqEff : 0;
}

bool Indy3MacSnd::MusicChannel::ctrl_setShape(const byte *&pos) {
	static const uint16 offsets[15] = { 0, 6, 12, 18, 24, 30, 36, 44, 52, 60, 68, 82, 76, 82, 90 };
	uint8 i = (*pos++) >> 1;
	assert(i < ARRAYSIZE(offsets));
	_envShape = offsets[i];
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_modPara(const byte *&pos) {
	static const uint16 table[10] = { 0x0000, 0x1000, 0x1000, 0x1000, 0x2000, 0x0020, 0x3020, 0x2000, 0x2020, 0x1000 };
	int ix = (*pos++);
	if ((ix & 1) || ((ix >> 1) + 1 >= ARRAYSIZE(table)))
		error("Indy3MacSnd::MusicChannel::ctrl_modPara(): data error");
	ix >>= 1;
	_modType = table[ix];
	_modRange = table[ix + 1];
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_init(const byte *&pos) {
	limitedClear();
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_returnFromSubroutine(const byte *&pos) {
	pos = _resource.get() + _savedOffset;
	if (pos >= _resource.get() + _resSize)
		error("Indy3MacSnd::MusicChannel::ctrl_returnFromSubroutine(): invalid address");
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_jumpToSubroutine(const byte *&pos) {
	uint16 offs = READ_LE_UINT16(pos);
	_savedOffset = pos + 2 - _resource.get();
	if (offs >= _resSize)
		error("Indy3MacSnd::MusicChannel::ctrl_jumpToSubroutine(): invalid address");
	pos = _resource.get() + offs;
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_initOther(const byte *&pos) {
	uint16 val = READ_LE_UINT16(pos);
	pos += 2;
	if (val % 50)
		error("Indy3MacSnd::MusicChannel::ctrl_initOther(): data error");
	_ctrlChan = _player->getMusicChannel(val / 50);
	assert(_ctrlChan);
	_ctrlChan->limitedClear();
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_decrJumpIf(const byte *&pos) {
	uint16 &var = getMemberRef(*pos++ >> 1);
	int16 offs = READ_LE_INT16(pos);
	pos += 2;
	if (var == 0) {
		pos += offs;
		if (pos < _resource.get() || pos >= _resource.get() + _resSize)
			error("Indy3MacSnd::MusicChannel::ctrl_decrJumpIf(): invalid address");
	} else {
		--var;
	}
	return true;
}

bool Indy3MacSnd::MusicChannel::ctrl_writeVar(const byte *&pos) {
	byte ix = *pos++;
	uint16 val = READ_LE_UINT16(pos);
	pos += 2;
	(getMemberRef(ix >> 1)) = val;
	return (bool)ix;
}

bool Indy3MacSnd::MusicChannel::ctrlProc(int procId, const byte *&arg) {
	return (_ctrlChan && _ctrlProc && procId >= 0 && procId <= 7) ? (_ctrlChan->*_ctrlProc[procId])(arg) : false;
}

void Indy3MacSnd::MusicChannel::setFrameLen(uint8 len) {
	static const uint8 durationTicks[22] = {
		0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09,
		0x0C, 0x10, 0x12, 0x18, 0x20, 0x26, 0x30, 0x40, 0x48, 0x60, 0x00
	};

	assert(_ctrlChan);
	len &= 0x1f;
	if (len >= ARRAYSIZE(durationTicks))
		error("Indy3MacSnd::MusicChannel::setFrameLen(): Out of range (val %d, range 0 - %d)", len, ARRAYSIZE(durationTicks) - 1);
	_ctrlChan->_frameLen = MAX<uint16>(_ctrlChan->_tempo, 1) * durationTicks[len];
}

void Indy3MacSnd::MusicChannel::limitedClear() {
	for (int i = 1; i < 7; ++i)
		getMemberRef(i) = 0;
	for (int i = 8; i < 10; ++i)
		getMemberRef(i) = 0;
	for (int i = 11; i < 15; ++i)
		getMemberRef(i) = 0;
	for (int i = 15; i < 19; ++i)
		getMemberRef(i) = 0;
}

uint16 &Indy3MacSnd::MusicChannel::getMemberRef(int pos) {
	assert(_vars);
	if (pos < 0 || pos >= _numVars)
		error("Indy3MacSnd::MusicChannel::getMemberRef(): attempting invalid access (var: %d, valid range: %d - %d)", pos, 0, _numVars - 1);
	return *_vars[pos];
}

const uint32 Indy3MacSnd::MusicChannel::_envShapes[98] = {
	0x0003ffff, 0x00000000, 0x00000000, 0x00000000, 0x0000ffff, 0x00000000,
	0x0003ffff, 0x00000020, 0x0000ffff, 0x00000000, 0x0000ffff, 0x00000000,
	0x0003ffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x0003ffff, 0x00000002, 0x0000ffff, 0x00000000, 0x0000ffff, 0x00000000,
	0x0003ffff, 0x00000006, 0x0000ffff, 0x00000000, 0x0000ffff, 0x00000000,
	0x0003ffff, 0x00000010, 0x0000ffff, 0x00000000, 0x0000ffff, 0x00000000,
	0xea60ffff, 0xfc180014, 0x00000000, 0x00000000, 0x9c40ffff, 0xec780005, 0x0000ffff, 0x00000000,
	0xc350ffff, 0x00000008, 0x7530ffff, 0x00000000, 0x6d60ffff, 0xec780005, 0x0000ffff, 0x00000000,
	0xea60ffff, 0xf8300010, 0x00000000, 0x00000000, 0x6d60ffff, 0xe8900005, 0x0000ffff, 0x00000000,
	0xd6d8ffff, 0x00000008, 0x88b8ffff, 0x00000000, 0x9c40ffff, 0xf63c000a, 0x0000ffff, 0x00000000,
	0xea60ffff, 0x00000004, 0xf63c0008, 0x00000000, 0x9c40ffff, 0xe8900005, 0x0000ffff, 0x00000000,
	0x0000ffff, 0x00960154, 0xff6a0154, 0x0000ffff, 0x0000ffff, 0x00000000,
	0x4e20ffff, 0x0fa00007, 0x03e8000f, 0x00000000, 0x88b8ffff, 0xf830000f, 0x0000ffff, 0x00000000,
	0x88b8ffff, 0x01f40014, 0x00000000, 0x00000000, 0xafc8ffff, 0xfe0c003c, 0x0000ffff, 0x00000000
};

#undef ASC_DEVICE_RATE
#undef PCM_BUFFER_SIZE

} // End of namespace Scumm
