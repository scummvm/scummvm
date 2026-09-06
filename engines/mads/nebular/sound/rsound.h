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
// The number of active notes registered per channel.
// The original code used 4, but some chord events contain more notes and
// overwrite in the active notes for the next channel.
#define RSOUND_ACTIVE_NOTES_COUNT 8

// Mode for fade-out to stop checks.
// kRSoundFadeCheckAlternating: fixed speed of 2 (every other tick)
// kRSoundFadeCheckProgrammable: speed is programmable using setFadeOutSpeed
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
	byte _midiChannel = 0;					// 1-9: the MIDI channel to which the data in this struct pertains

	byte _deltaCounter = 0;					// number of ticks until the next event occurs; loaded from the delta byte of a note or chord event
											// 0: channel is not active
	int8 _pitchSlideStepSize = 0;			// delta added to _pitchBend each pitch-slide step; 0: pitch slide is not active
	int8 _volumeFadeStepSize = 0;			// delta added to _volume each volume fade step; 0: volume fade is not active
	int8 _panningSweepStepSize = 0;			// delta added to _panning each panning sweep step; 0: panning sweep is not active
	byte _note = 0;							// MIDI note number, read from the note or chord event
	byte _program = 0;						// patch/instrument number, sent as a Program Change
	byte _velocity = 0;						// note velocity, used by RSound::sendNoteOn()
	int8 _noteDurationOffset = 0;			// subtracted from the event delta to derive the note duration; positive offset: note is turned off
											// before the next event is processed. Data might only use up to -1 in the negative direction.
	byte _noteDurationCounter = 0;			// number of ticks until the currently active note(s) is/are turned off
	byte _volumeFadeCounter = 0;			// number of ticks until the next volume fade step is processed
	byte _pitchSlideCounter = 0;			// number of ticks until the next pitch slide step is processed
	byte _panningSweepCounter = 0;			// number of ticks until the next panning sweep step is processed
	byte _volume = 0;						// current channel volume (MIDI CC#7)
	byte _pitchBend = 0;					// current pitch bend value (status 0xEn, coarse/MSB only); 0x40 = center
	byte _panning = 0;						// current pan value (MIDI CC#10); 0x40 = center
	byte _volumeFadeSpeed = 0;				// number of ticks between volume fade steps
	byte _pitchSlideSpeed = 0;				// number of ticks between pitch slide steps
	byte _panningSweepSpeed = 0;			// number of ticks between panning sweep steps
	byte _pitchSlideDurationCounter = 0;	// number of ticks until the pitch slide ends
	bool _fadeOutActive = false;			// true while the channel is fading out to silence (not to be confused with a volume fade)
	byte *_soundDataStart = nullptr;		// start of the sound data stream playing on this channel
	byte *_pSrc = nullptr;					// current read pointer into the sound data stream
	byte *_innerLoopStart = nullptr;		// inner loop restart address
	byte *_outerLoopStart = nullptr;		// outer loop restart address
	byte _innerLoopCounter = 0;				// number of repeats of the inner loop remaining
	byte _outerLoopCounter = 0;				// number of repeats of the outer loop remaining
	byte *_soundData = nullptr;				// identifies the sound data played by this channel; effectively the same as _soundDataStart

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
	// running-status cache, avoids resending an unchanged status byte
	// Note that the ScummVM MIDI drivers do not use this; they always send the status byte
	byte _runningStatus;
	// The note(s) currently playing on each MIDI channel (index 0 unused; channels are 1-9)
	byte _activeNotes[RSOUND_CHANNEL_COUNT + 1][RSOUND_ACTIVE_NOTES_COUNT];

	RSoundFadeCheckMode _fadeOutCheckMode;
	int _fadeOutCounter;
	int _fadeOutSpeed;

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

	void processTick();
	void processTickAllChannels();
	/**
	 * Brief description of the event loop:
	 * This function reads an opcode byte plus a number of data bytes (depending on the operation)
	 * from the sound data stream. Opcodes 0x00 - 0x7F are note events, playing the MIDI note
	 * indicated by the opcode. Opcodes 0xF1-0xFF do things like setting volume, program change
	 * and starting a panning sweep. Opcode 0xF5 plays a chord i.e. multiple notes at the same
	 * time (up to 4). The note events and the chord event are the only events that specify a
	 * delta as (one of) the data byte(s). This delta is the number of ticks until the next event
	 * should be processed. The other events are processed one after the other in the same tick,
	 * until an event with a delta is encountered.
	 * The note and chord events also have a duration, which is the number of ticks until the
	 * played note(s) is/are turned off. By default this duration is equal to the event delta,
	 * but this can be changed by setting the note duration offset (opcode 0xFB). This offset is
	 * subtracted from the event delta to determine the note duration. So a positive offset will
	 * cause notes to end before the next event is processed, while a negative offset will overlap
	 * the notes with the next events. The MIDI convention of pairing note on events with note off
	 * events is not used, but opcode 0x00 can be used to turn off all active notes.
	 * Specifying an event delta of 0 will stop playback. Because of this, when starting playback
	 * of sound data for a channel, a positive delta (usually 1) must be set on the channel.
	 */
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
	 * Loads new sound data into the channel, starts playback by setting
	 * _deltaCounter to 1 and resets pitch bend to center on the MT-32.
	 */
	void Channel_playData(Channel *channel, byte *soundData);

	/**
	 * Sends Note-Off (velocity 0) for all active notes on the
	 * given channel's MIDI channel, then clears the active note table for it.
	 */
	void Channel_turnOffActiveNotes(Channel *channel);

protected:
	int _commandParam;

	// Specifies the lowest channel in the channel range that is used for
	// dynamic channel allocation. F.e. if this is 5, channels 1-4 are
	// initialized and stopped by the static commands 2 and 3, and channels
	// 5-8 are initialized and stopped by the dyanmic commands 4 and 5. Also,
	// channels 5-8 are used by playSoundDynamic.
	// Channel 9 can be included with the static or dynamic channels by setting
	// the corresponding boolean field. This will only affect commands 2-5;
	// channel 9 is never used by playSoundDynamic.
	byte _dynamicStartChannel;
	bool _staticIncludeChannel9;
	bool _dynamicIncludeChannel9;

	void setFadeOutSpeed(int fadeOutSpeed);

	/**
	 * Clear the active and fade state for MIDI channels in [first, last].
	 * Specify includeChannel9 to also reset MIDI channel 9.
	 */
	void resetChannelRange(int firstChannel, int lastChannel, bool includeChannel9 = false);

	/** Reset the per-MIDI-channel active note tracking table to empty. */
	void clearActiveNotes();

	/** Reset active note slots for the inclusive MIDI-channel range. */
	void clearActiveNotesRange(int firstChannel, int lastChannel);

	byte *loadData(int offset);

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
	 * Returns a pointer to the channel data corresponding to the
	 * specified MIDI channel.
	 */
	Channel *getChannel(byte channel);

	/*
	 * MIDI channels are allocated to sound data either statically or
	 * dynamically. Channels 1-4 and 9 are usually statically allocated,
	 * where the command will specify the channel to use for loading the
	 * sound data using playSoundStatic. Because channel 9 is the rhythm
	 * channel and sound data must be specifically written for this
	 * channel, it can only be allocated statically.
	 * Channels 5-8 are usually dynamically allocated. The command will
	 * use playSoundDynamic to look for a free channel and then load the
	 * sound data into that channel.
	 * Usually, commands will load music statically into channels 1-4 and 9,
	 * and SFX dynamically into channels 5-8. However, there are many
	 * exceptions.
	 */

	/**
	 * Play the specified sound by statically allocating the specified
	 * channel. This will unload any sound already playing on the
	 * channel.
	 */
	Channel *playSoundStatic(int offset, byte channel);
	Channel *playSoundStatic(byte *soundData, byte channel);

	/**
	 * Play the specified sound by allocating one of the free dynamic
	 * channels. Returns the channel that was used (or nullptr if none
	 * was free), since some commands poke the just-loaded channel's
	 * loop pointer directly afterward.
	 */
	Channel *playSoundDynamic(int offset);

	/**
	 * Play the specified sound by allocating any free melodic channel,
	 * dynamic or static.
	 */
	Channel *playSoundAnyChannel(int offset);

	/**
	 * Dynamically allocates a melodic channel, loads the specified sound
	 * data into it and starts playback. Allocation will look for a
	 * free channel in the range starting with the specified channel
	 * (default 5) and ending with channel 8. If no suitable channel
	 * could be found, nullptr is returned and the sound data is not
	 * played.
	 */
	Channel *allocateAndPlay(byte *pData, int startingChannel);

	/**
	 * Checks to see whether the given block of data is already loaded
	 * into a channel and being played.
	 */
	bool isSoundPlaying(int offset);
	bool isSoundPlaying(byte *pData);

	/**
	 * Checks to see whether the given block of data is already loaded
	 * into the specified channel and being played.
	 */
	bool isSoundPlaying(byte channel, byte *pData);

	uint16 generateRandomNumber();

	// ---- Low-level MIDI send helpers -------------------------------
	// All send through the ScummVM MT-32 / General MIDI driver.
	void sendNoteOn(byte midiChannel, byte note, byte velocity);
	void sendProgramChange(byte midiChannel, byte program);
	void sendVolume(byte midiChannel, byte volume);
	void sendPitchBend(byte midiChannel, byte value);
	void sendPanning(byte midiChannel, byte value);
	void muteChannel(byte midiChannel);
	void unmuteChannel(byte midiChannel, byte volume);

	/**
	 * Resets the MIDI channel state (all notes off, reset all
	 * controllers, volume=100, pan=center) of MIDI channels [first, last]
	 * (inclusive, 1-based). Shared tail used by command0/command2/command4.
	 */
	void sendMidiChannelReset(int first, int last, bool includeChannel9 = false);

	/**
	 * Sets the volume for a channel. The value is set on the channel data
	 * and it is sent out to the MT-32.
	 */
	void setChannelVolume(byte channel, byte volume);

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

	/**
	 * Stop all notes playing on the device. Used when pausing or
	 * quitting the engine.
	 */
	void stopAllNotes() override;

	virtual int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int nullCommand();

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
	int getTicksSinceLastCommand();
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
