/* ScummVM - Scumm Interpreter
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LURE_SOUND_H
#define LURE_SOUND_H

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"

#include "common/singleton.h"
#include "common/ptr.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

class MidiParser;

namespace Lure {

#define NUM_CHANNELS 16

struct ChannelEntry {
	MidiChannel *midiChannel;
	uint8 volume;
};

class MidiMusic: public MidiDriver {
private:
	uint8 _soundNumber;
	uint8 _channelNumber;
	uint8 _numChannels;
	byte _volume;
	MemoryBlock *_decompressedSound;
	uint8 *_soundData;
	uint8 _soundSize;
	MidiDriver *_driver;
	MidiParser *_parser;
	ChannelEntry *_channels;
	bool _isMusic;
	bool _isPlaying;

	void queueUpdatePos();
	uint8 randomQueuePos();
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

	bool _passThrough;

public:
	MidiMusic(MidiDriver *driver, ChannelEntry channels[NUM_CHANNELS],
		 uint8 channelNum, uint8 soundNum, bool isMus, uint8 numChannels, void *soundData, uint32 size);
	~MidiMusic();
	void setVolume(int volume);
	int getVolume() { return _volume; }

	void playSong(uint16 songNum);
	void stopSong() { stopMusic(); }
	void playMusic();
	void stopMusic();
	void queueTuneList(int16 tuneList);
	bool queueSong(uint16 songNum);
	void setPassThrough(bool b) { _passThrough = b; }
	void toggleVChange();

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);
	void onTimer();

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo() { return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	uint8 channelNumber() { return _channelNumber; }
	uint8 soundNumber() { return _soundNumber; }
	bool isPlaying() { return _isPlaying; }
	bool isMusic() {return _isMusic; }
};

class SoundManager: public Common::Singleton<SoundManager> {
private:
	// Outer sound interface properties
	MemoryBlock *_descs;
	MemoryBlock *_soundData;
	uint8 _soundsTotal;
	int _numDescs;
	SoundDescResource *soundDescs() { return (SoundDescResource *) _descs->data(); }
	MidiDriver *_driver;
	typedef Common::List<Common::SharedPtr<SoundDescResource> > SoundList;
	typedef SoundList::iterator SoundListIterator;
	SoundList _activeSounds;
	typedef Common::List<Common::SharedPtr<MidiMusic> > MusicList;
	typedef MusicList::iterator MusicListIterator;
	MusicList _playingSounds;
	ChannelEntry _channelsInner[NUM_CHANNELS];
	bool _channelsInUse[NUM_CHANNELS];
	bool _isPlaying;
	bool _nativeMT32;
	bool _isRoland;
	Common::MutexRef _soundMutex;
	bool _paused;

	// Internal support methods
	void bellsBodge();
	void musicInterface_TidySounds();
	static void onTimer(void *data);
	void doTimer();
public:
	SoundManager();
	~SoundManager();

	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);

	void loadSection(uint16 sectionId);
	void killSounds();
	void addSound(uint8 soundIndex, bool tidyFlag = true);
	void addSound2(uint8 soundIndex);
	void stopSound(uint8 soundIndex);
	void killSound(uint8 soundNumber);
	void setVolume(uint8 soundNumber, uint8 volume);
	void syncSounds();
	void tidySounds();
	uint8 descIndexOf(uint8 soundNumber);
	SoundDescResource *findSound(uint8 soundNumber);
	void removeSounds();
	void restoreSounds();
	void fadeOut();
	void pause() { _paused = true; }
	void resume() { _paused = false; }
	bool getPaused() { return _paused; }
	bool hasNativeMT32() { return _nativeMT32; }
	bool isRoland() { return _isRoland; }

	// The following methods implement the external sound player module
	void musicInterface_Initialise();
	void musicInterface_Play(uint8 soundNumber, uint8 channelNumber, uint8 numChannels = 4);
	void musicInterface_Stop(uint8 soundNumber);
	bool musicInterface_CheckPlaying(uint8 soundNumber);
	void musicInterface_SetVolume(uint8 channelNum, uint8 volume);
	void musicInterface_KillAll();
	void musicInterface_ContinuePlaying();
	void musicInterface_TrashReverb();
};

} // End of namespace Lure

#define Sound (::Lure::SoundManager::instance())

#endif
