
/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "base/engine.h"
#include "scumm/player_nes.h"
#include "scumm/scumm.h"
#include "sound/mixer.h"

namespace Scumm {

static const byte channelMask[4] = {1, 2, 4, 8};

static const uint16 freqTable[64] = {
	0x07F0, 0x077E, 0x0712, 0x06AE, 0x064E, 0x05F3, 0x059E, 0x054D,
	0x0501, 0x04B9, 0x0475, 0x0435, 0x03F8, 0x03BF, 0x0389, 0x0357,
	0x0327, 0x02F9, 0x02CF, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A,
	0x01FC, 0x01DF, 0x01C4, 0x01AB, 0x0193, 0x017C, 0x0167, 0x0152,
	0x013F, 0x012D, 0x011C, 0x010C, 0x00FD, 0x00EE, 0x00E1, 0x00D4,
	0x00C8, 0x00BD, 0x00B2, 0x00A8, 0x009F, 0x0096, 0x008D, 0x0085,
	0x007E, 0x0076, 0x0070, 0x0069, 0x0063, 0x005E, 0x0058, 0x0053,
	0x004F, 0x004A, 0x0046, 0x0042, 0x003E, 0x003A, 0x0037, 0x0034
};

static const byte instChannel[16] = {
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 1, 3, 3, 3
};
static const byte startCmd[16] = {
	0x05, 0x03, 0x06, 0x08, 0x0B, 0x01, 0x01, 0x1A,
	0x16, 0x06, 0x04, 0x17, 0x02, 0x10, 0x0E, 0x0D
};
static const byte releaseCmd[16] = {
	0x0F, 0x00, 0x00, 0x09, 0x00, 0x14, 0x15, 0x00,
	0x00, 0x00, 0x1B, 0x1B, 0x0F, 0x0F, 0x0F, 0x0F
};
static const byte nextCmd[28] =	{
	0xFF, 0xFF, 0xFF, 0xFF, 0x17, 0xFF, 0x07, 0xFF,
	0xFF, 0x0A, 0x09, 0x0C, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x12, 0x11, 0x03, 0xFF, 0xFF, 0x18, 0x00,
	0x19, 0x00, 0x00, 0x00
};
static const byte nextDelay[28] = {
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
	0x00, 0x05, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x03, 0x00, 0x00, 0x00
};

namespace APUe {

struct tAPU {
	int	Cycles;
	int	BufPos;
	int	SampleRate;
} APU;

const byte LengthCounts[32] = {
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,

	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E
};

static struct {
	byte volume, envelope, wavehold, duty, swpspeed, swpdir, swpstep, swpenab;
	uint32 freq;	// short
	byte Vol;
	byte CurD;
	byte Timer;
	byte EnvCtr, Envelope, BendCtr;
	bool Enabled, ValidFreq, Active;
	bool EnvClk, SwpClk;
	uint32 Cycles;	// short
	int32 Pos;
} Square0, Square1;

const int8 Duties[4][8] = {
	{-4,+4,-4,-4,-4,-4,-4,-4},
	{-4,+4,+4,-4,-4,-4,-4,-4},
	{-4,+4,+4,+4,+4,-4,-4,-4},
	{+4,-4,-4,+4,+4,+4,+4,+4}
};

inline void Square0_CheckActive(void) {
	Square0.ValidFreq = (Square0.freq >= 0x8) && ((Square0.swpdir) || !((Square0.freq + (Square0.freq >> Square0.swpstep)) & 0x800));
	Square0.Active = Square0.Timer && Square0.ValidFreq;
	Square0.Pos = Square0.Active ? (Duties[Square0.duty][Square0.CurD] * Square0.Vol) : 0;
}

inline void Square0_Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		Square0.volume = Val & 0xF;
		Square0.envelope = Val & 0x10;
		Square0.wavehold = Val & 0x20;
		Square0.duty = (Val >> 6) & 0x3;
		Square0.Vol = Square0.envelope ? Square0.volume : Square0.Envelope;
		break;

	case 1:
		Square0.swpstep = Val & 0x07;
		Square0.swpdir = Val & 0x08;
		Square0.swpspeed = (Val >> 4) & 0x7;
		Square0.swpenab = Val & 0x80;
		Square0.SwpClk = true;
		break;

	case 2:
		Square0.freq &= 0x700;
		Square0.freq |= Val;
		break;

	case 3:
		Square0.freq &= 0xFF;
		Square0.freq |= (Val & 0x7) << 8;

		if (Square0.Enabled)
			Square0.Timer = LengthCounts[(Val >> 3) & 0x1F];

		Square0.CurD = 0;
		Square0.EnvClk = true;
		break;

	case 4:
		if (!(Square0.Enabled = Val ? true : false))
			Square0.Timer = 0;
		break;
	}
	Square0_CheckActive();
}

inline void Square0_Run(void) {
	if (!--Square0.Cycles) {
		Square0.Cycles = (Square0.freq + 1) << 1;
		Square0.CurD = (Square0.CurD + 1) & 0x7;

		if (Square0.Active)
			Square0.Pos = Duties[Square0.duty][Square0.CurD] * Square0.Vol;
	}
}

inline void Square0_QuarterFrame(void) {
	if (Square0.EnvClk) {
		Square0.EnvClk = false;
		Square0.Envelope = 0xF;
		Square0.EnvCtr = Square0.volume + 1;
	} else if (!--Square0.EnvCtr) {
		Square0.EnvCtr = Square0.volume + 1;

		if (Square0.Envelope)
			Square0.Envelope--;
		else
			Square0.Envelope = Square0.wavehold ? 0xF : 0x0;
	}

	Square0.Vol = Square0.envelope ? Square0.volume : Square0.Envelope;
	Square0_CheckActive();
}

inline void Square0_HalfFrame(void) {
	if (!--Square0.BendCtr) {
		Square0.BendCtr = Square0.swpspeed + 1;

		if (Square0.swpenab && Square0.swpstep && Square0.ValidFreq) {
			int sweep = Square0.freq >> Square0.swpstep;
			Square0.freq += Square0.swpdir ? ~sweep : sweep;
		}
	}

	if (Square0.SwpClk) {
		Square0.SwpClk = false;
		Square0.BendCtr = Square0.swpspeed + 1;
	}

	if (Square0.Timer && !Square0.wavehold)
		Square0.Timer--;

	Square0_CheckActive();
}

inline void Square1_CheckActive(void) {
	Square1.ValidFreq = (Square1.freq >= 0x8) && ((Square1.swpdir) || !((Square1.freq + (Square1.freq >> Square1.swpstep)) & 0x800));
	Square1.Active = Square1.Timer && Square1.ValidFreq;
	Square1.Pos = Square1.Active ? (Duties[Square1.duty][Square1.CurD] * Square1.Vol) : 0;
}

inline void Square1_Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		Square1.volume = Val & 0xF;
		Square1.envelope = Val & 0x10;
		Square1.wavehold = Val & 0x20;
		Square1.duty = (Val >> 6) & 0x3;
		Square1.Vol = Square1.envelope ? Square1.volume : Square1.Envelope;
		break;

	case 1:
		Square1.swpstep = Val & 0x07;
		Square1.swpdir = Val & 0x08;
		Square1.swpspeed = (Val >> 4) & 0x7;
		Square1.swpenab = Val & 0x80;
		Square1.SwpClk = true;
		break;

	case 2:
		Square1.freq &= 0x700;
		Square1.freq |= Val;
		break;

	case 3:
		Square1.freq &= 0xFF;
		Square1.freq |= (Val & 0x7) << 8;

		if (Square1.Enabled)
			Square1.Timer = LengthCounts[(Val >> 3) & 0x1F];

		Square1.CurD = 0;
		Square1.EnvClk = true;
		break;

	case 4:
		if (!(Square1.Enabled = Val ? true : false))
			Square1.Timer = 0;
		break;
	}
	Square1_CheckActive();
}

inline void Square1_Run(void) {
	if (!--Square1.Cycles) {
		Square1.Cycles = (Square1.freq + 1) << 1;
		Square1.CurD = (Square1.CurD + 1) & 0x7;

		if (Square1.Active)
			Square1.Pos = Duties[Square1.duty][Square1.CurD] * Square1.Vol;
	}
}

inline void Square1_QuarterFrame(void) {
	if (Square1.EnvClk) {
		Square1.EnvClk = false;
		Square1.Envelope = 0xF;
		Square1.EnvCtr = Square1.volume + 1;
	} else if (!--Square1.EnvCtr) {
		Square1.EnvCtr = Square1.volume + 1;

		if (Square1.Envelope)
			Square1.Envelope--;
		else
			Square1.Envelope = Square1.wavehold ? 0xF : 0x0;
	}

	Square1.Vol = Square1.envelope ? Square1.volume : Square1.Envelope;
	Square1_CheckActive();
}

inline void Square1_HalfFrame(void) {
	if (!--Square1.BendCtr) {
		Square1.BendCtr = Square1.swpspeed + 1;

		if (Square1.swpenab && Square1.swpstep && Square1.ValidFreq) {
			int sweep = Square1.freq >> Square1.swpstep;
			Square1.freq += Square1.swpdir ? -sweep : sweep;
		}
	}

	if (Square1.SwpClk) {
		Square1.SwpClk = false;
		Square1.BendCtr = Square1.swpspeed + 1;
	}

	if (Square1.Timer && !Square1.wavehold)
		Square1.Timer--;

	Square1_CheckActive();
}

static struct {
	byte linear, wavehold;
	uint32 freq;	// short
	byte CurD;
	byte Timer, LinCtr;
	bool Enabled, Active;
	bool LinClk;
	uint32 Cycles;	// short
	int32 Pos;
} Triangle;

const int8 TriDuty[32] = {
	-8,-7,-6,-5,-4,-3,-2,-1,
	+0,+1,+2,+3,+4,+5,+6,+7,
	+7,+6,+5,+4,+3,+2,+1,+0,
	-1,-2,-3,-4,-5,-6,-7,-8
};

inline void Triangle_CheckActive(void) {
	Triangle.Active = Triangle.Timer && Triangle.LinCtr;

	if (Triangle.freq < 4)
		Triangle.Pos = 0;	// beyond hearing range
	else
		Triangle.Pos = TriDuty[Triangle.CurD] * 8;
}

inline void Triangle_Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		Triangle.linear = Val & 0x7F;
		Triangle.wavehold = (Val >> 7) & 0x1;
		break;

	case 2:
		Triangle.freq &= 0x700;
		Triangle.freq |= Val;
		break;

	case 3:
		Triangle.freq &= 0xFF;
		Triangle.freq |= (Val & 0x7) << 8;

		if (Triangle.Enabled)
			Triangle.Timer = LengthCounts[(Val >> 3) & 0x1F];

		Triangle.LinClk = true;
		break;

	case 4:
		if (!(Triangle.Enabled = Val ? true : false))
			Triangle.Timer = 0;
		break;
	}
	Triangle_CheckActive();
}

inline void Triangle_Run(void) {
	if (!--Triangle.Cycles) {
		Triangle.Cycles = Triangle.freq + 1;

		if (Triangle.Active) {
			Triangle.CurD++;
			Triangle.CurD &= 0x1F;

			if (Triangle.freq < 4)
				Triangle.Pos = 0;	// beyond hearing range
			else
				Triangle.Pos = TriDuty[Triangle.CurD] * 8;
		}
	}
}

inline void Triangle_QuarterFrame(void) {
	if (Triangle.LinClk)
		Triangle.LinCtr = Triangle.linear;
	else if (Triangle.LinCtr)
		Triangle.LinCtr--;

	if (!Triangle.wavehold)
		Triangle.LinClk = false;

	Triangle_CheckActive();
}

inline void Triangle_HalfFrame(void) {
	if (Triangle.Timer && !Triangle.wavehold)
		Triangle.Timer--;

	Triangle_CheckActive();
}

static struct {
	byte volume, envelope, wavehold, datatype;
	uint32 freq;	// short
	uint32 CurD;	// short
	byte Vol;
	byte Timer;
	byte EnvCtr, Envelope;
	bool Enabled;
	bool EnvClk;
	uint32 Cycles;	// short
	int32 Pos;
}	Noise;

const uint32 NoiseFreq[16] = {
	0x004,0x008,0x010,0x020,0x040,0x060,0x080,0x0A0,
	0x0CA,0x0FE,0x17C,0x1FC,0x2FA,0x3F8,0x7F2,0xFE4
};

inline void Noise_Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		Noise.volume = Val & 0x0F;
		Noise.envelope = Val & 0x10;
		Noise.wavehold = Val & 0x20;
		Noise.Vol = Noise.envelope ? Noise.volume : Noise.Envelope;

		if (Noise.Timer)
			Noise.Pos = ((Noise.CurD & 0x4000) ? -2 : 2) * Noise.Vol;
		break;

	case 2:
		Noise.freq = Val & 0xF;
		Noise.datatype = Val & 0x80;
		break;

	case 3:
		if (Noise.Enabled)
			Noise.Timer = LengthCounts[(Val >> 3) & 0x1F];

		Noise.EnvClk = true;
		break;

	case 4:
		if (!(Noise.Enabled = Val ? true : false))
			Noise.Timer = 0;
		break;
	}
}

inline void Noise_Run(void) {
	if (!--Noise.Cycles) {
		Noise.Cycles = NoiseFreq[Noise.freq];	/* no + 1 here */

		if (Noise.datatype)
			Noise.CurD = (Noise.CurD << 1) | (((Noise.CurD >> 14) ^ (Noise.CurD >> 8)) & 0x1);
		else
			Noise.CurD = (Noise.CurD << 1) | (((Noise.CurD >> 14) ^ (Noise.CurD >> 13)) & 0x1);

		if (Noise.Timer)
			Noise.Pos = ((Noise.CurD & 0x4000) ? -2 : 2) * Noise.Vol;
	}
}

inline void Noise_QuarterFrame(void) {
	if (Noise.EnvClk) {
		Noise.EnvClk = false;
		Noise.Envelope = 0xF;
		Noise.EnvCtr = Noise.volume + 1;
	} else if (!--Noise.EnvCtr) {
		Noise.EnvCtr = Noise.volume + 1;

		if (Noise.Envelope)
			Noise.Envelope--;
		else
			Noise.Envelope = Noise.wavehold ? 0xF : 0x0;
	}

	Noise.Vol = Noise.envelope ? Noise.volume : Noise.Envelope;

	if (Noise.Timer)
		Noise.Pos = ((Noise.CurD & 0x4000) ? -2 : 2) * Noise.Vol;
}

inline void Noise_HalfFrame(void) {
	if (Noise.Timer && !Noise.wavehold)
		Noise.Timer--;
}

static struct {
	uint32 Cycles;
	int Num;
} Frame;

inline void Frame_Run(void) {
	if (!--Frame.Cycles) {
		Frame.Cycles = 7457;

		if (Frame.Num < 4) {
			Square0_QuarterFrame();
			Square1_QuarterFrame();
			Triangle_QuarterFrame();
			Noise_QuarterFrame();

			if (!(Frame.Num & 1)) {
				Square0_HalfFrame();
				Square1_HalfFrame();
				Triangle_HalfFrame();
				Noise_HalfFrame();
			}
		}

		if (Frame.Num & 1)
			Frame.Cycles++;

		Frame.Num++;

		if (Frame.Num == 5)
			Frame.Num = 0;
	}
}

void APU_WriteReg(int Addr, byte Val) {
	switch (Addr) {
	case 0x000:	Square0_Write(0,Val);	break;
	case 0x001:	Square0_Write(1,Val);	break;
	case 0x002:	Square0_Write(2,Val);	break;
	case 0x003:	Square0_Write(3,Val);	break;
	case 0x004:	Square1_Write(0,Val);	break;
	case 0x005:	Square1_Write(1,Val);	break;
	case 0x006:	Square1_Write(2,Val);	break;
	case 0x007:	Square1_Write(3,Val);	break;
	case 0x008:	Triangle_Write(0,Val);	break;
	case 0x009:	Triangle_Write(1,Val);	break;
	case 0x00A:	Triangle_Write(2,Val);	break;
	case 0x00B:	Triangle_Write(3,Val);	break;
	case 0x00C:	Noise_Write(0,Val);	break;
	case 0x00D:	Noise_Write(1,Val);	break;
	case 0x00E:	Noise_Write(2,Val);	break;
	case 0x00F:	Noise_Write(3,Val);	break;
	case 0x015:	Square0_Write(4,Val & 0x1);
			Square1_Write(4,Val & 0x2);
			Triangle_Write(4,Val & 0x4);
			Noise_Write(4,Val & 0x8);
						break;
	}
}

byte APU_Read4015(void) {
	byte result =
		(( Square0.Timer) ? 0x01 : 0) |
		(( Square1.Timer) ? 0x02 : 0) |
		((Triangle.Timer) ? 0x04 : 0) |
		((   Noise.Timer) ? 0x08 : 0);
	return result;
}

void APU_Reset (void) {
	APU.BufPos = 0;

	memset(&Frame, 0, sizeof(Frame));
	memset(&Square0, 0, sizeof(Square0));
	memset(&Square1, 0, sizeof(Square1));
	memset(&Triangle, 0, sizeof(Triangle));
	memset(&Noise, 0, sizeof(Noise));

	Noise.CurD = 1;
	APU.Cycles = 1;
	Square0.Cycles = 1;
	Square0.EnvCtr = 1;
	Square0.BendCtr = 1;
	Square1.Cycles = 1;
	Square1.EnvCtr = 1;
	Square1.BendCtr = 1;
	Triangle.Cycles = 1;
	Noise.Cycles = 1;
	Noise.EnvCtr = 1;
	Frame.Cycles = 1;
}

int16 APU_GetSample(void) {
	int sampcycles = 0, samppos = 0;
	int NewBufPos = APU.BufPos;
	while (NewBufPos == APU.BufPos) {
		NewBufPos = APU.SampleRate * ++APU.Cycles / 1789773;
		if (APU.Cycles == 1789773) // we've generated 1 second, so we can reset our counters now
			APU.Cycles = NewBufPos = 0;

		Frame_Run();
		Square0_Run();
		Square1_Run();
		Triangle_Run();
		Noise_Run();

		samppos += Square0.Pos + Square1.Pos + Triangle.Pos + Noise.Pos;
		sampcycles++;
	}

	APU.BufPos = NewBufPos;

	return (samppos << 6) / sampcycles;
}

}

Player_NES::Player_NES(ScummEngine *scumm) {
	int i;
	_vm = scumm;
	_mixer = scumm->_mixer;
	APUe::APU.SampleRate = _sample_rate = _mixer->getOutputRate();

	_samples_per_frame = _sample_rate / 60;
	_current_sample = 0;

	for (i = 0; i < NUMSLOTS; i++) {
		_slot[i].id = -1;
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].offset = 0;
		_slot[i].data = NULL;
	}

	for (i = 0; i < NUMCHANS; i++) {
		_mchan[i].command = 0;
		_mchan[i].framedelay = 0;
		_mchan[i].pitch = 0;
		_mchan[i].volume = 0;
		_mchan[i].voldelta = 0;
		_mchan[i].envflags = 0;
		_mchan[i].cmdlock = 0;
	}
	isSFXplaying = wasSFXplaying = false;

	auxData1 = auxData2 = NULL;
	numNotes = 0;

	APU_writeControl(0);

	APUe::APU_Reset();

	_mixer->setupPremix(this);
}

Player_NES::~Player_NES() {
	_mixer->setupPremix(0);
}

void Player_NES::setMusicVolume (int vol) {
	_maxvol = vol;
}

int Player_NES::readBuffer(int16 *buffer, const int numSamples) {
	for (int n = 0; n < numSamples; n++) {
		buffer[n] = APUe::APU_GetSample() * _maxvol / 255;
		_current_sample++;

		if (_current_sample == _samples_per_frame) {
			_current_sample = 0;
			sound_play();
		}
	}
	return numSamples;
}
void Player_NES::stopAllSounds() {
	for (int i = 0; i < NUMSLOTS; i++) {
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].id = -1;
	}

	isSFXplaying = 0;
	checkSilenceChannels(0);
}

void Player_NES::stopSound(int nr) {
	if (nr == -1)
		return;

	for (int i = 0; i < NUMSLOTS; i++) {
		if (_slot[i].id != nr)
			continue;

		isSFXplaying = 0;
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].id = -1;
		checkSilenceChannels(i);
	}
}

void Player_NES::startSound(int nr) {
	byte *data = _vm->getResourceAddress(rtSound, nr) + 2;
	assert(data);

	int soundType = data[1];
	int chan = data[0];

	if (chan == 4) {
		if (_slot[2].framesleft)
			return;
		chan = 0;
	}

	if (soundType < _slot[chan].type)
		return;

	_slot[chan].type = soundType;
	_slot[chan].id = nr;
	_slot[chan].data = data;
	_slot[chan].offset = 2;
	_slot[chan].framesleft = 1;
	checkSilenceChannels(chan);
	if (chan == 2) {
		numNotes = _slot[chan].data[2];
		auxData1 = _slot[chan].data + 3;
		auxData2 = auxData1 + numNotes;
		_slot[chan].data = auxData2 + numNotes;
		_slot[chan].offset = 0;

		for (int i = 0; i < NUMCHANS; i++)
			_mchan[i].cmdlock = 0;
	}
}

void Player_NES::checkSilenceChannels(int chan) {
	for (chan--; chan >= 0; chan--) {
		if (_slot[chan].framesleft)
			return;
	}
	APU_writeControl(0);
}

void Player_NES::sound_play() {
	if (_slot[0].framesleft)
		playSFX(0);
	else if (_slot[1].framesleft)
		playSFX(1);

	playMusic();
}

void Player_NES::playSFX (int nr) {
	if (--_slot[nr].framesleft)
		return;

	while (1) {
		int a = _slot[nr].data[_slot[nr].offset++];
		if (a < 16) {
			a >>= 2;
			APU_writeControl(APU_readStatus() | channelMask[a]);
			isSFXplaying = true;
			APU_writeChannel(a, 0, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 1, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 2, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 3, _slot[nr].data[_slot[nr].offset++]);
		} else if (a == 0xFE) {
			_slot[nr].offset = 2;
		} else if (a == 0xFF) {
			_slot[nr].id = -1;
			_slot[nr].type = 0;
			isSFXplaying = false;
			APU_writeControl(0);

			if (!nr && _slot[1].framesleft) {
				_slot[1].framesleft = 1;
				isSFXplaying = true;
			}
			return;
		} else {
			_slot[nr].framesleft = _slot[nr].data[_slot[nr].offset++];
			return;
		}
	}
}

void Player_NES::playMusic() {
	if (!_slot[2].framesleft)
		return;

	if (wasSFXplaying && !isSFXplaying)
		for (int x = 1; x >= 0; x--)
			if (_mchan[x].cmdlock) {
				_mchan[x].command = _mchan[x].cmdlock;
				_mchan[x].framedelay = 1;
			}

	wasSFXplaying = isSFXplaying;
	if (!--_slot[2].framesleft) {
top:
		int b = _slot[2].data[_slot[2].offset++];
		if (b == 0xFF) {
			_slot[2].id = -1;
			_slot[2].type = 0;
			b = 0;
		} else if (b == 0xFE) {
			_slot[2].offset = 0;
			goto top;
		} else {
			if (b < numNotes) {
				int inst = auxData1[b];
				int ch = instChannel[inst];
				_mchan[ch].pitch = auxData2[b];
				_mchan[ch].cmdlock = startCmd[inst];
				_mchan[ch].command = startCmd[inst];
				_mchan[ch].framedelay = 1;
				goto top;
			}
			b -= numNotes;
			if (b < 16) {
				int inst = b;
				int ch = instChannel[inst];
				_mchan[ch].cmdlock = 0;
				_mchan[ch].command = releaseCmd[inst];
				_mchan[ch].framedelay = 1;
				goto top;
			}
			b -= 16;
		}
		_slot[2].framesleft = b;
	}

	for (int x = NUMCHANS - 1; x >= 0; x--) {
		if (_slot[0].framesleft || _slot[1].framesleft) {
			_mchan[x].volume = 0;
			_mchan[x].framedelay = 0;
			continue;
		}

		if (_mchan[x].framedelay && !--_mchan[x].framedelay) {
			switch (_mchan[x].command) {
			case 0x00:
			case 0x13:
				_mchan[x].voldelta = -10;
				break;

			case 0x01:
			case 0x03:
			case 0x08:
			case 0x16:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x02:
				_mchan[x].envflags = 0xB0;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x84);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x04:
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x05:
				_mchan[x].envflags = 0xF0;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = -15;

				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x06:
				_mchan[x].pitch += 0x18;
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x07:
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch - 0x0C] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch - 0x0C] >> 8);

				chainCommand(x);
				break;

			case 0x09:
				_mchan[x].voldelta = -2;

				APU_writeChannel(x, 1, 0x7F);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0A:
				APU_writeChannel(x, 1, 0x86);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0B:	case 0x1A:
				_mchan[x].envflags = 0x70;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0C:
				_mchan[x].envflags = 0xB0;

				chainCommand(x);
				break;

			case 0x0D:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x5F;
				_mchan[x].voldelta = -22;

				APU_writeChannel(x, 0, 0x00);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x0E:
			case 0x10:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x5F;
				_mchan[x].voldelta = -6;

				APU_writeChannel(x, 0, 0x00);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x0F:
				chainCommand(x);
				break;

			case 0x11:
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x12:
				APU_writeChannel(x, 2, (_mchan[x].pitch + 3) & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x14:
				_mchan[x].voldelta = -12;

				APU_writeChannel(x, 1, 0x8C);

				chainCommand(x);
				break;

			case 0x15:
				_mchan[x].voldelta = -12;

				APU_writeChannel(x, 1, 0x84);

				chainCommand(x);
				break;

			case 0x17:
				_mchan[x].pitch += 0x0C;
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x18:
				_mchan[x].envflags = 0x70;

				chainCommand(x);
				break;

			case 0x19:
				_mchan[x].envflags = 0xB0;

				chainCommand(x);
				break;

			case 0x1B:
				_mchan[x].envflags = 0x00;
				_mchan[x].voldelta = -10;
				break;
			}
		}

		_mchan[x].volume += _mchan[x].voldelta;

		if (_mchan[x].volume < 0)
			_mchan[x].volume = 0;
		if (_mchan[x].volume > MAXVOLUME)
			_mchan[x].volume = MAXVOLUME;

		APU_writeChannel(x, 0, (_mchan[x].volume >> 3) | _mchan[x].envflags);
	}
}

void Player_NES::chainCommand(int c) {
	int i = _mchan[c].command;
	_mchan[c].command = nextCmd[i];
	_mchan[c].framedelay = nextDelay[i];
}

int Player_NES::getSoundStatus(int nr) const {
	for (int i = 0; i < NUMSLOTS; i++)
		if (_slot[i].id == nr)
			return 1;
	return 0;
}

void Player_NES::APU_writeChannel(int chan, int offset, byte value) {
	APUe::APU_WriteReg(0x000 + 4 * chan + offset, value);
}
void Player_NES::APU_writeControl(byte value) {
	APUe::APU_WriteReg(0x015, value);
}
byte Player_NES::APU_readStatus() {
	return APUe::APU_Read4015();
}

} // End of namespace Scumm
