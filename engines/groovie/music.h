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

#ifndef GROOVIE_MUSIC_H
#define GROOVIE_MUSIC_H

#include "common/array.h"
#include "common/mutex.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"
#include "audio/miles.h"

class MidiParser;

namespace Groovie {

class GroovieEngine;

class MusicPlayer {
public:
	MusicPlayer(GroovieEngine *vm);
	virtual ~MusicPlayer();

	void playSong(uint32 fileref);
	// Stops all music playback. Clears the current
	// background song.
	void stop();
	void setBackgroundSong(uint32 fileref);
	void playCD(uint8 track);
	void startBackground();
	bool isPlaying() { return _isPlaying; }
	// Pause or resume the music. Note that digital music
	// already pauses when the ScummVM menu is open, so
	// it does not seem to need an implementation.
	virtual void pause(bool pause) { }

	// Set the MIDI initialization state
	void setMidiInit(bool midiInit) { _midiInit = midiInit; }
	// Returns true if MIDI has been fully initialized
	bool isMidiInit() { return _midiInit; }

	void frameTick();
	void setBackgroundDelay(uint16 delay);

	// Volume
	void setUserVolume(uint16 volume);
	void setGameVolume(uint16 volume, uint16 time);

private:
	// Song playback
	bool play(uint32 fileref, bool loop);
	bool _isPlaying;
	uint32 _backgroundFileRef;
	uint8 _prevCDtrack;

	uint16 _backgroundDelay;

	// T7G iOS credits mp3 stream
	void playCreditsIOS();
	void stopCreditsIOS();
	Audio::SoundHandle _handleCreditsIOS;

	// Volume fading
	uint32 _fadingStartTime;
	uint16 _fadingStartVolume;
	uint16 _fadingEndVolume;
	uint16 _fadingDuration;
	void applyFading();

protected:
	GroovieEngine *_vm;

	// True if the MIDI initialization has completed
	bool _midiInit;

	// Callback
	static void onTimer(void *data);
	virtual void onTimerInternal() {}
	Common::Mutex _mutex;

	// User volume
	uint16 _userVolume;
	// Game volume
	uint16 _gameVolume;

	// These are specific for each type of music
	virtual void updateVolume() = 0;
	virtual bool load(uint32 fileref, bool loop) = 0;
	virtual void unload(bool updateState = true);
};

class MusicPlayerMidi : public MusicPlayer, public MidiDriver_BASE {
public:
	MusicPlayerMidi(GroovieEngine *vm);
	~MusicPlayerMidi() override;

	// MidiDriver_BASE interface
	void send(uint32 b) override;
	void sysEx(const byte* msg, uint16 length) override;
	uint16 sysExNoDelay(const byte *msg, uint16 length) override;
	void metaEvent(byte type, byte *data, uint16 length) override;

	void pause(bool pause) override;

private:
	// Channel volumes
	byte _chanVolumes[0x10];
	void updateChanVolume(byte channel);

protected:
	byte *_data;
	MidiParser *_midiParser;
	MidiDriver *_driver;

	void onTimerInternal() override;
	void updateVolume() override;
	void unload(bool updateState = true) override;
	void endTrack();

	bool loadParser(Common::SeekableReadStream *stream, bool loop);
};

class MusicPlayerXMI : public MusicPlayerMidi, public Audio::MidiDriver_Miles_Xmidi_Timbres {
public:
	MusicPlayerXMI(GroovieEngine *vm, const Common::String &gtlName);
	~MusicPlayerXMI();

	using MusicPlayerMidi::send;
	void send(int8 source, uint32 b) override;
	using MusicPlayerMidi::metaEvent;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;
	void stopAllNotes(bool stopSustainedNotes) override;
	void processXMIDITimbreChunk(const byte *timbreListPtr, uint32 timbreListSize) override {
		if (_milesMidiDriver)
			_milesMidiDriver->processXMIDITimbreChunk(timbreListPtr, timbreListSize);
	};
	bool isReady() override;

protected:
	void updateVolume() override;
	bool load(uint32 fileref, bool loop) override;
	void unload(bool updateState = true) override;

private:
	// Output music type
	uint8 _musicType;

	Audio::MidiDriver_Miles_Midi *_milesMidiDriver;
};

class MusicPlayerMac_t7g : public MusicPlayerMidi {
public:
	MusicPlayerMac_t7g(GroovieEngine *vm);

protected:
	bool load(uint32 fileref, bool loop) override;

private:
	Common::SeekableReadStream *decompressMidi(Common::SeekableReadStream *stream);
};

class MusicPlayerMac_v2 : public MusicPlayerMidi {
public:
	MusicPlayerMac_v2(GroovieEngine *vm);

protected:
	bool load(uint32 fileref, bool loop) override;
};

class MusicPlayerIOS : public MusicPlayer {
public:
	MusicPlayerIOS(GroovieEngine *vm);
	~MusicPlayerIOS() override;

protected:
	void updateVolume() override;
	bool load(uint32 fileref, bool loop) override;
	void unload(bool updateState = true) override;

private:
	Audio::SoundHandle _handle;
};

} // End of Groovie namespace

#endif // GROOVIE_MUSIC_H
