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


class MacSndChannel {
public:
	MacSndChannel(MacLowLevelPCMDriver *drv, Audio::Mixer::SoundType sndtp, int synth, bool interp, bool enableL, bool enableR, MacLowLevelPCMDriver::ChanCallback *callback);
	~MacSndChannel();

	MacLowLevelPCMDriver::ChanHandle getHandle() const;

	void playSamples(const MacLowLevelPCMDriver::PCMSound *snd);
	void playNote(uint8 note, uint16 duration);
	void quiet();
	void flush();
	void wait(uint32 duration);
	void loadWaveTable(const byte *data, uint16 dataSize);
	void loadInstrument(const MacLowLevelPCMDriver::PCMSound *snd);
	void setTimbre(uint16 timbre);
	void callback(uint16 p1, const void *p2);

	struct SoundCommand {
		SoundCommand() : cmd(0), arg1(0), arg2(0), ptr(nullptr) {}
		SoundCommand(uint8 c, uint16 p1, uint32 p2) : cmd(c), arg1(p1), arg2(p2), ptr(nullptr) {}
		SoundCommand(uint8 c, uint16 p1, void *p2, byte ptrType) : cmd(c), arg1(p1), arg2(ptrType), ptr(p2) {}
		uint8 cmd;
		uint16 arg1;
		uint32 arg2;
		void *ptr;
	};

	void enqueueSndCmd(uint8 c, uint16 p1, uint32 p2, byte mode);
	void enqueueSndCmd(uint8 c, uint16 p1, const void *p2, byte ptrType, byte mode);
	bool idle() const;

	void setFlags(uint8 flags) { _flags |= flags; }
	void clearFlags(uint8 flags) { _flags &= ~flags; }

	void feed(int32 *dst, uint32 dstSize, byte dstFrameSize);

	const Audio::Mixer::SoundType _sndType;
	const int _synth;
	const bool _interpolate;
	uint8 _flags;

private:
	void setupSound(const MacLowLevelPCMDriver::PCMSound *snd);
	void setupRateConv(uint32 drate, uint32 mod, uint32 srate, bool ppr);
	void startSound(uint32 tmr);
	void processSndCmdQueue();
	uint32 calcRate(uint32 outRate, uint32 factor, uint32 dataRate);
	uint32 calcNoteRateAdj(int diff);
	void makeSquareWave(int8 *dstBuff, uint16 dstSize, byte timbre);

	Common::Array<SoundCommand> _sndCmdQueue;
	MacLowLevelPCMDriver *_drv;
	MacLowLevelPCMDriver::ChanCallback *_callback;
	Common::SharedPtr<const int8> _res;
	const int8 *_data;

	int8 _lastSmp[2];
	bool _enable[2];
	uint32 _len;
	uint16 _rmH;
	uint16 _rmL;
	uint32 _loopSt2;
	uint32 _loopSt;
	uint32 _loopEnd;
	uint32 _loopLen;
	byte _baseFreq;
	uint32 _rcPos;
	uint32 _smpWtAcc;
	uint16 _frameSize;
	byte _timbre;
	uint32 _srate;
	uint32 _phase;
	uint32 _tmrPos;
	uint32 _tmrInc;
	uint32 _tmrRate;
	uint32 _duration;
};

uint32 fixDiv2Frac(uint32 fxdvnd, uint32 fxdvs, byte prcbits) {
	uint32 dv = fxdvnd << (prcbits - 16);
	uint32 res = 0;
	for (uint32 ck = fxdvs; ck; ck = ck >> 16) {
		res = (res << 16) | (dv / fxdvs);
		dv = (dv % fxdvs);
		fxdvs >>= 16;
	}
	return res;
}

uint32 fracMul(uint32 frac, uint32 fx) {
	uint32 a = (frac >> 30) * (fx >> 16);
	uint32 b = (frac >> 30) * (fx & 0xffff);
	uint32 c = ((frac >> 14) & 0xffff) * (fx >> 16);
	uint32 d = ((frac >> 14) & 0xffff) * (fx & 0xffff);
	uint32 e = ((frac << 2) & 0xffff) * (fx >> 16);
	return (a << 16) + b + c + (d >> 16) + (e >> 16);
}

uint32 fixMul(uint32 fx1, uint32 fx2) {
	uint32 a = (fx1 >> 16) * (fx2 >> 16);
	uint32 b = (fx1 >> 16) * (fx2 & 0xffff);
	uint32 c = (fx1 & 0xffff) * (fx2 >> 16);
	uint32 d = (fx1 & 0xffff) * (fx2 & 0xffff);
	return (a << 16) + b + c + (d >> 16);
}

MacPlayerAudioStream::MacPlayerAudioStream(VblTaskClientDriver *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate, bool internal16Bit) : Audio::AudioStream(), _drv(drv),
	_vblSmpQty(0), _vblSmpQtyRem(0), _frameSize((stereo ? 2 : 1) * (internal16Bit ? 2 : 1)), _vblCountDown(0), _vblCountDownRem(0), _outputRate(scummVMOutputrate),
		_vblCbProc(nullptr), _isStereo(stereo), _interp(interpolate), _smpInternalSize(internal16Bit ? 2 : 1) {
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
		uint64 irt = (uint64)_drv->getDriverStatus(i, Audio::Mixer::kPlainSoundType).deviceRate * (1 << RATECNV_BIT_PRECSN) / _outputRate;
		_buffers[i].rateConvInt = irt >> (RATECNV_BIT_PRECSN + 16);
		_buffers[i].rateConvFrac = (irt >> 16) & ((1 << RATECNV_BIT_PRECSN) - 1);
		_buffers[i].rateConvAcc = 0;
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
			int numch = _drv->getDriverStatus(ii, stype[ii]).numExternalMixChannels;
			bool interp = _interp && _drv->getDriverStatus(ii, stype[ii]).allowInterPolation;
			if (!numch)
				continue;

			int smpN = _smpInternalSize == 2 ? *reinterpret_cast<int16*>(_buffers[ii].pos) : _buffers[ii].pos[0];
			int diff = smpN - _buffers[ii].lastL;
			if (diff && _buffers[ii].rateConvAcc && interp)
				diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
			smpL += (int32)((_buffers[ii].lastL + diff) * (_buffers[ii].volume / numch));

			if (_isStereo) {
				smpN = _smpInternalSize == 2 ? *reinterpret_cast<int16*>(&_buffers[ii].pos[2]) : _buffers[ii].pos[1];
				diff = smpN - _buffers[ii].lastR;
				if (diff && _buffers[ii].rateConvAcc && interp)
					diff = (diff * _buffers[ii].rateConvAcc) >> RATECNV_BIT_PRECSN;
				smpR += (int32)((_buffers[ii].lastR + diff) * (_buffers[ii].volume / numch));
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

		*buffer++ = CLIP<int32>(smpL >> 8, -32768, 32767);
		if (_isStereo)
			*buffer++ = CLIP<int32>(smpR >> 8, -32768, 32767);
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

MacLowLevelPCMDriver::MacLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool internal16Bit) :
	MacSoundDriver(mutex, deviceRate, 0, true, internal16Bit), _numInternalMixChannels(1), _mixBufferSize(0), _mixBuffer(nullptr) {
}

MacLowLevelPCMDriver::~MacLowLevelPCMDriver() {
	for (Common::Array<MacSndChannel*>::const_iterator i = _channels.begin(); i != _channels.end(); ++i)
		delete *i;
	delete[] _mixBuffer;
}

void MacLowLevelPCMDriver::feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) {
	if (dst == nullptr)
		return;

	uint32 mixBufferReqSize = byteSize / _smpSize;
	assert(!(byteSize & (_smpSize - 1)));

	if (mixBufferReqSize > _mixBufferSize) {
		delete[] _mixBuffer;
		_mixBufferSize = mixBufferReqSize;
		_mixBuffer = new int32[mixBufferReqSize];
	}
	memset(_mixBuffer, 0, sizeof(int32) * mixBufferReqSize);
	bool bufferChanged = false;

	for (Common::Array<MacSndChannel*>::const_iterator itr = _channels.begin(); itr != _channels.end(); ++itr) {
		MacSndChannel *ch = *itr;
		if (ch->_sndType != type || ch->idle())
			continue;

		bufferChanged = true;
		ch->feed(_mixBuffer, mixBufferReqSize, expectStereo ? 2 : 1);
	}

	if (!bufferChanged)
		return;

	const int32 *src = _mixBuffer;
	for (const int8 *end = &dst[byteSize]; dst < end; ++src) {
		if (_smpSize == 2)
			*reinterpret_cast<int16*>(dst) += CLIP<int32>(*src, _smpMin, _smpMax);
		else
			*dst += CLIP<int32>(*src / _numInternalMixChannels, _smpMin, _smpMax);
		dst += _smpSize;
	}
}

MacLowLevelPCMDriver::ChanHandle MacLowLevelPCMDriver::createChannel(Audio::Mixer::SoundType sndType, SynthType synthType, byte attributes, ChanCallback *callback) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = new MacSndChannel(this, sndType, synthType, synthType == kSampledSynth && !(attributes & kNoInterp), !(synthType == kSampledSynth && ((attributes & 3) == kInitChanRight)), !(synthType == kSampledSynth && ((attributes & 3) == kInitChanLeft)), callback);
	assert(ch);

	_channels.push_back(ch);
	updateStatus(sndType);
	return ch->getHandle();
}

void MacLowLevelPCMDriver::disposeChannel(ChanHandle handle) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;

	Audio::Mixer::SoundType sndType = ch->_sndType;

	for (Common::Array<MacSndChannel*>::iterator i = _channels.begin(); i != _channels.end(); ++i) {
		if (*i == ch) {
			delete *i;
			_channels.erase(i--);
		}
	}
	updateStatus(sndType);
}

void MacLowLevelPCMDriver::playSamples(ChanHandle handle, ExecMode mode, const PCMSound *snd) {
	if (!snd || !snd->data)
		return;

	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kSampledSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(81, 0, snd, 1, mode);
}

void MacLowLevelPCMDriver::playNote(ChanHandle handle, ExecMode mode, uint8 note, uint16 duration) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(40, note, duration, mode);
}

void MacLowLevelPCMDriver::quiet(ChanHandle handle, ExecMode mode) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(3, 0, 0, mode);
}

void MacLowLevelPCMDriver::flush(ChanHandle handle, ExecMode mode) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;
	ch->enqueueSndCmd(4, 0, 0, mode);
}

void MacLowLevelPCMDriver::wait(ChanHandle handle, ExecMode mode, uint16 duration) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;
	ch->enqueueSndCmd(10, duration, 0, mode);
}

void MacLowLevelPCMDriver::loadWaveTable(ChanHandle handle, ExecMode mode, const byte *data, uint16 dataSize) {
	if (!data || !dataSize)
		return;

	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kWaveTableSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(60, dataSize, data, 2, mode);
}

void MacLowLevelPCMDriver::loadInstrument(ChanHandle handle, ExecMode mode, const PCMSound *snd) {
	if (!snd || !snd->data)
		return;

	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kSampledSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(80, 0, snd, 1, mode);
}

void MacLowLevelPCMDriver::setTimbre(ChanHandle handle, ExecMode mode, uint16 timbre) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kSquareWaveSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(44, timbre, 0, mode);
}

void MacLowLevelPCMDriver::callback(ChanHandle handle, ExecMode mode, uint16 arg1, const void *arg2) {
	Common::StackLock lock(_mutex);
	MacSndChannel *ch = findAndCheckChannel(handle, __FUNCTION__, kIgnoreSynth);
	if (!ch)
		return;

	ch->enqueueSndCmd(13, arg1, arg2, 0, mode);
}

uint8 MacLowLevelPCMDriver::getChannelStatus(ChanHandle handle) const {
	MacSndChannel *ch = findChannel(handle); return ch ? ch->_flags : 0;
}

void MacLowLevelPCMDriver::clearChannelFlags(ChanHandle handle, uint8 flags) {
	MacSndChannel *ch = findChannel(handle); if (ch) ch->clearFlags(flags);
}

void MacLowLevelPCMDriver::updateStatus(Audio::Mixer::SoundType sndType) {
	_numInternalMixChannels = _smpSize > 1 ? 1 : _channels.size();
	_status[sndType].numExternalMixChannels = _smpSize > 1 ? _channels.size() : 1;
	_status[sndType].allowInterPolation = true;
	for (Common::Array<MacSndChannel*>::const_iterator ch = _channels.begin(); ch != _channels.end(); ++ch) {
		if (!(*ch)->_interpolate)
			_status[sndType].allowInterPolation = false;
	}
}

MacSndChannel *MacLowLevelPCMDriver::findAndCheckChannel(ChanHandle h, const char *callingProcName, byte reqSynthType) const {
	MacSndChannel *ch = findChannel(h);
	if (!ch) {
		warning("%s(): Channel not found", callingProcName);
		return nullptr;
	}

	if (reqSynthType != kIgnoreSynth && reqSynthType != ch->_synth) {
		warning("%s(): Wrong channel type", callingProcName);
		return nullptr;
	}

	return ch;
}

MacSndChannel *MacLowLevelPCMDriver::findChannel(ChanHandle handle) const {
	for (Common::Array<MacSndChannel*>::const_iterator ch = _channels.begin(); ch != _channels.end(); ++ch) {
		if ((*ch)->getHandle() == handle)
			return *ch;
	}
	return nullptr;
}

MacSndChannel::MacSndChannel(MacLowLevelPCMDriver *drv, Audio::Mixer::SoundType sndtp, int synth, bool interp, bool enableL, bool enableR, MacLowLevelPCMDriver::ChanCallback *callback) : _drv(drv), _sndType(sndtp),
	_synth(synth), _interpolate(interp), _frameSize(1), _len(0), _rmH(0), _rmL(0), _smpWtAcc(0), _loopSt2(0), _loopEnd(0), _loopLen(0), _loopSt(0), _baseFreq(0), _rcPos(0),
	_flags(0), _tmrPos(0), _tmrInc(0), _timbre(0), _data(nullptr), _srate(0), _phase(0), _duration(0), _tmrRate(0), _callback(callback) {
	_lastSmp[0] = _lastSmp[1] = 0;
	_enable[0] = enableL;
	_enable[1] = enableR;
	_tmrRate = fixDiv2Frac(0x10000, fixDiv2Frac(_drv->getStatus().deviceRate, 0x7D00000, 16), 30);

	if (synth != MacLowLevelPCMDriver::kSampledSynth) {
		_len = 256;
		_frameSize = 1;
		_loopSt2 = 0;
		_loopEnd = _len;
		_baseFreq = 69;
		setupRateConv(_drv->getStatus().deviceRate, 0x10000, ASC_DEVICE_RATE, false);
		if (_synth == MacLowLevelPCMDriver::kSquareWaveSynth)
			setTimbre(0x50);
	}
}

MacSndChannel::~MacSndChannel() {
	flush();
}

MacLowLevelPCMDriver::ChanHandle MacSndChannel::getHandle() const {
	union {
		const void *ptr;
		int hdl;
	} cnv;
	cnv.ptr = this;
	return cnv.hdl;
}

void MacSndChannel::playSamples(const MacLowLevelPCMDriver::PCMSound *snd) {
	if (!snd) {
		warning("%s(): nullptr sound argument", __FUNCTION__);
		return;
	}
	setupRateConv(_drv->getStatus().deviceRate, calcNoteRateAdj(60 - snd->baseFreq), snd->rate, true);
	setupSound(snd);
	startSound(0);
}

void MacSndChannel::playNote(uint8 note, uint16 duration) {
	note &= 0x7f;
	if (!note) {
		quiet();
		return;
	}

	uint32 adj = calcNoteRateAdj(note - _baseFreq);
	if (_synth == MacLowLevelPCMDriver::kSampledSynth)
		setupRateConv(_drv->getStatus().deviceRate, adj, _srate, true);
	else
		_srate = fixMul(0x50FBA, adj);

	startSound(duration);
}

void MacSndChannel::quiet() {
	_data = nullptr;
	_tmrInc = 0;
	_duration = 0;
}

void MacSndChannel::wait(uint32 duration) {
	_duration = duration;
	if (duration) {
		_tmrInc = _tmrRate;
		_tmrPos = 0;
	}
}

void MacSndChannel::flush() {
	for (Common::Array<SoundCommand>::const_iterator i = _sndCmdQueue.begin(); i != _sndCmdQueue.end(); ++i) {
		if (i->ptr && i->arg2 == 1)
			delete reinterpret_cast<MacLowLevelPCMDriver::PCMSound*>(i->ptr);
		else if (i->ptr && i->arg2 == 2)
			delete[] reinterpret_cast<byte*>(i->ptr);
	}
	_sndCmdQueue.clear();
	_tmrInc = 0;
	_duration = 0;
}

void MacSndChannel::loadWaveTable(const byte *data, uint16 dataSize) {
	if (!data) {
		warning("MacSndChannel::loadWaveTable(): nullptr wavetable argument");
		return;
	}
	assert(dataSize == _len);
	int8 *buff = new int8[dataSize]();
	const int8 *s = reinterpret_cast<const int8*>(data);
	for (uint32 i = 0; i < dataSize; ++i)
		buff[i] = *s++ ^ 0x80;
	_res = Common::SharedPtr<const int8>(buff, Common::ArrayDeleter<const int8>());
}

void MacSndChannel::loadInstrument(const MacLowLevelPCMDriver::PCMSound *snd) {
	if (!snd) {
		warning("MacSndChannel::loadInstrument(): nullptr sound argument");
		return;
	}
	setupSound(snd);
	setupRateConv(_drv->getStatus().deviceRate, 0x10000, snd->rate, false);
}

void MacSndChannel::setTimbre(uint16 timbre) {
	if (_timbre == timbre)
		return;

	int8 *buff = new int8[256]();
	makeSquareWave(buff, 256, timbre);
	_res = Common::SharedPtr<const int8>(buff);
	_timbre = timbre;
}

void MacSndChannel::callback(uint16 p1, const void *p2) {
	if (_callback && _callback->isValid())
		(*_callback)(p1, p2);
}

void MacSndChannel::enqueueSndCmd(uint8 c, uint16 p1, uint32 p2, byte mode) {
	if (mode == MacLowLevelPCMDriver::kImmediate) {
		switch (c) {
		case 3:
			quiet();
			break;
		case 4:
			flush();
			break;
		case 10:
			wait(p1);
			break;
		case 44:
			setTimbre(p1);
			break;
		default:
			_sndCmdQueue.insert(_sndCmdQueue.begin(), SoundCommand(c, p1, p2));
			break;
		}

	} else {
		_sndCmdQueue.push_back(SoundCommand(c, p1, p2));
	}
}

void MacSndChannel::enqueueSndCmd(uint8 c, uint16 p1, const void *p2, byte ptrType, byte mode) {
	if (mode == MacLowLevelPCMDriver::kImmediate && (c == 13 || c == 60 || c == 80)) {
		if (c == 60)
			loadWaveTable(reinterpret_cast<const byte*>(p2), p1);
		else if (c == 80)
			loadInstrument(reinterpret_cast<const MacLowLevelPCMDriver::PCMSound*>(p2));
		else if (c == 13)
			callback(p1, p2);
		return;
	}

	void *ptr = nullptr;
	if (ptrType == 1) {
		const MacLowLevelPCMDriver::PCMSound *s = reinterpret_cast<const MacLowLevelPCMDriver::PCMSound*>(p2);
		MacLowLevelPCMDriver::PCMSound *p = new MacLowLevelPCMDriver::PCMSound(s->data, s->len, s->rate, s->loopst, s->loopend, s->baseFreq, s->enc, s->stereo);
		ptr = p;
	} else if (ptrType == 2) {
		byte *d = new byte[p1];
		memcpy(d, p2, p1);
		ptr = d;
	}

	if (mode == MacLowLevelPCMDriver::kImmediate)
		_sndCmdQueue.insert(_sndCmdQueue.begin(), SoundCommand(c, p1, ptr, ptrType));
	else
		_sndCmdQueue.push_back(SoundCommand(c, p1, ptr, ptrType));
}

bool MacSndChannel::idle() const {
	return _sndCmdQueue.empty() && !(_flags & MacLowLevelPCMDriver::kStatusPlaying);
}

void MacSndChannel::feed(int32 *dst, uint32 dstSize, byte dstFrameSize) {
	const bool interp = _interpolate && _rmL;
	int32 diff = 0;

	for (const int32 *end = &dst[dstSize]; dst < end; ) {
		processSndCmdQueue();

		int8 in = 0;
		for (int i = 0; i < dstFrameSize; ++i) {
			if (_data != nullptr && i < _frameSize) {
				if (_synth != MacLowLevelPCMDriver::kSampledSynth) {
					in = _data[(_phase >> 16) & 0xff];
				} else {
					in = _data[_rcPos + i];
					if (interp && in != _lastSmp[i]) {
						diff = in - _lastSmp[i];
						diff = (diff * (_smpWtAcc >> 1)) >> 15;
						in = (_lastSmp[i] + diff) & 0xff;
					}
				}
			}
			if (_enable[i])
				*dst += in;
			dst++;
		}

		uint32 cpos = _rcPos;
		_rcPos += (_rmH * _frameSize);
		_phase += (_rmH * _srate);
		_smpWtAcc += _rmL;
		if (_smpWtAcc > 0xffff) {
			_smpWtAcc &= 0xffff;
			_rcPos += _frameSize;
			_phase += _srate;
		}

		_tmrPos += _tmrInc;
		while (_tmrPos > 0x3fffffff) {
			_tmrPos -= 0x40000000;
			if (_duration)
				--_duration;
		}

		if (_synth != MacLowLevelPCMDriver::kSampledSynth || cpos == _rcPos || _data == nullptr)
			continue;

		if (interp) {
			for (int i = 0; i < _frameSize; ++i)
				_lastSmp[i] = _data[_loopSt + ((_rcPos - _frameSize + i - _loopSt) % _loopLen)];
		}

		if (_rcPos >= _loopSt + _loopLen) {
			if (_loopEnd) {
				_loopSt = _loopSt2;
				_loopLen = _loopEnd - _loopSt2;
				if (interp) {
					for (int i = 0; i < _frameSize; ++i)
						_lastSmp[i] = _data[_loopSt + ((_rcPos - _frameSize + i - _loopSt) % _loopLen)];
				}
				_rcPos = _loopSt + ((_rcPos - _loopSt) % _loopLen);
			} else {
				_data = nullptr;
			}
		}
	}
}

void MacSndChannel::setupSound(const MacLowLevelPCMDriver::PCMSound *snd) {
	assert(_synth == MacLowLevelPCMDriver::kSampledSynth);

	_len = snd->len;
	const byte *in = snd->data.get();
	assert(in);
	int8 *buff = new int8[_len];
	for (uint32 i = 0; i < _len; ++i)
		buff[i] = *in++ ^ 0x80;
	_res = Common::SharedPtr<const int8>(buff, Common::ArrayDeleter<const int8>());
	_frameSize = snd->stereo ? 2 : 1;
	_loopSt = 0;
	_data = nullptr;

	if (snd->loopend - snd->loopst < 2 || snd->loopend < snd->loopst) {
		_loopSt2 = 0;
		_loopEnd = 0;
	} else {
		_loopSt2 = snd->loopst - (snd->loopst % _frameSize);
		_loopEnd = snd->loopend - (snd->loopend % _frameSize);
		assert(_loopEnd <= _len);
	}

	_baseFreq = snd->baseFreq;
	_srate = snd->rate;
}

void MacSndChannel::setupRateConv(uint32 drate, uint32 mod, uint32 srate, bool ppr) {
	uint32 rmul = calcRate(drate, mod, srate);

	if (ppr) {
		if (rmul >= 0x7FFD && rmul <= 0x8003)
			rmul = 0x8000;
		else if (ABS((int16)(rmul & 0xffff)) <= 7)
			rmul = (rmul + 7) & ~0xffff;

		if (rmul > (uint32)-64)
			rmul = (uint32)-64;
	}

	assert(rmul);
	_rmL = rmul & 0xffff;
	_rmH = rmul >> 16;
}

void MacSndChannel::startSound(uint32 duration) {
	_duration = duration;
	_tmrInc = duration ? _tmrRate : 0;
	_tmrPos = 0;

	_data = _res.get();
	_lastSmp[0] = _data[0];
	if (_len >= _frameSize)
		_lastSmp[1] = _data[1];
	_rcPos = 0;
	_loopSt = 0;
	_loopLen = _loopEnd ? _loopEnd : _len;
	_smpWtAcc = 0;
	_phase = 0;
}

void MacSndChannel::processSndCmdQueue() {
	if ((_data && _tmrInc == 0) || _duration)
		return;

	if (_sndCmdQueue.empty()) {
		setFlags(MacLowLevelPCMDriver::kStatusDone);
		clearFlags(MacLowLevelPCMDriver::kStatusPlaying);
		return;
	}

	_drv->clearFlags(MacLowLevelPCMDriver::kStatusDone);
	clearFlags(MacLowLevelPCMDriver::kStatusDone);
	setFlags(MacLowLevelPCMDriver::kStatusPlaying);

	SoundCommand &c = _sndCmdQueue.front();
	MacLowLevelPCMDriver::PCMSound *p = (c.ptr && c.arg2 == 1) ? reinterpret_cast<MacLowLevelPCMDriver::PCMSound*>(c.ptr) : nullptr;
	const byte *b = (c.ptr && c.arg2 == 2) ? reinterpret_cast<byte*>(c.ptr) : nullptr;

	switch (c.cmd) {
	case 3:
		quiet();
		break;
	case 4:
		flush();
		break;
	case 10:
		wait(c.arg1);
		break;
	case 13:
		callback(c.arg1, c.ptr);
		break;
	case 40:
		playNote(c.arg1, c.arg2);
		break;
	case 44:
		setTimbre(c.arg1);
		break;
	case 60:
		loadWaveTable(b, c.arg1);
		break;
	case 80:
		loadInstrument(p);
		break;
	case 81:
		playSamples(p);
		break;
	default:
		break;
	}

	if (p) {
		p->data.reset();
		delete p;
	} else if (b) {
		delete[] b;
	}

	if (!_sndCmdQueue.empty())
		_sndCmdQueue.erase(_sndCmdQueue.begin());
}

uint32 MacSndChannel::calcRate(uint32 outRate, uint32 factor, uint32 dataRate) {
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
			t = ((c << 16) | (factor >> 16)) - ((dataRate & 0xffff) * (outRate & 0xffff));
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

uint32 MacSndChannel::calcNoteRateAdj(int diff) {
	static const uint32 adjFrac[23] = {
		0x21e71f26, 0x23eb3588, 0x260dfc14, 0x285145f3, 0x2ab70212, 0x2d413ccd,
		0x2ff221af, 0x32cbfd4a, 0x35d13f33, 0x39047c0f, 0x3c686fce, 0x40000000,
		0x43ce3e4b, 0x47d66b0f, 0x4c1bf829, 0x50a28be6, 0x556e0424, 0x5a82799a,
		0x5fe4435e, 0x6597fa95, 0x6ba27e65, 0x7208f81d, 0x78d0df9c
	};

	diff = CLIP<int>(diff, -127, 127);
	return fracMul(adjFrac[11 + (diff % 12)], (diff >= 0) ? 1 << ((diff / 12) + 16) : 0x10000 / (1 << (-diff / 12)));
}

void MacSndChannel::makeSquareWave(int8 *dstBuff, uint16 dstSize, byte timbre) {
	static const byte ampTbl[128] = {
		0x80, 0x82, 0x83, 0x85, 0x86, 0x88, 0x89, 0x8b, 0x8d, 0x8e, 0x90, 0x91, 0x93, 0x94, 0x96, 0x97,
		0x99, 0x9a, 0x9c, 0x9d, 0x9f, 0xa1, 0xa2, 0xa4,	0xa5, 0xa6, 0xa8, 0xa9, 0xab, 0xac, 0xae, 0xaf,
		0xb1, 0xb2, 0xb4, 0xb5, 0xb6, 0xb8, 0xb9, 0xbb,	0xbc, 0xbd, 0xbf, 0xc0, 0xc1, 0xc3, 0xc4, 0xc5,
		0xc7, 0xc8, 0xc9, 0xca, 0xcc, 0xcd, 0xce, 0xcf,	0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd8, 0xd9,
		0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
		0xe9, 0xea, 0xeb, 0xec, 0xed, 0xed, 0xee, 0xef,	0xf0, 0xf0, 0xf1, 0xf2, 0xf2, 0xf3, 0xf4, 0xf4,
		0xf5, 0xf5, 0xf6, 0xf7, 0xf7, 0xf8, 0xf8, 0xf9,	0xf9, 0xf9, 0xfa, 0xfa, 0xfb, 0xfb, 0xfb, 0xfc,
		0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xff, 0xff
	};
	assert(dstSize == 256);
	timbre = MAX<byte>(8, 255 - ampTbl[127 - (timbre >> 1)]);

	byte p = 0;
	int8 *d1 = dstBuff;
	int8 *d2 = &d1[128];
	for (int i = 0; i < 65; ++i) {
		*d1++ = *d2-- = ((ampTbl[p] ^ 0x80) >> 2);
		p = MIN<int>(127, timbre + p);
	}
	d1 = dstBuff;
	d2 = &d1[128];
	for (int i = 0; i < 128; ++i)
		*d2++ = *d1++ ^ 0xff;
}

const uint8 _fourToneSynthWaveForm[256] = {
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

const uint32 _fourToneSynthWaveFormSize = sizeof(_fourToneSynthWaveForm);

class Indy3MacSnd;
class LoomMonkeyMacSnd;

template<typename T> class MusicEngineImpl : public MusicEngine {
public:
	MusicEngineImpl(ScummEngine *vm, Audio::Mixer *mixer);
	~MusicEngineImpl() override;
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
	void enable() override;
	void disable() override;

private:
	Common::SharedPtr<T> _player;
};

template <typename T> MusicEngineImpl<T>::MusicEngineImpl(ScummEngine *vm, Audio::Mixer *mixer) : _player(nullptr) {
	_player = T::open(vm, mixer);
}

template <typename T> MusicEngineImpl<T>::~MusicEngineImpl() {
	_player = nullptr;
}

template <typename T> void MusicEngineImpl<T>::setMusicVolume(int vol) {
	if (_player != nullptr)
		_player->setMusicVolume(vol);
}

template <typename T> void MusicEngineImpl<T>::setSfxVolume(int vol) {
	if (_player != nullptr)
		_player->setSfxVolume(vol);
}

template <typename T> void MusicEngineImpl<T>::startSound(int id) {
	if (_player != nullptr)
		_player->startSound(id);
}

template <typename T> void MusicEngineImpl<T>::stopSound(int id) {
	if (_player != nullptr)
		_player->stopSound(id);
}

template <typename T> void MusicEngineImpl<T>::stopAllSounds() {
	if (_player != nullptr)
		_player->stopAllSounds();
}

template <typename T> int MusicEngineImpl<T>::getMusicTimer() {
	return (_player != nullptr) ? _player->getMusicTimer() : 0;
}

template <typename T> int MusicEngineImpl<T>::getSoundStatus(int id) const {
	return (_player != nullptr) ? _player->getSoundStatus(id) : 0;
}

template <typename T> void MusicEngineImpl<T>::setQuality(int qual) {
	if (_player != nullptr)
		_player->setQuality(qual);
}

template <typename T> void MusicEngineImpl<T>::saveLoadWithSerializer(Common::Serializer &ser) {
	if (_player != nullptr)
		_player->saveLoadWithSerializer(ser);
}

template <typename T> void MusicEngineImpl<T>::restoreAfterLoad() {
	if (_player != nullptr)
		_player->restoreAfterLoad();
}

template <typename T> void MusicEngineImpl<T>::enable() {
	if (_player != nullptr)
		_player->enable();
}

template <typename T> void MusicEngineImpl<T>::disable() {
	if (_player != nullptr)
		_player->disable();
}

namespace MacSound {

MusicEngine *createPlayer(ScummEngine *vm) {
	assert(vm);
	assert(vm->_mixer);
	MusicEngine *res = nullptr;

	switch (vm->_game.id) {
	case GID_INDY3:
		res = new MusicEngineImpl<Indy3MacSnd>(vm, vm->_mixer);
		break;
	case GID_LOOM:
	case GID_MONKEY:
		res = new MusicEngineImpl<LoomMonkeyMacSnd>(vm, vm->_mixer);
		break;
	default:
		break;
	}

	return res;
}

} // end of namespace MacSound

#undef ASC_DEVICE_RATE
#undef VBL_UPDATE_RATE
#undef PCM_BUFFER_RESERVE
#undef RATECNV_BIT_PRECSN

} // End of namespace Scumm
