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

#include "audio/fmopl.h"
#include "mads/madsv2/dragonsphere/asound.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {


bool AdlibChannel::_isDisabled;

/*
  * PATCH_ATTEN_TO_TL  (seg001:0x0092 / offset from _asound_samples base)
  * patchAttenuation (0-127) -> 6-bit OPL total-level.
  * The Dragonsphere asm uses PATCH_ATTEN_TO_TL[bx] for the modulator lookup
  * and unk_12431 - bx (i.e. PATCH_ATTEN_TO_TL[127 - patchAtt]) for the
  * carrier lookup.
  */
static const uint8 PATCH_ATTEN_TO_TL[128] = {
	63, 54, 49, 45, 42, 40, 38, 36, 34, 33, 32, 31, 30, 29, 28, 27,
	26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18,
	18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13,
	12, 12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 10,  9,  9,  9,
	 9,  9,  8,  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,  6,  6,  6,
	 6,  6,  6,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,
	 4,  3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,
	 2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0
};

/*
 * VOL_VEL_TO_ATTEN_STEP  (_volVelToAttenStep)
 * volume or velocity byte (0-127) -> attenuation step (1-32).
 * Groups of 4 inputs map to the same step.
 */
static const uint8 VOL_VEL_TO_ATTEN_STEP[128] = {
	 1, 1, 1, 1,  2, 2, 2, 2,  3, 3, 3, 3,  4, 4, 4, 4,
	 5, 5, 5, 5,  6, 6, 6, 6,  7, 7, 7, 7,  8, 8, 8, 8,
	 9, 9, 9, 9, 10,10,10,10, 11,11,11,11, 12,12,12,12,
	13,13,13,13, 14,14,14,14, 15,15,15,15, 16,16,16,16,
	17,17,17,17, 18,18,18,18, 19,19,19,19, 20,20,20,20,
	21,21,21,21, 22,22,22,22, 23,23,23,23, 24,24,24,24,
	25,25,25,25, 26,26,26,26, 27,27,27,27, 28,28,28,28,
	29,29,29,29, 30,30,30,30, 31,31,31,31, 32,32,32,32
};

/*
 * SEMITONE_FREQ_TABLE  (_semitoneFreqTable)
 * OPL F-number base for each semitone within an octave.
 */
static const uint16 SEMITONE_FREQ_TABLE[12] = {
	0x0200, 0x021E, 0x023F, 0x0261, 0x0285, 0x02AB,
	0x02D4, 0x02FF, 0x032D, 0x035D, 0x0390, 0x03C7
};

/*
 * VOICE_SLOTS  (byte_1239B in the binary, also used as the operator-reg
 * index table for command6/7)
 *
 * Layout for each voice: { slot0 (modulator), slot1 (carrier) }
 * The writeVolume loop uses:
 *   pass 0 -> VOICE_SLOTS[ch][0]  (modulator)
 *   pass 1 -> VOICE_SLOTS[ch][1]  (carrier)
 * The alg!=0 single-op path (loc_11692) goes directly to VOICE_SLOTS[ch][1].
 */
static const uint8 VOICE_SLOTS[ADLIB_CHANNEL_COUNT][2] = {
	{  0,  3 }, {  1,  4 }, {  2,  5 },
	{  6,  9 }, {  7, 10 }, {  8, 11 },
	{ 12, 15 }, { 13, 16 }, { 14, 17 }
};

/*
 * SLOT_TO_REG_OFFSET  (_slotToRegOffset)
 * operator-slot index (0-17) -> OPL register group offset.
 */
static const uint8 SLOT_TO_REG_OFFSET[18] = {
	 0,  1,  2,  3,  4,  5,
	 8,  9, 10, 11, 12, 13,
	16, 17, 18, 19, 20, 21
};

/*
 * byte_1239B  - all 22 operator TL register indices muted/restored by
 * command6 and command7.  They cover every OPL operator slot (0x40-0x55).
 */
static const uint8 ALL_OP_TL_REGS[22] = {
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55
};

/* Null / silence sound-data placeholder stream. */
static uint8 ADLIB_NULLDATA[] = {
	0x00, 0x01, 0xF3, 0x00, 0x00, 0xEF, 0x00, 0x00,
	0xF5, 0x00, 0x00, 0x00, 0xF7, 0x00, 0xF8, 0x1D,
	0xFF, 0x00, 0xF4, 0x6E, 0x2A, 0x1C, 0xF4, 0x5F,
	0x2A, 0x1C, 0x2A, 0x1C, 0x2A, 0x1C, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00
};

/* OPL version flag (_adlib_v5660 / opl_version_flag).
 * 0x18 = OPL3 (patch-attenuation-aware writeVolume path). */
constexpr uint16 OPL_VERSION_FLAG = 0x18;

/* Rhythm-mode flags (from data segment). */
constexpr bool RHYTHM_HI_HAT = true;
constexpr bool RHYTHM_CYMBAL = true;
constexpr bool RHYTHM_ENABLE = true;

AdlibSample::AdlibSample(Common::SeekableReadStream &s) {
	_attackRate = s.readByte();
	_decayRate = s.readByte();
	_sustainLevel = s.readByte();
	_releaseRate = s.readByte();
	_egTyp = s.readByte();
	_ksr = s.readByte();
	_totalLevel = s.readByte();
	_scalingLevel = s.readByte();
	_waveformSelect = s.readByte();
	_freqMultiple = s.readByte();
	_feedback = s.readByte();
	_ampMod = s.readByte();
	_vib = s.readByte();
	_alg = s.readByte();
	_freqSweepInit = s.readByte();
	_reserved = s.readByte();
	_freqMask = s.readUint16LE();
	_freqBase = s.readUint16LE();
	_outerLoopPtr = s.readUint16LE();
}

void AdlibChannel::reset() {
	/* Zero the scalar "live" fields only.
	 *
	 * From AdlibChannel_reset in the binary: clears _activeCount, _pitchBend,
	 * _volumeFadeStep, _vibratoDepth, _arpCounterReload (field_13),
	 * _writeVolumePending (field_11), _transpose, _patchAttenuation,
	 * _pendingStop, _velocity, _volume, _freqSweepCounter, _noiseFreqMask,
	 * _freqAccum, _freqStep, _loopStartPtr, _pSrc.
	 * All other fields are left as-is (they are overwritten by load() or
	 * by the bytecode stream before being read).
	 */
	_activeCount = 0;
	_pitchBend = 0;
	_volumeFadeStep = 0;
	_vibratoDepth = 0;
	_arpCounterReload = 0;   /* field_13 */
	_writeVolumePending = 0;   /* field_11 */
	_transpose = 0;
	_patchAttenuation = 0;
	_pendingStop = 0;
	_velocity = 0;
	_volume = 0;
	_freqSweepCounter = 0;
	_noiseFreqMask = 0;
	_freqAccum = 0;
	_freqStep = 0;
	_loopStartPtr = nullptr;
	_pSrc = nullptr;
}

void AdlibChannel::load(byte *soundData) {
	_isDisabled = true;

	/* Zero all bytes in the struct. */
	memset(this, 0, sizeof(AdlibChannel));

	/* Initialise all loop/source pointers to the supplied data block. */
	_loopStartPtr = soundData;
	_pSrc = soundData;
	_innerLoopPtr = soundData;
	_outerLoopPtr = soundData;
	_soundData = soundData;

	/* Start at maximum attenuation (silent) and mark the channel active. */
	_patchAttenuation = 0x40;
	_activeCount = 1;

	_isDisabled = false;
}

void AdlibChannel::setPtr2(byte *ptr) {
	/* Hard-redirect both read pointers, then arm a one-step fade. */
	_loopStartPtr = ptr;
	_pSrc = ptr;
	_volumeFadeStep = 0xFF;   /* -1 signed: fade down */
	_fadePeriodReload = 1;
	_fadePeriodCounter = 1;
}

void AdlibChannel::enable() {
	/* AdlibChannel_enable: mark pending-stop on active channels only. */
	if (_activeCount == 0)
		return;
	_pendingStop = 0xFF;
	_soundData = ADLIB_NULLDATA;
}

void AdlibChannel::processChannelFade() {
	if (_activeCount == 0)
		return;
	if (_pendingStop == 0)
		return;

	/* Channel has fully faded when both velocity and volume are zero. */
	if (_velocity == 0 && _volume == 0) {
		_loopStartPtr = ADLIB_NULLDATA;
		_pSrc = ADLIB_NULLDATA;
		_pendingStop = 0;
		return;
	}

	/* Arm a quick fade-down (period 2, step -1). */
	_volumeFadeStep = 0xFF;
	_fadePeriodReload = 2;
	if (_fadePeriodCounter == 0)
		_fadePeriodCounter = 1;
}

ASound::ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
	int dataOffset, int dataSize)
	: SoundDriver(mixer, opl, filename, dataOffset, dataSize) {
	AdlibChannel::_isDisabled = false;

	/* Standard OPL timer-reset sequence. */
	write(4, 0x60);
	write(4, 0x80);
	write(2, 0xFF);
	write(4, 0x21);
	write(4, 0x60);
	write(4, 0x80);

	Common::fill(_adlibPorts, _adlibPorts + 256, 0);
	command0();

	_opl->start(new Common::Functor0Mem<void, ASound>(this, &ASound::onTimer),
		CALLBACKS_PER_SECOND);
}

int ASound::stop() {
	command0();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

int ASound::poll() {
	update();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

void ASound::noise() {
	Common::StackLock slock(_driverMutex);
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		noise_inner(i);
}

int ASound::command0() {
	/* Push _isDisabled, temporarily force to 0xFFFF while resetting. */
	uint16 savedDisabled = _isDisabled;
	_isDisabled = 0xFFFF;

	/* 1. Reset all 9 channels. */
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i]->reset();

	/* 2. Mute all operator TL registers (0x40 down to 0x4F, via adlib_write2
	 *    with value 0x3F, stepping from 0x4F down while reg >= 0x40). */
	for (int reg = 0x4F; reg >= 0x40; --reg)
		write((uint8)reg, 0x3F);

	/* 3. Zero registers 0xFF down to 0x60. */
	for (int reg = 0xFF; reg >= 0x60; --reg)
		write((uint8)reg, 0x00);

	/* 4. Zero registers 0x3F down to 0x01. */
	for (int reg = 0x3F; reg >= 0x01; --reg)
		write((uint8)reg, 0x00);

	/* 5. Waveform Select Enable. */
	write(0x01, 0x20);

	/* 6. Reset tick callback. */
	resetCallback();

	_isDisabled = savedDisabled;
	return 0;
}

int ASound::command1() {
	/* Fade all channels: music 0-6 (command3) then SFX 7-8 (command5). */
	command3();
	command5();
	return 0;
}

int ASound::command2() {
	/* Hard-redirect music channels 0-6 to null stream with setPtr2. */
	_channel0.setPtr2(ADLIB_NULLDATA);
	_channel1.setPtr2(ADLIB_NULLDATA);
	_channel2.setPtr2(ADLIB_NULLDATA);
	_channel3.setPtr2(ADLIB_NULLDATA);
	_channel4.setPtr2(ADLIB_NULLDATA);
	_channel5.setPtr2(ADLIB_NULLDATA);
	_channel6.setPtr2(ADLIB_NULLDATA);
	return 0;
}

int ASound::command3() {
	/* Pending-stop music channels 0-6 (natural envelope fade via enable). */
	_channel0.enable();
	_channel1.enable();
	_channel2.enable();
	_channel3.enable();
	_channel4.enable();
	_channel5.enable();
	_channel6.enable();
	return 0;
}

int ASound::command4() {
	/* Hard-redirect SFX channels 7-8. */
	_channel7.setPtr2(ADLIB_NULLDATA);
	_channel8.setPtr2(ADLIB_NULLDATA);
	return 0;
}

int ASound::command5() {
	/* Pending-stop SFX channels 7-8. */
	_channel7.enable();
	_channel8.enable();
	return 0;
}

int ASound::command6() {
	/* Guard: already disabled. */
	if (_isDisabled == 0xFFFF)
		return 0;
	_isDisabled = 0xFFFF;

	/* Save each channel's _freqSweepCounter -> _savedSweepCounter, then zero it. */
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i) {
		AdlibChannel *ch = _channels[i];
		ch->_savedSweepCounter = ch->_freqSweepCounter;
		ch->_freqSweepCounter = 0;
	}

	/* Mute all 22 operator TL registers. */
	for (int i = 0; i < 22; ++i)
		adlib_channelOff(ALL_OP_TL_REGS[i]);

	return 0;
}

int ASound::command7() {
	/* Guard: only resume from a paused state. */
	if (_isDisabled != 0xFFFF)
		return 0;

	/* Restore all operator volumes from the _adlibPorts shadow
	 * (asound_playMusicC: just re-writes whatever was last written). */
	for (int i = 0; i < 22; ++i)
		adlib_channelOn(ALL_OP_TL_REGS[i]);

	/* Restore each channel's sweep counter and check whether any was non-zero. */
	uint8 anySweep = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i) {
		AdlibChannel *ch = _channels[i];
		ch->_freqSweepCounter = ch->_savedSweepCounter;
		anySweep |= ch->_savedSweepCounter;
	}

	if (anySweep != 0)
		signalSoundPlaying();

	_isDisabled = 0;
	return 0;
}

int ASound::command8() {
	/* Returns non-zero if any channel is currently active.
	 * Clears byte_12393 (music-only flag) first so all 9 channels are checked. */
	_musicOnlyFlag = 0;
	uint8 result = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		result |= _channels[i]->_activeCount;
	return result;
}

int ASound::command18() {
	/* Re-entrant background-music launcher (asound_command18 in the binary).
	 * Fades everything, then dispatches back through the command table using
	 * _musicIndex (word_12370) as the command ID. */
	command1();
	return command(_musicIndex, 0);
}

void ASound::callFunction(uint16 offset) {
	error("Unsupported call to sound driver function at offset %.4x", offset);
}

void ASound::write(uint8 reg, uint8 value) {
	_adlibPorts[reg] = value;
	_opl->writeReg(reg, value);
}

void ASound::onTimer() {
	Common::StackLock slock(_driverMutex);
	poll();
}

uint16 ASound::getRandomNumber() {
	/* asound_getRandomNumber: ax = ror3(0x9248 + seed). */
	uint16 ax = (uint16)(0x9248u + _randomSeed);
	ax = (uint16)((ax >> 3) | (ax << 13));
	_randomSeed = ax;
	return ax;
}

void ASound::adlib_channelOff(uint8 portIndex) {
	/* sub_1018F: OR the register with 0x3F (force max attenuation),
	 * then write back both to _adlibPorts and to the OPL chip.
	 * Note: unlike the Phantom driver, the original value is NOT preserved
	 * in _adlibPorts - the ORed value is stored back. */
	uint8 val = _adlibPorts[portIndex] | 0x3F;
	_adlibPorts[portIndex] = val;
	_opl->writeReg(portIndex, val);
}

void ASound::adlib_channelOn(uint8 portIndex) {
	/* asound_playMusicC: re-writes whatever _adlibPorts already holds.
	 * command7 simply replays the shadow without recalculating anything. */
	_opl->writeReg(portIndex, _adlibPorts[portIndex]);
}

void ASound::signalSoundPlaying() {
	/* asound_signalSoundPlaying: set resultFlag/pollResult to non-zero. */
	if (_resultFlag == (int16)0xFFFF)
		return;
	_resultFlag = (int16)0xFFFF;
	_pollResult = (int16)0xFFFF;
}

void ASound::clearCallback() {
	_callbackFnPtr = nullptr;
	_callbackCounter = 0;
	_callbackPeriod = 0;
}

void ASound::tickCallback() {
	if (_callbackPeriod == 0)
		return;
	if (--_callbackCounter != 0)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr == nullptr)
		return;

	auto fn = _callbackFnPtr;
	_callbackFnPtr = nullptr;
	(this->*fn)();
}

void ASound::writeVolume() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;

	/* Compute combined attenuation step from volume and velocity. */
	uint8  volIdx = ch->_volume;
	uint8  velIdx = ch->_velocity;
	int16  volStep = (int16)(uint16)VOL_VEL_TO_ATTEN_STEP[volIdx];
	int16  velStep = (int16)(uint16)VOL_VEL_TO_ATTEN_STEP[velIdx];
	int16  var4 = volStep + velStep - 1;   /* var_4: combined step (shared) */

	/* Check _alg of the first sample to determine loop count. */
	AdlibSample *smpFirst = &_samples[ch->_sampleIndex * 2];
	int passes = (smpFirst->_alg == 0) ? 2 : 1;

	/* si and di track the two TL values written per pass for caching. */
	int16 finalSi = 0, finalDi = 0;

	/* var_6 = loop counter (0, then 1 for the two-pass case).
	 * For alg != 0 we jump straight into the carrier path (var_6 stays at
	 * effectively 1: VOICE_SLOTS[ch][1]). */
	for (int var6 = (passes == 1 ? 1 : 0); var6 < 2; ++var6) {

		/* Reload var_2 = var_4 at the start of each pass (loc_11642). */
		int16 var2 = var4;

		/* Select the operator slot. */
		uint8 slot = VOICE_SLOTS[chanNum][var6];
		uint8 regOff = SLOT_TO_REG_OFFSET[slot];
		uint16 tlReg = (uint16)regOff + 0x40;   /* var_8 */

		/* KSL bits from the port shadow. */
		int16 kslBits = (int16)((uint16)_adlibPorts[tlReg] & 0xC0);   /* var_A */

		int16 si, di;

		if (OPL_VERSION_FLAG < 0x18) {
			/* ---- OPL2 simple path (loc_1167C / loc_1167C equivalent) ---- */
			int16 tl = (int16)0x3F - var2;
			tl |= kslBits;
			si = tl;
			di = tl;
			/* adlib_write2(8, tlReg, tl) */
			write((uint8)tlReg, (uint8)tl);
		} else {
			/* ---- OPL3 patch-attenuation path (loc_115BE / loc_116D4) ---- */
			uint8 pa = ch->_patchAttenuation;

			/* Modulator TL (first register, offset 0): */
			int16 tlMod = (int16)(uint16)PATCH_ATTEN_TO_TL[pa];
			/* -(tlMod - var2) = var2 - tlMod */
			int16 atten1 = var2 - tlMod;   /* ax = si initially */
			si = atten1;
			if (si < 0)       si = 0;
			else if (si > 63) si = 63;
			int16 reg0val = ((int16)0x3F - si) | kslBits;
			si = reg0val;
			write((uint8)tlReg, (uint8)reg0val);

			/* Carrier TL (second register, offset 2):
			 * unk_12431 - bx (where bx = pa) == PATCH_ATTEN_TO_TL[127 - pa]. */
			int16 tlCar = (int16)(uint16)PATCH_ATTEN_TO_TL[127 - pa];
			/* di = var_2 - tlCar  (var_4 for alg!=0, var_2=var_4 reload for alg==0) */
			di = var2 - tlCar;
			int16 diClamped = di;
			if (diClamped < 0)        diClamped = 0;
			else if (diClamped > 0x3F) diClamped = 0x3F;
			int16 reg2val = ((int16)0x3F - diClamped) | kslBits;
			di = reg2val;
			write((uint8)(tlReg + 2), (uint8)reg2val);
		}

		/* For the alg!=0 single-pass case we only run this once (var6==1).
		 * For the alg==0 two-pass case, record both sets. */
		if (var6 == 1) {
			finalSi = si;
			finalDi = di;
		} else {
			/* var6==0: record si/di for the mod pass; di is overwritten on
			 * the carrier pass so we use si for the high byte of field_2A. */
			finalSi = si;
			finalDi = di;
		}
	}

	/* Cache the written TL bytes in field_2A:
	 *   dl = di & 0x3F  (carrier / second register)
	 *   dh = si & 0x3F  (modulator / first register)
	 * Packed as a word at offset 0x2A. */
	ch->_cachedCarrierTL = (uint8)(finalDi & 0x3F);   /* dl -> field_2A low byte  */
	/* The high byte (si & 0x3F) is stored in _savedFreqSweep (offset 0x2B)
	 * by the assembly.  We write it separately to avoid clobbering the real
	 * savedFreqSweep which lives there - the original asm stores both bytes
	 * as a word at [bx+2Ah], meaning field_2A=dl and field_2B=dh=si&0x3F.
	 * In our C++ layout _savedFreqSweep is at 0x2B; we shadow the dh byte
	 * into it only when writing volume, consistent with the original. */
	ch->_savedFreqSweep = (uint8)(finalSi & 0x3F);
}

void ASound::writeFrequency() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;
	uint16 aReg = (uint16)chanNum + 0xA0;
	uint16 bReg = (uint16)chanNum + 0xB0;

	/* Note is 1-based; _octaveTranspose shifts by whole octaves. */
	int note = (int)ch->_note + (int)ch->_octaveTranspose - 1;
	int octave = note / 12;
	int semi = note % 12;
	if (semi < 0) {
		semi += 12; --octave;
	}

	/* F-number from table, with optional signed transpose offset. */
	int16 fnum = (int16)SEMITONE_FREQ_TABLE[semi] + (int16)(int8)ch->_transpose;

	write((uint8)aReg, (uint8)fnum);

	uint8 bVal = _adlibPorts[bReg] & 0x20;          /* preserve key-on bit */
	bVal |= (uint8)((fnum >> 8) & 0x03);            /* F-num bits 9:8 */
	bVal |= (uint8)(((uint8)octave & 0x07) << 2);   /* block/octave bits 4:2 */
	write((uint8)bReg, bVal);
}

void ASound::writePitchBend() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;
	uint16 aReg = (uint16)chanNum + 0xA0;
	uint16 bReg = (uint16)chanNum + 0xB0;

	/* Reconstruct the 10-bit F-number from the port shadow. */
	uint16 fnum = (uint16)_adlibPorts[aReg]
		| ((uint16)(_adlibPorts[bReg] & 0x1F) << 8);

	int16 bent = (int16)fnum + (int16)(int8)ch->_pitchBend;

	write((uint8)aReg, (uint8)bent);

	/* Preserve block + key-on, update F-num[9:8]. */
	uint8 bVal = _adlibPorts[bReg] & 0x20;
	bVal |= (uint8)((bent >> 8) & 0x03);
	write((uint8)bReg, bVal);
}

void ASound::writeArpeggio() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;
	uint16 aReg = (uint16)chanNum + 0xA0;
	uint16 bReg = (uint16)chanNum + 0xB0;

	/* dl = _note + _octaveTranspose + _writeVolumePending - 1 */
	int note = (int)ch->_note + (int)ch->_octaveTranspose
		+ (int)ch->_writeVolumePending - 1;
	int octave = note / 12;
	int semi = note % 12;
	if (semi < 0) {
		semi += 12; --octave;
	}

	uint16 freqEntry = SEMITONE_FREQ_TABLE[semi];
	uint8  fnHigh = (uint8)((freqEntry >> 8) & 0x03);
	uint8  block = (uint8)(octave & 0x07);
	uint8  bVal = (uint8)((block << 2) | fnHigh) | (_adlibPorts[bReg] & 0x20);

	write((uint8)aReg, (uint8)freqEntry);
	write((uint8)bReg, bVal);
}

void ASound::updateOctave() {
	uint8 chanNum = _activeChannelNumber;
	uint16 bReg = (uint16)chanNum + 0xB0;
	write((uint8)bReg, _adlibPorts[bReg] & 0xDF);
}

void ASound::noteOn() {
	AdlibChannel *ch = _activeChannelPtr;
	AdlibSample *smp = &_samples[ch->_sampleIndex];

	writeVolume();

	ch->_freqSweepCounter = smp->_freqSweepInit;

	if (ch->_freqSweepCounter != 0) {
		/* Sweep is pending; signal that sound is active but don't key on yet. */
		signalSoundPlaying();
		return;
	}

	/* No sweep - write frequency and set key-on immediately. */
	writeFrequency();

	uint8 chanNum = _activeChannelNumber;
	uint16 bReg = (uint16)chanNum + 0xB0;
	write((uint8)bReg, _adlibPorts[bReg] | 0x20);
}

void ASound::writeSampleRegs() {
	AdlibSample *smp = _samplePtr;
	uint16 base = _currentOpBase;

	/* 0xBD: rhythm mode flags. */
	uint8 bdVal = _adlibPorts[0xBD] & 0x3F;
	if (RHYTHM_HI_HAT) bdVal |= 0x80;
	if (RHYTHM_CYMBAL) bdVal |= 0x40;
	write(0xBD, bdVal);

	/* 0x08: note-select / rhythm enable. */
	write(0x08, RHYTHM_ENABLE ? 0x40 : 0x00);

	/* 0xC0+voice: feedback and algorithm.
	 * asm: feedback << 1, then alg==1 -> bit0=0, else -> bit0=1. */
	write((uint8)(_activeChannelReg + 0xC0),
		(uint8)((smp->_feedback << 1) | (smp->_alg == 1 ? 0 : 1)));

	/* 0x60+base: attack / decay. */
	write((uint8)(base + 0x60),
		(uint8)(((smp->_attackRate & 0x0F) << 4) | (smp->_decayRate & 0x0F)));

	/* 0x80+base: sustain level / release rate. */
	write((uint8)(base + 0x80),
		(uint8)(((smp->_sustainLevel & 0x0F) << 4) | (smp->_releaseRate & 0x0F)));

	/* 0x20+base: AM/VIB/EGT/KSR/mult flags. */
	uint8 amVal = smp->_freqMultiple & 0x0F;
	if (smp->_egTyp == 1) amVal |= 0x20;
	if (smp->_vib == 1) amVal |= 0x40;
	if (smp->_ksr == 1) amVal |= 0x10;
	if (smp->_ampMod == 1) amVal |= 0x80;
	write((uint8)(base + 0x20), amVal);

	/* 0xE0+base: waveform select. */
	write((uint8)(base + 0xE0), (uint8)(smp->_waveformSelect & 0x03));
}

void ASound::loadSample() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;

	/* Phase 1: Silence modulator operator (slot 0). */
	uint8 mSlot = VOICE_SLOTS[chanNum][0];
	uint8 mRegOff = SLOT_TO_REG_OFFSET[mSlot];
	write((uint8)(mRegOff + 0x80), 0xFF);
	write((uint8)(mRegOff + 0x40), 63);

	/* Phase 2: Silence carrier operator (slot 1). */
	uint8 cSlot = VOICE_SLOTS[chanNum][1];
	uint8 cRegOff = SLOT_TO_REG_OFFSET[cSlot];
	write((uint8)(cRegOff + 0x80), 0xFF);
	write((uint8)(cRegOff + 0x40), 63);

	/* Phase 3: Write first sample (modulator) registers. */
	_activeChannelReg = (uint16)chanNum;
	_currentOpBase = mRegOff;
	_samplePtr = &_samples[ch->_sampleIndex * 2];
	writeSampleRegs();

	/* Phase 4: Write second sample (carrier) registers. */
	_currentOpBase = cRegOff;
	_samplePtr = &_samples[ch->_sampleIndex * 2 + 1];
	writeSampleRegs();

	/* Phase 5: Write carrier TL (KSL | 0x3F for full attenuation initially). */
	write((uint8)(cRegOff + 0x40),
		(uint8)((_samplePtr->_scalingLevel << 6) | 0x3F));

	/* Phase 6: Use the first sample again for the modulator TL and for
	 * sweep/frequency initialisation. */
	AdlibSample *smp3 = &_samples[ch->_sampleIndex * 2];
	uint16        mTLReg = (uint16)mRegOff + 0x40;

	if (smp3->_alg == 0) {
		write((uint8)mTLReg, (uint8)((smp3->_scalingLevel << 6) | 0x3F));
	} else {
		/* alg != 0: negate (totalLevel - 63) to get the TL value. */
		uint8 tl = (uint8)(-(int8)(smp3->_totalLevel - 63));
		write((uint8)mTLReg, (uint8)((smp3->_scalingLevel << 6) | (tl & 0x3F)));
	}

	/* Copy sweep/frequency parameters from the sample into the channel. */
	ch->_freqSweepCounter = smp3->_freqSweepInit;
	ch->_noiseFreqMask = smp3->_freqMask;
	ch->_freqAccum = smp3->_freqBase;
	ch->_freqStep = smp3->_outerLoopPtr;
}

void ASound::update() {
	if (_isDisabled)
		return;

	(void)getRandomNumber();
	++_frameNumber2;

	pollAllChannels();
	tickCallback();       /* asound_updateCallback */
	updateAllChannels();

	_anySweepActive = 0;

	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		update1(i);

	/* If no sweep is active, flag the result as "done". */
	if (!_anySweepActive) {
		if (_resultFlag != (int16)0xFFFF) {
			_resultFlag = (int16)0xFFFF;
			_pollResult = (int16)0xFFFF;
		}
	}
}

void ASound::update1(int channelIndex) {
	/* asound_update1: called with cx = channelIndex+1 in the original loop.
	 * Accesses channel via the adlib_channels pointer table. */
	AdlibChannel *ch = _channels[channelIndex];

	if (ch->_freqSweepCounter == 0)
		return;

	_anySweepActive = 1;
	ch->_freqAccum += ch->_freqStep;
	ch->_freqSweepCounter--;

	if (ch->_freqSweepCounter != 0)
		return;

	/* Sweep finished: write zero frequency to silence this OPL voice. */
	uint8 voice = (uint8)channelIndex;
	write((uint8)(voice + 0xA0), 0x00);
	write((uint8)(voice + 0xB0), 0x00);
}

void ASound::setFrequency(uint8 voice, uint16 freq) {
	/* asound_setFrequency: writes A0+v and B0+v (with key-on). */
	write((uint8)(voice + 0xA0), (uint8)(freq & 0xFF));
	uint8 bVal = (_adlibPorts[voice + 0xB0] & 0x20)
		| (uint8)((freq >> 8) & 0x03)
		| 0x20;
	write((uint8)(voice + 0xB0), bVal);
}

void ASound::noise_inner(int channelIndex) {
	/* adlib_noise_inner */
	AdlibChannel *ch = _channels[channelIndex];

	if (ch->_freqSweepCounter == 0)
		return;

	uint16 rnd = getRandomNumber();
	uint16 freq = (rnd & ch->_noiseFreqMask) + ch->_freqAccum;
	setFrequency((uint8)channelIndex, freq);
}

void ASound::updateAllChannels() {
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i]->processChannelFade();
}

void ASound::findFreeChannel(byte *soundData) {
	_findChannelMode = 0;

	/* Scan channels 0-6 for an empty slot. */
	for (int ch = 0; ch < 7; ++ch) {
		if (_channels[ch]->_activeCount == 0) {
			_channels[ch]->load(soundData);
			return;
		}
	}

	/* _findChannelMode==0: fall through to full search. */
	findFreeChannelFull(soundData);
}

void ASound::findFreeChannelFull(byte *soundData) {
	_findChannelMode = 2;

	/* Scan channels 7-8 for an empty slot. */
	if (_channel7._activeCount == 0) {
		_channel7.load(soundData); return;
	}
	if (_channel8._activeCount == 0) {
		_channel8.load(soundData); return;
	}

	/* Scan ch8 then ch7 for pending-stop (pre-emptible). */
	if (_channel8._pendingStop == 0xFF) {
		_channel8.load(soundData); return;
	}
	if (_channel7._pendingStop == 0xFF) {
		_channel7.load(soundData); return;
	}

	/* If _findChannelMode allows it, scan ch6..ch0 for pending-stop. */
	if (_findChannelMode == 0) {
		if (_channel6._pendingStop == 0xFF) {
			_channel6.load(soundData); return;
		}
		if (_channel5._pendingStop == 0xFF) {
			_channel5.load(soundData); return;
		}
		if (_channel4._pendingStop == 0xFF) {
			_channel4.load(soundData); return;
		}
		if (_channel3._pendingStop == 0xFF) {
			_channel3.load(soundData); return;
		}
		if (_channel2._pendingStop == 0xFF) {
			_channel2.load(soundData); return;
		}
		if (_channel1._pendingStop == 0xFF) {
			_channel1.load(soundData); return;
		}
		if (_channel0._pendingStop == 0xFF) {
			_channel0.load(soundData); return;
		}
	}
}

int ASound::isMusicChannelsActive() {
	_musicOnlyFlag = 1;
	uint8 result = 0;
	for (int i = 0; i <= 6; ++i)
		result |= _channels[i]->_activeCount;
	return result;
}

int ASound::isAnyChannelActive() {
	_musicOnlyFlag = 0;
	uint8 result = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		result |= _channels[i]->_activeCount;
	return result;
}

void ASound::pollAllChannels() {
	_activeChannelNumber = 0;
	for (int ch = 0; ch < ADLIB_CHANNEL_COUNT; ++ch) {
		_activeChannelPtr = _channels[ch];
		pollActiveChannel();
		/* Note: _activeChannelNumber is incremented at the end of
		 * pollActiveChannel, not here. */
	}
}

bool ASound::isSoundActive(byte *ptr) const {
	for (int ch = 0; ch < ADLIB_CHANNEL_COUNT; ++ch) {
		if (_channels[ch]->_activeCount && _channels[ch]->_soundData == ptr)
			return true;
	}
	return false;
}

uint16 ASound::readWord_impl() {
	/* asound_readWord: advance pSrc past the low byte, read lo then hi. */
	uint16 lo = *++pSrc;
	uint16 hi = *++pSrc;
	return lo | (hi << 8);
}

void ASound::pollActiveChannel() {
	AdlibChannel *ch = _activeChannelPtr;

	if (ch->_activeCount == 0) {
		++_activeChannelNumber;
		return;
	}

	/* byte_16A0A: volume-dirty flag.  Cleared here, set by various opcodes
	 * and by the fade/vibrato sections; causes writeVolume at the end. */
	bool volDirty = false;

	/* ---- Key-on delay countdown ---- */
	if (ch->_keyOnDelay != 0) {
		ch->_keyOnDelay--;
		ch = _activeChannelPtr;
		if (ch->_keyOnDelay == 0)
			updateOctave();
	}

	/* ---- Duration countdown ---- */
	ch = _activeChannelPtr;
	ch->_activeCount--;
	ch = _activeChannelPtr;
	if (ch->_activeCount != 0)
		goto post_keyon;

	/* ================================================================
	 * Command-dispatch loop.
	 * Re-entered (via goto dispatch) after each command that does not
	 * consume a duration tick.  Falls through to the note-event path
	 * when a note byte is encountered.
	 * ================================================================ */
dispatch:
	{
		ch = _activeChannelPtr;
		pSrc = ch->_pSrc;

		uint8 b = *pSrc;

		if (!(b & 0x80))
			goto note_event;

		/* Decode group (si) and sub-opcode (di). */
		uint8 si = b & 0x70;
		uint8 di = b & 0x0F;

		if (si == 0x00) {
			/* ============================================================
			 * opcodes1  (group 0, 0x8_)
			 * Called with: pSrc pointing at the command byte.
			 * On entry, opcodes1 does: inc pSrc (skip past command byte).
			 * On exit, sets _pSrc = pSrc + 1 (def_10CDC).
			 * ============================================================ */
			pSrc++;   /* skip command byte */

			switch (di) {

			case 0x0: /* set sampleIndex, call loadSample */
				ch->_sampleIndex = *pSrc;
				loadSample();
				volDirty = true;
				/* def_10CDC: _pSrc = pSrc + 1 */
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x1: /* set velocity */
			{
				uint8 vel = *pSrc;
				if (ch->_pendingStop != 0) {
					/* If current velocity (signed) > new vel (signed), clamp down. */
					if ((int16)(int8)ch->_velocity > (int16)(int8)vel)
						ch->_velocity = vel;
				} else {
					ch->_velocity = vel;
				}
				volDirty = true;
				ch->_pSrc = pSrc + 1;
				goto dispatch;
			}

			case 0x2: /* set volume */
			{
				uint8 vol = *pSrc;
				if (ch->_pendingStop != 0) {
					/* Don't raise volume above current when fading out. */
					uint16 curVol = (uint16)ch->_volume;
					if (curVol <= (uint16)vol)
						goto op2_set_vol;
					/* else fall through and skip the set */
					volDirty = true;
					ch->_pSrc = pSrc + 1;
					goto dispatch;
				}
op2_set_vol:
				ch->_volume = vol;
				volDirty = true;
				ch->_pSrc = pSrc + 1;
				goto dispatch;
			}

			case 0x3: /* set patchAttenuation */
				ch->_patchAttenuation = *pSrc;
				volDirty = true;   /* opcodes1 case 3 jumps to loc_10B56 -> byte_16A0A=1 */
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x4: /* set fade: fadePeriodReload, volumeFadeStep */
				if (ch->_pendingStop != 0) {
					/* Skip this opcode entirely when pending-stop is set. */
					ch->_pSrc = pSrc + 2;
					goto dispatch;
				}
				ch->_fadePeriodReload = *pSrc++;
				ch->_volumeFadeStep = *pSrc;
				ch->_fadePeriodCounter = 1;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x5: /* set vibrato: vibPeriodReload, vibPeriodCounter=reload, vibratoDepth, vibratoMode */
				ch->_vibPeriodReload = *pSrc++;
				ch->_vibPeriodCounter = ch->_vibPeriodReload;
				ch->_vibratoDepth = *pSrc++;
				ch->_vibratoMode = *pSrc;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x6: /* set transpose */
				ch->_transpose = *pSrc;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x7: /* set octaveTranspose */
				ch->_octaveTranspose = *pSrc;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x8: /* set field_11 (_writeVolumePending) */
				ch->_writeVolumePending = *pSrc;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0x9: /* set arpeggio: field_12->field_C, pitchBend, field_13 */
				ch->_arpPeriodReload = *pSrc++;
				ch->_arpPeriodCounter = ch->_arpPeriodReload;   /* field_C = field_12 */
				ch->_pitchBend = *pSrc++;
				ch->_arpCounterReload = *pSrc;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0xA: /* skip forward by N+3 bytes (from the command byte) */
			{
				uint8 skip = *pSrc;
				/* pSrc currently points at the byte after the command byte.
				 * Advance _pSrc by (skip + 3) from the *command* byte position,
				 * which is pSrc-1.  So _pSrc += skip + 3 from original _pSrc,
				 * which was command byte.  But def_10CDC does _pSrc = pSrc+1
				 * giving +2 past command.  Then skip+3 total from command: */
				 /* asm: si = skip; ax = si + 3; add pSrc, ax; then def_10CDC
				  * does pSrc+1.  So net advance from the command byte =
				  * skip + 3 + 1 = skip + 4.  But pSrc already points one past
				  * command byte, so _pSrc = pSrc + skip + 3. */
				ch->_pSrc = pSrc + (uint16)skip + 3;
				goto dispatch;
			}

			case 0xB: /* set durationOverride, clear noteOffset */
				ch->_durationOverride = *pSrc;
				ch->_noteOffset = 0;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			case 0xC: /* set noteOffset, clear durationOverride */
				ch->_noteOffset = *pSrc;
				ch->_durationOverride = 0;
				ch->_pSrc = pSrc + 1;
				goto dispatch;

			default:
				/* Unknown sub-opcode: advance past the single operand byte. */
				ch->_pSrc = pSrc + 1;
				goto dispatch;
			}
		}

		if (si == 0x10) {
			/* ============================================================
			 * opcodes2  (group 1, 0x9_)
			 * Loop control, restart, branch / call.
			 * On exit: either sets _pSrc directly or falls to def_10EAC
			 * (which just returns without touching _pSrc - the handlers
			 * themselves are responsible for updating _pSrc).
			 * ============================================================ */
			switch (di) {

			case 0x0: /* inner loop */
			{
				ch = _activeChannelPtr;
				if (ch->_innerLoopCount == 0) {
					pSrc++;   /* advance to count byte */
					uint8 cnt = *pSrc;
					if (cnt == 0) {
						ch->_pSrc += 2;
						ch = _activeChannelPtr;
						ch->_innerLoopPtr = ch->_pSrc;
						ch->_innerLoopCount = 0;
						goto dispatch;
					}
					ch->_innerLoopCount = (uint16)cnt;
					/* Jump to innerLoopPtr (loc_10D5A). */
					ch->_pSrc = ch->_innerLoopPtr;
					goto dispatch;
				}
				/* Count is non-zero: decrement. */
				ch->_innerLoopCount--;
				ch = _activeChannelPtr;
				if (ch->_innerLoopCount == 0) {
					/* Loop done: advance past the two-byte opcode. */
					ch->_pSrc += 2;
					ch = _activeChannelPtr;
					ch->_innerLoopPtr = ch->_pSrc;
					goto dispatch;
				}
				/* Still looping: jump back. */
				ch->_pSrc = ch->_innerLoopPtr;
				goto dispatch;
			}

			case 0x1: /* outer loop */
			{
				ch = _activeChannelPtr;
				if (ch->_outerLoopCount == 0) {
					pSrc++;
					uint8 cnt = *pSrc;
					if (cnt == 0) {
						ch->_pSrc += 2;
						ch = _activeChannelPtr;
						ch->_outerLoopPtr = ch->_pSrc;
						ch->_innerLoopPtr = ch->_pSrc;
						ch->_innerLoopCount = 0;
						ch->_outerLoopCount = 0;
						goto dispatch;
					}
					ch->_outerLoopCount = (uint16)cnt;
					/* Jump to outerLoopPtr. */
					ch->_pSrc = ch->_outerLoopPtr;
					ch = _activeChannelPtr;
					ch->_innerLoopPtr = ch->_outerLoopPtr;
					goto dispatch;
				}
				/* Count non-zero: decrement. */
				ch->_outerLoopCount--;
				ch = _activeChannelPtr;
				if (ch->_outerLoopCount == 0) {
					ch->_pSrc += 2;
					ch = _activeChannelPtr;
					ch->_outerLoopPtr = ch->_pSrc;
					ch->_innerLoopPtr = ch->_pSrc;
					goto dispatch;
				}
				ch->_pSrc = ch->_outerLoopPtr;
				ch = _activeChannelPtr;
				ch->_innerLoopPtr = ch->_outerLoopPtr;
				goto dispatch;
			}

			case 0x2: /* restart: reset all loop pointers to _soundData */
			{
				ch = _activeChannelPtr;
				uint16 sd = (uint16)(uintptr_t)ch->_soundData;
				ch->_loopStartPtr = ch->_soundData;
				ch->_pSrc = ch->_soundData;
				ch->_innerLoopPtr = ch->_soundData;
				ch->_outerLoopPtr = ch->_soundData;
				(void)sd;
				goto dispatch;
			}

			case 0x3: /* set sound-data pointer (abs word offset), reset all loops */
			{
				byte *ptr = &_soundData[readWord_impl()];
				ch = _activeChannelPtr;
				ch->_loopStartPtr = ptr;
				ch->_pSrc = ptr;
				ch->_innerLoopPtr = ptr;
				ch->_outerLoopPtr = ptr;
				ch->_soundData = ptr;
				goto dispatch;
			}

			case 0x4: /* branch unconditional */
			{
				byte *dest = &_soundData[readWord_impl()];
				ch = _activeChannelPtr;
				ch->_pSrc = dest;
				goto dispatch;
			}

			case 0x5: /* branch with return-address save */
			{
				byte *dest = &_soundData[readWord_impl()];
				ch = _activeChannelPtr;
				ch->_branchTargetPtr = ch->_pSrc + 3;
				ch->_pSrc = dest;
				goto dispatch;
			}

			case 0x6: /* return from branch */
			{
				ch = _activeChannelPtr;
				if (ch->_branchTargetPtr != nullptr) {
					ch->_pSrc = ch->_branchTargetPtr;
					ch->_branchTargetPtr = nullptr;
				} else {
					ch->_pSrc++;
				}
				goto dispatch;
			}

			default:
				/* Unknown sub-opcode: no _pSrc advance (def_10EAC). */
				goto dispatch;
			}
		}

		if (si == 0x20) {
			/* ============================================================
			 * opcodes3  (group 2, 0xA_)
			 * Tempo/callback globals, random table operations,
			 * indirect-table operations, call-by-address, nullsub.
			 *
			 * opcodes3 receives di (sub-opcode) but NOT the already-
			 * incremented pSrc; each case increments pSrc itself as needed.
			 * ============================================================ */
			switch (di) {

			case 0x0: /* random pick from packed table -> write to stream */
			{
				/* Format: [cmd] [tblSize] [entry0..N-1] [targetSlot]
				 * pSrc points at the command byte on entry. */
				pSrc++;
				uint8  tblSize = *pSrc;
				pSrc++;   /* now at entry[0] */
				(void)getRandomNumber();
				uint16 rnd = _randomSeed & 0x7FFF;
				int16  idx = (int16)((int16)rnd % (int16)(uint16)tblSize);
				uint8  chosen = *(pSrc + idx);
				uint8  target = *(pSrc + tblSize);
				*(pSrc + (uint16)target + 1) = chosen;
				ch = _activeChannelPtr;
				ch->_pSrc += (uint16)tblSize + 3;
				goto dispatch;
			}

			case 0x1: /* random range [lo..hi] -> write to stream */
			{
				/* Format: [cmd] [lo] [hi] [targetSlot] ... */
				pSrc++;
				uint8 lo = *pSrc++;
				uint8 hi = *pSrc++;
				uint16 range = (uint16)(hi - lo) + 1;
				/* pSrc now at targetSlot byte. */
				(void)getRandomNumber();
				uint16 rnd = _randomSeed & 0x7FFF;
				uint8  res = (uint8)((int16)rnd % (int16)range) + lo;
				uint8  slot = *pSrc;
				*(pSrc + (uint16)slot + 1) = res;
				ch = _activeChannelPtr;
				ch->_pSrc += 4;   /* command+lo+hi+slot = 4 bytes consumed */
				goto dispatch;
			}

			case 0x2: /* indirect table read from scriptVar -> write to stream */
			{
				/* Format: [cmd] [varIdx] [tableSlot] [table...] */
				pSrc++;
				uint8 varIdx = *pSrc++;
				uint8 tableSlot = *pSrc++;
				/* pSrc now points at the inline table. */
				uint8 idxVal = _scriptVars[varIdx];
				uint8 chosen = *(pSrc + idxVal);
				uint8 target = *(pSrc + tableSlot);
				*(pSrc + (uint16)target + 1) = chosen;
				ch = _activeChannelPtr;
				ch->_pSrc += (uint16)tableSlot + 4;
				goto dispatch;
			}

			case 0x3: /* call function by address (near call in original) */
			{
				uint16 fnOffset = readWord_impl();
				callFunction(fnOffset);
				ch = _activeChannelPtr;
				ch->_pSrc += 3;
				goto dispatch;
			}

			case 0x4: /* nullsub_1 with byte arg */
			{
				/* "call near ptr aAsoundDriverAn+33h" - the target is a no-op. */
				pSrc++;
				/* ()*pSrc; */
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			case 0x5: /* advance _pSrc by 4 (from command byte) */
			{
				/* loc_10F55 is shared with case 1's epilogue: _pSrc += 4. */
				ch = _activeChannelPtr;
				ch->_pSrc += 4;
				goto dispatch;
			}

			case 0x6: /* set word_124F2 (_tempoFineStep) */
			{
				pSrc++;
				uint8 val = *pSrc;
				_tempoFineStep = (uint16)val;
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			case 0x7: /* set word_124F0 (_tempoCoarseStep) */
			{
				pSrc++;
				uint8 val = *pSrc;
				_tempoCoarseStep = (uint16)val;
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			case 0x8: /* set word_124EE (_tempoPeriod), enable tick callback */
			{
				uint16 period = readWord_impl();
				_tempoPeriod = period;
				ch = _activeChannelPtr;
				ch->_pSrc += 3;
				_tempoEnabled = 1;
				_tempoTickCounter = 1;
				goto dispatch;
			}

			case 0x9: /* set word_124F4 (_tempoShift) */
			{
				pSrc++;
				uint8 val = *pSrc;
				_tempoShift = (uint16)val;
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			default:
				goto dispatch;
			}
		}

		if (si == 0x30) {
			/* ============================================================
			 * opcodes4  (group 3, 0xB_)
			 * Script-variable load (imm or var), store to stream, inc, dec.
			 * Each case increments pSrc itself, then sets _pSrc.
			 *
			 * Switch on di (0-4); di >= 5 falls through to return.
			 * ============================================================ */
			switch (di) {

			case 0x0: /* var[dst] = imm */
			{
				pSrc++;
				uint8 dst = *pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[dst] = imm;
				ch = _activeChannelPtr;
				ch->_pSrc += 3;
				goto dispatch;
			}

			case 0x1: /* var[dst] = var[src] */
			{
				pSrc++;
				uint8 dst = *pSrc++;
				uint8 src = *pSrc;
				_scriptVars[dst] = _scriptVars[src];
				ch = _activeChannelPtr;
				ch->_pSrc += 3;
				goto dispatch;
			}

			case 0x2: /* stream[tableSlot+1] = var[src] */
			{
				/* asm: [bx+di+1] = cl, where bx=pSrc, di=tableSlot. */
				pSrc++;
				uint8 src = *pSrc++;
				uint8 tableSlot = *pSrc;
				*(pSrc + (uint16)tableSlot + 1) = _scriptVars[src];
				ch = _activeChannelPtr;
				ch->_pSrc += 3;
				goto dispatch;
			}

			case 0x3: /* var[dst]++ */
			{
				pSrc++;
				uint8 dst = *pSrc;
				_scriptVars[dst]++;
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			case 0x4: /* var[dst]-- */
			{
				pSrc++;
				uint8 dst = *pSrc;
				_scriptVars[dst]--;
				ch = _activeChannelPtr;
				ch->_pSrc += 2;
				goto dispatch;
			}

			default:
				goto dispatch;
			}
		}

		if (si == 0x40) {
			/* ============================================================
			 * opcodes5  (group 4, 0xC_)
			 * Script-variable ALU: add/sub/mul/div/rem/and/or/xor.
			 *
			 * Entry: inc pSrc, read dst (si), inc pSrc, read src (di).
			 * Then _pSrc += 3 (command + dst + src).
			 * 16 sub-opcodes (0-15).
			 *
			 * For sub-opcodes 0,2,4: src is an immediate (di from the byte).
			 * For sub-opcodes 1,3,5: src is _scriptVars[di].
			 * For 6-9: signed or unsigned division/remainder.
			 * For 10-15: bitwise AND/OR/XOR, imm or var.
			 * ============================================================ */
			pSrc++;
			uint8 dst = *pSrc++;
			uint8 src = *pSrc;   /* immediate value or var index */
			ch = _activeChannelPtr;
			ch->_pSrc += 3;

			switch (di) {
			case 0x0: _scriptVars[dst] += src;                                       break;
			case 0x1: _scriptVars[dst] += _scriptVars[src];                          break;
			case 0x2: _scriptVars[dst] -= src;                                       break;
			case 0x3: _scriptVars[dst] -= _scriptVars[src];                          break;
			case 0x4: /* mul ax=di, mul [si] -> al stored */
				_scriptVars[dst] = (uint8)((uint16)src * (uint16)_scriptVars[dst]);  break;
			case 0x5:
				_scriptVars[dst] = (uint8)((uint16)_scriptVars[src] * (uint16)_scriptVars[dst]); break;
			case 0x6: /* signed idiv by imm -> quotient in al */
				_scriptVars[dst] = (uint8)((int8)_scriptVars[dst] / (int8)src);      break;
			case 0x7: /* unsigned div by var -> quotient in al */
				_scriptVars[dst] = _scriptVars[dst] / _scriptVars[src];              break;
			case 0x8: /* signed idiv by imm -> remainder in dl (stored to dst) */
				_scriptVars[dst] = (uint8)((int8)_scriptVars[dst] % (int8)src);      break;
			case 0x9: /* unsigned div by var -> remainder in ah (stored to dst) */
				_scriptVars[dst] = _scriptVars[dst] % _scriptVars[src];              break;
			case 0xA: _scriptVars[dst] &= src;                                       break;
			case 0xB: _scriptVars[dst] &= _scriptVars[src];                          break;
			case 0xC: _scriptVars[dst] |= src;                                       break;
			case 0xD: _scriptVars[dst] |= _scriptVars[src];                          break;
			case 0xE: _scriptVars[dst] ^= src;                                       break;
			case 0xF: _scriptVars[dst] ^= _scriptVars[src];                          break;
			default: break;
			}
			goto dispatch;
		}

		if (si == 0x50) {
			/* ============================================================
			 * opcodes6  (group 5, 0xD_)
			 * Conditional branch: compare var[si] vs immediate (di as imm byte).
			 *
			 * Format: [cmd] [varIdx] [imm] [addrLo] [addrHi]
			 * Entry: inc pSrc, read varIdx (si reg), inc pSrc, read imm (di reg).
			 * Comparison: var[varIdx] vs imm.
			 *
			 * If condition met: call asound_readWord (reads addrLo/addrHi
			 *   from current pSrc position), set _pSrc = &_soundData[addr].
			 * If not met: _pSrc += 5 (command+varIdx+imm+addr word = 5 bytes).
			 *
			 * var_2 is used as a flag: 0 = branch taken, 1 = not taken.
			 * (Confusingly, var_2 = 1 means "taken" in opcodes7 but
			 *  0 means "taken" in opcodes6 - see def_11292 / def_11360 difference.)
			 *
			 * opcodes6: var_2=1 -> taken (call readWord, set pSrc),
			 *            var_2=0 -> not taken (_pSrc += 5).
			 * ============================================================ */
			pSrc++;
			uint8 varIdx = *pSrc++;
			uint8 imm = *pSrc;
			uint8 vval = _scriptVars[varIdx];
			bool  taken = false;

			switch (di) {
			case 0x0: taken = ((uint16)vval == (uint16)imm);                    break; /* == */
			case 0x1: taken = ((uint16)vval != (uint16)imm);                    break; /* != */
			case 0x2: taken = ((int16)(uint16)vval >= (int16)(uint16)imm);      break; /* jge -> taken if >= */
			case 0x3: taken = ((int16)(uint16)vval <= (int16)(uint16)imm);      break; /* jle -> taken if <= */
			case 0x4: taken = (_scriptVars[varIdx] != _scriptVars[imm]);        break; /* jnz after cmp [di],[si] */
			case 0x5: taken = (_scriptVars[varIdx] == _scriptVars[imm]);        break; /* jz  after cmp [di],[si] */
			case 0x6: taken = (_scriptVars[imm] <= _scriptVars[varIdx]);        break; /* jbe -> [di]<=[si] taken */
			case 0x7: taken = (_scriptVars[imm] >= _scriptVars[varIdx]);        break; /* jnb -> [di]>=[si] taken */
			default:  break;
			}

			ch = _activeChannelPtr;
			if (taken) {
				ch->_pSrc = &_soundData[readWord_impl()];
			} else {
				ch->_pSrc += 5;
			}
			goto dispatch;
		}

		if (si == 0x60) {
			/* ============================================================
			 * opcodes7  (group 6, 0xE_)
			 * Conditional branch: compare var[si] vs var[di] (both scriptVars).
			 *
			 * Format: [cmd] [varIdxA] [varIdxB] [addrLo] [addrHi]
			 * Entry: inc pSrc, read varIdxA (si), inc pSrc, read varIdxB (di).
			 * var_2 = 0 initially.
			 *
			 * If condition met: var_2 = 1 -> branch taken:
			 *   _pSrc = pSrc + 5; _branchTargetPtr = pSrc + 5;
			 *   call readWord -> _pSrc = &_soundData[addr].
			 * If not met (var_2 = 0): _pSrc += 5 (skip condition + word).
			 *
			 * Note: unlike opcodes6, opcodes7 DOES save a branch-target
			 * (_branchTargetPtr = _pSrc + 5) on the taken path.
			 * ============================================================ */
			pSrc++;
			uint8 idxA = *pSrc++;
			uint8 idxB = *pSrc;
			uint8 va = _scriptVars[idxA];
			uint8 vb = (uint8)idxB;   /* di byte used as an immediate in some cases... */

			/* Actually in opcodes7 the layout is var vs var:
			 * si = varIdxA, di = varIdxB (both are variable indices). */
			vb = _scriptVars[idxB];

			bool taken = false;
			switch (di & 0x07) {
			case 0x0: taken = ((uint16)va != (uint16)vb);  break; /* jnz after cmp ax,di (case 0: jz -> NOT taken if ==; var_2 stays 0; but then def path: var_2==0 -> skip. Wait - let me re-read.) */
				/* Re-reading loc_112F0: cmp ax,di; jz loc_11352 (-> var_2=1=taken).
				 * So case 0: taken = (va == vb). */
			default: break;
			}

			/* Actually re-reading carefully:
			 * case 0 (loc_112F0): cmp ax,di; jz -> loc_11352 (var_2=1, taken)
			 *                      else -> loc_112FA (ax=0, var_2=0, not taken)
			 * case 1 (loc_11302): cmp ax,di; jz -> loc_1130A -> (jz loc_112FA, not taken)
			 *                     else -> loc_11284 (var_2=1, taken)
			 * -> case 1: taken = (va != vb)
			 * case 2 (loc_1130E): jge -> loc_112FA (not taken); else loc_11284 (taken)
			 * -> taken = (va < vb) (signed)
			 * case 3 (loc_1131A): jle -> not taken; else taken
			 * -> taken = (va > vb) (signed)
			 * case 4 (loc_11326): cmp [di],al; jnz -> loc_112FA (not taken); else loc_11284 (taken)
			 * -> taken = (_scriptVars[idxB] == va) (already same as case 0 with vars swapped)
			 * case 5 (loc_11332): cmp [di],al; jz->loc_1130A (not taken if ==, taken if !=)
			 * -> taken = (_scriptVars[idxB] != va)
			 * case 6 (loc_1133C): jbe -> not taken; else taken -> taken = (_scriptVars[idxB] > va)
			 * case 7 (loc_11348): jnb -> not taken; else taken -> taken = (_scriptVars[idxB] < va)
			 */
			switch (di) {
			case 0x0: taken = (va == vb);               break;
			case 0x1: taken = (va != vb);               break;
			case 0x2: taken = ((int8)va < (int8)vb);   break;
			case 0x3: taken = ((int8)va > (int8)vb);   break;
			case 0x4: taken = (vb == va);               break;
			case 0x5: taken = (vb != va);               break;
			case 0x6: taken = (vb > va);                break;
			case 0x7: taken = (vb < va);                break;
			default:  break;
			}

			ch = _activeChannelPtr;
			if (taken) {
				/* Save return target (pSrc+5 from the command byte). */
				ch->_branchTargetPtr = ch->_pSrc + 5;
				ch->_pSrc = &_soundData[readWord_impl()];
			} else {
				ch->_pSrc += 5;
			}
			goto dispatch;
		}

		/* Unknown group: skip. */
		goto dispatch;
	}

note_event:
	{
		/* ---- Note event: [note][duration] ---- */
		ch = _activeChannelPtr;
		pSrc = ch->_pSrc;
		ch->_note = *pSrc;
		pSrc++;
		ch->_activeCount = *pSrc;
		pSrc++;
		ch->_pSrc += 2;

		ch = _activeChannelPtr;
		if (ch->_note == 0 || ch->_activeCount == 0) {
			updateOctave();
			goto post_keyon;
		}

		/* ---- Key-on ---- */
		ch = _activeChannelPtr;
		if (ch->_durationOverride != 0)
			ch->_keyOnDelay = ch->_durationOverride;
		else
			ch->_keyOnDelay = ch->_activeCount - ch->_noteOffset;
		noteOn();
	}

post_keyon:
	{
		ch = _activeChannelPtr;

		/* ---- Arpeggio counter (field_13 / _arpCounterReload) ---- */
		if (ch->_arpCounterReload != 0) {
			/* Decrement the arpeggio period counter (field_C / _arpPeriodCounter). */
			ch->_arpPeriodCounter--;
			ch = _activeChannelPtr;
			if (ch->_arpPeriodCounter == 0) {
				/* Reload from field_12 (_arpPeriodReload). */
				ch->_arpPeriodCounter = ch->_arpPeriodReload;
				/* Call sub_117E8 (writeArpeggio - writes the arpeggio frequency). */
				writeArpeggio();
			}
			ch = _activeChannelPtr;
			/* Decrement the repeat counter unless it is the infinite sentinel 0xFF. */
			if (ch->_arpCounterReload != 0xFF)
				ch->_arpCounterReload--;
		}

		/* ---- Write-volume pending (field_11 / _writeVolumePending) ---- */
		ch = _activeChannelPtr;
		if (ch->_writeVolumePending != 0) {
			/* sub_11856 was already called by writeArpeggio above (or this is
			 * a standalone field_11 set via opcode 8).  Clear the flag. */
			writeArpeggio();
			ch = _activeChannelPtr;
			ch->_writeVolumePending = 0;
		}

		/* ---- Fade / volume step ---- */
		ch = _activeChannelPtr;
		if (ch->_fadePeriodCounter != 0) {
			ch->_fadePeriodCounter--;
			ch = _activeChannelPtr;
			if (ch->_fadePeriodCounter == 0) {
				ch->_fadePeriodCounter = ch->_fadePeriodReload;
				ch = _activeChannelPtr;
				if (ch->_volumeFadeStep != 0) {
					if (ch->_pendingStop != 0) {
						/* Pending-stop fade: step velocity and volume down. */
						if (ch->_velocity > 0)
							ch->_velocity += ch->_volumeFadeStep;
						ch = _activeChannelPtr;
						if (ch->_volume != 0)
							ch->_volume += ch->_volumeFadeStep;
					} else {
						/* Normal fade: step velocity, clamp to [0, 0x7F]. */
						if ((int8)ch->_volumeFadeStep > 0) {
							ch->_velocity += ch->_volumeFadeStep;
							ch = _activeChannelPtr;
							if ((uint16)ch->_velocity > 0x7F)
								ch->_velocity = 0x7F;
						} else {
							ch->_velocity += ch->_volumeFadeStep;
							ch = _activeChannelPtr;
							if ((int8)ch->_velocity < 0)
								ch->_velocity = 0;
						}
					}
					volDirty = true;   /* byte_16A0A = 1 */
				}
			}
		}

		/* ---- Vibrato ---- */
		ch = _activeChannelPtr;
		if (ch->_vibPeriodCounter != 0) {
			ch->_vibPeriodCounter--;
			ch = _activeChannelPtr;
			if (ch->_vibPeriodCounter == 0) {
				ch->_vibPeriodCounter = ch->_vibPeriodReload;
				ch = _activeChannelPtr;

				if ((int8)ch->_vibratoDepth > 0) {
					/* Positive vibrato: ceiling at 0x7F. */
					uint8 sum = ch->_vibratoDepth + ch->_patchAttenuation;
					if (sum > 0x7F) {
						if (ch->_vibratoMode == 0xFF) {
							/* One-shot: clamp and stop. */
							ch->_vibratoDepth = 0;
							ch->_patchAttenuation = 0x7F;
						} else if (sum == 0x80 && ch->_vibratoDepth != 1) {
							/* Soft bounce at 0x80: patchAtten = 0x7F - vibratoDepth. */
							ch->_patchAttenuation = 0x7F - ch->_vibratoDepth;
						} else {
							ch->_vibratoDepth = (uint8)(-(int8)ch->_vibratoDepth);
						}
					}
					ch = _activeChannelPtr;
					ch->_patchAttenuation += ch->_vibratoDepth;
					volDirty = true;

				} else if ((int8)ch->_vibratoDepth < 0) {
					/* Negative vibrato: floor at 0. */
					uint8 sum = ch->_vibratoDepth + ch->_patchAttenuation;
					/* Note: sum is uint8 wrapping, but the asm uses "or al,al; jge"
					 * treating the result as signed. */
					if ((int8)sum < 0) {
						if (ch->_vibratoMode == 0xFF) {
							ch->_vibratoDepth = 0;
							ch->_patchAttenuation = 0;
						} else if (sum == 0xFF && ch->_vibratoDepth != 0xFF) {
							/* Soft bounce: patchAtten = neg(vibratoDepth). */
							ch->_patchAttenuation = (uint8)(-(int8)ch->_vibratoDepth);
						} else {
							ch->_vibratoDepth = (uint8)(-(int8)ch->_vibratoDepth);
						}
					}
					ch = _activeChannelPtr;
					ch->_patchAttenuation += ch->_vibratoDepth;
					volDirty = true;
				}
				/* vibratoDepth == 0: nothing to do. */
			}
		}

		if (volDirty)
			writeVolume();
	}

	++_activeChannelNumber;
}

void ASound::playSound(int offset) {
	findFreeChannelFull(loadData(offset));
}

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
