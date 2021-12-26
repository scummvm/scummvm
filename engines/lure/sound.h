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

#ifndef LURE_SOUND_H
#define LURE_SOUND_H

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"

#include "common/mutex.h"
#include "common/singleton.h"
#include "common/ptr.h"

#include "audio/adlib_ms.h"
#include "audio/mididrv.h"
#include "audio/mt32gm.h"

class MidiParser;
class MidiChannel;

namespace Lure {

#define NUM_CHANNELS 16
#define LURE_MAX_SOURCES 10

class MidiMusic: public MidiDriver_BASE {
private:
	uint8 _soundNumber;
	uint8 _numChannels;
	byte _volume;
	MemoryBlock *_decompressedSound;
	uint8 *_soundData;
	uint8 _soundSize;
	MidiDriver_Multisource *_driver;
	MidiDriver_MT32GM *_mt32Driver;
	int8 _source;
	MidiParser *_parser;
	bool _isMusic;
	bool _loop;
	bool _isPlaying;

	void queueUpdatePos();
	uint8 randomQueuePos();
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

public:
	MidiMusic(MidiDriver_Multisource *driver, uint8 soundNum, bool isMus, bool loop,
		int8 source, uint8 numChannels, void *soundData, uint32 size, uint8 volume);
	~MidiMusic() override;
	void setVolume(int volume);
	int getVolume() const { return _volume; }

	void playSong(uint16 songNum);
	void stopSong() { stopMusic(); }
	void playMusic();
	void stopMusic();
	void pauseMusic();
	void resumeMusic();
	void queueTuneList(int16 tuneList);
	bool queueSong(uint16 songNum);
	void toggleVChange();

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;
	void send(int8 source, uint32 b) override;
	void metaEvent(byte type, byte *data, uint16 length) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

	void onTimer();

	uint8 soundNumber() const { return _soundNumber; }
	int8 getSource() const { return _source; }
	bool isPlaying() const { return _isPlaying; }
	bool isMusic() const { return _isMusic; }
};

class SoundManager : public Common::Singleton<SoundManager> {
private:
	// Outer sound interface properties
	MemoryBlock *_descs;
	MemoryBlock *_soundData;
	uint8 _soundsTotal;
	int _numDescs;
	SoundDescResource *soundDescs() { return (SoundDescResource *) _descs->data(); }
	MidiDriver_Multisource *_driver;
	MidiDriver_MT32GM *_mt32Driver;
	typedef Common::List<Common::SharedPtr<SoundDescResource> > SoundList;
	typedef SoundList::iterator SoundListIterator;
	SoundList _activeSounds;
	typedef Common::List<Common::SharedPtr<MidiMusic> > MusicList;
	typedef MusicList::iterator MusicListIterator;
	MusicList _playingSounds;
	bool _sourcesInUse[LURE_MAX_SOURCES];
	bool _nativeMT32;
	bool _isRoland;
	Common::Mutex _soundMutex;
	bool _paused;

	// Internal support methods
	void bellsBodge();
	void musicInterface_TidySounds();
	static void onTimer(void *data);
	void doTimer();

public:
	SoundManager();
	~SoundManager() override;

	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);

	void loadSection(uint16 sectionId);
	bool initCustomTimbres(bool canAbort = false);
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
	bool fadeOut();
	void pause();
	void resume();
	bool getPaused() const { return _paused; }
	bool hasNativeMT32() const { return _nativeMT32; }
	bool isRoland() const { return _isRoland; }

	// The following methods implement the external sound player module
	//void musicInterface_Initialize();
	void musicInterface_Play(uint8 soundNumber, bool isMusic = false, uint8 numChannels = 4, uint8 volume = 0x80);
	void musicInterface_Stop(uint8 soundNumber);
	bool musicInterface_CheckPlaying(uint8 soundNumber);
	void musicInterface_SetVolume(uint8 soundNumber, uint8 volume);
	void musicInterface_KillAll();
	void musicInterface_ContinuePlaying();
	void musicInterface_TrashReverb();
};

// AdLib MidiDriver subclass implementing the behavior specific to Lure of the
// Temptress.
class MidiDriver_ADLIB_Lure : public MidiDriver_ADLIB_Multisource {
protected:
	// Lookup array for OPL frequency (F-num) values.
	static const uint16 OPL_FREQUENCY_LOOKUP[];

public:
	MidiDriver_ADLIB_Lure();

	// Channel aftertouch is used to adjust a note's volume. This is done by
	// overriding the velocity of the active note and recalculating the volume.
	void channelAftertouch(uint8 channel, uint8 pressure, uint8 source) override;
	// The MIDI data uses three custom sequencer meta events; the most important
	// one sets the instrument definition for a channel.
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

protected:
	InstrumentInfo determineInstrument(uint8 channel, uint8 source, uint8 note)	override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	// Returns the number of semitones in bits 8+ and an 8 bit fraction of a
	// semitone.
	int32 calculatePitchBend(uint8 channel, uint8 source, uint16 oplFrequency) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;

	// Stores the instrument definitions set by sequencer meta events.
	OplInstrumentDefinition _instrumentDefs[LURE_MAX_SOURCES][MIDI_CHANNEL_COUNT];
	// Pitch bend sensitivity in semi-tones. This is a global setting;
	// it cannot be specified for a specific MIDI channel.
	uint8 _pitchBendSensitivity;
};

} // End of namespace Lure

#define Sound (::Lure::SoundManager::instance())

#endif
