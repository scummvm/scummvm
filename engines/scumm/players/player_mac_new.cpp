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
#define VBL_UPDATE_RATE		0x003C25BD
#define PCM_BUFFER_RESERVE	64
#define RATECNV_BIT_PRECSN	24

MacPlayerAudioStream::MacPlayerAudioStream(VblTaskClientDriver *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate, bool internal16Bit) : Audio::AudioStream(), _drv(drv),
	_vblSmpQty(0), _vblSmpQtyRem(0), _frameSize((stereo ? 2 : 1) * (internal16Bit ? 2 : 1)), _vblCountDown(0), _vblCountDownRem(0), _outputRate(scummVMOutputrate),
		_vblCbProc(nullptr), _isStereo(stereo), _interp(interpolate), _smpInternalSize(internal16Bit ? 2 : 1), _volDown(internal16Bit ? 2 : 0) {
	assert(_drv);
	_vblSmpQty = (_outputRate << 16) / VBL_UPDATE_RATE;
	_vblSmpQtyRem = (_outputRate << 16) % VBL_UPDATE_RATE;
	_vblCountDown = _vblSmpQty;
	_vblCountDownRem = 0;
}

MacPlayerAudioStream::~MacPlayerAudioStream() {
	for (int i = 0; i < 2; ++i)
		delete[] _buffers[i].start;
}

void MacPlayerAudioStream::initBuffers(uint32 feedBufferSize) {
	for (int i = 0; i < 2; ++i)
		delete[] _buffers[i].start;

	for (int i = 0; i < 2; ++i) {
		_buffers[i].size = feedBufferSize / _frameSize;
		_buffers[i].start = new int8[_buffers[i].size + PCM_BUFFER_RESERVE];
		_buffers[i].end = &_buffers[i].start[_buffers[i].size];
	}
	clearBuffer();
}

void MacPlayerAudioStream::initDrivers() {
	for (int i = 0; i < 2; ++i) {
		if (!_drv)
			error("MacPlayerAudioStream::initDrivers(): Failed to query device rate for device %d", i);
		uint64 irt = (uint64)_drv->getDriverCaps(i).deviceRate * (1 << RATECNV_BIT_PRECSN) / _outputRate;
		_buffers[i].rateConvInt = irt >> (RATECNV_BIT_PRECSN + 16);
		_buffers[i].rateConvFrac = (irt >> 16) & ((1 << RATECNV_BIT_PRECSN) - 1);
		_buffers[i].rateConvAcc = 0;
		_buffers[i].interpolate = _interp && _drv->getDriverCaps(i).allowInterPolation;
	}
}

void MacPlayerAudioStream::setVblCallback(const CallbackProc *proc) {
	_vblCbProc = proc;
}

void MacPlayerAudioStream::clearBuffer() {
	for (int i = 0; i < 2; ++i) {
		memset(_buffers[i].start, 0, _buffers[i].size + PCM_BUFFER_RESERVE);
		_buffers[i].pos = _buffers[i].start;
	}
}

void MacPlayerAudioStream::setMasterVolume(Audio::Mixer::SoundType type, uint16 vol) {
	if (type == Audio::Mixer::kMusicSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[0].volume = vol * vol;
	if (type == Audio::Mixer::kSFXSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[1].volume = vol * vol;
}

int MacPlayerAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	static const Audio::Mixer::SoundType stype[2] = {
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType
	};

	static const char errFnNames[2][8] = {"Buffers", "Drivers"};
	int errNo = (!_buffers[0].size || !_buffers[1].size) ? 0 : ((_buffers[0].rateConvAcc == -1 || _buffers[1].rateConvAcc == -1) ? 1 : -1);
	if (errNo != -1)
		error("MacPlayerAudioStream::readBuffer(): init%s() must be called before playback", errFnNames[errNo]);

	for (int i = _isStereo ? numSamples >> 1 : numSamples; i; --i) {
		if (!--_vblCountDown) {
			_vblCountDown = _vblSmpQty;
			_vblCountDownRem += _vblSmpQtyRem;
			while (_vblCountDownRem >= (_vblSmpQty << 16)) {
				_vblCountDownRem -= (_vblSmpQty << 16);
				++_vblCountDown;
			}
			runVblTask();
		}

		int32 smpL = 0;
		int32 smpR = 0;
		for (int ii = 0; ii < 2; ++ii) {
			int smpN = _smpInternalSize == 2 ? *reinterpret_cast<int16*>(_buffers[ii].pos) : _buffers[ii].pos[0];
			int diff = smpN - _buffers[ii].lastL;
			if (diff && _buffers[ii].rateConvAcc && _buffers[ii].interpolate)
				diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
			smpL += (int32)((_buffers[ii].lastL + diff) * (_buffers[ii].volume >> _volDown));
		}
		if (_isStereo) {
			for (int ii = 0; ii < 2; ++ii) {
				int smpN = _smpInternalSize == 2 ? *reinterpret_cast<int16*>(&_buffers[ii].pos[2]) : _buffers[ii].pos[1];
				int diff = smpN - _buffers[ii].lastR;
				if (diff && _buffers[ii].rateConvAcc && _buffers[ii].interpolate)
					diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
				smpR += (int32)((_buffers[ii].lastR + diff) * (_buffers[ii].volume >> _volDown));
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
				const int8 *lpos = _buffers[ii].pos;
				if (lpos >= _buffers[ii].start + _frameSize)
					lpos -= _frameSize;

				if (_smpInternalSize == 2) {
					_buffers[ii].lastL = *reinterpret_cast<const int16*>(lpos);
					if (_isStereo)
						_buffers[ii].lastR = *reinterpret_cast<const int16*>(&lpos[2]);
				} else {
					_buffers[ii].lastL = lpos[0];
					if (_isStereo)
						_buffers[ii].lastR = lpos[1];
				}

				if (_buffers[ii].pos >= _buffers[ii].end) {
					int refreshSize = MIN<int>(_vblCountDown * _frameSize, _buffers[ii].size);
					_buffers[ii].pos -= refreshSize;
					assert(_buffers[ii].pos + refreshSize < _buffers[ii].end + PCM_BUFFER_RESERVE);
					generateData(_buffers[ii].pos, refreshSize, stype[ii], _isStereo);
				}
			}
		}

		*buffer++ = CLIP<int16>(smpL >> 8, -32768, 32767);
		if (_isStereo)
			*buffer++ = CLIP<int16>(smpR >> 8, -32768, 32767);
	}
	return numSamples;
}

void MacPlayerAudioStream::generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const {
	if (_drv)
		_drv->generateData(dst, byteSize, type, expectStereo);
}

void MacPlayerAudioStream::runVblTask() {
	if (_vblCbProc && _vblCbProc->isValid())
		(*_vblCbProc)();
}

MacLowLevelPCMDriver::MacLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool enableInterpolation, bool internal16Bit) :
	MacSoundDriver(mutex, deviceRate, true, internal16Bit), _interp(enableInterpolation), _frameSize(1), _len(0), _rmH(0), _rmL(0), _smpWtAcc(0), _loopSt(0),
		_loopEnd(0), _baseFreq(0), _rcPos(0), _data(nullptr) {
			_lastSmp[0] = _lastSmp[1] = 0;
}

void MacLowLevelPCMDriver::feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr || type != Audio::Mixer::kSFXSoundType)
		return;

	memset(dst, 0, byteSize);

	if (_data == nullptr)
		return;

	int32 diff = 0;
	uint16 destFrameSize = expectStereo ? 2 : 1;
	bool interp = (_interp && _rmL);

	for (const int8 *end = &dst[byteSize]; dst < end; ) {
		int8 in = 0;
		for (int i = 0; i < destFrameSize; ++i) {
			if (i < _frameSize) {
				in = _data[_rcPos + i];
				if (interp && in != _lastSmp[i]) {
					diff = in - _lastSmp[i];
					diff = (diff * (_smpWtAcc >> 1)) >> 15;
					in = (_lastSmp[i] + diff) & 0xff;
				}
			}
			if (_smpSize == 2)
				*reinterpret_cast<int16*>(dst) = in << 2;
			else
				*dst = in;
			dst += _smpSize;
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
			setFlags(kStatusDone);
		}
	}
}

void MacLowLevelPCMDriver::play(PCMSound *snd) {
	if (!snd || !snd->data)
		return;

	Common::StackLock lock(_mutex);

	_res = snd->data.reinterpretCast<const int8>();
	_data = _res.get();
	_len = snd->len;
	uint32 rmul = calcRate(_caps.deviceRate, 0x10000, snd->rate);

	if (rmul >= 0x7FFD && rmul <= 0x8003)
		rmul = 0x8000;
	else if (ABS((int16)(rmul & 0xffff)) <= 7)
		rmul = (rmul + 7) & ~0xffff;

	if (rmul > (uint32)-64)
		rmul = (uint32)-64;

	assert(rmul);

	_rmL = rmul & 0xffff;
	_rmH = rmul >> 16;

	_frameSize = snd->stereo ? 2 : 1;

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
	clearFlags(kStatusDone);
}

void MacLowLevelPCMDriver::stop() {
	Common::StackLock lock(_mutex);
	_data = nullptr;
	_res.reset();
	setFlags(kStatusDone);
}

uint32 MacLowLevelPCMDriver::calcRate(uint32 outRate, uint32 factor, uint32 dataRate) {
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

class Indy3MacSnd;

template<typename T> class Player_Mac_New : public MusicEngine {
public:
	Player_Mac_New(ScummEngine *vm, Audio::Mixer *mixer);
	~Player_Mac_New() override;
	void setMusicVolume(int vol) override;
	void setSfxVolume(int vol) override;
	void startSound(int id) override;
	void stopSound(int id) override;
	void stopAllSounds() override;
	int getMusicTimer() override;
	int getSoundStatus(int id) const override;
	void setQuality(int qual) override;
	void saveLoadWithSerializer(Common::Serializer &ser) override;
	void restoreAfterLoad() override;

private:
	Common::SharedPtr<T> _player;
};

template <typename T> Player_Mac_New<T>::Player_Mac_New(ScummEngine *vm, Audio::Mixer *mixer) : _player(nullptr) {
	_player = T::open(vm, mixer);
}

template <typename T> Player_Mac_New<T>::~Player_Mac_New() {
	_player = nullptr;
}

template <typename T> void Player_Mac_New<T>::setMusicVolume(int vol) {
	if (_player != nullptr)
		_player->setMusicVolume(vol);
}

template <typename T> void Player_Mac_New<T>::setSfxVolume(int vol) {
	if (_player != nullptr)
		_player->setSfxVolume(vol);
}

template <typename T> void Player_Mac_New<T>::startSound(int id) {
	if (_player != nullptr)
		_player->startSound(id);
}

template <typename T> void Player_Mac_New<T>::stopSound(int id) {
	if (_player != nullptr)
		_player->stopSound(id);
}

template <typename T> void Player_Mac_New<T>::stopAllSounds() {
	if (_player != nullptr)
		_player->stopAllSounds();
}

template <typename T> int Player_Mac_New<T>::getMusicTimer() {
	return (_player != nullptr) ? _player->getMusicTimer() : 0;
}

template <typename T> int Player_Mac_New<T>::getSoundStatus(int id) const {
	return (_player != nullptr) ? _player->getSoundStatus(id) : 0;
}

template <typename T> void Player_Mac_New<T>::setQuality(int qual) {
	if (_player != nullptr)
		_player->setQuality(qual);
}

template <typename T> void Player_Mac_New<T>::saveLoadWithSerializer(Common::Serializer &ser) {
	if (_player != nullptr)
		_player->saveLoadWithSerializer(ser);
}

template <typename T> void Player_Mac_New<T>::restoreAfterLoad() {
	if (_player != nullptr)
		_player->restoreAfterLoad();
}

namespace MacSound {
MusicEngine *createPlayer(ScummEngine *vm) {
	assert(vm);
	assert(vm->_mixer);
	if (vm->_game.id == GID_INDY3)
		return new Player_Mac_New<Indy3MacSnd>(vm, vm->_mixer);

	return nullptr;
}
} // end of namespace MacSound

#undef ASC_DEVICE_RATE
#undef VBL_UPDATE_RATE
#undef PCM_BUFFER_SIZE
#undef PCM_BUFFER_RESERVE
#undef RATECNV_BIT_PRECSN

} // End of namespace Scumm
