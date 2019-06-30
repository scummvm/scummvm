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

#ifndef XEEN_SOUND_DRIVER_ADLIB_H
#define XEEN_SOUND_DRIVER_ADLIB_H

#include "xeen/sound_driver.h"

namespace OPL {
	class OPL;
}

namespace Xeen {

class SoundDriverAdlib : public SoundDriver {
	struct RegisterValue {
		uint8 _regNum;
		uint8 _value;

		RegisterValue(int regNum, int value) {
			_regNum = regNum; _value = value;
		}
	};
private:
	static const byte OPERATOR1_INDEXES[CHANNEL_COUNT];
	static const byte OPERATOR2_INDEXES[CHANNEL_COUNT];
	static const uint WAVEFORMS[24];
private:
	OPL::OPL *_opl;
	Common::Queue<RegisterValue> _queue;
	Common::Mutex _driverMutex;
	const byte *_musInstrumentPtrs[16];
	const byte *_fxInstrumentPtrs[16];
	int _field180;
	int _field181;
	int _field182;
	int _musicVolume, _sfxVolume;
private:
	/**
	 * Initializes the state of the Adlib OPL driver
	 */
	void initialize();

	/**
	 * Adds a register write to the pending queue that will be flushed
	 * out to the OPL on the next timer call
	 */
	void write(int reg, int val);

	/**
	 * Timer function for OPL
	 */
	void onTimer();

	/**
	 * Flushes any pending writes to the OPL
	 */
	void flush();

	/**
	 * Resets all the output frequencies
	 */
	void resetFrequencies();

	/**
	 * Sets the frequency for an operator
	 */
	void setFrequency(byte operatorNum, uint frequency);

	/**
	 * Calculates the frequency for a note
	 */
	uint calcFrequency(byte note);

	/**
	 * Sets the output level for a channel
	 */
	void setOutputLevel(byte channelNum, uint level);

	/**
	 * Starts playing an instrument
	 */
	void playInstrument(byte channelNum, const byte *data, bool isFx);

	/**
	 * Calculates the scaling/volume level to output based on sfx or music master volume
	 */
	byte calculateLevel(byte level, bool isFx);
protected:
	virtual bool musSetInstrument(const byte *&srcP, byte param);
	virtual bool musSetPitchWheel(const byte *&srcP, byte param);
	virtual bool musSetPanning(const byte *&srcP, byte param);
	virtual bool musFade(const byte *&srcP, byte param);
	virtual bool musStartNote(const byte *&srcP, byte param);
	virtual bool musSetVolume(const byte *&srcP, byte param);
	virtual bool musInjectMidi(const byte *&srcP, byte param);
	virtual bool musPlayInstrument(const byte *&srcP, byte param);

	virtual bool fxSetInstrument(const byte *&srcP, byte param);
	virtual bool fxSetVolume(const byte *&srcP, byte param);
	virtual bool fxMidiReset(const byte *&srcP, byte param);
	virtual bool fxMidiDword(const byte *&srcP, byte param);
	virtual bool fxSetPanning(const byte *&srcP, byte param);
	virtual bool fxChannelOff(const byte *&srcP, byte param);
	virtual bool fxFade(const byte *&srcP, byte param);
	virtual bool fxStartNote(const byte *&srcP, byte param);
	virtual bool fxInjectMidi(const byte *&srcP, byte param);
	virtual bool fxPlayInstrument(const byte *&srcP, byte param);

	/**
	 * Post-processing done when a pause countdown starts or is in progress
	 */
	virtual void pausePostProcess();

	/**
	 * Does a reset of any sound effect
	 */
	virtual void resetFX();
public:
	/**
	 * Constructor
	 */
	SoundDriverAdlib();

	/**
	 * Destructor
	 */
	virtual ~SoundDriverAdlib();

	/**
	 * Starts an special effect playing
	 */
	virtual void playFX(uint effectId, const byte *data);

	/**
	 * Plays a song
	 */
	virtual void playSong(const byte *data);

	/**
	 * Executes special music command
	 */
	virtual int songCommand(uint commandId, byte musicVolume = 0, byte sfxVolume = 0);
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_DRIVER_H */
