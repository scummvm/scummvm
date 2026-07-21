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

#ifndef MADS_DRAGONSPHERE_ASOUND_H
#define MADS_DRAGONSPHERE_ASOUND_H

#include "common/mutex.h"
#include "common/queue.h"
#include "common/util.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace Dragonsphere {

#define ADLIB_CHANNEL_COUNT 9

struct AdlibChannel {
	static bool _isDisabled;

	// ---- byte fields (offsets 0x00 - 0x13) --------------------------------

	uint8  _activeCount = 0; // 0x00  duration tick countdown; 0 = channel idle
	uint8  _pitchBend = 0; // 0x01  signed pitch-bend offset applied by writePitchBend
	uint8  _volumeFadeStep = 0; // 0x02  signed per-period volume/velocity delta (0xFF = fade out)
	uint8  _vibratoDepth = 0; // 0x03  signed LFO depth; negated each period to oscillate
	uint8  _note = 0; // 0x04  MIDI note number for current event
	uint8  _sampleIndex = 0; // 0x05  index into _samples[] selecting the OPL patch
	uint8  _volume = 0; // 0x06  channel volume (0-127)
	uint8  _noteOffset = 0; // 0x07  subtracted from _activeCount to shorten note duration
	uint8  _durationOverride = 0; // 0x08  when non-zero, overrides the stream's duration byte
	uint8  _keyOnDelay = 0; // 0x09  countdown before the OPL key-on bit is cleared (gate time)
	uint8  _fadePeriodCounter = 0; // 0x0A  counts down to 0 before applying _volumeFadeStep
	uint8  _fadePeriodReload = 0; // 0x0B  reload value for _fadePeriodCounter
	uint8  _arpPeriodCounter = 0; // 0x0C  arpeggio tick counter (reloaded from _arpPeriodReload)
	uint8  _vibPeriodCounter = 0; // 0x0D  LFO tick counter; when it reaches 0 a vibrato step fires
	uint8  _vibPeriodReload = 0; // 0x0E  reload value for _vibPeriodCounter
	uint8  _patchAttenuation = 0; // 0x0F  per-note attenuation offset added on top of the patch TL
	uint8  _velocity = 0; // 0x10  note velocity (0-127); used together with _volume for TL
	uint8  _writeVolumePending = 0; // 0x11  non-zero -> call sub_11856 (secondary freq write) this tick
	uint8  _arpPeriodReload = 0; // 0x12  arpeggio period reload value (set by opcode 9)
	uint8  _arpCounterReload = 0; // 0x13  arpeggio counter reload (decremented; 0xFF = infinite)

	// ---- pointer fields (offsets 0x14 - 0x1F, word-sized in original) ----

	byte *_loopStartPtr = nullptr; // 0x14  start of the current loop body
	byte *_pSrc = nullptr; // 0x16  current read pointer into the sound-data stream
	byte *_innerLoopPtr = nullptr; // 0x18  inner-loop restart address
	byte *_outerLoopPtr = nullptr; // 0x1A  outer-loop restart address
	byte *_soundData = nullptr; // 0x1C  base address of this channel's sound-data block
	byte *_branchTargetPtr = nullptr; // 0x1E  target of the most recent branch/jump opcode

	// ---- word counter fields (offsets 0x20 - 0x29) ----------------------

	uint16 _innerLoopCount = 0; // 0x20  remaining inner-loop iterations (0 = infinite until opcode)
	uint16 _outerLoopCount = 0; // 0x22  remaining outer-loop iterations (0 = infinite until opcode)
	uint16 _noiseFreqMask = 0; // 0x24  AND mask applied to the random number in noise mode
	uint16 _freqAccum = 0; // 0x26  frequency sweep accumulator (base frequency + swept offset)
	uint16 _freqStep = 0; // 0x28  per-tick increment added to _freqAccum during a sweep

	// ---- sweep / pause state (offsets 0x2A - 0x2D) ----------------------

	// 0x2A-0x2B  Last computed OPL total-level bytes written by writeVolume.
	//            lo byte (0x2A) = carrier TL nibble; hi byte (0x2B) = modulator TL nibble.
	//            Written at the end of asound_writeVolume so that command7 (resume) can
	//            restore operator levels without a full recalculation.
	uint8  _cachedCarrierTL = 0; // 0x2A  cached carrier total-level (6-bit, 0 = loudest)
	uint8  _savedFreqSweep = 0; // 0x2B  copy of _freqSweepCounter saved by command6 (pause)
	uint8  _freqSweepCounter = 0; // 0x2C  countdown for frequency-sweep ticks; 0 = sweep done
	uint8  _savedSweepCounter = 0; // 0x2D  second save slot: _freqSweepCounter is mirrored here
	//       by command6/7 so command7 can restore the live value

// ---- tuning / control (offsets 0x2E - 0x31) ------------------------

	uint8  _transpose = 0; // 0x2E  semitone offset added when looking up _semitoneFreqTable
	uint8  _octaveTranspose = 0; // 0x2F  octave shift: added to _note before octave calculation
	uint8  _pendingStop = 0; // 0x30  0xFF = channel is fading out and will go idle when silent
	uint8  _vibratoMode = 0; // 0x31  0xFF = one-shot vibrato (clamp & stop); else = continuous

	/**
	 * Zeroes the "live" fields of one channel without touching the
	 * loop/sound-data pointers (those are handled separately by load).
	 */
	void reset();

	/**
	 * Clears the entire channel struct, then initialises all loop/source
	 * pointers to the supplied sound-data address, sets _patchAttenuation to
	 * 0x40 (maximum, silent), and marks the channel active (_activeCount = 1).
	 */
	void load(byte *soundData);

	/**
	 * Redirects _loopStartPtr and _pSrc to the null/silence stream and arms a
	 * one-step volume fade (_volumeFadeStep = 0xFF, _fadePeriodReload/Counter = 1)
	 * so the channel fades out gracefully over the next tick.
	 */
	void setPtr2(byte *ptr);

	/**
	 * Marks the channel pending-stop (_pendingStop = 0xFF) and redirects
	 * _soundData to the null/silence stream.  Unlike setPtr2 this does not
	 * touch _volumeFadeStep; the channel fades naturally via processChannelFade.
	 */
	void enable();

	/**
	 * Called every frame for each channel.  When _pendingStop is set, waits
	 * until both _velocity and _volume are zero before marking the channel idle
	 * (_activeCount = 0); otherwise applies _volumeFadeStep to velocity/volume
	 * each _fadePeriodReload ticks and clamps the result to [0, 0x7F].
	 */
	void processChannelFade();
};

// ---------------------------------------------------------------------------
// AdlibSample  (unchanged from Return of the Phantom, 0x16 bytes)
// ---------------------------------------------------------------------------
struct AdlibSample {
	uint8  _attackRate = 0;
	uint8  _decayRate = 0;
	uint8  _sustainLevel = 0;
	uint8  _releaseRate = 0;
	uint8  _egTyp = 0;
	uint8  _ksr = 0;
	uint8  _totalLevel = 0;
	uint8  _scalingLevel = 0;
	uint8  _waveformSelect = 0;
	uint8  _freqMultiple = 0;
	uint8  _feedback = 0;
	uint8  _ampMod = 0;
	uint8  _vib = 0;
	uint8  _alg = 0;
	uint8  _freqSweepInit = 0; // initial _freqSweepCounter value; 0 = immediate key-on
	uint8  _reserved = 0;
	uint16 _freqMask = 0; // loaded into channel _noiseFreqMask
	uint16 _freqBase = 0; // loaded into channel _freqAccum
	uint16 _outerLoopPtr = 0; // loaded into channel _freqStep

	AdlibSample() {
	}
	AdlibSample(Common::SeekableReadStream &s);
};

// ---------------------------------------------------------------------------
// ASound  -  Dragonsphere Adlib sound driver base class
//
// Command dispatch table layout (from off_11A14 / off_11A26 / off_11A2E /
// funcs_12251 / off_11A64 in the disassembly):
//
//   Table 1  off_11A14  commands  0- 8   (max=8,    base=0,    9 entries)
//   Table 2  off_11A26  commands 16-19   (max=0x13, base=0x10, 4 entries)
//             command16  = background-music dispatcher (calls command18)
//             command17  = play specific piece (loads 7 channels direct)
//             command18  = re-entrant music launcher (reads word_12370 to pick a sub-command)
//             command19  = no-op (asound_command98)
//   Table 3  off_11A2E  commands 24-32   (max=0x20, base=0x18, 9 entries)
//   Table 4  funcs_12251 commands 32-49  (max=0x31, base=0x20, 18 entries)
//             Includes asound_command32-48 (music pieces / SFX loaders)
//             and asound_command98 (no-op) in the last slot.
//   Table 5  off_11A64  commands 64-101  (max=0x65, base=0x40, 38 entries)
//             Includes asound_command64-101 (single-shot SFX loaders via
//             findFreeChannel / findFreeChannelFull) and two no-ops.
//
// The driver also exposes:
//   asound_command90 / 91  - two-voice SFX (findFreeChannelFull x2)
//   asound_command95       - four-voice music piece (findFreeChannel x4)
//   sub_11F98              - two-voice SFX (findFreeChannelFull x2)
//
// word_12370 tracks the "current music index" used by command18 to select
// which music-piece loader to call.
// ---------------------------------------------------------------------------
class ASound : public SoundDriver {
protected:
	/** Member-function pointer type for deferred sound-loader callbacks. */
	typedef void (ASound::*CallbackFunction)();

private:
	// ---- callback / tick state ------------------------------------------
	uint16 _callbackCounter = 0;  // per-tick countdown
	uint16 _callbackPeriod = 0;  // reload value; 0 = callback disabled
	// Pointer to the deferred sound-loader called when the
	// counter fires (stored as uint16 in the original; typed as a member
	// function pointer in the C++ port).
	CallbackFunction _callbackFnPtr = nullptr;

	// ---- active-channel context (set by pollAllChannels) ----------------
	AdlibChannel *_activeChannelPtr = nullptr;
	uint8          _activeChannelNumber = 0;
	uint16         _activeChannelReg = 0; // 0xA0+ch register for the active channel
	uint16         _currentOpBase = 0; // operator register base for the active channel

	// ---- per-frame working state ----------------------------------------
	AdlibSample *_samplePtr = nullptr; // patch being loaded/written
	byte *pSrc = nullptr; // current read pointer (mirrors channel _pSrc)
	int16          _pollResult = 0;
	int16          _resultFlag = 0;
	uint16         _randomSeed = 0x4D2;

	// ---- driver-wide flags ----------------------------------------------
	uint16 _isDisabled = 0; // non-zero while the engine is paused (command6)
	uint8  _findChannelMode = 0; // 0=full search, 1=ch0-5 only, 2=ch6-8 then pending

	// ---- per-channel sweep shadows (for channel 5 special-casing) -------
	//      Not present as separate globals in Dragonsphere; the channel
	//      struct fields _savedFreqSweep / _savedSweepCounter handle this.

	// ---- misc globals ---------------------------------------------------
	uint8  _anySweepActive = 0; // set to 1 if any channel has _freqSweepCounter > 0
	int    _frameNumber2 = 0; // secondary frame counter incremented every update

	// ---- script / sequencer registers -----------------------------------
	uint8  _scriptVars[32] = {}; // byte_16A10: 32 general-purpose script registers

	// ---- music-index tracker (word_12370) --------------------------------
	// Tracks which music piece was last launched by command18.
	uint16 _musicIndex = 0;

	// ---- tempo / sequencer state (from opcodes3 group 2 handlers) --------
	uint8  _musicOnlyFlag = 0;     // byte_12393: 1=music-only check, 0=all channels
	uint16 _tempoFineStep = 0;     // word_124F2: fine tempo step (opcode A6)
	uint16 _tempoCoarseStep = 0;   // word_124F0: coarse tempo step (opcode A7)
	uint16 _tempoPeriod = 0;       // word_124EE: tempo period in ticks (opcode A8)
	uint8  _tempoEnabled = 0;      // non-zero when tempo tick is active (opcode A8)
	uint16 _tempoTickCounter = 0;  // countdown for tempo callback
	uint16 _tempoShift = 0;        // word_124F4: tempo shift (opcode A9)

	// =========================================================================
	// Private helpers
	// =========================================================================

	/** Timer callback entry point; calls update(). */
	void onTimer();

	/** Zeros _callbackFnPtr, _callbackCounter, and _callbackPeriod. */
	void clearCallback();
	void resetCallback() {
		clearCallback();
	}

	/**
	 * Forces one OPL operator to maximum attenuation (total-level = 0x3F)
	 * while preserving its upper KSL bits.  'portIndex' is the register index
	 * into _adlibPorts (i.e. the 0x40-range operator register).
	 * Used by command6 to mute all operator pairs during a pause.
	 */
	void adlib_channelOff(uint8 portIndex);

	/**
	 * Computes and writes the total-level registers for the active channel
	 * (asound_writeVolume).
	 *
	 * Two top-level paths depending on the _alg field of the first sample:
	 *   _alg == 0  (FM): both operators contribute; loop twice writing
	 *              VOICE_SLOTS[ch][0] then VOICE_SLOTS[ch][1].
	 *   _alg != 0  (additive): only the carrier (slot 1) carries volume.
	 *
	 * Within each pass, two inner paths based on OPL version:
	 *   < 0x18  (OPL2): TL = clamp(0x3F - var_2, 0, 0x3F).
	 *   >= 0x18 (OPL3): patch-attenuation-aware mapping using
	 *              PATCH_ATTEN_TO_TL and VOL_VEL_TO_ATTEN_STEP tables;
	 *              writes two registers (offset 0 and offset 2).
	 *
	 * Caches the two written TL bytes in _cachedCarrierTL / _savedFreqSweep
	 * so command7 can restore levels without a full recalculation.
	 */
	void writeVolume();

	/**
	 * Derives the OPL F-number and block (octave) from _note, _octaveTranspose,
	 * and _transpose using _semitoneFreqTable, then writes registers 0xA0+ch
	 * and 0xB0+ch (with key-on bit set).
	 */
	void writeFrequency();

	/**
	 * Applies _pitchBend as a signed offset to the already-computed frequency
	 * registers without a full note recalculation.
	 */
	void writePitchBend();

	/**
	 * Arpeggio frequency write (sub_11856).
	 * Called from pollActiveChannel when _writeVolumePending (field_11) is set.
	 * Computes a modified frequency from _note + _octaveTranspose + field_11 - 1
	 * and writes it to the OPL registers, preserving the key-on bit.
	 */
	void writeArpeggio();

	/**
	 * Clears the key-on bit (bit 5) of the 0xB0+ch register, silencing the
	 * note while preserving pitch.  Called during _keyOnDelay countdown.
	 */
	void updateOctave();

	/**
	 * Arms the frequency sweep counter from the sample definition, then - if
	 * _freqSweepCounter is zero - immediately triggers the note by writing
	 * volume and frequency registers and setting the key-on bit.
	 */
	void noteOn();

	/**
	 * Writes all OPL operator registers for the patch pointed to by _samplePtr,
	 * using _currentOpBase as the operator base.  Covers ADSR, KSL/TL,
	 * feedback/algorithm, AM/VIB flags, and waveform-select registers.
	 */
	void writeSampleRegs();

	/**
	 * Loads all OPL registers for the patch assigned to the active channel,
	 * covering both modulator and carrier operators, and sets up the channel's
	 * sweep/frequency state from the AdlibSample definition.
	 */
	void loadSample();

	/**
	 * Main per-frame update: increments frame counters, polls all channels,
	 * fires the tick callback, calls updateAllChannels, then runs the per-channel
	 * frequency-sweep tick for all 9 voices.
	 */
	void update();

	/**
	 * Per-channel frequency-sweep tick for channel 'channelIndex' (0-based).
	 * When _freqSweepCounter > 0: adds _freqStep to _freqAccum, decrements the
	 * counter, and - when it reaches zero - zeroes the voice's frequency registers.
	 * Sets _anySweepActive when the counter is still non-zero.
	 */
	void update1(int channelIndex);

	/**
	 * Writes a frequency value directly to the OPL registers for 'voice'.
	 * The high byte selects the block/octave; the low byte is the F-number LSB.
	 */
	void setFrequency(uint8 voice, uint16 freq);

	/**
	 * Noise-channel inner tick: picks a random frequency offset masked by
	 * _noiseFreqMask, adds _freqAccum, and writes the result to the voice
	 * frequency registers.
	 */
	void noise_inner(int channelIndex);

	/**
	 * Calls processChannelFade for all 9 channels each frame.
	 */
	void updateAllChannels();

	/**
	 * Advances pSrc by one, reads two bytes little-endian, and returns the word.
	 * Used by the bytecode opcodes that take 16-bit operands.
	 */
	uint16 readWord_impl();

	/**
	 * Deferred-callback tick: decrements _callbackCounter; when it reaches zero,
	 * reloads it from _callbackPeriod and calls _callbackFnPtr (if non-null),
	 * then clears _callbackFnPtr so it fires exactly once.
	 */
	void tickCallback();

protected:
	// ---- nine AdlibChannel instances ------------------------------------
	AdlibChannel _channel0, _channel1, _channel2;
	AdlibChannel _channel3, _channel4, _channel5;
	AdlibChannel _channel6, _channel7, _channel8;
	AdlibChannel *_channels[ADLIB_CHANNEL_COUNT] = {
		&_channel0, &_channel1, &_channel2,
		&_channel3, &_channel4, &_channel5,
		&_channel6, &_channel7, &_channel8
	};

	uint8  _adlibPorts[0x100] = { 0 };
	Common::Array<AdlibSample> _samples;

protected:
	// =========================================================================
	// Protected helpers  (used by ASound1–ASound9)
	// =========================================================================

	/**
	 * Checks whether any of channels 0-6 (or 0-8 when _musicOnlyFlag is clear)
	 * have a non-zero _activeCount.  Returns non-zero if sound is playing.
	 * This is 'sub_1061A' in the disassembly.
	 */
	int isMusicChannelsActive();

	/**
	 * Like isMusicChannelsActive but scans all 9 channels unconditionally
	 * (clears the ch0-6-only flag first).  This is 'sub_1064E'.
	 */
	int isAnyChannelActive();

	/**
	 * Schedule fn as the next deferred-load callback.
	 * Does NOT touch _callbackCounter or _callbackPeriod — those are preserved
	 * from the previous loader so the callback fires on the right beat.
	 * Cast the derived-class member-function pointer with reinterpret_cast.
	 */
	void scheduleCallback(CallbackFunction fn) { _callbackFnPtr = fn; }

	/**
	 * Arm the periodic timer and clear any pending callback pointer.
	 * Call at the head of every immediate-load function (symmetric counter/period).
	 */
	void resetCallbackTimer(uint16 period) {
		_callbackFnPtr = nullptr;
		_callbackCounter = period;
		_callbackPeriod = period;
	}

	/**
	 * Arm the periodic timer with separate counter and period values.
	 * Used by command44 which sets counter=0x60 but period=0xE0.
	 */
	void resetCallbackTimerEx(uint16 counter, uint16 period) {
		_callbackFnPtr = nullptr;
		_callbackCounter = counter;
		_callbackPeriod = period;
	}

	/** Set the music-piece index (word_12370) read by command18. */
	void setMusicIndex(uint16 idx) { _musicIndex = idx; }
	/** Read the current music-piece index. */
	uint16 getMusicIndex() const { return _musicIndex; }

	/** Write one script-variable register (byte_16A10[idx]). */
	void setScriptVar(int idx, uint8 val) { _scriptVars[idx] = val; }
	/**
	 * Writes (reg, value) to the OPL chip and updates the _adlibPorts shadow
	 * array so subsequent reads return the last-written value.
	 */
	void write(uint8 reg, uint8 value);

	/** Updates and returns _randomSeed using a simple linear-feedback shift. */
	uint16 getRandomNumber();

	/**
	 * Restores one OPL operator by re-writing the value already stored in the
	 * _adlibPorts shadow array.  command7 uses this to resume playback after
	 * a command6 pause without recalculating any TL values.
	 * 'portIndex' is the operator register index into _adlibPorts.
	 */
	void adlib_channelOn(uint8 portIndex);

	/** Sets _pollResult and _resultFlag to indicate that sound is playing. */
	void signalSoundPlaying();

	/**
	 * Iterates over all 9 channels, sets _activeChannelPtr / _activeChannelNumber,
	 * and calls pollActiveChannel for each one.
	 */
	void pollAllChannels();

	/**
	 * Per-channel bytecode interpreter, called once per frame per active channel.
	 *
	 * Sound data is a sequence of (note, duration) byte pairs plus command bytes
	 * with the high bit set (0x80-0xFF).  The upper nibble of a command byte
	 * (after masking out the high bit) selects one of seven opcode groups:
	 *
	 *   0x0_  -> opcodes1  (patch/velocity/volume/vibrato/transpose/arpeggio)
	 *   0x1_  -> opcodes2  (inner/outer loop control, restart, branch/call)
	 *   0x2_  -> opcodes3  (tempo, script-variable arithmetic, call-by-address)
	 *   0x3_  -> opcodes4  (script-variable load/store/copy/inc/dec)
	 *   0x4_  -> opcodes5  (script-variable ALU: add/sub/mul/div with imm or var)
	 *   0x5_  -> opcodes6  (extended: random-range, indexed table read/write)
	 *   0x6_  -> opcodes7  (driver-level calls: command dispatch, etc.)
	 *
	 * The lower nibble is passed as the sub-opcode to each group handler.
	 * Note bytes (high bit clear) consume one duration tick per call.
	 */
	void pollActiveChannel();

	/** Returns true if the sound data block at 'ptr' is already playing. */
	bool isSoundActive(byte *ptr) const;

	/**
	 * Scans channels 0-6 for an empty slot (_activeCount == 0) and calls
	 * load() on the first one found.  Falls through to findFreeChannelFull
	 * for channels 6-8 when _findChannelMode != 1.
	 */
	void findFreeChannel(byte *soundData);

	/**
	 * Extends the search to channels 7-8, then checks for pending-stop
	 * channels (which can be pre-empted), working in reverse priority order
	 * (ch8, ch7, ch6 ... ch0).
	 */
	void findFreeChannelFull(byte *soundData);

	/** Returns a pointer to the sound data at the given offset. */
	byte *loadData(int offset) {
		return &_soundData[offset];
	}

protected:
	// =========================================================================
	// Core commands (0-8)  -  identical in purpose to Return of the Phantom
	// =========================================================================

	/**
	 * command0: Full hardware reset.
	 *   1. Reset all 9 channels.
	 *   2. Mute all operator TL registers (0x40-0x55) to 0x3F.
	 *   3. Zero remaining operator registers (0x60-0xFF and 0x01-0x3F).
	 *   4. Write Waveform Select Enable (register 0x01 = 0x20).
	 *   5. Reset the tick callback.
	 */
	int command0();

	/**
	 * command1: Fade out all channels.
	 *   Calls command3 (fade music channels 0-6) then command5 (fade SFX 7-8).
	 */
	int command1();

	/**
	 * command2: Fade out music channels 0-6.
	 *   Calls AdlibChannel::setPtr2 on each, redirecting to the null stream
	 *   and arming a one-step fade.
	 */
	int command2();

	/**
	 * command3: Fade out music channels 0-6 with pending-stop.
	 *   Calls AdlibChannel::enable on channels 0-6.
	 */
	int command3();

	/**
	 * command4: Fade out SFX channels 7-8.
	 *   Calls AdlibChannel::setPtr2 on channels 7 and 8.
	 */
	int command4();

	/**
	 * command5: Stop SFX channels 7-8 with pending-stop flag.
	 *   Calls AdlibChannel::enable on channels 7 and 8, letting each channel
	 *   finish its current OPL envelope before going idle.
	 */
	int command5();

	/**
	 * command6: Pause playback.
	 *   Saves each channel's _freqSweepCounter into _savedSweepCounter, zeroes
	 *   _freqSweepCounter on all channels, then mutes all 22 operator TL
	 *   registers (the byte_1239B table covers all operator slots 0x40-0x55).
	 *   Sets _isDisabled to prevent further updates.
	 */
	int command6();

	/**
	 * command7: Resume playback.
	 *   Restores operator volumes for all channels from _adlibPorts shadow,
	 *   copies _savedSweepCounter back to _freqSweepCounter on all channels,
	 *   signals sound playing if any channel was active, then clears _isDisabled.
	 */
	int command7();

	/**
	 * command8: Returns non-zero if any of the 9 channels has a non-zero
	 *   _activeCount (i.e. sound is currently playing).
	 *   Also clears the music-only flag (byte_12393 = 0) so the check covers
	 *   all 9 channels.
	 */
	int command8();

	/**
	 * Calls a function at a fixed offset within the sound driver.
	 * @param offset		Offset of the function
	 */
	virtual void callFunction(uint16 offset);

	// =========================================================================
	// Music-index launcher (called via command18)
	// =========================================================================

	/**
	 * command18: Re-entrant music launcher.
	 *   First calls command1 to fade current output, then branches on _musicIndex
	 *   (word_12370):
	 *     <= 0x12  -> calls off_11A26 table (commands 16-19)
	 *     > 0x12  -> calls funcs_12251 table (commands 32-49), index = _musicIndex - 0x20
	 */
	int command18();

public:
	/**
	 * Constructor.
	 * @param mixer       Mixer instance
	 * @param opl         OPL chip instance
	 * @param filename    Path to the .DR1 (or equivalent) sound-driver file
	 * @param dataOffset  Offset in the file of the data segment
	 * @param dataSize    Size of the data segment
	 */
	ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
		int dataOffset, int dataSize);

	~ASound() override {
	}

	/** Stop all currently playing sounds (wraps command0). */
	virtual int stop() override;

	/** Main poll method; drives the per-frame update. */
	int poll() override;

	/**
	 * Noise channel tick: for each of the 9 channels calls noise_inner,
	 * which randomises the voice frequency each frame using _noiseFreqMask.
	 */
	void noise() override;

	/**
	 * Starts playback of the sound data at the given byte offset within the
	 * driver's data segment, using findFreeChannelFull to select a channel.
	 */
	void playSound(int offset);

	void setVolume(int volume) override {
		// TODO: implement if needed
	}
};

} // namespace Dragonsphere
} // namespace MADS

#endif
