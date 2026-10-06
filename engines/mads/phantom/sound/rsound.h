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

#ifndef MADS_PHANTOM_SOUND_RSOUND_H
#define MADS_PHANTOM_SOUND_RSOUND_H

#include "audio/mt32gm.h"
#include "mads/core/native_sound_timer.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace Phantom {
namespace Sound {

class RSound;

#define RSOUND_CHANNEL_COUNT 9

/**
 * Represents the data for a channel on the Return of the Phantom MT-32 /
 * MPU-401 driver. Ported from the Channel struct identified in rsound.ph1's
 * disassembly (sizeof 0x27, five bytes larger than the equivalent Rex
 * Nebular RSound Channel struct).
 *
 * The word-sized fields at 0x14-0x20 (_loopStartPtr through _soundData) are
 * at the IDENTICAL offsets/order/roles as Rex Nebular's Channel struct -
 * that part of the layout is unchanged between games. The byte fields at
 * 0x00-0x13 are the same overall SET as Rex Nebular (confirmed by matching
 * each field against the exact MIDI status byte/controller number sent by
 * its dedicated helper, e.g. sendProgramChange sends status 0xC0 using
 * _program), just reordered, with _pendingStop moved down to the very last
 * byte (0x26) to make room for a new field (_keyOnDelayOverride, 0x08) with
 * no Rex Nebular equivalent. Two more new fields exist past the Rex Nebular
 * struct's end: _branchTarget (0x22) and _transpose (0x25).
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
	int _keyOnDelayOverride = 0;  // when non-zero, used directly as _keyOnDelay instead of the relative computation (absolute mode; new to Phantom)
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
	byte *_branchTarget = nullptr; // resume-after-branch pointer, used by the call/return opcode pair (new to Phantom)
	int _transpose = 0;            // added to note bytes before comparison/storage (new to Phantom)
	int _pendingStop = 0;          // non-zero while the channel is fading out to silence

public:
	Channel() {}

	/**
	 * Zeroes the entire 40-byte channel struct, then re-initialises the
	 * loop/source/sound-data pointers to startPtr and centers pan and
	 * pitch bend (0x40). More aggressive than Rex Nebular's equivalent,
	 * which leaves volume/program/velocity/key-on state untouched.
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
 * Base class for the Return of the Phantom MT-32 / MPU-401 sound player
 * resource files (rsound.ph1-.ph9). Mirrors the structure of the Rex
 * Nebular RSound family (see nebular/rsound.h), adapted for this game's
 * substantially richer Channel_pollActive script VM (~65 opcodes here vs.
 * ~15 in Rex Nebular): general arithmetic on a script-variable table,
 * conditional branches, and a call/return pair, in addition to the shared
 * note/fade/loop mechanics.
 *
 * NOTE: DOS-specific driver ceremony from the original (timer IRQ hooking,
 * MPU-401 hardware detection/reset, PIT-based SysEx delay calibration, the
 * system-clock save/restore around it) has no ScummVM equivalent and is not
 * ported - matching how Rex Nebular's RSound skips the same kind of
 * hardware-detection dance.
 */
class RSound : public SoundDriver {
	friend class Channel;
private:
	uint16 _randomSeed;
	int _masterVolume = 255;
	int _stateChangedFlag;        // latches _pollResult=0xFFFF once per state change
	MidiDriver_MT32GM *_midiDriver;
	uint32 _driverCallbackDelta;
	NativeSoundTimer _hostTimer;

	/**
	 * Per-MIDI-channel held-note slots (index 0 unused; channels are
	 * 1-9; 4 = max chord polyphony). The apparent second table in the
	 * reset/flush helpers is this same storage expressed relative to the
	 * overlay's data segment. Converting it using independently named
	 * data-segment fields resolves to the absolute table read and written by
	 * the chord interpreter, reset, and held-note flush paths.
	 */
	byte _heldNotes[RSOUND_CHANNEL_COUNT + 1][4];

	/**
	 * Data-segment offset of this driver's own "command0_array" (the
	 * table sent by command0() via sendSysEx). Each driver has its own
	 * copy of this table at a verified offset within its resource file.
	 *
	 * The retail and demo subclasses supply that offset to the base
	 * constructor.
	 */
	int _sysExOffset;

	/**
	 * General-purpose script variable table. Confirmed 32 bytes via the
	 * gap to the next declared global (_scriptReadPtr) - also matches the
	 * sibling ASound driver's analogous _scriptVars[32].
	 */
	byte _scriptVariables[32];

	/**
	 * Half-rate fade-check timer - same counter/period/reload shape as the
	 * Rex Nebular RSound4/RSound6 callback mechanism, but drives
	 * checkFadingChannels() directly rather than an arbitrary function
	 * pointer.
	 */
	int _fadeCheckCounter;

	/**
	 * Cluster of globals written by opcodes 0xBE-0xC1. All supported retail
	 * and demo overlays were checked: the values are stored, and 0xBF/0xC0
	 * conditionally copy their targets before the first update tick, but no
	 * later reader exists. Their native purpose therefore remains unresolved,
	 * and the names below are descriptive rather than semantic.
	 *
	 * _clockFine/_clockMed/_clockCoarse default to 7/28/112, a clean 4x
	 * progression that suggests a coarse/medium/fine clock-division hierarchy.
	 * This remains an inference rather than implemented behavior because the
	 * checked overlays never read those values. _tickCounter gates the only
	 * observed copies: opcodes 0xBF/0xC0 can replace the coarse/medium values
	 * before the first update tick, after which the counter is nonzero.
	 */
	int _tickCounter;
	int _clockMedTarget;          // pending value for _clockMed, set by opcode 0xC0
	int _clockCoarseTarget;       // pending value for _clockCoarse, set by opcode 0xBF
	int _clockUnknown;            // default 0; doesn't fit the 4x pattern, standalone (opcode 0xBE)
	int _clockCoarse;             // default 112 (=28*4)
	int _clockMed;                // default 28 (=7*4)
	int _clockFine;               // default 7
	int _clockEnabled1;           // set to 1 by opcode 0xBF
	int _clockEnabled2;           // set to 1 by opcode 0xBF

	void update();
	void pollAllChannels();

	/**
	 * Per-channel opcode interpreter (Channel_pollActive in the
	 * disassembly). Implements a bytecode VM: bytes with the high bit
	 * clear are two-byte [note][duration] events; bytes > 0xBD are
	 * commands (0xBE-0xFF), dispatched via the switch in the .cpp.
	 * Re-enters its own dispatch point (via goto) after every opcode that
	 * doesn't consume a duration tick, matching the sibling ASound
	 * driver's pollActiveChannel() structure.
	 */
	void pollActiveChannel(Channel *channel);

	/**
	 * Reads one byte from the channel's current script position and
	 * sign-extends it, advancing pSrc by 1.
	 */
	int readScriptByte(byte *&pSrc);

	/**
	 * Reads two bytes (little-endian) from the channel's current script
	 * position as a raw offset into _soundData, advancing pSrc by 2.
	 * Distinct from readScriptByte() - used only for absolute
	 * jump/restart targets embedded in the script.
	 */
	uint16 readScriptWord(byte *&pSrc);

	void resetAllChannels();
	void resetChannels1to5();

	/**
	 * CORRECTED (was wrongly named/ranged resetChannels6to9): resets
	 * channels 4-9 (0-based indices 3-8), confirmed directly from
	 * disassembly - "CODE XREF: rsound_command4".
	 */
	void resetChannels4to9();

	void checkFadingChannels();
	void Channel_checkFade(Channel *channel, int midiChannel);
	void Channel_flushHeldNotes(Channel *channel);

protected:
	int _commandParam;

	byte *loadData(int offset) {
		return &_soundData[offset];
	}

	/**
	 * Cleared to 0 by RSound1's command37 (a "cancel any pending
	 * random-ambiance trigger" side effect of playing that specific
	 * sound). CONFIRMED: the only code that ever sets it to 0xFF (arming
	 * checkRandomAmbianceTrigger()) is itself unreachable/dead code - so
	 * in the real game this mechanism never actually fires. Implemented
	 * faithfully anyway in case that changes for a different driver.
	 * Protected (not private) so driver subclasses with their own
	 * commands touching it (like RSound1's command37) can reach it
	 * directly.
	 */
	int _randomAmbianceTriggerFlag;

	/**
	 * Half-rate fade-check period reload value (see _fadeCheckCounter
	 * above). Protected (not private) so per-driver command1()/command3()/
	 * command5() overrides that need to arm it directly (matching an
	 * inline "mov cs:_fadeCheckPeriod, 1" in the disassembly, e.g.
	 * RSound5's driver-specific 6-channel command3() and 3-channel
	 * enableChannels678()) can do so without going through the base
	 * class's own command3()/enableUpperChannels().
	 */
	int _fadeCheckPeriod;

	/**
	 * Zeroes _activeCount/_pitchBendFadeStep/_volumeFadeStep/_panFadeStep
	 * for channels [first, last] (0-based indices). Protected (not
	 * private) so per-driver command4()/command5() overrides that need a
	 * narrower or differently-shaped reset than resetChannels4to9() /
	 * resetAndGmResetUpperChannels() below can call it directly - e.g.
	 * RSound3's command4(), confirmed to reset only channels 5-9
	 * (indices 4-8), not 4-9.
	 */
	void resetChannelRange(int first, int last);

	/**
	 * Hook called once per update() frame after the disabled check.
	 * Only drivers with a random-ambiance/music picker (e.g. RSound1's
	 * command16) override this; every other driver leaves it a no-op.
	 * Confirmed shape: if _randomAmbianceTriggerFlag
	 * == 0xFF, clear it and fire the driver-specific picker.
	 */
	virtual void checkRandomAmbianceTrigger() {
	}

	void resultCheck();

	/**
	 * Handles a native C4 callback target embedded in a sequence. Controllers
	 * override this only for statically identified native targets; the default
	 * remains a fatal rejection in the bytecode interpreter.
	 */
	virtual bool callFunction(uint16 targetOffset) {
		(void)targetOffset;
		return false;
	}

	/**
	 * Plays the specified sound, using any free channel from 6 to 8.
	 * Matches the disassembly's playSound exactly (rsound_channel6-8).
	 */
	Channel *playSound(int offset);

	/**
	 * Plays the specified sound, using any free channel from 1 to 5.
	 * NOTE: unlike Rex Nebular's playSoundAny() (which reaches all 9
	 * channels), THIS driver's playSoundAny only scans channels 1-5 for a
	 * free slot - confirmed directly from the disassembly. Also confirmed:
	 * the pending-stop fallback scan only reaches down to channel 4, NOT
	 * channel 5 - a genuine asymmetry preserved exactly (channel 5 can
	 * never be pre-empted by this call, only picked while free).
	 */
	Channel *playSoundChannels1To5(int offset);

	/**
	 * Plays the specified sound, using any free channel from 1 to 8
	 * (everything except channel 9) - a third, distinct scan range from
	 * playSound() and playSoundAny() above.
	 */
	Channel *playSoundAny(int offset);

	/**
	 * Plays the specified sound, using any free channel from 5 to 8.
	 * Matches playChannels5to8 in the disassembly - a fourth, distinct
	 * scan range (symmetric free/fallback scan, unlike playSoundAny()'s
	 * asymmetry, and one channel narrower than playSoundAny()'s 1-8).
	 */
	Channel *playSoundChannels5To8(int offset);

	/**
	 * Plays the specified sound, using any free channel from 1 to 6.
	 * Matches playSoundChannels1to6 in the disassembly - a fifth,
	 * distinct scan range (symmetric free/fallback scan).
	 */
	Channel *playSoundChannels1To6(int offset);

	/**
	 * Scans [startingChannel, freeScanEnd] for a free channel; if none
	 * found, scans [startingChannel, fallbackScanEnd] in reverse for a
	 * pending-stop channel to pre-empt. The two end bounds are usually
	 * the same, but playSoundAny() is a confirmed exception (see above).
	 */
	Channel *playSoundData(byte *pData, int startingChannel, int freeScanEnd, int fallbackScanEnd);

	/**
	 * Checks whether the given block of data is already loaded into a channel.
	 */
	bool isSoundActive(byte *pData);

	int getRandomNumber();

	// ---- Low-level MIDI send helpers -------------------------------
	void sendNoteOn(int midiChannel, int note, int velocity);
	void sendProgramChange(int midiChannel, int program);
	void sendVolume(int midiChannel, int volume);
	void sendVolumeCC(int midiChannel, int volume);
	void resetPitchBend(int midiChannel);
	void sendPitchBend(int midiChannel, int value);
	void sendPan(int midiChannel, int value);
	void muteChannel(int midiChannel);

	/**
	 * Sends the GM-reset Control Change sequence (all notes off, reset all
	 * controllers, volume=100, pan=center) to `count` MIDI channels,
	 * counting down from `count` to 1.
	 */
	void sendGmReset(int count);

	/**
	 * Sends a single Roland DT1-style SysEx message from a raw buffer:
	 * the fixed header, then bytes from pData up to (not including) a
	 * 0xFF terminator - each byte sent and folded into a running
	 * checksum - then the checksum byte and a closing F7. The shared
	 * core of sendSysEx()/sendReverbSysEx() below - split out so
	 * hardcoded protocol buffers (not driver-specific loaded sound data)
	 * can be sent without going through loadData(). Returns a pointer to
	 * the terminating 0xFF byte (matching the disassembly's own si
	 * register value on return), so callers walking a sequence of
	 * consecutive messages can advance past it to find the next one.
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
	 * after stopping playback. All checked Phantom retail and demo
	 * overlays use the same four-block, 32-record structure.
	 *
	 * ScummVM does not call the DOS hardware teardown path: opening each
	 * MidiDriver_MT32GM resets the selected device, while closing follows
	 * the shared MIDI-driver lifecycle. Keep the exact native translation
	 * available without imposing its synchronous teardown sequence.
	 */
	void restorePatchMemory();

	/**
	 * CONFIRMED: masks the 3 caller-supplied values
	 * to 2/3/3 bits (mode 0-3, time 0-7, level 0-7) and sends them via
	 * the real Roland MT-32 System Area Reverb SysEx address (10 00 01h),
	 * hardcoded here rather than read via loadData(), since it's a fixed
	 * hardware protocol address, not driver-specific sound data.
	 */
	void sendReverbSysEx(int mode, int time, int level);
	void onTimer();
	static void timerCallback(void *data);

	/**
	 * A confirmed no-op (reads one operand, does
	 * nothing with it).
	 */
	void noOpHandler(int param) {
	}

	virtual int command0();
	int command1();
	int command2();
	int command3();

	/**
	 * Shared tail of command1() (falls through into it after command3())
	 * and command5() (jumps straight into it, ungated, in every driver
	 * confirmed so far): enables channels 5,6,7,8.
	 */
	void enableUpperChannels();

	/**
	 * Shared tail of command4() in every driver confirmed so far:
	 * resetChannels4to9() + sendGmReset(9).
	 */
	void resetAndGmResetUpperChannels();

	/**
	 * PURE VIRTUAL, unlike command1-3/6-8. Confirmed (from RSound1 AND
	 * RSound2, independently) that every driver's command4/command5 are
	 * gated by isSoundActive() on a driver-specific data offset before
	 * calling resetAndGmResetUpperChannels()/enableUpperChannels() above
	 * - a per-driver detail that must not live in the shared base.
	 * Deliberately NO default implementation, so a new driver subclass
	 * can't compile without explicitly providing its own gate - silently
	 * falling back to an ungated version would be wrong (and was, in an
	 * earlier version of this port, until RSound1/RSound2 confirmed the
	 * gate is universal even though its offset isn't).
	 */
	virtual int command4() = 0;
	virtual int command5() = 0;
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
	 * Validates the presence of the sound driver files.
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
		int dataOffset, int dataSize, int sysExOffset);

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
} // namespace Phantom
} // namespace MADS

#endif
