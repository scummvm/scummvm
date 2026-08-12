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

#ifndef MADS_DRAGONSPHERE_SOUND_RSOUND_H
#define MADS_DRAGONSPHERE_SOUND_RSOUND_H

#include "audio/mt32gm.h"
#include "mads/core/native_sound_timer.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

class RSound;

#define RSOUND_CHANNEL_COUNT 9

/**
 * Represents the data for a channel on the Dragonsphere MT-32 / MPU-401
 * driver. Confirmed identical in layout (sizeof 0x27, same field offsets/
 * order/roles) to the equivalent Return of the Phantom RSound Channel
 * struct (engines/mads/phantom/rsound.h). The interpreter and callback
 * audits checked every retail and demo overlay, while the structure itself
 * was checked against this game's rsound.dr1 at every anchor point that has
 * an IDA-resolved name or a distinctive access pattern: _activeCount(0x00),
 * the 0x00-0x01/0x02-0x03 word-pair zeroing in resetAllChannels, _program
 * (0x05, sendProgramChange), _velocity (0x06, the note-on helper),
 * field_9/_keyOnDelay (0x09, the countdown+flush at the top of
 * Channel_pollActive), _pan (0x0F), _volume (0x10), _pitchBend (0x11),
 * _pSrc (0x16, the working script-read pointer), _soundData (0x20),
 * _pendingStop (0x26). The remaining byte fields (0x00-0x13) were NOT
 * individually re-derived from Dragonsphere-specific code - they're
 * carried over from Phantom's confirmed layout on the strength of every
 * checked anchor matching exactly; flag if a future command's behavior
 * contradicts one of the un-rechecked fields.
 */
class Channel {
public:
	RSound *_owner = nullptr;
	int _midiChannel = 0;         // 1-9: the MIDI channel this struct drives (C++ convenience, not a real field)

	int _activeCount = 0;         // gate/duration countdown; also freshly loaded from the duration byte of a note event
	int _pitchBendFadeStep = 0;   // per-step delta added to _pitchBend each ramp tick
	int _volumeFadeStep = 0;      // per-step delta added to _volume each ramp tick
	int _panFadeStep = 0;         // per-step delta added to _pan each ramp tick
	int _note = 0;                // last-played note value (note + _transpose), used to detect a note change
	int _program = 0;             // patch/instrument number, sent as a Program Change
	int _velocity = 0;            // note velocity, used by RSound::sendNoteOn()
	int _noteOffset = 0;          // subtracted from _activeCount to derive _keyOnDelay (relative mode)
	int _keyOnDelayOverride = 0;  // when non-zero, used directly as _keyOnDelay instead of the relative computation (absolute mode)
	int _keyOnDelay = 0;          // countdown to RSound::Channel_flushHeldNotes()
	int _volumeFadeCounter = 0;   // counts down to 0 before applying _volumeFadeStep
	int _volumeFadeReload = 0;    // reload value for _volumeFadeCounter
	int _pitchBendFadeCounter = 0; // counts down to 0 before applying _pitchBendFadeStep
	int _panFadeCounter = 0;      // counts down to 0 before applying _panFadeStep
	int _panFadeReload = 0;       // reload value for _panFadeCounter
	int _pan = 0;                 // current pan value (MIDI CC#10); 64 = center
	int _volume = 0;              // current channel volume (MIDI CC#7)
	int _pitchBend = 0;           // current pitch bend value (status 0xEn, coarse/MSB only)
	int _pitchBendFadeReload = 0; // reload value for _pitchBendFadeCounter
	int _pitchBendFadeCount = 0;  // total ramp steps remaining before the pitch-bend ramp halts
	byte *_loopStartPtr = nullptr; // fixed restart anchor used by the full-reset/loop-restart opcode
	byte *_pSrc = nullptr;         // current read pointer into the sound-data stream
	byte *_innerLoopPtr = nullptr; // inner-loop restart address
	byte *_outerLoopPtr = nullptr; // outer-loop restart address
	uint16 _innerLoopCount = 0; // signed byte stored as a 16-bit loop count
	uint16 _outerLoopCount = 0; // signed byte stored as a 16-bit loop count
	byte *_soundData = nullptr;    // identity pointer used by RSound::isSoundActive()
	byte *_branchTarget = nullptr; // resume-after-branch pointer, used by the call/return opcode pair
	int _transpose = 0;            // added to note bytes before comparison/storage
	int _pendingStop = 0;          // non-zero while the channel is fading out to silence; also gates RSound::sendVolume()

public:
	Channel() {}

	/**
	 * Zeroes the entire 40-byte channel struct, then re-initialises the
	 * loop/source/sound-data pointers to startPtr and centers pan and
	 * pitch bend (0x40). Matches Channel_reset exactly.
	 */
	void reset(byte *startPtr);

	/**
	 * Marks the channel as pending-stop (fading toward silence) and
	 * invalidates _soundData so isSoundActive() no longer matches it.
	 */
	void enable(int flag);

	/**
	 * Loads a brand new sound into the channel: resets it, marks it
	 * active, and resets pitch bend to center on the real device.
	 */
	void load(byte *pData);
};

/**
 * Base class for the Dragonsphere MT-32 / MPU-401 sound player resource
 * files (rsound.dr1-.dr6, .dr9). Ported from rsound.dr1's disassembly and
 * audited across every retail and demo overlay. The family uses the same
 * 0xBE-0xFF opcode range as Return of the Phantom, but the direct audit also
 * found and preserves demo-specific arithmetic, random, and pitch-bend
 * behavior instead of assuming that the two families are identical.
 * Genuine, CONFIRMED differences from Phantom's RSound base:
 *
 *  - command1/command3/command5 use a 6-channel "lower" group (1-5 AND 9)
 *    and a 3-channel "upper" group (6,7,8), not Phantom's default 5+4
 *    split (this shape matched Phantom's RSound5 specifically, but here
 *    it's the shared base behavior, not a per-driver override).
 *  - command4/command5 are concrete (non-virtual) in this base, not pure
 *    virtual like Phantom's - rsound.dr1's command4/command5 have NO
 *    isSoundActive() gate at all, unlike every Phantom driver. This may
 *    need revisiting once RSound2+ confirm whether that's universal here
 *    too, or driver-specific.
 *  - A new deferred-callback subsystem (_callbackCounter/_callbackPeriod/
 *    _callbackFnPtr, tickCallback(), scheduleCallback(),
 *    resetCallbackTimer()) mirrors the sibling Dragonsphere ASound
 *    driver's identically-shaped mechanism (see asound.h) - ABSENT from
 *    Phantom's RSound family entirely. Used by RSound1's music-loading
 *    commands (16, 32-48) for the same "immediate load, or defer until
 *    the music channels free up" idiom documented for ASound1's Pattern B.
 *  - isMusicChannelsActive() checks this driver's own
 *    6-channel "lower" group (1-5, 9), not Phantom's fixed 6-channel(0-6)
 *    equivalent from ASound.
 *  - No checkRandomAmbianceTrigger()/_randomAmbianceTriggerFlag hook -
 *    rsound_update() has no equivalent call at all; that Phantom RSound1
 *    mechanism is not present here (replaced by the deferred-callback
 *    subsystem above).
 *  - sendVolume()/sendPan() naming CORRECTED from a mixed-up disassembly
 *    symbol: the function IDA auto-named "sendVolume" actually sends CC#10
 *    (Pan) using the channel's _pan field; the real volume-sender (CC#7,
 *    _volume field) was an unnamed helper. Also, CONFIRMED NEW: the
 *    real sendVolume() only actually transmits when the channel's
 *    _pendingStop is zero - Channel_checkFade's own separate fade-out
 *    mechanism otherwise takes precedence. No equivalent gate exists on
 *    Phantom's sendVolume().
 *  - sendReverbSysEx()'s exact byte layout is confirmed in every retail
 *    and demo overlay: each native helper masks the parameters to 2/3/3
 *    bits, writes them after the fixed 10 00 01h Roland System Area
 *    Reverb address, and sends that SysEx template.
 *  - null_sound_data (see _silenceStream) is a fixed 2-byte (0, 0)
 *    silence marker referenced by BOTH Channel::enable() and
 *    Channel_checkFade() - same intent as Phantom's fixed 3-byte
 *    silence stream (2 bytes here, unlike Phantom's 3 - confirmed
 *    directly from this disassembly).
 *
 * NOTE: DOS-specific driver ceremony from the original (timer IRQ hooking,
 * MPU-401 hardware detection/reset, PIT-based SysEx delay calibration, the
 * system-clock save/restore around it) has no ScummVM equivalent and is
 * not ported, matching every sibling driver.
 */
class RSound : public SoundDriver {
	friend class Channel;
private:
	int _masterVolume = 255;
public:
	/**
	 * Member-function pointer type for deferred sound-loader callbacks.
	 * Returns int (the return value is discarded by tickCallback()) so
	 * that MAKE_CALLBACK's reinterpret_cast only ever crosses the
	 * enclosing-class boundary, never the return type as well. Every
	 * driver-specific callback target returns int to match.
	 * Public so driver subclasses can build a MAKE_CALLBACK-style cast
	 * (reinterpret_cast<RSound::CallbackFunction>(&DerivedClass::fn)) to
	 * pass to scheduleCallback().
	 */
	typedef int (RSound::*CallbackFunction)();

	private:
	uint16 _randomSeed;
	int _stateChangedFlag;        // latches _pollResult=0xFFFF once per state change
	MidiDriver_MT32GM *_midiDriver;
	uint32 _driverCallbackDelta;
	NativeSoundTimer _hostTimer;

	/**
	 * Per-MIDI-channel held-note slots (index 0 unused; channels are
	 * 1-9; 4 = max chord polyphony). CONFIRMED byte-addressed, 4 bytes
	 * per channel (channel*4 byte offset, single-byte reads/writes in
	 * Channel_flushHeldNotes and the chord opcode). The real table
	 * (declared as "dw 20h dup(?)" in the disassembly) is 64 bytes/16 rows - 6 rows larger
	 * than this [10][4] array - but rows 10-15 are never addressed since
	 * channel numbers only run 1-9; that's unused padding in the
	 * original; this smaller array is behaviorally equivalent.
	 */
	byte _heldNotes[RSOUND_CHANNEL_COUNT + 1][4];

	/**
	 * Data-segment offset of this driver's own "command0_array" (the
	 * table sent by command0()/reset() via sendSysEx). Each driver has
	 * its own copy of this table at its own offset within its own
	 * resource file. Confirmed 0x9C for rsound.dr1.
	 */
	int _sysExOffset;

	/**
	 * "null_sound_data" - a fixed 2-byte (0,0) silence marker,
	 * referenced by BOTH Channel::enable() (Channel_enable, written to
	 * _soundData) and Channel_checkFade() (written to _pSrc once a
	 * pending-stop channel's volume has fully decayed) - the same single
	 * symbol in the disassembly, confirmed at a different address in
	 * each driver's own resource file, but always the same (0,0)
	 * content, same intent as Phantom's fixed 3-byte silence stream.
	 * pollActiveChannel() processing a (note=0, duration=0) pair sets
	 * _activeCount to 0, and its own top-of-function guard (checked
	 * before any decrement) then short-circuits every later call before
	 * _pSrc is ever read again - so only these 2 bytes are ever actually
	 * consumed. A shared static array (rather than reading via
	 * loadData()) is therefore sufficient and avoids needing a
	 * per-driver offset for content that never varies.
	 */
	static byte _silenceStream[2];

	/**
	 * General-purpose script variable table (matches the equivalent
	 * Phantom mechanism, 32 bytes).
	 */
	byte _scriptVariables[32];

	/**
	 * Half-rate fade-check timer -
	 * drives checkFadingChannels() directly.
	 */
	int _fadeCheckCounter;

	/**
	 * Bytes written by opcodes 0xBE-0xC1. Their purpose remains unknown
	 * because no reader exists in the checked overlays. Every retail and
	 * demo overlay writes 0xBE and 0xBF to the same byte, and 0xC0 and
	 * 0xC1 to separate bytes.
	 */
	byte _opcodeBeBfValue;
	byte _opcodeC0Value;
	byte _opcodeC1Value;

	// ---- Deferred-callback subsystem - NEW vs. the
	// Phantom RSound family, mirrors the sibling ASound driver's
	// identically-shaped mechanism. ----
	uint16 _callbackCounter = 0;
	uint16 _callbackPeriod = 0;
	CallbackFunction _callbackFnPtr = nullptr;

	/** Tracks which bucket-4 (32-48) music piece was last launched, for command18's re-entry. */
	uint16 _musicIndex = 0;

	void update();
	void pollAllChannels();

	/**
	 * Per-channel opcode interpreter (Channel_pollActive in the
	 * disassembly). The common and demo-specific paths are based on the
	 * direct retail/demo overlay audit described in the class comment.
	 */
	void pollActiveChannel(Channel *channel);

	int readScriptByte(byte *&pSrc);
	uint16 readScriptWord(byte *&pSrc);

	void resetAllChannels();
	void resetChannels1to5();

	/** Resets channels 6,7,8 (0-based indices 5-7). Matches resetChannels6to8. */
	void resetChannels6to8();

	void checkFadingChannels();
	void Channel_checkFade(Channel *channel, int midiChannel);
	void Channel_flushHeldNotes(Channel *channel);

protected:
	int _commandParam;

	/**
	 * The DR1 demo's 0xDC/0xDE handlers use their immediate divisor.
	 * Retail drivers and the DR9 demo instead reproduce the Phantom
	 * handlers' self-divisor bug.
	 */
	bool _usesImmediateArithmeticOperands;

	/**
	 * The demo VM omits the retail random mask and pending-stop pitch-bend
	 * guard. Both demo overlays share those two differences.
	 */
	bool _usesDemoOpcodeSemantics;

	byte *loadData(int offset) {
		return &_soundData[offset];
	}

	/**
	 * Zeroes _activeCount/_pitchBendFadeStep/_volumeFadeStep/_panFadeStep
	 * for channels [first, last] (0-based indices). Protected (not
	 * private) so per-driver reset helpers with a different range (e.g.
	 * RSound2's resetChannels1to6()) can call it directly.
	 */
	void resetChannelRange(int first, int last);

	/**
	 * A driver-specific variant of Channel::enable() confirmed across
	 * multiple drivers so far (RSound4's and RSound5's command5)
	 * - redirects _soundData (and, if the channel is about to expire this
	 * tick, _pSrc too) to _silenceStream instead of nullptr.
	 */
	void disableChannelTo(int channelIndex, byte flag);

	/**
	 * Resets the _heldNotes table (see its field comment). Protected so
	 * per-driver reset helpers with a different channel range can still
	 * clear this shared table without needing direct access to the
	 * private member.
	 */
	void resetHeldNotes();

	/**
	 * Half-rate fade-check period reload value (see _fadeCheckCounter
	 * above). Protected (not private) so per-driver command1()/command3()/
	 * command5() overrides that need to arm it directly (matching an
	 * inline "mov cs:_fadeCheckPeriod, 1" / armFadeCheck in the
	 * disassembly) can do so without going through the base class's own
	 * command3()/command5().
	 */
	int _fadeCheckPeriod;

	void resultCheck();

	/**
	 * Schedule fn as the next deferred-load callback. Does NOT touch
	 * _callbackCounter/_callbackPeriod - those are preserved from the
	 * previous loader so the callback fires on the right beat. Cast the
	 * derived-class member-function pointer with reinterpret_cast.
	 */
	void scheduleCallback(CallbackFunction fn) {
		_callbackFnPtr = fn;
	}

	/**
	 * Returns true if a deferred-load callback is currently scheduled.
	 * Used by RSound2's command32/command35 to work around a confirmed
	 * bug in the original (see their .cpp comments).
	 */
	bool isCallbackScheduled() const {
		return _callbackFnPtr != nullptr;
	}

	/**
	 * Arm the periodic timer and clear any pending callback pointer.
	 * Call at the head of every immediate-load function.
	 */
	void resetCallbackTimer(uint16 period) {
		_callbackFnPtr = nullptr;
		_callbackCounter = period;
		_callbackPeriod = period;
	}

	/**
	 * Arm the periodic timer with separate counter and period values.
	 * Used by RSound1's command17 (counter=0xC0, period=0x60) and
	 * command44 (counter=0x60, period=0xE0).
	 */
	void resetCallbackTimerEx(uint16 counter, uint16 period) {
		_callbackFnPtr = nullptr;
		_callbackCounter = counter;
		_callbackPeriod = period;
	}

	/** Set the music-piece index read by command18. */
	void setMusicIndex(uint16 idx) {
		_musicIndex = idx;
	}
	uint16 getMusicIndex() const {
		return _musicIndex;
	}

	/**
	 * Deferred-callback tick: decrements _callbackCounter; when it
	 * reaches zero, reloads it from _callbackPeriod and calls
	 * _callbackFnPtr (if non-null), then clears _callbackFnPtr so it
	 * fires exactly once.
	 */
	void tickCallback();

	/**
	 * Checks whether channels 1-5 or 9 (this driver's "lower"/music
	 * group) have any non-zero _activeCount - the
	 * Dragonsphere-specific equivalent of the sibling ASound driver's
	 * isMusicChannelsActive(), but scanning THIS driver's own channel
	 * grouping rather than ASound's fixed channels 0-6.
	 */
	int isMusicChannelsActive();

	/**
	 * Plays the specified sound, using any free channel from 6 to 8.
	 * Matches the disassembly's own playSoundChannels6to8 exactly
	 * (symmetric free/fallback scan: free scan ch6,7,8; fallback scan
	 * ch8,7,6). Renamed from playSound for clarity, matching the
	 * disassembly's own name.
	 */
	Channel *playSoundChannels6to8(int offset);

	/**
	 * Plays the specified sound, using any free channel from 7 to 8.
	 * Matches playSoundChannels7to8: symmetric free/fallback scan (free
	 * scan ch7 then ch8; fallback scan ch8 then ch7). Confirmed
	 * identical across 2 drivers so far (RSound2, RSound6).
	 */
	Channel *playSoundChannels7to8(int offset);

	/**
	 * Plays the specified sound, using any free channel from 1 to 5, with
	 * a fully symmetric free/fallback scan. Matches the disassembly's own
	 * playSoundAny exactly - renamed here to playSoundChannels1To5 for
	 * clarity, since (unlike Phantom's identically-named playSoundAny(),
	 * which reaches channels 1-8) this driver's version only ever reaches
	 * channels 1-5. Also distinct from Phantom's playSoundChannels1To5(),
	 * whose fallback scan is asymmetric (never reaches channel 5) - this
	 * one's fallback does.
	 */
	Channel *playSoundChannels1To5(int offset);

	/**
	 * Scans [startingChannel, freeScanEnd] for a free channel; if none
	 * found, scans [startingChannel, fallbackScanEnd] in reverse for a
	 * pending-stop channel to pre-empt.
	 */
	Channel *playSoundData(byte *pData, int startingChannel, int freeScanEnd, int fallbackScanEnd);

	/** Checks whether the given block of data is already loaded into a channel (channels 1-8 only, matching the disassembly). */
	bool isSoundActive(byte *pData);

	int getRandomNumber();

	/** Resolve native near callbacks embedded in section-specific streams. */
	virtual bool callFunction(uint16, Channel &) {
		return false;
	}

	// ---- Low-level MIDI send helpers -------------------------------
	void sendNoteOn(int midiChannel, int note, int velocity);
	void sendProgramChange(int midiChannel, int program);

	/**
	 * CORRECTED naming (see class comment): sends CC#7 (Volume) - but
	 * ONLY if the channel is not currently pending-stop (matches
	 * the disassembly's "cmp [bx+_pendingStop],0" gate, confirmed new vs.
	 * Phantom's unconditional sendVolume()).
	 */
	void sendVolume(Channel *ch);

	void sendVolumeCC(int midiChannel, int volume);
	void resetPitchBend(int midiChannel);
	void sendPitchBend(int midiChannel, int value);

	/** CORRECTED naming (see class comment): sends CC#10 (Pan). Matches the disassembly's mislabeled "sendVolume". */
	void sendPan(int midiChannel, int value);

	void muteChannel(int midiChannel);

	/**
	 * Sends the GM-reset Control Change sequence (all notes off, reset all
	 * controllers, volume=100, pan=center) to `count` MIDI channels,
	 * counting down from `count` to 1. Implemented in terms of
	 * sendGmResetRange(count, 1).
	 */
	void sendGmReset(int count);

	/**
	 * Sends the GM-reset Control Change sequence to MIDI channels
	 * `high` down to `low` (inclusive), using each count value as the
	 * MIDI channel number - a generalization of sendGmReset() confirmed
	 * needed by RSound6's command4() (which only resets channels 8-6,
	 * not 1..count like every other driver's command4()).
	 */
	void sendGmResetRange(int high, int low);

	/**
	 * Sends a single Roland DT1-style SysEx message from a raw buffer:
	 * the fixed header, then bytes from pData up to (not including) a
	 * 0xFF terminator - each byte sent and folded into a running
	 * checksum - then the checksum byte and a closing F7. Returns a
	 * pointer to the terminating 0xFF byte (matching the disassembly's
	 * own si register value on return), so callers walking a sequence
	 * of consecutive messages can advance past it to find the next one.
	 * Returns nullptr when no terminator occurs within maxLength bytes.
	 */
	const byte *sendSysExData(const byte *pData, uint maxLength);

	/** sendSysExData() for a block already in this driver's own loaded sound data. */
	const byte *sendSysEx(int offset);

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
	 * Restore all 128 MT-32 Patch Memory records to the standard A/B
	 * timbre mapping. Native export 2 calls this during driver teardown,
	 * after stopping playback. All checked Dragonsphere retail and demo
	 * overlays use the same four-block, 32-record structure.
	 *
	 * ScummVM does not call the DOS hardware teardown path: opening each
	 * MidiDriver_MT32GM resets the selected device, while closing follows
	 * the shared MIDI-driver lifecycle. Keep the exact native translation
	 * available without imposing its synchronous teardown sequence.
	 */
	void restorePatchMemory();

	/**
	 * Masks the 3 caller-supplied values to 2/3/3 bits (mode 0-3, time
	 * 0-7, level 0-7) and sends them via the Roland MT-32 System Area
	 * Reverb SysEx address (10 00 01h), matching the native retail and
	 * demo templates.
	 */
	void sendReverbSysEx(int mode, int time, int level);
	void onTimer();
	static void timerCallback(void *data);

	int command0();
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

public:
	/**
	 * Validate the sound driver files needed for data
	 */
	static void validate();

public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param filename		Specifies the Roland sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 * @param dataSize		Size of the data segment
	 * @param sysExOffset	Offset of this driver's own command0_array
	 */
	RSound(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset,
		bool usesDemoOpcodeSemantics = false);

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
};

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS

#endif
