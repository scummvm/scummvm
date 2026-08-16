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

#ifndef MADS_NEBULAR_SOUND_RSOUND_H
#define MADS_NEBULAR_SOUND_RSOUND_H

#include "mads/core/sound_manager.h"
#include "mads/core/native_sound_timer.h"

#include "audio/mt32gm.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

class RSound;

#define RSOUND_CHANNEL_COUNT 9

enum RSoundFadeCheckMode {
	kRSoundFadeCheckAlternating,
	kRSoundFadeCheckProgrammable
};

/**
 * Represents the data for a channel on the Roland MT-32 / MPU-401 driver.
 * Ported from the Channel struct identified in rsound.009's disassembly;
 * field names/roles were derived by tracing Channel_processTick() (the
 * per-channel opcode interpreter) and cross-referencing against the
 * equivalent AdlibChannel fields in asound.h.
 * Note: fields have been renamed here; this will be done in asound.* too.
 *
 * Confirmed against the real DOS struct layout (IDA struct dump,
 * sizeof=0x22): every field below from _deltaCounter through _soundData
 * matches the original both in name and in byte offset/order exactly:
 *   0x00 _deltaCounter			0x0C _volume					0x18 _innerLoopStart
 *   0x01 _pitchSlideStepSize	0x0D _pitchBend					0x1A _outerLoopStart
 *   0x02 _volumeFadeStepSize	0x0E _panning					0x1C _innerLoopCounter
 *   0x03 _panningSweepStepSize	0x0F _volumeFadeSpeed			0x1E _outerLoopCounter
 *   0x04 _note					0x10 _pitchSlideSpeed			0x20 _soundData
 *   0x05 _program				0x11 _panningSweepSpeed
 *   0x06 _velocity				0x12 _pitchSlideDurationCounter
 *   0x07 _noteDurationOffset	0x13 _fadeOutActive
 *   0x08 _noteDurationCounter	0x14 _soundDataStart
 *   0x09 _volumeFadeCounter	0x16 _pSrc
 *   0x0A _pitchSlideCounter
 *   0x0B _panningSweepCounter
 * _owner and _midiChannel below are NOT part of the original struct (it
 * has no equivalent fields) - they're C++-side conveniences so Channel
 * methods and callers don't need the MIDI channel number (array index+1)
 * threaded through separately. Any future raw "[bx+N]" disassembly offset
 * can be mapped directly via the table above.
 */
class Channel {
public:
	RSound *_owner = nullptr;
	int _midiChannel = 0;				// 1-9: the MIDI channel to which the data in this struct pertains

	int _deltaCounter = 0;				// number of ticks until the next event occurs; loaded from the delta byte of a note or chord event
										// 0: channel is not active
	int _pitchSlideStepSize = 0;		// delta added to _pitchBend each pitch-slide step; 0: pitch slide is not active
	int _volumeFadeStepSize = 0;		// delta added to _volume each volume fade step; 0: volume fade is not active
	int _panningSweepStepSize = 0;		// delta added to _panning each panning sweep step; 0: panning sweep is not active
	int _note = 0;						// MIDI note number, read from the note or chord event
	int _program = 0;					// patch/instrument number, sent as a Program Change
	int _velocity = 0;					// note velocity, used by RSound::sendNoteOn()
	int _noteDurationOffset = 0;		// subtracted from the event delta to derive the note duration; positive offset: note is turned off
										// before the next event is processed. Data might only use up to -1 in the negative direction.
	int _noteDurationCounter = 0;		// number of ticks until the currently active note(s) is/are turned off
	int _volumeFadeCounter = 0;			// number of ticks until the next volume fade step is processed
	int _pitchSlideCounter = 0;			// number of ticks until the next pitch slide step is processed
	int _panningSweepCounter = 0;		// number of ticks until the next panning sweep step is processed
	int _volume = 0;					// current channel volume (MIDI CC#7)
	int _pitchBend = 0;					// current pitch bend value (status 0xEn, coarse/MSB only); 0x40 = center
	int _panning = 0;					// current pan value (MIDI CC#10); 0x40 = center
	int _volumeFadeSpeed = 0;			// number of ticks between volume fade steps
	int _pitchSlideSpeed = 0;			// number of ticks between pitch slide steps
	int _panningSweepSpeed = 0;			// number of ticks between panning sweep steps
	int _pitchSlideDurationCounter = 0;	// number of ticks until the pitch slide ends
	bool _fadeOutActive = false;		// true while the channel is fading out to silence (not to be confused with a volume fade)
	byte *_soundDataStart = nullptr;	// start of the sound data stream playing on this channel
	byte *_pSrc = nullptr;				// current read pointer into the sound data stream
	byte *_innerLoopStart = nullptr;	// inner loop restart address
	byte *_outerLoopStart = nullptr;	// outer loop restart address
	int _innerLoopCounter = 0;			// number of repeats of the inner loop remaining
	int _outerLoopCounter = 0;			// number of repeats of the outer loop remaining
	byte *_soundData = nullptr;			// identifies the sound data played by this channel; effectively the same as _soundDataStart

public:
	Channel() {}

	/**
	 * Resets most of the channel data to its default values and loads
	 * the specified sound data into the channel. Deliberately does NOT
	 * touch volume, program, velocity, key-on state or delta/duration -
	 * matches the original disassembly.
	 */
	void loadData(byte *soundData);

	/**
	 * Marks the channel as fading out (fading toward silence) and stopping
	 * playback when the fade-out is complete. Not to be confused with a
	 * volume fade triggered by event 0xF8.
	 */
	void setFadeOut(bool fadeOut);

	/**
	 * Loads new sound data into the channel, starts playback by setting
	 * _deltaCounter to 1 and resets pitch bend to center on the MT-32.
	 */
	void playData(byte *soundData);
};

/**
 * Base class for the Roland MT-32 / MPU-401 sound player resource files.
 * Mirrors the structure of ASound (the Adlib equivalent in asound.h), but
 * for a driver family that sends real MIDI messages instead of poking
 * OPL registers.
 */
class RSound : public SoundDriver {
	friend class Channel;
private:
	uint16 _randomSeed;
	int _masterVolume;
	byte _runningStatus;							// running-status cache, avoids resending an unchanged status byte
													// Note that the ScummVM MIDI drivers do not use this; they always send the status byte
	byte _activeNotes[RSOUND_CHANNEL_COUNT + 1][4];	// The note(s) currently playing on each MIDI channel (index 0 unused; channels are 1-9)
	RSoundFadeCheckMode _fadeCheckMode;
	bool _fadeCheckAlternate;
	int _fadeCheckCounter;
	int _fadeCheckPeriod;

	/**
	 * Data-segment offset of this driver's own "command0_array" (the
	 * MT-32 title-display + patch-init SysEx table sent by command0()).
	 * Each driver has its own copy of this table at its own offset
	 * within its own resource file. The table's contents differs per
	 * driver beyond a shared prefix, so it can't be hardcoded once;
	 * parameterizing the offset via the constructor avoids needing a
	 * command0() override in every derived class.
	 */
	int _sysExOffset;

	MidiDriver_MT32GM *_midiDriver;
	uint32 _driverCallbackDelta;

	void processTick();
	void processTickAllChannels();
	void Channel_processTick(Channel *channel);

	/**
	 * Resets all 9 channels and the held-notes table.
	 */
	void resetAllChannels();

	/**
	 * Process channels fading out to stop. Not to be confused with the
	 * volume fade event 0xF8.
	 * Sections 1, 2, and 9 and both demo overlays use a fixed
	 * every-other-poll toggle. Sections 3-8 use a programmable
	 * countdown; zero disables it and the counter reloads after each pass.
	 */
	void processChannelFadeOuts();
	void Channel_processFadeOut(Channel *channel);

	/**
	 * Sends Note-Off (velocity 0) for all active notes on the
	 * given channel's MIDI channel, then clears the active note table for it.
	 */
	void Channel_turnOffActiveNotes(Channel *channel);

protected:
	int _commandParam;

	void setFadeCheckPeriod(int period) {
		if (_fadeCheckMode == kRSoundFadeCheckProgrammable)
			_fadeCheckPeriod = period;
	}

	/**
	 * Clear the active and fade state for MIDI channels in [first, last].
	 * Specify includeChannel9 to also reset MIDI channel 9.
	 */
	void resetChannelRange(int firstChannel, int lastChannel, bool includeChannel9 = false);

	/** Reset the per-MIDI-channel active note tracking table to empty. */
	void clearActiveNotes();

	/** Reset active note slots for the inclusive MIDI-channel range. */
	void clearActiveNotesRange(int firstChannel, int lastChannel);

	byte *loadData(int offset) {
		return &_soundData[offset];
	}

	/**
	 * Hook called once per processTick() frame, immediately after the disabled
	 * check and before channel polling. Only RSound9's driver data makes
	 * use of a recurring deferred-callback timer (g_callbackCounter/
	 * g_callbackPeriod/_soundPtr in the original disassembly, mirroring
	 * the identical mechanism in ASound9); every other driver leaves
	 * this as a no-op.
	 */
	virtual void tickCallback() {
	}

	void resultCheck();

	/**
	 * Play the specified sound using any free channel from 5 to 8.
	 * Returns the channel that was used (or nullptr if none was free),
	 * since some commands poke the just-loaded channel's loop pointer
	 * directly afterward.
	 */
	Channel *playSoundCh5To8(int offset);

	/**
	 * Play the specified sound using any free channel from 1 to 8.
	 */
	Channel *playSoundCh1To8(int offset);

	/**
	 * Allocates a MIDI channel, loads the specified sound data into it
	 * and starts playback. Allocation will look for a free channel in
	 * the range starting with the specified channel (default 5) and
	 * ending with channel 8. If no suitable channel could be found,
	 * nullptr is returned and the sound data is not played.
	 */
	Channel *allocateAndPlay(byte *pData, int startingChannel = 5);

	/**
	 * Checks to see whether the given block of data is already loaded into a channel.
	 */
	bool isSoundPlaying(byte *pData);

	int generateRandomNumber();

	// ---- Low-level MIDI send helpers -------------------------------
	// All send through the ScummVM MT-32 / General MIDI driver.
	void sendNoteOn(int midiChannel, int note, int velocity);
	void sendProgramChange(int midiChannel, int program);
	void sendVolume(int midiChannel, int volume);
	void sendPitchBend(int midiChannel, int value);
	void sendPanning(int midiChannel, int value);
	void muteChannel(int midiChannel);
	void unmuteChannel(int midiChannel, int volume);

	/**
	 * Resets the MIDI channel state (all notes off, reset all
	 * controllers, volume=100, pan=center) of MIDI channels [first, last]
	 * (inclusive, 1-based). Shared tail used by command0/command2/command4.
	 */
	void sendMidiChannelReset(int first, int last);

	/**
	 * Sends a single SysEx message: bytes from pData up to (but not
	 * including) a 0xFF terminator, via the MT32GM MIDI driver. Returns a
	 * pointer to the terminating byte, so callers walking a sequence of
	 * consecutive messages can advance past it. Returns nullptr when no
	 * terminator occurs within maxLength bytes.
	 */
	byte *sendSysExData(byte *pData, uint maxLength);

	/** sendSysExData() for a block already in this driver's own loaded sound data. */
	byte *sendSysEx(int offset);

	/**
	 * Matches sendSysExSequence: repeatedly calls sendSysEx(), starting
	 * from this driver's own command0_array (_sysExOffset) and advancing
	 * past each message's terminating 0xFF to the start of the next one,
	 * until an empty message (two consecutive 0xFF bytes) marks the end
	 * of the table. Called once from the constructor (matching
	 * initDeviceOnce) - the disassembly's _deviceInitialized guard flag
	 * isn't needed since nothing else ever calls this again.
	 */
	void sendSysExSequence();

	virtual int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int nullCommand() {
		return 0;
	}

public:
	Channel _channels[RSOUND_CHANNEL_COUNT];
	int _ticksSinceLastCommand;
	bool _ticksProcessingDisabled;
	bool _mute;
	int _pollResult;
	int _resultFlag;

public:
	static void validate(bool isDemo);

public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param midiDriver	MIDI driver instance used for MIDI message output
	 * @param filename		Specifies the Roland sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 * @param dataSize		Size of the data segment
	 * @param sysExOffset	Offset of this driver's own command0_array
	 * @param fadeCheckMode Native pending-stop fade scheduler
	 */
	RSound(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset, RSoundFadeCheckMode fadeCheckMode);

	~RSound() override;

	int stop() override;
	int poll() override;
	void noise() override {
		// No equivalent in the Roland driver - noise() is an Adlib/OPL-only concept
	}
	void setVolume(int volume) override;

	int getTicksSinceLastCommand() {
		return _ticksSinceLastCommand;
	}
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
