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

#ifndef SCUMM_PLAYERS_MAC_SOUND_LOWLEVEL_H
#define SCUMM_PLAYERS_MAC_SOUND_LOWLEVEL_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/func.h"

namespace Scumm {

class MacSoundDriver {
public:
	MacSoundDriver(Common::Mutex &mutex, uint32 deviceRate, int activeChannels, bool canInterpolate, bool internal16Bit) : _mutex(mutex),
		_smpSize(internal16Bit ? 2 : 1), _smpMin(internal16Bit ? -32768 : -128), _smpMax(internal16Bit ? 32767 : 127) {
		for (int i = 0; i < 4; ++i) {
			_status[i].deviceRate = deviceRate;
			_status[i].numExternalMixChannels = activeChannels;
			_status[i].allowInterPolation = canInterpolate;
			_status[i].flags = 0;
		}
	}
	virtual ~MacSoundDriver() {}
	virtual void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) = 0;

	struct Status {
		Status() : deviceRate(0), numExternalMixChannels(0), allowInterPolation(false), flags(0) {}
		uint32 deviceRate;
		int numExternalMixChannels;
		bool allowInterPolation;
		uint8 flags;
	};
	const Status &getStatus(Audio::Mixer::SoundType sndType = Audio::Mixer::kPlainSoundType) const { return _status[sndType]; }

	enum StatusFlag : uint8 {
		kStatusPlaying =		1	<<		0,
		kStatusOverflow =		1	<<		1,
		kStatusStartup =		1	<<		2,
		kStatusDone =			1	<<		3,
		kStatusDisabled =		1	<<		7
	};

	void clearFlags(uint8 flags, Audio::Mixer::SoundType sndType = Audio::Mixer::kPlainSoundType) { _status[sndType].flags &= ~flags; }
	void setFlags(uint8 flags, Audio::Mixer::SoundType sndType = Audio::Mixer::kPlainSoundType) { _status[sndType].flags |= flags; }

protected:
	Common::Mutex &_mutex;
	const int _smpSize;
	const int16 _smpMin;
	const int16 _smpMax;
	Status _status[4];
};

class MacSndChannel;
class MacLowLevelPCMDriver final : public MacSoundDriver {
public:
	struct PCMSound {
		PCMSound() : len(0), rate(0), loopst(0), loopend(0), baseFreq(0), stereo(false), enc(0) {}
		PCMSound(Common::SharedPtr<const byte> a, uint32 b, uint32 c, uint32 d, uint32 e, byte f, byte g, bool h) : data(a), len(b), rate(c), loopst(d), loopend(e), baseFreq(f), enc(g), stereo(h) {}
		Common::SharedPtr<const byte> data;
		uint32 len;
		uint32 rate;
		uint32 loopst;
		uint32 loopend;
		byte baseFreq;
		byte enc;
		bool stereo;
	};

	enum SynthType : byte {
		kSquareWaveSynth = 1,
		kWaveTableSynth = 3,
		kSampledSynth = 5,
		kIgnoreSynth = 0xff
	};

	enum ChanAttrib : byte {
		kInitChanLeft = 2,
		kInitChanRight = 3,
		kWaveInitChannel0 = 4,
		kWaveInitChannel1 = 5,
		kWaveInitChannel2 = 6,
		kWaveInitChannel3 = 7,
		kNoInterp = 4,
		kInitNoDrop = 8,
		kInitMono = 0x80,
		kInitStereo = 0xC0
	};

	enum ExecMode : byte {
		kImmediate,
		kEnqueue,
	};

	typedef int ChanHandle;

	struct DoubleBuffer {
		enum Flags : uint32 {
			kBufferReady = 1,
			kLastBufferLast = 4
		};
		DoubleBuffer(ChanHandle hdl, uint32 numframes) : numFrames(numframes), flags(0), data(0), chanHandle(hdl) {}
		~DoubleBuffer() { delete[] data; }
		uint32 numFrames;
		const ChanHandle chanHandle;
		uint32 flags;
		byte *data;
	};

	class CallbackClient {
	public:
		virtual ~CallbackClient() {}
		virtual void sndChannelCallback(uint16 arg1, const void *arg2) {}
		virtual void dblBuffCallback(DoubleBuffer *dblBuffer) {}
	};
	typedef Common::Functor2Mem<uint16, const void*, void, CallbackClient> ChanCallback;
	typedef Common::Functor1Mem<DoubleBuffer*, void, CallbackClient> DBCallback;

public:
	MacLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool internal16Bit);
	~MacLowLevelPCMDriver() override;

	void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) override;

	ChanHandle createChannel(Audio::Mixer::SoundType sndType, SynthType synthType, byte attributes, ChanCallback *callback);
	void disposeChannel(ChanHandle handle);

	void playSamples(ChanHandle handle, ExecMode mode, const PCMSound *snd);
	void playNote(ChanHandle handle, ExecMode mode, uint8 note, uint16 duration);
	void quiet(ChanHandle handle, ExecMode mode);
	void flush(ChanHandle handle, ExecMode mode);
	void wait(ChanHandle handle, ExecMode mode, uint16 duration);
	void loadWaveTable(ChanHandle handle, ExecMode mode, const byte *data, uint16 dataSize);
	void loadInstrument(ChanHandle handle, ExecMode mode, const PCMSound *snd);
	void setTimbre(ChanHandle handle, ExecMode mode, uint16 timbre);
	void callback(ChanHandle handle, ExecMode mode, uint16 arg1, const void *arg2);

	bool playDoubleBuffer(ChanHandle handle, byte numChan, byte bitsPerSample, uint32 rate, DBCallback *callback, byte numMixChan = 1);

	uint8 getChannelStatus(ChanHandle handle) const;
	void clearChannelFlags(ChanHandle handle, uint8 flags);

	static uint32 calcRate(uint32 outRate, uint32 factor, uint32 dataRate);

private:
	void updateStatus(Audio::Mixer::SoundType sndType);
	MacSndChannel *findAndCheckChannel(ChanHandle h, const char *caller, byte reqSynthType) const;
	MacSndChannel *findChannel(ChanHandle h) const;
	Common::Array<MacSndChannel*> _channels;
	int _numInternalMixChannels[4];
	int32 *_mixBuffer = 0;
	uint32 _mixBufferSize;
};

class VblTaskClientDriver {
public:
	virtual ~VblTaskClientDriver() {}
	virtual void vblCallback() = 0;
	virtual void generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const = 0;
	virtual const MacSoundDriver::Status &getDriverStatus(Audio::Mixer::SoundType sndType) const = 0;
};

class MacPlayerAudioStream : public Audio::AudioStream {
public:
	MacPlayerAudioStream(VblTaskClientDriver *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate, bool internal16Bit);
	~MacPlayerAudioStream() override;

	void initBuffers(uint32 feedBufferSize);
	void initDrivers();
	void addVolumeGroup(Audio::Mixer::SoundType type);
	void scaleVolume(uint upscale, uint downscale) { _upscale = upscale; _downscale = downscale; }
	typedef Common::Functor0Mem<void, VblTaskClientDriver> CallbackProc;
	void setVblCallback(const CallbackProc *proc);
	void clearBuffer();

	void setMasterVolume(Audio::Mixer::SoundType type, uint16 vol);

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return _isStereo; }
	int getRate() const override { return _outputRate; }
	bool endOfData() const override { return false; }

private:
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType, bool expectStereo) const;
	void runVblTask();

	VblTaskClientDriver *_drv;
	int _numGroups;
	uint16 _upscale;
	uint16 _downscale;

	uint32 _vblSmpQty;
	uint32 _vblSmpQtyRem;
	uint32 _vblCountDown;
	uint32 _vblCountDownRem;
	const CallbackProc *_vblCbProc;

	struct SmpBuffer {
		SmpBuffer() : start(0), pos(0), end(0), volume(0), lastL(0), lastR(0), size(0), rateConvInt(0), rateConvFrac(0), rateConvAcc(-1), group(Audio::Mixer::kPlainSoundType) {}
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
		Audio::Mixer::SoundType group;
	} *_buffers;

	const uint32 _outputRate;
	const uint8 _frameSize;
	const bool _interp;
	const int _smpInternalSize;

	const bool _isStereo;
};

struct MacSndResource {
public:
	// Construct from Mac resource stream
	MacSndResource(uint32 id, Common::SeekableReadStream *&in, Common::String &&name);
	// Construct from Mac sound data buffer
	MacSndResource(uint32 id, const byte *in, uint32 size);
	~MacSndResource() {}
	const MacLowLevelPCMDriver::PCMSound *data() const { return &_snd; }
	uint32 id() const { return _id; }
	const char* name() { return _name.c_str(); }

private:
	uint32 _id;
	Common::String _name;
	MacLowLevelPCMDriver::PCMSound _snd;
};

extern const uint8 _fourToneSynthWaveForm[256];
extern const uint32 _fourToneSynthWaveFormSize;

} // End of namespace Scumm

#endif
