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

#ifndef DUNE_ADLIB_H
#define DUNE_ADLIB_H

#include "audio/fmopl.h"
#include "dune/gold_volume.h"
#include "common/array.h"
#include "common/mutex.h"
#include "common/str.h"

namespace Dune {

class Archive;

/**
 * The songs the game keeps for the AdLib Gold, which are the `.AGD` version of
 * each song.  They are not MIDI for a general instrument: they carry the
 * chip's own instruments with them, and the driver DNADG.HSQ says how each of
 * them is written into its registers.
 *
 * A song is up to eighteen streams of events, one to each channel of the two
 * OPL3 banks, laid end to end.  Words from +2 name their beginnings less two,
 * ending at the first zero; the word at +0 says where they all end, and what
 * follows is the instruments, forty bytes apiece.
 *
 * An event is the same shape as the MT-32 songs': a delta counted in the
 * song's own ticks, then a status byte that may be left out where it is the
 * same as the last.  A stream only ever speaks of its own channel, and ends
 * with 0xFF.
 */
class AdLibPlayer {
public:
	AdLibPlayer();
	~AdLibPlayer();

	bool open();
	bool play(Archive &archive, const Common::String &name, bool loop);
	void stop();
	bool playing() const { return _playing; }
	void setLooping(bool loop);

	// Nought to 255, the way the rest of the mixer counts.
	void setVolume(int volume);
	int getVolume() const { return _volume; }
	void fadeDriverVolume(uint16 duration, byte volume, byte pan);
	void setDriverVolume(byte volume, byte pan);

private:
	struct Voice;

	static void onTimer(void *self);
	void tick();
	void songTick();

	void writeReg(int reg, int value, uint channel = 0);
	void loadInstrument(uint channel, uint which);
	void noteOn(uint channel, byte note, byte velocity);
	void noteOff(uint channel, byte note);
	void writeNote(uint channel, byte note, bool on, bool update = false);
	void aftertouch(uint channel, byte velocity);
	void pitchBend(uint channel, byte bend);
	void slide(uint channel);
	void scaleOperator(uint channel, bool carrier, int sensitivity, byte velocity);
	void scaleFeedback(uint channel, int sensitivity, byte velocity);
	bool readDelta(Voice &voice, uint32 &delta);
	void executeEvent(uint channel);
	void silence();

	// One of the streams: where it is, how long until its next event,
	// and what it is doing.
	struct Voice {
		uint32 at = 0;
		uint32 from = 0;
		uint32 to = 0;
		uint32 waiting = 0;
		byte running = 0;
		bool done = false;
		bool first = true;
		bool nextDelta = true;
		int note = -1;
		int instrument = -1;
		byte bend = 0x40;
		byte slideDuration = 0;
		bool keyOn = false;
	};

	void keepTheVoices();
	void putTheVoicesBack();

	OPL::OPL *_opl = nullptr;
	Common::Mutex _mutex;

	Common::Array<byte> _song;
	uint32 _bank = 0;
	uint _instruments = 0;

	enum { kMaxVoices = 18 };
	Voice _voices[kMaxVoices];
	uint _voiceCount = 0;

	// Where the song asks to be repeated from and to, counted in measures of
	// ninety-six ticks, and how many times over; and the voices as they stood
	// when the repeat began.
	uint16 _repeatFrom = 0;
	uint16 _repeatTo = 0;
	int _repeats = 0;
	bool _armed = false;
	Voice _kept[kMaxVoices];
	uint32 _measure = 0;
	uint32 _inMeasure = 96;
	uint16 _speed = 0x400;
	int _time = 0;

	bool _playing = false;
	bool _loop = false;
	int _volume = 255;
	bool _hasOutputGain = false;
	GoldVolume _goldVolume;
};

} // End of namespace Dune

#endif
