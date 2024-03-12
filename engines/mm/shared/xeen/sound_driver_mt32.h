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

#ifndef MM_SHARED_XEEN_SOUND_DRIVER_MT32_H
#define MM_SHARED_XEEN_SOUND_DRIVER_MT32_H


#include "audio/mididrv.h"
#include "mm/shared/xeen/sound_driver.h"

namespace MM {
namespace Shared {
namespace Xeen {

class SoundDriverMT32 : public SoundDriver {
	struct MidiValue {
		uint32 _val;

		MidiValue(uint8 command, uint8 op1, uint8 op2) {
			_val = (command) | ((uint32)op2 << 16) | ((uint32)op1 << 8);
		}
	};
public:
	MidiDriver *_midiDriver;
	uint32 _timerCount;
private:
	static const uint8 MIDI_NOTE_MAP[24];
private:
	MidiChannel *_midiChannels[CHANNEL_COUNT];
	Common::Queue<MidiValue> _queue;
	Common::Mutex _driverMutex;
	const byte *_musInstrumentPtrs[16];
	const byte *_fxInstrumentPtrs[16];
	byte _last_notes[16];
	int _field180;
	int _field181;
	int _field182;
	int _musicVolume, _sfxVolume;
private:
	/**
	 * Initializes the state of the MT32 driver
	 */
	void initialize();

	/**
	 * Adds a register write to the pending queue that will be flushed
	 * out to the MT32 on the next timer call
	 */
	void write(uint8 command, uint8 op1, uint8 op2);

	/**
	 * Flushes any pending writes to the OPL
	 */
	void flush();

	/**
	 * Resets all the output frequencies
	 */
	void resetFrequencies();

	/**
	 * Starts playing an instrument
	 */
	void playInstrument(byte channelNum, const byte *data, bool isFx);

	/**
	 * Calculates the scaling/volume level to output based on sfx or music master volume
	 */
	byte calculateLevel(byte level, bool isFx);

	/**
	 * Maps note using hardcoded notes table
	 */
	byte noteMap(byte note);
protected:
	bool musSetInstrument(const byte *&srcP, byte param) override;
	bool musSetPitchWheel(const byte *&srcP, byte param) override;
	bool musSetPanning(const byte *&srcP, byte param) override;
	bool musFade(const byte *&srcP, byte param) override;
	bool musStartNote(const byte *&srcP, byte param) override;
	bool musSetVolume(const byte *&srcP, byte param) override;
	bool musInjectMidi(const byte *&srcP, byte param) override;
	bool musPlayInstrument(const byte *&srcP, byte param) override;
	bool cmdFreezeFrequency(const byte *&srcP, byte param) override;
	bool cmdChangeFrequency(const byte *&srcP, byte param) override;

	bool fxSetInstrument(const byte *&srcP, byte param) override;
	bool fxSetVolume(const byte *&srcP, byte param) override;
	bool fxMidiReset(const byte *&srcP, byte param) override;
	bool fxMidiDword(const byte *&srcP, byte param) override;
	bool fxSetPanning(const byte *&srcP, byte param) override;
	bool fxChannelOff(const byte *&srcP, byte param) override;
	bool fxFade(const byte *&srcP, byte param) override;
	bool fxStartNote(const byte *&srcP, byte param) override;
	bool fxInjectMidi(const byte *&srcP, byte param) override;
	bool fxPlayInstrument(const byte *&srcP, byte param) override;

	/**
	 * Post-processing done when a pause countdown starts or is in progress
	 */
	void pausePostProcess() override;

	/**
	 * Does a reset of any sound effect
	 */
	void resetFX() override;

	/**
	 * Sends SysEx message
	 */
	void sysExMessage(const byte *&data) override;
public:
	/**
	 * Constructor
	 */
	SoundDriverMT32();

	/**
	 * Destructor
	 */
	~SoundDriverMT32() override;

	/**
	 * Starts a special effect playing
	 */
	void playFX(uint effectId, const byte *data) override;

	/**
	 * Plays a song
	 */
	void playSong(const byte *data) override;

	/**
	 * Executes special music command
	 */
	int songCommand(uint commandId, byte musicVolume = 0, byte sfxVolume = 0) override;

	/**
	 * Timer function for MT32
	 */
	void onTimer();
};

} // namespace Xeen
} // namespace Shared
} // namespace MM

#endif
