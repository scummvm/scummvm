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

#include "kyra/kyra_v1.h"
#include "kyra/sound/sound_mac_res.h"
#include "kyra/sound/drivers/halestorm.h"

#include "audio/audiostream.h"

#include "common/func.h"
#include "common/queue.h"

#define ASC_DEVICE_RATE		22254

namespace Kyra {

class HSLowLevelDriver;
class HSAudioStream : public Audio::AudioStream {
public:
	HSAudioStream(HSLowLevelDriver *drv, uint32 scummVMOutputrate, uint32 deviceRate, uint32 feedBufferSize, bool output16Bit);
	~HSAudioStream() override;

	typedef Common::Functor0Mem<void, HSSoundSystem> CallbackProc;
	void setVblCallback(CallbackProc *proc);
	void clearBuffer();

	void setMasterVolume(Audio::Mixer::SoundType type, uint16 vol);

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return _isStereo; }
	int getRate() const override { return _outputRate; }
	bool endOfData() const override { return false; }

private:
	template<typename T> void generateData(T *dst, uint32 len, Audio::Mixer::SoundType);
	void runVblTask();

	HSLowLevelDriver *_drv;

	uint32 _vblSmpQty;
	uint32 _vblSmpQtyRem;
	uint32 _vblCountDown;
	uint32 _vblCountDownRem;

	CallbackProc *_vblCbProc;

	struct SmpBuffer {
		SmpBuffer() : start(0), pos(0), end(0), volume(0x10000) {}
		void *start;
		void *pos;
		const void *end;
		uint32 volume;
	} _buffers[2];

	const uint32 _intRate;
	const uint32 _outputRate;
	const uint32 _buffSize;
	const uint8 _outputByteSize;
	uint32 _rateConvCnt;

	const bool _isStereo;
};

static int DEBUG_BUFFERS_COUNT = 0;

class ShStBuffer {
public:
	ShStBuffer(const ShStBuffer &buff) : ptr(buff.ptr), len(buff.len), lifes(buff.lifes) { if (lifes) (*lifes)++; }
	ShStBuffer(ShStBuffer &&buff) noexcept : ptr(buff.ptr), len(buff.len), lifes(buff.lifes) { buff.lifes = nullptr; }
	ShStBuffer(const void *p, uint32 cb, bool allocate = false) : ptr((const uint8*)p), len(cb), lifes(nullptr) { if (allocate) memcpy(crtbuf(), p, cb); }
	ShStBuffer() : ShStBuffer(nullptr, 0) {}
	ShStBuffer(Common::SeekableReadStream *s) : len(s ? s->size() : 0), lifes(nullptr), ptr(nullptr) { if (s) s->read(crtbuf(), len); }
	~ShStBuffer() { dcrlif(); }
	ShStBuffer &operator=(Common::SeekableReadStream *s) { return operator=(ShStBuffer(s)); }
	ShStBuffer &operator=(ShStBuffer &&buff) noexcept {
		trans(buff);
		buff.lifes = nullptr;
		return *this;
	}
	ShStBuffer &operator=(const ShStBuffer &buff) {
		trans(buff);
		if (lifes)
			(*lifes)++;
		return *this;
	}
	const uint8 *ptr;
	uint32 len;
private:
	void dcrlif() {
		if (lifes && *lifes) {
			if (!--(*lifes)) {
				delete[] ptr;
				DEBUG_BUFFERS_COUNT--;
			}
		}
	}
	uint8 *crtbuf() {
		uint32 uplen = (len + sizeof(int) - 1) & ~(sizeof(int) - 1);
		uint8 *tptr = new uint8[uplen + sizeof(int)];
		lifes = (int*)(tptr + uplen);
		ptr = tptr;
		*lifes = 1;
		DEBUG_BUFFERS_COUNT++;
		return tptr;
	}
	void trans(const ShStBuffer &buff) {
		dcrlif();
		ptr = buff.ptr;
		len = buff.len;
		lifes = buff.lifes;
	}
	int *lifes;
};

class HSEffectFilter {
public:
	HSEffectFilter() {}
	virtual ~HSEffectFilter() {}

	virtual bool process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16 para2) = 0;
};

class HSVolumeScaler : public HSEffectFilter {
public:
	HSVolumeScaler() {}
	~HSVolumeScaler() override {}

	bool process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16 para2) override;
};

class HSLowPassFilter : public HSEffectFilter {
public:
	HSLowPassFilter() {}
	~HSLowPassFilter() override {}

	bool process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16 para2) override;
};

class HSSquareOMatic : public HSEffectFilter {
public:
	HSSquareOMatic() {}
	~HSSquareOMatic() override {}

	bool process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16) override;
};

class HSTriangulizer : public HSEffectFilter {
public:
	HSTriangulizer() {}
	~HSTriangulizer() override {}

	bool process(const ShStBuffer &src, uint8 *dst, uint16, uint16) override;
};

class HSSong {
public:
	HSSong() : _data(), _flags(0), _amplitudeScaleFlags(0), _interpolateType(0), _transpose(0), _tickLen(0), _tempo(0), _ticksPerSecond(0), _internalTempo(0),
		_numChanMusic(0), _numChanSfx(0), _convertUnitSize(0), _midiResId(0), _scan(false), _loop(false), _ready(false) {}
	void load(const ShStBuffer &data);
	void reset();
	void release();

	void setTempo(uint32 tempo);
	void setTicksPerSecond(uint32 tps);

	uint16 getTempo() const { return _internalTempo; }

public:
	int _numChanMusic;
	int _convertUnitSize;
	int _numChanSfx;

	uint16 _midiResId;
	uint16 _flags;
	uint8 _amplitudeScaleFlags;
	uint16 _interpolateType;
	int16 _transpose;
	uint16 _tickLen;

	bool _loop;
	bool _ready;

	Common::Array<uint16> _programMappings;
	bool _scan;

private:
	void updateTempo();

	ShStBuffer _data;

	uint32 _ticksPerSecond;
	uint16 _tempo;
	uint16 _internalTempo;
};

class HSMidiParser {
public:
	HSMidiParser(HSLowLevelDriver *driver);
	~HSMidiParser();

	bool loadTracks(HSSong &song);
	bool nextTick(HSSong &song);
	void stopResource(int id);
	bool isPlaying() const;
	void release();

private:
	struct TrackState {
		Common::Array<ShStBuffer>::const_iterator data;
		char status;
		uint16 resId;
		uint8 program;
		int32 ticker;
		const uint8 *curPos;
	};

	bool parseEvent(HSSong &song, TrackState *s);
	void noteOnOff(HSSong &song, TrackState *s, uint8 chan, uint8 note, uint8 velo);

	uint8 _partVolume[16];
	uint8 _partPrograms[16];
	uint8 _curCmd;

	ShStBuffer _data;
	Common::Array<ShStBuffer> _tracks;
	TrackState *_trackState;
	HSLowLevelDriver *_driver;
};

class HSLowLevelDriver {
	friend class HSMidiParser;
public:
	HSLowLevelDriver(SoundMacRes *res, Common::Mutex &mutex);
	~HSLowLevelDriver();

	HSAudioStream *init(uint32 scummVMOutputrate, bool output16bit);
	int send(int cmd, ...);

	template<typename T> void generateData(T *dst, uint32 len, Audio::Mixer::SoundType type);

private:
	typedef Common::Functor1Mem<va_list&, int, HSLowLevelDriver> HSOpcode;
	Common::Array<HSOpcode*> _hsOpcodes;

	int cmd_startSong(va_list&);
	int cmd_stopSong(va_list&);
	int cmd_restartSong(va_list&);
	int cmd_getDriverStatus(va_list&);
	int cmd_getSongStatus(va_list&);
	int cmd_stopSong2(va_list&);
	int smd_stopSong3(va_list&);
	int cmd_releaseSongData(va_list&);
	int cmd_deinit(va_list&);
	int cmd_09(va_list&);
	int cmd_10(va_list&);
	int cmd_11(va_list&);
	int cmd_12(va_list&);
	int cmd_setLoop(va_list&);
	int cmd_playSoundEffect(va_list&);
	int cmd_stopSoundEffect(va_list&);
	int cmd_setVolume(va_list&);
	int cmd_isSoundEffectPlaying(va_list&);
	int cmd_reserveChannels(va_list&);
	int cmd_stopAllSoundEffects(va_list&);
	int cmd_resetSoundEffectRate(va_list&);
	int cmd_setRate(va_list&);
	int cmd_25(va_list&);
	template<int res> int cmd_void(va_list&);

	SoundMacRes *_res;
	HSAudioStream *_vcstr;
	Common::Mutex &_mutex;

private:
	enum InterpolationMode {
		kTable = -1,
		kNone = 0,
		kSimple = 1
	};

	struct HSSoundChanState {
		const uint8 *dataPos;
		const uint8 *amplScaleData;
		uint32 rate;
		uint16 phase;
		uint8 velocity;
	};

	struct HSSoundChannel {
		int16 status;
		uint16 note;
		uint8 prg;
		uint16 id;
		uint16 flags;
		int8 mode;
		uint32 tickDataLen;
		int16 *numLoops;
		uint16 pmRate;
		InterpolationMode imode;
		const uint8 *dataEnd;
		const uint8 *loopStart;
		const uint8 *loopEnd;
		const uint16 *pmData;
		HSSoundChanState stateCur;
		HSSoundChanState stateSaved;

		const void *handle;
	};
	HSSoundChannel *_chan;

	void createTables();
	void pcmNextTick(int chanFirst, int chanLast);
	void pcmUpdateChannel(HSSoundChannel &chan);
	template<typename T> void fillBuffer(T *dst);

	uint8 *_sampleConvertBuffer;
	uint8 *_interpolationTable;
	uint8 *_interpolationTable2;
	uint8 *_amplitudeScaleBuffer;
	uint16 *_transBuffer;
	int16 *_wtable;

	int _convertBufferNumUnits;
	int _numChanSfxLast;
	int _convertUnitSizeLast;
	int _wtableCount;
	int _wtableCount2;
	InterpolationMode _interpolationMode;
	uint16 _samplesPerTick;
	uint16 _transCycleLenDef;
	uint16 _smpTransLen;
	uint16 _transCycleLenInter;
	const uint16 _pmDataTrm;
	bool _updateTypeHq;
	const uint16 _pcmDstBufferSize;

private:
	bool songStart();
	bool songInit();
	void songStopAllChannels();
	void songNextTick();
	bool songIsPlaying();

	void noteOn(uint8 part, uint8 prg, uint8 note, uint8 velo, uint16 ticker, const void *handle);
	void noteOff(uint8 part, uint8 note, const void *handle);

	int16 noteFromTable();

	HSSong _song;
	HSMidiParser *_midi;

private:
	void loadInstrument(int id);
	ShStBuffer loadInstrumentSamples(int id, bool sharedBuffer);
	ShStBuffer processWithEffect(const ShStBuffer &buf, uint16 smodId, uint16 para1, uint16 para2);

	struct NoteRangeSubset {
		NoteRangeSubset(uint8 min, uint8 max, const ShStBuffer &res) : rmin(min), rmax(max), sndRes(res) {}
		uint8 rmin, rmax;
		ShStBuffer sndRes;
	};

	struct InstrumentEntry {
		InstrumentEntry() : status(0), transpose(0), flags(0), flags2(0), refinst(0), sndRes(), pmData(), noteRangeSubsets() {}
		enum {
			kUnusable = -1,
			kRequestLoad = 0,
			kReady = 1
		};
		int8 status;
		int16 transpose;
		uint16 flags;
		uint8 flags2;
		uint16 refinst;
		ShStBuffer sndRes;
		ShStBuffer pmData;
		Common::Array<NoteRangeSubset> noteRangeSubsets;
	};

	InstrumentEntry *_instruments;

	struct InstrSamples {
		InstrSamples(uint16 id, ShStBuffer &b) : _id(id), _resource(b) {}
		ShStBuffer _resource;
		uint16 _id;
	};

	Common::Array<InstrSamples> _instrumentsSharedSamples;
	Common::Array<HSEffectFilter*> _hsFilters;

	static const uint32 _periods[156];
};

class HSSoundSystem {
private:
	HSSoundSystem(SoundMacRes *res, Audio::Mixer *mixer);
public:
	~HSSoundSystem();

	static HSSoundSystem *open(SoundMacRes *res, Audio::Mixer *mixer);
	static void close();

	bool init(bool hiQuality, uint8 interpolationMode, uint8 numChanSfx, bool output16bit);
	void registerSamples(const uint16 *resList, bool registerOnly);
	void releaseSamples();
	int changeSystemVoices(int numChanMusicTotal, int numChanMusicPoly, int numChanSfx);

	void startSoundEffect(int id, int rate = 0);
	void enqueueSoundEffect(int id, int rate, int note);
	void stopSoundEffect(int id);
	void stopAllSoundEffects();

	int doCommand(int cmd, va_list &arg);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	void vblTaskProc();

public:
	struct HSSoundEffectVoice {
		HSSoundEffectVoice() : enabled(false), loopStartDuration(0), loopEndDuration(0), duration(0), sync(0), dataPtr(nullptr), numSamples(0), rate(0), loopStart(0), loopEnd(0),
			resId(0), numLoops(0), b4(0), vblProc(nullptr), cb(nullptr), baseNote(60) {}
		bool enabled;
		uint32 loopStartDuration;
		uint32 loopEndDuration;
		uint32 duration;
		uint32 sync;
		const uint8 *dataPtr;
		uint32 numSamples;
		uint32 rate;
		uint32 loopStart;
		uint32 loopEnd;
		uint16 resId;
		mutable int16 numLoops;
		uint8 b4;
		void *vblProc;
		void *cb;
		uint16 baseNote;
	};

private:
	void setupSfxChannels(int num);
	HSSoundEffectVoice *findFreeVoice() const;
	HSSoundEffectVoice *findVoice(uint16 id) const;
	uint32 sync() const { return _sync; }

	int _numChanSfx;
	uint32 _sync;
	HSSoundEffectVoice **_voices;

private:
	struct SampleSlot {
		bool reverse;
		const uint8 *data;
		const uint8 *samples;
		uint32 numSamples;
		uint32 loopStart;
		uint32 loopEnd;
		uint16 resId;
		uint32 rate;
	};

	int doCommandIntern(int cmd, ...);

	bool loadSamplesIntoSlot(uint16 id, SampleSlot &slot, bool registerOnly) const;
	void deltaDecompress(uint8 *out, uint8 *in, uint32 outSize, uint32 inSize) const;
	void releaseSamplesFromSlot(SampleSlot &slot);
	SampleSlot *findSampleSlot(int id) const;

	void playSamples(const uint8 *data, uint32 numSamples, uint32 samplingRate, uint16 id, uint32 loopStart, uint32 loopEnd, int numLoops, int unk4, void *vblproc, void *cb);
	void reverseSamples(SampleSlot *slot);
	uint32 calculatePlaybackDuration(uint32 numSamples, uint32 samplingRate);

	int startSong(int id, int loop);

	void autoFadeHandler();

	SampleSlot *_sampleSlots;
	int _numSampleSlots;
	int _currentSong;
	uint16 _sfxDuration;
	int _volumeMusic;
	int _volumeSfx;
	bool _ready;

	bool _isFading;
	int16 _fadeState;
	int16 _fadeStep;
	int16 _fadeStepTicks;
	int16 _fadeStepTicksCounter;
	bool _fadeDirection;
	bool _fadeComplete;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	SoundMacRes *_res;
	HSAudioStream *_voicestr;
	HSLowLevelDriver *_driver;
	HSAudioStream::CallbackProc *_vblTask;
	Common::Mutex &_mutex;

	struct SfxQueueEntry {
		SfxQueueEntry(uint16 id, uint32 rate, uint16 duration) : _id(id), _rate(rate), _duration(duration) {}
		uint16 _id;
		uint32 _rate;
		uint16 _duration;
	};

	Common::Queue<SfxQueueEntry> _sfxQueue;

	static HSSoundSystem *_refInstance;
	static int _refCount;

	static const uint16 _noteFreq[58];
};

HSAudioStream::HSAudioStream(HSLowLevelDriver *drv, uint32 scummVMOutputrate, uint32 deviceRate, uint32 feedBufferSize, bool output16Bit) : Audio::AudioStream(), _drv(drv),
_outputRate(scummVMOutputrate), _intRate(deviceRate), _buffSize(feedBufferSize), _outputByteSize(output16Bit ? 2 : 1), _isStereo(false), _vblSmpQty(0), _vblSmpQtyRem(0),
_vblCountDown(0), _vblCountDownRem(0), _rateConvCnt(0), _vblCbProc(nullptr) {
	assert(drv);
	_vblSmpQty = scummVMOutputrate / 60;
	_vblSmpQtyRem = scummVMOutputrate % 60;
	_vblCountDown = _vblSmpQty;
	_vblCountDownRem = 0;

	for (int i = 0; i < 2; ++i) {
		_buffers[i].start = new uint8[_buffSize * _outputByteSize];
		_buffers[i].end = (uint8*)_buffers[i].start + _buffSize * _outputByteSize;
	}
	clearBuffer();
}

HSAudioStream::~HSAudioStream() {
	for (int i = 0; i < 2; ++i)
		delete[] (uint8*)_buffers[i].start;
}

void HSAudioStream::setVblCallback(CallbackProc *proc) {
	_vblCbProc = proc;
}

void HSAudioStream::clearBuffer() {
	for (int i = 0; i < 2; ++i) {
		memset(_buffers[i].start, _outputByteSize == 2 ? 0 : 0x80, _buffSize * _outputByteSize);
		_buffers[i].pos = _buffers[i].start;
	}
}

void HSAudioStream::setMasterVolume(Audio::Mixer::SoundType type, uint16 vol) {
	if (type == Audio::Mixer::kMusicSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[0].volume = vol * vol;
	if (type == Audio::Mixer::kSFXSoundType || type == Audio::Mixer::kPlainSoundType)
		_buffers[1].volume = vol * vol;
}

int HSAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	static const Audio::Mixer::SoundType stype[2] = {
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType
	};

	for (int i = _isStereo ? numSamples >> 1 : numSamples; i; --i) {
		if (!--_vblCountDown) {
			_vblCountDownRem += _vblSmpQtyRem;
			_vblCountDown = _vblSmpQty + _vblCountDownRem / _vblSmpQty;
			_vblCountDownRem %= _vblSmpQty;
			runVblTask();
		}

		int32 smp = 0;
		for (int ii = 0; ii < 2; ++ii)
			smp += (int32)(((_outputByteSize == 2) ? *(int16*)_buffers[ii].pos : *(uint8*)_buffers[ii].pos - 0x80) * _buffers[ii].volume);

		_rateConvCnt += _intRate;
		if (_rateConvCnt >= _outputRate) {
			_rateConvCnt -= _outputRate;

			for (int ii = 0; ii < 2; ++ii) {
				_buffers[ii].pos = (uint8*)_buffers[ii].pos + _outputByteSize;
				if (_buffers[ii].pos == _buffers[ii].end) {
					_buffers[ii].pos = _buffers[ii].start;
					if (_outputByteSize == 2)
						generateData<int16>((int16*)_buffers[ii].pos, _buffSize, stype[ii]);
					else
						generateData<uint8>((uint8*)_buffers[ii].pos, _buffSize, stype[ii]);
				}
			}
		}

		*buffer++ = CLIP<int16>(smp >> 8, -32768, 32767);
		if (_isStereo)
			*buffer++ = CLIP<int16>(smp >> 8, -32768, 32767);
	}

	return numSamples;
}

template<typename T> void HSAudioStream::generateData(T *dst, uint32 len, Audio::Mixer::SoundType type) {
	if (_drv)
		_drv->generateData<T>(dst, len, type);
}

void HSAudioStream::runVblTask() {
	if (_vblCbProc && _vblCbProc->isValid())
		(*_vblCbProc)();
}

void HSSong::load(const ShStBuffer &data) {
	_data = data;
	assert(_data.len >= 16);
}

void HSSong::reset() {
	_midiResId = READ_BE_UINT16(_data.ptr);
	_interpolateType = _data.ptr[2];
	_tickLen = READ_BE_UINT16(_data.ptr + 4);
	if (!_tickLen)
		_tickLen = 16667;

	_tempo = (500000u / _tickLen) & 0xffff;
	_ticksPerSecond = 60;
	updateTempo();

	_transpose = READ_BE_INT16(_data.ptr + 6);

	_numChanSfx = _data.ptr[8];
	_numChanMusic = MIN<uint8>(_data.ptr[9] + _numChanSfx, 16) - _numChanSfx;
	_convertUnitSize = MIN<uint16>(READ_BE_UINT16(_data.ptr + 10), 16);

	_flags = READ_BE_UINT16(_data.ptr + 12);
	_amplitudeScaleFlags = _data.ptr[15];

	const uint8 *pos = _data.ptr + 16;
	uint16 cnt = READ_BE_UINT16(pos) * 2;
	pos += 2;
	assert(18 + cnt * 2 <= (int32)_data.len);
	_programMappings.clear();

	while (cnt--) {
		_programMappings.push_back(READ_BE_UINT16(pos));
		pos += 2;
	}
}

void HSSong::release() {
	_data = ShStBuffer();
}

void HSSong::setTempo(uint32 tempo) {
	_tempo = (tempo / _tickLen) & 0xffff;
	updateTempo();
}

void HSSong::setTicksPerSecond(uint32 tps) {
	_ticksPerSecond = tps;
	updateTempo();
}

void HSSong::updateTempo() {
	_internalTempo = _scan ? 32767 : ((_ticksPerSecond << 6) / _tempo);
}

HSMidiParser::HSMidiParser(HSLowLevelDriver *driver) : _driver(driver), _trackState(nullptr), _tracks(), _data(), _curCmd(0) {
	_trackState = new TrackState[24]();
	memset(_partPrograms, 0, sizeof(_partPrograms));
	memset(_partVolume, 0, sizeof(_partVolume));
}

HSMidiParser::~HSMidiParser() {
	delete[] _trackState;
}

bool HSMidiParser::loadTracks(HSSong &song) {
	memset(_partVolume, 0x7f, sizeof(_partVolume));
	for (int i = 0; i < ARRAYSIZE(_partPrograms); ++i)
		_partPrograms[i] = i;

	Common::SeekableReadStream *midi = _driver->_res->getResource(song._midiResId, MKTAG('M', 'I', 'D', 'I'));
	if (!midi)
		midi = _driver->_res->getResource(song._midiResId, MKTAG('M', 'i', 'd', 'i'));
	assert(midi);

	_data = midi;
	const uint8 *in = _data.ptr;
	const uint8 *end = &_data.ptr[_data.len];
	_tracks.clear();

	while (in < end) {
		if (READ_BE_UINT32(in) == MKTAG('M', 'T', 'h', 'd'))
			break;
		in += 2;
	}
	if (in >= end)
		return false;

	int tps = READ_BE_INT16(in + 12);
	if (tps >= 0)
		song.setTicksPerSecond(tps);

	while (in < end) {
		if (READ_BE_UINT32(in) == MKTAG('M', 'T', 'r', 'k'))
			break;
		++in;
	}
	if (in >= end)
		return false;

	do {
		ShStBuffer track(in + 8, READ_BE_UINT32(in + 4));
		_tracks.push_back(track);
		in += (track.len + 8);
	} while (in < end && READ_BE_UINT32(in) == MKTAG('M', 'T', 'r', 'k'));

	uint8 prg = 0;
	for (Common::Array<ShStBuffer>::const_iterator i = _tracks.begin(); i != _tracks.end(); ++i) {
		int ch = 0;
		for (; ch < 24; ++ch) {
			if (!_trackState[ch].status)
				break;
		}
		if (ch == 24)
			return false;

		_trackState[ch].data = i;
		_trackState[ch].curPos = i->ptr;
		_trackState[ch].resId = song._midiResId;
		_trackState[ch].status = 'F';
		_trackState[ch].ticker = 0;
		_trackState[ch].program = prg++;
	}

	return true;
}

uint32 vlqRead(const uint8 *&s) {
	uint32 res = 0;
	do {
		res = (res << 7) | (*s & 0x7f);
	} while (*s++ & 0x80);
	return res;
}

bool HSMidiParser::nextTick(HSSong &song) {
	bool res = false;
	for (int ch = 0; ch < 24; ++ch) {
		TrackState *s = &_trackState[ch];
		if (!s->status)
			continue;

		res = true;
		bool checkPos = true;

		if (s->status == 'F') {
			s->status = 'R';
			checkPos = false;
		} else {
			s->ticker -= song.getTempo();
			if (s->ticker >= 0)
				continue;
		}

		bool contMain = false;
		for (bool checkTicker = true; checkPos || checkTicker; ) {
			if (checkPos) {
				if (s->curPos >= &s->data->ptr[s->data->len]) {
					s->status = '\0';
					contMain = true;
					break;
				}
				contMain = !parseEvent(song, s);
			}

			if (contMain)
				break;

			checkPos = false;

			uint32 val = vlqRead(s->curPos);
			if (val) {
				s->ticker += (val << 6);
				if (s->ticker >= 0)
					checkTicker = false;
				else
					checkPos = true;
			} else {
				checkTicker = parseEvent(song, s);
			}
		}
	}
	return res;
}

void HSMidiParser::stopResource(int id) {
	for (int i = 0; i < 24; ++i) {
		if (id < 0 || _trackState[i].resId == id)
			_trackState[i].status = '\0';
	}

	_driver->songStopAllChannels();
}

bool HSMidiParser::isPlaying() const {
	for (int ch = 0; ch < 24; ++ch) {
		if (_trackState[ch].status)
			return true;
	}
	return false;
}

void HSMidiParser::release() {
	_data = ShStBuffer();
}

bool HSMidiParser::parseEvent(HSSong &song, TrackState *s) {
	uint8 in = *s->curPos++;

	if (in < 0x80) {
		if (s->curPos <= s->data->ptr)
			error("HSMidiParser::parseEvent(): Data error");
		s->curPos--;
		in = _curCmd;
	} else if (in == 0xff) {
		uint evt = *s->curPos++;
		if (evt == 0x2f) {
			s->status = '\0';
			return false;
		} else if (evt == 0x51) {
			song.setTempo(s->curPos[1] << 16 | s->curPos[2] << 8 | s->curPos[3]);
		}

		s->curPos += vlqRead(s->curPos);
		return true;
	}

	_curCmd = in;
	uint8 evt = in & 0xf0;
	uint8 chan = in & 0x0f;
	uint8 arg1 = *s->curPos++;
	uint8 arg2 = (evt > 0xb0 && evt < 0xe0) ? 0 : *s->curPos++;

	if (evt < 0xa0)
		noteOnOff(song, s, chan, arg1, evt == 0x90 ? arg2 : 0);
	else if (evt == 0xb0 && arg1 == 7)
		_partVolume[chan] = arg2;
	else if (evt == 0xc0 && (song._flags & 0x400))
		s->program = _partPrograms[chan] = arg1;

	return true;
}

void HSMidiParser::noteOnOff(HSSong &song, TrackState *s, uint8 chan, uint8 note, uint8 velo) {
	uint16 prg = (song._flags & 0x800) ? s->program : _partPrograms[chan];

	for (Common::Array<uint16>::const_iterator i = song._programMappings.begin(); i != song._programMappings.end(); i += 2) {
		if (prg == i[0]) {
			prg = i[1];
			break;
		}
	}

	if (note + song._transpose > 0)
		note += song._transpose;

	if (velo)
		_driver->noteOn(chan, prg, note, _partVolume[chan] * velo / 0x7f, 10000, s);
	else
		_driver->noteOff(chan, note, s);
}

HSLowLevelDriver::HSLowLevelDriver(SoundMacRes *res, Common::Mutex &mutex) : _res(res), _vcstr(nullptr), _mutex(mutex), _sampleConvertBuffer(nullptr), _interpolationTable(nullptr),
_transCycleLenDef(0), _interpolationTable2(nullptr), _amplitudeScaleBuffer(nullptr), _interpolationMode(kNone), _wtable(nullptr), _wtableCount(0), _midi(nullptr),
_convertBufferNumUnits(0), _chan(nullptr), _samplesPerTick(0), _smpTransLen(0), _transCycleLenInter(0), _updateTypeHq(0), _instruments(nullptr), _pcmDstBufferSize(370),
_transBuffer(nullptr), _convertUnitSizeLast(0), _numChanSfxLast(0), _wtableCount2(0), _pmDataTrm(0x8000) {
#define HSOPC(x)	_hsOpcodes.push_back(new HSOpcode(this, &HSLowLevelDriver::x))
	HSOPC(cmd_startSong);
	HSOPC(cmd_stopSong);
	HSOPC(cmd_restartSong);
	HSOPC(cmd_getDriverStatus);
	HSOPC(cmd_getSongStatus);
	HSOPC(cmd_stopSong2);
	HSOPC(smd_stopSong3);
	HSOPC(cmd_releaseSongData);
	HSOPC(cmd_deinit);
	HSOPC(cmd_09);
	HSOPC(cmd_10);
	HSOPC(cmd_11);
	HSOPC(cmd_12);
	HSOPC(cmd_setLoop);
	HSOPC(cmd_void<-1>);
	HSOPC(cmd_void<-1>);
	HSOPC(cmd_void<0>);
	HSOPC(cmd_playSoundEffect);
	HSOPC(cmd_stopSoundEffect);
	HSOPC(cmd_setVolume);
	HSOPC(cmd_isSoundEffectPlaying);
	HSOPC(cmd_reserveChannels);
	HSOPC(cmd_stopAllSoundEffects);
	HSOPC(cmd_resetSoundEffectRate);
	HSOPC(cmd_setRate);
	HSOPC(cmd_25);
	HSOPC(cmd_void<1>);
#undef HSOPC
#define HSFLT(x)	_hsFilters.push_back(new x)
	HSFLT(HSVolumeScaler);
	HSFLT(HSLowPassFilter);
	HSFLT(HSSquareOMatic);
	HSFLT(HSTriangulizer);
#undef HSFLT
}

HSLowLevelDriver::~HSLowLevelDriver() {
	Common::StackLock lock(_mutex);
	delete _vcstr;
	delete[] _sampleConvertBuffer;
	delete[] _amplitudeScaleBuffer;
	delete[] _interpolationTable;
	delete[] _transBuffer;
	delete[] _wtable;
	delete[] _instruments;
	delete[] _chan;
	delete _midi;

	for (Common::Array<HSOpcode*>::iterator i = _hsOpcodes.begin(); i != _hsOpcodes.end(); ++i)
		delete *i;
	for (Common::Array<HSEffectFilter*>::iterator i = _hsFilters.begin(); i != _hsFilters.end(); ++i)
		delete *i;
}

HSAudioStream *HSLowLevelDriver::init(uint32 scummVMOutputrate, bool output16bit) {
	_chan = new HSSoundChannel[16];
	memset(_chan, 0, sizeof(HSSoundChannel) * 16);

	_instruments = new InstrumentEntry[128]();

	_transBuffer = new uint16[750];
	memset(_transBuffer, 0, 750 * sizeof(uint16));

	_wtable = new int16[17];
	memset(_wtable, 0, 17 * sizeof(int16));

	_midi = new HSMidiParser(this);

	_vcstr = new HSAudioStream(this, scummVMOutputrate, ASC_DEVICE_RATE, _pcmDstBufferSize, output16bit);
	return _vcstr;
}

int HSLowLevelDriver::send(int cmd, ...) {
	va_list args;
	va_start(args, cmd);

	int res = -1;
	if (_hsOpcodes[cmd]->isValid())
		res = (*_hsOpcodes[cmd])(args);

	va_end(args);

	return res;
}

template<typename T> void HSLowLevelDriver::generateData(T *dst, uint32 len, Audio::Mixer::SoundType type) {
	int first = 0;
	int last = _song._numChanMusic + _song._numChanSfx;

	if (type == Audio::Mixer::kMusicSoundType)
		last = _song._numChanMusic;
	else if (type == Audio::Mixer::kSFXSoundType)
		first = _song._numChanMusic;
	else if (type == Audio::Mixer::kSpeechSoundType)
		error("HSLowLevelDriver::generateData(): Unsupported sound type 'kSpeechSoundType'");

	pcmNextTick(first, last);
	if (type != Audio::Mixer::kSFXSoundType)
		songNextTick();
	fillBuffer<T>(dst);
}

int HSLowLevelDriver::cmd_startSong(va_list &arg) {
	Common::SeekableReadStream *song = _res->getResource(va_arg(arg, int), MKTAG('S', 'O', 'N', 'G'));
	Common::SeekableReadStream *midi = nullptr;
	if (song) {
		uint16 idm = song->readUint16BE();
		if (!(midi = _res->getResource(idm, MKTAG('M', 'I', 'D', 'I'))))
			midi = _res->getResource(idm, MKTAG('M', 'i', 'd', 'i'));
	}
	if (!song || !midi)
		error("HSLowLevelDriver::cmd_startSong(): Error encountered while loading song.");

	song->seek(0);
	_song.load(ShStBuffer(song));
	delete song;
	delete midi;

	for (int i = 0; i < 128; ++i)
		_instruments[i].status = InstrumentEntry::kUnusable;

	_song._scan = true;
	_midi->stopResource(-1);
	if (!songStart())
		error("HSLowLevelDriver::cmd_startSong(): Error reading song data.");

	// Fast-forward through the whole song to check which instruments need to be loaded
	bool loop = _song._loop;
	_song._loop = false;
	_song._ready = true;
	for (bool lp = true; lp; lp = songIsPlaying())
		songNextTick();

	_song._loop = loop;
	_song._ready = _song._scan = false;
	for (int i = 0; i < 128; ++i)
		loadInstrument(i);

	_song._ready = true;
	_midi->stopResource(-1);
	if (!songStart())
		error("HSLowLevelDriver::cmd_startSong(): Error reading song data.");

	_vcstr->clearBuffer();

	return 0;
}

int HSLowLevelDriver::cmd_stopSong(va_list &arg) {
	_midi->stopResource(-1);
	return 0;
}

int HSLowLevelDriver::cmd_restartSong(va_list &arg) {
	songStart();
	return 0;
}

int HSLowLevelDriver::cmd_getDriverStatus(va_list &arg) {
	return _vcstr ? -1 : 0;
}

int HSLowLevelDriver::cmd_getSongStatus(va_list &arg) {
	return songIsPlaying() ? -1 : 0;
}

int HSLowLevelDriver::cmd_stopSong2(va_list &arg) {
	_song._loop = false;
	_midi->stopResource(-1);
	return 0;
}

int HSLowLevelDriver::smd_stopSong3(va_list &arg) {
	for (int i = 0; i < _song._numChanMusic; ++i)
		_chan[i].status = -1;
	return 0;
}

int HSLowLevelDriver::cmd_releaseSongData(va_list &arg) {
	_song._ready = false;
	for (int i = 0; i < _song._numChanMusic; ++i)
		_chan[i].status = -1;

	_song.release();
	_midi->release();
	_instrumentsSharedSamples.clear();

	for (int i = 0; i < 128; ++i) {
		_instruments[i].pmData = ShStBuffer();
		_instruments[i].sndRes = ShStBuffer();
		_instruments[i].noteRangeSubsets.clear();
	}

	return 0;
}

int HSLowLevelDriver::cmd_deinit(va_list &arg) {
	send(7);
	delete[] _sampleConvertBuffer;
	_sampleConvertBuffer = nullptr;
	delete[] _amplitudeScaleBuffer;
	_amplitudeScaleBuffer = nullptr;
	delete[] _interpolationTable;
	_interpolationTable = nullptr;
	return 0;
}

int HSLowLevelDriver::cmd_09(va_list &arg) {
	return 0;
}

int HSLowLevelDriver::cmd_10(va_list &arg) {
	return 0;
}

int HSLowLevelDriver::cmd_11(va_list &arg) {
	return 0;
}

int HSLowLevelDriver::cmd_12(va_list &arg) {
	return 0;
}

int HSLowLevelDriver::cmd_setLoop(va_list &arg) {
	_song._loop = va_arg(arg, int);
	return 0;
}

int HSLowLevelDriver::cmd_playSoundEffect(va_list &arg) {
	const HSSoundSystem::HSSoundEffectVoice *vc = va_arg(arg, const HSSoundSystem::HSSoundEffectVoice*);
	if (!vc || !vc->dataPtr || !_song._numChanSfx)
		return 0;

	HSSoundChannel *chan = nullptr;
	int16 lowest = 32767;
	for (int i = _song._numChanMusic; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		HSSoundChannel *c = &_chan[i];
		if (c->status < 0) {
			chan = c;
			break;
		}
		if (c->status < lowest) {
			chan = c;
			lowest = c->status;
		}
	}

	if (!chan)
		return -1;

	chan->status = -1;
	chan->flags = 0x7fff;
	chan->mode = 0;
	chan->stateCur.phase = 0;
	chan->tickDataLen = 0;
	chan->pmData = &_pmDataTrm;
	chan->stateCur.dataPos = vc->dataPtr;
	chan->dataEnd = vc->dataPtr + vc->numSamples;
	chan->loopStart = chan->loopEnd = nullptr;
	chan->numLoops = &vc->numLoops;
	chan->imode = _interpolationMode ? kSimple : kNone;

	if (vc->loopStart && vc->loopEnd) {
		chan->loopStart = vc->dataPtr + vc->loopStart;
		chan->loopEnd = vc->dataPtr + vc->loopEnd;
	}

	chan->stateCur.rate = (vc->rate >> 5) / 11127;
	if (chan->stateCur.rate & ~0xffff)
		chan->stateCur.rate = 4096;
	chan->stateCur.rate <<= 5;
	if ((chan->stateCur.rate & 0xffff) == 0x20)
		chan->stateCur.rate &= ~0xffff;
	else if ((chan->stateCur.rate & 0xffff) == 0xffe0)
		chan->stateCur.rate += 32;

	chan->id = vc->resId;
	chan->status = 0x7ffe;
	return 0;
}

int HSLowLevelDriver::cmd_stopSoundEffect(va_list &arg) {
	const HSSoundSystem::HSSoundEffectVoice *vc = va_arg(arg, const HSSoundSystem::HSSoundEffectVoice*);
	for (int i = _song._numChanMusic; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		if (_chan[i].id == vc->resId)
			_chan[i].status = -1;
	}
	return 0;
}

int HSLowLevelDriver::cmd_setVolume(va_list &arg) {
	int volpara = va_arg(arg, int);
	int len = _song._numChanMusic + _song._numChanSfx - _song._convertUnitSize;
	uint8 cur = 0x80 - ((volpara * 0x80) >> 8);
	uint8 *dst = _sampleConvertBuffer;
	uint16 fn = 0;

	if (len > 0) {
		memset(dst, cur, len << 7);
		dst += (len << 7);
	} else if (len < 0) {
		error("void HSLowLevelDriver::cmd_setVolume(): Unknown error");
	}

	for (int i = 0; i < 256; ++i) {
		memset(dst, cur, _song._convertUnitSize);
		dst += _song._convertUnitSize;
		fn += volpara;
		cur += (fn >> 8);
		fn &= 0xff;
	}

	if (len > 0)
		memset(dst, 0xff, (len << 7) + 64);
	else if (len < 0)
		error("void HSLowLevelDriver::cmd_setVolume(): Unknown error");

	return 0;
}

int HSLowLevelDriver::cmd_isSoundEffectPlaying(va_list &arg) {
	const HSSoundSystem::HSSoundEffectVoice *vc = va_arg(arg, const HSSoundSystem::HSSoundEffectVoice*);
	for (int i = _song._numChanMusic; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		if (_chan[i].id == vc->resId)
			return (_chan[i].status != -1) ? -1 : 0;
	}
	return 0;
}

int HSLowLevelDriver::cmd_reserveChannels(va_list &arg) {
	_song._numChanMusic = va_arg(arg, int);
	_song._convertUnitSize = va_arg(arg, int);
	_song._numChanSfx = va_arg(arg, int);
	createTables();

	Common::StackLock lock(_mutex);
	_vcstr->clearBuffer();

	return 0;
}

int HSLowLevelDriver::cmd_stopAllSoundEffects(va_list &arg) {
	for (int i = _song._numChanMusic; i < _song._numChanMusic + _song._numChanSfx; ++i)
		_chan[i].status = -1;
	return 0;
}

int HSLowLevelDriver::cmd_resetSoundEffectRate(va_list &arg) {
	const HSSoundSystem::HSSoundEffectVoice *vc = va_arg(arg, const HSSoundSystem::HSSoundEffectVoice*);
	for (int i = _song._numChanMusic; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		HSSoundChannel *chan = &_chan[i];
		if (chan->status == -1 || chan->id != vc->resId)
			continue;

		chan->stateCur.rate = (vc->rate >> 5) / 11127;
		if (chan->stateCur.rate & ~0xffff)
			chan->stateCur.rate = 4096;
		chan->stateCur.rate <<= 5;
		if ((chan->stateCur.rate & 0xffff) == 0x20)
			chan->stateCur.rate &= ~0xffff;
		else if ((chan->stateCur.rate & 0xffff) == 0xffe0)
			chan->stateCur.rate += 32;

		chan->tickDataLen = 0;
		break;
	}
	return 0;
}

int HSLowLevelDriver::cmd_setRate(va_list &arg) {
	int rateKhz = va_arg(arg, int);
	_interpolationMode = (rateKhz & 0x200) ? kTable : ((rateKhz & 0x100) ? kSimple : kNone);
	rateKhz &= 0xff;

	if (rateKhz == 22) {
		_samplesPerTick = 370;
		_transCycleLenDef = 73;
		_smpTransLen = 36;
		_transCycleLenInter = 369;
		_updateTypeHq = true;
	} else if (rateKhz == 11) {
		_samplesPerTick = 185;
		_transCycleLenDef = 36;
		_smpTransLen = 17;
		_transCycleLenInter = 184;
		_updateTypeHq = false;
	} else {
		error("HSLowLevelDriver::cmd_setRate(): Invalid rate '%d'", rateKhz);
	}

	_smpTransLen = (_smpTransLen + 1) * 10 + 5;
	return 0;
}

int HSLowLevelDriver::cmd_25(va_list &arg) {
	for (int i = 0; i < 128; ++i)
		_instruments[i].status = InstrumentEntry::kReady;

	// TODO? Turbo music driver things. I don't see any support for it in KYRA,
	// so it couldn't even be tested. It uses 'TRBo' resources, so if anyone
	// ever sees one of these, it could be investigated...

	return 0;
}

template<int res> int HSLowLevelDriver::cmd_void(va_list &arg) {
	return res;
}

void HSLowLevelDriver::createTables() {
	static const uint16 wtable[] = {
		0x0937, 0x309c, 0xf62a, 0x90c6, 0x584b, 0x7f4f, 0x76b0, 0x040d,
		0x25ef, 0x7f3b, 0x696a, 0x1cbd, 0x87fd, 0x2fb4, 0xcdf9, 0xdb1a,
		0x07c7
	};

	memcpy(_wtable, wtable, sizeof(wtable));

	for (int i = 0; i < 16; ++i)
		_chan[i].status = -1;

	int sfxChanMult = 1;

	// sample convert buffer
	if (_sampleConvertBuffer) {
		if (_song._convertUnitSize != _convertUnitSizeLast || _song._numChanSfx != _numChanSfxLast || _convertBufferNumUnits - _song._numChanSfx * sfxChanMult != _song._numChanMusic) {
			delete[] _sampleConvertBuffer;
			_sampleConvertBuffer = nullptr;
		}
	}

	if (!_sampleConvertBuffer || _convertBufferNumUnits - _song._numChanSfx * sfxChanMult != _song._numChanMusic) {
		_convertBufferNumUnits = _song._numChanMusic + _song._numChanSfx * sfxChanMult;
		_convertUnitSizeLast = _song._convertUnitSize;
		_numChanSfxLast = _song._numChanSfx;
		_sampleConvertBuffer = new uint8[(_convertBufferNumUnits << 8) + 64];
		uint8 *dst = _sampleConvertBuffer;
		int len = _convertBufferNumUnits - _song._convertUnitSize;

		if (len > 0) {
			memset(dst, 0, len << 7);
			dst += (len << 7);
		} else if (len < 0) {
			error("void HSLowLevelDriver::createTables(): Invalid parameters");
		}

		for (int i = 0; i < 256; ++i) {
			memset(dst, i & 0xff, _song._convertUnitSize);
			dst += _song._convertUnitSize;
		}

		if (len > 0)
			memset(dst, 0xff, (len << 7) + 64);
		else if (len < 0)
			error("void HSLowLevelDriver::createTables(): Invalid parameters");

		assert(dst - _sampleConvertBuffer + (len << 7) <= (_convertBufferNumUnits << 8));
	}

	// ampitude scale buffer
	if ((_song._amplitudeScaleFlags & 0x02) && !_amplitudeScaleBuffer) {
		_amplitudeScaleBuffer = new uint8[0x8000];
		uint8 *dst = _amplitudeScaleBuffer;
		for (uint16 i = 0; i < 128; ++i) {
			uint16 cnt = i + 1;
			uint8 val = 128 - cnt;
			uint16 inc = (i + 1) << 1;
			for (int ii = 0; ii < 64; ++ii) {
				for (int iii = 0; iii < 4; ++iii) {
					*dst++ = val;
					cnt += inc;
					val += (cnt >> 8);
					cnt &= 0xff;
				}
			}
		}
	}

	// interpolation table
	if ((_song._flags & 0x3000) && (_interpolationMode != kSimple) && !_interpolationTable) {
		_interpolationTable = new uint8[0x20000];
		uint8 *dst = _interpolationTable;
		_interpolationTable2 = _interpolationTable + 0x10000;
		for (uint16 i = 256; i; --i) {
			for (uint16 ii = 0; ii < 256; ++ii)
				*dst++ = ((i * ii) + 0x7f) >> 8;
		}
		for (uint16 i = 0; i < 256; ++i) {
			for (uint16 ii = 0; ii < 256; ++ii)
				*dst++ = ((i * ii) + 0x80) >> 8;
		}
	}
}

void HSLowLevelDriver::pcmNextTick(int chanFirst, int chanLast) {
	int16 cnt = 0;
	uint16 val = 0;
	for (int i = 0; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		++cnt;
		if (i >= chanFirst && i < chanLast && _chan[i].status >= 0)
			continue;
		--cnt;
		val += 0x80;
	}

	if (!cnt)
		val = (_song._numChanMusic + _song._numChanSfx) << 7;

	Common::fill<uint16*, uint16>(_transBuffer, &_transBuffer[_smpTransLen], val);

	if (!cnt)
		return;

	for (int i = 0; i < _song._numChanMusic + _song._numChanSfx; ++i) {
		if (i < chanFirst || i >= chanLast || _chan[i].status < 0)
			continue;
		pcmUpdateChannel(_chan[i]);
	}
}

#define HS_CYCL_DEF(len) int i = 0; i < len; ++i
#define HS_CYCL_NOINIEX(len, cond) ; i < len && cond; ++i
#define HS_ADVSRC \
	ih += rate; \
	src += (ih >> 16); \
	ih &= 0xffff

#define HS_VOID { }
#define HS_LOOPCHECK \
	if (src >= chan.loopEnd) src = (src - chan.loopEnd) + chan.loopStart;
#define HS_LOOPCHECK_IP \
if (src >= chan.loopEnd - 1) { \
	if (src >= chan.loopEnd || (ih & 0x8000)) \
		src = (src - chan.loopEnd) + chan.loopStart; \
}
#define HS_LOOPCHECK_IP2 \
	bool oneByteOnly = (src == chan.loopEnd - 1); \
	if (src >= chan.loopEnd) \
		src = (src - chan.loopEnd) + chan.loopStart;
#define HS_DOCYCLE(cycCond, srcOp, loopOp) \
	for (cycCond) { \
		loopOp \
		(*dst++) += (srcOp); \
		HS_ADVSRC; \
	}


void HSLowLevelDriver::pcmUpdateChannel(HSSoundChannel &chan) {
	int next = 0;
	if (chan.status == 0) {
		if (chan.mode == 0) {
			if (chan.flags) {
				chan.flags--;
				next = 1;
			} else {
				chan.status = -1;
				if (!(_song._flags & 0x200) && ((int)chan.tickDataLen < (chan.dataEnd - chan.stateCur.dataPos))) {
					chan.mode = -1;
					chan.stateSaved = chan.stateCur;
				}
			}
		} else {
			next = 2;
		}
	}

	if (!next) {
		if (chan.mode == 0) {
			if (chan.status > -1)
				chan.status--;
		} else {
			next = 2;
		}
	}

	if (next < 2) {
		uint16 val = 0;
		for (const uint16 *in = chan.pmData; *in != 0x8000; ) {
			val = *in;
			chan.pmData = ++in;
			if (!(val & 0x8000) || val >= 0x8200)
				break;
			val -= 0x8100;
			in += (val - 1);
		}
		if (val) {
			chan.stateCur.rate += (int16)val * chan.pmRate;
			chan.tickDataLen = 0;
		}

	}
	next = 0;

	const uint8 *src = chan.stateCur.dataPos;
	uint16 *dst = _transBuffer;
	uint32 rate = chan.stateCur.rate;
	if (_updateTypeHq)
		rate >>= 1;
	uint32 ih = (rate & 0xffff) ? chan.stateCur.phase : 0;
	const uint8 *at = chan.stateCur.amplScaleData;

	if (!chan.tickDataLen)
		chan.tickDataLen = (chan.stateCur.rate * _samplesPerTick) >> 16;

	if (chan.mode == 0) {
		if (chan.status && chan.loopEnd)
			next = (src + chan.tickDataLen > chan.loopEnd) ? 2 : 1;

		if (next == 2) {
			if (chan.numLoops) {
				chan.status = 0x7fff;
				if (*chan.numLoops == 0 || (*chan.numLoops != -1 && --*chan.numLoops == 0))
					next = 1;
			}
		}

		if (next == 1 || (int)chan.tickDataLen < (chan.dataEnd - src)) {
			if (!(rate & 0xffff) || chan.imode == kNone) {
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF((_transCycleLenDef + 1) * 5), at[*src], HS_VOID)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF((_transCycleLenDef + 1) * 5), *src, HS_VOID)
				}
			} else if (chan.imode == kSimple) {
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), ih & 0x8000 ? at[(src[0] + src[1]) >> 1] : at[*src], HS_VOID)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), (ih & 0x8000 ? ((src[0] + src[1]) >> 1) : *src), HS_VOID)
				}
			} else if (chan.imode == kTable) {
				const uint8 *s1 = _interpolationTable;
				const uint8 *s2 = _interpolationTable2;
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), at[s1[src[0]] + s2[src[1]]], HS_VOID)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), (uint8)(s1[src[0]] + s2[src[1]]), HS_VOID)
				}
			}

		} else if (next == 0) {
			int i = 0;
			if (chan.stateCur.velocity) {
				HS_DOCYCLE(HS_CYCL_NOINIEX(_transCycleLenInter + 1, src < chan.dataEnd), at[*src], HS_VOID)
			} else {
				HS_DOCYCLE(HS_CYCL_NOINIEX(_transCycleLenInter + 1, src < chan.dataEnd), *src, HS_VOID)
			}
			for (; i <= _transCycleLenInter; ++i)
					(*dst++) += 0x80;

			chan.status = -1;

		} else if (next == 2) {
			if (!(rate & 0xffff) || chan.imode == kNone) {
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF((_transCycleLenDef + 1) * 5), at[*src], HS_LOOPCHECK)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF((_transCycleLenDef + 1) * 5), *src, HS_LOOPCHECK)
				}
			} else if (chan.imode == kSimple) {
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), (ih & 0x8000 ? at[(src[ih >> 16] + src[(ih >> 16) + 1]) >> 1] : at[src[ih >> 16]]), HS_LOOPCHECK_IP)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), (ih & 0x8000 ? ((src[0] + src[1]) >> 1) : *src), HS_LOOPCHECK_IP)
				}
			} else if (chan.imode == kTable) {
				const uint8 *s1 = _interpolationTable;
				const uint8 *s2 = _interpolationTable2;
				if (chan.stateCur.velocity) {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), at[oneByteOnly ? *src : (s1[(ih & 0xff00) | src[0]] + s2[(ih & 0xff00) | src[1]]) & 0xff], HS_LOOPCHECK_IP2)
				} else {
					HS_DOCYCLE(HS_CYCL_DEF(_transCycleLenInter + 1), (oneByteOnly ? *src : (s1[(ih & 0xff00) | src[0]] + s2[(ih & 0xff00) | src[1]]) & 0xff), HS_LOOPCHECK_IP2)
				}
			}
		}
	} else if (chan.mode < 0) {
		chan.mode = 0;
		rate = chan.stateSaved.rate;
		ih = chan.stateSaved.phase;
		src = chan.stateSaved.dataPos;
		at = chan.stateSaved.amplScaleData;
		if (_updateTypeHq)
			rate >>= 1;

		uint8 v1 = *src;
		uint8 v2 = 0;
		int i = 0;

		if (chan.stateSaved.velocity) {
			for (; i <= _transCycleLenInter; ++i) {
				v2 = *src;
				v1 ^= v2;
				if (v1 & 0x80)
					break;
				v1 = v2;
				(*dst++) += at[v2];
				ih += rate;
				src += (ih >> 16);
				ih &= 0xffff;
			}
			if (i <= _transCycleLenInter) {
				v1 = v1 ^ v2;
				v2 = at[(v2 + v1) >> 1];
			}
		} else {
			for (; i <= _transCycleLenInter; ++i) {
				v2 = *src;
				v1 ^= v2;
				if (v1 & 0x80)
					break;
				v1 = v2;
				(*dst++) += v2;
				ih += rate;
				src += (ih >> 16);
				ih &= 0xffff;
			}
			if (i <= _transCycleLenInter) {
				v1 = v1 ^ v2;
				v2 = (v2 + v1) >> 1;
			}
		}
		for (; i <= _transCycleLenInter; ++i) {
			(*dst++) += v2;
			v2 = ((v2 - 0x80) >> 1) + 0x80;
		}

		ih = 0;
		src = chan.stateCur.dataPos;

	} else {
		chan.mode = 0;
		for (int i = 0; i < (_transCycleLenDef + 1) * 5; ++i)
			(*dst++) += 0x80;
		ih = 0;
	}

	chan.stateCur.phase = ih & 0xffff;
	chan.stateCur.dataPos = src;
}

#undef HS_CYCL_DEF
#undef HS_CYCL_NOINIEX
#undef HS_ADVSRC
#undef HS_DOCYCLE
#undef HS_LOOPCHECK
#undef HS_LOOPCHECK_IP
#undef HS_LOOPCHECK_IP2
#undef HS_VOID

template<typename T> void HSLowLevelDriver::fillBuffer(T *dst) {
	const uint16 *src = _transBuffer;

	if (sizeof(T) == 2) {
		int16 offset = (int16)(_song._numChanMusic + _song._numChanSfx) << 7;
		if (_updateTypeHq || _pcmDstBufferSize != 370) {
			for (int i = 0; i < _pcmDstBufferSize; ++i)
				*dst++ = (T)(*src++) - offset;
		} else {
			uint16 a = *src++;
			for (int i = 0; i < 92; ++i) {
				uint16 b = *src++;
				*dst++ = (T)a - offset;
				*dst++ = (T)((a + b) >> 1) - offset;
				*dst++ = (T)b - offset;
				a = *src++;
				*dst++ = (T)((a + b) >> 1) - offset;
			}
			*dst++ = (T)a - offset;
			*dst++ = (T)a - offset;
		}
	} else {
		if (_updateTypeHq || _pcmDstBufferSize != 370) {
			for (int i = 0; i < _pcmDstBufferSize; ++i)
				*dst++ = _sampleConvertBuffer[*src++];
		} else {
			uint16 a = *src++;
			for (int i = 0; i < 92; ++i) {
				uint16 b = *src++;
				*dst++ = _sampleConvertBuffer[a];
				*dst++ = _sampleConvertBuffer[(a + b) >> 1];
				*dst++ = _sampleConvertBuffer[b];
				a = *src++;
				*dst++ = _sampleConvertBuffer[(a + b) >> 1];
			}
			*dst++ = _sampleConvertBuffer[a];
			*dst++ = _sampleConvertBuffer[a];
		}
	}
}

bool HSLowLevelDriver::songStart() {
	if (!songInit())
		return false;
	createTables();

	return true;
}

bool HSLowLevelDriver::songInit() {
	_song.reset();
	return _midi->loadTracks(_song);
}

void HSLowLevelDriver::songStopAllChannels() {
	for (int i = 0; i < _song._numChanMusic; ++i)
		_chan[i].status = -1;
}

void HSLowLevelDriver::songNextTick() {
	if (!_song._ready)
		return;

	if (!_midi->nextTick(_song) && _song._loop)
		songInit();
}

bool HSLowLevelDriver::songIsPlaying() {
	if (!_song._ready)
		return false;
	if (_song._loop)
		return true;

	return _midi->isPlaying();
}

void HSLowLevelDriver::noteOn(uint8 part, uint8 prg, uint8 note, uint8 velo, uint16 ticker, const void *handle) {
	if (_song._scan) {
		_instruments[prg].status = InstrumentEntry::kRequestLoad;
		return;
	}

	const uint8 *snd = nullptr;
	const NoteRangeSubset *nrs = nullptr;
	uint16 note2 = 0;
	uint16 flags = 0;
	uint8 flags2 = 0;
	int16 nrTranspose = 0;

	for (int lp = 1; lp; --lp) {
		note2 = note;
		flags = _instruments[prg].flags;
		flags2 = _instruments[prg].flags2;

		if (_instruments[prg].transpose)
			note2 = note2 + 60 - _instruments[prg].transpose;

		if (!_instruments[prg].noteRangeSubsets.size())
			break;

		uint8 ntier = 0;
		for (int i = 0; i < 11; ++i) {
			ntier = note2 & 0xff;

			if (flags & 0x100)
				ntier = noteFromTable() & 0x7f;

			if (flags2 & 1)
				ntier = velo;

			int num = _instruments[prg].noteRangeSubsets.size();
			nrTranspose = 0;

			for (int ii = 0; ii < num && !nrs; ++ii) {
				uint8 liml = _instruments[prg].noteRangeSubsets[ii].rmin;
				uint8 limu = _instruments[prg].noteRangeSubsets[ii].rmax;

				if (liml && liml > ntier) {
					nrTranspose = 12;
				} else if (limu < 127 && limu < ntier) {
					nrTranspose = -12;
				} else {
					nrs = &_instruments[prg].noteRangeSubsets[ii];
					break;
				}
			}
			if (nrs)
				break;
			if (flags & 0x100)
				continue;
			if (!(flags & 0x2000))
				return;
			if ((note2 += nrTranspose) & 0x80)
				break;
		}

		if (flags & 0x800) {
			if (_instruments[prg].refinst) {
				prg = _instruments[prg].refinst;
				++lp;
			}
		}
	}

	snd = (nrs && nrs->sndRes.ptr) ? nrs->sndRes.ptr : _instruments[prg].sndRes.ptr;
	if (!snd)
		return;

	if (!(_song._amplitudeScaleFlags & 2) || (!(_song._amplitudeScaleFlags & 4) && !(flags2 & 0x40)))
		velo = 0;

	if (!_song._numChanMusic)
		return;

	int busy = 0;
	HSSoundChannel *chan = nullptr;

	for (int i = 0; i < _song._numChanMusic && !chan; ++i) {
		HSSoundChannel *c = &_chan[i];
		if (c->status >= 0)
			++busy;
		if (c->note != note || c->id != part || c->prg != prg || c->handle != handle)
			continue;
		if ((flags & 0x400) || (c->status == 0))
			chan = c;
	}

	if (!chan) {
		int srchStatus = ((_song._flags & 0x4000) && (_song._convertUnitSize <= busy)) ? 0 : -1;
		for (int a = 0; a < 2 && !chan; ++a) {
			for (int i = 0; i < _song._numChanMusic && !chan; ++i) {
				HSSoundChannel *c = &_chan[i];
				if (c->status == srchStatus)
					chan = c;
			}
			srchStatus ^= -1;
		}
	}

	if (!chan)
		return;

	chan->handle = handle;
	chan->id = part;

	if (!(_song._flags & 0x200)) {
		chan->mode = 1;
		if (chan->status >= 0 && chan->tickDataLen && (int)chan->tickDataLen < (chan->dataEnd - chan->stateCur.dataPos)) {
			chan->mode = -1;
			chan->stateSaved = chan->stateCur;
		}
	}

	uint16 type = READ_BE_UINT16(snd);
	uint16 n = 0;
	if (type == 1 || type == 2) {
		uint16 numTypes = (type == 1) ? READ_BE_UINT16(snd + 2) : 0;
		n = READ_BE_UINT16(snd + numTypes * 6 + 4) * 8;
		snd = snd + n + numTypes * 6 + 6;
	} else {
		warning("void HSLowLevelDriver::noteOn((): Corrupted sound resource.  Instrument ID: %d", prg);
		return;
	}

	chan->stateCur.dataPos = snd + 22;
	chan->dataEnd = chan->stateCur.dataPos + READ_BE_UINT32(snd + 4);
	chan->loopStart = chan->loopEnd = nullptr;
	uint32 loopStart = READ_BE_UINT32(snd + 12);
	uint32 loopEnd = READ_BE_UINT32(snd + 16);

	if (loopStart && loopEnd && (loopEnd - loopStart >= 100) && !(flags2 & 0x20)) {
		chan->loopStart = chan->stateCur.dataPos + loopStart;
		chan->loopEnd = chan->stateCur.dataPos + loopEnd;
	}

	chan->numLoops = nullptr;
	chan->imode = (!(flags & 0x8000) && (((_song._flags & 0x2000) || ((_song._flags & 0x1000) && (flags2 & 0x80 || _song._interpolateType == n))))) ? _interpolationMode : kNone;

	chan->prg = prg;
	chan->note = note;
	chan->flags = _song._flags & 0x3f;

	if (flags & 0x4000) {
		chan->stateCur.rate = 0x20000;
	} else {
		n = 60 + note2 - READ_BE_UINT16(snd + 20);
		assert(n < ARRAYSIZE(_periods));
		chan->stateCur.rate = _periods[n];
		if ((chan->stateCur.rate & 0xffff) < 4)
			chan->stateCur.rate &= ~0xffff;
	}

	chan->pmRate = chan->stateCur.rate >> 10;

	if ((flags & 0x200) && (_song._amplitudeScaleFlags & 0x10))
		chan->stateCur.rate += ((chan->pmRate * noteFromTable()) >> 16);

	chan->pmData = (const uint16*)_instruments[prg].pmData.ptr;
	chan->tickDataLen = 0;
	chan->status = ticker;

	if (velo > 125)
		velo = 0;

	chan->stateCur.velocity = velo;
	chan->stateCur.amplScaleData = &_amplitudeScaleBuffer[(velo ? velo : 127) << 8];
	chan->stateCur.phase = 0;
}

void HSLowLevelDriver::noteOff(uint8 part, uint8 note, const void *handle) {
	for (int i = 0; i < _song._numChanMusic; ++i) {
		HSSoundChannel *c = &_chan[i];
		if (c->status < 0 || c->note != note || c->id != part || c->handle != handle)
			continue;
		c->status = 0;
		c->flags = _song._flags & 0x3f;
	}
}

int16 HSLowLevelDriver::noteFromTable() {
	int16 val = _wtable[_wtableCount2] = _wtable[_wtableCount];
	if (++_wtableCount == 17)
		_wtableCount = 0;
	if (++_wtableCount2 == 17)
		_wtableCount2 = 0;
	return val;
}

void HSLowLevelDriver::loadInstrument(int id) {
	assert(id < 128);
	if (_instruments[id].status == InstrumentEntry::kUnusable)
		return;

	Common::SeekableReadStream *inst = _res->getResource(id, 'INST');
	if (!inst && _instruments[id].status != InstrumentEntry::kReady)
		error("HSLowLevelDriver::loadInstrument(): Failed to load INST resource '0x02%x'", id);

	uint16 sndId = inst->readUint16BE();
	_instruments[id].transpose = inst->readSint16BE();
	inst->skip(1);
	_instruments[id].flags2 = inst->readByte();
	_instruments[id].flags = inst->readUint16BE();

	uint8 smodId = _instruments[id].flags & 0x0f;
	uint16 smodPara1 = inst->readUint16BE();
	uint16 smodPara2 = inst->readUint16BE();

	ShStBuffer tmp = loadInstrumentSamples(sndId, !(_instruments[id].flags & 0x1000));
	if ((_instruments[id].flags & 0x1000) && smodPara1 && smodPara2)
		tmp = processWithEffect(tmp, smodId, smodPara1, smodPara2);
	_instruments[id].sndRes = tmp;
	_instruments[id].refinst = 0;

	if (_instruments[id].flags & 0x800) {
		inst->skip(4);
		_instruments[id].refinst = inst->readUint16BE();
		delete inst;
		return;
	}

	_instruments[id].noteRangeSubsets.clear();
	for (int num = inst->readUint16BE(); num; --num) {
		uint8 liml = inst->readByte();
		uint8 limu = inst->readByte();
		if (!(sndId = inst->readUint16BE()))
			continue;

		smodPara1 = inst->readUint16BE();
		smodPara2 = inst->readUint16BE();

		tmp = loadInstrumentSamples(sndId, !((_instruments[id].flags & 0x1000) && smodPara1));
		if ((_instruments[id].flags & 0x1000) && smodPara1)
			tmp = processWithEffect(tmp, smodId, smodPara1, smodPara2);
		_instruments[id].noteRangeSubsets.push_back(NoteRangeSubset(liml, limu, tmp));
	}

	inst->skip(2);
	int pmlen = (inst->size() - 4 - inst->pos()) >> 1;
	if (pmlen) {
		uint16 *pmdata = new uint16[pmlen];
		for (int i = 0; i < pmlen; ++i)
			pmdata[i] = inst->readUint16BE();
		_instruments[id].pmData = ShStBuffer(pmdata, pmlen << 1, true);
		delete[] pmdata;
	}
	delete inst;
}

ShStBuffer HSLowLevelDriver::loadInstrumentSamples(int id, bool sharedBuffer) {
	if (sharedBuffer) {
		for (Common::Array<InstrSamples>::const_iterator i = _instrumentsSharedSamples.begin(); i != _instrumentsSharedSamples.end(); ++i) {
			if (i->_id == id)
				return i->_resource;
		}
	}

	Common::SeekableReadStream *snd = _res->getResource(id, MKTAG('s', 'n', 'd', ' '));
	if (!snd) {
		// This happens from time to time, but apparently not with resources that are actually
		// meant to be used. So I don't see any value in throwing a warning here...
		// warning("HSLowLevelDriver::loadInstrumentSamples(): Failed to load snd resource '0x02%x'", id);
		return ShStBuffer();
	}

	ShStBuffer res(snd);
	if (sharedBuffer)
		_instrumentsSharedSamples.push_back(InstrSamples(id, res));
	delete snd;

	return res;
}

ShStBuffer HSLowLevelDriver::processWithEffect(const ShStBuffer &buf, uint16 smodId, uint16 para1, uint16 para2) {
	const uint8 *src = buf.ptr;
	if (!src)
		return ShStBuffer();

	if (smodId >= _hsFilters.size())
		error("HSLowLevelDriver::processWithEffect(): Invalid effect id");

	uint8 *dst = new uint8[buf.len];
	_hsFilters[smodId]->process(buf, dst, para1, para2);

	ShStBuffer res(dst, buf.len, true);
	delete[] dst;

	return res;
}

const uint32 HSLowLevelDriver::_periods[156] = {
	0x00001000, 0x000010f3, 0x000011f5, 0x00001307, 0x00001427, 0x00001557, 0x0000169e, 0x000017f4,
	0x00001961, 0x00001ae4, 0x00001c7f, 0x00001e30, 0x00002000, 0x000021e6, 0x000023eb, 0x0000260e,
	0x0000284f, 0x00002aaf, 0x00002d3d, 0x00002fe9, 0x000032c2, 0x000035c9, 0x000038fe, 0x00003c60,
	0x00004000, 0x000043cc, 0x000047d6, 0x00004c1c, 0x0000509f, 0x0000555f, 0x00005a7a, 0x00005fd2,
	0x00006585, 0x00006b93, 0x000071fd, 0x000078c1, 0x00008000, 0x00008799, 0x00008fac, 0x00009839,
	0x0000a13f, 0x0000aabf, 0x0000b4f5, 0x0000bfa5, 0x0000cb0b, 0x0000d727, 0x0000e3fa, 0x0000f183,
	0x00010000, 0x00010f33, 0x00011f59, 0x00013072, 0x0001427e, 0x0001557e, 0x000169ea, 0x00017f4a,
	0x00019616, 0x0001ae4f, 0x0001c7f5, 0x0001e307, 0x00020000, 0x00021e66, 0x00023eb2, 0x000260e4,
	0x000284fd, 0x0002aafc, 0x0002d3d5, 0x0002fe94, 0x00032c2c, 0x00035c9e, 0x00038fea, 0x0003c60f,
	0x00040001, 0x00043ccc, 0x00047d64, 0x0004c1c9, 0x000509fb, 0x000555f9, 0x0005a7aa, 0x0005fd28,
	0x00065859, 0x0006b93d, 0x00071fd4, 0x00078c1f, 0x00080002, 0x00087999, 0x0008fac9, 0x00098393,
	0x000a13f6, 0x000aabf2, 0x000b4f55, 0x000bfa50, 0x000cb0b3, 0x000d727b, 0x000e3fa9, 0x000f183e,
	0x00100005, 0x0010f332, 0x0011f593, 0x00130726, 0x001427ec, 0x001557e4, 0x00169eaa, 0x0017f4a1,
	0x00196166, 0x001ae4f6, 0x001c7f53, 0x001e307c, 0x0020000a, 0x0021e665, 0x0023eb26, 0x00260e4c,
	0x00284fd8, 0x002aafc9, 0x002d3d54, 0x002fe943, 0x0032c2cc, 0x0035c9ed, 0x0038fea6, 0x003c60f8,
	0x00400014, 0x0043ccca, 0x0047d64c, 0x004c1c98, 0x00509fb0, 0x00555f92, 0x005a7aa8, 0x005fd286,
	0x00658598, 0x006b93da, 0x0071fd4c, 0x0078c1f0, 0x00400014, 0x0043ccca, 0x0047d64c, 0x004c1c98,
	0x00509fb0, 0x00555f92, 0x005a7aa8, 0x005fd286, 0x00658598, 0x006b93da, 0x0071fd4c, 0x0078c1f0,
	0x00400014, 0x0043ccca, 0x0047d64c, 0x004c1c98, 0x00509fb0, 0x00555f92, 0x005a7aa8, 0x005fd286,
	0x00658598, 0x006b93da, 0x0071fd4c, 0x0078c1f0
};

HSSoundSystem *HSSoundSystem::_refInstance = nullptr;
int HSSoundSystem::_refCount = 0;

HSSoundSystem::HSSoundSystem(SoundMacRes *res, Audio::Mixer *mixer) : _res(res), _mixer(mixer), _driver(nullptr), _voicestr(nullptr), _vblTask(nullptr), _sampleSlots(nullptr), _voices(nullptr), _sync(0),
_numChanSfx(0), _numSampleSlots(0), _currentSong(-1), _ready(false), _isFading(false), _sfxDuration(0), _fadeState(0), _fadeStep(0), _fadeStepTicksCounter(0), _fadeDirection(false),
_fadeComplete(false), _fadeStepTicks(0), _volumeMusic(Audio::Mixer::kMaxMixerVolume), _volumeSfx(Audio::Mixer::kMaxMixerVolume), _mutex(mixer->mutex()) {
	DEBUG_BUFFERS_COUNT = 0;
}

HSSoundSystem::~HSSoundSystem() {
	_mixer->stopHandle(_soundHandle);
	Common::StackLock lock(_mutex);
	delete _driver;
	delete _vblTask;
	setupSfxChannels(0);
	assert(!DEBUG_BUFFERS_COUNT);
	releaseSamples();
}

HSSoundSystem *HSSoundSystem::open(SoundMacRes *res, Audio::Mixer *mixer) {
	_refCount++;

	if (_refCount == 1 && _refInstance == nullptr)
		_refInstance = new HSSoundSystem(res, mixer);
	else if (_refCount < 2 || _refInstance == nullptr)
		error("HSSoundSystem::open(): Internal ref management failure");

	return _refInstance;
}

void HSSoundSystem::close() {
	if (!_refCount)
		return;

	_refCount--;

	if (!_refCount) {
		delete _refInstance;
		_refInstance = nullptr;
	}
}

bool HSSoundSystem::init(bool hiQuality, uint8 interpolationMode, uint8 numChanSfx, bool output16bit) {
	if (_ready)
		return true;

	_driver = new HSLowLevelDriver(_res, _mutex);
	_voicestr = _driver->init(_mixer->getOutputRate(), output16bit);
	if (!_voicestr)
		return false;
	_voicestr->setMasterVolume(Audio::Mixer::kMusicSoundType, _volumeMusic);
	_voicestr->setMasterVolume(Audio::Mixer::kSFXSoundType, _volumeSfx);

	Common::StackLock lock(_mutex);
	_vblTask = new HSAudioStream::CallbackProc(this, &HSSoundSystem::vblTaskProc);
	_voicestr->setVblCallback(_vblTask);

	assert(interpolationMode < 3);

	if (hiQuality) {
		_driver->send(21, 8 - numChanSfx, 4, numChanSfx);
		_driver->send(24, (interpolationMode << 8) + 22);
	} else {
		_driver->send(21, 4, 2 + numChanSfx, numChanSfx);
		_driver->send(24, (interpolationMode << 8) + 11);
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _voicestr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_ready = true;

	return true;
}

void HSSoundSystem::registerSamples(const uint16 *resList, bool registerOnly) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	releaseSamples();

	for (const uint16 *pos = resList; *pos != 0xffff; ++pos)
		_numSampleSlots++;

	_sampleSlots = new SampleSlot[_numSampleSlots];
	memset(_sampleSlots, 0, _numSampleSlots * sizeof(SampleSlot));

	for (int i = 0; i <_numSampleSlots; ++i)
		loadSamplesIntoSlot(*resList++, _sampleSlots[i], registerOnly);
}

void HSSoundSystem::releaseSamples() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i <_numSampleSlots; ++i)
		releaseSamplesFromSlot(_sampleSlots[i]);

	delete[] _sampleSlots;
	_sampleSlots = nullptr;
	_numSampleSlots = 0;
}

int HSSoundSystem::changeSystemVoices(int numChanMusicTotal, int numChanMusicPoly, int numChanSfx) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return -211;

	setupSfxChannels(numChanSfx);
	_driver->send(21, numChanMusicTotal, numChanMusicPoly, numChanSfx);

	return 0;
}

void HSSoundSystem::startSoundEffect(int id, int rate) {
	if (!_ready)
		return;

	SampleSlot *slot = findSampleSlot(id);
	if (!slot)
		return;

	Common::StackLock lock(_mutex);

	if (slot->reverse) {
		reverseSamples(slot);
		slot->reverse = false;
	}

	playSamples(slot->samples, slot->numSamples, rate ? rate : slot->rate, id, 0, 0, 0, 0, nullptr, nullptr);
}

void HSSoundSystem::enqueueSoundEffect(int id, int rate, int note) {
	if (!_ready || !id || !rate || !note)
		return;

	SampleSlot *s = findSampleSlot(id);
	if (!s)
		return;

	Common::StackLock lock(_mutex);

	assert(note > 21 && note < 80);
	_sfxQueue.push(SfxQueueEntry(id, (s->rate >> 8) * _noteFreq[note - 22], rate * 60 / 1000));
	_sfxDuration = 0;
}

void HSSoundSystem::stopSoundEffect(int id) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	HSSoundEffectVoice *vc = findVoice(id);
	if (!vc)
		return;

	doCommandIntern(101, vc);

	vc->enabled = false;
	vc->duration = 0;
}

void HSSoundSystem::stopAllSoundEffects() {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	for (int i = 0; i < _numChanSfx; ++i) {
		_voices[i]->enabled = false;
		_voices[i]->duration = 0;
	}

	doCommandIntern(103);
}

int HSSoundSystem::doCommand(int cmd, va_list &arg) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return 0;

	int res = 0;
	int tmp = 0;

	switch (cmd) {
	case 0:
	case 1:
		res = startSong(va_arg(arg, int), cmd);
		break;

	case 2:
		if (_currentSong != -1) {
			_driver->send(5);
			_driver->send(7);
			_currentSong = -1;
		}
		break;

	case 3:
		if (_currentSong != -1)
			res = _driver->send(4);
		break;

	case 10:
		tmp = MAX<int>(1, va_arg(arg, int));
		_isFading = _fadeDirection = true;
		_fadeComplete = false;
		_fadeState = 256;
		_fadeStep = 1024 / tmp;
		_fadeStepTicks = _fadeStepTicksCounter = tmp / 4;
		break;

	case 11:
		tmp = MAX<int>(1, va_arg(arg, int));
		_driver->send(19, 0);
		_isFading = true;
		_fadeDirection = _fadeComplete = false;
		_fadeState = 0;
		_fadeStep = 1024 / tmp;
		_fadeStepTicks = _fadeStepTicksCounter = tmp / 4;
		break;

	case 12:
		res = _fadeState;
		break;

	case 13:
		_isFading = false;
		_driver->send(19, CLIP(va_arg(arg, int), 0, 256));
		break;

	case 14:
		tmp = va_arg(arg, int);
		if (tmp & 4) {
			if (tmp & 2)
				_driver->send(24, 0x200 | ((tmp & 1) ? 22 : 11));
			else
				_driver->send(24, 0x100 | ((tmp & 1) ? 22 : 11));
		} else if (tmp & 2) {
			_driver->send(24, (tmp & 1) ? 22 : 11);
		}
		break;

	case 15:
		res = _driver->send(25) ? -1 : changeSystemVoices(16, 8, 0);
		break;

	case 100:
		_driver->send(17, va_arg(arg, const HSSoundEffectVoice*));
		break;

	case 101:
		_driver->send(18, va_arg(arg, const HSSoundEffectVoice*));
		break;

	case 102:
		res = _driver->send(20, va_arg(arg, const HSSoundEffectVoice*));
		break;

	case 103:
		_isFading = false;
		_driver->send(22);
		break;

	default:
		break;
	}

	return res;
}

void HSSoundSystem::setMusicVolume(int volume) {
	_volumeMusic = volume;
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;
	_voicestr->setMasterVolume(Audio::Mixer::kMusicSoundType, volume);
}

void HSSoundSystem::setSoundEffectVolume(int volume) {
	_volumeSfx = volume;
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;
	_voicestr->setMasterVolume(Audio::Mixer::kSFXSoundType, volume);
}

void HSSoundSystem::vblTaskProc() {
	if (!_ready)
		return;

	if (_isFading)
		autoFadeHandler();

	if (_currentSong != -1) {
		if (!_driver->send(4))
			_currentSong = -1;
	}

	for (int ii = 0; ii < _numChanSfx; ++ii) {
		if (!_voices[ii]->dataPtr || !_voices[ii]->enabled)
			continue;

		/*
		if (doCommand(102, _voices[ii]->dataPtr)) {
			_voices[ii]->enabled = false;
			_voices[ii]->b4 = 0;
			_voices[ii]->numLoops = 0;
			if (_voices[ii]->cb)
				(_voices[ii]->cb)();
		}
		*/

		if (_voices[ii]->duration && _voices[ii]->duration != (uint32)-1)
			_voices[ii]->duration--;

		//if (_voices[ii]->b4 && _voices[ii]->vblProc)
		//	_voices[ii]->numLoops = (_voices[ii]->vblProc)() ? (uint16)-1 : 0;
	}

	if (!_sfxQueue.empty() && _sfxDuration == 0) {
		SfxQueueEntry s = _sfxQueue.pop();
		/*if (s->_id == (uint16)-1)
			stopSound(s->_id);
		else*/
			startSoundEffect(s._id, s._rate);

		if (s._duration == (uint16)-1) {
			HSSoundEffectVoice *vc = findVoice(s._id);
			_sfxDuration = vc ? vc->duration : 0;
		} else {
			_sfxDuration = s._duration;
		}
	} else if (!_sfxQueue.empty()) {
		--_sfxDuration;
	}

	++_sync;
}

void HSSoundSystem::setupSfxChannels(int num) {
	for (int i = 0; i < _numChanSfx; ++i)
		delete _voices[i];
	delete[] _voices;
	_voices = nullptr;

	_numChanSfx = num;
	if (num <= 0)
		return;

	_voices = new HSSoundEffectVoice*[_numChanSfx];
	assert(_voices);
	for (int i = 0; i < _numChanSfx; ++i)
		_voices[i] = new HSSoundEffectVoice();
}

HSSoundSystem::HSSoundEffectVoice *HSSoundSystem::findFreeVoice() const {
	HSSoundEffectVoice *chan = nullptr;
	for (int i = 0; i < _numChanSfx; ++i) {
		if (_voices[i] && !_voices[i]->enabled) {
			chan = _voices[i];
			break;
		}
	}

	if (chan)
		return chan;

	uint32 temp = _sync;
	for (int i = 0; i < _numChanSfx; ++i) {
		if (!_voices[i] || _voices[i]->sync >= temp)
			continue;
		temp = _voices[i]->sync;
		chan = _voices[i];
	}

	if (!chan)
		chan = _voices[0];

	if (chan) {
		chan->enabled = false;
		chan->sync = 0;
	}

	return chan;
}

HSSoundSystem::HSSoundEffectVoice *HSSoundSystem::findVoice(uint16 id) const {
	for (int i = 0; i < _numChanSfx; ++i) {
		if (_voices[i] && _voices[i]->resId == id)
			return _voices[i];
	}
	return nullptr;
}

int HSSoundSystem::doCommandIntern(int cmd, ...) {
	va_list args;
	va_start(args, cmd);
	int res = doCommand(cmd, args);
	va_end(args);

	return res;
}

bool HSSoundSystem::loadSamplesIntoSlot(uint16 id, SampleSlot &slot, bool registerOnly) const {
	slot.resId = id;
	slot.data = nullptr;
	uint8 *data = nullptr;

	if (registerOnly)
		return true;

	slot.reverse = false;

	Common::SeekableReadStream *in = _res->getResource(id, MKTAG('c', 's', 'n', 'd'));
	if (in) {
		uint32 inSize = (uint32)in->size() - 4;
		uint32 outSize = in->readUint32BE();
		uint8 *tmp = new uint8[inSize];
		in->read(tmp, inSize);
		data = new uint8[outSize];
		memset(data, 0, outSize);
		deltaDecompress(data, tmp, outSize, inSize);
		delete[] tmp;
	} else if ((in = _res->getResource(id, MKTAG('s', 'n', 'd', ' ')))) {
		uint32 inSize = (uint32)in->size();
		data = new uint8[inSize];
		in->read(data, inSize);
	}
	delete in;

	uint16 type = data ? READ_BE_UINT16(data) : 0;
	if (type == 1 || type == 2) {
		uint16 numTypes = (type == 1) ? READ_BE_UINT16(data + 2) : 0;
		const uint8 *header = data + READ_BE_UINT16(data + numTypes * 6 + 4) * 8 + numTypes * 6 + 6;
		slot.samples = header + 22;
		slot.numSamples = READ_BE_UINT32(header + 4);
		slot.rate = READ_BE_UINT32(header + 8);
		slot.loopStart = READ_BE_UINT32(header + 12);
		slot.loopEnd = READ_BE_UINT32(header + 16);
	} else if (type) {
		warning("SoundSystem::loadSamplesIntoSlot(): Unexpected resource header type '%d' encountered", type);
		delete[] data;
		data = nullptr;
	}

	slot.data = data;

	return data;
}

void HSSoundSystem::deltaDecompress(uint8 *out, uint8 *in, uint32 outSize, uint32 inSize) const {
	error("SoundSystem::deltaDecompress(): Not implemented");
}

void HSSoundSystem::releaseSamplesFromSlot(SampleSlot &slot) {
	delete[] slot.data;
	slot.data = slot.samples = nullptr;
}

HSSoundSystem::SampleSlot *HSSoundSystem::findSampleSlot(int id) const {
	SampleSlot *res = nullptr;
	for (int i = 0; i <_numSampleSlots; ++i) {
		SampleSlot &s = _sampleSlots[i];
		if (s.resId != id)
			continue;
		if (s.data || loadSamplesIntoSlot(id, s, false))
			res = &s;
		break;
	}

	return res;
}

void HSSoundSystem::playSamples(const uint8 *data, uint32 numSamples, uint32 samplingRate, uint16 id, uint32 loopStart, uint32 loopEnd, int numLoops, int unk4, void *vblproc, void *cb) {
	HSSoundEffectVoice *vc = findFreeVoice();
	if (!data || !vc)
		return;

	vc->duration = (numLoops != -1) ? calculatePlaybackDuration(numSamples, samplingRate) : (uint32)-1;
	vc->sync = _sync;
	vc->loopStartDuration = calculatePlaybackDuration(loopStart, samplingRate);
	vc->loopEndDuration = calculatePlaybackDuration(loopEnd, samplingRate) - loopStart;
	vc->dataPtr = data;
	vc->numSamples = numSamples;
	vc->rate = samplingRate;
	vc->resId = id;
	vc->loopStart = loopStart;
	vc->loopEnd = loopEnd;
	vc->numLoops = numLoops;
	vc->vblProc = vblproc;
	vc->cb = cb;
	vc->b4 = unk4;
	vc->enabled = true;
	doCommandIntern(100, vc);
}

void HSSoundSystem::reverseSamples(SampleSlot *slot) {
	if (!slot || !slot->data)
		return;
	int headerSize = (int)(slot->samples - slot->data);
	uint8 *dataNew = new uint8[headerSize + slot->numSamples];
	memcpy(dataNew, slot->data, headerSize);
	uint8 *smpNew = dataNew + headerSize;
	const uint8 *in = slot->samples + slot->numSamples - 1;
	uint8 *dst = smpNew;
	for (uint32 i = 0; i < slot->numSamples; ++i)
		*dst++ = *in--;
	delete[] slot->data;
	slot->data = dataNew;
	slot->samples = smpNew;
}

uint32 HSSoundSystem::calculatePlaybackDuration(uint32 numSamples, uint32 samplingRate) {
	return (samplingRate >> 16) ? numSamples * 60 / (samplingRate >> 16) : 0;
}

int HSSoundSystem::startSong(int id, int loop) {
	Common::SeekableReadStream *song = _res->getResource(id, MKTAG('S', 'O', 'N', 'G'));
	if (!song)
		return -192;
	uint16 idm = song->readUint16BE();
	delete song;

	Common::SeekableReadStream *midi = _res->getResource(idm, MKTAG('M', 'I', 'D', 'I'));
	if (!midi && !(midi = _res->getResource(idm, MKTAG('M', 'i', 'd', 'i'))))
		return -1300;
	delete midi;

	if (_currentSong != -1)
		doCommandIntern(2, 0);

	_driver->send(13, loop);
	_driver->send(0, id);
	if (_isFading)
		_driver->send(19, _fadeState);

	_currentSong = id;
	return 0;
}

void HSSoundSystem::autoFadeHandler() {
	_fadeStepTicksCounter--;
	if (_fadeDirection) {
		if (_fadeComplete) {
			_isFading = false;
			_driver->send(19, 256);
			stopAllSoundEffects();
		} else if (_fadeStepTicksCounter < 0) {
			_fadeStepTicksCounter = _fadeStepTicks;
			if (_fadeState >= 0) {
				_fadeState -= _fadeStep;
				if (_fadeState >= 0)
					_driver->send(19, _fadeState);
			} else {
				_fadeComplete = true;
				doCommandIntern(2, 0);
				_driver->send(6);
				_driver->send(19, 0);
			}
		}
	} else {
		if (_fadeComplete) {
			_isFading = false;
			_driver->send(19, 256);
			_fadeState = 256;
		} else if (_fadeStepTicksCounter < 0) {
			_fadeStepTicksCounter = _fadeStepTicks;
			if (_fadeState > 256) {
				_fadeComplete = true;
			} else {
				_fadeState += _fadeStep;
				if (_fadeState < 256)
					_driver->send(19, _fadeState);
			}
		}
	}
}

const uint16 HSSoundSystem::_noteFreq[58] = {
	0x001c, 0x001e, 0x0020, 0x0022, 0x0024, 0x0026, 0x0028, 0x002b,
	0x002d, 0x0030, 0x0033, 0x0036, 0x0039, 0x003c, 0x0040, 0x0044,
	0x0048, 0x004c, 0x0051, 0x0055, 0x005a, 0x0060, 0x0066, 0x006c,
	0x0072, 0x0079, 0x0080, 0x0088, 0x0090, 0x0098, 0x00a1, 0x00ab,
	0x00b5, 0x00c0, 0x00cb, 0x00d7, 0x00e4, 0x00f2, 0x0100, 0x010f,
	0x011f, 0x0130, 0x0143, 0x0156, 0x016a, 0x0180, 0x0196, 0x01af,
	0x01c8, 0x01e3, 0x0200, 0x021e, 0x023f, 0x0261, 0x0285, 0x02ab,
	0x02d4, 0x02d4
};

uint32 copySndHeader(const uint8 *&src, uint8 *&dst) {
	uint16 type = src ? READ_BE_UINT16(src) : 0;
	uint32 len = 0;
	if (type == 1 || type == 2) {
		uint16 numTypes = (type == 1) ? READ_BE_UINT16(src + 2) : 0;
		const uint8 *header = src + READ_BE_UINT16(src + numTypes * 6 + 4) * 8 + numTypes * 6 + 6;
		len = header + 22 - src;
		memcpy(dst, src, len);
		src += len;
		dst += len;
	}
	return len;
}

bool HSVolumeScaler::process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16 para2) {
	if (!dst)
		return false;

	if (!para1)
		para1 = 1;
	if (!para2)
		para2 = 1;

	const uint8 *s = src.ptr;
	uint32 len = src.len - copySndHeader(s, dst);

	for (uint32 i = 0; i < len; ++i) {
		int16 a = (int16)*s++;
		a = (a - 128) * para1;
		if (a > 0)
			a += (para2 << 1);
		else
			a -= (para2 << 1);
		a = CLIP<int16>(a / para2, -128, 127) + 128;
		*dst++ = (a & 0xff);
	}

	return true;
}

bool HSLowPassFilter::process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16 para2) {
	if (!dst)
		return false;

	if (!para2)
		para2 = 1;

	para1 &= 0xff;

	int cnt = para2;
	bool swtch = true;
	int16 v = 0;

	const uint8 *s = src.ptr;
	for (uint32 len = src.len - copySndHeader(s, dst); len; --len) {
		uint8 in = *s++;
		if ((int8)in <= 0) {
			if (swtch)
				v = 0x80;
			swtch = false;
			if (in > v) {
				v = in;
			} else if (!--cnt) {
				cnt = para2;
				if ((int8)(v + para1) < 0)
					v += para1;
			}
		} else {
			if (!swtch)
				v = 0x7f;
			swtch = true;
			if (in <= v) {
				v = in;
			} else if (!--cnt) {
				cnt = para2;
				if ((int8)(v - para1) > 0)
					v -= para1;
			}
		}
		*dst++ = (v & 0xff);
	}

	return true;
}

bool HSSquareOMatic::process(const ShStBuffer &src, uint8 *dst, uint16 para1, uint16) {
	if (!dst)
		return false;

	if (!para1)
		para1 = 1;

	const uint8 *s = src.ptr;
	uint32 len = src.len - copySndHeader(s, dst) - 1;
	uint8 a = *s++;

	while (len--) {
		uint8 b = *s;
		if (a <= b) {
			b -= a;
			if (para1 <= b)
				b = para1;
			a += b;
		} else if (a > b) {
			b -= a;
			if (para1 <= (uint8)-b)
				b = (uint8)-para1;
			a += b;
		}
		*dst++ = a;
		++s;
	}

	return true;
}

bool HSTriangulizer::process(const ShStBuffer &src, uint8 *dst, uint16, uint16) {
	if (!dst)
		return false;

	const uint8 *s = src.ptr;
	uint32 len = src.len - copySndHeader(s, dst) - 6;
	uint8 a = 0;
	uint8 b = *dst++ = *s++;
	uint8 c = *dst++ = *s++;
	uint8 d = *dst++ = *s++;
	uint8 e = *dst++ = *s++;

	while (len--) {
		a = b;
		b = c;
		c = d;
		d = e;
		e = *s++;
		int16 v = a - b - b + (c << 3) + c + c - d - d + e;
		*dst++ = CLIP<int16>(v, 0, 2047) >> 3;
	}

	*dst++ = *s++;
	*dst++ = *s++;

	return true;
}

HalestormDriver::HalestormDriver(SoundMacRes *res, Audio::Mixer *mixer) : _hs(nullptr) {
	_hs = HSSoundSystem::open(res, mixer);
}

HalestormDriver::~HalestormDriver() {
	HSSoundSystem::close();
	_hs = nullptr;
}

bool HalestormDriver::init(bool hiQuality, InterpolationMode imode, int numChanSfx, bool output16bit) {
	return _hs->init(hiQuality, (uint8)imode, numChanSfx, output16bit);
}

void HalestormDriver::registerSamples(const uint16 *resList, bool registerOnly) {
	_hs->registerSamples(resList, registerOnly);
}

void HalestormDriver::releaseSamples() {
	_hs->releaseSamples();
}

int HalestormDriver::changeSystemVoices(int numChanMusicTotal, int numChanMusicPoly, int numChanSfx) {
	return _hs->changeSystemVoices(numChanMusicTotal, numChanMusicPoly, numChanSfx);
}

void HalestormDriver::stopSoundEffect(int id) {
	_hs->stopSoundEffect(id);
}

void HalestormDriver::stopAllSoundEffects() {
	_hs->stopAllSoundEffects();
}

void HalestormDriver::startSoundEffect(int id, int rate) {
	_hs->startSoundEffect(id, rate);
}

void HalestormDriver::enqueueSoundEffect(int id, int rate, int note) {
	_hs->enqueueSoundEffect(id, rate, note);
}

int HalestormDriver::doCommand(int cmd, ...) {
	va_list args;
	va_start(args, cmd);
	int res = _hs->doCommand(cmd, args);
	va_end(args);

	return res;
}

void HalestormDriver::setMusicVolume(int volume) {
	_hs->setMusicVolume(volume);
}

void HalestormDriver::setSoundEffectVolume(int volume) {
	_hs->setSoundEffectVolume(volume);
}

} // End of namespace Kyra
