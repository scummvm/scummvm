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


#include "common/macresman.h"
#include "common/punycode.h"
#include "scumm/detection.h"
#include "scumm/file.h"
#include "scumm/scumm.h"
#include "scumm/players/mac_sound_lowlevel.h"
#include "scumm/players/player_mac_new.h"
#include "scumm/imuse/drivers/macintosh.h"
#include "scumm/imuse/imuse.h"


namespace IMSMacintosh {
using namespace Scumm;

enum : byte {
	kRhythmPart = 9
};

struct ChanControlNode;
struct DeviceChannel {
	DeviceChannel(const uint32 *pitchtable) : pitchTable(pitchtable), frequency(0), phase(0), end(nullptr), pos(nullptr), smpBuffStart(nullptr),
		smpBuffEnd(nullptr), loopStart(nullptr), loopEnd(nullptr), pitch(0), mute(true), release(false), instr(nullptr), rhtm(false),
		prog(0), baseFreq(0), note(0), volumeL(0), volumeR(0), rate(0), totalLevelL(0), totalLevelR(0), node(nullptr), prev(nullptr), next(nullptr) {}
	~DeviceChannel() {}

	void recalcFrequency();

	uint32 frequency;
	uint32 phase;
	const byte *end;
	const byte *pos;
	const byte *smpBuffStart;
	const byte *smpBuffEnd;
	const byte *loopStart;
	const byte *loopEnd;
	uint16 pitch;
	byte volumeL;
	byte volumeR;
	byte totalLevelL;
	byte totalLevelR;
	byte baseFreq;
	uint32 rate;
	uint16 prog;
	byte note;
	bool rhtm;
	bool mute;
	bool release;
	const ChanControlNode *node;
	Common::SharedPtr<MacSndResource> instr;
	const uint32 *pitchTable;
	DeviceChannel *prev;
	DeviceChannel *next;
};

class IMSMacSoundSystem : public VblTaskClientDriver, public MacLowLevelPCMDriver::CallbackClient {
public:
	IMSMacSoundSystem(Audio::Mixer *mixer, byte version);
	virtual ~IMSMacSoundSystem();

	bool init(const char *const *instrFileNames, int numInstrFileNames, bool stereo, bool internal16Bit);
	void deinit();
	virtual bool start() = 0;
	void stop();

	virtual void setQuality(int qual) = 0;
	void noteOn(const ChanControlNode *node);
	void noteOff(const ChanControlNode *node);
	void voiceOff(const ChanControlNode *node);
	void setVolume(const ChanControlNode *node);
	void setPitchBend(const ChanControlNode *node);

	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc);

	void vblCallback() override;
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const override;
	const MacSoundDriver::Status &getDriverStatus(Audio::Mixer::SoundType type) const override;

	void dblBuffCallback(MacLowLevelPCMDriver::DoubleBuffer *dblBuffer) override;

	void setMasterVolume(Audio::Mixer::SoundType type, uint16 volume);

protected:
	bool setupResourceFork(Common::MacResManager &rm, const char *const *fileNames, int numFileNames);
	Common::SharedPtr<MacSndResource> getSndResource(uint16 id);
	void fillPitchTable();

	Common::Array<Common::SharedPtr<MacSndResource> > _smpResources;
	MacLowLevelPCMDriver::ChanHandle _musicChan;
	MacLowLevelPCMDriver::ChanHandle _sfxChan;
	MacLowLevelPCMDriver *_sdrv;
	bool _internal16Bit;
	bool _stereo;
	uint16 _feedBufferSize;
	uint16 _defaultInstrID;
	byte _quality;
	const uint16 _numChannels;
	Audio::Mixer *_mixer;
	DeviceChannel **_channels;

private:
	virtual bool loadInstruments(const char *const *fileNames, int numFileNames) = 0;
	virtual void setInstrument(DeviceChannel *chan) = 0;
	virtual void recalcFrequency(DeviceChannel *chan) = 0;
	virtual void recalcVolume(DeviceChannel *chan) = 0;
	virtual void noteOffIntern(DeviceChannel *chan) = 0;

	virtual DeviceChannel *allocateChannel(const ChanControlNode *node) = 0;

	uint32 *_pitchTable;
	byte *_ampTable;
	int16 *_mixTable;
	int16 *_mixBuffer16Bit;

	MacPlayerAudioStream *_macstr;
	Audio::SoundHandle _soundHandle;
	MacPlayerAudioStream::CallbackProc _vblTskProc;

	void *_timerParam;
	Common::TimerManager::TimerProc _timerProc;

	const byte _version;
};

class DJMSoundSystem final : public IMSMacSoundSystem {
public:
	DJMSoundSystem(Audio::Mixer *mixer);
	~DJMSoundSystem() override {}

	bool start() override;
	void setQuality(int qual) override;

private:
	bool loadInstruments(const char *const *fileNames, int numFileNames) override;
	void setInstrument(DeviceChannel *chan) override;
	void recalcFrequency(DeviceChannel *chan) override;
	void recalcVolume(DeviceChannel *chan) override;
	void noteOffIntern(DeviceChannel *chan) override;

	DeviceChannel *allocateChannel(const ChanControlNode *node) override;

	MacLowLevelPCMDriver::DBCallback _dbCbProc;
};

class NewMacSoundSystem final : public IMSMacSoundSystem {
public:
	NewMacSoundSystem(ScummEngine *vm, Audio::Mixer *mixer);
	~NewMacSoundSystem() override;

	bool start() override;
	void setQuality(int qual) override {}

private:
	bool loadInstruments(const char *const *fileNames, int numFileNames) override;
	Common::SharedPtr<MacSndResource> getNoteRangeSndResource(uint16 id, byte note);
	void setInstrument(DeviceChannel *chan) override;
	void recalcFrequency(DeviceChannel *chan) override;
	void recalcVolume(DeviceChannel *chan) override;
	void noteOffIntern(DeviceChannel *chan) override;

	DeviceChannel *allocateChannel(const ChanControlNode *node) override;

	MacLowLevelPCMDriver::DBCallback _dbCbProc;
	ScummFile *_fileMan;
	Common::Path &_container;
	Common::Path _dummy;

	struct Instrument {
		Instrument(uint16 resId) : id(resId), noteSmplsMapping(nullptr) {
			noteSmplsMapping = new byte[128]();
			memset(noteSmplsMapping, 0xFF, 128);
		}
		~Instrument() {
			delete[] noteSmplsMapping;
		}
		uint16 id;
		byte *noteSmplsMapping;
		Common::Array<Common::SharedPtr<MacSndResource> > sndRes;
	};
	Common::Array<Common::SharedPtr<Instrument> > _instruments;
};

class IMuseChannel_Macintosh final : public MidiChannel {
public:
	IMuseChannel_Macintosh(IMuseDriver_Macintosh *drv, int number);
	~IMuseChannel_Macintosh() override {}

	MidiDriver *device() override { return _drv; }
	byte getNumber() override {	return _number; }

	bool allocate();
	void release() override { _allocated = false; }

	void send(uint32 b) override { if (_drv) _drv->send((b & ~0x0F) | _number); }

	// Regular messages
	void noteOff(byte note) override;
	void noteOn(byte note, byte velocity) override;
	void controlChange(byte control, byte value) override;
	void programChange(byte program) override;
	void pitchBend(int16 bend) override;

	// Control Change and SCUMM specific functions
	void pitchBendFactor(byte value) override { pitchBend(0); _pitchBendSensitivity = value; }
	void transpose(int8 value) override { _transpose = value; pitchBend(_pitchBendSet); }
	void detune(int16 value) override { _detune = value; pitchBend(_pitchBendSet); }
	void priority(byte value) override  { _prio = value; }
	void sustain(bool value) override;
	void bankSelect(byte bank) override { _bank = bank; }
	void allNotesOff() override;
	void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override {}

private:
	void dataEntry(byte value);
	void updateVolume();
	bool _allocated;

	byte _prio;
	byte _bank;
	byte _volume;
	byte _panPos;
	int16 _detune;
	int8 _transpose;
	byte _polyphony;
	byte _usage;
	bool _overuse;
	int16 _pitchBendSet;
	byte _pitchBendSensitivity;
	int16 _pitchBendEff;
	uint16 _pitchBendRange;
	byte _rpn;
	bool _sustain;
	uint16 _prog;
	const byte _number;

	ChanControlNode *allocateNode(int prio);

	ChanControlNode **_channels;
	ChanControlNode *_rtmChannel;
	ChanControlNode *_out;
	static byte _allocCur;

	IMuseDriver_Macintosh *_drv;
	IMSMacSoundSystem *_device;

	const byte _numChannels;
	const int8 _version;
};

struct ChanControlNode {
	ChanControlNode(byte num) : in(nullptr), number(num), note(0), sustain(false), rhythmPart(false), prio(0x7F), volume(0x7F), panPos(0x40), velocity(0), pitchBend(0), prog(0), prev(nullptr), next(nullptr) {}
	IMuseChannel_Macintosh *in;
	const byte number;
	bool sustain;
	bool rhythmPart;
	byte prio;
	byte note;
	byte volume;
	byte panPos;
	byte velocity;
	int16 pitchBend;
	uint16 prog;
	ChanControlNode *prev;
	ChanControlNode *next;
};

void connect(ChanControlNode *&chain, ChanControlNode *node) {
	if (!node || node->prev || node->next)
		return;
	if ((node->next = chain))
		chain->prev = node;
	chain = node;
}

void disconnect(ChanControlNode *&chain, ChanControlNode *node) {
	if (!node || !chain)
		return;

	const ChanControlNode *ch = chain;
	while (ch && ch != node)
		ch = ch->next;
	if (!ch)
		return;

	if (node->next)
		node->next->prev = node->prev;

	if (node->prev)
		node->prev->next = node->next;
	else
		chain = node->next;

	node->in = nullptr;
	node->next = node->prev = nullptr;
}

void DeviceChannel::recalcFrequency() {
	int cpos = (pitch >> 7) + 60 - baseFreq;
	if (cpos < 0)
		frequency = (uint32)-1;
	else if ((pitch & 0x7F) && cpos < 0x7F)
		frequency = pitchTable[cpos] + (((pitchTable[cpos + 1] - pitchTable[cpos]) * (pitch & 0x7F)) >> 7);
	else
		frequency = pitchTable[cpos];
}

IMSMacSoundSystem::IMSMacSoundSystem(Audio::Mixer *mixer, byte version) : VblTaskClientDriver(), _mixer(mixer), _macstr(nullptr), _sdrv(nullptr), _vblTskProc(this, &VblTaskClientDriver::vblCallback),
	_musicChan(0), _sfxChan(0), _quality(22), _feedBufferSize(1024), _channels(nullptr), _timerParam(nullptr), _timerProc(nullptr), _pitchTable(nullptr),
	_ampTable(nullptr), _mixTable(nullptr), _numChannels(version ? 12 : 8), _defaultInstrID(0), _version(version), _stereo(false) {
	_pitchTable = new uint32[128]();
	assert(_pitchTable);
	_channels = new DeviceChannel*[_numChannels];
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i)
		_channels[i] = new DeviceChannel(_pitchTable);
	_mixBuffer16Bit = new int16[_feedBufferSize]();
	assert(_mixBuffer16Bit);
	fillPitchTable();
	_ampTable = new byte[8192]();
	assert(_ampTable);
	_mixTable = new int16[_numChannels << 8]();
	assert(_mixTable);
}

IMSMacSoundSystem::~IMSMacSoundSystem() {
	deinit();
	if (_channels) {
		for (int i = 0; i < _numChannels; ++i)
			delete _channels[i];
		delete[] _channels;
	}
	delete[] _pitchTable;
	delete[] _mixTable;
	delete[] _mixBuffer16Bit;
}

bool IMSMacSoundSystem::init(const char *const *instrFileNames, int numInstrFileNames, bool stereo, bool internal16Bit) {
	if (!loadInstruments(instrFileNames, numInstrFileNames))
		return false;

	_internal16Bit = internal16Bit;
	_stereo = (_version) > 0 && stereo;

	uint16 feedBufferSize = _stereo ? 2048 : 1024;
	if (_feedBufferSize != feedBufferSize) {
		delete[] _mixBuffer16Bit;
		_feedBufferSize = feedBufferSize;
		_mixBuffer16Bit = new int16[_feedBufferSize]();
		assert(_mixBuffer16Bit);
	}

	static const byte ampTableV0[32] = {
		0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x05, 0x06,
		0x08, 0x0b, 0x0d, 0x10, 0x13, 0x16, 0x1a, 0x1e,
		0x22, 0x26, 0x2b, 0x30, 0x35, 0x3a, 0x40, 0x46,
		0x4c, 0x53, 0x59, 0x60, 0x68, 0x6f, 0x77, 0x7f
	};

	byte *dst = _ampTable;
	for (int i = 0; i < 32; ++i) {
		byte mul = (_version == 0) ? ampTableV0[i] : i << 2;
		for (int ii = 0; ii < 256; ++ii)
			*dst++ = (((ii - 128) * mul) / 127) + 128;
	}

	uint32 m = _numChannels << 7;
	int16 *d1 = &_mixTable[m];
	int16 *d2 = &_mixTable[m - 1];

	if (_version == 0) {
		byte sh = _internal16Bit ? 5 : 0;
		byte div = _internal16Bit ? 1 : _numChannels;
		byte base = _internal16Bit? 0 : 128;
		for (uint32 i = 0; i < m; ++i) {
			uint16 val = (i << sh) / div;
			*d1++ = base + val;
			*d2-- = base - val - 1;
		}
	} else if (_internal16Bit) {
		for (uint32 i = 0; i < m; ++i) {
			uint16 val = (((i * _numChannels * 127) << 6) / (((_numChannels >> 1) - 1) * i + _numChannels * 127)) << 1;
			*d1++ = val;
			*d2-- = -val - 1;
		}
	} else {
		for (uint32 i = 0; i < m; ++i) {
			uint16 val = ((((i * _numChannels * 127) << 7) >> 1) / (((_numChannels >> 1) - 1) * i + _numChannels * 127) + 128) >> 7;
			*d1++ = (byte)(128 + val);
			*d2-- = (byte)(128 - val - 1);
		}
	}

	_macstr = new MacPlayerAudioStream(this, _mixer->getOutputRate(), _stereo, false, internal16Bit);
	if (!_macstr)
		return false;

	// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
	// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
	_sdrv = new MacLowLevelPCMDriver(_mixer->mutex(), _mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, internal16Bit);
	if (!_sdrv)
		return false;

	_macstr->initBuffers(1024);
	_macstr->addVolumeGroup(Audio::Mixer::kMusicSoundType);
	// Only MI2 and FOA have MIDI sound effects. Also, the later versions use a different update method.
	if (_version == 0) {
		_macstr->addVolumeGroup(Audio::Mixer::kSFXSoundType);
		_macstr->setVblCallback(&_vblTskProc);		
	}

	// The stream will by default expect 8-bit data and scale that to 16-bit. Or at least only low amplitude 16-bit data that only needs minor adjustment.
	// For full range 16-bit input, we need some post-process downscaling, otherwise the application of the ScummVM global volume setting will cause overflows.
	if (internal16Bit)
		_macstr->scaleVolume(0, 5);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void IMSMacSoundSystem::deinit() {
	Common::StackLock lock(_mixer->mutex());
	stop();
	_mixer->stopHandle(_soundHandle);
	_timerProc = nullptr;
	delete _macstr;
	_macstr = nullptr;
	delete _sdrv;
	_sdrv = nullptr;
	delete[] _ampTable;
	_ampTable = nullptr;
}

void IMSMacSoundSystem::stop() {
	Common::StackLock lock(_mixer->mutex());
	if (_sdrv) {
		_sdrv->disposeChannel(_musicChan);
		if (_sfxChan)
			_sdrv->disposeChannel(_sfxChan);
	}
}

void IMSMacSoundSystem::noteOn(const ChanControlNode *node) {
	assert(node);
	Common::StackLock lock(_mixer->mutex());

	DeviceChannel *c = allocateChannel(node);
	if (!c)
		return;

	setInstrument(c);

	if (c->instr == nullptr)
		return;

	debug(6, "NOTE ON: ims part %d, chan node %d, note %d, instr id %d (%s)", node->in ? node->in->getNumber() : node->number, node->number, node->note, node->prog, c->instr.get()->name());

	// SAMNMAX: This is a workaround to fix a hanging note in the final credits track. The issue is present in the original game,too.
	bool fixHangingNote = (c->instr.get()->id() == 321 && c->note == 40);
	if (fixHangingNote)
		debug(7, "%s:() Triggered hanging note workaround.", __FUNCTION__);

	recalcVolume(c);

	const MacLowLevelPCMDriver::PCMSound *s = c->instr.get()->data();
	c->baseFreq = !node->rhythmPart ? s->baseFreq : 60;
	c->rate = s->rate;
	c->smpBuffStart = s->data.get();
	c->smpBuffEnd = c->smpBuffStart + s->len;
	if ((_version == 0 && (int32)s->loopst >= (int32)s->loopend - 12) || (_version > 0 && (fixHangingNote || !s->loopst || !s->loopend || node->rhythmPart || (int32)s->loopst > (int32)s->len - 10))) {
		c->loopStart = nullptr;
		c->loopEnd = c->smpBuffEnd;
	} else {
		c->loopStart = c->smpBuffStart + s->loopst;
		c->loopEnd = c->smpBuffStart + s->loopend;
	}
	c->pitch = (node->note << 7) + node->pitchBend;
	c->mute = c->release = false;
	c->end = c->loopEnd;
	c->pos = c->smpBuffStart;
	c->phase = 0;

	recalcFrequency(c);
}

void IMSMacSoundSystem::noteOff(const ChanControlNode *node) {
	Common::StackLock lock(_mixer->mutex());
	for (int i = 0; i < _numChannels; ++i) {
		DeviceChannel *c = _channels[i];
		if (c->node == node) {
			noteOffIntern(c);
			c->node = nullptr;
		}
	}
}

void IMSMacSoundSystem::voiceOff(const ChanControlNode *node) {
	Common::StackLock lock(_mixer->mutex());
	for (int i = 0; i < _numChannels; ++i) {
		DeviceChannel *c = _channels[i];
		if (c->node == node) {
			c->mute = true;
			c->release = false;
			c->node = nullptr;
		}
	}
}

void IMSMacSoundSystem::setVolume(const ChanControlNode *node) {
	Common::StackLock lock(_mixer->mutex());
	for (int i = 0; i < _numChannels; ++i) {
		DeviceChannel *c = _channels[i];
		if (c->node == node)
			recalcVolume(c);
	}
}

void IMSMacSoundSystem::setPitchBend(const ChanControlNode *node) {
	Common::StackLock lock(_mixer->mutex());
	for (int i = 0; i < _numChannels; ++i) {
		DeviceChannel *c = _channels[i];
		if (c->node == node) {
			c->pitch = (node->note << 7) + node->pitchBend;
			recalcFrequency(c);
		}
	}
}

void IMSMacSoundSystem::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	_timerParam = timerParam;
	_timerProc = timerProc;
}

void IMSMacSoundSystem::vblCallback() {
	if (_timerProc)
		_timerProc(_timerParam);
}

void IMSMacSoundSystem::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	assert(dst);
	memset(dst, 0, len);
	_sdrv->feed(dst, len, type, expectStereo);
}

const MacSoundDriver::Status &IMSMacSoundSystem::getDriverStatus(Audio::Mixer::SoundType type) const {
	return _sdrv->getStatus(type);
}

void IMSMacSoundSystem::dblBuffCallback(MacLowLevelPCMDriver::DoubleBuffer *dblBuffer) {
	if (_version > 0 && _timerProc)
		_timerProc(_timerParam);

	uint16 sil = 0;
	memset(_mixBuffer16Bit, 0, _feedBufferSize * sizeof(uint16));
	uint16 frameSize = _stereo ? 2 : 1;

	for (int i = 0; i < _numChannels; ++i) {
		DeviceChannel &c = *_channels[i];

		if (c.release) {
			c.totalLevelL = (c.totalLevelL > 12) ? c.totalLevelL - 12 : 0;
			c.totalLevelR = (c.totalLevelR > 12) ? c.totalLevelR - 12 : 0;

			if (c.totalLevelL == 0 && c.totalLevelR == 0) {
				c.release = false;
				c.mute = true;
			}
		}

		if (c.mute || c.frequency == (uint32)-1 ||
			// This is our "trick" for telling apart music and sfx in MI2/INDY4: We look at the sound bank that
			// is currently in use, the musical one (1000 - 1127) or the sfx one (2000 - 2255). Unfortunately,
			// most sound effects seem to use the musical bank, though. So, in the end we can't distinguish all that much...
			(_version == 0 && ((dblBuffer->chanHandle == _musicChan && c.instr.get()->id() >= 2000) ||
			(dblBuffer->chanHandle == _sfxChan && c.instr.get()->id() < 2000)))) {
				++sil;
				continue;
		}

		const byte *a1 = &_ampTable[(c.totalLevelL & ~3) << 6];
		const byte *a2 = &_ampTable[(c.totalLevelR & ~3) << 6];
		int16 *t = _mixBuffer16Bit;

		for (int ii = 0; ii < _feedBufferSize; ii += frameSize) {
			if (!c.pos) {
				*t++ += 0x80;
				if (_stereo)
					*t++ += 0x80;
				continue;
			}
			c.phase += c.frequency;
			if (c.phase >> 16) {
				c.pos += (c.phase >> 16);
				c.phase &= 0xFFFF;
				if (c.loopEnd <= c.pos) {
					if (c.loopStart) {
						c.pos -= (c.loopEnd - c.loopStart);
					} else {
						c.pos = nullptr;
						c.mute = true;
						c.release = false;
						ii -= frameSize;
						continue;
					}
				}
			}
			*t++ += a1[*c.pos];
			if (_stereo)
				*t++ += a2[*c.pos];
		}
	}

	const int16 *s = _mixBuffer16Bit;
	sil <<= 7;

	if (_internal16Bit) {
		int16 *d = reinterpret_cast<int16*>(dblBuffer->data);
		for (int i = 0; i < _feedBufferSize; ++i)
			*d++ = _mixTable[sil + *s++];
	} else {
		byte *d = dblBuffer->data;
		for (int i = 0; i < _feedBufferSize; ++i)
			*d++ = _mixTable[sil + *s++];
	}

	dblBuffer->numFrames = _feedBufferSize / frameSize;
	dblBuffer->flags |= MacLowLevelPCMDriver::DoubleBuffer::kBufferReady;
}

void IMSMacSoundSystem::setMasterVolume(Audio::Mixer::SoundType type, uint16 volume) {
	if (_macstr)
		_macstr->setMasterVolume(type, volume);
}

bool IMSMacSoundSystem::setupResourceFork(Common::MacResManager &rm, const char *const *fileNames, int numFileNames) {
	const Common::CodePage tryCodePages[] = {
		Common::kMacRoman,
		Common::kISO8859_1
	};

	Common::Path resFile;

	for (int i = 0; resFile.empty() && i < numFileNames && fileNames[i] != nullptr; ++i) {
		for (int ii = 0; resFile.empty() && ii < ARRAYSIZE(tryCodePages); ++ii) {
			Common::U32String fn(fileNames[i], tryCodePages[ii]);
			resFile = Common::Path(fn.encode(Common::kUtf8));
			if (!rm.exists(resFile) || !rm.open(resFile) || !rm.hasResFork()) {
				rm.close();
				resFile = Common::Path(Common::punycode_encodefilename(fn));
				if (!rm.exists(resFile) || !rm.open(resFile) || !rm.hasResFork()) {
					rm.close();
					resFile.clear();
				}
			}
		}
	}

	if (resFile.empty()) {
		warning("%s(): Resource fork not found", __FUNCTION__);
		return false;
	}

	return true;
}

Common::SharedPtr<MacSndResource> IMSMacSoundSystem::getSndResource(uint16 id) {
	Common::SharedPtr<MacSndResource> res;
	Common::SharedPtr<MacSndResource> *def = nullptr;
	for (Common::Array<Common::SharedPtr<MacSndResource> >::iterator it = _smpResources.begin(); res == nullptr && it != _smpResources.end(); ++it) {
		uint16 cid = (*it)->id();
		if (cid == id)
			res = *it;
		else if (cid == _defaultInstrID)
			def = it;
	}
	if (res == nullptr) {
		if (def != nullptr)
			res = *def;
		else
			error("%s(): Failure (instrument id %d)", __FUNCTION__, id);
	}
	return res;
}

void IMSMacSoundSystem::fillPitchTable() {
	static const double ptbl[12] = {
		1664510.645469, 1763487.599042, 1868350.028545, 1979447.902589,
		2097152.000000, 2221855.147262, 2353973.529536, 2493948.079642,
		2642245.949629,	2799362.069852, 2965820.800758, 3142177.682886
	};

	for (int i = 0; i < 12; ++i)
		_pitchTable[116 + i] = (uint32)trunc(ptbl[i]);
	for (int i = 115; i >= 0; --i)
		_pitchTable[i] = _pitchTable[i + 12] >> 1;
	if (_quality != 11)
		return;
	for (int i = 0; i < 128; ++i)
		_pitchTable[i] <<= 1;
}

DJMSoundSystem::DJMSoundSystem(Audio::Mixer *mixer) : IMSMacSoundSystem(mixer, 0), _dbCbProc(this, &MacLowLevelPCMDriver::CallbackClient::dblBuffCallback) {
	_defaultInstrID = 999;
}

bool DJMSoundSystem::start() {
	_musicChan = _sdrv->createChannel(Audio::Mixer::kMusicSoundType, MacLowLevelPCMDriver::kSampledSynth, 0x8C, nullptr);
	_sfxChan = _sdrv->createChannel(Audio::Mixer::kSFXSoundType, MacLowLevelPCMDriver::kSampledSynth, 0x8C, nullptr);
	if (!_musicChan || !_sfxChan)
		return false;

	uint32 rate = 0;
	switch (_quality) {
	case 5:
		rate = 0x15BBA2E8;
		break;
	case 7:
		rate = 0x1CFA2E8B;
		break;
	case 11:
		rate = 0x2B7745D1;
		break;
	case 22:
		rate = 0x56EE8BA3;
		break;
	default:
		warning("%s(): Invalid quality setting %d", __FUNCTION__, _quality);
		return false;
	}

	return _sdrv->playDoubleBuffer(_musicChan, 1, _internal16Bit ? 16 : 8, rate, &_dbCbProc, _internal16Bit ? _numChannels : 1) &&
		_sdrv->playDoubleBuffer(_sfxChan, 1, _internal16Bit ? 16 : 8, rate, &_dbCbProc, _internal16Bit ? _numChannels : 1);
}

void DJMSoundSystem::setQuality(int qual) {
	Common::StackLock lock(_mixer->mutex());

	qual = qual > 1 ? 22 : 11;
	if (qual == _quality)
		return;

	_quality = qual;
	_feedBufferSize = (qual == 22) ? 1024 : 512;
	fillPitchTable();

	for (int i = 0; i < _numChannels; ++i)
		_channels[i]->recalcFrequency();

	stop();
	if (!start())
		error("%s(): Unknown error", __FUNCTION__);
}

bool DJMSoundSystem::loadInstruments(const char *const *fileNames, int numFileNames) {
	Common::MacResManager resMan;
	if (!setupResourceFork(resMan, fileNames, numFileNames))
		return false;

	_smpResources.clear();

	Common::MacResIDArray ids = resMan.getResIDArray(MKTAG('s', 'n', 'd', ' '));
	for (Common::MacResIDArray::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		Common::SeekableReadStream *str = resMan.getResource(MKTAG('s', 'n', 'd', ' '), *i);
		uint16 type = str ? str->readUint16BE() : 0;
		if (type == 1 || type == 2)
			_smpResources.push_back(Common::SharedPtr<MacSndResource>(new MacSndResource(*i, str, resMan.getResName(MKTAG('s', 'n', 'd', ' '), *i))));
		delete str;
	}

	return !_smpResources.empty();
}

void DJMSoundSystem::setInstrument(DeviceChannel *chan) {
	assert(chan && chan->node);
	if (chan->instr == nullptr || chan->prog != chan->node->prog) {
		chan->instr = getSndResource(chan->node->prog);
		chan->prog = chan->node->prog;
	}
}

void DJMSoundSystem::recalcFrequency(DeviceChannel *chan) {
	assert(chan);
	chan->recalcFrequency();
	if (chan->instr != nullptr)
		chan->mute = false;
}

void DJMSoundSystem::recalcVolume(DeviceChannel *c) {
	assert(c && c->node);
	c->totalLevelL = c->node->volume;
	if (c->instr != nullptr)
		c->mute = false;
 }

void DJMSoundSystem::noteOffIntern(DeviceChannel *chan) {
	if (chan->loopStart)
		chan->mute = true;
	chan->note = 0;
}

DeviceChannel *DJMSoundSystem::allocateChannel(const ChanControlNode *node) {
	assert(node && node->number < _numChannels);
	DeviceChannel *c = _channels[node->number];
	c->node = node;
	return c;
}

NewMacSoundSystem::NewMacSoundSystem(ScummEngine *vm, Audio::Mixer *mixer) : IMSMacSoundSystem(mixer, 1), // It is sufficient to distinguish version 0 and 1 here.
	_fileMan(nullptr), _container(vm ? vm->_containerFile : _dummy), _dbCbProc(this, &MacLowLevelPCMDriver::CallbackClient::dblBuffCallback) {
	assert(vm);
	_defaultInstrID = 0xFFFF;
	_fileMan = new ScummFile(vm);
}

NewMacSoundSystem::~NewMacSoundSystem() {
	delete _fileMan;
}

bool NewMacSoundSystem::start() {
	_musicChan = _sdrv->createChannel(Audio::Mixer::kMusicSoundType, MacLowLevelPCMDriver::kSampledSynth, _stereo ? 0xCC : 0x8C, nullptr);
	return _musicChan ? _sdrv->playDoubleBuffer(_musicChan, _stereo ? 2 : 1, _internal16Bit ? 16 : 8, 0x56220000, &_dbCbProc,  _internal16Bit ? _numChannels : 1) : false;
}

bool NewMacSoundSystem::loadInstruments(const char *const *fileNames, int numFileNames) {
	if ((_container.empty() && !_fileMan->open(fileNames[0])) || (!_container.empty() && (!_fileMan->open(_container) || !_fileMan->openSubFile(fileNames[0]))))
		return false;

	uint32 num = _fileMan->readUint32BE();
	uint32 size = _fileMan->readUint32BE();

	byte *buff = new byte[size]();
	_fileMan->read(buff, size);
	_fileMan->close();

	_smpResources.clear();
	const byte *s = buff;
	for (uint i = 0; i < num; ++i) {
		uint32 offset = READ_BE_UINT32(s + 4);
		uint32 resSize = (i < num - 1 ? READ_BE_UINT32(s + 12) : size) - offset;
		_smpResources.push_back(Common::SharedPtr<MacSndResource>(new MacSndResource(READ_BE_UINT16(s + 2), buff + offset, resSize)));
		s += 8;
	}
	delete[] buff;

	if (_smpResources.empty())
		return false;

	Common::MacResManager resMan;
	if (!setupResourceFork(resMan, &fileNames[1], numFileNames - 1))
		return false;

	Common::MacResIDArray instIDs = resMan.getResIDArray(MKTAG('I', 'N', 'S', 'T'));
	for (Common::MacResIDArray::const_iterator i = instIDs.begin(); i != instIDs.end(); ++i) {
		Common::SeekableReadStream *str = resMan.getResource(MKTAG('I', 'N', 'S', 'T'), *i);
		if (!str)
			error("%s(): Failed to load instrument resource (type 'INST', id '%d')", __FUNCTION__, *i);
		uint sz = str->size();
		byte *b = new byte[sz]();
		str->read(b, sz);
		delete str;

		Instrument *ins = new Instrument(*i);

		ins->sndRes.push_back(getSndResource(READ_BE_UINT16(b)));
		if (ins->sndRes[0] != nullptr)
			memset(ins->noteSmplsMapping, 0, 128);

		byte numRanges = CLIP<int8>(b[13], 0, 7);
		assert(sz >= 16u + numRanges * 8u);

		for (int ii = 0; ii < numRanges; ++ii) {
			ins->sndRes.push_back(getSndResource(READ_BE_INT16(b + 16 + ii * 8)));
			if (ins->sndRes.back() != nullptr) {
				byte rl = b[14 + ii * 8];
				byte rh = b[15 + ii * 8];
				assert(rl < 128 && rh < 128);
				for (int iii = rl; iii <= rh; ++iii)
					ins->noteSmplsMapping[iii] = ii + 1;
			}
		}
		delete[] b;
		_instruments.push_back(Common::SharedPtr<Instrument>(ins));
	}

	return !_instruments.empty();
}

Common::SharedPtr<MacSndResource> NewMacSoundSystem::getNoteRangeSndResource(uint16 id, byte note) {
	assert(note < 128);
	Common::SharedPtr<MacSndResource> res;
	for (Common::Array<Common::SharedPtr<Instrument> >::const_iterator it = _instruments.begin(); res == nullptr && it != _instruments.end(); ++it) {
		uint16 cid = (*it)->id;
		if (cid == id)
			res = (*it)->sndRes[(*it)->noteSmplsMapping[note]];
	}
	return res;
}

void NewMacSoundSystem::setInstrument(DeviceChannel *chan) {
	assert(chan && chan->node);
	if (chan->instr == nullptr || chan->node->rhythmPart != chan->rhtm || (!chan->node->rhythmPart && chan->prog != chan->node->prog) || chan->note != chan->node->note) {
		chan->note = chan->node->note;
		chan->prog = chan->node->rhythmPart ? 0 : chan->node->prog;
		chan->rhtm = chan->node->rhythmPart;
		chan->instr = chan->node->rhythmPart ? getSndResource(6000 + chan->note) : getNoteRangeSndResource(chan->prog, chan->note);
	}
}

void NewMacSoundSystem::recalcFrequency(DeviceChannel *chan) {
	assert(chan && chan->node);
	if (chan->node->rhythmPart)
		chan->frequency = 0x8000;
	else
		chan->recalcFrequency();
	chan->frequency = MacLowLevelPCMDriver::calcRate(0x56220000, chan->rate , chan->frequency);
	//if (chan->frequency == (uint32)-1)
	//	error("%s(): Frequency calculation failed", __FUNCTION__);
}

void NewMacSoundSystem::recalcVolume(DeviceChannel *chan) {
	assert(chan && chan->node);

	static const byte volumeTable[] = {
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
		0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05,
		0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07,
		0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a,
		0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0d,
		0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x10, 0x11,
		0x11, 0x12, 0x12, 0x13, 0x14, 0x14, 0x15, 0x16,
		0x16, 0x17, 0x18, 0x18, 0x19, 0x1a, 0x1b, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
		0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
		0x2d, 0x2e, 0x2f, 0x31, 0x32, 0x33, 0x35, 0x36,
		0x38, 0x39, 0x3b, 0x3c, 0x3e, 0x40, 0x42, 0x43,
		0x45, 0x47, 0x49, 0x4b, 0x4d, 0x4f, 0x51, 0x53,
		0x56, 0x58, 0x5a, 0x5d, 0x5f, 0x62, 0x64, 0x67,
		0x6a, 0x6c, 0x6f, 0x72, 0x75, 0x78, 0x7c, 0x7f
	};

	if (_stereo) {
		chan->volumeL = (volumeTable[127 - (chan->node->panPos >> 2)] * chan->node->volume) >> 7;
		chan->volumeR = (volumeTable[96 + (chan->node->panPos >> 2)] * chan->node->volume) >> 7;
	} else {
		chan->volumeL = chan->node->volume;
	}

	static const byte veloTable[] = {
		0x41, 0x43, 0x45, 0x46, 0x47, 0x49, 0x4a, 0x4b,
		0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
		0x53, 0x54, 0x55, 0x55, 0x56, 0x57, 0x57, 0x58,
		0x58, 0x59, 0x59, 0x59, 0x5a, 0x5a, 0x5b, 0x5b,
		0x5b, 0x5c, 0x5c, 0x5c, 0x5d, 0x5d, 0x5d, 0x5d,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5f, 0x5f,
		0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x60, 0x60, 0x60,
		0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
		0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
		0x60, 0x60, 0x60, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62,
		0x63, 0x63, 0x63, 0x63, 0x64, 0x64, 0x64, 0x65,
		0x65, 0x65, 0x66, 0x66, 0x67, 0x67, 0x67, 0x68,
		0x68, 0x69, 0x69, 0x6a, 0x6b, 0x6b, 0x6c, 0x6d,
		0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
		0x75, 0x76, 0x77, 0x79, 0x7a, 0x7b, 0x7d, 0x7f
	};

	if (chan->release)
		return;

	chan->totalLevelL = volumeTable[(veloTable[chan->node->velocity] * chan->volumeL) >> 7];
	chan->totalLevelR = volumeTable[(veloTable[chan->node->velocity] * chan->volumeR) >> 7];
}

void NewMacSoundSystem::noteOffIntern(DeviceChannel *chan) {
	assert(chan && chan->node);
	if (!chan->node->rhythmPart)
		chan->release = true;
	chan->loopStart = nullptr;
	chan->loopEnd = chan->smpBuffEnd;
}

DeviceChannel *NewMacSoundSystem::allocateChannel(const ChanControlNode *node) {
	assert(node);
	DeviceChannel *res = nullptr;
	DeviceChannel *res2 = nullptr;
	DeviceChannel *res3 = nullptr;
	byte lovol = 0x7F;
	byte lopri = node->prio;

	for (byte i = 0; i < _numChannels; ++i) {
		DeviceChannel *c = _channels[i];

		if (c->mute) {
			res = c;
			break;
		}

		if (c->release) {
			if (c->node && c->node->volume < lovol) {
				res2 = c;
				lovol = c->node->volume;
			}
		}

		if (c->node && c->node->prio < lopri) {
			res3 = c;
			lopri = c->node->prio;
		}
	}

	if (!res)
		res = res2;
	if (!res)
		res = res3;

	if (res) {
		res->release = false;
		res->mute = true;
		res->node = node;
	}

	return res;
}

byte IMuseChannel_Macintosh::_allocCur = 0;

IMuseChannel_Macintosh::IMuseChannel_Macintosh(IMuseDriver_Macintosh *drv, int number) : MidiChannel(), _drv(drv), _number(number), _allocated(false), _polyphony(1), _usage(0),
	_sustain(false), _bank(0), _panPos(0x40), _pitchBendEff(0), _prio(0x80), _detune(0), _transpose(0), _pitchBendSet(0), _pitchBendSensitivity(2), _volume(0x7F), _overuse(false),
	_rpn(0), _pitchBendRange(0), _channels(drv ? drv->_channels : nullptr), _prog(0), _out(nullptr), _device(drv ? drv->_device : nullptr), _version(drv ? drv->_version : -1),
	_rtmChannel((drv && drv->_version == 1) ? drv->_channels[drv->_numChannels - 1] : nullptr), _numChannels(drv ? (drv->_version != 1 ? drv->_numChannels : drv->_numChannels - 1) : 0) {
	assert(_drv);
	assert(_channels);
	assert(_device);
	_allocCur = 0;
}

bool IMuseChannel_Macintosh::allocate() {
	if (_allocated)
		return false;

	_usage = 0;
	_overuse = false;

	return (_allocated = true);
}

void IMuseChannel_Macintosh::noteOff(byte note)  {
	for (ChanControlNode *node = (_version == 1 && _number == kRhythmPart) ? _rtmChannel : _out; node; ) {
		ChanControlNode *n = node->next;
		if (node->note == note) {
			if (_sustain && node != _rtmChannel) {
				node->sustain = true;
			} else {
				_device->noteOff(node);
				disconnect(_out, node);
				if (_version == 2)
					_overuse = (--_usage > _polyphony);
			}
		}
		node = n;
	}
}

void IMuseChannel_Macintosh::noteOn(byte note, byte velocity)  {
	ChanControlNode *node = allocateNode(_prio);
	if (node == nullptr)
		return;

	if (_version == 0 || _number != kRhythmPart) {
		node->in = this;
		node->prio = _prio;
		node->prog = _prog;
		node->pitchBend = _pitchBendEff;
		node->volume = _volume;
		node->rhythmPart = false;
	} else {
		if (_version == 2) {
			node->in = this;
			node->prio = _prio;
		}
		node->prog = 0;
		node->volume = _volume * 6 / 7;
		node->rhythmPart = true;
	}

	node->note = note;
	node->sustain = false;
	node->panPos = _panPos;
	node->velocity = velocity;

	if (_version == 2)
		_overuse = (++_usage > _polyphony);

	_device->noteOn(node);
}

void IMuseChannel_Macintosh::controlChange(byte control, byte value)  {
	switch (control) {
	case 0:
		// The original MI2/INDY4 code doesn't have that. It will just call a different
		// programChange() method from the sysex handler. Only DOTT and SAMNMAX call a
		// bank select, but the actual selection is not implemented, since it is not needed.
		_bank = value;
		break;
	case 6:
		dataEntry(value);
		break;
	case 7:
	case 10: {
		byte &param = (control == 7) ? _volume : _panPos;
		param = value;
		updateVolume();
		} break;
	case 17:
		if (_version == 2)
			_polyphony = value;
		else
			detune(value);
		break;
	case 18:
		priority(value);
		break;
	case 64:
		sustain(value);
		break;
	case 100:
		_rpn = value;
		break;
	case 101:
		_rpn = value | 0x80;
		break;
	case 123:
		allNotesOff();
		break;
	default:
		break;
	}
}

void IMuseChannel_Macintosh::programChange(byte program)  {
	if (_version == 0)
		_prog = (_bank ? 2000 : 1000) + program;
	else
		_prog = program;
}

void IMuseChannel_Macintosh::pitchBend(int16 bend)  {
	_pitchBendSet = bend;

	if (_version == 2)
		bend = (((bend * _pitchBendSensitivity) >> 5) + _detune + (_transpose << 8)) << 1; // SAMNMAX formula
	else
		bend = ((bend * _pitchBendSensitivity) >> 6) + _detune + (_transpose << 7); // DOTT, INDY4 and MI2 formula

	if (_version > 0) {
		if (_version == 1)
			bend = CLIP<int16>(bend, -2048, 2047) << 2;
		if (bend > 7936)
			bend = 8192;
		else if (bend < -7936)
			bend = -8192;
		bend = (bend * _pitchBendRange) >> 13;
	}

	_pitchBendEff = bend;

	if (_pitchBendSet)
		debug(6, "PITCH BEND: ims part %d, pb para %d, pb snstvty %d, pb range %d, _detune %d, transpose %d, pb eff %d", _number, _pitchBendSet, _pitchBendSensitivity, _pitchBendRange, _detune, _transpose, _pitchBendEff);

	for (ChanControlNode *node = _out; node; node = node->next) {
		node->pitchBend = _pitchBendEff;
		_device->setPitchBend(node);
	}
}

void IMuseChannel_Macintosh::sustain(bool value) {
	_sustain = value;
	if (value)
		return;

	for (ChanControlNode *node = _out; node; )  {
		ChanControlNode *n = node->next;
		if (node->sustain) {
			_device->noteOff(node);
			disconnect(_out, node);
			if (_version == 2)
				_overuse = (--_usage > _polyphony);
		}
		node = n;
	}
}

void IMuseChannel_Macintosh::allNotesOff() {
	for (ChanControlNode *node = (_version == 1 && _number == kRhythmPart) ? _rtmChannel : _out; node; ) {
		_device->voiceOff(node);
		ChanControlNode *n = node->next;
		disconnect(_out, node);
		if (_version == 2)
			_overuse = (--_usage > _polyphony);
		node = n;
	}
}

void IMuseChannel_Macintosh::dataEntry(byte value) {
	uint16 m = 0x7F;
	uint16 v = value;
	if (_rpn & 0x80) {
		m <<= 7;
		v <<= 7;
	}
	if (!(_rpn & 0x7F)) {
		_pitchBendRange = (_pitchBendRange & ~m) + v;
		pitchBend(_pitchBendSet);
	}
	_rpn = 0;
}

void IMuseChannel_Macintosh::updateVolume() {
	for (ChanControlNode *node = _out; node; node = node->next) {
		node->panPos = _panPos;
		node->volume = (_version > 0 && _number == kRhythmPart) ? _volume * 6 / 7 : _volume;
		_device->setVolume(node);
	}
}

ChanControlNode *IMuseChannel_Macintosh::allocateNode(int prio) {
	if (_version > 0) {
		_allocCur = 0;
		if (_version == 1 && _number == kRhythmPart)
			return _rtmChannel;
	}

	if (_version < 1 && _prog == 0)
		return nullptr;

	IMuseChannel_Macintosh *best = this;
	ChanControlNode *res = nullptr;

	for (byte i = 0; i < _numChannels; ++i) {
		_allocCur = (_allocCur + 1) % _numChannels;
		ChanControlNode *node = _channels[_allocCur];
		if (node->in == nullptr) {
			res = node;
			best = nullptr;
			break;
		}

		if ((_version == 2 && ((best->_overuse == node->in->_overuse && best->_prio >= node->prio) || (!best->_overuse && node->in->_overuse))) ||
			(_version < 2 && (!node->next && node->in && node->in->_prio <= prio))) {
			res = node;
			best = node->in;
			prio = best->_prio;
		}
	}

	if (res && best) {
		if (_version == 2)
			best->_overuse = (--best->_usage > best->_polyphony);
		_device->voiceOff(res);
		disconnect(best->_out, res);
	}

	assert(!(_out && _out == res));
	connect(_out, res);

	return res;
}

} // End of namespace IMSMacintosh

namespace Scumm {
using namespace IMSMacintosh;

IMuseDriver_Macintosh::IMuseDriver_Macintosh(ScummEngine *vm, Audio::Mixer *mixer, byte gameID) : MidiDriver(), _isOpen(false), _device(nullptr), _imsParts(nullptr), _channels(nullptr),
	_numParts(32), _numChannels(8), _baseTempo(16666), _quality(1), _musicVolume(0), _sfxVolume(0), _version(-1) {

	switch (gameID) {
	case GID_TENTACLE:
	case GID_SAMNMAX:
		if (gameID == GID_SAMNMAX) {
			_version = 2;
			_numChannels = 12;
		} else {
			_version = 1;
			_numChannels = 16;
		}
		_baseTempo = 46439;
		_device = new NewMacSoundSystem(vm, mixer);
		break;
	case GID_INDY4:
	case GID_MONKEY2:
		_version = 0;
		_device = new DJMSoundSystem(mixer);
		break;
	default:
		error("%s(): Unsupported game ID %d", __FUNCTION__, gameID);
		break;
	}
}

IMuseDriver_Macintosh::~IMuseDriver_Macintosh() {
	close();
	delete _device;
}

int IMuseDriver_Macintosh::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (_version < 0 || _version > 2)
		return MERR_DEVICE_NOT_AVAILABLE;

	createChannels();

	static const char *const fileNames[3][3] = {
		{
			"iMUSE Setups",
			nullptr,
			nullptr
		},
		{
			"Instruments",
			"Day of the Tentacle Demo",
			"Day of the Tentacle"
		},
		{
			"Instruments",
			"Sam & Max Demo",
			"Sam & Max"
		}
	};

	if (!_device->init(fileNames[_version], ARRAYSIZE(fileNames[_version]), false, true) || !_device->start())
		return MERR_DEVICE_NOT_AVAILABLE;

	_isOpen = true;

	for (int i = 0; i < _numParts; ++i) {
		_imsParts[i]->controlChange(0x64, 0x00);
		_imsParts[i]->controlChange(0x65, 0x00);
		_imsParts[i]->controlChange(0x06, 0x10);
		_imsParts[i]->controlChange(0x07, 0x7F);
		_imsParts[i]->controlChange(0x0A, 0x40);
		_imsParts[i]->controlChange(0x01, 0x00);
		_imsParts[i]->controlChange(0x40, 0x00);
		_imsParts[i]->controlChange(0x5B, 0x40);
		_imsParts[i]->controlChange(0x5D, 0x00);
		_imsParts[i]->controlChange(0x00, 0x00);
		_imsParts[i]->controlChange(0x7B, 0x00);
		_imsParts[i]->programChange(0);
		_imsParts[i]->pitchBend(0);
	}

	return 0;
}

void IMuseDriver_Macintosh::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	_device->deinit();
	releaseChannels();
}

uint32 IMuseDriver_Macintosh::property(int prop, uint32 param) {
	uint32 res = 0;
	switch (prop) {
	case IMuse::PROP_QUALITY:
		res = _quality;
		if (param != (uint32)-1 && param != _quality) {
			_quality = param;
			if (_device)
				_device->setQuality(param == MacSound::kQualityAuto || param == MacSound::kQualityHighest ? 2: 0);
		}
		break;

	case IMuse::PROP_MUSICVOLUME:
		res = _musicVolume;
		if (param != (uint32)-1 && param != _musicVolume) {
			_musicVolume = param;
			if (_device)
				_device->setMasterVolume(Audio::Mixer::kMusicSoundType, param);
		}
		break;
	case IMuse::PROP_SFXVOLUME:
		res = _sfxVolume;
		if (param != (uint32)-1 && param != _sfxVolume) {
			_sfxVolume = param;
			if (_device)
				_device->setMasterVolume(Audio::Mixer::kSFXSoundType, param);
		}
		break;
	default:
		break;
	}
	return res;
}

void IMuseDriver_Macintosh::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	if (_device)
		_device->setTimerCallback(timerParam, timerProc);
}

MidiChannel *IMuseDriver_Macintosh::allocateChannel() {
	if (!_isOpen)
		return nullptr;

	for (int i = 0; i < _numParts; ++i) {
		IMuseChannel_Macintosh *ch = _imsParts[i];
		if (ch && !(_version > 0 && i == kRhythmPart) && ch->allocate())
			return ch;
	}

	return nullptr;
}

MidiChannel *IMuseDriver_Macintosh::getPercussionChannel() {
	return (_isOpen && _version > 0) ? _imsParts[kRhythmPart] : nullptr;
}

void IMuseDriver_Macintosh::createChannels() {
	releaseChannels();

	_channels = new ChanControlNode*[_numChannels];
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i) {
		_channels[i] = new ChanControlNode(i < kRhythmPart ? i : (i == _numChannels - 1 ? kRhythmPart : i + 1));
		assert(_channels[i]);
	}

	_imsParts = new IMuseChannel_Macintosh*[_numParts];
	assert(_imsParts);
	for (int i = 0; i < _numParts; ++i)
		_imsParts[i] = new IMuseChannel_Macintosh(this, i);
}

void IMuseDriver_Macintosh::releaseChannels() {
	if (_imsParts) {
		for (int i = 0; i < _numParts; ++i)
			delete _imsParts[i];
		delete[] _imsParts;
		_imsParts = nullptr;
	}

	if (_channels) {
		for (int i = 0; i < _numChannels; ++i)
			delete _channels[i];
		delete[] _channels;
		_channels = nullptr;
	}
}

} // End of namespace Scumm
