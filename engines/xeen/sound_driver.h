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

#ifndef XEEN_SOUND_DRIVER_H
#define XEEN_SOUND_DRIVER_H

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
	STOP_SONG = 0, RESTART_SONG = 1, SET_VOLUME = 0x100, GET_STATUS = 0xFFE0
};

class SoundDriver;

typedef bool (SoundDriver::*CommandFn)(const byte *&srcP, byte param);

/**
 * Base class for sound drivers
 */
class SoundDriver {
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
		byte _totalLevel;
		bool _isFx;
		uint _frequency;
		Channel() : _changeFrequency(false), _freqCtr(0), _freqCtrChange(0),
			_freqChange(0), _volume(0), _totalLevel(0), _frequency(0), _isFx(false) {}
	};
	enum StreamType {
		stMUSIC,
		stFX,

		stLAST
	};
	class Stream {
	public:
		Stream() {}
		Stream(const CommandFn *commands) : _playing(false), _countdownTimer(0), _dataPtr(nullptr), _startPtr(nullptr), _commands(commands) {}

		bool _playing;
		int _countdownTimer;
		const byte *_dataPtr;
		const byte *_startPtr;
		const CommandFn *_commands;
	};

private:
	static const CommandFn FX_COMMANDS[16];
	static const CommandFn MUSIC_COMMANDS[16];
private:
	Common::Stack<Subroutine> _musSubroutines, _fxSubroutines;
	uint _frameCtr;
private:
	/**
	 * Executes the next command
	 * @param srcP		Command data pointer
	 * @returns		If true, execution of commands for the current timer call stops
	 */
	bool command(const byte *&srcP);

	Stream *tickStream();
protected:
	Common::Array<Channel> _channels;
	Stream _streams[stLAST];

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
	SoundDriver();

	/**
	 * Destructor
	 */
	virtual ~SoundDriver();

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
	virtual int songCommand(uint commandId, byte musicVolume = 0, byte sfxVolume = 0);

	/**
	 * Returns whether music is currently playing
	 */
	bool isPlaying() const { return _streams[stMUSIC]._playing; }
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_DRIVER_H */
