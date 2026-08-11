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

#include "engines/scumm/players/mac_sound_lowlevel.h"
#include "common/serializer.h"

namespace Scumm {

class LegacyMusicDriver;
class ScummEngine;
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
	void toggleMusic(bool enable);
	void toggleSoundEffects(bool enable);

	void vblCallback() override;
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const override;
	const MacSoundDriver::Status &getDriverStatus(Audio::Mixer::SoundType sndType) const override;

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
	byte _disableFlags;

	bool _soundEffectPlaying;
	bool _soundEffectReschedule;
	int _qmode;
	bool _16bit;
	bool _qualHi;
	bool _mixerThread;

	MacLowLevelPCMDriver::PCMSound _pcmSnd;
	MacLowLevelPCMDriver::ChanHandle _sfxChan;

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
		const uint8 *const &_modShapes;
		const uint32 &_modShapesTableSize;

		bool ctrlProc(int procId, const byte *&arg);
		void setFrameLen(uint8 len);
	};

	MusicChannel **_musicChannels;
	const int _numMusicChannels;
	const int _numMusicTracks;

public:
	MusicChannel *getMusicChannel(uint8 id) const;
};

class MacSndLoader;
class LoomMonkeyMacSnd final : public VblTaskClientDriver, public MacLowLevelPCMDriver::CallbackClient {
private:
	LoomMonkeyMacSnd(ScummEngine *vm, Audio::Mixer *mixer);
public:
	~LoomMonkeyMacSnd();
	static Common::SharedPtr<LoomMonkeyMacSnd> open(ScummEngine *scumm, Audio::Mixer *mixer);
	bool startDevice(uint32 outputRate, uint32 pcmDeviceRate, uint32 feedBufferSize, bool enableInterpolation, bool stereo, bool internal16Bit);

	void setMusicVolume(int vol);
	void setSfxVolume(int vol);
	void startSound(int id, int jumpToTick = 0);
	void stopSound(int id);
	void stopAllSounds();
	int getMusicTimer();
	int getSoundStatus(int id) const;
	void setQuality(int qual);
	void saveLoadWithSerializer(Common::Serializer &ser);
	void restoreAfterLoad();
	void toggleMusic(bool enable);
	void toggleSoundEffects(bool enable);

	void vblCallback() override;
	void generateData(int8 *dst, uint32 byteSize, Audio::Mixer::SoundType type, bool expectStereo) const override;
	const MacSoundDriver::Status &getDriverStatus(Audio::Mixer::SoundType sndType) const override;

	void sndChannelCallback(uint16 arg1, const void *arg2) override;

private:
	void sendSoundCommands(int timeStamp);
	void stopActiveSound();
	void setupChannels();
	void disposeAllChannels();
	void updateDisabledState();

	void detectQuality();
	bool isSoundCardType10() const;

	int _curSound;
	int _restartSound;
	int _curSoundSaveVar;
	int &_checkSound;
	int _songTimer;
	byte _songTimerInternal;
	byte *_chanConfigTable;
	const int _idRangeMax;
	const byte _saveVersionChange;
	const byte _legacySaveUnits;
	bool _mixerThread;

	int _machineRating;
	int _selectedQuality;
	int _effectiveChanConfig;
	int _defaultChanConfig;
	bool _16bit;
	byte _disableFlags;

	MacLowLevelPCMDriver::ChanHandle _sndChannel;
	MacLowLevelPCMDriver::ChanHandle _musChannels[4];

	MacPlayerAudioStream *_macstr;
	MacSndLoader *_loader;
	MacLowLevelPCMDriver *_sdrv;
	Audio::SoundHandle _soundHandle;
	MacPlayerAudioStream::CallbackProc _vblTskProc;
	MacLowLevelPCMDriver::ChanCallback _chanCbProc;

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	static Common::WeakPtr<LoomMonkeyMacSnd> *_inst;

	byte _curChanConfig;
	byte _chanUse;
	byte _curSynthType;
	Audio::Mixer::SoundType _curSndType;
	Audio::Mixer::SoundType _lastSndType;
	byte _chanPlaying;
};

} // End of namespace Scumm

#endif
