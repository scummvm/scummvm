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

#ifndef AGOS_DRIVERS_ACCOLADE_CMS_H
#define AGOS_DRIVERS_ACCOLADE_CMS_H

#include "audio/mididrv.h"

#include "audio/cms.h"

namespace AGOS {

// MIDI driver for the Creative Music System / Gameblaster.
// This driver uses only 6 of the available 12 channels and does not support
// stereo. It only supports note on and off and program change; the selected
// instrument only affects the note volume.
class MidiDriver_Accolade_Cms : public MidiDriver {
protected:
	static const byte REGISTER_BASE_AMPLITUDE = 0x00;
	static const byte REGISTER_BASE_FREQUENCY = 0x08;
	static const byte REGISTER_BASE_OCTAVE = 0x10;
	static const byte REGISTER_FREQUENCY_ENABLE = 0x14;
	static const byte REGISTER_RESET_SOUND_ENABLE = 0x1C;

	// Frequency register values for octave notes.
	static const byte CMS_NOTE_FREQUENCIES[12];
	// Volume adjustments for all instruments.
	static const byte CMS_VOLUME_ADJUSTMENTS[128];

public:
	MidiDriver_Accolade_Cms();
	~MidiDriver_Accolade_Cms() override;

	int open() override;
	void close() override;
	bool isOpen() const override;
	uint32 getBaseTempo() override;
	// This driver does not support MidiChannel objects.
	MidiChannel *allocateChannel() override;
	// This driver does not support MidiChannel objects.
	MidiChannel *getPercussionChannel() override;

	using MidiDriver::send;
	void send(uint32 b) override;

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;
	void onTimer();

protected:
	void cmsInit();

	void noteOff(uint8 channel, uint8 note);
	void noteOn(uint8 channel, uint8 note, uint8 velocity);
	void programChange(uint8 channel, uint8 instrument);
	void controlChange(uint8 channel, uint8 controller, uint8 value);

	// Returns the value for the frequency enable register of the first CMS
	// chip based on the _activeNotes array (if a note is active, the bit for
	// the corresponding channel is set).
	byte determineFrequencyEnableRegisterValue();
	void writeRegister(uint16 reg, uint8 value);

	CMS::CMS *_cms;
	bool _isOpen;

	// The selected instrument on each MIDI channel.
	byte _instruments[16];
	// The active note on each CMS channel (0xFF if no note is active).
	byte _activeNotes[12];
	// The current values of the CMS octave registers (0x10 - 0x12).
	byte _octaveRegisterValues[6];

	// External timer callback
	void *_timer_param;
	Common::TimerManager::TimerProc _timer_proc;
};

} // End of namespace AGOS

#endif
