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

#ifndef MADS_CORE_ASOUND_H
#define MADS_CORE_ASOUND_H

#include "common/mutex.h"
#include "common/queue.h"
#include "common/util.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace MADSV2 {

#define ADLIB_CHANNEL_COUNT 9

struct AdlibChannel {
	static bool _isDisabled;
	uint8  _activeCount = 0;
	uint8  _pitchBend = 0;
	uint8  _volumeFadeStep = 0;
	uint8  _vibratoDepth = 0;
	uint8  _note = 0;
	uint8  _sampleIndex = 0;
	uint8  _volume = 0;
	uint8  _noteOffset = 0;
	uint8  _keyOnDelay = 0;
	uint8  _fadePeriodCounter = 0;
	uint8  _fadePeriodReload = 0;
	uint8  _vibPeriodCounter = 0;
	uint8  _vibPeriodReload = 0;
	uint8  _patchAttenuation = 0;
	byte *_loopStartPtr = nullptr;
	byte *_pSrc = nullptr;
	byte *_innerLoopPtr = nullptr;
	byte *_outerLoopPtr = nullptr;
	byte *_soundData = nullptr;
	byte *_branchTargetPtr = nullptr;
	uint16 _innerLoopCount = 0;
	uint16 _outerLoopCount = 0;
	uint16 _noiseFreqMask = 0;
	uint16 _freqAccum = 0;
	uint16 _freqStep = 0;
	uint8  _freqSweepCounter = 0;
	uint8  _savedFreqSweep = 0;
	uint8  _transpose = 0;
	uint8  _velocity = 0;
	uint8  _opVolBytes = 0;
	uint8  _opVolBytes2 = 0;
	uint8  _octaveTranspose = 0;
	uint8  _pendingStop = 0;
	uint8  _durationOverride = 0;

	/**
	 * Zeroes the "live" fields of one channel without touching loop/sound-data
	 * pointers (those are handled separately by AdlibChannel_load).
	 */
	void reset();

	/**
	 * Clears the entire channel struct, then initialises the loop/source
	 * pointers to the supplied sound-data address and marks the channel active.
	 */
	void load(byte *soundData);

	/**
	 * Point a channel at the null/silence data and
	 * arm a one-step volume fade so it fades out gracefully.
	 */
	void setPtr2(byte *ptr);

	/**
	 * Mark a channel as pending-stop and redirect its
	 * sound-data pointer to the null/silence stream.
	 */
	void enable();

	/**
	 * Called every frame for each channel.  When _pendingStop is set,
	 * waits until both velocity and volume are zero before marking the channel
	 * idle; otherwise arms a fade-down of velocity/volume each period.
	 */
	void processChannelFade();
};

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
	uint8  _freqSweepInit = 0;
	uint8  _reserved = 0;
	uint16 _freqMask = 0;
	uint16 _freqBase = 0;
	uint16 _outerLoopPtr = 0;

	AdlibSample() {}
	AdlibSample(Common::SeekableReadStream &s);
};

class ASound : public SoundDriver {
private:
	Common::Queue< Common::Pair<byte, byte> > _queue;
	uint16 _callbackCounter = 0;		// Period counter
	uint16 _callbackPeriod = 0;			// Period reload
	AdlibChannel *_activeChannelPtr = NULL;
	uint8 _activeChannelNumber = 0;
	uint16 _activeChannelReg = 0;
	uint16 _currentOpBase = 0;			// Current operator base
	AdlibSample *_samplePtr = NULL;
	byte *pSrc = nullptr;				// Current read pointer
	int16 _pollResult = 0;
	int16 _resultFlag = 0;
	uint16 _randomSeed = 0x4D2;
	uint8  _isDisabled = 0;
	uint8  _findChannelMode = 0;		// findFreeChannel mode
	uint8  _ch5SweepLive = 0;			// Channel5 savedFreqSweep shadow
	uint8  _ch5SweepSaved = 0;			// Channel5 savedFreqSweep shadow 2
	uint8  _ch5PendingStop = 0;			// Channel5 pendingStop shadow
	uint8 _anySweepActive = 0;			// any-sweep-active flag
	int _frameNumber2 = 0;
	uint8 _scriptVars[32];				// General-purpose script registers
	uint16 _tickEnabled = 1;
	uint16 _tickCounter = 0;
	uint16 _tempoReload = 0;
	uint16 _tempoTarget = 0;
	uint16 _tempoShift = 0;
	uint16 _tempoBase = 0xA0;
	uint16 _tempoCurrent = 0x28;
	uint16 _tempoScale = 0x0A;

	/**
	 * Timer function for OPL
	 */
	void onTimer();

	/**
	 * Clears the periodic tick-callback state.
	 */
	void clearCallback();
	void resetCallback() {
		clearCallback();
	}

	/**
	 * Mutes one output operator by setting its total-level to maximum (0x3F)
	 * while preserving the upper KSL bits.
	 * 'portIndex' is the base port index for that operator (from _adlibPorts).
	 */
	void adlib_channelOff(uint8 portIndex);


	/**
	 * Computes and writes the total-level (volume) registers for the current
	 * active channel to the OPL chip.
	 *
	 * Two paths depending on _adlib_v5660_2:
	 *   < 0x18  -> simple linear mapping of (volume + velocity) -> TL
	 *   >= 0x18 -> patch-attenuation-aware mapping using _patchAttenToTL table
	 */
	void writeVolume();

	/**
	 * Computes the OPL F-number / block (octave) from the channel's note,
	 * octaveTranspose and transpose, then writes registers 0xA0+ch and 0xB0+ch.
	 */
	void writeFrequency();

	/**
	 * Applies the channel's pitchBend offset to the current frequency registers
	 * without recalculating from the note table.
	 */
	void writePitchBend();

	/**
	 * Clears the key-on bit (bit 5) of the 0xB0+ch register to silence the
	 * note without changing pitch (used during note-delay countdown).
	 */
	void updateOctave();

	/**
	 * Writes volume, loads the freq-sweep counter from the sample definition,
	 * and - if the sweep counter is zero - triggers the note (writes frequency
	 * and sets the key-on bit in the 0xB0+ch register).
	 */
	void noteOn();

	/**
	 * Writes a full set of OPL operator registers for the sample pointed to
	 * by the global asound_samplePtr, using _currentOpBase as the operator base.
	 */
	void writeSampleRegs();

	/**
	 * Loads all OPL registers for the patch assigned to the current channel,
	 * covering both the modulator and carrier operators.
	 */
	void loadSample();

	/**
	 * Main sound update function
	 */
	void update();

	/**
	 * Per-channel frequency-sweep tick.  Called once per frame per channel.
	 * When freqSweepCounter > 0: accumulates freqStep into freqAccum and
	 * decrements the counter.  When it reaches zero, zeroes the frequency
	 * registers for that voice.
	 */
	void update1(int channelIndex);

	/**
	 * Sets voice frequency
	 */
	void setFrequency(uint8 voice, uint16 freq);

	/**
	 * Noise channel : each tick picks a random frequency offset masked by
	 * noiseFreqMask, adds freqAccum, and writes the result.
	 */
	void noise_inner(int channelIndex);

	/**
	 * Calls processChannelFade for all 9 channels each frame.
	 */
	void updateAllChannels();

	/**
	 * Advances pSrc by one, reads two bytes little-endian, returns the word.
	 */
	uint16 readWord_impl();

protected:
	AdlibChannel _channel0, _channel1, _channel2;
	AdlibChannel _channel3, _channel4, _channel5;
	AdlibChannel _channel6, _channel7, _channel8;
	AdlibChannel *_channels[ADLIB_CHANNEL_COUNT] = {
		&_channel0, &_channel1, &_channel2, &_channel3, &_channel4,
		&_channel5, &_channel6, &_channel7, &_channel8
	};

	uint8  _adlibPorts[0x100] = { 0 };
	Common::Array<AdlibSample> _samples;

protected:
	/**
	  * write(register, value)
	  *
	  * Updates the local port-shadow array so that subsequent reads from
	  * _adlibPorts[reg] return the last-written value, then sends the byte
	  * pair to the real hardware (or whatever back-end is wired in).
	  */
	void write(uint8 reg, uint8 value);

	/**
	 * Flush any pending Adlib register values to the OPL driver
	 */
	void flush();

	/**
	 * Updates and returns _asound_randomSeed.
	 */
	uint16 getRandomNumber();

	/**
	 * Restores one output operator: clears the forced-max bits (0x3F) and
	 * writes the caller-supplied volume nibble (bh in the asm).
	 * 'portIndex' is the operator register index; 'vol' is the volume nibble.
	 */
	void adlib_channelOn(uint8 portIndex, uint8 vol);

	/**
	 * Sets the poll-result flags to indicate that sound is playing.
	 */
	void signalSoundPlaying();

	/**
	 * Polls all the channels
	 */
	void pollAllChannels();

	/**
	 * Per-channel update, called once per frame by pollAllChannels.
	 * Implements a bytecode interpreter : the sound data stream is a sequence of
	 * (note, duration) pairs plus command bytes(0x80-0xFF). Commands with high
	 * bit clear are notes; high bit set triggers the switch below.
	 *
	 * The interpreter loop re-enters the top of the dispatch after
	 * each command that does not consume a duration tick. Note bytes consume
	 * one tick per call (activeCount counts down).
	  */
	void pollActiveChannel();

	/**
	 * Returns true if a given sound data is already active.
	 */
	bool isSoundActive(byte *ptr) const;

	/**
	 * Channel finder helpers
	 * Scan channels in priority order and call AdlibChannel_load on the first
	 * inactive (or pending-stop) one found.
	 *
	 * Searches channels 0-5 for an empty slot, falling through to
	 * asound_findFreeChannelFull (channels 6-8) if none found.
	 * _findChannelMode == 1 means "only search 0-5".
	  */
	void findFreeChannel(byte *soundData);

	/**
	 * Extends the search to channels 6-8 and then checks for pending-stop
	 * channels (which can be pre-empted), in reverse priority order.
	 */
	void findFreeChannelFull(byte *soundData);

	/**
	 * Returns data for the specified offset.
	 */
	byte *loadData(int offset, int /*size*/) {
		return &_soundData[offset];
	}

protected:
	/**
	 * Silences every voice and mutes the hardware:
	 *   1. Reset all 9 channels.
	 *   2. Mute all operator total-level registers (0x40-0x55) to max attenuation.
	 *   3. Zero remaining operator registers (0x60-0xFF and 0x01-0x3F).
	 *   4. Write Waveform Select Enable (register 0x01 = 0x20).
	 *   5. Reset the tick callback.
	 */
	int command0();

	 /*
	  * Fade out all channels (calls command2 + command4 via
	  * the command3/command5 helpers that enable pending-stop on each channel).
	  */
	int command1();

	/**
	 * Fade out music channels 0-5.
	 */
	int command2();

	/**
	 * Fade out low channels
	 */
	int command3();

	/**
	 * Fade out SFX channels 6-8.
	 */
	int command4();

	/*
	 * Stop SFX channels (6-8) with pending-stop flag.
	 *
	 * Marks each of the three SFX channels (6, 7, 8) as pending-stop by setting
	 * their _pendingStop field to 0xFF and redirecting their sound-data pointer
	 * to the null/silence stream.  Channels that are already inactive
	 * (_activeCount == 0) are skipped.
	 *
	 * Unlike command4 (which forces an immediate fade-out via AdlibChannel_setPtr2),
	 * this command lets each channel finish its current envelope naturally:
	 * asound_processChannelFade checks _pendingStop each frame and only silences
	 * the channel once both _velocity and _volume have reached zero.
	 */
	int command5();

	/**
	 * Saves each channel's current freqSweepCounter into savedFreqSweep,
	 * zeroes freqSweepCounter on all channels, then mutes the output operators
	 * for voices 3-5 (the secondary operator set at 0x43-0x55).
	 */
	int command6();

	/**
	 * Resume playback after a pause.
	 * Restores operator volumes for channels 0-5, re-saves sweep counters from
	 * the live values (which were frozen to 0 by command6), then re-enables
	 * the audio engine.
	 *
	 * The OPL2 voice-to-operator mapping used here:
	 *   voice 0 -> operator index 67  (0x43)
	 *   voice 1 -> operator index 68  (0x44)
	 *   voice 2 -> operator index 69  (0x45)
	 *   voice 3 -> operator index 75  (0x4B)
	 *   voice 4 -> operator index 76  (0x4C)
	 *   voice 5 -> operator index 77  (0x4D)
	 */
	int command7();

	/**
	 * Returns non-zero if any of the 9 Adlib channels has a non-zero
	 * _activeCount (i.e. sound is currently playing).
	 */
	int command8();

	/**
	 * Calls a function at a fixed offset within the sound driver.
	 * @param offset		Offset of the function
	 */
	virtual void callFunction(uint16 offset);

public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param opl			OPL
	 * @param filename		Specifies the adlib sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 */
	ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
		int dataOffset, int dataSize);

	/**
	 * Destructor
	 */
	~ASound() override {}

	/**
	 * Stop all currently playing sounds
	 */
	virtual int stop();

	/**
	 * Main poll method to allow sounds to progress
	 */
	int poll() override;

	/**
	 * Noise channel : each tick picks a random frequency offset masked by
	 * noiseFreqMask, adds freqAccum, and writes the result.
	 */
	void noise() override;

	void setVolume(int volume) override {
		// TODO: Does this need implementation?
	}

	/**
	 * Plays a sound
	*/
	void playSound(int offset, int size);
};

} // namespace MADSV2
} // namespace MADS

#endif
