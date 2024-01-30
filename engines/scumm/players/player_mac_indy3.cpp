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


#include "scumm/players/player_mac_indy3.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/endian.h"

namespace Scumm {

#define ASC_DEVICE_RATE		0x56EE8BA3
#define PCM_BUFFER_SIZE		1024
#define RATECNV_BIT_PRECSN	24

extern const uint8 *g_pv2ModTbl;
extern const uint32 g_pv2ModTblSize;

class I3MPlayer;
class AudioStream_I3M : public Audio::AudioStream {
public:
	AudioStream_I3M(I3MPlayer *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate);
	~AudioStream_I3M() override;

	void initBuffers(uint32 feedBufferSize);
	void initDrivers();
	typedef Common::Functor0Mem<void, I3MPlayer> CallbackProc;
	void setVblCallback(const CallbackProc *proc);
	void clearBuffer();

	void setMasterVolume(Audio::Mixer::SoundType type, uint16 vol);

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return _isStereo; }
	int getRate() const override { return _outputRate; }
	bool endOfData() const override { return false; }

private:
	void generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType, bool expectStereo) const;
	void runVblTask();

	I3MPlayer *_drv;

	uint32 _vblSmpQty;
	uint32 _vblSmpQtyRem;
	uint32 _vblCountDown;
	uint32 _vblCountDownRem;
	const CallbackProc *_vblCbProc;

	struct SmpBuffer {
		SmpBuffer() : start(0), pos(0), end(0), volume(0x10000), lastL(0), lastR(0), size(0), rateConvInt(0), rateConvFrac(0), rateConvAcc(-1) {}
		int8 *start;
		int8 *pos;
		const int8 *end;
		uint32 volume;
		int32 lastL;
		int32 lastR;
		uint32 size;
		uint32 rateConvInt;
		uint32 rateConvFrac;
		int32 rateConvAcc;
	} _buffers[2];

	const uint32 _outputRate;
	const uint8 _frameSize;
	bool _interp;

	const bool _isStereo;
};

class I3MSoundDriver {
public:
	I3MSoundDriver(Common::Mutex &mutex, uint32 deviceRate, bool isStereo) : _mutex(mutex), _sig(false), _deviceRate(deviceRate), _stereo(isStereo) {}
	virtual ~I3MSoundDriver() {}
	virtual void feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) = 0;
	uint32 getDeviceRate() const { return _deviceRate; }
	bool checkSignal() const { return _sig; }
	void resetSignal(bool state) { _sig = state; }
protected:
	Common::Mutex &_mutex;
	const bool _stereo;
	const uint32 _deviceRate;
	bool _sig;
};

class I3MLowLevelPCMDriver final : public I3MSoundDriver {
public:
	struct PCMSound {
		PCMSound() : len(0), rate(0), loopst(0), loopend(0), baseFreq(0) {}
		Common::SharedPtr<const byte> data;
		uint32 len;
		uint32 rate;
		uint32 loopst;
		uint32 loopend;
		byte baseFreq;
	};
public:
	I3MLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool enableInterpolation, bool isStereo);
	void feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) override;
	void play(PCMSound *snd);
	void stop();
private:
	uint32 calcRate(uint32 outRate, uint32 factor, uint32 dataRate);

	Common::SharedPtr<const int8> _res;
	const int8 *_data;
	const uint16 _frameSize;
	const bool _interp;
	int8 _lastSmp[2];
	uint32 _len;
	uint16 _rmH;
	uint16 _rmL;
	uint32 _loopSt;
	uint32 _loopEnd;
	byte _baseFreq;
	uint32 _rcPos;
	uint32 _smpWtAcc;
};

class I3MMusicDriver : public I3MSoundDriver {
public:
	I3MMusicDriver(Common::Mutex &mutex, bool isStereo) : I3MSoundDriver(mutex, ASC_DEVICE_RATE, isStereo) {}
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void setDuration(uint16 duration) = 0;
	virtual void setRate(uint8 chan, uint16 rate) = 0;
};

class I3MFourToneSynthDriver final : public I3MMusicDriver {
public:
	I3MFourToneSynthDriver(Common::Mutex &mutex, bool isStereo);
	~I3MFourToneSynthDriver() override;

	void feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) override;
	void start() override;
	void stop() override;

	void setDuration(uint16 duration) override;
	void setWaveForm(uint8 chan, const uint8 *data, uint32 dataSize);
	void setRate(uint8 chan, uint16 rate) override;

private:
	uint32 _pos;
	uint16 _duration;

	struct Channel {
		Channel() : rate(0), phase(0), waveform(nullptr) {}
		uint32 rate;
		uint32 phase;
		const int8 *waveform;
	};

	Channel *_chan;
	const uint16 _numChan;
};

class I3MLQSynthDriver final : public I3MMusicDriver {
public:
	I3MLQSynthDriver(Common::Mutex &mutex, bool isStereo) : I3MMusicDriver(mutex, isStereo) {}
	~I3MLQSynthDriver() override {}

	void feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) override {}
	void start() override {}
	void stop() override {}

	void setDuration(uint16 duration) override {}
	void setRate(uint8 chan, uint16 rate) override {}
};

class I3MPlayer {
private:
	I3MPlayer(ScummEngine *vm, Audio::Mixer *mixer);
public:
	~I3MPlayer();
	static Common::SharedPtr<I3MPlayer> open(ScummEngine *scumm, Audio::Mixer *mixer);
	bool startDevices(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation);

	void setMusicVolume(int vol);
	void setSfxVolume(int vol);
	void startSound(int id);
	void stopSound(int id);
	void stopAllSounds();
	int getMusicTimer();
	int getSoundStatus(int id) const;
	void setQuality(int qual);

	void generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const;
	uint32 getDriverDeviceRate(uint8 drvID) const;
	void nextTick();

private:
	void startSong(int id);
	void startSoundEffect(int id);
	void stopSong();
	void stopSoundEffect();
	void stopActiveSound();
	void updateSong();
	void updateSoundEffect();

	void checkRestartSoundEffects();
	void endOfTrack();

	bool isSong(int id) const;
	bool isHiQuality() const;

	int _lastSound;
	int _lastSong;
	int _lastSoundEffectPrio;
	int _soundEffectNumLoops;
	int _songTimer;
	uint _activeChanCount;
	byte _songTimerInternal;
	byte *_soundUsage;

	bool _songPlaying;
	bool _soundEffectPlaying;
	int _qmode;
	bool _qualHi;
	bool _mixerThread;

	I3MLowLevelPCMDriver::PCMSound _pcmSnd;

	AudioStream_I3M *_macstr;
	Audio::SoundHandle _soundHandle;
	AudioStream_I3M::CallbackProc _nextTickProc;

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	static Common::WeakPtr<I3MPlayer> *_inst;

	const byte *_musicIDTable;
	int _musicIDTableLen;
	const int _idRangeMax;

	I3MMusicDriver *_mdrv;
	I3MLowLevelPCMDriver *_sdrv;
	Common::Array<I3MSoundDriver*> _drivers;

private:
	class MusicChannel {
	public:
		MusicChannel(I3MPlayer *pl);
		~MusicChannel();
		void clear();

		void start(Common::SharedPtr<const byte> &songRes, uint16 offset, bool hq);
		void nextTick();
		void parseNextEvents();
		void noteOn(uint16 duration, uint8 note);
		uint16 checkPeriod() const;

		uint16 _frameLen;
		uint16 _curPos;
		uint16 _freqCur;
		uint16 _freqIncr;
		uint16 _freqEff;
		uint16 _envPhase;
		uint16 _envRate;
		uint16 _tempo;
		uint16 _envCutoff;
		int16 _transpose;
		uint16 _envLen;
		uint16 _envShape;
		uint16 _envStep;
		uint16 _envStepLen;
		uint16 _modType;
		uint16 _modState;
		uint16 _modStep;
		uint16 _modSensitivity;
		uint16 _modRange;
		uint16 _localVars[5];
		Common::SharedPtr<const byte> _resource;
		bool _hq;

	private:
		typedef bool (I3MPlayer::MusicChannel::*CtrlProc)(const byte *&);

		bool ctrl_setShape(const byte *&pos);
		bool ctrl_modPara(const byte *&pos);
		bool ctrl_init(const byte *&pos);
		bool ctrl_returnFromSubroutine(const byte *&pos);
		bool ctrl_jumpToSubroutine(const byte *&pos);
		bool ctrl_initOther(const byte *&pos);
		bool ctrl_decrJumpIf(const byte *&pos);
		bool ctrl_writeVar(const byte *&pos);

		const CtrlProc *_ctrlProc;

		void limitedClear();
		uint16 &getMemberRef(int pos);

		uint16 **_vars;
		int _numVars;
		uint16 &_savedOffset;

		uint16 _resSize;

		I3MPlayer *_player;
		static MusicChannel *_ctrlChan;

		static const uint32 _envShapes[98];
		const uint8 *&_modShapes;
		const uint32 &_modShapesTableSize;

		bool ctrlProc(int procId, const byte *&arg);
		void setFrameLen(uint8 len);
	};

	MusicChannel **_musicChannels;
	const int _numMusicChannels;

	static const uint8 _fourToneSynthWaveForm[256];

public:
	MusicChannel *getMusicChannel(uint8 id) const;
};

AudioStream_I3M::AudioStream_I3M(I3MPlayer *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate) : Audio::AudioStream(), _drv(drv), _vblSmpQty(0), _vblSmpQtyRem(0), _frameSize(stereo ? 2 : 1),
	_vblCountDown(0), _vblCountDownRem(0), _outputRate(scummVMOutputrate), _vblCbProc(nullptr), _isStereo(stereo), _interp(interpolate) {
	assert(_drv);
	_vblSmpQty = _outputRate / 60;
	_vblSmpQtyRem = _outputRate % 60;
	_vblCountDown = _vblSmpQty;
	_vblCountDownRem = 0;
}

AudioStream_I3M::~AudioStream_I3M() {
	for (int i = 0; i < 2; ++i)
		delete[] _buffers[i].start;
}

void AudioStream_I3M::initBuffers(uint32 feedBufferSize) {
	for (int i = 0; i < 2; ++i)
		delete[] _buffers[i].start;

	for (int i = 0; i < 2; ++i) {
		_buffers[i].size = feedBufferSize;
		_buffers[i].start = new int8[_buffers[i].size];
		_buffers[i].end = _buffers[i].start + _buffers[i].size;
	}
	clearBuffer();
}

void AudioStream_I3M::initDrivers() {
	for (int i = 0; i < 2; ++i) {
		uint32 dr = _drv ? _drv->getDriverDeviceRate(i) : 0;
		if (!dr)
			error("AudioStream_I3M::initDrivers(): Failed to query device rate for device %d", i);
		uint64 irt = (uint64)dr * (1 << RATECNV_BIT_PRECSN) / _outputRate;
		_buffers[i].rateConvInt = irt >> (RATECNV_BIT_PRECSN + 16);
		_buffers[i].rateConvFrac = (irt >> 16) & ((1 << RATECNV_BIT_PRECSN) - 1);
		_buffers[i].rateConvAcc = 0;
	}
}

void AudioStream_I3M::setVblCallback(const CallbackProc *proc) {
	_vblCbProc = proc;
}

void AudioStream_I3M::clearBuffer() {
	for (int i = 0; i < 2; ++i) {
		memset(_buffers[i].start, 0, _buffers[i].size);
		_buffers[i].pos = _buffers[i].start;
	}
}

void AudioStream_I3M::setMasterVolume(Audio::Mixer::SoundType type, uint16 vol) {
	if (type == Audio::Mixer::kMusicSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[0].volume = vol * vol;
	if (type == Audio::Mixer::kSFXSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[1].volume = vol * vol;
}

int AudioStream_I3M::readBuffer(int16 *buffer, const int numSamples) {
	static const Audio::Mixer::SoundType stype[2] = {
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType
	};

	static const char errFnNames[2][8] = {"Buffers", "Drivers"};
	int errNo = (!_buffers[0].size || !_buffers[1].size) ? 0 : ((_buffers[0].rateConvAcc == -1 || _buffers[1].rateConvAcc == -1) ? 1 : -1);
	if (errNo != -1)
		error("AudioStream_I3M::readBuffer(): init%s() must be called before playback", errFnNames[errNo]);

	for (int i = _isStereo ? numSamples >> 1 : numSamples; i; --i) {
		if (!--_vblCountDown) {
			_vblCountDownRem += _vblSmpQtyRem;
			_vblCountDown = _vblSmpQty + _vblCountDownRem / _vblSmpQty;
			_vblCountDownRem %= _vblSmpQty;
			runVblTask();
		}

		int32 smpL = 0;
		int32 smpR = 0;
		for (int ii = 0; ii < 2; ++ii) {
			int diff = _buffers[ii].pos[0] - _buffers[ii].lastL;
			if (diff && _buffers[ii].rateConvAcc && _interp)
				diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
			smpL += (int32)((_buffers[ii].lastL + diff) * _buffers[ii].volume);
		}
		if (_isStereo) {
			for (int ii = 0; ii < 2; ++ii) {
				int diff = _buffers[ii].pos[1] - _buffers[ii].lastR;
				if (diff && _buffers[ii].rateConvAcc && _interp)
					diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
				smpR += (int32)((_buffers[ii].lastR + diff) * _buffers[ii].volume);
			}
		}

		for (int ii = 0; ii < 2; ++ii) {
			uint32 incr = (_buffers[ii].rateConvInt * _frameSize);
			_buffers[ii].rateConvAcc += _buffers[ii].rateConvFrac;
			if (_buffers[ii].rateConvAcc & ~((1 << RATECNV_BIT_PRECSN) - 1)) {
				incr += _frameSize;
				_buffers[ii].rateConvAcc &= ((1 << RATECNV_BIT_PRECSN) - 1);
			}

			if (incr) {
				_buffers[ii].pos += incr;
				if (_buffers[ii].pos == _buffers[ii].end) {
					_buffers[ii].pos -= _buffers[ii].size;
					generateData(_buffers[ii].pos, _buffers[ii].size, stype[ii], _isStereo);
				}

				const int8 *lpos = _buffers[ii].pos;
				if (lpos >= _buffers[ii].start + _frameSize)
					lpos -= _frameSize;
				_buffers[ii].lastL = lpos[0];
				if (_isStereo)
					_buffers[ii].lastR = lpos[1];
			}
		}

		*buffer++ = CLIP<int16>(smpL >> 8, -32768, 32767);
		if (_isStereo)
			*buffer++ = CLIP<int16>(smpR >> 8, -32768, 32767);
	}
	return numSamples;
}

void AudioStream_I3M::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	if (_drv)
		_drv->generateData(dst, len, type, expectStereo);
}

void AudioStream_I3M::runVblTask() {
	if (_vblCbProc && _vblCbProc->isValid())
		(*_vblCbProc)();
}

I3MLowLevelPCMDriver::I3MLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool enableInterpolation, bool isStereo) :
	I3MSoundDriver(mutex, deviceRate, isStereo), _interp(enableInterpolation), _frameSize(isStereo ? 2 : 1), _len(0), _rmH(0), _rmL(0), _smpWtAcc(0), _loopSt(0), _loopEnd(0), _baseFreq(0), _rcPos(0), _data(nullptr) {
		_lastSmp[0] = _lastSmp[1] = 0;
}

void I3MLowLevelPCMDriver::feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr || type != Audio::Mixer::kSFXSoundType)
		return;

	memset(dst, 0, len);

	if (_data == nullptr)
		return;

	if (expectStereo != _stereo)
		error("I3MLowLevelPCMDriver::feed(): stereo/mono mismatch between sound data and mixer stream");

	int32 diff = 0;
	bool interp = (_interp && _rmL);

	for (const int8 *end = dst + len; dst < end; ) {
		if (interp) {
			for (int i = 0; i < _frameSize; ++ i) {
				int8 in = _data[_rcPos + i];
				if (in != _lastSmp[i]) {
					diff = in - _lastSmp[i];
					diff = (diff * (_smpWtAcc >> 1)) >> 15;
					in = (_lastSmp[i] + diff) & 0xff;
				}
				*dst++ = in;
			}
		} else {
			*dst++ = _data[_rcPos];
		}

		uint32 lpos = _rcPos;
		_rcPos += (_rmH * _frameSize);
		_smpWtAcc += _rmL;
		if (_smpWtAcc > 0xffff) {
			_smpWtAcc &= 0xffff;
			_rcPos += _frameSize;
		}

		if (interp && _rcPos >= lpos + _frameSize) {
			for (int i = 0; i < _frameSize; ++i)
				_lastSmp[i] = _data[_rcPos - _frameSize + i];
		}

		if (_rcPos >= _len) {
			if (_loopSt && _loopEnd) {
				_rcPos = _loopSt + (_rcPos - _len);
				_len = _loopEnd;
				_lastSmp[0] = _data[_rcPos];
				if ((_len - _rcPos) > 1)
					_lastSmp[1] = _data[_rcPos + 1];
				_smpWtAcc = 0;
			} else {
				_data = nullptr;
				_res.reset();
				end = dst;
			}
			_sig = true;
		}
	}
}

void I3MLowLevelPCMDriver::play(PCMSound *snd) {
	if (!snd || !snd->data)
		return;

	Common::StackLock lock(_mutex);

	_res = snd->data.reinterpretCast<const int8>();
	_data = _res.get();
	_len = snd->len;
	uint32 rmul = calcRate(_deviceRate, 0x10000, snd->rate);

	if (rmul >= 0x7FFD && rmul <= 0x8003)
		rmul = 0x8000;
	else if (ABS((int16)(rmul & 0xffff)) <= 7)
		rmul = (rmul + 7) & ~0xffff;

	if (rmul > (uint32)-64)
		rmul = (uint32)-64;

	assert(rmul);

	_rmL = rmul & 0xffff;
	_rmH = rmul >> 16;

	if (snd->loopend - snd->loopst < 2 || snd->loopend < snd->loopst) {
		_loopSt = 0;
		_loopEnd = 0;
	} else {
		_loopSt = snd->loopst - (snd->loopst % _frameSize);
		_loopEnd = snd->loopend - (snd->loopend % _frameSize);
	}

	_baseFreq = snd->baseFreq;
	_rcPos = 0;
	_smpWtAcc = 0;
	_lastSmp[0] = _data[0];
	if (_len >= _frameSize)
		_lastSmp[1] = _data[1];
	_sig = false;
}

void I3MLowLevelPCMDriver::stop() {
	Common::StackLock lock(_mutex);
	_data = nullptr;
	_res.reset();
	_sig = true;
}

uint32 I3MLowLevelPCMDriver::calcRate(uint32 outRate, uint32 factor, uint32 dataRate) {
	uint32 result = outRate;
	uint64 t = 0;
	uint32 c = 0;

	if (!factor || !dataRate)
		return (uint32)-1;

	if (factor > 0x10000 && dataRate > 0x10000) {
		bool altpth = true;

		if (!(dataRate & 0xffff)) {
			SWAP(factor, dataRate);
			if (!(dataRate & 0xffff)) {
				dataRate = (dataRate >> 16) * (factor >> 16);
				factor = 0;
				altpth = false;
			}
		} else if (factor & 0xffff) {
			t = (dataRate & 0xffff) * (factor >> 16) + (dataRate >> 16) * (factor & 0xffff);
			c = (factor & 0xffff) * (dataRate & 0xffff);
			dataRate = (factor >> 16) * (dataRate >> 16) + (t >> 16);
			t = c + ((t & 0xffff) << 16);
			factor = t & (uint32)-1;
			dataRate += (t >> 32);
			altpth = false;
		}

		if (altpth) {
			c = dataRate;
			dataRate = (factor >> 16) * (dataRate >> 16);
			factor = (factor >> 16) * (c & 0xffff);
			dataRate += (factor >> 16);
			factor <<= 16;
		}

	} else if (factor < 0x10000 && dataRate < 0x10000) {
		factor = factor * dataRate;
		dataRate = 0;
	} else if (factor == 0x10000 || dataRate == 0x10000) {
		if (dataRate == 0x10000)
			SWAP(dataRate, factor);
		factor = dataRate << 16;
		dataRate = (factor | (dataRate >> 16)) ^ factor;
	} else {
		if (factor > 0x10000 && dataRate <= 0x10000)
			SWAP(dataRate, factor);

		c = (dataRate >> 16) * (factor & 0xffff);
		factor = (factor & 0xffff) * (dataRate & 0xffff);
		uint32 x = ((factor >> 16) + (c & 0xffff)) & ~0xffff;
		factor += (c << 16);
		result = (c + x) >> 16;
		dataRate = result;
	}

	t = factor + (outRate >> 1);
	factor = t & (uint32)-1;
	dataRate += (t >> 32);

	if (dataRate >= outRate)
		return (uint32)-1;

	dataRate ^= factor;

	if (outRate < 0x10000) {
		factor <<= 16;
		dataRate = (dataRate >> 16) | (dataRate << 16);
		outRate = (outRate >> 16) | (outRate << 16);
	}

	int32 sh = -1;

	if (outRate < 0x1000000) {
		outRate <<= 8;
		sh = -9;
	}

	for (t = (int32)outRate; !(t >> 32); t = (int32)outRate) {
		--sh;
		outRate += outRate;
	}

	sh = ~sh;
	if (sh) {
		factor <<= sh;
		dataRate = ((dataRate >> (32 - sh)) | (dataRate << sh));
	}

	dataRate ^= factor;

	if (outRate & 0xffff) {
		bool altpth = false;

		if (dataRate / (outRate >> 16) > 0xffff) {
			dataRate = ((dataRate - outRate) << 16) | (factor >> 16);
			factor &= ~0xffff;
			altpth = true;
		} else {
			c = dataRate % (outRate >> 16);
			dataRate /= (outRate >> 16);
			t = ((c << 16) | (factor >> 16)) - ((dataRate & 0xffff) * (outRate & 0xffff));
			factor = (factor << 16) | dataRate;
			dataRate =  t & (uint32)-1;
			altpth = (int64)t < 0;
		}

		if (altpth) {
			for (t = dataRate; !(t >> 32); ) {
				--factor;
				t += outRate;
			}
			dataRate =  t & (uint32)-1;
		}

		if (dataRate / (outRate >> 16) > 0xffff) {
			dataRate = ((dataRate - outRate) << 16) | (factor >> 16);
			factor <<= 16;
			altpth = true;
		} else {
			c = dataRate % (outRate >> 16);
			dataRate /= (outRate >> 16);
			t = ((c << 16) | (factor >> 16)) - (dataRate * (outRate & 0xffff));
			factor = (factor << 16) | dataRate;
			dataRate =  t & (uint32)-1;;
			altpth = (int64)t < 0;
		}

		if (altpth) {
			t = dataRate;
			do {
				factor = (factor & ~0xffff) | (((factor & 0xffff) - 1) & 0xffff);
				t += outRate;
			} while (!(t >> 32));
			dataRate =  t & (uint32)-1;
		}

		result = factor;
	} else {
		outRate >>= 16;
		if (outRate == 0x8000) {
			t = factor << 1;
			t = (t >> 32) + (dataRate << 1);
		} else {
			c = dataRate % outRate;
			t = ((dataRate / outRate) << 16) | (((c << 16) | (factor >> 16)) / outRate);
		}
		result = t & (uint32)-1;
	}

	return result;
}

I3MFourToneSynthDriver::I3MFourToneSynthDriver(Common::Mutex &mutex, bool isStereo) :
	I3MMusicDriver(mutex, isStereo), _duration(0), _pos(0), _chan(nullptr), _numChan(4) {
	_chan = new Channel[_numChan];
}

I3MFourToneSynthDriver::~I3MFourToneSynthDriver() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numChan; ++i)
		setWaveForm(i, 0, 0);
	delete[] _chan;
}

void I3MFourToneSynthDriver::feed(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr || type != Audio::Mixer::kMusicSoundType)
		return;

	if (expectStereo != _stereo)
		error("I3MFourToneSynthDriver::feed(): stereo/mono mismatch between sound data and mixer stream");

	const int8 *end = dst + len;

	while (_duration && dst < end) {
		if (_pos == 0)
			--_duration;

		int16 smp = 0;
		for (int i = 0; i < _numChan; ++i) {
			_chan[i].phase += _chan[i].rate;
			smp += _chan[i].waveform[(_chan[i].phase >> 16) & 0xff];
		}

		smp = CLIP<int8>(smp >> 2, -128, 127);
		*dst++ = smp;
		if (_stereo)
			*dst++ = smp;

		if (++_pos == 370) {
			_pos = 0;
			if (!_duration)
				_sig = true;
		}
	}

	if (end > dst)
		memset(dst, 0, end - dst);
}

void I3MFourToneSynthDriver::start() {
	Common::StackLock lock(_mutex);
	stop();
	setDuration(50);
}

void I3MFourToneSynthDriver::stop() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numChan; ++i) {
		_chan[i].phase = 0;
		_chan[i].rate = 0;
	}
	setDuration(0);
}

void I3MFourToneSynthDriver::setDuration(uint16 duration) {
	Common::StackLock lock(_mutex);
	_duration = duration;
	_pos = 0;
	_sig = false;
}

void I3MFourToneSynthDriver::setWaveForm(uint8 chan, const uint8 *data, uint32 dataSize) {
	assert(chan < _numChan);
	Common::StackLock lock(_mutex);

	delete[] _chan[chan].waveform;
	if (data == nullptr || dataSize == 0)
		return;
	dataSize = MIN<uint32>(256, dataSize);
	int8 *wf = new int8[256];
	memset(wf, 0, 256);
	for (uint32 i = 0; i < dataSize; ++i)
		wf[i] = data[i] ^ 0x80;
	_chan[chan].waveform = wf;
}

void I3MFourToneSynthDriver::setRate(uint8 chan, uint16 rate) {
	assert(chan < _numChan);
	Common::StackLock lock(_mutex);

	_chan[chan].rate = rate ? (0x5060000 / (rate >> ((rate < 1600) ? 8 : 6))) : 0;
}

Common::WeakPtr<I3MPlayer> *I3MPlayer::_inst = nullptr;

I3MPlayer::I3MPlayer(ScummEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer), _musicChannels(nullptr), _lastSound(0), _lastSong(-1), _lastSoundEffectPrio(0), _soundEffectNumLoops(-1),
	_musicIDTable(nullptr), _macstr(nullptr), _musicIDTableLen(0), _soundUsage(0), _idRangeMax(86), _mdrv(nullptr), _sdrv(nullptr), _nextTickProc(this, &I3MPlayer::nextTick),
	_songPlaying(false), _soundEffectPlaying(false), _songTimer(0), _songTimerInternal(0), _qmode(0), _qualHi(false), _mixerThread(false), _activeChanCount(0), _numMusicChannels(4) {
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

I3MPlayer::~I3MPlayer() {
	_mixer->stopHandle(_soundHandle);
	delete _macstr;
	delete[] _soundUsage;

	for (Common::Array<I3MSoundDriver*>::const_iterator i = _drivers.begin(); i != _drivers.end(); ++i)
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

Common::SharedPtr<I3MPlayer> I3MPlayer::open(ScummEngine *vm, Audio::Mixer *mixer) {
	Common::SharedPtr<I3MPlayer> scp = nullptr;

	if (_inst == nullptr) {
		scp = Common::SharedPtr<I3MPlayer>(new I3MPlayer(vm, mixer));
		_inst = new Common::WeakPtr<I3MPlayer>(scp);
		// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
		// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
		if ((_inst == nullptr) || (mixer == nullptr) || !(scp->startDevices(mixer->getOutputRate(), mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, PCM_BUFFER_SIZE, true)))
			error("I3MPlayer::open(): Failed to start player");
	}

	return _inst->lock();
}

bool I3MPlayer::startDevices(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation) {
	_macstr = new AudioStream_I3M(this, outputRate, false, enableInterpolation);
	if (!_macstr || !_mixer)
		return false;

	_sdrv = new I3MLowLevelPCMDriver(_mixer->mutex(), pcmDeviceRate, enableInterpolation, false);
	I3MFourToneSynthDriver *mdrv = new I3MFourToneSynthDriver(_mixer->mutex(), false);
	if (!mdrv || !_sdrv)
		return false;

	for (int i = 0; i < 4; ++i)
		mdrv->setWaveForm(i, _fourToneSynthWaveForm, sizeof(_fourToneSynthWaveForm));
	_qualHi = true;
	_mdrv = mdrv;

	_drivers.push_back(_mdrv);
	_drivers.push_back(_sdrv);

	_macstr->initDrivers();
	_macstr->initBuffers(feedBufferSize);
	_macstr->setVblCallback(&_nextTickProc);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void I3MPlayer::setMusicVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kMusicSoundType, vol);
}

void I3MPlayer::setSfxVolume(int vol) {
	Common::StackLock lock(_mixer->mutex());
	if (_macstr)
		_macstr->setMasterVolume(Audio::Mixer::kSFXSoundType, vol);
}

void I3MPlayer::startSound(int id) {
	if (id < 0 || id >= _idRangeMax)
		return;

	if (isSong(id))
		startSong(id);
	else
		startSoundEffect(id);
}

void I3MPlayer::stopSound(int id) {
	if (id < 0 || id >= _idRangeMax) {
		warning("I3MPlayer::stopSound(): sound id '%d' out of range (0 - 85)", id);
		return;
	}

	Common::StackLock lock(_mixer->mutex());
	_soundUsage[id] = 0;

	if (id == _lastSound)
		stopActiveSound();
}

void I3MPlayer::stopAllSounds() {
	Common::StackLock lock(_mixer->mutex());
	memset(_soundUsage, 0, _idRangeMax);
	stopActiveSound();
}

int I3MPlayer::getMusicTimer() {
	Common::StackLock lock(_mixer->mutex());
	return _songTimer;
}

int I3MPlayer::getSoundStatus(int id) const {
	if (id < 0 || id >= _idRangeMax) {
		warning("I3MPlayer::getSoundStatus(): sound id '%d' out of range (0 - 85)", id);
		return 0;
	}
	Common::StackLock lock(_mixer->mutex());
	return _soundUsage[id];
}

void I3MPlayer::setQuality(int qual) {
	assert(qual >= Player_Mac_Indy3::kQualAuto && qual <= Player_Mac_Indy3::kQualLo);
	while (_qualHi == isHiQuality()) {
		if (_qmode == qual)
			return;
		_qmode = qual;
	}

	Common::StackLock lock(_mixer->mutex());
	Common::Array<I3MSoundDriver*>::iterator dr = Common::find(_drivers.begin(), _drivers.end(), _mdrv);
	delete _mdrv;
	_qmode = qual;

	if (isHiQuality()) {
		I3MFourToneSynthDriver *mdrv = new I3MFourToneSynthDriver(_mixer->mutex(), false);
		assert(mdrv);
		for (int i = 0; i < 4; ++i)
			mdrv->setWaveForm(i, _fourToneSynthWaveForm, sizeof(_fourToneSynthWaveForm));
		_mdrv = mdrv;
		_qualHi = true;
	} else {
		_mdrv = new I3MLQSynthDriver(_mixer->mutex(), false);
		_qualHi = false;
		assert(_mdrv);
	}

	if (dr != _drivers.end())
		*dr = _mdrv;
	else if (_drivers.empty())
		_drivers.push_back(_mdrv);
	else
		error("I3MPlayer::setQuality(): Invalid usage");

	assert(_macstr);
	_macstr->initDrivers();
}

void I3MPlayer::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	for (Common::Array<I3MSoundDriver*>::const_iterator i = _drivers.begin(); i != _drivers.end(); ++i)
		(*i)->feed(dst, len, type, expectStereo);
}

uint32 I3MPlayer::getDriverDeviceRate(uint8 drvID) const {
	return (drvID < _drivers.size()) ? _drivers[drvID]->getDeviceRate() : 0;
}

void I3MPlayer::nextTick() {
	if (_songTimerInternal++ == 29) {
		_songTimerInternal = 0;
		++_songTimer;
	}

	_mixerThread = true;

	if (!_songPlaying && _sdrv->checkSignal())
		updateSoundEffect();
	else if (_songPlaying)
		updateSong();

	_mixerThread = false;
}

void I3MPlayer::startSong(int id) {
	if (_mdrv == nullptr || id < 0 || id >= _idRangeMax) {
		warning("I3MPlayer::startSong(): sound id '%d' out of range (0 - 85)", id);
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
	if (_lastSong != -1)
		--_soundUsage[_lastSong];
	_lastSong = _lastSound = id;

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
	for (int i = 0; i < 3; ++i) {
		uint16 offs = READ_LE_UINT16(ptr);
		ptr += 2;
		if (offs)
			++_activeChanCount;
		_musicChannels[i]->start(sres, offs, _qualHi);
	}
	_songPlaying = true;
}

void I3MPlayer::startSoundEffect(int id) {
	if (_sdrv == nullptr || id < 0 || id >= _idRangeMax) {
		warning("I3MPlayer::startSoundEffect(): sound id '%d' out of range (0 - 85)", id);
		return;
	}

	Common::StackLock lock(_mixer->mutex());

	const uint8 *ptr = _vm->getResourceAddress(rtSound, id);
	assert(ptr);

	if (READ_LE_UINT16(ptr) < 28) {
		warning("I3MPlayer::startSoundEffect(%d): invalid resource", id);
		return;
	}

	if (_songPlaying)
		return;

	uint16 prio = READ_BE_UINT16(ptr + 4);

	if (_lastSound) {
		if (prio < _lastSoundEffectPrio)
			return;
		const uint8 *ptr2 = _vm->getResourceAddress(rtSound, _lastSound);
		assert(ptr2);
		if (READ_BE_UINT16(ptr2 + 6) == 0)
			_soundUsage[_lastSound] = 0;
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
	memcpy(buff, ptr + spos + 22, numSamples - 22);

	_pcmSnd.rate = 0x4E200000 / (READ_BE_UINT16(ptr + 20 + offs) >> 7);
	_pcmSnd.data = Common::SharedPtr<const byte> (buff, Common::ArrayDeleter<const byte>());
	_pcmSnd.len = numSamples - 23;
	_pcmSnd.loopst = numSamples - 2;
	_pcmSnd.loopend = numSamples - 1;
	_pcmSnd.baseFreq = 60;

	_sdrv->play(&_pcmSnd);

	_lastSound = id;
	_soundUsage[id]++;
}

void I3MPlayer::stopSong() {
	Common::StackLock lock(_mixer->mutex());
	_mdrv->stop();
	_songPlaying = false;
	--_soundUsage[_lastSong];
	_lastSound = _lastSong = 0;
}

void I3MPlayer::stopSoundEffect() {
	Common::StackLock lock(_mixer->mutex());
	_sdrv->stop();
	_soundEffectPlaying = false;
	_lastSoundEffectPrio = 0;
	_lastSound = 0;
}

void I3MPlayer::stopActiveSound() {
	if (_soundEffectPlaying)
		stopSoundEffect();
	else if (_songPlaying)
		stopSong();
}

void I3MPlayer::updateSong() {
	if (_lastSong) {
		for (int i = (_qualHi ? 4 : 1); i; --i) {
			for (int ii = 0; ii < _numMusicChannels && _songPlaying; ++ii)
				_musicChannels[ii]->nextTick();
		}
	}

	for (int i = 0; i < _numMusicChannels; ++i)
		_mdrv->setRate(i, _lastSong ? _musicChannels[i]->checkPeriod() : 0);
	if (_songPlaying)
		_mdrv->setDuration(10);
}

uint16 savedOffset = 0;
I3MPlayer::MusicChannel *I3MPlayer::MusicChannel::_ctrlChan = nullptr;

I3MPlayer::MusicChannel::MusicChannel(I3MPlayer *pl) : _player(pl), _vars(nullptr), _numVars(0), _ctrlProc(nullptr),
	_resSize(0), _savedOffset(savedOffset), _modShapes(g_pv2ModTbl), _modShapesTableSize(g_pv2ModTblSize) {
	static const CtrlProc ctrl[8] {
		&I3MPlayer::MusicChannel::ctrl_setShape,
		&I3MPlayer::MusicChannel::ctrl_modPara,
		&I3MPlayer::MusicChannel::ctrl_init,
		&I3MPlayer::MusicChannel::ctrl_returnFromSubroutine,
		&I3MPlayer::MusicChannel::ctrl_jumpToSubroutine,
		&I3MPlayer::MusicChannel::ctrl_initOther,
		&I3MPlayer::MusicChannel::ctrl_decrJumpIf,
		&I3MPlayer::MusicChannel::ctrl_writeVar
	};

	const uint16 *mVars[] = {
	/*  0 */	&_frameLen,			&_curPos,			&_freqCur,			&_freqIncr,			&_freqEff,
	/*  5 */	&_envPhase,				&_envRate,				&_tempo,			&_envCutoff,		(uint16*)&_transpose,
	/* 10 */	&_envLen,			&_envShape,			&_envStep,			&_envStepLen,		&_modType,
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

I3MPlayer::MusicChannel::~MusicChannel() {
	clear();
	delete[] _vars;
	_vars = nullptr;
	_numVars = 0;
}

void I3MPlayer::MusicChannel::clear() {
	for (int i = 0; i < _numVars; ++i)
		getMemberRef(i) = 0;
	_resource.reset();
	_resSize = 0;
	_hq = false;
}

void I3MPlayer::MusicChannel::start(Common::SharedPtr<const byte> &songRes, uint16 offset, bool hq) {
	clear();
	_resource = songRes;
	_resSize = READ_LE_UINT16(_resource.get());
	_curPos = offset;
	_frameLen = 1;
	_hq = hq;
}

void I3MPlayer::MusicChannel::nextTick() {
	if (!_frameLen)
		return;

	_ctrlChan = this;
	_envPhase += _envRate;
	_freqCur += _freqIncr;

	uint16 v = _modState + _modStep;
	int frqAdjust = 0;

	if (v != 0) {
		if (v >= _modRange)
			v -= _modRange;
		_modState = v;
		uint16 ix = (_modType + (v >> 4)) >> 4;
		assert(ix < _modShapesTableSize);
		frqAdjust = (((_modShapes[ix] << 7) * _modSensitivity) >> 16);
	}

	_freqEff = _freqCur + frqAdjust;

	if (_envLen && !--_envLen) {
		_envStep = 4;
		_envStepLen = 1;
	}

	if (!--_frameLen)
		parseNextEvents();

	if (!_envStepLen || --_envStepLen)
		return;

	int ix = _envShape + _envStep++;
	assert(ix < ARRAYSIZE(_envShapes));
	const uint32 *in = &_envShapes[ix];

	for (; (*in & 0xffff) == 0xffff; ++in) {
		_envPhase = *in >> 16;
		if (_envPhase == 0)
			_envRate = 0;
		++_envStep;
	}

	_envStepLen = *in & 0xffff;
	_envRate = *in >> 16;
}

void I3MPlayer::MusicChannel::parseNextEvents() {
	if (_resSize && _curPos >= _resSize) {
		warning("I3MPlayer::MusicChannel::parseNext(): playback error");
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
		warning("I3MPlayer::MusicChannel::parseNext(): playback error");
		_frameLen = 0;
		_player->stopSong();
	}

	_curPos = _frameLen ? cp : 0;
	if (!_frameLen)
		_player->endOfTrack();
}

void I3MPlayer::MusicChannel::noteOn(uint16 duration, uint8 note) {
	static const uint16 noteFreqTable[2][12] = {
		{ 0xFFC0, 0xF140, 0xE3C0, 0xD700, 0xCB40, 0xBF80, 0xB4C0, 0xAA80, 0xA100, 0x9800, 0x8F80, 0x8740 },
		{ 0x8E84, 0x8684, 0x7EF7, 0x77D7, 0x714F, 0x6AC4, 0x64C6, 0x5F1E, 0x59C7, 0x54BD, 0x4FFC, 0x4B7E }
	};
	_envStep = 0;
	_envStepLen = 1;
	_frameLen = duration;
	_envLen = _frameLen - _envCutoff;
	int n = note + _transpose;
	while (n < 0)
		n += 12;

	_freqEff = _freqCur = noteFreqTable[_hq ? 0 : 1][n % 12] >> ( n / 12);
}

uint16 I3MPlayer::MusicChannel::checkPeriod() const {
	return (_frameLen && _envPhase) ? _freqEff : 0;
}

bool I3MPlayer::MusicChannel::ctrl_setShape(const byte *&pos) {
	static const uint16 offsets[15] = { 0, 6, 12, 18, 24, 30, 36, 44, 52, 60, 68, 82, 76, 82, 90 };
	uint8 i = (*pos++) >> 1;
	assert(i < ARRAYSIZE(offsets));
	_envShape = offsets[i];
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_modPara(const byte *&pos) {
	static const uint16 table[10] = { 0x0000, 0x1000, 0x1000, 0x1000, 0x2000, 0x0020, 0x3020, 0x2000, 0x2020, 0x1000 };
	int ix = (*pos++);
	if ((ix & 1) || ((ix >> 1) + 1 >= ARRAYSIZE(table)))
		error("I3MPlayer::MusicChannel::ctrl_modPara(): data error");
	ix >>= 1;
	_modType = table[ix];
	_modRange = table[ix + 1];
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_init(const byte *&pos) {
	limitedClear();
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_returnFromSubroutine(const byte *&pos) {
	pos = _resource.get() + _savedOffset;
	if (pos >= _resource.get() + _resSize)
		error("I3MPlayer::MusicChannel::ctrl_returnFromSubroutine(): invalid address");
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_jumpToSubroutine(const byte *&pos) {
	uint16 offs = READ_LE_UINT16(pos);
	_savedOffset = pos + 2 - _resource.get();
	if (offs >= _resSize)
		error("I3MPlayer::MusicChannel::ctrl_jumpToSubroutine(): invalid address");
	pos = _resource.get() + offs;
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_initOther(const byte *&pos) {
	uint16 val = READ_LE_UINT16(pos);
	pos += 2;
	if (val % 50)
		error("I3MPlayer::MusicChannel::ctrl_initOther(): data error");
	_ctrlChan = _player->getMusicChannel(val / 50);
	assert(_ctrlChan);
	_ctrlChan->limitedClear();
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_decrJumpIf(const byte *&pos) {
	uint16 &var = getMemberRef(*pos++ >> 1);
	int16 offs = READ_LE_INT16(pos);
	pos += 2;
	if (var == 0) {
		pos += offs;
		if (pos < _resource.get() || pos >= _resource.get() + _resSize)
			error("I3MPlayer::MusicChannel::ctrl_jumpToSubroutine(): invalid address");
	} else {
		--var;
	}
	return true;
}

bool I3MPlayer::MusicChannel::ctrl_writeVar(const byte *&pos) {
	byte ix = *pos++;
	uint16 val = READ_LE_UINT16(pos);
	pos += 2;
	(getMemberRef(ix >> 1)) = val;
	return (bool)ix;
}

bool I3MPlayer::MusicChannel::ctrlProc(int procId, const byte *&arg) {
	return (_ctrlChan && _ctrlProc && procId >= 0 && procId <= 7) ? (_ctrlChan->*_ctrlProc[procId])(arg) : false;
}

void I3MPlayer::MusicChannel::setFrameLen(uint8 len) {
	static const uint8 durationTicks[22] = {
		0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09,
		0x0C, 0x10, 0x12, 0x18, 0x20, 0x26, 0x30, 0x40, 0x48, 0x60, 0x00
	};

	assert(_ctrlChan);
	len &= 0x1f;
	if (len >= ARRAYSIZE(durationTicks))
		error("I3MPlayer::MusicChannel::setFrameLen(): Out of range (val %d, range 0 - %d)", len, ARRAYSIZE(durationTicks) - 1);
	_ctrlChan->_frameLen = MAX<uint16>(_ctrlChan->_tempo, 1) * durationTicks[len];
}

void I3MPlayer::MusicChannel::limitedClear() {
	for (int i = 1; i < 7; ++i)
		getMemberRef(i) = 0;
	for (int i = 8; i < 10; ++i)
		getMemberRef(i) = 0;
	for (int i = 11; i < 15; ++i)
		getMemberRef(i) = 0;
	for (int i = 15; i < 19; ++i)
		getMemberRef(i) = 0;
}

uint16 &I3MPlayer::MusicChannel::getMemberRef(int pos) {
	assert(_vars);
	if (pos < 0 || pos >= _numVars)
		error("I3MPlayer::MusicChannel::getMemberRef(): attempting invalid access (var: %d, valid range: %d - %d)", pos, 0, _numVars - 1);
	return *_vars[pos];
}

const uint32 I3MPlayer::MusicChannel::_envShapes[98] = {
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

void I3MPlayer::updateSoundEffect() {
	_sdrv->resetSignal(false);
	bool chkRestart = false;

	if (!_soundEffectPlaying || !_lastSound) {
		chkRestart = true;
	} else {
		if (_soundEffectNumLoops > 0)
			--_soundEffectNumLoops;
		if (_soundEffectNumLoops)
			_sdrv->play(&_pcmSnd);
		else
			--_soundUsage[_lastSound];
		chkRestart = (_soundEffectNumLoops == 0);
	}

	if (chkRestart) {
		_lastSound = 0;
		_lastSoundEffectPrio = 0;
		checkRestartSoundEffects();
	}
}

void I3MPlayer::checkRestartSoundEffects() {
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

const uint8 I3MPlayer::_fourToneSynthWaveForm[256] = {
	0x80, 0x7a, 0x74, 0x6e, 0x69, 0x63, 0x5d, 0x57, 0x52, 0x4c, 0x47, 0x42, 0x3e, 0x3b, 0x38, 0x35,
	0x34, 0x33, 0x34, 0x35, 0x37, 0x3a, 0x3e, 0x43, 0x49, 0x4e, 0x54, 0x5b, 0x61, 0x67, 0x6c, 0x71,
	0x75, 0x78, 0x7a, 0x7c, 0x7c, 0x7b, 0x79, 0x76, 0x73, 0x6f, 0x6b, 0x66, 0x62, 0x5e, 0x5b, 0x58,
	0x56, 0x56, 0x57, 0x59, 0x5c, 0x61, 0x67, 0x6e, 0x77, 0x80, 0x8a, 0x95, 0xa0, 0xac, 0xb7, 0xc2,
	0xcc, 0xd6, 0xdf, 0xe7, 0xee, 0xf4, 0xf8, 0xfb, 0xfe, 0xff, 0xff, 0xfe, 0xfd, 0xfb, 0xf9, 0xf6,
	0xf3, 0xf0, 0xec, 0xe9, 0xe6, 0xe3, 0xe0, 0xdd, 0xda, 0xd7, 0xd4, 0xd1, 0xce, 0xca, 0xc6, 0xc2,
	0xbd, 0xb8, 0xb3, 0xad, 0xa7, 0xa1, 0x9a, 0x93, 0x8d, 0x86, 0x7f, 0x79, 0x73, 0x6d, 0x68, 0x63,
	0x5f, 0x5c, 0x5a, 0x58, 0x57, 0x57, 0x58, 0x5a, 0x5c, 0x5f, 0x63, 0x67, 0x6b, 0x70, 0x75, 0x7b,
	0x80, 0x85, 0x8b, 0x90, 0x95, 0x99, 0x9d, 0xa1, 0xa4, 0xa6, 0xa8, 0xa9, 0xa9, 0xa8, 0xa6, 0xa4,
	0xa1, 0x9d, 0x98, 0x93, 0x8d, 0x87, 0x81, 0x7a, 0x73, 0x6d, 0x66, 0x5f, 0x59, 0x53, 0x4d, 0x48,
	0x43, 0x3e, 0x3a, 0x36, 0x32, 0x2f, 0x2c, 0x29, 0x26, 0x23, 0x20, 0x1d, 0x1a, 0x17, 0x14, 0x10,
	0x0d, 0x0a, 0x07, 0x05, 0x03, 0x02, 0x01, 0x01, 0x02, 0x05, 0x08, 0x0c, 0x12, 0x19, 0x21, 0x2a,
	0x34, 0x3e, 0x49, 0x54, 0x60, 0x6b, 0x76, 0x80, 0x89, 0x92, 0x99, 0x9f, 0xa4, 0xa7, 0xa9, 0xaa,
	0xaa, 0xa8, 0xa5, 0xa2, 0x9e, 0x9a, 0x95, 0x91, 0x8d, 0x8a, 0x87, 0x85, 0x84, 0x84, 0x86, 0x88,
	0x8b, 0x8f, 0x94, 0x99, 0x9f, 0xa5, 0xac, 0xb2, 0xb7, 0xbd, 0xc2, 0xc6, 0xc9, 0xcb, 0xcc, 0xcd,
	0xcc, 0xcb, 0xc8, 0xc5, 0xc2, 0xbe, 0xb9, 0xb4, 0xae, 0xa9, 0xa3, 0x9d, 0x97, 0x92, 0x8c, 0x86
};

void I3MPlayer::endOfTrack() {
	if (!_activeChanCount || !--_activeChanCount)
		stopSong();
}

bool I3MPlayer::isSong(int id) const {
	return (Common::find(_musicIDTable, &_musicIDTable[_musicIDTableLen], id) != &_musicIDTable[_musicIDTableLen]);
}

bool I3MPlayer::isHiQuality() const {
	return _mixerThread ? _qualHi : (_qmode == Player_Mac_Indy3::kQualAuto && (_vm->VAR_SOUNDCARD == 0xff || _vm->VAR(_vm->VAR_SOUNDCARD) == 11)) || (_qmode == Player_Mac_Indy3::kQualHi);
}

I3MPlayer::MusicChannel *I3MPlayer::getMusicChannel(uint8 id) const {
	return (id < _numMusicChannels) ? _musicChannels[id] : 0;
}

Player_Mac_Indy3::Player_Mac_Indy3(ScummEngine *vm, Audio::Mixer *mixer) : _player(nullptr) {
	_player = I3MPlayer::open(vm, mixer);
}

Player_Mac_Indy3::~Player_Mac_Indy3() {
	_player = nullptr;
}

void Player_Mac_Indy3::setMusicVolume(int vol) {
	if (_player != nullptr)
		_player->setMusicVolume(vol);
}

void Player_Mac_Indy3::setSfxVolume(int vol) {
	if (_player != nullptr)
		_player->setSfxVolume(vol);
}

void Player_Mac_Indy3::startSound(int id) {
	if (_player != nullptr)
		_player->startSound(id);
}

void Player_Mac_Indy3::stopSound(int id) {
	if (_player != nullptr)
		_player->stopSound(id);
}

void Player_Mac_Indy3::stopAllSounds() {
	if (_player != nullptr)
		_player->stopAllSounds();
}

int Player_Mac_Indy3::getMusicTimer() {
	return (_player != nullptr) ? _player->getMusicTimer() : 0;
}

int Player_Mac_Indy3::getSoundStatus(int id) const {
	return (_player != nullptr) ? _player->getSoundStatus(id) : 0;
}

void Player_Mac_Indy3::setQuality(int qual) {
	if (_player != nullptr)
		_player->setQuality(qual);
}

#undef ASC_DEVICE_RATE
#undef PCM_BUFFER_SIZE
#undef RATECNV_BIT_PRECSN

} // End of namespace Scumm
