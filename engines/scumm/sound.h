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

#ifndef SCUMM_SOUND_H
#define SCUMM_SOUND_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/str.h"
#include "audio/mididrv.h"
#include "scumm/file.h"
#include "scumm/soundcd.h"
#include "scumm/soundse.h"

#define DIGI_SND_MODE_EMPTY  0
#define DIGI_SND_MODE_SFX    1
#define DIGI_SND_MODE_TALKIE 2

namespace Common {
class SeekableSubReadStream;
}

namespace Audio {
class Mixer;
class SoundHandle;
class SeekableAudioStream;
}

namespace Scumm {

class ScummEngine;

struct MP3OffsetTable;

enum {
	kTalkSoundID = 10000
};

// TODO: Consider splitting Sound into even more subclasses.
// E.g. for v1-v4, v5, v6+, ...
class Sound : public Common::Serializable {
public:
	enum SoundMode {
		kVOCMode,
		kMP3Mode,
		kVorbisMode,
		kFLACMode
	};

protected:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;

	int16 _midiQueuePos, _midiQueue[0x100];
	int16 _soundQueuePos;

	struct {
		int16 sound;
		int32 offset;
		int16 channel;
		int16 flags;
		int16 freq;
		int16 pan;
		int16 vol;
	} _soundQueue[10];

	Common::String _sfxFilename;
	byte _sfxFileEncByte;
	SoundMode _soundMode;
	MP3OffsetTable *_offsetTable;	// For compressed audio
	int _numSoundEffects;		// For compressed audio
	int64 _cachedSfxLocationInPak = -1;	// For sfx files in pak files
	int32 _cachedSfxLengthInPak = 0;    // For sfx files in pak files

	uint32 _queuedSfxOffset, _queuedTalkieOffset, _queuedSfxLen, _queuedTalkieLen;
	byte _queuedSoundMode, _queuedSfxChannel;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[64];
	uint _curSoundPos;

	int16 _currentMusic;	// used by HE games

	SoundCD *_soundCD = nullptr;
	SoundSE *_soundSE = nullptr;
	bool _useRemasteredAudio = false;

	int32 _currentMISpeechIndex = -1;

public:
	Audio::SoundHandle *_talkChannelHandle;	// Handle of mixer channel actor is talking on

	bool _soundsPaused;
	byte _digiSndMode;
	uint _lastSound;
	uint32 _speechTimerMod;

	MidiDriverFlags _musicType;

public:
	Sound(ScummEngine *parent, Audio::Mixer *mixer, bool useReplacementAudioTracks);
	~Sound() override;
	virtual void startSound(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0);
	virtual void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0);
	void processSound();
	virtual void modifySound(int sound, int offset, int frequencyShift, int pan, int volume, int flags) {};

	void triggerSound(int soundID);
	void startTalkSound(uint32 offset, uint32 b, int mode, Audio::SoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	virtual int isSoundRunning(int sound) const;
	virtual bool isSoundInUse(int sound) const;
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 offset, uint32 length, int mode, int channel = 0);
	virtual void setupSound();
	virtual void pauseSounds(bool pause);
	bool isSfxFileCompressed();
	bool hasSfxFile() const;
	ScummFile *restoreDiMUSESpeechFile(const char *fileName);
	void extractSyncsFromDiMUSEMarker(const char *marker);
	void incrementSpeechTimer();
	void resetSpeechTimer();
	void startSpeechTimer();
	void stopSpeechTimer();
	bool speechIsPlaying(); // Used within MIDI iMUSE

	void saveLoadWithSerializer(Common::Serializer &ser) override;
	void restoreAfterLoad();

	bool isAudioDisabled();

	void updateMusicTimer();

	bool useRemasteredAudio() const { return _useRemasteredAudio; }
	void startRemasteredSpeech(const char *msgString, uint16 roomNumber, uint16 actorTalking, uint16 currentScriptNum, uint16 currentScriptOffset, uint16 numWaits);

	// TODO: Duplicate this in Sound as well?
	bool isRolandLoom() const { return _soundCD->isRolandLoom(); }

	// CD audio wrapper methods
	int pollCD() const { return _soundCD->pollCD(); }
	void updateCD() { _soundCD->updateCD(); }
	void stopCD() {
		_soundCD->stopCD();
		_soundCD->stopCDTimer();
	}
	void playCDTrack(int track, int numLoops, int startFrame, int duration) {
		_soundCD->playCDTrack(track, numLoops, startFrame, duration);
	}
	int getCurrentCDSound() const { return _soundCD->getCurrentCDSound(); }
	void restoreCDAudioAfterLoad(AudioCDManager::Status &info) {
		_soundCD->restoreCDAudioAfterLoad(info);
	}

protected:
	void setupSfxFile();
	bool isSfxFinished() const;
	void processSfxQueues();

	bool isSoundInQueue(int sound) const;

	virtual void processSoundQueues();
};


} // End of namespace Scumm

#endif
