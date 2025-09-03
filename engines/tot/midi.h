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

#ifndef TOT_MIDI_H
#define TOT_MIDI_H

#include "audio/adlib_ms.h"
#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

namespace Tot {

class MidiDriver_AdLib : public MidiDriver_ADLIB_Multisource {
public:
	MidiDriver_AdLib(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_AdLib();

	void loadInstrumentBank(uint8 *instrumentBankData);
	void loadInstrumentBankFromDriver(int32 offset);

protected:
	OplInstrumentDefinition *_dsfInstrumentBank;
	OplInstrumentDefinition *_rhythmBank;
};

class MidiPlayer {
protected:
	// Driver used for music. This points to the same object as _driverMsMusic,
	// except if a PC-98 driver is used (these do not implement the Multisource
	// interface).
	MidiDriver *_driver;
	// Multisource driver used for music. Provides access to multisource
	// methods without casting. If this is not nullptr, it points to the same
	// object as _driver.
	MidiDriver_AdLib *_driverMsMusic;
	// MIDI parser and data used for music.
	MidiParser *_parserMusic;
	byte *_musicData;

	bool _paused;

protected:
	static void onTimer(void *data);

public:
	MidiPlayer();
	~MidiPlayer();

	// Creates and opens the relevant parsers and drivers for the game version
	// and selected sound device.
	int open();
	// Loads music or SFX data supported by the MidiParser or SfxParser used
	// for the detected version of the game. Specify sfx to indicate that this
	// is a synthesized sound effect.
	void load(Common::SeekableReadStream *in, int32 size = -1);

	void play(int track);

	void setLoop(bool loop);

	bool isPlaying();
	void stop();
	void pause(bool b);
	void syncSoundSettings();

private:
};

} // End of namespace Tot

#endif
