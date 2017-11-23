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

#ifndef XEEN_MUSIC_H
#define XEEN_MUSIC_H

#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/stack.h"
#include "xeen/files.h"

#define CHANNEL_COUNT 9

namespace OPL {
	class OPL;
}

namespace Xeen {

enum MusicCommand {
	STOP_SONG = 0, RESTART_SONG = 1, SET_VOLUME = 0x100,
	GET_STATUS = 0xFFE0
};

class MusicDriver;

typedef bool (MusicDriver::*CommandFn)(const byte *&srcP, byte param);

/**
 * Base class for music drivers
 */
class MusicDriver {
protected:
	struct Subroutine {
		const byte *_returnP;
		const byte *_jumpP;
		Subroutine() : _returnP(nullptr), _jumpP(nullptr) {}
		Subroutine(const byte *returnP, const byte *endP) :
			_returnP(returnP), _jumpP(endP) {}
	};
	struct Channel {
		bool _changeFrequency;
		int _freqCtrChange;
		int _freqChange;
		int _freqCtr;
		byte _volume;
		byte _scalingValue;
		uint _frequency;
		Channel() : _changeFrequency(false), _freqCtr(0), _freqCtrChange(0),
			_freqChange(0), _volume(0), _scalingValue(0), _frequency(0) {}
	};
private:
	static const CommandFn FX_COMMANDS[16];
	static const CommandFn MUSIC_COMMANDS[16];
private:
	Common::Stack<Subroutine> _musSubroutines, _fxSubroutines;
	int _musCountdownTimer;
	int _fxCountdownTimer;
	const byte *_fxDataPtr, *_musDataPtr;
	const byte *_fxStartPtr;
	const byte *_musStartPtr;
	uint _frameCtr;
private:
	/**
	 * Executes the next command
	 * @param srcP		Command data pointer
	 * @returns		If true, execution of commands for the current timer call stops
	 */
	bool command(const byte *&srcP);
protected:
	Common::Array<Channel> _channels;
	bool _exclude7;
	bool _musicPlaying;
	bool _fxPlaying;
protected:
	/**
	 * Executes a series of commands until instructed to stop
	 */
	void execute();

	// Music commands (with some also used by FX)
	virtual bool musCallSubroutine(const byte *&srcP, byte param);
	virtual bool musSetCountdown(const byte *&srcP, byte param);
	virtual bool musSetInstrument(const byte *&srcP, byte param) = 0;
	virtual bool cmdNoOperation(const byte *&srcP, byte param);
	virtual bool musSetPitchWheel(const byte *&srcP, byte param) = 0;
	virtual bool musSkipWord(const byte *&srcP, byte param);
	virtual bool musSetPanning(const byte *&srcP, byte param) = 0;
	virtual bool musFade(const byte *&srcP, byte param) = 0;
	virtual bool musStartNote(const byte *&srcP, byte param) = 0;
	virtual bool musSetVolume(const byte *&srcP, byte param) = 0;
	virtual bool musInjectMidi(const byte *&srcP, byte param) = 0;
	virtual bool musPlayInstrument(const byte *&srcP, byte param) = 0;
	virtual bool cmdFreezeFrequency(const byte *&srcP, byte param);
	virtual bool cmdChangeFrequency(const byte *&srcP, byte param);
	virtual bool musEndSubroutine(const byte *&srcP, byte param);

	// FX commands
	virtual bool fxCallSubroutine(const byte *&srcP, byte param);
	virtual bool fxSetCountdown(const byte *&srcP, byte param);
	virtual bool fxSetInstrument(const byte *&srcP, byte param) = 0;
	virtual bool fxSetVolume(const byte *&srcP, byte param) = 0;
	virtual bool fxMidiReset(const byte *&srcP, byte param) = 0;
	virtual bool fxMidiDword(const byte *&srcP, byte param) = 0;
	virtual bool fxSetPanning(const byte *&srcP, byte param) = 0;
	virtual bool fxChannelOff(const byte *&srcP, byte param) = 0;
	virtual bool fxFade(const byte *&srcP, byte param) = 0;
	virtual bool fxStartNote(const byte *&srcP, byte param) = 0;
	virtual bool fxInjectMidi(const byte *&srcP, byte param) = 0;
	virtual bool fxPlayInstrument(const byte *&srcP, byte param) = 0;
	virtual bool fxEndSubroutine(const byte *&srcP, byte param);

	/**
	 * Post-processing done when a pause countdown starts or is in progress
	 */
	virtual void pausePostProcess() = 0;

	/**
	 * Does a reset of any sound effect
	 */
	virtual void resetFX() = 0;
public:
	/**
	 * Constructor
	 */
	MusicDriver();

	/**
	 * Destructor
	 */
	virtual ~MusicDriver();

	/**
	 * Starts an special effect playing
	 */
	virtual void playFX(uint effectId, const byte *data);

	/**
	 * Stop any playing FX
	 */
	void stopFX();

	/**
	 * Plays a song
	 */
	virtual void playSong(const byte *data);

	/**
	 * Executes special music command
	 */
	virtual int songCommand(uint commandId, byte volume = 0);
};

class AdlibMusicDriver : public MusicDriver {
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
	int _volume;
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
	void playInstrument(byte channelNum, const byte *data);
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
	AdlibMusicDriver();

	/**
	 * Destructor
	 */
	virtual ~AdlibMusicDriver();

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
	virtual int songCommand(uint commandId, byte volume = 0);
};

class Music {
private:
	MusicDriver *_musicDriver;
	const byte *_effectsData;
	Common::Array<uint16> _effectsOffsets;
	const byte *_songData;
	ArchiveType _archiveType;
private:
	/**
	 * Loads effects data that was embedded in the music driver
	 */
	void loadEffectsData();

	/**
	 * Updates any playing music
	 */
	void update();
public:
	bool _musicOn;
	Common::String _currentMusic, _priorMusic;
public:
	Music();
	~Music();

	/**
	 * Starts an effect playing
	 */
	void playFX(uint effectId);

	/**
	 * Stops any currently playing FX
	 */
	void stopFX();

	/**
	 * Executes special music command
	 */
	int songCommand(uint commandId, byte volume = 0);

	/**
	 * Stops any currently playing music
	 */
	void stopSong() { songCommand(STOP_SONG); }

	/**
	 * Restart a previously playing song (which must still be loaded)
	 */
	void restartSong() { songCommand(RESTART_SONG); }

	/**
	 * Sets the music volume
	 */
	void setMusicVolume(byte volume) { songCommand(SET_VOLUME, volume); }

	/**
	 * Plays a song
	 */
	void playSong(Common::SeekableReadStream &stream);

	/**
	 * Plays a song
	 */
	void playSong(const Common::String &name, int param = 0);

	/**
	 * Plays a song
	 */
	void playSong(const byte *data) {
		_musicDriver->playSong(data);
	}
};

} // End of namespace Xeen

#endif /* XEEN_MUSIC_H */
