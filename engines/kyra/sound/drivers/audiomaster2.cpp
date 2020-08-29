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

#ifdef ENABLE_EOB

#include "kyra/kyra_v1.h"
#include "kyra/sound/drivers/audiomaster2.h"

#include "audio/mods/paula.h"

#include "common/iff_container.h"

namespace Kyra {

class AudioMaster2ResourceManager;
class AudioMaster2Internal;
class SoundResource;

class AudioMaster2IOManager {
public:
	AudioMaster2IOManager();
	~AudioMaster2IOManager();

	struct IOUnit {
		IOUnit() : _next(0), _sampleData(0), _sampleDataRepeat(0), _lenOnce(0), _lenRepeat(0), _startTick(0), _endTick(0), _transposeData(0), _rate(0), _period(0), _transposePara(0), _transposeDuration(0),
			_levelAdjustData(0), _volumeSetting(0), _outputVolume(0), _levelAdjustPara(0), _levelAdjustDuration(0), _fadeOutState(-1), _flags(0) {}

		IOUnit *_next;
		const int8 *_sampleData;
		const int8 *_sampleDataRepeat;
		uint32 _lenOnce;
		uint32 _lenRepeat;
		uint32 _startTick;
		uint32 _endTick;
		const uint8 *_transposeData;
		uint16 _rate;
		uint16 _period;
		uint16 _transposePara;
		uint8 _transposeDuration;
		const uint8 *_levelAdjustData;
		uint16 _volumeSetting;
		uint16 _outputVolume;
		int16 _levelAdjustPara;
		uint8 _levelAdjustDuration;
		int16 _fadeOutState;
		uint8 _flags;
	};

	void clearChain();
	void deployChannels(IOUnit **dest);
	IOUnit *requestFreeUnit();

	void fadeOut();
	bool isFading();

	uint32 _sync;
	uint32 _tempo;

private:
	IOUnit *_units[8];
	IOUnit *_ioChain;
};

class AudioMaster2Internal : public Audio::Paula {
friend class AudioMaster2IOManager;
private:
	AudioMaster2Internal(Audio::Mixer *mixer);
public:
	~AudioMaster2Internal() override;

	static AudioMaster2Internal *open(Audio::Mixer *mixer);
	static void close();

	bool init();
	bool loadRessourceFile(Common::SeekableReadStream *data);

	bool startSound(const Common::String &name);
	bool stopSound(const Common::String &name);

	void flushResource(const Common::String &name);
	void flushAllResources();

	void fadeOut(int delay);
	bool isFading();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	void interrupt() override;

	void resetCounter();
	int getPlayDuration();

	void sync(SoundResource *res);
	void stopChannels();

private:
	void updateDevice();

	AudioMaster2IOManager::IOUnit *_channels[4];
	AudioMaster2IOManager *_io;
	AudioMaster2ResourceManager *_res;
	Audio::Mixer *_mixer;

	uint32 _durationCounter;
	uint8 _fadeOutSteps;

	static AudioMaster2Internal *_refInstance;
	static int _refCount;

	Audio::SoundHandle _soundHandle;
	bool _ready;
};

class SoundResource {
protected:
	SoundResource(AudioMaster2ResourceManager *res, int type) : _res(res), _type(type), _playing(false), _next(0), _flags(0), _masterVolume(64), _refCnt(1) {}
	virtual ~SoundResource() {}
public:
	void loadName(Common::ReadStream *stream, uint32 size);

	void open();
	void close();

	virtual void prepare() {}
	virtual void setSync(uint32 sync) {}

	const Common::String &getName() const;
	uint8 getType() const;
	bool getPlayStatus() const;
	void setPlayStatus(bool playing);
	void setMasterVolume(int volume);

	virtual void interrupt(AudioMaster2IOManager *io);
	virtual void setupMusicNote(AudioMaster2IOManager::IOUnit *unit, uint8 note, uint16 volume) {}
	virtual void setupSoundEffect(AudioMaster2IOManager::IOUnit *unit, uint32 sync, uint32 tempo) {}

	enum Mode {
		kIdle = 0,
		kRestart = 1
	};

	SoundResource *_next;

protected:
	Common::String _name;
	const uint8 _type;
	uint8 _flags;
	uint16 _masterVolume;
	AudioMaster2ResourceManager *_res;

private:
	virtual void release() = 0;
	virtual void setupEnvelopes(AudioMaster2IOManager::IOUnit *unit) {}

	int _refCnt;
	bool _playing;
};

class SoundResource8SVX : public SoundResource {
public:
	SoundResource8SVX(AudioMaster2ResourceManager *res);
private:
	~SoundResource8SVX() override;
public:
	void loadHeader(Common::ReadStream *stream, uint32 size);
	void loadData(Common::ReadStream *stream, uint32 size);

	void setupMusicNote(AudioMaster2IOManager::IOUnit *unit, uint8 note, uint16 volume) override;
	void setupSoundEffect(AudioMaster2IOManager::IOUnit *unit, uint32 sync, uint32 tempo) override;

private:
	void release() override;

	void setupEnvelopes(AudioMaster2IOManager::IOUnit *unit) override;

	uint32 _numSamplesOnce;
	uint32 _numSamplesRepeat;
	uint32 _numSamplesPerCycle;
	uint16 _rate;
	uint8 _numBlocks;
	uint8 _format;
	uint32 _trackVolume;

	const int8 *_data;
	uint32 _dataSize;

	static const uint32 _periods[128];
};

class SoundResourceINST : public SoundResource {
public:
	SoundResourceINST(AudioMaster2ResourceManager *res) : SoundResource(res, 2), _samplesResource(0), _transpose(0), _levelAdjust(0) {}
private:
	~SoundResourceINST() override;
public:
	void loadPitchData(Common::ReadStream *stream, uint32 size);
	void loadSamples(Common::ReadStream *stream, uint32 size);
	void loadVolumeData(Common::ReadStream *stream, uint32 size);

	void setupMusicNote(AudioMaster2IOManager::IOUnit *unit, uint8 note, uint16 volume) override;
	void setupSoundEffect(AudioMaster2IOManager::IOUnit *unit, uint32 sync, uint32 rate) override;

	struct EnvelopeData {
		EnvelopeData(const uint8 *data, uint32 size) : volume(0x40), _data(data), _dataSize(size) {}
		~EnvelopeData() { delete[] _data; }
		uint8 volume;
		const uint8 *_data;
		uint32 _dataSize;
	};

private:
	void release() override;

	void setupEnvelopes(AudioMaster2IOManager::IOUnit *unit) override;

	EnvelopeData *_transpose;
	EnvelopeData *_levelAdjust;
	SoundResource *_samplesResource;
};

class SoundResourceSMUS : public SoundResource {
public:
	SoundResourceSMUS(AudioMaster2ResourceManager *res) : SoundResource(res, 1), _tempo(0), _songVolume(0), _playFlags(0) {}
private:
	~SoundResourceSMUS() override;
public:
	void loadHeader(Common::ReadStream *stream, uint32 size);
	void loadInstrument(Common::ReadStream *stream, uint32 size);
	void loadTrack(Common::ReadStream *stream, uint32 size);

	void prepare() override;
	uint16 getTempo() const;
	void setSync(uint32 sync) override;

	void interrupt(AudioMaster2IOManager *io) override;

private:
	void release() override;

	struct Track {
		Track() : _dataStart(0), _dataEnd(0), _dataCur(0), _instrument(0), _volume(0), _sync(0) {}
		~Track() {
			if (_instrument)
				_instrument->close();
			delete[] _dataStart;
		}

		void setInstrument(SoundResource *instr) {
			if (_instrument)
				_instrument->close();
			_instrument = instr;
			_instrument->open();
		}

		uint32 _sync;
		SoundResource *_instrument;
		uint8 _volume;
		const uint8 *_dataStart;
		const uint8 *_dataEnd;
		const uint8 *_dataCur;
	};

	bool parse(AudioMaster2IOManager *io, Track *track);

	uint16 _tempo;
	uint8 _songVolume;

	static const uint16 _durationTable[64];

	Common::Array<Track*> _tracks;
	Common::Array<SoundResource*> _instruments;
	uint16 _playFlags;
};

class AudioMaster2ResourceManager {
public:
	AudioMaster2ResourceManager(AudioMaster2Internal *driver, Common::Mutex &mutex);
	~AudioMaster2ResourceManager();

	void loadResourceFile(Common::SeekableReadStream *data);

	void initResource(SoundResource *resource);
	void deinitResource(SoundResource *resource);
	void releaseResource(const Common::String &resName);

	void stopChain();
	void flush();

	SoundResource *getResource(const Common::String &resName, SoundResource::Mode mode);

	void setMasterVolume(int type, int volume);

	void interrupt(AudioMaster2IOManager *io);

private:
	SoundResource *retrieveFromChain(const Common::String &resName);
	void linkToChain(SoundResource *resource, SoundResource::Mode mode);

	SoundResource *_chainPlaying;
	SoundResource *_chainStorage;

	uint16 _masterVolume[3];

	AudioMaster2Internal *_driver;
	Common::Mutex &_mutex;
};

class AudioMaster2IFFLoader : public Common::IFFParser {
public:
	AudioMaster2IFFLoader(Common::SeekableReadStream *stream, AudioMaster2ResourceManager *res) : Common::IFFParser(stream), _res(res), _curSong(0), _curSfx(0), _curIns(0) {}
	virtual ~AudioMaster2IFFLoader();

	bool loadChunk(Common::IFFChunk &chunk);

private:
	void initResource();

	// define only additional chunk types which aren't already defined in common/iff_container.h
	enum ChunkTypes {
		ID_INST = MKTAG('I', 'N', 'S', 'T'),
		ID_PTCH = MKTAG('P', 'T', 'C', 'H'),
		ID_SAMP = MKTAG('S', 'A', 'M', 'P'),
		ID_VLUM = MKTAG('V', 'L', 'U', 'M'),
		ID_SMUS = MKTAG('S', 'M', 'U', 'S'),
		ID_TRAK = MKTAG('T', 'R', 'A', 'K'),
		ID_SHDR = MKTAG('S', 'H', 'D', 'R'),
		ID_INS1 = MKTAG('I', 'N', 'S', '1')
	};

	SoundResourceINST *_curIns;
	SoundResourceSMUS *_curSong;
	SoundResource8SVX *_curSfx;

	AudioMaster2ResourceManager *_res;
};

AudioMaster2IOManager::AudioMaster2IOManager() : _sync(1), _tempo(1), _ioChain(0) {
	for (int i = 0; i < 8; ++i)
		_units[i] = new IOUnit();
}

AudioMaster2IOManager::~AudioMaster2IOManager() {
	for (int i = 0; i < 8; ++i)
		delete _units[i];
}

void AudioMaster2IOManager::clearChain() {
	_ioChain = 0;
}

void AudioMaster2IOManager::deployChannels(IOUnit **dest) {
	IOUnit *cur = _ioChain;
	IOUnit *prev = 0;
	_ioChain = 0;

	for (; cur; cur = cur->_next) {
		if (!(cur->_flags & 1)) {
			cur->_flags &= ~2;

			if (prev)
				prev->_next = cur->_next;
			else
				_ioChain = cur->_next;

			continue;
		}

		IOUnit *unit = 0;
		uint32 lowest = 0xFFFFFFFF;
		uint8 chan = 3;
		bool foundFree = false;

		for (uint8 i = 3; i < 4; --i) {
			unit = dest[i];
			if (!unit) {
				chan = i;
				foundFree = true;
				break;
			}

			if (unit->_endTick < lowest) {
				lowest = unit->_endTick;
				chan = i;
			}
		}

		if (!foundFree) {
			dest[chan]->_flags &= ~2;
			// The original driver sends ADCMD_FINISH here, but we don't need to do anything,
			// since this driver runs on the same thread as the emulated hardware.
		}

		dest[chan] = cur;
		prev = cur;
	}
}

AudioMaster2IOManager::IOUnit *AudioMaster2IOManager::requestFreeUnit() {
	for (int i = 0; i < 8; ++i) {
		if (_units[i]->_flags & 2)
			continue;
		_units[i]->_flags = 7;
		_units[i]->_next = _ioChain;
		_ioChain = _units[i];
		return _units[i];
	}

	return 0;
}

void AudioMaster2IOManager::fadeOut() {
	for (int i = 0; i < 8; ++i) {
		if (_units[i]->_flags & 2)
			_units[i]->_fadeOutState = 0;
	}
}

bool AudioMaster2IOManager::isFading() {
	for (int i = 0; i < 8; ++i) {
		if (_units[i]->_flags & 2) {
			if (_units[i]->_fadeOutState > -1)
				return true;
		} else {
			_units[i]->_fadeOutState = -1;
		}
	}
	return false;
}

void SoundResource::loadName(Common::ReadStream *stream, uint32 size) {
	char *data = new char[size + 1];

	stream->read(data, size);
	data[size] = '\0';
	_name = data;

	delete[] data;
}

void SoundResource::open() {
	_refCnt++;
	debugC(8, kDebugLevelSound, "SoundResource::open(): '%s', type '%s', new refCount: '%d'", _name.c_str(), (_type == 1) ? "SMUS" : (_type == 2 ? "INST" : "8SVX"), _refCnt);
}

void SoundResource::close() {
	_refCnt--;
	debugC(8, kDebugLevelSound, "SoundResource::close(): '%s', type '%s', new refCount: '%d' %s", _name.c_str(), (_type == 1) ? "SMUS" : (_type == 2 ? "INST" : "8SVX"), _refCnt, _refCnt <= 0 ? "--> RELEASED" : "");
	if (_refCnt == 0) {
		_res->deinitResource(this);
		release();
	}
}

const Common::String &SoundResource::getName() const {
	return _name;
}

uint8 SoundResource::getType() const {
	return _type;
}

bool SoundResource::getPlayStatus() const {
	return _playing;
}

void SoundResource::setPlayStatus(bool playing) {
	_playing = playing;
}

void SoundResource::setMasterVolume(int volume) {
	_masterVolume = volume >> 2;
}

void SoundResource::interrupt(AudioMaster2IOManager *io) {
	setPlayStatus(false);
	AudioMaster2IOManager::IOUnit *unit = io->requestFreeUnit();
	setupSoundEffect(unit, io->_sync, io->_tempo);
}

SoundResource8SVX::SoundResource8SVX(AudioMaster2ResourceManager *res) : SoundResource(res, 4),
	_numSamplesOnce(0), _numSamplesRepeat(0), _numSamplesPerCycle(0), _trackVolume(0), _dataSize(0), _rate(0), _numBlocks(0), _format(0), _data(0) {
}

SoundResource8SVX::~SoundResource8SVX() {
	delete[] _data;
}

void SoundResource8SVX::loadHeader(Common::ReadStream *stream, uint32 size) {
	if (size < 20)
		error("SoundResource8SVX:loadHeader(): Invalid data chunk size");

	_numSamplesOnce = stream->readUint32BE();
	_numSamplesRepeat = stream->readUint32BE();
	_numSamplesPerCycle = stream->readUint32BE();
	_rate = stream->readUint16BE();
	_numBlocks = stream->readByte();
	_format = stream->readByte();

	if (_format)
		error("SoundResource8SVX:loadHeader(): Unsupported data format");

	_trackVolume = stream->readUint32BE();
}

void SoundResource8SVX::loadData(Common::ReadStream *stream, uint32 size) {
	delete[] _data;
	_dataSize = size;
	int8 *data = new int8[size];
	stream->read(data, size);
	_data = data;
}

void SoundResource8SVX::setupMusicNote(AudioMaster2IOManager::IOUnit *unit, uint8 note, uint16 volume) {
	uint32 no = _numSamplesOnce;
	uint32 nr = _numSamplesRepeat;
	// The original code uses 3579546 here. But since the Paula code uses kPalPaulaClock I do the same.
	uint32 rt = Audio::Paula::kPalPaulaClock;
	uint32 offs = 0;

	if (_numSamplesRepeat && _numSamplesPerCycle) {
		uint32 octave = _numBlocks;
		rt = _periods[note] << 13;

		for (rt /= _numSamplesPerCycle; rt >= 0x4000000 && octave > 1; octave--) {
			offs += (no + nr);
			no <<= 1;
			nr <<= 1;
			rt >>= 1;
			rt /= _numSamplesPerCycle;
		}

		for (; octave > 1 && rt >= 0x46000; octave--) {
			offs += (no + nr);
			no <<= 1;
			nr <<= 1;
			rt >>= 1;
		}
		rt >>= 13;

	} else if (_rate) {
		rt /= _rate;
	}

	unit->_sampleData = _data + offs;
	unit->_sampleDataRepeat = nr ? unit->_sampleData + no : 0;
	unit->_lenOnce = no;
	unit->_lenRepeat = nr;
	unit->_rate = unit->_period = rt;
	unit->_volumeSetting = unit->_outputVolume = volume;
	setupEnvelopes(unit);
}

void SoundResource8SVX::setupSoundEffect(AudioMaster2IOManager::IOUnit *unit, uint32 sync, uint32 tempo) {
	if (!unit)
		return;

	unit->_startTick = sync;
	// The original code uses 3579546 here. But since the Paula code uses kPalPaulaClock I do the same.
	unit->_rate = unit->_period = Audio::Paula::kPalPaulaClock / (_rate ? _rate : 2000);

	uint32 no = _numSamplesOnce;
	uint32 nr = _numSamplesRepeat;
	uint32 octave = _numBlocks;
	uint32 offs = 0;

	for (; octave > 1; octave--) {
		offs += (no + nr);
		no <<= 1;
		nr <<= 1;
	}

	unit->_sampleData = _data + offs;
	unit->_sampleDataRepeat = nr ? unit->_sampleData + no : 0;
	unit->_lenOnce = no;
	unit->_lenRepeat = nr;

	unit->_endTick = _numSamplesRepeat ? 0xFFFFFFFF : (tempo * _numSamplesOnce * 60) / _rate + sync;
	unit->_volumeSetting = unit->_outputVolume = ((_trackVolume >= 0xFFFF ? _trackVolume >> 2 : 0x4000) * _masterVolume) / 64;
	setupEnvelopes(unit);
}

void SoundResource8SVX::release() {
	delete this;
}

void SoundResource8SVX::setupEnvelopes(AudioMaster2IOManager::IOUnit *unit) {
	assert(unit);
	unit->_transposeData = 0;
	unit->_levelAdjustData = 0;
}

const uint32 SoundResource8SVX::_periods[128] = {
	0x0006ae3f, 0x00064e42, 0x0005f3a8, 0x00059e23, 0x00054d6c, 0x0005013c, 0x0004b953, 0x00047573,
	0x00043563, 0x0003f8eb, 0x0003bfd7, 0x000389f8, 0x0003571f, 0x00032721, 0x0002f9d4, 0x0002cf11,
	0x0002a6b6, 0x0002809e, 0x00025ca9, 0x00023ab9, 0x00021ab1, 0x0001fc75, 0x0001dfeb, 0x0001c4fc,
	0x0001ab8f, 0x00019390, 0x00017cea, 0x00016788, 0x0001535b, 0x0001404f, 0x00012e54, 0x00011d5c,
	0x00010d58, 0x0000fe3a, 0x0000eff5, 0x0000e27e, 0x0000d5c7, 0x0000c9c8, 0x0000be75, 0x0000b3c4,
	0x0000a9ad, 0x0000a027, 0x0000972a, 0x00008eae, 0x000086ac, 0x00007f1d, 0x000077fa, 0x0000713f,
	0x00006ae3, 0x000064e4, 0x00005f3a, 0x000059e2, 0x000054d6, 0x00005013, 0x00004b95, 0x00004757,
	0x00004356, 0x00003f8e, 0x00003bfd, 0x0000389f, 0x00003571, 0x00003272, 0x00002f9d, 0x00002cf1,
	0x00002a6b, 0x00002809, 0x000025ca, 0x000023ab, 0x000021ab, 0x00001fc7, 0x00001dfe, 0x00001c4f,
	0x00001ab8, 0x00001939, 0x000017ce, 0x00001678, 0x00001535, 0x00001404, 0x000012e5, 0x000011d5,
	0x000010d5, 0x00000fe3, 0x00000eff, 0x00000e27, 0x00000d5c, 0x00000c9c, 0x00000be7, 0x00000b3c,
	0x00000a9a, 0x00000a02, 0x00000972, 0x000008ea, 0x0000086a, 0x000007f1, 0x0000077f, 0x00000713,
	0x000006ae, 0x0000064e, 0x000005f3, 0x0000059e, 0x0000054d, 0x00000501, 0x000004b9, 0x00000475,
	0x00000435, 0x000003f8, 0x000003bf, 0x00000389, 0x00000357, 0x00000327, 0x000002f9, 0x000002cf,
	0x000002a6, 0x00000280, 0x0000025c, 0x0000023a, 0x0000021a, 0x000001fc, 0x000001df, 0x000001c4,
	0x000001ab, 0x00000193, 0x0000017c, 0x00000167, 0x00000153, 0x00000140, 0x0000012e, 0x0000011d
};

SoundResourceINST::~SoundResourceINST() {
	if (_samplesResource)
		_samplesResource->close();

	delete _transpose;
	delete _levelAdjust;
}

void SoundResourceINST::loadPitchData(Common::ReadStream *stream, uint32 size) {
	delete _transpose;
	uint8 *data = new uint8[size];
	stream->read(data, size);
	_transpose = new EnvelopeData(data, size);
}

void SoundResourceINST::loadSamples(Common::ReadStream *stream, uint32 size) {
	char *data = new char[size + 1];
	stream->read(data, size);
	data[size] = '\0';

	if (_samplesResource)
		_samplesResource->close();

	SoundResource *instr = _res->getResource(data, SoundResource::kIdle);
	if (instr) {
		int type = instr->getType();
		if (type == 1)
			error("SoundResourceINST::loadInstrument(): Unexpected resource type");
		instr->open();
		_samplesResource = instr;
	} else {
		// This will come up quite often in EOB II. But never with instruments that are actually used. No need to bother the user with a warning here.
		debugC(9, kDebugLevelSound, "SoundResourceINST::loadInstrument(): Samples resource '%s' not found for '%s'.", data, _name.c_str());
		_samplesResource = 0;
	}

	delete[] data;
}

void SoundResourceINST::loadVolumeData(Common::ReadStream *stream, uint32 size) {
	delete _levelAdjust;
	uint8 *data = new uint8[size];
	stream->read(data, size);
	_levelAdjust = new EnvelopeData(data, size);
}

void SoundResourceINST::setupMusicNote(AudioMaster2IOManager::IOUnit *unit, uint8 note, uint16 volume) {
	assert(unit);
	_samplesResource->setupMusicNote(unit, note, volume);
	setupEnvelopes(unit);
}

void SoundResourceINST::release() {
	delete this;
}

void SoundResourceINST::setupEnvelopes(AudioMaster2IOManager::IOUnit *unit) {
	assert(unit);
	if (_transpose) {
		unit->_transposeData = _transpose->_data;
		unit->_transposeDuration = 0;
		unit->_transposePara = 0;
	} else {
		unit->_transposeData = 0;
	}

	if (_levelAdjust) {
		unit->_levelAdjustData = _levelAdjust->_data;
		unit->_levelAdjustDuration = 0;
		unit->_levelAdjustPara = 0;
	} else {
		unit->_levelAdjustData = 0;
	}
}

void SoundResourceINST::setupSoundEffect(AudioMaster2IOManager::IOUnit *unit, uint32 sync, uint32 rate) {
	if (!unit)
		return;

	if (_samplesResource)
		_samplesResource->setupSoundEffect(unit, sync, rate);

	setupEnvelopes(unit);
}

SoundResourceSMUS::~SoundResourceSMUS() {
	for (Common::Array<Track*>::iterator i = _tracks.begin(); i != _tracks.end(); ++i)
		delete *i;
	for (Common::Array<SoundResource*>::iterator i = _instruments.begin(); i != _instruments.end(); ++i)
		(*i)->close();
}

void SoundResourceSMUS::loadHeader(Common::ReadStream *stream, uint32 size) {
	if (size < 3)
		error("SoundResourceSMUS:loadHeader(): Invalid data chunk size");

	_tempo = stream->readUint16BE() / 68;
	_songVolume = stream->readByte();
}

void SoundResourceSMUS::loadInstrument(Common::ReadStream *stream, uint32 size) {
	stream->readUint32BE();
	size -= 4;

	char *data = new char[size + 1];
	stream->read(data, size);
	data[size] = '\0';

	SoundResource *instr = _res->getResource(data, SoundResource::kIdle);
	if (instr) {
		int type = instr->getType();
		if (type == 1)
			error("SoundResourceSMUS::loadInstrument(): Unexpected resource type");
		instr->open();
		_instruments.push_back(instr);
	} else {
		warning("SoundResourceSMUS::loadInstrument(): Samples resource '%s' not found for '%s'.", data, _name.c_str());
	}

	delete[] data;
}

void SoundResourceSMUS::loadTrack(Common::ReadStream *stream, uint32 size) {
	Track *track = new Track();
	uint8 *data = new uint8[size];
	stream->read(data, size);

	track->_dataStart = data;
	track->_dataEnd = data + size;
	track->_volume = 128;

	_tracks.push_back(track);
}

void SoundResourceSMUS::prepare() {
	_playFlags = 0;
	for (Common::Array<Track*>::iterator trk = _tracks.begin(); trk != _tracks.end(); ++trk) {
		(*trk)->_dataCur = (*trk)->_dataStart;
		(*trk)->setInstrument(*_instruments.begin());

		if (!(*trk)->_instrument)
			error("SoundResourceSMUS::prepare():: Unable to assign default instrument to track (resource files loaded in the wrong order?)");

		_playFlags = (_playFlags << 1) | 1;
	}
}

uint16 SoundResourceSMUS::getTempo() const {
	return _tempo;
}

void SoundResourceSMUS::setSync(uint32 sync) {
	for (Common::Array<Track*>::iterator i = _tracks.begin(); i != _tracks.end(); ++i)
		(*i)->_sync = sync;
}

void SoundResourceSMUS::interrupt(AudioMaster2IOManager *io) {
	for (uint32 i = 0; i < _tracks.size(); ++i) {
		if (!parse(io, _tracks[i]))
			_playFlags &= ~(1 << i);
	}

	if (!_playFlags)
		setPlayStatus(false);
}

void SoundResourceSMUS::release() {
	delete this;
}

bool SoundResourceSMUS::parse(AudioMaster2IOManager *io, Track *track) {
	uint32 duration = 0;

	while (track->_sync <= io->_sync) {
		if (track->_dataCur >= track->_dataEnd)
			return false;

		uint8 cmd = *track->_dataCur++;
		uint8 para = *track->_dataCur++;

		if (cmd <= 0x80) {
			if (para & 0x80)
				continue;

			duration += _durationTable[para & 0x3f];
			if (para & 0x40)
				continue;

			if (cmd < 0x80) {
				AudioMaster2IOManager::IOUnit *unit = io->requestFreeUnit();
				if (unit) {
					unit->_startTick = track->_sync;
					unit->_endTick = unit->_startTick + duration;
					track->_instrument->setupMusicNote(unit, cmd, _masterVolume * track->_volume);
				}
			}

			track->_sync += duration;
			duration = 0;

		} else {
			switch (cmd) {
			case 0x81:
				assert(para < _instruments.size());
				track->setInstrument(_instruments[para]);
				break;

			case 0x84:
				track->_volume = para;
				break;

			case 0x88:
				//not implemented
				break;

			case 0xFF:
				return false;

			default:
				break;
			}
		}
	}

	return true;
}

const uint16 SoundResourceSMUS::_durationTable[64] = {
	0x6900, 0x3480, 0x1a40, 0x0d20, 0x0690, 0x0348, 0x01a4, 0x00d2,
	0x9d80, 0x4ec0, 0x2760, 0x13b0, 0x09d8, 0x04ec, 0x0276, 0x013b,
	0x4600, 0x2300, 0x1180, 0x08c0, 0x0460, 0x0230, 0x0118, 0x008c,
	0x6900, 0x3480, 0x1a40, 0x0d20, 0x0690, 0x0348, 0x01a4, 0x00d2,
	0x5400, 0x2a00, 0x1500, 0x0a80, 0x0540, 0x02a0, 0x0150, 0x00a8,
	0x7e00, 0x3f00, 0x1f80, 0x0fc0, 0x07e0, 0x03f0, 0x01f8, 0x00fc,
	0x5a00, 0x2d00, 0x1680, 0x0b40, 0x05a0, 0x02d0, 0x0168, 0x00b4,
	0x8700, 0x4380, 0x21c0, 0x10e0, 0x0870, 0x0438, 0x021c, 0x010e
};

AudioMaster2ResourceManager::AudioMaster2ResourceManager(AudioMaster2Internal *driver, Common::Mutex &mutex) : _driver(driver), _mutex(mutex), _chainPlaying(0), _chainStorage(0) {
	memset(_masterVolume, 0, sizeof(_masterVolume));
}

AudioMaster2ResourceManager::~AudioMaster2ResourceManager() {
	flush();
}

void AudioMaster2ResourceManager::loadResourceFile(Common::SeekableReadStream *data) {
	do {
		AudioMaster2IFFLoader loader(data, this);
		Common::Functor1Mem<Common::IFFChunk&, bool, AudioMaster2IFFLoader> cb(&loader, &AudioMaster2IFFLoader::loadChunk);
		loader.parse(cb);
	} while (data->pos() + 8 < data->size());

	for (int i = 0; i < 3; ++i)
		setMasterVolume(1 << i, _masterVolume[i]);
}

void AudioMaster2ResourceManager::initResource(SoundResource *resource) {
	if (!resource)
		return;

	Common::StackLock lock(_mutex);

	SoundResource *res = retrieveFromChain(resource->getName());
	// The driver does not replace resources with the same name, but disposes the new resource instead.
	// So these names seem to be considered "globally unique".
	if (res)
		resource->close();
	else
		res = resource;

	linkToChain(res, SoundResource::kIdle);
}

void AudioMaster2ResourceManager::deinitResource(SoundResource *resource) {
	Common::StackLock lock(_mutex);

	SoundResource *prev = 0;
	for (SoundResource *cur = _chainPlaying; cur; cur = cur->_next) {
		if (cur == resource) {
			if (prev)
				prev->_next = cur->_next;
			else
				_chainPlaying = cur->_next;
			cur->_next = 0;
			return;
		}
		prev = cur;
	}

	prev = 0;
	for (SoundResource *cur = _chainStorage; cur; cur = cur->_next) {
		if (cur == resource) {
			if (prev)
				prev->_next = cur->_next;
			else
				_chainStorage = cur->_next;
			cur->_next = 0;
			return;
		}
		prev = cur;
	}
}

void AudioMaster2ResourceManager::releaseResource(const Common::String &resName) {
	stopChain();

	SoundResource *res = retrieveFromChain(resName);
	if (!res)
		return;

	res->setPlayStatus(false);
	res->close();
}

void AudioMaster2ResourceManager::stopChain() {
	Common::StackLock lock(_mutex);

	SoundResource *cur = _chainPlaying;
	while (cur) {
		cur->setPlayStatus(false);
		cur = cur->_next;
	}

	_driver->stopChannels();
}

void AudioMaster2ResourceManager::flush() {
	Common::StackLock lock(_mutex);

	stopChain();

	while (_chainPlaying) {
		SoundResource *res = _chainPlaying;
		deinitResource(res);
		res->close();
	}

	while (_chainStorage) {
		SoundResource *res = _chainStorage;
		deinitResource(res);
		res->close();
	}
}

SoundResource *AudioMaster2ResourceManager::getResource(const Common::String &resName, SoundResource::Mode mode) {
	if (resName.empty())
		return 0;

	SoundResource *res = retrieveFromChain(resName);
	if (!res)
		return 0;

	if (mode == SoundResource::kIdle)
		res->setPlayStatus(false);
	else if (res->getType() == 1)
		_driver->resetCounter();

	linkToChain(res, mode);

	return res;
}

void AudioMaster2ResourceManager::setMasterVolume(int type, int volume) {
	assert(type == 1 || type == 2 || type == 4);
	Common::StackLock lock(_mutex);

	_masterVolume[type >> 1] = volume & 0xFFFF;

	for (SoundResource *res = _chainPlaying; res; res = res->_next) {
		if (res->getType() == type)
			res->setMasterVolume(volume);
	}

	for (SoundResource *res = _chainStorage; res; res = res->_next) {
		if (res->getType() == type)
			res->setMasterVolume(volume);
	}
}

void AudioMaster2ResourceManager::interrupt(AudioMaster2IOManager *io) {
	SoundResource *cur = _chainPlaying;
	SoundResource *prev = 0;

	while (cur) {
		cur->interrupt(io);

		if (cur->getPlayStatus()) {
			prev = cur;
			cur = cur->_next;
		} else if (prev) {
			prev->_next = cur->_next;
			cur->_next = _chainStorage;
			_chainStorage = cur;
			cur = prev->_next;
		} else {
			_chainPlaying = cur->_next;
			cur->_next = _chainStorage;
			_chainStorage = cur;
			cur = _chainPlaying;
		}
	}
}

SoundResource *AudioMaster2ResourceManager::retrieveFromChain(const Common::String &resName) {
	if (resName.empty())
		return 0;

	// The requesting (SMUS or INST) resources use shorter (cut off) versions of the actual name strings stored in the requested
	// (INST or 8SVX) resources. E. g. the EOB intro song will request (among other things) a 'flute' and a 'vibe', although
	// the actual resource names are 'flute3c' and 'vibe3c'. I would have liked to have all these strings hashed into uint32 IDs,
	// but this "feature" spoils that idea.

	const char *srchStr = resName.c_str();
	uint32 srchDepth = strlen(srchStr);

	Common::StackLock lock(_mutex);

	SoundResource *cur = _chainPlaying;
	SoundResource *prev = 0;

	while (cur) {
		if (!scumm_strnicmp(cur->getName().c_str(), srchStr, srchDepth)) {
			if (prev)
				prev->_next = cur->_next;
			else
				_chainPlaying = cur->_next;
			cur->_next = 0;
			return cur;
		}
		prev = cur;
		cur = cur->_next;
	}

	cur = _chainStorage;
	prev = 0;

	while (cur) {
		if (!scumm_strnicmp(cur->getName().c_str(), srchStr, srchDepth)) {
			if (prev)
				prev->_next = cur->_next;
			else
				_chainStorage = cur->_next;
			cur->_next = 0;
			return cur;
		}
		prev = cur;
		cur = cur->_next;
	}

	return 0;
}


void AudioMaster2ResourceManager::linkToChain(SoundResource *resource, SoundResource::Mode mode) {
	Common::StackLock lock(_mutex);

	if (resource->getType() == 1) {
		stopChain();
		resource->prepare();
	}

	if (mode == SoundResource::kRestart) {
		resource->setPlayStatus(true);
		resource->_next = _chainPlaying;
		_chainPlaying = resource;

		if (resource->getType() == 1)
			_driver->sync(resource);

	} else {
		resource->_next = _chainStorage;
		_chainStorage = resource;
	}
}

AudioMaster2IFFLoader::~AudioMaster2IFFLoader() {
	initResource();
}

bool AudioMaster2IFFLoader::loadChunk(Common::IFFChunk &chunk) {
	if (_formType == ID_INST) {
		if (_curIns == 0)
			_curIns = new SoundResourceINST(_res);

		switch (chunk._type) {
		case ID_NAME:
			_curIns->loadName(chunk._stream, chunk._size);
			break;
		case ID_SAMP:
			_curIns->loadSamples(chunk._stream, chunk._size);
			break;
		case ID_VLUM:
			_curIns->loadVolumeData(chunk._stream, chunk._size);
			break;
		case ID_PTCH:
			_curIns->loadPitchData(chunk._stream, chunk._size);
			break;
		default:
			break;
		}

	} else if (_formType == ID_SMUS) {
		if (_curSong == 0)
			_curSong = new SoundResourceSMUS(_res);

		switch (chunk._type) {
		case ID_SHDR:
			_curSong->loadHeader(chunk._stream, chunk._size);
			break;
		case ID_NAME:
			_curSong->loadName(chunk._stream, chunk._size);
			break;
		case ID_TRAK:
			_curSong->loadTrack(chunk._stream, chunk._size);
			break;
		case ID_INS1:
			_curSong->loadInstrument(chunk._stream, chunk._size);
			break;
		default:
			break;
		}

	} else if (_formType == ID_8SVX) {
		if (_curSfx == 0)
			_curSfx = new SoundResource8SVX(_res);

		switch (chunk._type) {
		case ID_VHDR:
			_curSfx->loadHeader(chunk._stream, chunk._size);
			break;
		case ID_NAME:
			_curSfx->loadName(chunk._stream, chunk._size);
			break;
		case ID_BODY:
			_curSfx->loadData(chunk._stream, chunk._size);
			break;
		case ID_ANNO:
			break;
		default:
			break;
		}
	}

	return false;
}

void AudioMaster2IFFLoader::initResource() {
	if (_curIns) {
		_res->initResource(_curIns);
		_curIns = 0;
	} else if (_curSong) {
		_res->initResource(_curSong);
		_curSong = 0;
	} else if (_curSfx) {
		_res->initResource(_curSfx);
		_curSfx = 0;
	}
}

AudioMaster2Internal *AudioMaster2Internal::_refInstance = 0;
int AudioMaster2Internal::_refCount = 0;

AudioMaster2Internal::AudioMaster2Internal(Audio::Mixer *mixer) : Paula(true, mixer->getOutputRate(), mixer->getOutputRate() / 50), _mixer(mixer), _res(0), _io(0), _fadeOutSteps(0), _durationCounter(0), _ready(false) {
	_channels[0] = _channels[1] = _channels[2] = _channels[3] = 0;
	setAudioFilter(true);
}

AudioMaster2Internal::~AudioMaster2Internal() {
	stopPaula();
	_mixer->stopHandle(_soundHandle);

	Common::StackLock lock(_mutex);

	delete _res;
	delete _io;
}

AudioMaster2Internal *AudioMaster2Internal::open(Audio::Mixer *mixer) {
	_refCount++;

	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new AudioMaster2Internal(mixer);
	else if (_refCount < 2 || _refInstance == 0)
		error("AudioMaster2Internal::open(): Internal instance management failure");

	return _refInstance;
}

void AudioMaster2Internal::close() {
	if (!_refCount)
		return;

	_refCount--;

	if (!_refCount) {
		delete _refInstance;
		_refInstance = 0;
	}
}

bool AudioMaster2Internal::init() {
	if (_ready)
		return true;

	_io = new AudioMaster2IOManager();
	_res = new AudioMaster2ResourceManager(this, _mutex);

	startPaula();

	_mixer->playStream(Audio::Mixer::kPlainSoundType,
		&_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_ready = true;

	return true;
}

bool AudioMaster2Internal::loadRessourceFile(Common::SeekableReadStream *data) {
	if (!_ready || !data)
		return false;

	_res->loadResourceFile(data);

	return true;
}

bool AudioMaster2Internal::startSound(const Common::String &name) {
	return _ready ? (_res->getResource(name, SoundResource::kRestart) != 0) : false;
}

bool AudioMaster2Internal::stopSound(const Common::String &name) {
	return _ready ? (_res->getResource(name, SoundResource::kIdle) != 0) : false;
}

void AudioMaster2Internal::flushResource(const Common::String &name) {
	if (_ready)
		_res->releaseResource(name);
}

void AudioMaster2Internal::flushAllResources() {
	if (_ready)
		_res->flush();
}

void AudioMaster2Internal::fadeOut(int delay) {
	if (!_ready)
		return;

	_fadeOutSteps = delay >> 3;
	_io->fadeOut();
}

bool AudioMaster2Internal::isFading() {
	return _ready ? _io->isFading() : false;
}

void AudioMaster2Internal::setMusicVolume(int volume) {
	if (_ready)
		_res->setMasterVolume(1, volume);
}

void AudioMaster2Internal::setSoundEffectVolume(int volume) {
	if (_ready)
		_res->setMasterVolume(4, volume);
}

void AudioMaster2Internal::interrupt() {
	if (!_ready)
		return;

	_durationCounter++;
	_io->_sync += _io->_tempo;
	_res->interrupt(_io);
	_io->deployChannels(_channels);
	updateDevice();
}

void AudioMaster2Internal::resetCounter() {
	_durationCounter = 0;
}

int AudioMaster2Internal::getPlayDuration() {
	return _ready ? _durationCounter : 0;
}

void AudioMaster2Internal::sync(SoundResource *res) {
	if (!_ready || !res)
		return;

	Common::StackLock lock(_mutex);

	if (res->getType() != 1)
		return;

	SoundResourceSMUS *smus = static_cast<SoundResourceSMUS*>(res);
	_io->_tempo = smus->getTempo();
	smus->setSync(_io->_sync);
}

void AudioMaster2Internal::stopChannels() {
	if (!_ready)
		return;

	Common::StackLock lock(_mutex);

	for (uint8 i = 0; i < 4; ++i) {
		if (_channels[i]) {
			_channels[i]->_endTick = 0;
			disableChannel(i);
		}
	}

	_io->clearChain();
}

void AudioMaster2Internal::updateDevice() {
	for (uint8 i = 3; i < 4; --i) {
		AudioMaster2IOManager::IOUnit *unit = _channels[i];
		if (!unit)
			continue;

		if (_io->_sync > unit->_endTick) {
			_channels[i] = 0;
			unit->_flags &= ~2;
			disableChannel(i);
			continue;
		}

		bool next = false;

		if (unit->_transposeData) {
			unit->_period += unit->_transposePara;
			const uint8 *data = unit->_transposeData;

			if (unit->_transposeDuration-- <= 1) {
				for (bool loop = true; loop; ) {
					uint8 para = *data++;

					if (para == 0xFF) {
						para = *data++;

						if (para == 0) {
							unit->_flags &= ~2;
							disableChannel(i);
							loop = false;
							next = true;
							continue;

						} else if (para == 1) {
							unit->_transposeData = 0;
							loop = false;

						} else {
							unit->_period = READ_BE_UINT16(data);
							data += 2;
						}

					} else if (para == 0xFE) {
						para = *data++;
						data -= ((para + 1) << 1);

					} else {
						unit->_transposeDuration = para;
						unit->_transposePara = *data++;
						unit->_transposeData = data;
						loop = false;
					}
				}
			}
		}

		if (next)
			continue;

		next = false;

		if (unit->_levelAdjustData) {
			unit->_outputVolume += unit->_levelAdjustPara;
			const uint8 *data = unit->_levelAdjustData;

			if (unit->_levelAdjustDuration-- <= 1) {
				for (bool loop = true; loop; ) {
					uint8 para = *data++;
					if (para == 0xFF) {
						para = *data++;

						if (para == 0) {
							unit->_flags &= ~2;
							disableChannel(i);
							loop = false;
							next = true;
							continue;

						} else {
							unit->_levelAdjustData = 0;
							loop = false;
						}

					} else if (para == 0xFE) {
						para = *data++;
						data -= ((para + 1) << 1);

					} else {
						uint16 para2 = *data++;

						if (para2 & 0x80) {
							para2 = unit->_outputVolume + ((para2 - 0xC0) << 8);

						} else {
							para2 = (unit->_volumeSetting * para2) >> 6;
							if (para2 > 0x4000)
								para2 = 0x4000;
						}

						if (!para) {
							unit->_outputVolume = para2;
							continue;
						}

						unit->_levelAdjustDuration = para;

						if (para == 1) {
							unit->_outputVolume = para2;
							unit->_levelAdjustPara = 0;
						} else {
							int16 va = para2 - unit->_outputVolume;
							unit->_levelAdjustPara =  va / para;
						}

						unit->_levelAdjustData = data;
						loop = false;
					}
				}
			}
		}

		if (next)
			continue;

		if (unit->_flags & 4) {
			unit->_flags &= ~4;

			setChannelPeriod(i, unit->_period);
			setChannelVolume(i, unit->_outputVolume >> 8);

			if (unit->_lenOnce) {
				setChannelData(i, unit->_sampleData, unit->_sampleDataRepeat, unit->_lenOnce, unit->_lenRepeat);
			} else if (unit->_lenRepeat) {
				setChannelSampleStart(i, unit->_sampleDataRepeat);
				setChannelSampleLen(i, unit->_lenRepeat);
			}

		} else if (unit->_transposeData || unit->_levelAdjustData) {
			setChannelPeriod(i, unit->_period);
			setChannelVolume(i, unit->_outputVolume >> 8);
		}

		if (unit->_fadeOutState > -1) {
			setChannelVolume(i, ((unit->_outputVolume / _fadeOutSteps) * (_fadeOutSteps - unit->_fadeOutState)) >> 8);
			if (++unit->_fadeOutState > _fadeOutSteps)
				unit->_fadeOutState = -1;
		}
	}

	if (_fadeOutSteps && !_io->isFading()) {
		_fadeOutSteps = 0;
		_res->stopChain();
	}
}

AudioMaster2::AudioMaster2(Audio::Mixer *mixer) {
	_am2i = AudioMaster2Internal::open(mixer);
}

AudioMaster2::~AudioMaster2() {
	AudioMaster2Internal::close();
	_am2i = 0;
}

bool AudioMaster2::init() {
	return _am2i->init();
}

bool AudioMaster2::loadRessourceFile(Common::SeekableReadStream *data) {
	return _am2i->loadRessourceFile(data);
}

bool AudioMaster2::startSound(const Common::String &name) {
	return _am2i->startSound(name);
}

bool AudioMaster2::stopSound(const Common::String &name) {
	return _am2i->stopSound(name);
}

void AudioMaster2::flushResource(const Common::String &name) {
	_am2i->flushResource(name);
}

void AudioMaster2::flushAllResources() {
	_am2i->flushAllResources();
}

void AudioMaster2::fadeOut(int delay) {
	_am2i->fadeOut(delay);
}

bool AudioMaster2::isFading() {
	return _am2i->isFading();
}

int AudioMaster2::getPlayDuration() {
	return _am2i->getPlayDuration();
}

void AudioMaster2::setMusicVolume(int volume) {
	_am2i->setMusicVolume(volume);
}

void AudioMaster2::setSoundEffectVolume(int volume) {
	_am2i->setSoundEffectVolume(volume);
}

} // End of namespace Kyra

#endif
