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
#include "mads/madsv2/core/asound.h"

namespace MADS {
namespace MADSV2 {

bool AdlibChannel::_isDisabled;

/* =========================================================================
 * Lookup tables
 * ========================================================================= */

 /*
  * PATCH_ATTEN_TO_TL - Patch-attenuation to OPL total-level conversion table.
  * Index is the patchAttenuation value (0-127); output is the 6-bit TL field.
  */
static const uint8 PATCH_ATTEN_TO_TL[128] = {
	63, 54, 49, 45, 42, 40, 38, 36, 34, 33, 32, 31, 30, 29, 28, 27,
	26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18,
	18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13,
	12, 12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9,
	9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 6,
	6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4,
	4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
};

/*
 * VOL_VEL_TO_ATTEN_STEP - Volume/velocity to OPL attenuation step table.
 * 128 entries; each group of 4 maps to the same step value (1..32).
 */
static const uint8 VOL_VEL_TO_ATTEN_STEP[128] = {
	0x01,0x01,0x01,0x01, 0x02,0x02,0x02,0x02, 0x03,0x03,0x03,0x03,
	0x04,0x04,0x04,0x04, 0x05,0x05,0x05,0x05, 0x06,0x06,0x06,0x06,
	0x07,0x07,0x07,0x07, 0x08,0x08,0x08,0x08, 0x09,0x09,0x09,0x09,
	0x0A,0x0A,0x0A,0x0A, 0x0B,0x0B,0x0B,0x0B, 0x0C,0x0C,0x0C,0x0C,
	0x0D,0x0D,0x0D,0x0D, 0x0E,0x0E,0x0E,0x0E, 0x0F,0x0F,0x0F,0x0F,
	0x10,0x10,0x10,0x10, 0x11,0x11,0x11,0x11, 0x12,0x12,0x12,0x12,
	0x13,0x13,0x13,0x13, 0x14,0x14,0x14,0x14, 0x15,0x15,0x15,0x15,
	0x16,0x16,0x16,0x16, 0x17,0x17,0x17,0x17, 0x18,0x18,0x18,0x18,
	0x19,0x19,0x19,0x19, 0x1A,0x1A,0x1A,0x1A, 0x1B,0x1B,0x1B,0x1B,
	0x1C,0x1C,0x1C,0x1C, 0x1D,0x1D,0x1D,0x1D, 0x1E,0x1E,0x1E,0x1E,
	0x1F,0x1F,0x1F,0x1F, 0x20,0x20,0x20,0x20
};

/* Carrier and operator slot offset per voice */
static const uint8 VOICE_SLOTS[ADLIB_CHANNEL_COUNT][2] = {
	{ 0, 3 }, { 1, 4 }, { 2, 5 }, { 6, 9 }, { 7, 10 },
	{ 8, 11 }, { 12, 15 }, { 13, 16 }, { 14, 17 }
};

/*
 * SLOT_TO_REG_OFFSET - Operator slot index to OPL register offset.
 * Maps an operator slot (0-17) to its base register offset within a group.
 */
static const uint8 SLOT_TO_REG_OFFSET[18] = {
	0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21
};

/*
 * SEMITONE_FREQ_TABLE - F-number table: one entry per semitone within an octave.
 * 12 entries (C through B).
 */
static const uint16 SEMITONE_FREQ_TABLE[12] = {
	0x0200,0x021E,0x023F,0x0261,0x0285,0x02AB,
	0x02D4,0x02FF,0x032D,0x035D,0x0390,0x03C7
};

/* Null / silence sound data pointer placeholder */
static uint8 ADLIB_NULLDATA[] = {
	0x00, 0x01, 0xF3, 0x00, 0x00, 0xEF, 0x00, 0x00,
	0xF5, 0x00, 0x00, 0x00, 0xF7, 0x00, 0xF8, 0x1D,
	0xFF, 0x00, 0xF4, 0x6E, 0x2A, 0x1C, 0xF4, 0x5F,
	0x2A, 0x1C, 0x2A, 0x1C, 0x2A, 0x1C, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00
};

// =========================================================================

constexpr uint8 OPL_VERSION_FLAG = 18;	// OPL version flag
constexpr bool RHYTHM_HI_HAT = true;	// Rhythm mode: hi-hat
constexpr bool RHYTHM_CYMBAL = true;	// Rhythm mode: cymbal
constexpr bool RHYTHM_ENABLE = true;	// Rhythm mode: enable

// =========================================================================

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

// =========================================================================

void AdlibChannel::reset() {
	_activeCount = 0;
	_pitchBend = 0;
	_volumeFadeStep = 0;
	_vibratoDepth = 0;
	_freqSweepCounter = 0;
	_noiseFreqMask = 0;
	_freqAccum = 0;
	_freqStep = 0;
	_loopStartPtr = 0;
	_pSrc = 0;
}

void AdlibChannel::load(byte *soundData) {
	_isDisabled = true;

	memset(this, 0, sizeof(AdlibChannel));

	_loopStartPtr = soundData;
	_pSrc = soundData;
	_innerLoopPtr = soundData;
	_outerLoopPtr = soundData;
	_soundData = soundData;
	_patchAttenuation = 0x40;
	_activeCount = 1;

	_isDisabled = false;
}

void AdlibChannel::setPtr2(byte *ptr) {
	_loopStartPtr = ptr;
	_pSrc = ptr;
	_volumeFadeStep = 0xFF;   /* -1 signed: fade down */
	_fadePeriodReload = 1;
	_fadePeriodCounter = 1;
}

void AdlibChannel::enable() {
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

	/* If both velocity and volume are zero the sound has fully faded */
	if (_velocity == 0 && _volume == 0) {
		_loopStartPtr = ADLIB_NULLDATA;
		_pSrc = ADLIB_NULLDATA;
		_pendingStop = 0;
		return;
	}

	/* Arm a fast fade-down */
	_volumeFadeStep = 0xFF;   /* -1: decrement each period */
	_fadePeriodReload = 2;
	if (_fadePeriodCounter == 0)
		_fadePeriodCounter = 1;
}

// =========================================================================

ASound::ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
		int dataOffset, int dataSize) :
		SoundDriver(mixer, opl, filename, dataOffset, dataSize) {
	AdlibChannel::_isDisabled = false;

	write(4, 0x60);		// Mask off both adlib timers
	write(4, 0x80);		// IRQ reset timer flags
	write(2, 0xff);		// Timer 1 rate 80us
	write(4, 0x21);		// Activate timer 1
	write(4, 0x60);
	write(4, 0x80);

	Common::fill(_adlibPorts, _adlibPorts + 256, 0);
	command0();

	_opl->start(new Common::Functor0Mem<void, ASound>(this, &ASound::onTimer), CALLBACKS_PER_SECOND);
}

int ASound::stop() {
	command0();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

int ASound::poll() {
	// Update any playing sounds
	update();

	// Return result
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

void ASound::noise() {
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; i++)
		noise_inner(i);
}

void ASound::playSound(int offset, int size) {
	findFreeChannelFull(loadData(offset, size));
}

/* =========================================================================
 * Shared Commands
 * ========================================================================= */

int ASound::command0() {
	int i;

	/* 1. Reset all 9 channels */
	for (i = 0; i < ADLIB_CHANNEL_COUNT; i++)
		_channels[i]->reset();

	/* 2. Registers 0x40-0x4F: set total-level to maximum (0x3F) combined
	 *    with whatever flags were already there (asm used OR with 0x3F then
	 *    writes 0x3F-remaining).  The reset state is simply full attenuation. */
	for (int reg = 0x4F; reg >= 0x40; reg--)
		write(reg, 0x3F);

	/* 3. Registers 0x60-0xFF: clear (silence envelope / frequency / etc.) */
	for (int reg = 0xFF; reg >= 0x60; reg--)
		write(reg, 0x00);

	/* 4. Registers 0x01-0x3F: clear */
	for (int reg = 0x3F; reg >= 0x01; reg--)
		write(reg, 0x00);

	/* 5. Waveform Select Enable */
	write(0x01, 0x20);

	/* 6. Reset callback counters */
	resetCallback();

	return 0;
}

int ASound::command1() {
	command3();
	command5();

	return 0;
}

int ASound::command2() {
	_channel0.setPtr2(ADLIB_NULLDATA);
	_channel1.setPtr2(ADLIB_NULLDATA);
	_channel2.setPtr2(ADLIB_NULLDATA);
	_channel3.setPtr2(ADLIB_NULLDATA);
	_channel4.setPtr2(ADLIB_NULLDATA);
	_channel5.setPtr2(ADLIB_NULLDATA);

	return 0;
}

int ASound::command3() {
	_channel0.enable();
	_channel1.enable();
	_channel2.enable();
	_channel3.enable();
	_channel4.enable();
	_channel5.enable();

	return 0;
}

int ASound::command4() {
	_channel6.setPtr2(ADLIB_NULLDATA);
	_channel7.setPtr2(ADLIB_NULLDATA);
	_channel8.setPtr2(ADLIB_NULLDATA);

	return 0;
}

int ASound::command5() {
	_channel6.enable();
	_channel7.enable();
	_channel8.enable();

	return 0;
}

int ASound::command6() {
	_isDisabled = true;

	/* Save sweep counters */
	_channel0._savedFreqSweep = _channel0._freqSweepCounter;
	_channel1._savedFreqSweep = _channel1._freqSweepCounter;
	_channel2._savedFreqSweep = _channel2._freqSweepCounter;
	_channel3._savedFreqSweep = _channel3._freqSweepCounter;
	_channel4._savedFreqSweep = _channel4._freqSweepCounter;
	_ch5SweepSaved = _ch5SweepLive;   /* channel5 shadow */
	_channel6._savedFreqSweep = _channel6._freqSweepCounter;
	_channel7._savedFreqSweep = _channel7._freqSweepCounter;
	_channel8._savedFreqSweep = _channel8._freqSweepCounter;

	/* Zero sweep counters */
	_channel0._freqSweepCounter = 0;
	_channel1._freqSweepCounter = 0;
	_channel2._freqSweepCounter = 0;
	_channel3._freqSweepCounter = 0;
	_channel4._freqSweepCounter = 0;
	_ch5SweepLive = 0;
	_channel6._freqSweepCounter = 0;
	_channel7._freqSweepCounter = 0;
	_channel8._freqSweepCounter = 0;

	/*
	 * Mute output operators for voices 3-5 (operator registers 0x43-0x45,
	 * 0x4B-0x4D, 0x53-0x55 in the OPL2 register map).
	 * These correspond to the modulator operators for channels 3, 4, 5 and
	 * their carrier operators.
	 */
	adlib_channelOff(0x43);
	adlib_channelOff(0x44);
	adlib_channelOff(0x45);
	adlib_channelOff(0x4B);
	adlib_channelOff(0x4C);
	adlib_channelOff(0x4D);
	adlib_channelOff(0x53);
	adlib_channelOff(0x54);
	adlib_channelOff(0x55);

	return 0;
}

int ASound::command7() {
	/* Restore operator volumes */
	adlib_channelOn(67, _channel0._volume);
	adlib_channelOn(68, _channel1._volume);
	adlib_channelOn(69, _channel2._volume);
	adlib_channelOn(75, _channel3._volume);
	adlib_channelOn(76, _channel4._volume);
	adlib_channelOn(77, _channel5._volume);

	/* Check if any sweep counter was saved (OR all savedFreqSweep fields) */
	uint8 anySweep =
		_channel0._savedFreqSweep |
		_channel1._savedFreqSweep |
		_channel2._savedFreqSweep |
		_channel3._savedFreqSweep |
		_channel4._savedFreqSweep |
		_ch5SweepSaved |
		_channel6._savedFreqSweep |
		_channel7._savedFreqSweep |
		_channel8._savedFreqSweep;

	if (anySweep != 0) {
		/* Re-save live sweep counters (which are currently 0 from command6) */
		_channel0._savedFreqSweep = _channel0._freqSweepCounter;
		_channel1._savedFreqSweep = _channel1._freqSweepCounter;
		_channel2._savedFreqSweep = _channel2._freqSweepCounter;
		_channel3._savedFreqSweep = _channel3._freqSweepCounter;
		_channel4._savedFreqSweep = _channel4._freqSweepCounter;
		_ch5SweepSaved = _ch5SweepLive;
		_channel6._savedFreqSweep = _channel6._freqSweepCounter;
		_channel7._savedFreqSweep = _channel7._freqSweepCounter;
		_channel8._savedFreqSweep = _channel8._freqSweepCounter;
		signalSoundPlaying();
	}

	_isDisabled = false;
	return 0;
}

int ASound::command8() {
	uint8 result = 0;
	result |= _channel0._activeCount;
	result |= _channel1._activeCount;
	result |= _channel2._activeCount;
	result |= _channel3._activeCount;
	result |= _channel4._activeCount;
	result |= _channel5._activeCount;
	result |= _channel6._activeCount;
	result |= _channel7._activeCount;
	result |= _channel8._activeCount;

	return result;
}

void ASound::callFunction(uint16 offset) {
	error("Unsupported call to sound driver function at offset %.4x", offset);
}

/* =========================================================================
 * Support functions
 * ========================================================================= */

void ASound::write(uint8 reg, uint8 value) {
	_adlibPorts[reg] = value;
	_queue.push(Common::Pair<byte, byte>(reg, value));
}

void ASound::onTimer() {
	Common::StackLock slock(_driverMutex);

	poll();
	flush();
}

void ASound::flush() {
	while (!_queue.empty()) {
		auto v = _queue.pop();
		_opl->writeReg(v.first, v.second);
	}
}

uint16 ASound::getRandomNumber() {
	uint16 ax = 0x9248 + _randomSeed;
	/* ror ax, 3  (three right-rotates of a 16-bit value) */
	ax = (ax >> 3) | (ax << 13);
	_randomSeed = ax;
	return ax;
}

void ASound::adlib_channelOff(uint8 portIndex) {
	uint8 val = _adlibPorts[portIndex] | 0x3F;
	write(portIndex, val);
}

void ASound::adlib_channelOn(uint8 portIndex, uint8 vol) {
	uint8 val = (_adlibPorts[portIndex] & 0xC0) | (vol ^ 0x3F);
	write(portIndex, val);
}

void ASound::signalSoundPlaying() {
	if (_resultFlag == 1)
		return;
	_resultFlag = 1;
	_pollResult = 1;
}

void ASound::clearCallback() {
	_callbackCounter = 0;
	_callbackPeriod = 0;
}

/* =========================================================================
 * writeVolume
 * Computes and writes the total-level (volume) registers for the current
 * active channel to the OPL chip.
 *
 * Two paths depending on _adlib_v5660_2:
 *   < 0x18  -> simple linear mapping of (volume + velocity) -> TL
 *   >= 0x18 -> patch-attenuation-aware mapping using PATCH_ATTEN_TO_TL table
 * ========================================================================= */

void ASound::writeVolume() {
	AdlibChannel *ch = _activeChannelPtr;
	int16 var4, var6 = 0;

	/* Step 1: map volume through VOL_VEL_TO_ATTEN_STEP */
	int16 volStep = (int16)VOL_VEL_TO_ATTEN_STEP[(uint8)ch->_volume];
	/* Step 2: map velocity through VOL_VEL_TO_ATTEN_STEP */
	int16 velStep = (int16)VOL_VEL_TO_ATTEN_STEP[(int8)ch->_velocity];
	int16 var8 = volStep + velStep - 1;   /* combined attenuation step */

	/* Determine carrier operator register for this voice */
	uint8 chanNum = _activeChannelNumber;
	uint8 slot = VOICE_SLOTS[chanNum][1];          /* modulator slot index */
	uint8 regOff = SLOT_TO_REG_OFFSET[slot];
	uint16 siReg = (uint16)regOff + 0x40;         /* TL register base */

	/* KSL flags from port shadow */
	uint8 kslBits = _adlibPorts[siReg] & 0xC0;

	if (OPL_VERSION_FLAG < 0x18) {
		/* ---- OPL2 simple path ---- */
		if (var8 < 0)  var8 = 0;
		if (var8 > 63) var8 = 63;
		var4 = (0x3F - var8) | kslBits;
		write((uint8)siReg, (uint8)var4);
	} else {
		/* ---- OPL3 patch-attenuation path ---- */
		/* Modulator TL */
		int16 patchAtt = (int16)ch->_patchAttenuation;
		int16 tl1 = PATCH_ATTEN_TO_TL[patchAtt];
		var4 = -(tl1 - var8);           /* negate of (table[patchAtt] - combined) */
		if (var4 < 0)  var4 = 0;
		if (var4 > 63) var4 = 63;
		var4 = (0x3F - var4) | kslBits;
		write((uint8)siReg, (uint8)var4);

		/* Carrier TL (uses complementary lookup at PATCH_ATTEN_TO_TL - patchAtt) */
		int16 tl2 = PATCH_ATTEN_TO_TL[127 - patchAtt];   /* PATCH_ATTEN_TO_TL offset */
		var6 = -(tl2 - var8);
		if (var6 < 0)  var6 = 0;
		if (var6 > 63) var6 = 63;
		/* carrier register */
		uint8 cslot = VOICE_SLOTS[chanNum][1];
		uint8 cregOff = SLOT_TO_REG_OFFSET[cslot];
		uint16 cReg = (uint16)cregOff + 0x40;
		uint8 cksl = _adlibPorts[cReg] & 0xC0;
		var6 = (0x3F - var6) | cksl;
		write((uint8)(siReg + 2), (uint8)var4);   /* mod in OPL3 second pair */
		write((uint8)cReg, (uint8)var6);
	}

	/* Cache the written TL bytes in the channel struct */
	ch->_opVolBytes = (uint8)(var4 & 0x3F);
	ch->_opVolBytes2 = (uint8)(var6 & 0x3F);

	/* ---- Second operator (alg == 0 check) ---- */
	int sampleIdx = (int)ch->_sampleIndex;
	AdlibSample *smp = &_samples[sampleIdx * 2];
	if (smp->_alg != 0)
		return;   /* FM: only one operator carries volume */

	/* Second operator TL register */
	uint8 slot2 = VOICE_SLOTS[chanNum][0];
	uint8 roff2 = SLOT_TO_REG_OFFSET[slot2];
	uint16 siReg2 = (uint16)roff2 + 0x40;
	uint8 ksl2 = _adlibPorts[siReg2] & 0xC0;

	int16 tl_s = (int16)smp->_totalLevel - 63;
	tl_s = -tl_s;   /* neg */
	int16 var8b = (int16)var8 - tl_s;   /* subtract from combined */

	if (OPL_VERSION_FLAG < 0x18) {
		if (var8b < 0)  var8b = 0;
		if (var8b > 63) var8b = 63;
		int16 val2 = (0x3F - var8b) | ksl2;
		write((uint8)siReg2, (uint8)val2);
	} else {
		int16 patchAtt2 = (int16)ch->_patchAttenuation;
		int16 tl1b = PATCH_ATTEN_TO_TL[patchAtt2];
		var4 = -(tl1b - var8b);
		if (var4 < 0)  var4 = 0;
		if (var4 > 63) var4 = 63;
		var4 = (0x3F - var4) | ksl2;
		write((uint8)siReg2, (uint8)var4);

		int16 tl2b = PATCH_ATTEN_TO_TL[127 - patchAtt2];
		var6 = -(tl2b - var8b);
		if (var6 < 0)  var6 = 0;
		if (var6 > 63) var6 = 63;
		var6 = (0x3F - var6) | ksl2;
		write((uint8)(siReg2 + 2), (uint8)var4);
		write((uint8)(siReg2 + 0), (uint8)var6);
	}
}

void ASound::writeFrequency() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;
	uint16 aReg = (uint16)chanNum + 0xA0;   /* freq-low register */
	uint16 bReg = aReg + 0x10;                /* freq-high / keyon register */

	/* Note number (1-based), adjusted by octaveTranspose, clamped */
	int note = (int)ch->_note + (int)ch->_octaveTranspose - 1;

	/* Octave = note / 12 */
	uint8 octave = (uint8)(note / 12);

	/* Semitone within octave */
	int semi = note % 12;
	if (semi < 0) {
		semi += 12; octave--;
	}

	/* F-number from table, offset by transpose */
	int16 fnumLow = (int16)SEMITONE_FREQ_TABLE[semi] + (int16)(int8)ch->_transpose;

	/* Write F-number low byte to 0xA0+ch */
	write((uint8)aReg, (uint8)fnumLow);

	/* Build high register: preserve key-on bit, add F-num high bits and octave */
	uint8 bVal = _adlibPorts[bReg] & 0x20;        /* keep key-on */
	bVal |= (uint8)((fnumLow >> 8) & 0x03);       /* F-num bits 8-9 */
	bVal |= (uint8)(octave << 2);                 /* octave in bits 2-4 */
	write((uint8)bReg, bVal);
}

void ASound::writePitchBend() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;
	uint16 aReg = (uint16)chanNum + 0xA0;
	uint16 bReg = aReg + 0x10;

	/* Reconstruct 10-bit F-number from port shadow */
	uint16 fnum = (uint16)_adlibPorts[aReg]
		| ((uint16)(_adlibPorts[bReg] & 0x1F) << 8);

	/* Add signed pitch bend */
	int16 bent = (int16)fnum + (int16)(int8)ch->_pitchBend;

	/* Write low byte */
	write((uint8)aReg, (uint8)bent);

	/* Write high byte: keep key-on and octave bits, update F-num[9:8] */
	uint8 bVal = _adlibPorts[bReg] & 0x20;
	bVal |= (uint8)((bent >> 8) & 0x03);
	write((uint8)bReg, bVal);
}

void ASound::updateOctave() {
	uint8 chanNum = _activeChannelNumber;
	uint16 bReg = (uint16)chanNum + 0xB0;
	uint8 val = _adlibPorts[bReg] & 0xDF;   /* clear bit 5 */
	write((uint8)bReg, val);
}

void ASound::noteOn() {
	AdlibChannel *ch = _activeChannelPtr;

	writeVolume();

	int sampleIdx = (int)ch->_sampleIndex;
	AdlibSample *smp = &_samples[sampleIdx];
	ch->_freqSweepCounter = smp->_freqSweepInit;

	if (ch->_freqSweepCounter != 0) {
		/* Sweep active: signal playing but don't key on yet */
		signalSoundPlaying();
		return;
	}

	/* No sweep: write frequency and key on */
	writeFrequency();

	uint8 chanNum = _activeChannelNumber;
	uint16 bReg = (uint16)chanNum + 0xB0;
	uint8 bVal = _adlibPorts[bReg] | 0x20;   /* set key-on bit */
	write((uint8)bReg, bVal);
}

void ASound::writeSampleRegs() {
	AdlibSample *smp = _samplePtr;
	uint16 base = _currentOpBase;

	/* --- Register 0xBD: Hi-hat / Cymbal / percussion bits --- */
	uint8 bdVal = 0;
	if (RHYTHM_HI_HAT) bdVal |= 0x80;
	if (RHYTHM_CYMBAL) bdVal |= 0x40;
	bdVal |= _adlibPorts[0xBD] & 0x3F;
	write(0xBD, bdVal);

	/* --- Register 0x08: Note-select --- */
	uint8 ns = RHYTHM_ENABLE ? 0x40 : 0x00;  // was !=, should be ==
	write(0x08, ns);

	/* --- Register 0xC0+voice: Feedback / algorithm --- */
	uint8 value = ((uint16)smp->_feedback << 1) | (smp->_alg == 1 ? 0 : 1);
	write(_activeChannelReg + 0xC0, value);

	/* --- Register 0x60+base: Attack/Decay --- */
	uint8 ad = (uint8)(((smp->_attackRate & 0x0F) << 4) | (smp->_decayRate & 0x0F));
	write((uint8)(base + 0x60), ad);

	/* --- Register 0x80+base: Sustain/Release --- */
	uint8 sr = (uint8)(((smp->_sustainLevel & 0x0F) << 4) | (smp->_releaseRate & 0x0F));
	write((uint8)(base + 0x80), sr);

	/* --- Register 0x20+base: Tremolo/Vibrato/Sustain/KSR/Multiplier --- */
	// Each flag: bit set when field == 1 (sbb/neg pattern with cmc)
	uint8 am = 0;
	if (smp->_egTyp == 1) am |= 0x20;
	if (smp->_vib == 1) am |= 0x40;
	if (smp->_ksr == 1) am |= 0x10;
	if (smp->_ampMod == 1) am |= 0x80;
	am |= smp->_freqMultiple & 0x0F;
	write((uint8)(base + 0x20), am);

	/* --- Register 0xE0+base: Waveform select --- */
	write((uint8)(base + 0xE0), (uint8)(smp->_waveformSelect & 0x03));
}

void ASound::loadSample() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 chanNum = _activeChannelNumber;

	/* --- Phase 1: Silence modulator operator --- */
	uint8 mSlot = VOICE_SLOTS[chanNum][1];
	_currentOpBase = SLOT_TO_REG_OFFSET[mSlot];
	write(_currentOpBase + 0x80, 0xFF);
	write(_currentOpBase + 0x40, 63);

	/* --- Phase 2: Silence carrier operator --- */
	uint8 cSlot = VOICE_SLOTS[chanNum][0];
	_currentOpBase = SLOT_TO_REG_OFFSET[cSlot];
	write(_currentOpBase + 0x80, 0xFF);
	write(_currentOpBase + 0x40, 63);

	/* --- Phase 3: First writeSampleRegs --- */
	_activeChannelReg = (uint16)chanNum;
	_samplePtr = &_samples[ch->_sampleIndex * 2];      // operator 1
	writeSampleRegs();

	/* --- Phase 4: Second writeSampleRegs --- */
	_samplePtr = &_samples[ch->_sampleIndex * 2 + 1];  // operator 2
	_currentOpBase = SLOT_TO_REG_OFFSET[VOICE_SLOTS[chanNum][1]];
	writeSampleRegs();

	/* --- Phase 5: Carrier TL write --- */
	write(_currentOpBase + 0x40, (uint8)((_samplePtr->_scalingLevel << 6) | 0x3F));

	/* --- Phase 6: Third sample --- */
	uint8 thirdIdx = *((uint8 *)ch + 5);
	_samplePtr = &_samples[thirdIdx * 2];   // operator 1 of third sample
	AdlibSample *smp3 = _samplePtr;
	uint16 cRegPlus40 = SLOT_TO_REG_OFFSET[VOICE_SLOTS[chanNum][0]] + 0x40;

	if (smp3->_alg == 0) {
		write(cRegPlus40, (uint8)((smp3->_scalingLevel << 6) | 0x3F));
	} else {
		uint8 tl = (uint8)(-(smp3->_totalLevel - 63) & 0xFF);
		write(cRegPlus40, (uint8)((smp3->_scalingLevel << 6) | (tl & 0x3F)));
	}

	/* --- Copy sweep/freq parameters from sample into channel --- */
	ch->_freqSweepCounter = smp3->_freqSweepInit;
	ch->_noiseFreqMask = smp3->_freqMask;
	ch->_freqAccum = smp3->_freqBase;
	ch->_freqStep = smp3->_outerLoopPtr;
}

void ASound::update() {
	if (!_isDisabled) {
		(void)getRandomNumber();
		++_frameNumber2;

		pollAllChannels();
		updateAllChannels();
		_anySweepActive = false;

		for (int chan = 0; chan < ADLIB_CHANNEL_COUNT; ++chan)
			update1(chan);

		if (!_anySweepActive) {
			if (_resultFlag != -1) {
				_resultFlag = -1;
				_pollResult = -1;
			}
		}
	}
}

void ASound::update1(int channelIndex) {
	AdlibChannel *ch = _channels[channelIndex];

	if (ch->_freqSweepCounter == 0)
		return;

	_anySweepActive = 1;
	ch->_freqAccum += ch->_freqStep;
	ch->_freqSweepCounter--;

	if (ch->_freqSweepCounter != 0)
		return;

	/* Sweep finished: write zeroes to the frequency registers */
	uint8 voice = (uint8)channelIndex;
	write((uint8)(voice + 0xA0), 0x00);
	write((uint8)(voice + 0xB0), 0x00);
}

void ASound::setFrequency(uint8 voice, uint16 freq) {
	write((uint8)(voice + 0xA0), (uint8)(freq & 0xFF));
	uint8 bVal = (_adlibPorts[voice + 0xB0] & 0x20)
		| (uint8)((freq >> 8) & 0x03)
		| 0x20;   /* key-on */
	write((uint8)(voice + 0xB0), bVal);
}

void ASound::noise_inner(int channelIndex) {
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
	_findChannelMode = 1;

	for (int chan = 0; chan < 6; ++chan) {
		if (_channels[chan]->_activeCount == 0) {
			_channels[chan]->load(soundData);
			return;
		}
	}

	/* Fall through to full search (channels 6-8 + pending-stop channels) */
	findFreeChannelFull(soundData);
}

void ASound::findFreeChannelFull(byte *soundData) {
	int chan;
	_findChannelMode = 2;

	// Check channels 6-8 for empty slots
	for (chan = 6; chan < ADLIB_CHANNEL_COUNT; ++chan) {
		if (_channels[chan]->_activeCount == 0) {
			_channels[chan]->load(soundData);
			return;
		}
	}

	// Check channels 6-8 for pending-stop (can be pre-empted)
	for (chan = ADLIB_CHANNEL_COUNT - 1; chan >= 6; --chan) {
		if (_channels[chan]->_pendingStop == 0xff) {
			_channels[chan]->load(soundData);
			return;
		}
	}

	// Fall through to check channels 0-5 pending-stop
	if (_ch5PendingStop != 0xFF) {
		for (chan = 4; chan >= 0; --chan) {
			if (_channels[chan]->_pendingStop == 0xff) {
				_channels[chan]->load(soundData);
				return;
			}
		}
	}

	// No free channel found - sound is dropped
}

void ASound::pollAllChannels() {
	_activeChannelNumber = 0;
	for (int chan = 0; chan < ADLIB_CHANNEL_COUNT; ++chan) {
		_activeChannelPtr = _channels[chan];
		pollActiveChannel();
	}
}

void ASound::pollActiveChannel() {
	AdlibChannel *ch = _activeChannelPtr;
	uint8 var_8 = 0;		// volume-dirty flag
	uint16 var_6;			// temp word
	uint16 var_4 = 0;		// temp word 2
	uint16 var_A = 0;		// temp word 3
	uint16 var_C = 0;		// temp word 4

	/* Channel inactive - nothing to do */
	if (ch->_activeCount == 0) goto done;

	/* Key-on delay countdown */
	if (ch->_keyOnDelay != 0) {
		ch->_keyOnDelay--;
		if (ch->_keyOnDelay == 0)
			updateOctave();
	}

	/* Decrement the duration counter; when it hits zero, fetch next command */
	ch->_activeCount--;
	if (ch->_activeCount != 0)
		goto post_keyon;    /* still counting down - skip to pitch-bend / fade */

	/* ---- top of command dispatch (def_11A34) ---- */
dispatch:
	{
		/* Load pSrc from channel and read the first byte */
		pSrc = ch->_pSrc;
		uint8 b = *pSrc;

		if (!(b & 0x80)) {
			/*
			 * Note byte (bit 7 clear): two-byte format [note][duration].
			 * Read note into _note, duration into _activeCount, advance pSrc.
			 */
			ch->_note = *pSrc;   /* b already */
			pSrc++;
			ch->_activeCount = *pSrc;
			pSrc++;
			ch->_pSrc += 2;

			if (ch->_note == 0 || ch->_activeCount == 0) {
				updateOctave();
				goto post_keyon;
			}
			goto key_on;
		}

		/* High bit set - it is a command.  Commands above 0xBD go to switch. */
		if (b > 0xBD) {
			/*
			 * Decode the command opcode.
			 * The byte is sign-extended to int8 (0xFF..-0x80 -> -1..-128),
			 * then ax = int8(b) - (-66).
			 *
			 * Opcode -> ax mapping (selected):
			 *   0xFF (-1)  -> 65    0xFE (-2)  -> 64    0xFD (-3)  -> 63
			 *   0xFC (-4)  -> 62    0xFB (-5)  -> 61    0xFA (-6)  -> 60
			 *   0xF9 (-7)  -> 59    0xF8 (-8)  -> 58    0xF7 (-9)  -> 57
			 *   0xF6 (-10) -> 56    0xF5 (-11) -> 55    0xF4 (-12) -> 54
			 *   0xF3 (-13) -> 53    0xF2 (-14) -> 52    0xF1 (-15) -> 51
			 *   0xF0 (-16) -> 50    0xEF (-17) -> 49    0xEE (-18) -> 48
			 *   0xED (-19) -> 47    0xEC (-20) -> 46    0xEB (-21) -> 45
			 *   0xEA (-22) -> 44    0xE9 (-23) -> 43    0xE8 (-24) -> 42
			 *   0xE7 (-25) -> 41    0xE6 (-26) -> 40    0xE5 (-27) -> 39
			 *   0xE4 (-28) -> 38    0xE3 (-29) -> 37    0xE2 (-30) -> 36
			 *   0xE1 (-31) -> 35    0xE0 (-32) -> 34    0xDF (-33) -> 33
			 *   0xDE (-34) -> 32    0xDD (-35) -> 31    0xDC (-36) -> 30
			 *   0xDB (-37) -> 29    0xDA (-38) -> 28    0xD9 (-39) -> 27
			 *   0xD8 (-40) -> 26    0xD7 (-41) -> 25    0xD6 (-42) -> 24
			 *   0xD5 (-43) -> 23    0xD4 (-44) -> 22    0xD3 (-45) -> 21
			 *   0xD2 (-46) -> 20    0xD1 (-47) -> 19    0xD0 (-48) -> 18
			 *   0xCF (-49) -> 17    0xCE (-50) -> 16    0xCD (-51) -> 15
			 *   0xCC (-52) -> 14    0xCB (-53) -> 13    0xCA (-54) -> 12
			 *   0xC9 (-55) -> 11    0xC8 (-56) -> 10    0xC7 (-57) ->  9
			 *   0xC6 (-58) ->  8    0xC5 (-59) ->  7    0xC4 (-60) ->  6
			 *   0xC3 (-61) ->  5    0xC2 (-62) ->  4    0xC1 (-63) ->  3
			 *   0xC0 (-64) ->  2
			 */
			int16 ax = (int16)(int8)b;   /* sign-extend */
			var_8 = 0;
			ax = (int16)(ax - (-66));
			if ((uint16)ax > 65) goto dispatch;  /* unknown - skip */

			switch (ax) {

				/* ---- opcode -1  (0xFF): inner loop ---- */
			case 65:
			{
				if (ch->_innerLoopCount == 0) {
					pSrc++;
					uint8 cnt = *pSrc;
					if (cnt == 0) {
						ch->_pSrc += 2;
						ch->_innerLoopPtr = ch->_pSrc;
						ch->_innerLoopCount = 0;
						goto dispatch;
					}
					ch->_innerLoopCount = (uint16)cnt;
				}
				ch->_innerLoopCount--;
				if (ch->_innerLoopCount == 0) {
					ch->_pSrc += 2;
					ch->_innerLoopPtr = ch->_pSrc;
					goto dispatch;
				}
				ch->_pSrc = ch->_innerLoopPtr;
				goto dispatch;
			}

			/* ---- opcode -2  (0xFE): outer loop ---- */
			case 64:
			{
				if (ch->_outerLoopCount == 0) {
					pSrc++;
					uint8 cnt = *pSrc;
					if (cnt == 0) {
						ch->_pSrc += 2;
						ch->_outerLoopPtr = ch->_pSrc;
						ch->_innerLoopPtr = ch->_pSrc;
						ch->_innerLoopCount = 0;
						ch->_outerLoopCount = 0;
						goto dispatch;
					}
					ch->_outerLoopCount = (uint16)cnt;
				}
				ch->_outerLoopCount--;
				if (ch->_outerLoopCount == 0) {
					ch->_pSrc += 2;
					ch->_outerLoopPtr = ch->_pSrc;
					ch->_innerLoopPtr = ch->_pSrc;
					goto dispatch;
				}
				ch->_pSrc = ch->_outerLoopPtr;
				ch->_innerLoopPtr = ch->_outerLoopPtr;
				goto dispatch;
			}

			/* ---- opcode -3  (0xFD): restart / reset all channel state ---- */
			case 63:
			{
				ch->_loopStartPtr = ch->_soundData;
				ch->_pSrc = ch->_soundData;
				ch->_innerLoopPtr = ch->_soundData;
				ch->_outerLoopPtr = ch->_soundData;
				ch->_pitchBend = 0;
				ch->_volumeFadeStep = 0;
				ch->_vibratoDepth = 0;
				ch->_transpose = 0;
				ch->_velocity = 0;
				ch->_opVolBytes = 0;
				ch->_opVolBytes2 = 0;
				ch->_volume = 0;
				ch->_fadePeriodCounter = 0;
				ch->_vibPeriodCounter = 0;
				ch->_innerLoopCount = 0;
				ch->_outerLoopCount = 0;
				ch->_noteOffset = 0;
				goto dispatch;
			}

			/* ---- opcode -4  (0xFC): set sound-data pointer (abs) ---- */
			case 62:
			{
				byte *ptr = &_soundData[readWord_impl()];
				ch->_loopStartPtr = ptr;
				ch->_pSrc = ptr;
				ch->_innerLoopPtr = ptr;
				ch->_outerLoopPtr = ptr;
				ch->_soundData = ptr;
				goto dispatch;
			}

			/* ---- opcode -5  (0xFB): branch (unconditional) ---- */
			case 61:
			{
				byte *ptr = &_soundData[readWord_impl()];
				ch->_pSrc = ptr;
				goto dispatch;
			}

			/* ---- opcode -6  (0xFA): branch-with-save ---- */
			case 60:
			{
				byte *ptr = &_soundData[readWord_impl()];
				ch->_branchTargetPtr = ch->_pSrc + 3;
				ch->_pSrc = ptr;
				goto dispatch;
			}

			/* ---- opcode -7  (0xF9): return-from-branch ---- */
			case 59:
			{
				if (ch->_branchTargetPtr != 0) {
					ch->_pSrc = ch->_branchTargetPtr;
					ch->_branchTargetPtr = 0;
				} else {
					ch->_pSrc++;
				}
				goto dispatch;
			}

			/* ---- opcode -8  (0xF8): set sample / patch ---- */
			case 58:
			{
				pSrc++;
				ch->_sampleIndex = *pSrc;
				ch->_pSrc += 2;
				loadSample();
				goto dispatch;
			}

			/* ---- opcode -9  (0xF7): set note offset (with 0 duration override) ---- */
			case 57:
			{
				pSrc++;
				ch->_noteOffset = *pSrc;
				ch->_durationOverride = 0;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -10 (0xF6): set duration override ---- */
			case 56:
			{
				pSrc++;
				ch->_durationOverride = *pSrc;
				ch->_noteOffset = 0;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -11 (0xF5): set pitch bend ---- */
			case 55:
			{
				pSrc++;
				ch->_pitchBend = *pSrc;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -12 (0xF4): set volume ---- */
			case 54:
			{
				pSrc++;
				uint8 vol = *pSrc;
				var_6 = (uint16)vol;
				if (ch->_pendingStop != 0) {
					/* Clamp: don't raise volume above current when fading out */
					if ((uint16)ch->_volume <= var_6)
						goto set_vol_patch;
				}
				ch->_volume = (uint8)var_6;
				goto vol_advance;
set_vol_patch:
				ch->_volume = vol;
vol_advance:
				ch->_pSrc += 2;
				var_8 = 1;
				goto dispatch;
			}

			/* ---- opcode -13 (0xF3): set fade ---- */
			case 53:
			{
				if (ch->_pendingStop != 0) {
					ch->_pSrc += 3;
					goto dispatch;
				}
				pSrc++;
				ch->_fadePeriodReload = *pSrc;
				pSrc++;
				ch->_volumeFadeStep = *pSrc;
				ch->_fadePeriodCounter = 1;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -14 (0xF2): set transpose ---- */
			case 52:
			{
				pSrc++;
				ch->_transpose = *pSrc;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -15 (0xF1): set velocity ---- */
			case 51:
			{
				pSrc++;
				uint8 vel = *pSrc;
				var_6 = (uint16)vel;
				if (ch->_pendingStop != 0) {
					if ((uint16)ch->_velocity > var_6)
						ch->_velocity = vel;
				} else {
					ch->_velocity = vel;
				}
				var_8 = 1;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -16 (0xF0): set patch attenuation ---- */
			case 50:
			{
				pSrc++;
				ch->_patchAttenuation = *pSrc;
				ch->_pSrc += 2;
				var_8 = 1;
				goto dispatch;
			}

			/* ---- opcode -17 (0xEF): set vibrato ---- */
			case 49:
			{
				pSrc++;
				ch->_vibPeriodReload = *pSrc;
				pSrc++;
				ch->_vibratoDepth = *pSrc;
				ch->_vibPeriodCounter = 1;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -18 (0xEE): set octave transpose ---- */
			case 48:
			{
				pSrc++;
				ch->_octaveTranspose = *pSrc;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -19 (0xED): skip forward by N+3 bytes ---- */
			case 47:
			{
				pSrc++;
				var_C = *pSrc;
				uint16 skip = var_C + 3;
				ch->_pSrc += skip;
				goto dispatch;
			}

			/* ---- opcode -20 (0xEC): random pick from table ---- */
			case 46:
			{
				pSrc++;
				uint8 tblSize = *pSrc;
				var_C = (uint16)tblSize;
				pSrc++;
				byte *base = pSrc;

				(void)getRandomNumber();
				uint16 rnd = _randomSeed & 0x7FFF;
				uint16 idx = (uint16)((int16)rnd % (int16)var_C);
				var_6 = idx;

				uint8 chosen = *(base + idx);
				uint8 target = *(base + var_C);
				/* Write chosen into table[target+1] */
				*(base + target + 1) = chosen;

				uint16 advance = var_C + 3;
				ch->_pSrc += advance;
				goto dispatch;
			}

			/* ---- opcode -21 (0xEB): random value in range [lo..hi] ---- */
			case 45:
			{
				pSrc++;
				var_A = *pSrc;
				pSrc++;
				var_4 = *pSrc;
				pSrc++;
				var_C = var_4 - var_A + 1;

				(void)getRandomNumber();
				uint16 rnd = _randomSeed & 0x7FFF;
				var_6 = (uint16)((int16)rnd % (int16)var_C);

				byte *base = pSrc;
				uint8 targetSlot = *base;
				uint8 result = var_6 + var_A;
				*(base + targetSlot + 1) = result;

				ch->_pSrc += 4;
				goto dispatch;
			}

			/* ---- opcode -22 (0xEA): indirect random pick from var table ---- */
			case 44:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				pSrc++;
				byte *base = pSrc;
				uint8 indIdx = _scriptVars[var_6];
				uint8 chosen = *(base + (uintptr_t)indIdx);
				uint8 target = *(base + var_C);
				*(base + target + 1) = chosen;
				uint16 advance = (uint16)(var_C + 4);
				ch->_pSrc += advance;
				goto dispatch;
			}

			/* ---- opcode -23 (0xE9): store immediate into script var ---- */
			case 43:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 val = *pSrc;
				_scriptVars[var_6] = val;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -24 (0xE8): var-to-var copy ---- */
			case 42:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] = _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -25 (0xE7): indirect copy to stream ---- */
			case 41:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				byte *base = pSrc;
				uint8 src = _scriptVars[var_6];
				*(base + var_C + 1) = src;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -26 (0xE6): increment script var ---- */
			case 40:
			{
				pSrc++;
				var_6 = *pSrc;
				_scriptVars[var_6]++;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -27 (0xE5): decrement script var ---- */
			case 39:
			{
				pSrc++;
				var_6 = *pSrc;
				_scriptVars[var_6]--;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -28 (0xE4): add immediate to script var ---- */
			case 38:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] += imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -29 (0xE3): add var to script var ---- */
			case 37:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] += _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -30 (0xE2): subtract immediate from script var ---- */
			case 36:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] -= imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -31 (0xE1): subtract var from script var ---- */
			case 35:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] -= _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -32 (0xE0): multiply script var by var ---- */
			case 34:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] *= _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -33 (0xDF): multiply script var by immediate ---- */
			case 33:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] *= imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -34 (0xDE): divide script var by immediate -> quotient ---- */
			case 32:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] = (uint8)((int8)_scriptVars[var_6] / (int8)var_C);
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -35 (0xDD): divide var by var -> quotient to dst ---- */
			case 31:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] = (uint8)((uint8)_scriptVars[var_6] / (uint8)_scriptVars[var_C]);
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -36 (0xDC): divide script var by immediate -> remainder ---- */
			case 30:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] = (uint8)((int8)_scriptVars[var_6] % (int8)var_C);
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -37 (0xDB): divide var by var -> remainder to dst ---- */
			case 29:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] = (uint8)((uint8)_scriptVars[var_6] % (uint8)_scriptVars[var_C]);
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -38 (0xDA): AND script var by immediate ---- */
			case 28:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] &= imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -39 (0xD9): AND var by var ---- */
			case 27:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] &= _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -40 (0xD8): OR script var by immediate ---- */
			case 26:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] |= imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -41 (0xD7): OR var by var ---- */
			case 25:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] |= _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -42 (0xD6): XOR script var by immediate ---- */
			case 24:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				uint8 imm = *pSrc;
				_scriptVars[var_6] ^= imm;
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -43 (0xD5): XOR var by var ---- */
			case 23:
			{
				pSrc++;
				var_6 = *pSrc;
				pSrc++;
				var_C = *pSrc;
				_scriptVars[var_6] ^= _scriptVars[var_C];
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- Comparison / conditional-branch opcodes -44 through -59 ----
			 * Each reads two operands, compares them, and either branches
			 * (taken: read next word as target -> pSrc) or skips 5 bytes ahead.
			 * For opcodes -44..-51 the two operands are: imm vs var[idx].
			 * For opcodes -52..-59 they are: var[idx] vs var[idx2].
			 */

			 /* ---- opcode -44 (0xD4): branch if imm == var ---- */
			case 22:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				uint8 v = _scriptVars[var_6];
				if ((uint16)var_C == (uint16)v) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -45 (0xD3): branch if imm != var ---- */
			case 21:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C != (uint16)_scriptVars[var_6]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -46 (0xD2): branch if imm > var ---- */
			case 20:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C > (uint16)_scriptVars[var_6]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -47 (0xD1): branch if imm >= var ---- */
			case 19:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C >= (uint16)_scriptVars[var_6]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -48 (0xD0): branch if var[idx1] == var[idx2] ---- */
			case 18:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] == _scriptVars[var_C]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -49 (0xCF): branch if var[idx1] != var[idx2] ---- */
			case 17:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] != _scriptVars[var_C]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -50 (0xCE): branch if var[idx1] > var[idx2] ---- */
			case 16:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] > _scriptVars[var_C]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -51 (0xCD): branch if var[idx1] < var[idx2] ---- */
			case 15:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] < _scriptVars[var_C]) goto branch_taken;
				goto branch_skip5;
			}

			/* ---- opcode -52 (0xCC): branch if imm == var (extended) ---- */
			case 14:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C == (uint16)_scriptVars[var_6]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -53 (0xCB): branch if imm != var (extended) ---- */
			case 13:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C != (uint16)_scriptVars[var_6]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -54 (0xCA): branch if imm > var (extended) ---- */
			case 12:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C > (uint16)_scriptVars[var_6]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -55 (0xC9): branch if imm < var (extended) ---- */
			case 11:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if ((uint16)var_C < (uint16)_scriptVars[var_6]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -56 (0xC8): branch if var[idx1] == var[idx2] (extended) ---- */
			case 10:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] == _scriptVars[var_C]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -57 (0xC7): branch if var[idx1] != var[idx2] (extended) ---- */
			case 9:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] != _scriptVars[var_C]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -58 (0xC6): branch if var[idx1] > var[idx2] (extended) ---- */
			case 8:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] > _scriptVars[var_C]) goto branch_taken2;
				goto branch_skip5;
			}

			/* ---- opcode -59 (0xC5): branch if var[idx1] < var[idx2] (extended) ---- */
			case 7:
			{
				pSrc++; var_6 = *pSrc;
				pSrc++; var_C = *pSrc;
				if (_scriptVars[var_6] < _scriptVars[var_C]) goto branch_taken2;
				goto branch_skip5;
			}

			/* Shared branch-taken logic for -44..-51: save pSrc+5 as return target */
branch_taken:
			{
				ch->_branchTargetPtr = ch->_pSrc + 5;
				byte *dest = &_soundData[readWord_impl()];
				ch->_pSrc = dest;
				goto dispatch;
			}

			/* Shared branch-taken logic for -52..-59: same, different label for clarity */
branch_taken2:
			{
				ch->_branchTargetPtr = ch->_pSrc + 5;
				byte *dest = &_soundData[readWord_impl()];
				ch->_pSrc = dest;
				goto dispatch;
			}

			/* Shared branch-skip: advance 5 bytes */
branch_skip5:
			{
				ch->_pSrc += 5;
				goto dispatch;
			}

			/* ---- opcode -60 (0xC4): call a function pointer ---- */
			case 6:
			{
				uint16 fptr = readWord_impl();
				callFunction(fptr);
				ch->_pSrc += 3;
				goto dispatch;
			}

			/* ---- opcode -61 (0xC3): call nullsub_1 with byte arg ---- */
			case 5:
			{
				pSrc++;
				uint8 arg = *pSrc;
				(void)arg;   /* nullsub_1 is a no-op */
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -62 (0xC2): advance pSrc by 4 ---- */
			case 4:
			{
				ch->_pSrc += 4;
				goto dispatch;
			}

			/* ---- opcode -63 (0xC1): set _tempoScale ---- */
			case 3:
			{
				pSrc++;
				_tempoScale = *pSrc;
				ch->_pSrc += 2;
				goto dispatch;
			}

			/* ---- opcode -64 (0xC0): set tempo / frame sync ---- */
			case 2:
			{
				pSrc++;
				_tempoReload = *pSrc;
				ch->_pSrc += 2;
				if (_frameNumber2 != 0)
					goto dispatch;
				_tempoCurrent = _tempoReload;
				goto dispatch;
			}

			/* ---- opcode -65 (0xBF): set _tempoTarget / _tempoBase, enable tick ---- */
			case 1:
			{
				uint16 w = readWord_impl();
				_tempoTarget = w;
				ch->_pSrc += 3;
				if (_frameNumber2 == 0)
					_tempoBase = _tempoTarget;
				_tickEnabled = 1;
				_tickCounter = 1;
				goto dispatch;
			}

			/* ---- opcode -66 (0xBE): set _tempoShift ---- */
			case 0:
			{
				pSrc++;
				_tempoShift = (int16)(int8)*pSrc;
				ch->_pSrc += 2;
				goto dispatch;
			}
			default:
				error("Unhandled sound opcode");
				break;
			} /* end switch */
		}
	} /* end command decode block */

	/* Should not reach here in normal execution */
	goto post_keyon;

key_on:
	{
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
		if (ch->_pitchBend != 0)
			writePitchBend();

		var_8 = 0;

		/* ---- Fade / vibrato processing ---- */
		if (ch->_fadePeriodCounter == 0 && ch->_vibPeriodCounter == 0)
			goto done;

		ch->_fadePeriodCounter--;
		ch = _activeChannelPtr;

		if (ch->_fadePeriodCounter == 0) {
			ch->_fadePeriodCounter = ch->_fadePeriodReload;
			ch = _activeChannelPtr;

			if (ch->_volumeFadeStep != 0) {
				if (ch->_pendingStop != 0) {
					/* Fading a channel that is pending-stop: step velocity down */
					if (ch->_velocity > 0) {
						ch->_velocity += ch->_volumeFadeStep; /* step is signed */
					}
					ch = _activeChannelPtr;
					if (ch->_volume != 0) {
						ch->_volume += ch->_volumeFadeStep;
					}
				} else {
					/* Normal fade: clamp velocity at 0..127 */
					if ((int8)ch->_volumeFadeStep > 0) {
						ch->_velocity += ch->_volumeFadeStep;
						ch = _activeChannelPtr;
						if ((int16)ch->_velocity > 0x7F)
							ch->_velocity = 0x7F;
					} else {
						ch->_velocity += ch->_volumeFadeStep;
						ch = _activeChannelPtr;
						if ((int8)ch->_velocity < 0)
							ch->_velocity = 0;
					}
				}
				var_8 = 1;
			}
		}

		/* ---- Vibrato ---- */
		ch = _activeChannelPtr;
		ch->_vibPeriodCounter--;
		ch = _activeChannelPtr;

		if (ch->_vibPeriodCounter == 0) {
			ch->_vibPeriodCounter = ch->_vibPeriodReload;
			ch = _activeChannelPtr;

			if ((int8)ch->_vibratoDepth != 0) {
				int16 pa = (int16)(int8)ch->_patchAttenuation;
				int16 vib = (int16)(int8)ch->_vibratoDepth;
				int16 sum = pa + vib;

				if ((int8)ch->_vibratoDepth > 0) {
					/* Positive vibrato: bounce if above 0x7F */
					if (sum > 0x7F)
						ch->_vibratoDepth = (uint8)(-ch->_vibratoDepth);
				} else {
					/* Negative vibrato: bounce if below zero */
					if (sum < 0)
						ch->_vibratoDepth = (uint8)(-ch->_vibratoDepth);
				}

				ch = _activeChannelPtr;
				ch->_patchAttenuation += ch->_vibratoDepth;
				var_8 = 1;
			}
		}

		if (var_8 != 0)
			writeVolume();
	}

done:
	_activeChannelNumber++;
}

bool ASound::isSoundActive(byte *ptr) const {
	for (int chan = 0; chan < ADLIB_CHANNEL_COUNT; ++chan) {
		if (_channels[chan]->_activeCount && _channels[chan]->_soundData == ptr)
			return true;
	}

	return false;
}

uint16 ASound::readWord_impl() {
	uint16 lo = *++pSrc;
	uint16 hi = *++pSrc;
	return lo | (hi << 8);
}

} // namespace MADSV2
} // namespace MADS
