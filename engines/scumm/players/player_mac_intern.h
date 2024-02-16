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

#ifndef SCUMM_PLAYERS_PLAYER_MAC_INTERN_H
#define SCUMM_PLAYERS_PLAYER_MAC_INTERN_H

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Scumm {

class MacSoundDriver {
public:
	MacSoundDriver(Common::Mutex &mutex, uint32 deviceRate, int activeChannels, bool canInterpolate, bool internal16Bit) : _mutex(mutex), _status(deviceRate, canInterpolate, activeChannels),
		_smpSize(internal16Bit ? 2 : 1), _smpMin(internal16Bit ? -32768 : -128), _smpMax(internal16Bit ? 32767 : 127) {}
	virtual ~MacSoundDriver() {}
	virtual void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) = 0;

	struct Status {
		Status(uint32 rate, bool interp, int actChan) :deviceRate(rate), allowInterPolation(interp), numExternalMixChannels(actChan), flags(0) {}
		uint32 deviceRate;
		int numExternalMixChannels;
		bool allowInterPolation;
		uint8 flags;
	};
	const Status &getStatus() const { return _status; }

	enum StatusFlag : uint8 {
		kStatusPlaying =		1	<<		0,
		kStatusOverflow =		1	<<		1,
		kStatusStartup =		1	<<		2,
		kStatusDone =			1	<<		3
	};

	void clearFlags(uint8 flags) { _status.flags &= ~flags; }

protected:
	void setFlags(uint8 flags) { _status.flags |= flags; }

	Common::Mutex &_mutex;
	const int _smpSize;
	const int16 _smpMin;
	const int16 _smpMax;
	Status _status;
};

class MacLowLevelPCMDriver final : public MacSoundDriver {
public:
	struct PCMSound {
		PCMSound() : len(0), rate(0), loopst(0), loopend(0), baseFreq(0), stereo(false) {}
		Common::SharedPtr<const byte> data;
		uint32 len;
		uint32 rate;
		uint32 loopst;
		uint32 loopend;
		byte baseFreq;
		bool stereo;
	};

	enum SynthType {
		kSquareWaveSynth = 1,
		kWaveTableSynth = 3,
		kSampledSynth = 5
	};

	typedef int ChanHandle;
public:
	MacLowLevelPCMDriver(Common::Mutex &mutex, uint32 deviceRate, bool internal16Bit);
	~MacLowLevelPCMDriver() override;

	void feed(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) override;

	ChanHandle createChannel(Audio::Mixer::SoundType sndType, SynthType synthType, bool interpolate);
	void disposeChannel(ChanHandle handle);

	void playSamples(ChanHandle handle, PCMSound *snd);
	void stop(ChanHandle handle);

	uint8 getChannelStatus(ChanHandle handle) const { Channel *ch = findChannel(handle); return ch ? ch->_flags : 0; }
private:
	uint32 calcRate(uint32 outRate, uint32 factor, uint32 dataRate);
	void updateStatus();

private:
	class Channel {
	public:
		Channel(Audio::Mixer::SoundType sndtp, int synth, bool interp);
		ChanHandle getHandle() const;
		void stop();

		const Audio::Mixer::SoundType _sndType;
		const int _synth;
		const bool _interpolate;

		Common::SharedPtr<const int8> _res;
		const int8 *_data;
		int8 _lastSmp[2];
		uint32 _len;
		uint16 _rmH;
		uint16 _rmL;
		uint32 _loopSt;
		uint32 _loopEnd;
		byte _baseFreq;
		uint32 _rcPos;
		uint32 _smpWtAcc;
		uint16 _frameSize;
		uint8 _flags;
	};

	void setChanFlags(Channel *ch, uint8 flags) { if (ch) ch->_flags |= flags; }
	void clearChanFlags(Channel *ch, uint8 flags) { if (ch) ch->_flags &= ~flags; }
	Channel *findChannel(ChanHandle h) const;
	Common::Array<Channel*> _channels;
	int _numInternalMixChannels;
	int32 *_mixBuffer = 0;
	uint32 _mixBufferSize;
};

class VblTaskClientDriver {
public:
	virtual void callback() = 0;
	virtual void generateData(int8 *dst, uint32 len, Audio::Mixer::SoundType type, bool expectStereo) const = 0;
	virtual const MacSoundDriver::Status &getDriverStatus(uint8 drvID) const = 0;
};

class MacPlayerAudioStream : public Audio::AudioStream {
public:
	MacPlayerAudioStream(VblTaskClientDriver *drv, uint32 scummVMOutputrate, bool stereo, bool interpolate, bool internal16Bit);
	~MacPlayerAudioStream() override;

	void initBuffers(uint32 feedBufferSize);
	void initDrivers();
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
	const bool _interp;
	const int _smpInternalSize;

	const bool _isStereo;
};

class LegacyMusicDriver;
class Indy3MacSnd final : public VblTaskClientDriver {
private:
	Indy3MacSnd(ScummEngine *vm, Audio::Mixer *mixer);
public:
	~Indy3MacSnd();
	static Common::SharedPtr<Indy3MacSnd> open(ScummEngine *scumm, Audio::Mixer *mixer);
	bool startDevices(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation, bool stereo, bool internal16Bit);

	void setMusicVolume(int vol);
	void setSfxVolume(int vol);
	void startSound(int id);
	void stopSound(int id);
	void stopAllSounds();
	int getMusicTimer();
	int getSoundStatus(int id) const;
	void setQuality(int qual);
	void saveLoadWithSerializer(Common::Serializer &ser);
	void restoreAfterLoad();

	void nextTick();
	void callback() override { nextTick(); }
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const override;
	const MacSoundDriver::Status &getDriverStatus(uint8 drvID) const override;

private:
	void startSong(int id);
	void startSoundEffect(int id);
	void stopSong();
	void stopSoundEffect();
	void stopActiveSound();
	void finishSong();
	void updateSong();
	void updateSoundEffect();

	void checkRestartSoundEffects();
	void endOfTrack();

	bool isSong(int id) const;
	bool isHiQuality() const;

	int _curSound;
	int _curSong;
	int _lastSoundEffectPrio;
	int _soundEffectNumLoops;
	int _songTimer;
	bool _songUnfinished;
	uint _activeChanCount;
	byte _songTimerInternal;
	byte *_soundUsage;

	bool _soundEffectPlaying;
	int _qmode;
	bool _16bit;
	bool _qualHi;
	bool _mixerThread;

	MacLowLevelPCMDriver::PCMSound _pcmSnd;
	MacLowLevelPCMDriver::ChanHandle _handle;

	MacPlayerAudioStream *_macstr;
	Audio::SoundHandle _soundHandle;
	MacPlayerAudioStream::CallbackProc _nextTickProc;

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	static Common::WeakPtr<Indy3MacSnd> *_inst;

	const byte *_musicIDTable;
	int _musicIDTableLen;
	const int _idRangeMax;

	LegacyMusicDriver *_mdrv;
	MacLowLevelPCMDriver *_sdrv;
	Common::Array<MacSoundDriver*> _drivers;

private:
	class MusicChannel {
	public:
		MusicChannel(Indy3MacSnd *pl);
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
		uint16 _envSust;
		int16 _transpose;
		uint16 _envAtt;
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
		typedef bool (Indy3MacSnd::MusicChannel::*CtrlProc)(const byte *&);

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

		Indy3MacSnd *_player;
		static MusicChannel *_ctrlChan;

		static const uint32 _envShapes[98];
		const uint8 *&_modShapes;
		const uint32 &_modShapesTableSize;

		bool ctrlProc(int procId, const byte *&arg);
		void setFrameLen(uint8 len);
	};

	MusicChannel **_musicChannels;
	const int _numMusicChannels;
	const int _numMusicTracks;

	static const uint8 _fourToneSynthWaveForm[256];

public:
	MusicChannel *getMusicChannel(uint8 id) const;
};

} // End of namespace Scumm

#endif
