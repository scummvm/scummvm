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

#ifndef SCI_SFX_SOFTSEQ_MIDIDRIVER_H
#define SCI_SFX_SOFTSEQ_MIDIDRIVER_H

#include "sci/sci.h"
#include "sci/util.h"
#include "audio/mididrv.h"
#include "common/error.h"
#include "common/platform.h"

namespace Sci {

// Music patches in SCI games:
// ===========================
// 1.pat - MT-32 driver music patch
// 2.pat - Yamaha FB01 driver music patch
// 3.pat - Adlib driver music patch
// 4.pat - Casio MT-540 (in earlier SCI0 games)
// 4.pat - GM driver music patch (in later games that support GM)
// 7.pat (newer) / patch.200 (older) - Mac driver music patch / Casio CSM-1
// 9.pat (newer) / patch.005 (older) - Amiga driver music patch
// 98.pat - Unknown, found in later SCI1.1 games. A MIDI format patch
// 101.pat - CMS/PCjr driver music patch.
//           Only later PCjr drivers use this patch, earlier ones don't use a patch
// bank.001 - older SCI0 Amiga instruments

class ResourceManager;

enum {
	MIDI_CHANNELS = 16,
	MIDI_PROP_MASTER_VOLUME = 0
};

#define MIDI_RHYTHM_CHANNEL 9

/* Special SCI sound stuff */

#define SCI_MIDI_TIME_EXPANSION_PREFIX 0xF8
#define SCI_MIDI_TIME_EXPANSION_LENGTH 240

#define SCI_MIDI_EOT 0xFC
#define SCI_MIDI_SET_SIGNAL 0xCF
#define SCI_MIDI_SET_POLYPHONY 0x4B
#define SCI_MIDI_RESET_ON_SUSPEND 0x4C
#define SCI_MIDI_CHANNEL_MUTE 0x4E
#define SCI_MIDI_SET_REVERB 0x50
#define SCI_MIDI_HOLD 0x52
#define SCI_MIDI_CUMULATIVE_CUE 0x60
#define SCI_MIDI_CHANNEL_SOUND_OFF 0x78 /* all-sound-off for Bn */
#define SCI_MIDI_CHANNEL_NOTES_OFF 0x7B /* all-notes-off for Bn */

#define SCI_MIDI_SET_SIGNAL_LOOP 0x7F
/* If this is the parameter of 0xCF, the loop point is set here */

#define SCI_MIDI_CONTROLLER(status) ((status & 0xF0) == 0xB0)

class MidiPlayer : public MidiDriver_BASE {
protected:
	MidiDriver *_driver;
	int8 _reverb;

public:
	MidiPlayer(SciVersion version) : _driver(0), _reverb(-1), _version(version) { }

	int open() {
		ResourceManager *resMan = g_sci->getResMan();	// HACK
		return open(resMan);
	}
	virtual int open(ResourceManager *resMan) { return _driver->open(); }
	virtual void close() { _driver->close(); }
	void send(uint32 b) override { _driver->send(b); }
	virtual uint32 getBaseTempo() { return _driver->getBaseTempo(); }
	virtual bool hasRhythmChannel() const = 0;
	virtual void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) { _driver->setTimerCallback(timer_param, timer_proc); }

	virtual byte getPlayId() const = 0;
	virtual int getPolyphony() const = 0;
	virtual int getFirstChannel() const { return 0; }
	virtual int getLastChannel() const { return 15; }

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

	virtual void playSwitch(bool play) {
		if (!play) {
			// Send "All Sound Off" on all channels
			for (int i = 0; i < MIDI_CHANNELS; ++i)
				_driver->send(0xb0 + i, SCI_MIDI_CHANNEL_NOTES_OFF, 0);
		}
	}

	// Prepares the driver for the playback of SCI0 midi tracks.
	// The main purpose is the assignment of voices ("hardware" sound channels) to the 16 midi parts.
	// This is basically the predecessor of the 0x4B midi event.
	// Some drivers also do other things in here.
	virtual void initTrack(SciSpan<const byte> &) {}

	// There are several sound drivers which weren' part of the
	// original game setup and came in the form of aftermarket patches.
	// This method allows each driver to report missing patch or other
	// required files which will then be displayed in an error dialog box.
	// The method returns only a single string (instead of a string list),
	// because no more than two files will be required.
	virtual const char *reportMissingFiles() { return 0; }

protected:
	SciVersion _version;
};

extern MidiPlayer *MidiPlayer_AdLib_create(SciVersion version);
extern MidiPlayer *MidiPlayer_AmigaMac0_create(SciVersion version, Common::Platform platform);
extern MidiPlayer *MidiPlayer_AmigaMac1_create(SciVersion version, Common::Platform platform);
extern MidiPlayer *MidiPlayer_PCJr_create(SciVersion version);
extern MidiPlayer *MidiPlayer_PCSpeaker_create(SciVersion version);
extern MidiPlayer *MidiPlayer_CMS_create(SciVersion version);
extern MidiPlayer *MidiPlayer_MacSci0_create(SciVersion version);
extern MidiPlayer *MidiPlayer_Midi_create(SciVersion version);
extern MidiPlayer *MidiPlayer_Fb01_create(SciVersion version);
extern MidiPlayer *MidiPlayer_FMTowns_create(SciVersion version);
extern MidiPlayer *MidiPlayer_PC9801_create(SciVersion version);

} // End of namespace Sci

#endif // SCI_SFX_SOFTSEQ_MIDIDRIVER_H
