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

#ifndef SHERLOCK_SOFTSEQ_MIDIDRIVER_H
#define SHERLOCK_SOFTSEQ_MIDIDRIVER_H

#include "sherlock/sherlock.h"
//#include "audio/mididrv.h"
#include "common/error.h"

namespace Sherlock {

#define USE_SCI_MIDIPLAYER 1

#if USE_SCI_MIDIPLAYER
enum {
	MIDI_CHANNELS = 16,
	MIDI_PROP_MASTER_VOLUME = 0
};

#define MIDI_RHYTHM_CHANNEL 9

class MidiPlayer : public MidiDriver_BASE {
protected:
	MidiDriver *_driver;
	int8 _reverb;

public:
	MidiPlayer() : _driver(0), _reverb(-1) { }

	virtual int open() { return _driver->open(); }
	virtual void close() { _driver->close(); }
	virtual void send(uint32 b) { _driver->send(b); }
	virtual uint32 getBaseTempo() { return _driver->getBaseTempo(); }
	virtual bool hasRhythmChannel() const = 0;
	virtual void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) { _driver->setTimerCallback(timer_param, timer_proc); }

	virtual byte getPlayId() const = 0;
	virtual int getPolyphony() const = 0;
	virtual int getFirstChannel() const { return 0; }
	//virtual int getLastChannel() const { return 15; }

	virtual void setVolume(byte volume) {
		if(_driver)
			_driver->property(MIDI_PROP_MASTER_VOLUME, volume);
	}

	virtual int getVolume() {
		return _driver ? _driver->property(MIDI_PROP_MASTER_VOLUME, 0xffff) : 0;
	}

	// Returns the current reverb, or -1 when no reverb is active
	int8 getReverb() const { return _reverb; }
	// Sets the current reverb, used mainly in MT-32
	virtual void setReverb(int8 reverb) { _reverb = reverb; }

	// Special stuff for Sherlock Holmes
//	virtual void newMusicData(byte *musicData, int32 musicDataSize);

//protected:
};

extern MidiPlayer *MidiPlayer_AdLib_create();
extern void MidiPlayer_AdLib_newMusicData(MidiPlayer *driver, byte *musicData, int32 musicDataSize);
#endif

extern MidiDriver *MidiDriver_AdLib_create();
extern void MidiDriver_AdLib_newMusicData(MidiDriver *driver, byte *musicData, int32 musicDataSize);

} // End of namespace Sci

#endif // SHERLOCK_SOFTSEQ_MIDIDRIVER_H
