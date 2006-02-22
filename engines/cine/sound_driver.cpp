/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cine/cine.h"
#include "cine/sound_driver.h"

#include "sound/mixer.h"
#include "sound/fmopl.h"

u8 snd_useAdlib = 0;
u16 snd_fadeOutCounter = 0;
u16 snd_songTicksCounter = 0;
u8 *snd_adlibInstrumentsTable[4];
sndDriverStruct snd_driver;

static u8 snd_adlibVibrato = 0;
static s16 snd_adlibChannelVolume[4];

static const u16 snd_adlibFreqTable[] = {
	0x0157, 0x016C, 0x0181, 0x0198, 0x01B1, 0x01CB, 0x01E6, 0x0203,
	0x0222, 0x0243, 0x0266, 0x028A
};

static const u8 snd_adlibOpTable[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A,
	0x0B, 0x0C, 0x0D, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15
};

static const u8 snd_adlibNoteTable[] = {
	0x00, 0x03, 0x01, 0x04, 0x02, 0x05, 0x06, 0x09, 0x07,
	0x0A, 0x08, 0x0B, 0x0C, 0x0F, 0x10, 0x10, 0x0E, 0x0E,
	0x11, 0x11, 0x0D, 0x0D, 0x00, 0x00
};

static const s16 snd_adlibNoteFreqTable[] = {
	0x0EEE, 0x0E17, 0x0D4D, 0x0C8C, 0x0BD9, 0x0B2F, 0x0A8E, 0x09F7,
	0x0967, 0x08E0, 0x0861, 0x07E8, 0x0777, 0x070B, 0x06A6, 0x0647,
	0x05EC, 0x0597, 0x0547, 0x04FB, 0x04B3, 0x0470, 0x0430, 0x03F4,
	0x03BB, 0x0385, 0x0353, 0x0323, 0x02F6, 0x02CB, 0x02A3, 0x027D,
	0x0259, 0x0238, 0x0218, 0x01FA, 0x01DD, 0x01C2, 0x01A9, 0x0191,
	0x017B, 0x0165, 0x0151, 0x013E, 0x012C, 0x011C, 0x010C, 0x00FD,
	0x00EE, 0x00E1, 0x00D4, 0x00C8, 0x00BD, 0x00B2, 0x00A8, 0x009F,
	0x0096, 0x008E, 0x0086, 0x007E, 0x0077, 0x0070, 0x006A, 0x0064,
	0x005E, 0x0059, 0x0054, 0x004F, 0x004B, 0x0047, 0x0043, 0x003F,
	0x003B, 0x0038, 0x0035, 0x0032, 0x002F, 0x002C, 0x002A, 0x0027,
	0x0025, 0x0023, 0x0021, 0x001F, 0x001D, 0x001C, 0x001A, 0x0019,
	0x0017, 0x0016, 0x0015, 0x0013, 0x0012, 0x0011, 0x0010, 0x000F
};

static void snd_adlibWriteData(int port, int value)
{
	OPLWriteReg(g_cine_adlib->getOPL(), port, value);
}

static void snd_adlibDriverSetupInstrument(const u8 *instrumentData, int channelNum) {
	s16 tmp;

	u8 waveSelect1 = instrumentData[54] & 3;	/* var2 */
	u8 waveSelect2 = instrumentData[56] & 3;	/* var1 */

	u8 fl = *instrumentData++;	/* varB */
	u8 ch = *instrumentData++;	/* var4 */

	u8 adlibOp1, adlibOp2;	/* _di, varA */

	if (fl != 0) {
		adlibOp1 = snd_adlibOpTable[snd_adlibNoteTable[ch * 2 + 0]];
		adlibOp2 = snd_adlibOpTable[snd_adlibNoteTable[ch * 2 + 1]];
	} else {
		adlibOp1 =
		    snd_adlibOpTable[snd_adlibNoteTable[channelNum * 2 + 0]];
		adlibOp2 =
		    snd_adlibOpTable[snd_adlibNoteTable[channelNum * 2 + 1]];
	}

	if (fl == 0 || ch == 6) {
		/* vibrato */
		tmp = 0;
		if (readU16LE(instrumentData + 18) != 0)
			tmp |= 0x80;
		if (readU16LE(instrumentData + 20) != 0)
			tmp |= 0x40;
		if (readU16LE(instrumentData + 10) != 0)
			tmp |= 0x20;
		if (readU16LE(instrumentData + 22) != 0)
			tmp |= 0x10;
		tmp |= (readU16LE(instrumentData + 2) & 0xF);
		snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_EG_KS + adlibOp1, tmp);

		/* key scaling */
		tmp = 0x3F - (readU16LE(instrumentData + 16) & 0x3F);
		tmp = snd_adlibChannelVolume[channelNum] * tmp;
		tmp += tmp + 0x7F;
		tmp = 0x3F - (tmp / 0xFE);
		if (readU16LE(instrumentData + 24) != 0)
			tmp = readU16LE(instrumentData + 16) & 0x3F;
		tmp |= readU16LE(instrumentData) << 6;
		snd_adlibWriteData(ADLIB_REG_KEY_SCALING_OPERATOR_OUTPUT +
		    adlibOp1, tmp);

		/* attack/decay rates */
		tmp =
		    (readU16LE(instrumentData +
			6) << 4) | (readU16LE(instrumentData + 12) & 0xF);
		snd_adlibWriteData(ADLIB_REG_ATTACK_RATE_DECAY_RATE + adlibOp1,
		    tmp);

		/* sustain/release rates */
		tmp =
		    (readU16LE(instrumentData +
			8) << 4) | (readU16LE(instrumentData + 14) & 0xF);
		snd_adlibWriteData(ADLIB_REG_SUSTAIN_LEVEL_RELEASE_RATE_0 +
		    adlibOp1, tmp);

		if (fl != 0) {
			tmp = readU16LE(instrumentData + 4) * 2;
			if (readU16LE(instrumentData + 24) == 0)
				tmp |= 1;

			snd_adlibWriteData
			    (ADLIB_REG_FEEDBACK_STRENGTH_CONNECTION_TYPE + ch,
			    tmp);
		} else {
			tmp = readU16LE(instrumentData + 4) * 2;
			if (readU16LE(instrumentData + 24) == 0)
				tmp |= 1;

			snd_adlibWriteData
			    (ADLIB_REG_FEEDBACK_STRENGTH_CONNECTION_TYPE +
			    channelNum, tmp);
		}
		snd_adlibWriteData(ADLIB_REG_WAVE_SELECT + adlibOp1,
		    waveSelect1);
		instrumentData += 26;
	}

	/* vibrato */
	tmp = 0;
	if (readU16LE(instrumentData + 18) != 0)
		tmp |= 0x80;
	if (readU16LE(instrumentData + 20) != 0)
		tmp |= 0x40;
	if (readU16LE(instrumentData + 10) != 0)
		tmp |= 0x20;
	if (readU16LE(instrumentData + 22) != 0)
		tmp |= 0x10;
	tmp |= (readU16LE(instrumentData + 2) & 0xF);
	snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_EG_KS + adlibOp2, tmp);

	/* key scaling */
	tmp = 0x3F - (readU16LE(instrumentData + 16) & 0x3F);
	tmp = snd_adlibChannelVolume[channelNum] * tmp;
	tmp += tmp + 0x7F;
	tmp = 0x3F - (tmp / 0xFE);
	tmp |= readU16LE(instrumentData) << 6;
	snd_adlibWriteData(ADLIB_REG_KEY_SCALING_OPERATOR_OUTPUT + adlibOp2,
	    tmp);

	/* attack/decay rates */
	tmp =
	    (readU16LE(instrumentData + 6) << 4) | (readU16LE(instrumentData +
		12) & 0xF);
	snd_adlibWriteData(ADLIB_REG_ATTACK_RATE_DECAY_RATE + adlibOp2, tmp);

	/* sustain/release rates */
	tmp =
	    (readU16LE(instrumentData + 8) << 4) | (readU16LE(instrumentData +
		14) & 0xF);
	snd_adlibWriteData(ADLIB_REG_SUSTAIN_LEVEL_RELEASE_RATE_0 + adlibOp2,
	    tmp);
	snd_adlibWriteData(ADLIB_REG_WAVE_SELECT + adlibOp2, waveSelect2);
}

static void snd_adlibInterrupt(void *param, s16 *buf, int len) {
	int16 *origData = buf;
	uint origLen = len;
	static int samplesLeft = 0;

	while (len != 0) {
		int count;
		if (samplesLeft == 0) {
			if (snd_songIsPlaying || (snd_fadeOutCounter != 0
				&& snd_fadeOutCounter < 100)) {
				++snd_songTicksCounter;
				if (snd_songTicksCounter > snd_eventsDelay) {
					snd_handleEvents();
					snd_songTicksCounter = 0;
				}
			}
			samplesLeft = g_cine_adlib->getRate() / 50;
		}
		count = samplesLeft;
		if (count > len)
			count = len;

		YM3812UpdateOne(g_cine_adlib->getOPL(), buf, count);

		samplesLeft -= count;
		len -= count;
		buf += count;
	}

	// Convert mono data to stereo
	for (int i = (origLen - 1); i >= 0; i--) {
		origData[2 * i] = origData[2 * i + 1] = origData[i];
	}
}

static void snd_adlibDriverSetupChannel(int channelNum, const u8 *data,
										int instrumentNum) {
	s16 vol = snd_sfxState.songData[instrumentNum];
	if (vol != 0 && vol < 0x50)
		vol = 0x50;

	vol -= snd_fadeOutCounter;
	if (vol < 0)
		vol = 0;

	vol += vol / 4;
	if (vol > 0x7F)
		vol = 0x7F;

	snd_adlibChannelVolume[channelNum] = vol;
	snd_adlibDriverSetupInstrument(data, channelNum);
}

static void snd_getAdlibFrequency(int frequency, int *adlibFreq) {
	int i;

	*adlibFreq = 95;
	for (i = 0; i < 96; ++i) {
		if (snd_adlibNoteFreqTable[i] <= frequency) {
			*adlibFreq = i;
			break;
		}
	}
}

static void snd_adlibDriverSetChannelFrequency(int channelNum, int frequency) {
	const u8 *instr = snd_adlibInstrumentsTable[channelNum];
	u8 fl = *instr++;	/* var2 */
	u8 ch = *instr++;	/* var1 */

	if (fl != 0 && ch == 6)
		channelNum = 6;

	if (fl == 0 || channelNum == 6) {
		u16 freqLow, freqHigh;	/* var8 */
		int adlibFreq;

		snd_getAdlibFrequency(frequency, &adlibFreq);
		if (channelNum == 6)
			adlibFreq %= 12;

		freqLow = snd_adlibFreqTable[adlibFreq % 12];
		snd_adlibWriteData(ADLIB_REG_FREQUENCY_0 + channelNum,
		    freqLow);
		freqHigh = ((adlibFreq / 12) << 2) | ((freqLow & 0x300) >> 8);
		if (fl == 0)
			freqHigh |= 0x20;

		snd_adlibWriteData(ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 +
		    channelNum, freqHigh);
	}
	if (fl != 0) {
		snd_adlibVibrato |= 1 << (10 - ch);
		snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_RHYTHM,
		    snd_adlibVibrato);
	}
}

static void snd_adlibDriverStopChannel(int channelNum) {
	const u8 *instr = snd_adlibInstrumentsTable[channelNum];
	u8 fl = *instr++;	/* var2 */
	u8 ch = *instr++;	/* var1 */

	if (fl != 0 && ch == 6)
		channelNum = 6;

	if (fl == 0 || channelNum == 6)
		snd_adlibWriteData(ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 +
		    channelNum, 0);

	if (fl != 0) {
		snd_adlibVibrato &= (1 << (10 - ch)) ^ 0xFF;
		snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_RHYTHM,
		    snd_adlibVibrato);
	}
}

static void snd_adlibDriverPlaySound(u8 * data, int channelNum, int volume) {
/*  if (_snd_mute) return;*/
	u8 fl, ch;		/* var2, var1 */

	assert(channelNum < 4);
	data += 257;
	snd_adlibInstrumentsTable[channelNum] = data;
	snd_resetChannel(channelNum);
	snd_adlibChannelVolume[channelNum] = 0x7F;
	snd_adlibDriverSetupInstrument(data, channelNum);
	fl = *data++;
	ch = *data++;

	if (fl != 0 && ch == 6)
		channelNum = 6;

	if (fl == 0 || channelNum == 6) {
		u16 freqLow, freqHigh;
		freqLow = snd_adlibFreqTable[0];
		snd_adlibWriteData(ADLIB_REG_FREQUENCY_0 + channelNum,
		    freqLow);
		freqHigh = 4 | ((freqLow & 0x300) >> 8);
		if (fl == 0)
			freqHigh |= 0x20;

		snd_adlibWriteData(ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 +
		    channelNum, freqHigh);
	}
	if (fl != 0) {
		snd_adlibVibrato = 1 << (10 - ch);
		snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_RHYTHM,
		    snd_adlibVibrato);
	}
}

static sndDriverStruct snd_adlibDriver = {
	&snd_adlibDriverSetupChannel,
	&snd_adlibDriverSetChannelFrequency,
	&snd_adlibDriverStopChannel,
	&snd_adlibDriverPlaySound
};

void snd_adlibDriverStopSong() {
	int i;

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_KEY_SCALING_OPERATOR_OUTPUT +
		    snd_adlibOpTable[i], 0x3F);

	for (i = 0; i < 9; ++i)
		snd_adlibWriteData(ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 + i, 0);

	snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_RHYTHM, 0);
}

void snd_resetChannel(int channelNum) {
	(*snd_driver.stopChannel) (channelNum);
	if (snd_useAdlib)
		snd_adlibDriverStopSong();
}

AdlibMusic::AdlibMusic(Audio::Mixer *pMixer) {
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	g_cine_adlib = this;
	_opl = makeAdlibOPL(_sampleRate);

	snd_adlibVibrato = 0x20;
	snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_RHYTHM, snd_adlibVibrato);
	snd_adlibWriteData(0x08, 0x40);

	int i;

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_KEY_SCALING_OPERATOR_OUTPUT +
		    snd_adlibOpTable[i], 0);

	for (i = 0; i < 9; ++i)
		snd_adlibWriteData(ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 + i, 0);

	for (i = 0; i < 9; ++i)
		snd_adlibWriteData(ADLIB_REG_FEEDBACK_STRENGTH_CONNECTION_TYPE
		    + i, 0);

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_ATTACK_RATE_DECAY_RATE +
		    snd_adlibOpTable[i], 0);

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_SUSTAIN_LEVEL_RELEASE_RATE_0 +
		    snd_adlibOpTable[i], 0);

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_AM_VIBRATO_EG_KS +
		    snd_adlibOpTable[i], 0);

	for (i = 0; i < 18; ++i)
		snd_adlibWriteData(ADLIB_REG_WAVE_SELECT + snd_adlibOpTable[i],
		    0);

	snd_adlibWriteData(1, 0x20);
	snd_adlibWriteData(1, 0);

	for (i = 0; i < 4; ++i)
		snd_adlibInstrumentsTable[i] = snd_nullInstrument;

	snd_useAdlib = 1;
	snd_driver = snd_adlibDriver;

	_mixer->setupPremix(this);
}

void AdlibMusic::premixerCall(int16 *data, uint len) {
	snd_adlibInterrupt(NULL, data, len);
}

void AdlibMusic::setVolume(uint8 volume) {
	for (int i = 0; i < 4; ++i)
		snd_adlibChannelVolume[i] = volume | 128;
}

AdlibMusic::~AdlibMusic(void) {
	_mixer->setupPremix(NULL);
}
