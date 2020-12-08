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

/// \brief Declarations related to the MidiParser class

#ifndef AUDIO_MIDIPARSER_H
#define AUDIO_MIDIPARSER_H

#include "common/scummsys.h"
#include "common/endian.h"

class MidiDriver_BASE;

/**
 * @defgroup audio_midiparser MIDI parser
 * @ingroup audio
 *
 * @brief A framework and common functionality for parsing event-based music streams.
 * @{
 */


//////////////////////////////////////////////////
//
// Support entities
//
//////////////////////////////////////////////////

/**
 * Maintains time and position state within a MIDI stream.
 * A single Tracker struct is used by MidiParser to keep track
 * of its current position in the MIDI stream. The Tracker
 * struct, however, allows alternative locations to be cached.
 * See MidiParser::jumpToTick() for an example of tracking
 * multiple locations within a MIDI stream. NOTE: It is
 * important to also maintain pre-parsed EventInfo data for
 * each Tracker location.
 */
struct Tracker {
	byte * _playPos;        ///< A pointer to the next event to be parsed
	uint32 _playTime;       ///< Current time in microseconds; may be in between event times
	uint32 _playTick;       ///< Current MIDI tick; may be in between event ticks
	uint32 _lastEventTime; ///< The time, in microseconds, of the last event that was parsed
	uint32 _lastEventTick; ///< The tick at which the last parsed event occurs
	byte   _runningStatus;  ///< Cached MIDI command, for MIDI streams that rely on implied event codes

	Tracker() { clear(); }

	/// Clears all data; used by the constructor for initialization.
	void clear() {
		_playPos = 0;
		_playTime = 0;
		_playTick = 0;
		_lastEventTime = 0;
		_lastEventTick = 0;
		_runningStatus = 0;
	}
};

/**
 * Provides comprehensive information on the next event in the MIDI stream.
 * An EventInfo struct is instantiated by format-specific implementations
 * of MidiParser::parseNextEvent() each time another event is needed.
 */
struct EventInfo {
	byte * start; ///< Position in the MIDI stream where the event starts.
	              ///< For delta-based MIDI streams (e.g. SMF and XMIDI), this points to the delta.
	uint32 delta; ///< The number of ticks after the previous event that this event should occur.
	byte   event; ///< Upper 4 bits are the command code, lower 4 bits are the MIDI channel.
	              ///< For META, event == 0xFF. For SysEx, event == 0xF0.
	union {
		struct {
			byte param1; ///< The first parameter in a simple MIDI message.
			byte param2; ///< The second parameter in a simple MIDI message.
		} basic;
		struct {
			byte   type; ///< For META events, this indicates the META type.
			byte * data; ///< For META and SysEx events, this points to the start of the data.
		} ext;
	};
	uint32 length; ///< For META and SysEx blocks, this indicates the length of the data.
	               ///< For Note On events, a non-zero value indicates that no Note Off event
	               ///< will occur, and the MidiParser will have to generate one itself.
	               ///< For all other events, this value should always be zero.
	bool   loop;   ///< Indicates that this event loops (part of) the MIDI data.

	byte channel() const { return event & 0x0F; } ///< Separates the MIDI channel from the event.
	byte command() const { return event >> 4; }   ///< Separates the command code from the event.

	EventInfo() : start(0), delta(0), event(0), length(0), loop(false) { basic.param1 = 0; basic.param2 = 0; ext.type = 0; ext.data = 0; }
};

/**
 * Provides expiration tracking for hanging notes.
 * Hanging notes are used when a MIDI format does not include explicit Note Off
 * events, or when "Smart Jump" is enabled so that active notes are intelligently
 * expired when a jump occurs. The NoteTimer struct keeps track of how much
 * longer a note should remain active before being turned off.
 */
struct NoteTimer {
	byte channel;     ///< The MIDI channel on which the note was played
	byte note;        ///< The note number for the active note
	uint32 timeLeft; ///< The time, in microseconds, remaining before the note should be turned off
	NoteTimer() : channel(0), note(0), timeLeft(0) {}
};




//////////////////////////////////////////////////
//
// MidiParser declaration
//
//////////////////////////////////////////////////

/**
 * A framework and common functionality for parsing event-based music streams.
 * The MidiParser provides a framework in which to load,
 * parse and traverse event-based music data. Note the
 * avoidance of the phrase "MIDI data." Despite its name,
 * MidiParser derivatives can be used to manage a wide
 * variety of event-based music formats. It is, however,
 * based on the premise that the format in question can
 * be played in the form of specification MIDI events.
 *
 * In order to use MidiParser to parse your music format,
 * follow these steps:
 *
 * <b>STEP 1: Write a MidiParser derivative.</b>
 * The MidiParser base class provides functionality
 * considered common to the task of parsing event-based
 * music. In order to parse a particular format, create
 * a derived class that implements, at minimum, the
 * following format-specific methods:
 *   - loadMusic
 *   - parseNextEvent
 *
 * In addition to the above functions, the derived class
 * may also override the default MidiParser behavior for
 * the following methods:
 *   - resetTracking
 *   - getTick
 *   - jumpToIndex
 *   - hasJumpIndex
 *   - allNotesOff
 *   - unloadMusic
 *   - property
 *   - processEvent
 *   - onTrackStart
 *   - sendToDriver
 *   - sendMetaEventToDriver
 *   - setMidiDriver
 *
 * Please see the documentation for these individual
 * functions for more information on their use.
 *
 * The naming convention for classes derived from
 * MidiParser is MidiParser_XXX, where "XXX" is some
 * short designator for the format the class will
 * support. For instance, the MidiParser derivative
 * for parsing the Standard MIDI File format is
 * MidiParser_SMF.
 *
 * <b>STEP 2: Create an object of your derived class.</b>
 * Each MidiParser object can parse at most one (1) song
 * at a time. However, a MidiParser object can be reused
 * to play another song once it is no longer needed to
 * play whatever it was playing. In other words, MidiParser
 * objects do not have to be destroyed and recreated from
 * one song to the next.
 *
 * <b>STEP 3: Specify a MidiDriver to send events to.</b>
 * MidiParser works by sending MIDI and meta events to a
 * MidiDriver. In the simplest configuration, you can plug
 * a single MidiParser directly into the output MidiDriver
 * being used. However, you can only plug in one at a time;
 * otherwise channel conflicts will occur. Multiple parsers
 * can be used if they do not use the same channels, or if
 * they use some form of dynamic channel allocation.
 * Furthermore, meta events that may be needed to
 * interactively control music flow cannot be handled
 * because they are being sent directly to the output device.
 *
 * If you need more control over the MidiParser while it's
 * playing, you can create your own "pseudo-MidiDriver" and
 * place it in between your MidiParser and the output
 * MidiDriver. The MidiParser will send events to your
 * pseudo-MidiDriver, which in turn must send them to the
 * output MidiDriver (or do whatever special handling is
 * required). Make sure to implement all functions which
 * are necessary for proper functioning of the parser and
 * forward the calls to the real driver (even if you do not
 * want to customize the functionality).
 *
 * To specify the MidiDriver to send music output to,
 * use the MidiParser::setMidiDriver method.
 *
 * <b>STEP 4: Specify the onTimer call rate.</b>
 * MidiParser bases the timing of its parsing on an external
 * clock. Every time MidiParser::onTimer is called, a bit
 * more music is parsed. You must specify how many
 * microseconds will occur between each call to onTimer,
 * in order to ensure an accurate music tempo.
 *
 * To set the onTimer call rate, in microseconds,
 * use the MidiParser::setTimerRate method. The onTimer
 * call rate will typically match the timer rate for
 * the output MidiDriver used. This rate can be obtained
 * by calling MidiDriver::getBaseTempo.
 *
 * <b>STEP 5: Load the music.</b>
 * MidiParser requires that the music data already be loaded
 * into memory. The client code is responsible for memory
 * management on this block of memory. That means that the
 * client code must ensure that the data remain in memory
 * while the MidiParser is using it, and properly freed
 * after it is no longer needed. Some MidiParser variants may
 * require internal buffers as well; memory management for those
 * buffers is the responsibility of the MidiParser object.
 *
 * To load the music into the MidiParser, use the
 * MidiParser::loadMusic method, specifying a memory pointer
 * to the music data and the size of the data. (NOTE: Some
 * MidiParser variants don't require a size, and 0 is fine.
 * However, when writing client code to use MidiParser, it is
 * best to assume that a valid size will be required.)
 *
 * Convention requires that each implementation of
 * MidiParser::loadMusic automatically set up default tempo
 * and current track. This effectively means that the
 * MidiParser will start playing as soon as timer events
 * start coming in. If you want to start playback at a later
 * point, you can specify the mpDisableAutoStartPlayback
 * property. You can then specify the track and/or starting
 * point using setTrack, jumpToTick or jumpToIndex, and then
 * call startPlaying to start playback.
 *
 * <b>STEP 6: Activate a timer source for the MidiParser.</b>
 * The easiest timer source to use is the timer of the
 * output MidiDriver. You can attach the MidiDriver's
 * timer output directly to a MidiParser by calling
 * MidiDriver::setTimerCallback. In this case, the timer_proc
 * will be the static method MidiParser::timerCallback,
 * and timer_param will be a pointer to your MidiParser object.
 *
 * This configuration only allows one MidiParser to be driven
 * by the MidiDriver at a time. To drive more MidiParsers, you
 * will need to create a "pseudo-MidiDriver" as described earlier,
 * In such a configuration, the pseudo-MidiDriver should be set
 * as the timer recipient in MidiDriver::setTimerCallback, and
 * could then call MidiParser::onTimer for each MidiParser object.
 *
 * <b>STEP 7: Music shall begin to play!</b>
 * Congratulations! At this point everything should be hooked up
 * and the MidiParser should generate music. You can pause
 * playback and resume playing from the point you left off using
 * the pausePlaying and resumePlaying functions. (Note that MIDI
 * does not pause very well and active notes will be missing when
 * you resume playback.) You can also "pause" the MidiParser
 * simply by not sending timer events to it. You can stop
 * playback using the stopPlaying function; you can then later
 * play the track again from the start using startPlaying (or
 * select a new track first using setTrack). You can call
 * MidiParser::unloadMusic to permanently stop the music. (This
 * method resets everything and detaches the MidiParser from the
 * memory block containing the music data.)
 */
class MidiParser {
protected:
	static const uint8 MAXIMUM_TRACKS = 120;

	uint16    _activeNotes[128];   ///< Each uint16 is a bit mask for channels that have that note on.
	NoteTimer _hangingNotes[32];   ///< Maintains expiration info for up to 32 notes.
	                                ///< Used for "Smart Jump" and MIDI formats that do not include explicit Note Off events.
	byte      _hangingNotesCount; ///< Count of hanging notes, used to optimize expiration.

	MidiDriver_BASE *_driver;    ///< The device to which all events will be transmitted.
	uint32 _timerRate;     ///< The time in microseconds between onTimer() calls. Obtained from the MidiDriver.
	uint32 _ppqn;           ///< Pulses Per Quarter Note. (We refer to "pulses" as "ticks".)
	uint32 _tempo;          ///< Microseconds per quarter note.
	uint32 _psecPerTick;  ///< Microseconds per tick (_tempo / _ppqn).
	uint32 _sysExDelay;     ///< Number of microseconds until the next SysEx event can be sent.
	bool   _autoLoop;       ///< For lightweight clients that don't provide their own flow control.
	bool   _smartJump;      ///< Support smart expiration of hanging notes when jumping
	bool   _centerPitchWheelOnUnload;  ///< Center the pitch wheels when unloading a song
	bool   _sendSustainOffOnNotesOff;   ///< Send a sustain off on a notes off event, stopping hanging notes
	bool   _disableAllNotesOffMidiEvents;   ///< Don't send All Notes Off MIDI messages
	bool   _disableAutoStartPlayback;  ///< Do not automatically start playback after parsing MIDI data or setting the track
	byte  *_tracks[MAXIMUM_TRACKS];    ///< Multi-track MIDI formats are supported, up to 120 tracks.
	byte   _numTracks;     ///< Count of total tracks for multi-track MIDI formats. 1 for single-track formats.
	byte   _activeTrack;   ///< Keeps track of the currently active track, in multi-track formats.

	Tracker _position;      ///< The current time/position in the active track.
	EventInfo _nextEvent;  ///< The next event to transmit. Events are preparsed
	                        ///< so each event is parsed only once; this permits
	                        ///< simulated events in certain formats.
	bool   _abortParse;    ///< If a jump or other operation interrupts parsing, flag to abort.
	bool   _jumpingToTick; ///< True if currently inside jumpToTick
	bool   _doParse;       ///< True if the parser should be parsing; false if it should not be active
	bool   _pause;		   ///< True if the parser has paused parsing

protected:
	static uint32 readVLQ(byte * &data);
	virtual void resetTracking();
	virtual void allNotesOff();
	virtual void parseNextEvent(EventInfo &info) = 0;
	virtual bool processEvent(const EventInfo &info, bool fireEvents = true);

	void activeNote(byte channel, byte note, bool active);
	void hangingNote(byte channel, byte note, uint32 ticksLeft, bool recycle = true);
	void hangAllActiveNotes();

	/**
	 * Called before starting playback of a track.
	 * Can be implemented by subclasses if they need to
	 * perform actions at this point.
	 */
	virtual void onTrackStart(uint8 track) { };

	virtual void sendToDriver(uint32 b);
	void sendToDriver(byte status, byte firstOp, byte secondOp) {
		sendToDriver(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
	}
	virtual void sendMetaEventToDriver(byte type, byte *data, uint16 length);

	/**
	 * Platform independent BE uint32 read-and-advance.
	 * This helper function reads Big Endian 32-bit numbers
	 * from a memory pointer, at the same time advancing
	 * the pointer.
	 */
	uint32 read4high(byte * &data) {
		uint32 val = READ_BE_UINT32(data);
		data += 4;
		return val;
	}

	/**
	 * Platform independent LE uint16 read-and-advance.
	 * This helper function reads Little Endian 16-bit numbers
	 * from a memory pointer, at the same time advancing
	 * the pointer.
	 */
	uint16 read2low(byte * &data) {
		uint16 val = READ_LE_UINT16(data);
		data += 2;
		return val;
	}

public:
	/**
	 * Configuration options for MidiParser
	 * The following options can be set to modify MidiParser's
	 * behavior.
	 */
	enum {
		/**
		 * Events containing a pitch bend command should be treated as
		 * single-byte padding before the  real event. This allows the
		 * MidiParser to work with some malformed SMF files from Simon 1/2.
		 */
		mpMalformedPitchBends = 1,

		/**
		 * Sets auto-looping, which can be used by lightweight clients
		 * that don't provide their own flow control.
		 */
		mpAutoLoop = 2,

		/**
		 * Sets smart jumping, which intelligently expires notes that are
		 * active when a jump is made, rather than just cutting them off.
		 */
		mpSmartJump = 3,

		/**
		 * Center the pitch wheels when unloading music in preparation
		 * for the next piece of music.
		 */
		mpCenterPitchWheelOnUnload = 4,

		/**
		 * Sends a sustain off event when a notes off event is triggered.
		 * Stops hanging notes.
		 */
		 mpSendSustainOffOnNotesOff = 5,

		 /**
		  * Prevent sending out all notes off events on all channels when
		  * playback of a track is stopped. This option is useful when
		  * multiple sources are used; otherwise stopping playback of one
		  * source will interrupt playback of the other sources.
		  * Any active notes registered by this parser will still be turned
		  * off.
		  */
		 mpDisableAllNotesOffMidiEvents = 6,

		 /**
		  * Does not automatically start playback after parsing MIDI data
		  * or setting the track. Use startPlaying to start playback.
		  * Note that not every parser implementation might support this.
		  */
		 mpDisableAutoStartPlayback = 7
	};

public:
	typedef void (*XMidiCallbackProc)(byte eventData, void *refCon);

	MidiParser();
	virtual ~MidiParser() { stopPlaying(); }

	virtual bool loadMusic(byte *data, uint32 size) = 0;
	virtual void unloadMusic();
	virtual void property(int prop, int value);

	virtual void setMidiDriver(MidiDriver_BASE *driver) { _driver = driver; }
	void setTimerRate(uint32 rate) { _timerRate = rate; }
	void setTempo(uint32 tempo);
	void onTimer();

	bool isPlaying() const { return (_position._playPos != 0 && _doParse); }
	/**
	 * Start playback from the current position in the current track, or at
	 * the beginning if there is no current position.
	 * If the parser is already playing or there is no valid current track,
	 * this function does nothing.
	 */
	bool startPlaying();
	/**
	 * Stops playback. This resets the current playback position.
	 */
	void stopPlaying();
	/**
	 * Pauses playback and stops all active notes. Use resumePlaying to
	 * continue playback at the current track position; startPlaying will
	 * do nothing if the parser is paused.
	 * stopPlaying, unloadMusic, loadMusic and setTrack will unpause the
	 * parser. jumpToTick and jumpToIndex do nothing while the parser is
	 * paused.
	 * If the parser is not playing or already paused, this function does
	 * nothing. Note that isPlaying will continue to return true while
	 * playback is paused.
	 * Not every parser implementation might support pausing properly.
	 */
	void pausePlaying();
	/**
	 * Resumes playback at the current track position.
	 * If the parser is not paused, this function does nothing.
	 */
	void resumePlaying();

	bool setTrack(int track);
	bool jumpToTick(uint32 tick, bool fireEvents = false, bool stopNotes = true, bool dontSendNoteOn = false);
	/**
	 * Returns true if the active track has a jump point defined for the
	 * specified index number.
	 * Can be implemented for MIDI formats with support for some form of index
	 * points.
	 */
	virtual bool hasJumpIndex(uint8 index) { return false; }
	/**
	 * Stops playback and resumes it at the position defined for the specified
	 * index number.
	 * Can be implemented for MIDI formats with support for some form of index
	 * points.
	 */
	virtual bool jumpToIndex(uint8 index, bool stopNotes = true) { return false; }

	uint32 getPPQN() { return _ppqn; }
	virtual uint32 getTick() { return _position._playTick; }

	static void defaultXMidiCallback(byte eventData, void *refCon);

	static MidiParser *createParser_SMF(int8 source = -1);
	static MidiParser *createParser_XMIDI(XMidiCallbackProc proc = defaultXMidiCallback, void *refCon = 0, int source = -1);
	static MidiParser *createParser_QT();
	static void timerCallback(void *data) { ((MidiParser *) data)->onTimer(); }
};
/** @} */
#endif
