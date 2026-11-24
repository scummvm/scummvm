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

#ifndef DUNE_MUSIC_H
#define DUNE_MUSIC_H

#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "common/str.h"
#include "dune/midi_volume.h"

namespace Dune {

class AdLibPlayer;

class DuneEngine;

/**
 * The songs are MIDI in all but their wrapping.  A song file opens with
 *
 *   uint16 end;             // where the event streams end
 *   uint16 trackOffsets[21]; // offsets relative to the words holding them
 *   uint16 repeatFrom;
 *   uint16 repeatTo;
 *   uint16 repeatCount;
 *   uint16 speed;           // 8.8 fixed-point timer divider
 *
 * and the track itself opens with a word of its own before the first delta.
 * What follows is a plain MIDI event stream: variable length delta times,
 * running status, and the channel messages the game's drivers understand -
 * note on and off, program change, controller, and the channel pressure the
 * music swells with.  A single 0xFF ends it.
 *
 * The same song comes in three flavours: .HSQ for AdLib, .AGD for the AdLib
 * Gold, and .M32 for the MT-32, which is the one played here.
 */
class MidiParser_Dune : public MidiParser {
public:
	MidiParser_Dune() : MidiParser() {}

	bool loadMusic(const byte *data, uint32 size) override;
	void parseNextEvent(EventInfo &info) override;
};

class MusicPlayer : public Audio::MidiPlayer {
public:
	explicit MusicPlayer(DuneEngine *vm);

	// Loads a song by its bare name, e.g. "CRYOMUS", and plays it.
	bool play(const Common::String &name, bool loop = true);
	void stop() override;

	// Whether a song is still sounding.
	bool playing() const;
	void setLooping(bool loop);

	void setVolume(int volume) override;
	int volume() const;
	void fadeDriverVolume(uint16 duration, byte volume, byte pan);

protected:
	// The song's own repeat, which 0x418 works by 96-tick measures: it is
	// watched here as the parser goes by.
	void onTimer() override;

public:

	// Takes the song that is playing down to nothing over the time asked for,
	// which is what the original does before it puts another one on.  It is
	// carried along by fade(), and fading() says whether it is still going.
	void fadeOut(uint32 overMs);
	bool fading() const { return _fadeFor != 0; }
	void fade();

protected:
	void send(uint32 b) override;

	// Where the song asks to be repeated from and to, counted in measures of
	// ninety-six of its own ticks, and how many times over.
	uint32 _repeatFrom = 0;
	uint32 _repeatTo = 0;
	int _repeats = 0;
	bool _armed = false;

	// How long the fade lasts and when it began, and what the volume was
	// before it started.
	uint32 _fadeFor = 0;
	uint32 _fadeFrom = 0;
	int _fadeVolume = 0;

	// The player the AdLib songs go through, where the device is one.
	AdLibPlayer *_adlib = nullptr;

private:
	void updateMidiVolumes();
	MidiVolume _midiVolume;
	uint64 _midiTimerFraction = 0;
	DuneEngine *_vm;
};

} // End of namespace Dune

#endif
