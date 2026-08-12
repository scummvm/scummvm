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
 * field names/roles were derived by tracing Channel_pollActive() (the
 * per-channel opcode interpreter) and cross-referencing against the
 * equivalent AdlibChannel fields in asound.h.
 *
 * Confirmed against the real DOS struct layout (IDA struct dump,
 * sizeof=0x22): every field below from _activeCount through _soundData
 * matches the original both in name and in byte offset/order exactly:
 *   0x00 _activeCount    0x0C _volume          0x18 _innerLoopPtr
 *   0x01 _pitchBendFadeStep 0x0D _pitchBend     0x1A _outerLoopPtr
 *   0x02 _volumeFadeStep  0x0E _pan             0x1C _innerLoopCount
 *   0x03 _panFadeStep     0x0F _volumeFadeReload 0x1E _outerLoopCount
 *   0x04 _note            0x10 _pitchBendFadeReload 0x20 _soundData
 *   0x05 _program         0x11 _panFadeReload
 *   0x06 _velocity        0x12 _pitchBendFadeCount
 *   0x07 _noteOffset      0x13 _pendingStop
 *   0x08 _keyOnDelay      0x14 _loopStartPtr
 *   0x09 _volumeFadeCounter 0x16 _pSrc
 *   0x0A _pitchBendFadeCounter
 *   0x0B _panFadeCounter
 * _owner and _midiChannel below are NOT part of the original struct (it
 * has no equivalent fields) - they're C++-side conveniences so Channel
 * methods and callers don't need the MIDI channel number (array index+1)
 * threaded through separately. Any future raw "[bx+N]" disassembly offset
 * can be mapped directly via the table above.
 */
class Channel {
public:
	RSound *_owner = nullptr;
	int _midiChannel = 0;        // 1-9: the MIDI channel this struct drives

	int _activeCount = 0;        // gate/duration countdown; also freshly loaded from the duration byte of a note event
	int _pitchBendFadeStep = 0;  // per-step delta added to _pitchBend each ramp tick
	int _volumeFadeStep = 0;     // per-step delta added to _volume each ramp tick
	int _panFadeStep = 0;        // per-step delta added to _pan each ramp tick
	int _note = 0;               // MIDI note number, read from the note/duration stream
	int _program = 0;            // patch/instrument number, sent as a Program Change
	int _velocity = 0;           // note velocity, used by RSound::sendNoteOn()
	int _noteOffset = 0;         // subtracted from _activeCount to derive _keyOnDelay; 0xFF = sustain full duration +1 (no early cutoff)
	int _keyOnDelay = 0;         // countdown to RSound::Channel_flushHeldNotes()
	int _volumeFadeCounter = 0;  // counts down to 0 before applying _volumeFadeStep
	int _pitchBendFadeCounter = 0; // counts down to 0 before applying _pitchBendFadeStep
	int _panFadeCounter = 0;     // counts down to 0 before applying _panFadeStep
	int _volume = 0;             // current channel volume (MIDI CC#7)
	int _pitchBend = 0;          // current pitch bend value (status 0xEn, coarse/MSB only)
	int _pan = 0;                // current pan value (MIDI CC#10); 64 = center
	int _volumeFadeReload = 0;   // reload value for _volumeFadeCounter
	int _pitchBendFadeReload = 0; // reload value for _pitchBendFadeCounter
	int _panFadeReload = 0;      // reload value for _panFadeCounter
	int _pitchBendFadeCount = 0; // total ramp steps remaining before the pitch-bend ramp halts
	int _pendingStop = 0;        // non-zero while the channel is fading out to silence
	byte *_loopStartPtr = nullptr; // fixed restart anchor used by the full-reset/loop-restart opcode
	byte *_pSrc = nullptr;         // current read pointer into the sound-data stream
	byte *_innerLoopPtr = nullptr; // inner-loop restart address
	byte *_outerLoopPtr = nullptr; // outer-loop restart address
	uint16 _innerLoopCount = 0; // signed byte stored as a 16-bit loop count
	uint16 _outerLoopCount = 0; // signed byte stored as a 16-bit loop count
	byte *_soundData = nullptr;    // identity pointer used by RSound::isSoundActive()

public:
	Channel() {}

	/**
	 * Partial reset used both when loading a new sound and when the
	 * loop-restart opcode fires. Deliberately does NOT touch volume,
	 * program, velocity, key-on state or active/duration - matches
	 * Channel_reset() in the original disassembly.
	 */
	void reset(byte *startPtr);

	/**
	 * Marks the channel as pending-stop (fading toward silence).
	 */
	void enable(int flag);

	/**
	 * Loads a brand new sound into the channel: resets it, marks it
	 * active, and resets pitch bend to center on the real device.
	 */
	void load(byte *pData);
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
	byte _lastMidiStatus;             // running-status cache, avoids resending an unchanged status byte
	bool _noteTriggeredThisPoll;      // throttles note-on dispatch to at most one per update() tick, across all channels
	byte _heldNotes[RSOUND_CHANNEL_COUNT + 1][4]; // per-MIDI-channel held-note slots (index 0 unused; channels are 1-9)
	RSoundFadeCheckMode _fadeCheckMode;
	bool _fadeCheckAlternate;
	int _fadeCheckCounter;
	int _fadeCheckPeriod;

	/**
	 * Data-segment offset of this driver's own "command0_array" (the
	 * MT-32 title-display + patch-init SysEx table sent by command0()).
	 * Each driver has its own copy of this table at its own offset
	 * within its own resource file - unlike the fixed 5-byte SysEx
	 * header (_sysExHeader below), the table's content differs per
	 * driver beyond a shared prefix, so it can't be hardcoded once;
	 * parameterizing the offset via the constructor avoids needing a
	 * command0() override in every derived class.
	 */
	int _sysExOffset;

	MidiDriver_MT32GM *_midiDriver;
	uint32 _driverCallbackDelta;
	NativeSoundTimer _hostTimer;

	void update();
	void pollAllChannels();
	void Channel_pollActive(Channel *channel);

	/**
	 * Zeroes _activeCount and the three fade-step fields for channels in
	 * [first, last).
	 */
	/**
	 * Resets all 9 channels and the held-notes table.
	 */
	void resetAllChannels();

	/**
	 * Run pending-stop volume decay using the scheduler embedded in the
	 * loaded overlay. Sections 1, 2, and 9 and both demo overlays use a
	 * fixed every-other-poll toggle. Sections 3-8 use a programmable
	 * countdown; zero disables it and the counter reloads after each pass.
	 */
	void checkFadingChannels();
	void Channel_checkFade(Channel *channel);

	/**
	 * Sends Note-Off (velocity 0) for all currently-held notes on the
	 * given channel's MIDI channel, then clears the held-note table for it.
	 */
	void Channel_flushHeldNotes(Channel *channel);

protected:
	int _commandParam;

	void setFadeCheckPeriod(int period) {
		if (_fadeCheckMode == kRSoundFadeCheckProgrammable)
			_fadeCheckPeriod = period;
	}

	/** Clear the active and fade state for channel indices in [first, last). */
	void resetChannelRange(int first, int last);

	/** Reset the per-MIDI-channel held-note tracking table to empty. */
	void resetHeldNotes();

	/** Reset held-note slots for the inclusive MIDI-channel range. */
	void resetHeldNotesRange(int firstChannel, int lastChannel);

	byte *loadData(int offset) {
		return &_soundData[offset];
	}

	/**
	 * Hook called once per update() frame, immediately after the disabled
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
	 * Play the specified sound, using any free channel from 6 to 8.
	 * Channel 9 is deliberately never touched here - matches the
	 * disassembly, which never includes it in this scan. Returns the
	 * channel that was used (or nullptr if none was free), since some
	 * commands poke the just-loaded channel's loop pointer directly
	 * afterward.
	 */
	Channel *playSound(int offset);

	/**
	 * Play the specified sound using any channel from 0 to 8, including
	 * channel 9 - confirmed distinct from playSound() by rsound.001's
	 * disassembly (rsound_command30/32/38).
	 */
	Channel *playSoundAny(int offset) {
		return playSoundData(loadData(offset), 0);
	}

	Channel *playSoundData(byte *pData, int startingChannel = 5);

	/**
	 * Checks to see whether the given block of data is already loaded into a channel.
	 */
	bool isSoundActive(byte *pData);

	int getRandomNumber();

	// ---- Low-level MIDI send helpers -------------------------------
	// All send through the ScummVM MT-32/MIDI driver.
	void sendNoteOn(int midiChannel, int note, int velocity);
	void sendProgramChange(int midiChannel, int program);
	void sendVolume(int midiChannel, int volume);
	void sendPitchBend(int midiChannel, int value);
	void sendPan(int midiChannel, int value);
	void muteChannel(int midiChannel);
	void restoreChannelVolume(int midiChannel, int volume);

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
	int _frameCounter;
	bool _isDisabled;
	int _pollResult;
	int _resultFlag;

public:
	static void validate(bool isDemo);

public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param filename		Specifies the Roland sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 * @param dataSize		Size of the data segment
	 * @param sysExOffset	Offset of this driver's own command0_array
	 * @param fadeCheckMode Native pending-stop fade scheduler
	 */
	RSound(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset,
		RSoundFadeCheckMode fadeCheckMode);

	~RSound() override;

	int stop() override;
	int poll() override;
	void noise() override {
		// No equivalent in the Roland driver - noise() is an Adlib/OPL-only concept
	}
	void setVolume(int volume) override;

	int getFrameCounter() {
		return _frameCounter;
	}

	void onTimer();
	static void timerCallback(void *data);
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
