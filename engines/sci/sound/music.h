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

#ifndef SCI_MUSIC_H
#define SCI_MUSIC_H

#include "common/serializer.h"
#include "common/mutex.h"

#include "audio/mixer.h"

#include "sci/sci.h"
#include "sci/resource/resource.h"
#include "sci/sound/drivers/mididriver.h"
#ifdef ENABLE_SCI32
#include "sci/sound/audio32.h"
#endif

namespace Audio {
class LoopingAudioStream;
class RewindableAudioStream;
}

namespace Sci {

enum SoundStatus {
	kSoundStopped = 0,
	kSoundInitialized = 1,
	kSoundPaused = 2,
	kSoundPlaying = 3
};

#define MUSIC_VOLUME_DEFAULT 127
#define MUSIC_VOLUME_MAX 127
#define MUSIC_MASTERVOLUME_DEFAULT 15
#define MUSIC_MASTERVOLUME_MAX 15

class MidiParser_SCI;
class SegManager;

typedef Common::Array<uint16> SignalQueue;


struct MusicEntryChannel {
	// Channel info
	int8 _prio; // 0 = essential; lower is higher priority
	int8 _voices;
	bool _dontRemap;
	bool _dontMap;
	bool _mute;
};


class MusicEntry : public Common::Serializable {
public:
	// Do not get these directly for the sound objects!
	// It's a bad idea, as the sound code (i.e. the SciMusic
	// class) should be as separate as possible from the rest
	// of the engine

	reg_t soundObj;

	SoundResource *soundRes;
	uint16 resourceId;

	int time; // "tim"estamp to indicate in which order songs have been added

	bool isQueued; // for SCI0 only!

	uint16 dataInc;
	uint16 ticker;
	uint16 signal;
	int16 priority; // must be int16, at least in Laura Bow 1, main music (object conMusic) uses priority -1
	uint16 loop;
	int16 volume;
	int16 hold;
	int8 reverb;
	bool playBed;
	bool overridePriority; // Use soundObj's priority instead of resource's

	int16 pauseCounter;
	uint sampleLoopCounter;

	byte fadeTo;
	short fadeStep;
	uint32 fadeTicker;
	uint32 fadeTickerStep;
	bool fadeSetVolume;
	bool fadeCompleted;
	bool stopAfterFading;

	SoundStatus status;

	Audio::Mixer::SoundType soundType;

	int _usedChannels[16];
	MusicEntryChannel _chan[16];
	MidiParser_SCI *pMidiParser;

	// this is used for storing signals, when the current signal is not yet
	//  sent to the scripts. We shouldn't need to save it, this normally only
	//  happens in rare situations like lb1, knocking on the door in the attic
	SignalQueue signalQueue;

	// TODO: We need to revise how we store the different
	// audio stream objects we require.
	Audio::RewindableAudioStream *pStreamAud;
	Audio::LoopingAudioStream *pLoopStream;
	Audio::SoundHandle hCurrentAud;
	bool isSample;

public:
	MusicEntry();
	~MusicEntry() override;

	void doFade();
	void onTimer();
	void setSignal(int signal);

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

struct DeviceChannelUsage {
	MusicEntry *_song;
	int _channel;
	bool operator==(const DeviceChannelUsage& other) const { return _song == other._song && _channel == other._channel; }
	bool operator!=(const DeviceChannelUsage& other) const { return !(*this == other); }
};

struct ChannelRemapping {
	DeviceChannelUsage _map[16];
	int _prio[16];
	int _voices[16];
	bool _dontRemap[16];
	int _freeVoices;

	void clear();
	void swap(int i, int j);
	void evict(int i);
	ChannelRemapping& operator=(ChannelRemapping& other);
	int lowestPrio() const;
};

typedef Common::Array<MusicEntry *> MusicList;
typedef Common::Array<uint32> MidiCommandQueue;

class SciMusic : public Common::Serializable {

public:
	SciMusic(SciVersion soundVersion, bool useDigitalSFX);
	~SciMusic() override;

	void init();

	void onTimer();
	void putMidiCommandInQueue(byte status, byte firstOp, byte secondOp);
	void putMidiCommandInQueue(uint32 midi);
private:
	static void miditimerCallback(void *p);
	void sendMidiCommandsFromQueue();

public:
	void clearPlayList();
	void pauseAll(bool pause);
	void stopAll();

	// sound and midi functions
	void soundInitSnd(MusicEntry *pSnd);
	void soundPlay(MusicEntry *pSnd);
	void soundStop(MusicEntry *pSnd);
	void soundKill(MusicEntry *pSnd);
	void soundPause(MusicEntry *pSnd);
	void soundResume(MusicEntry *pSnd);
	void soundToggle(MusicEntry *pSnd, bool pause);
	void soundSetVolume(MusicEntry *pSnd, byte volume);
	void soundSetSampleVolume(MusicEntry *pSnd, byte volume);
	void soundSetPriority(MusicEntry *pSnd, byte prio);
	uint16 soundGetMasterVolume();
	void soundSetMasterVolume(uint16 vol);
	uint16 soundGetSoundOn() const { return _soundOn; }
	void soundSetSoundOn(bool soundOnFlag);
	uint16 soundGetVoices();
	uint32 soundGetTempo() const { return _dwTempo; }
	MusicType soundGetMusicType() const { return _musicType; }

	bool soundIsActive(MusicEntry *pSnd) {
		assert(pSnd->pStreamAud != 0);
		return _pMixer->isSoundHandleActive(pSnd->hCurrentAud);
	}

	void updateAudioStreamTicker(MusicEntry *pSnd) {
		assert(pSnd->pStreamAud != 0);
		pSnd->ticker = (uint16)(_pMixer->getSoundElapsedTime(pSnd->hCurrentAud) * 0.06);
	}

	MusicEntry *getSlot(reg_t obj);
	MusicEntry *getActiveSci0MusicSlot();

	void pushBackSlot(MusicEntry *slotEntry) {
		Common::StackLock lock(_mutex);
		_playList.push_back(slotEntry);
	}

	void printPlayList(Console *con);
	void printSongInfo(reg_t obj, Console *con);

	// The following two methods are NOT thread safe - make sure that
	// the mutex is always locked before calling them
	MusicList::iterator getPlayListStart() { return _playList.begin(); }
	MusicList::iterator getPlayListEnd() { return _playList.end(); }

	void sendMidiCommand(uint32 cmd);
	void sendMidiCommand(MusicEntry *pSnd, uint32 cmd);

	void setGlobalReverb(int8 reverb);
	int8 getGlobalReverb() { return _globalReverb; }

	byte getCurrentReverb();

	void needsRemap() { _needsRemap = true; }

	void saveLoadWithSerializer(Common::Serializer &ser) override;

	// Mutex for music code. Used to guard access to the song playlist, to the
	// MIDI parser and to the MIDI driver/player. Note that guarded code must NOT
	// include references to the mixer, otherwise there will probably be situations
	// where a deadlock can occur
	Common::Mutex _mutex;

protected:
	void sortPlayList();

	SciVersion _soundVersion;

	Audio::Mixer *_pMixer;
	MidiPlayer *_pMidiDrv;

	uint32 _dwTempo;
	// If true and a sound has a digital track, the sound from the AdLib track is played
	bool _useDigitalSFX;

	// remapping:
	void remapChannels(bool mainThread = true);
	ChannelRemapping *determineChannelMap();
	void resetDeviceChannel(int devChannel, bool mainThread);

private:
	MusicList _playList;
	bool _soundOn;
	byte _masterVolume;
	MusicEntry *_usedChannel[16];
	int8 _channelRemap[16];
	int8 _globalReverb;
	bool _needsRemap;

	DeviceChannelUsage _channelMap[16];

	MidiCommandQueue _queuedCommands;
	MusicType _musicType;

	int _driverFirstChannel;
	int _driverLastChannel;

	MusicEntry *_currentlyPlayingSample;

	int _timeCounter; // Used to keep track of the order in which MusicEntries
	                  // are added, for priority purposes.
};

} // End of namespace Sci

#endif
