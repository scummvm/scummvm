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
#include "scumm/players/mac_sound_lowlevel.h"
#include "scumm/players/player_mac_new.h"
#include "scumm/imuse/drivers/macintosh.h"
#include "scumm/imuse/imuse.h"


namespace IMSMacintosh {
using namespace Scumm;

class DJMSoundChannel {
public:
	DJMSoundChannel(const uint32 *pitchTable) : _pitchTable(pitchTable), _frequency(0), _phase(0), _end(nullptr), _pos(nullptr), _smpBuffStart(nullptr), _smpBuffEnd(nullptr), _loopStart(nullptr), _loopEnd(nullptr), _pitch(0), _volume(0), _mute(1), _instr(nullptr), _baseFreq(0) {}
	~DJMSoundChannel() {}

	void recalcFrequency();

	uint32 _frequency;
	uint32 _phase;
	const byte *_end;
	const byte *_pos;
	const byte *_smpBuffStart;
	const byte *_smpBuffEnd;
	const byte *_loopStart;
	const byte *_loopEnd;
	uint16 _pitch;
	byte _volume;
	byte _mute;
	byte _baseFreq;
	Common::SharedPtr<MacSndInstrument> _instr;
	const uint32 *_pitchTable;
};

class DJMSoundSystem : public VblTaskClientDriver, public MacLowLevelPCMDriver::CallbackClient {
public:
	DJMSoundSystem(Audio::Mixer *mixer);
	~DJMSoundSystem();

	bool init(bool internal16Bit);
	void deinit();
	bool start();
	void stop();

	void setQuality(int qual);

	void setInstrument(byte chan, Common::SharedPtr<MacSndInstrument> &instr);
	void noteOn(byte chan, uint16 pitch, byte volume);
	void noteOff(byte chan);
	void setVolume(byte chan, byte volume);
	void setPitch(byte chan, uint16 pitch);

	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc);

	void vblCallback() override;
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const override;
	const MacSoundDriver::Status &getDriverStatus(Audio::Mixer::SoundType type) const override;

	void dblBuffCallback(MacLowLevelPCMDriver::DoubleBuffer *dblBuffer) override;

	void setMasterVolume(Audio::Mixer::SoundType type, uint16 volume);

private:
	void fillPitchTable();

	byte _quality;
	uint16 _feedBufferSize;
	uint32 *_pitchTable;
	byte *_ampTable;
	int16 *_mixBuffer16Bit;

	DJMSoundChannel **_channels;

	MacPlayerAudioStream *_macstr;
	MacLowLevelPCMDriver *_sdrv;
	Audio::SoundHandle _soundHandle;
	MacPlayerAudioStream::CallbackProc _vblTskProc;
	MacLowLevelPCMDriver::ChanHandle _musicChan;
	MacLowLevelPCMDriver::ChanHandle _sfxChan;
	MacLowLevelPCMDriver::DBCallback _dbCbProc;

	Audio::Mixer *_mixer;
	void *_timerParam;
	Common::TimerManager::TimerProc _timerProc;
	bool _internal16Bit;
	const byte _numChannels;
};

struct ChannelNode;
class IMuseChannel_Mac_DJM final : public MidiChannel {
public:
	IMuseChannel_Mac_DJM(IMuseDriver_Mac_DJM *drv, int number);
	~IMuseChannel_Mac_DJM() override;

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
	void priority(byte value) override { _prio = value; }
	void sustain(bool value) override;
	void allNotesOff() override;
	void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override {}

private:
	bool _allocated;

	byte _prio;
	byte _bank;
	byte _volume;
	int16 _detune;
	int8 _transpose;
	int16 _pitchBendSet;
	byte _pitchBendSensitivity;
	int16 _pitchBendEff;
	bool _sustain;
	Common::SharedPtr<MacSndInstrument> _prog;
	const byte _number;

	ChannelNode *allocateDeviceChannel(int prio);

	ChannelNode **_channels;
	ChannelNode *_out;
	static byte _allocCur;

	IMuseDriver_Mac_DJM *_drv;
	DJMSoundSystem *_device;

	Common::Array<Common::SharedPtr<MacSndInstrument> > *_instruments;
	const byte _numChannels;
};

struct ChannelNode {
	ChannelNode(int num) : _prev(nullptr), _next(nullptr), _in(nullptr), _number(num), _note(0), _sustain(false) {}
	ChannelNode *_prev;
	ChannelNode *_next;
	IMuseChannel_Mac_DJM *_in;
	const byte _number;
	byte _note;
	bool _sustain;
};

void connect(ChannelNode *&chain, ChannelNode *node) {
	if (!node || node->_prev || node->_next)
		return;
	if ((node->_next = chain))
		chain->_prev = node;
	chain = node;
}

void disconnect(ChannelNode *&chain, ChannelNode *node) {
	if (!node || !chain)
		return;

	const ChannelNode *ch = chain;
	while (ch && ch != node)
		ch = ch->_next;
	if (!ch)
		return;

	if (node->_next)
		node->_next->_prev = node->_prev;

	if (node->_prev)
		node->_prev->_next = node->_next;
	else
		chain = node->_next;

	node->_in = nullptr;
	node->_next = node->_prev = nullptr;
}

void DJMSoundChannel::recalcFrequency() {
	int pos = (_pitch >> 7) + 60 - _baseFreq;
	assert(pos>=0);
	if (_pitch & 0x7F)
		_frequency = _pitchTable[pos] + (((_pitchTable[pos + 1] - _pitchTable[pos]) * (_pitch & 0x7F)) >> 7);
	else
		_frequency = _pitchTable[pos];
}

DJMSoundSystem::DJMSoundSystem(Audio::Mixer *mixer) : VblTaskClientDriver(), _mixer(mixer), _macstr(nullptr), _sdrv(nullptr), _vblTskProc(this, &VblTaskClientDriver::vblCallback),
	_dbCbProc(this, &MacLowLevelPCMDriver::CallbackClient::dblBuffCallback), _musicChan(0), _sfxChan(0), _quality(22), _feedBufferSize(1024), _channels(nullptr),
		_timerParam(nullptr), _timerProc(nullptr), _pitchTable(nullptr), _ampTable(nullptr), _numChannels(8) {
	_pitchTable = new uint32[128]();
	assert(_pitchTable);
	_channels = new DJMSoundChannel*[_numChannels];
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i)
		_channels[i] = new DJMSoundChannel(_pitchTable);
	_mixBuffer16Bit = new int16[_feedBufferSize]();
	assert(_mixBuffer16Bit);
	fillPitchTable();
	_ampTable = new byte[8192]();
	assert(_ampTable);
}

DJMSoundSystem::~DJMSoundSystem() {
	deinit();
	if (_channels) {
		for (int i = 0; i < _numChannels; ++i)
			delete _channels[i];
		delete[] _channels;
	}
	delete[] _pitchTable;
	delete[] _mixBuffer16Bit;
}

bool DJMSoundSystem::init(bool internal16Bit) {
	_internal16Bit = internal16Bit;

	static const byte ampTable[32] = {
		0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x05, 0x06,
		0x08, 0x0b, 0x0d, 0x10, 0x13, 0x16, 0x1a, 0x1e,
		0x22, 0x26, 0x2b, 0x30, 0x35, 0x3a, 0x40, 0x46,
		0x4c, 0x53, 0x59, 0x60, 0x68, 0x6f, 0x77, 0x7f
	};

	byte *dst = _ampTable;
	byte offs = _internal16Bit ? 0 : 128;
	for (int i = 0; i < 32; ++i) {
		for (int ii = 0; ii < 256; ++ii)
			*dst++ = (((ii - 128) * ampTable[i]) / 127) + offs;
	}

	_macstr = new MacPlayerAudioStream(this, _mixer->getOutputRate(), false, false, internal16Bit);
	if (!_macstr)
		return false;

	// We can start this with the ScummVM mixer output rate instead of the ASC rate. The Mac sample rate converter can handle it (at
	// least for up to 48 KHz, I haven't tried higher settings) and we don't have to do another rate conversion in the ScummVM mixer.
	_sdrv = new MacLowLevelPCMDriver(_mixer->mutex(), _mixer->getOutputRate() << 16/*ASC_DEVICE_RATE*/, internal16Bit);
	if (!_sdrv)
		return false;

	_macstr->initBuffers(1024);
	_macstr->addVolumeGroup(Audio::Mixer::kMusicSoundType);
	_macstr->addVolumeGroup(Audio::Mixer::kSFXSoundType);
	_macstr->scaleVolume(2);
	_macstr->setVblCallback(&_vblTskProc);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _macstr, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return true;
}

void DJMSoundSystem::deinit() {
	Common::StackLock lock(_mixer->mutex());
	stop();
	_mixer->stopHandle(_soundHandle);
	delete _macstr;
	_macstr = nullptr;
	delete _sdrv;
	_sdrv = nullptr;
	delete[] _ampTable;
	_ampTable = nullptr;
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

	return _sdrv->playDoubleBuffer(_musicChan, 1, _internal16Bit ? 16 : 8, rate, &_dbCbProc, _internal16Bit ? 8 : 1) &&
		_sdrv->playDoubleBuffer(_sfxChan, 1, _internal16Bit ? 16 : 8, rate, &_dbCbProc, _internal16Bit ? 8 : 1);
}

void DJMSoundSystem::stop() {
	Common::StackLock lock(_mixer->mutex());
	if (_sdrv) {
		_sdrv->disposeChannel(_musicChan);
		_sdrv->disposeChannel(_sfxChan);
	}
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

void DJMSoundSystem::setInstrument(byte chan, Common::SharedPtr<MacSndInstrument> &instr) {
	Common::StackLock lock(_mixer->mutex());
	if (chan > 7)
		return;
	DJMSoundChannel &c = *_channels[chan];

	if (c._instr == instr)
		return;

	const MacLowLevelPCMDriver::PCMSound *s = instr.get()->data();
	c._instr = instr;
	c._baseFreq = s->baseFreq;
	c._smpBuffStart = s->data.get();
	c._smpBuffEnd = c._smpBuffStart + s->len;
	if (s->loopst >= s->loopend - 12) {
		c._loopStart = nullptr;
		c._loopEnd = c._smpBuffEnd;
	} else {
		c._loopStart = c._smpBuffStart + s->loopst;
		c._loopEnd = c._smpBuffStart + s->loopend;
	}	
}

void DJMSoundSystem::noteOn(byte chan, uint16 pitch, byte volume) {
	Common::StackLock lock(_mixer->mutex());
	if (chan > 7)
		return;
	DJMSoundChannel &c = *_channels[chan];
	c._pitch = pitch;
	c._volume = volume;
	c._mute = 0;
	c._end = c._loopEnd;
	c._pos = c._smpBuffStart;
	c._phase = 0;
	c.recalcFrequency();
}

void DJMSoundSystem::noteOff(byte chan) {
	Common::StackLock lock(_mixer->mutex());
	if (chan > 7)
		return;
	DJMSoundChannel &c = *_channels[chan];
	if (c._loopStart)
		c._mute = 1;
}

void DJMSoundSystem::setVolume(byte chan, byte volume) {
	Common::StackLock lock(_mixer->mutex());
	if (chan > 7)
		return;
	DJMSoundChannel &c = *_channels[chan];
	c._volume = volume;
	c._mute = 0;
}

void DJMSoundSystem::setPitch(byte chan, uint16 pitch) {
	Common::StackLock lock(_mixer->mutex());
	if (chan > 7)
		return;
	DJMSoundChannel &c = *_channels[chan];
	c._pitch = pitch;
	c._mute = 0;
	c.recalcFrequency();
}

void DJMSoundSystem::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	_timerParam = timerParam;
	_timerProc = timerProc;
}

void DJMSoundSystem::vblCallback() {
	if (_timerProc)
		_timerProc(_timerParam);
}

void DJMSoundSystem::generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const {
	assert(dst);
	memset(dst, 0, len);
	_sdrv->feed(dst, len, type, expectStereo);
}

const MacSoundDriver::Status &DJMSoundSystem::getDriverStatus(Audio::Mixer::SoundType type) const {
	return _sdrv->getStatus(type);
}

void DJMSoundSystem::dblBuffCallback(MacLowLevelPCMDriver::DoubleBuffer *dblBuffer) {
	uint16 sil = 0;
	memset(_mixBuffer16Bit, 0, _feedBufferSize * sizeof(uint16));
	byte b = _internal16Bit ? 0 : 0x80;

	for (int i = 0; i < _numChannels; ++i) {
		DJMSoundChannel &c = *_channels[i];
		if (c._mute ||
			// This is our "trick" for telling apart music and sfx: We look at the sound bank that
			// is currently in use, the musical one (1000 - 1127) or the sfx one (2000 - 2255).
			(dblBuffer->chanHandle == _musicChan && c._instr.get()->id() >= 2000) ||
			(dblBuffer->chanHandle == _sfxChan && c._instr.get()->id() < 2000)) {
				++sil;
				continue;
		}

		const byte *a = &_ampTable[(c._volume & ~3) << 6];
		int16 *t = _mixBuffer16Bit;

		for (int ii = 0; ii < _feedBufferSize; ++ii) {
			if (!c._pos) {
				*t++ += b;
				continue;
			}
			c._phase += c._frequency;
			if (c._phase >> 16) {
				c._pos += (c._phase >> 16);
				c._phase &= 0xFFFF;
				if (c._loopEnd <= c._pos) {
					c._pos = c._loopStart;
					c._phase = 0;
					if (!c._pos) {
						c._mute = 1;
						--ii;
						continue;
					}
				}
			}
			*t++ += _internal16Bit ? static_cast<int8>(a[*c._pos]) : a[*c._pos];
		}
	}

	const int16 *s = _mixBuffer16Bit;
	byte *d = dblBuffer->data;

	if (_internal16Bit) {
		memcpy(d, s, _feedBufferSize * sizeof(int16));
	} else {
		sil <<= 7;
		for (int i = 0; i < _feedBufferSize; ++i)
			*d++ = (sil + *s++) >> 3;
	}

	dblBuffer->numFrames = _feedBufferSize;
	dblBuffer->flags |= MacLowLevelPCMDriver::DoubleBuffer::kBufferReady;
}

void DJMSoundSystem::setMasterVolume(Audio::Mixer::SoundType type, uint16 volume) {
	if (_macstr)
		_macstr->setMasterVolume(type, volume);
}

void DJMSoundSystem::fillPitchTable() {
	static const float fltbl[12] = {
		1664510.62f, 1763487.62f, 1868350.00f, 1979447.88f, 2097152.00f, 2221855.25f,
		2353973.50f, 2493948.00f, 2642246.00f, 2799362.00f, 2965820.75f, 3142177.75f
	};
	for (int i = 0; i < 12; ++i)
		_pitchTable[116 + i] = (uint32)fltbl[i];
	for (int i = 115; i >= 0; --i)
		_pitchTable[i] = _pitchTable[i + 12] >> 1;
	if (_quality != 11)
		return;
	for (int i = 0; i < 128; ++i)
		_pitchTable[i] <<= 1; 
}

byte IMuseChannel_Mac_DJM::_allocCur = 0;

IMuseChannel_Mac_DJM::IMuseChannel_Mac_DJM(IMuseDriver_Mac_DJM *drv, int number) : MidiChannel(), _drv(drv), _number(number), _allocated(false), _sustain(false), _bank(0),
	_pitchBendEff(0), _prio(0x80), _detune(0), _transpose(0), _pitchBendSet(0), _pitchBendSensitivity(2), _volume(0), _channels(drv ? drv->_channels : nullptr), _prog(nullptr),
	_out(nullptr), _device(drv ? drv->_device : nullptr), _instruments(drv ? &drv->_instruments : nullptr), _numChannels(8) {
	assert(_drv);
	assert(_channels);
	assert(_device);
	assert(_instruments);
	_allocCur = 0;
}

IMuseChannel_Mac_DJM::~IMuseChannel_Mac_DJM() {
}

bool IMuseChannel_Mac_DJM::allocate() {
	if (_allocated)
		return false;

	return (_allocated = true);
}

void IMuseChannel_Mac_DJM::noteOff(byte note)  {
	for (ChannelNode *node = _out; node; node = node->_next) {
		if (node->_note == note) {
			if (_sustain) {
				node->_sustain = true;
			} else {
				debug(5, "NOTE OFF: chan '%d', note '%d'", node->_number, note);
				_device->noteOff(node->_number);
				disconnect(_out, node);
			}
		}
	}
}

void IMuseChannel_Mac_DJM::noteOn(byte note, byte velocity)  {
	ChannelNode *node = allocateDeviceChannel(_prio);
	if (node == nullptr || _prog == nullptr)
		return;
	connect(_out, node);
	node->_in = this;
	node->_note = note;
	node->_sustain = false;

	_device->setInstrument(node->_number, _prog);
	debug(4, "NOTE ON: chan '%d', note '%d', instr id '%d' ('%s')", node->_number, note, _prog.get()->id(), _prog.get()->name());
	_device->noteOn(node->_number, ((note + _transpose) << 7) + _pitchBendEff, _volume); // ignoring velocity
}

void IMuseChannel_Mac_DJM::controlChange(byte control, byte value)  {
	switch (control) {
	case 7:
		_volume = value;
		for (ChannelNode *node = _out; node; node = node->_next)
			_device->setVolume(node->_number, value);
		break;
	case 32:
		_bank = value; // The original code doesn't have that. It will just call a different programChange() method from the sysex handler. 
		break;
	case 64:
		_sustain = (value != 0);
		if (value == 0) {
			for (ChannelNode *node = _out; node; node = node->_next)  {
				if (node->_sustain) {
					_device->noteOff(node->_number);
					disconnect(_out, node);
				}
			}
		}
		break;
	default:
		break;
	}
}

void IMuseChannel_Mac_DJM::programChange(byte program)  {
	_prog.reset();
	uint16 base = _bank ? 2000 : 1000;
	Common::SharedPtr<MacSndInstrument> *def = nullptr;
	for (Common::Array<Common::SharedPtr<MacSndInstrument> >::iterator it = _instruments->begin(); _prog == nullptr && it != _instruments->end(); ++it) {
		uint16 id = (*it)->id();
		if (id == program + base)
			_prog = *it;
		else if (id == 999u)
			def = it;
	}
	if (_prog == nullptr) {
		if (def != nullptr)
			_prog = *def;
		else
			error("%s(): Failure (program %d)", __FUNCTION__, program);
	}
}

void IMuseChannel_Mac_DJM::pitchBend(int16 bend)  {
	_pitchBendSet = bend;
	//if (_newSystem) {
		// SAMNMAX formula (same for Roland MT-32 and GM)
		//bend = (((bend * _pitchBendSensitivity) >> 5) + _detune + (_transpose << 8)) << 1;
	//} else {
		// DOTT, INDY4 and MI2 formula
		bend = CLIP<int>(((bend * _pitchBendSensitivity) >> 6) + _detune, -2048, 2047);
	//}
		
	_pitchBendEff = bend;

	debug(4, "PITCH BEND: chan %d, eff pitch '%d'", _number, _pitchBendEff);
	for (ChannelNode *node = _out; node; node = node->_next)
		_device->setPitch(node->_number, ((node->_note + _transpose) << 7) + _pitchBendEff);
}

void IMuseChannel_Mac_DJM::sustain(bool value) {
	_sustain = value;
}

void IMuseChannel_Mac_DJM::allNotesOff() {
	for (ChannelNode *node = _out; node; node = node->_next) {
		_device->noteOff(node->_number);
		disconnect(_out, node);
	}
}

ChannelNode *IMuseChannel_Mac_DJM::allocateDeviceChannel(int prio) {
	ChannelNode *res = nullptr;
	for (byte i = 0; i < _numChannels; ++i) {
		_allocCur = (_allocCur + 1) % _numChannels;
		ChannelNode *node = _channels[_allocCur];
		if (node->_in == nullptr)
			return node;
		if (!node->_next && node->_in && node->_in->_prio <= prio) {
			res = node;
			prio = node->_in->_prio;
		}
	}

	if (res) {
		_device->noteOff(res->_number);
		disconnect(res->_in->_out, res);
	}
	return res;
}

} // End of namespace IMSMacintosh

namespace Scumm {
using namespace IMSMacintosh;

IMuseDriver_Mac_DJM::IMuseDriver_Mac_DJM(Audio::Mixer *mixer) : MidiDriver(), _isOpen(false), _device(nullptr), _imsParts(nullptr), _channels(nullptr),
	_numParts(24), _numChannels(8), _baseTempo(16667), _quality(1), _musicVolume(0), _sfxVolume(0) {
	_device = new DJMSoundSystem(mixer);
}

IMuseDriver_Mac_DJM::~IMuseDriver_Mac_DJM() {
	close();
	delete _device;
}

int IMuseDriver_Mac_DJM::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	createChannels();

	static const char *fileNames[] = {
		"iMUSE Setups",
		nullptr
	};

	if (!loadDefaultInstruments(fileNames, ARRAYSIZE(fileNames)) || !_device->init(false) || !_device->start())
		return MERR_DEVICE_NOT_AVAILABLE;

	_isOpen = true;

	return 0;
}

void IMuseDriver_Mac_DJM::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	_device->deinit();
	releaseChannels();
}

uint32 IMuseDriver_Mac_DJM::property(int prop, uint32 param) {
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

void IMuseDriver_Mac_DJM::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	if (_device)
		_device->setTimerCallback(timerParam, timerProc);
}

MidiChannel *IMuseDriver_Mac_DJM::allocateChannel() {
	if (!_isOpen)
		return nullptr;

	for (int i = 0; i < _numParts; ++i) {
		IMuseChannel_Mac_DJM *ch = _imsParts[i];
		if (ch && ch->allocate())
			return ch;
	}

	return nullptr;
}

MidiChannel *IMuseDriver_Mac_DJM::getPercussionChannel() {
	return nullptr;
}

void IMuseDriver_Mac_DJM::createChannels() {
	releaseChannels();

	_channels = new ChannelNode*[_numChannels];
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i) {
		_channels[i] = new ChannelNode(i);
		assert(_channels[i]);
	}

	_imsParts = new IMuseChannel_Mac_DJM*[_numParts];
	assert(_imsParts);
	for (int i = 0; i < _numParts; ++i)
		_imsParts[i] = new IMuseChannel_Mac_DJM(this, i);
}

void IMuseDriver_Mac_DJM::releaseChannels() {
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

bool IMuseDriver_Mac_DJM::loadDefaultInstruments(const char *const *tryFileNames, uint16 numTryFileNames) {
	const Common::CodePage tryCodePages[] = {
		Common::kMacRoman,
		Common::kISO8859_1
	};

	Common::MacResManager resMan;
	Common::Path resFile;

	for (int i = 0; resFile.empty() && i < numTryFileNames && tryFileNames[i] != nullptr; ++i) {
		for (int ii = 0; resFile.empty() && ii < ARRAYSIZE(tryCodePages); ++ii) {
			Common::U32String fn(tryFileNames[i], tryCodePages[ii]);
			resFile = Common::Path(fn.encode(Common::kUtf8));
			if (!resMan.exists(resFile) || !resMan.open(resFile) || !resMan.hasResFork()) {
				resMan.close();
				resFile = Common::Path(Common::punycode_encodefilename(fn));
				if (!resMan.exists(resFile) || !resMan.open(resFile) || !resMan.hasResFork()) {
					resMan.close();
					resFile.clear();
				}
			}
		}
	}

	if (resFile.empty()) {
		warning("%s(): Resource fork not found", __FUNCTION__);
		return false;
	}

	_instruments.clear();

	Common::MacResIDArray ids = resMan.getResIDArray(MKTAG('s', 'n', 'd', ' '));
	for (Common::MacResIDArray::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		Common::SeekableReadStream *str = resMan.getResource(MKTAG('s', 'n', 'd', ' '), *i);
		uint16 type = str ? str->readUint16BE() : 0;
		if (type == 1 || type == 2)
			_instruments.push_back(Common::SharedPtr<MacSndInstrument>(new MacSndInstrument(*i, str, resMan.getResName(MKTAG('s', 'n', 'd', ' '), *i))));
		delete str;
	}

	return !_instruments.empty();
}

} // End of namespace Scumm
