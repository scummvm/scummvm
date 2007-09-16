/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include "sound/mididrv.h"
#include "sound/mixer.h"

class MidiParser;

namespace Lure {

#define NUM_CHANNELS_OUTER 8
#define NUM_CHANNELS_INNER 16

struct ChannelEntry {
	MidiChannel *midiChannel;
	byte volume;
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
	bool _isPlaying;
	bool _nativeMT32;

	void queueUpdatePos();
	uint8 randomQueuePos();
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

	bool _passThrough;

public:
	MidiMusic(MidiDriver *driver, ChannelEntry channels[NUM_CHANNELS_INNER], 
		uint8 channelNum, uint8 soundNum, void *soundData, uint32 size);
	~MidiMusic();
	void setVolume(int volume);
	int getVolume()	{ return _volume; }

	void hasNativeMT32(bool b)	{ _nativeMT32 = b; }
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
	uint32 getBaseTempo(void)	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	uint8 channelNumber() { return _channelNumber; }
	uint8 soundNumber() { return _soundNumber; }
	bool isPlaying() { return _isPlaying; }
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
	ManagedList<SoundDescResource *> _activeSounds;
	ManagedList<MidiMusic *> _playingSounds;
	ChannelEntry _channelsInner[NUM_CHANNELS_INNER];
	bool _channelsInUse[NUM_CHANNELS_OUTER];
	bool _isPlaying;
	bool _nativeMT32;

	// Internal support methods
	void bellsBodge();
	void musicInterface_TidySounds();
	static void onTimer(void *data);
public:
	SoundManager();
	~SoundManager();

	void loadSection(uint16 sectionId);
	void killSounds();
	void addSound(uint8 soundIndex, bool tidyFlag = true);
	void addSound2(uint8 soundIndex);
	void stopSound(uint8 soundIndex);
	void killSound(uint8 soundNumber);
	void setVolume(uint8 soundNumber, uint8 volume);
	void setVolume(uint8 volume);
	void tidySounds();
	SoundDescResource *findSound(uint8 soundNumber);
	void removeSounds();
	void restoreSounds();
	void fadeOut();

	// The following methods implement the external sound player module
	void musicInterface_Initialise();
	void musicInterface_Play(uint8 soundNumber, uint8 channelNumber);
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
