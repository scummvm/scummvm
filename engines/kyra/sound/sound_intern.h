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

#ifndef KYRA_SOUND_INTERN_H
#define KYRA_SOUND_INTERN_H


#include "kyra/sound/sound.h"
#include "kyra/sound/sound_pc_v1.h"

#include "audio/midiparser.h"
#include "audio/miles.h"
#include "audio/softsynth/emumidi.h"
#include "audio/softsynth/fmtowns_pc98/towns_audio.h"

#include "common/mutex.h"

class EuphonyPlayer;
class TownsPC98_AudioDriver;

namespace Audio {
class PCSpeaker;
class MaxTrax;
} // End of namespace Audio

namespace Kyra {

class MidiOutput;

/**
 * MIDI output device.
 *
 * This device supports both MT-32 MIDI, as used in
 * Kyrandia 1 and 2, and GM MIDI, as used in Kyrandia 2.
 */
class SoundMidiPC : public Sound {
public:
	SoundMidiPC(KyraEngine_v1 *vm, Audio::Mixer *mixer, MidiDriver *driver, kType type);
	~SoundMidiPC() override;

	kType getMusicType() const override { return _type; }

	bool init() override;

	void updateVolumeSettings() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String file) override;
	void loadSfxFile(Common::String file) override;

	void playTrack(uint8 track) override;
	void haltTrack() override;
	bool isPlaying() const override;

	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;
	void stopAllSoundEffects() override;

	void beginFadeOut() override;

	void pause(bool paused) override;
private:
	static void onTimer(void *data);

	// Our channel handling
	int _musicVolume, _sfxVolume;

	uint32 _fadeStartTime;
	bool _fadeMusicOut;

	// Midi file related
	Common::String _mFileName, _sFileName;
	byte *_musicFile, *_sfxFile;

	MidiParser *_music;
	MidiParser *_sfx[3];

	const SoundResourceInfo_PC *res() const {return _resInfo[_currentResourceSet]; }
	SoundResourceInfo_PC *_resInfo[3];
	int _currentResourceSet;

	// misc
	kType _type;
	Common::String getFileName(const Common::String &str);

	bool _nativeMT32;
	MidiDriver *_driver;
	Audio::MidiDriver_Miles_Midi *_output;

	Common::Mutex _mutex;
};

class SoundTowns_LoK : public Sound {
public:
	SoundTowns_LoK(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundTowns_LoK() override;

	kType getMusicType() const override { return kTowns; }

	bool init() override;
	void process() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String) override {}

	void playTrack(uint8 track) override;
	void haltTrack() override;

	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;
	void stopAllSoundEffects() override;

	void beginFadeOut() override;

	void updateVolumeSettings() override;

private:
	bool loadInstruments();
	void playEuphonyTrack(uint32 offset, int loop);

	void fadeOutSoundEffects();

	int _lastTrack;
	Audio::SoundHandle _sfxHandle;

	uint8 *_musicTrackData;

	uint _sfxFileIndex;
	uint8 *_sfxFileData;
	uint8 _sfxChannel;

	EuphonyPlayer *_player;

	bool _cdaPlaying;

	const SoundResourceInfo_Towns *res() const {return _resInfo[_currentResourceSet]; }
	SoundResourceInfo_Towns *_resInfo[3];
	int _currentResourceSet;

	const uint8 *_musicFadeTable;
	const uint8 *_sfxBTTable;
	const uint8 *_sfxWDTable;
};

class SoundPC98_LoK : public Sound {
public:
	SoundPC98_LoK(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundPC98_LoK() override;

	kType getMusicType() const override { return kPC98; }

	bool init() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String file) override;

	void playTrack(uint8 track) override;
	void haltTrack() override;
	void beginFadeOut() override;

	int32 voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool isSfx) override { return -1; }
	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;

	void updateVolumeSettings() override;

private:
	int _lastTrack;
	uint8 *_musicTrackData;
	uint8 *_sfxTrackData;
	TownsPC98_AudioDriver *_driver;

	const char *resPattern() {return _resInfo[_currentResourceSet]->c_str(); }
	Common::String *_resInfo[3];
	int _currentResourceSet;
};

class SoundTownsPC98_v2 : public Sound {
public:
	SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundTownsPC98_v2() override;

	kType getMusicType() const override { return _vm->gameFlags().platform == Common::kPlatformFMTowns ? kTowns : kPC98; }

	bool init() override;
	void process() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override {}
	void loadSoundFile(Common::String file) override;

	void playTrack(uint8 track) override;
	void haltTrack() override;
	void beginFadeOut() override;

	int32 voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume = 255, uint8 priority = 255, bool isSfx = true) override;
	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;

	void updateVolumeSettings() override;

private:
	Audio::AudioStream *_currentSFX;
	int _lastTrack;
	bool _useFmSfx;

	uint8 *_musicTrackData;
	uint8 *_sfxTrackData;
	TownsPC98_AudioDriver *_driver;

	const SoundResourceInfo_TownsPC98V2 *res() const {return _resInfo[_currentResourceSet]; }
	SoundResourceInfo_TownsPC98V2 *_resInfo[3];
	int _currentResourceSet;
};

// PC Speaker MIDI driver
class MidiDriver_PCSpeaker : public MidiDriver_Emulated {
public:
	MidiDriver_PCSpeaker(Audio::Mixer *mixer);
	~MidiDriver_PCSpeaker() override;

	// MidiDriver interface
	void close() override {}

	void send(uint32 data) override;

	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }

	// MidiDriver_Emulated interface
	void generateSamples(int16 *buffer, int numSamples) override;

	// AudioStream interface
	bool isStereo() const override { return false; }
	int getRate() const override { return _rate; }
private:
	Common::Mutex _mutex;
	Audio::PCSpeaker *_speaker;
	int _rate;

	struct Channel {
		uint8 pitchBendLow, pitchBendHigh;
		uint8 hold;
		uint8 modulation;
		uint8 voiceProtect;
		uint8 noteCount;
	} _channel[2];

	void resetController(int channel);

	struct Note {
		bool enabled;
		uint8 hardwareChannel;
		uint8 midiChannel;
		uint8 note;
		bool processHold;
		uint8 flags;
		uint8 hardwareFlags;
		uint16 priority;
		int16 modulation;
		uint16 precedence;
	} _note[2];

	void noteOn(int channel, int note);
	void noteOff(int channel, int note);

	void turnNoteOn(int note);
	void overwriteNote(int note);
	void turnNoteOff(int note);

	void setupTone(int note);

	uint16 _countdown;
	uint8 _hardwareChannel[1];
	bool _modulationFlag;

	uint8 _timerValue;
	void onTimer() override;

	static const uint8 _noteTable1[];
	static const uint8 _noteTable2[];
};

// for StaticResource (maybe we can find a nicer way to handle it)
struct AmigaSfxTable {
	uint8 note;
	uint8 patch;
	uint16 duration;
	uint8 volume;
	uint8 pan;
};

class SoundAmiga_LoK : public Sound {
public:
	SoundAmiga_LoK(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundAmiga_LoK() override;

	kType getMusicType() const override { return kAmiga; } //FIXME

	bool init() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String) override {}

	void playTrack(uint8 track) override;
	void haltTrack() override;
	void beginFadeOut() override;

	int32 voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool isSfx) override { return -1; }
	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;

protected:
	Audio::MaxTrax *_driver;
	Audio::SoundHandle _musicHandle;
	enum FileType { kFileNone = -1, kFileIntro = 0, kFileGame = 1, kFileFinal = 2 } _fileLoaded;

	const AmigaSfxTable *_tableSfxIntro;
	int _tableSfxIntro_Size;

	const AmigaSfxTable *_tableSfxGame;
	int _tableSfxGame_Size;
};

#ifdef ENABLE_EOB

class SoundTowns_Darkmoon : public Sound, public TownsAudioInterfacePluginDriver {
public:
	SoundTowns_Darkmoon(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundTowns_Darkmoon() override;

	kType getMusicType() const override { return kTowns; }

	bool init() override;

	void timerCallback(int timerId) override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String name) override;

	void playTrack(uint8 track) override;
	void haltTrack() override;
	bool isPlaying() const override;

	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;
	void stopAllSoundEffects() override;

	void beginFadeOut() override;

	void updateVolumeSettings() override;

	int checkTrigger() override;

	void resetTrigger() override;

private:
	struct SoundTableEntry {
		int8 type;
		int32 para1;
		int16 para2;
	} _soundTable[120];

	const char *const *_fileList;
	uint _fileListLen;

	uint8 _lastSfxChan;
	uint8 _lastEnvChan;
	uint8 *_pcmData;
	uint32 _pcmDataSize;
	uint8 _pcmVol;

	int _timer;
	int _timerSwitch;

	SoundResourceInfo_TownsEoB *_resource[3];

	TownsAudioInterface *_intf;
};

class AudioMaster2;
class SoundAmiga_EoB: public Sound {
public:
	SoundAmiga_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundAmiga_EoB() override;

	kType getMusicType() const override;

	bool init() override;
	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override { return false; }
	void loadSoundFile(uint) override {}
	void loadSoundFile(Common::String file) override;
	void unloadSoundFile(Common::String file) override;
	void playTrack(uint8 track) override;
	void haltTrack() override;
	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;
	void beginFadeOut() override { beginFadeOut(160); }
	void beginFadeOut(int delay) override;
	void updateVolumeSettings() override;
	int checkTrigger() override;

private:
	uint8 *_fileBuffer;

	KyraEngine_v1 *_vm;
	AudioMaster2 *_driver;
	SoundResourceInfo_AmigaEoB *_resInfo[3];
	Common::String _lastSound;

	int _currentResourceSet;

	bool _ready;
};

class MLALF98;
class SoundPC98_EoB : public Sound {
public:
	SoundPC98_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundPC98_EoB() override;

	kType getMusicType() const override;

	bool init() override;
	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override { return false; }
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String file) override {}
	void loadSfxFile(Common::String file) override;
	void playTrack(uint8 track) override;
	void haltTrack() override;
	void playSoundEffect(uint16 track, uint8) override;
	void beginFadeOut() override {}
	void updateVolumeSettings() override;

private:
	KyraEngine_v1 *_vm;
	MLALF98 *_driver;

	SoundResourceInfo_PC *_resInfo[3];
	int _currentResourceSet;

	uint32 _sfxDelay;

	bool _ready;
};

class SegaAudioDriver;
class SoundSegaCD_EoB : public Sound {
public:
	SoundSegaCD_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundSegaCD_EoB() override;

	kType getMusicType() const override;

	bool init() override;
	void initAudioResourceInfo(int, void*) override {}
	void selectAudioResourceSet(int) override {}
	bool hasSoundFile(uint file) const override { return false; }
	void loadSoundFile(uint file) override {}
	void loadSoundFile(Common::String file) override {}
	void playTrack(uint8 track) override;
	void haltTrack() override;
	void playSoundEffect(uint16 track, uint8 volume) override;
	bool isPlaying() const override;
	void beginFadeOut() override {}
	void updateVolumeSettings() override;

private:
	void loadPCMData();
	void loadFMData();

	KyraEngine_v1 *_vm;
	SegaAudioDriver *_driver;

	uint8 _pcmOffsets[8];
	uint16 _fmOffsets[140];
	const uint8 *_fmData;
	int _lastSoundEffect;
	bool _ready;

	static const uint8 _fmTrackMap[140];
};

#endif

} // End of namespace Kyra

#endif
