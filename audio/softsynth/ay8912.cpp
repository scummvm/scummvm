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

#include "audio/softsynth/ay8912.h"
#include "common/util.h"

namespace Audio {

bool AY8912Stream::_envGenInit = false;
int AY8912Stream::_envelope[16][128];

/* AY volume table (c) by V_Soft and Lion 17 */
static int Lion17_AY_table[32] = {
	0, 513, 828, 1239, 1923, 3238, 4926, 9110,
	10344, 17876, 24682, 30442, 38844, 47270, 56402, 65535,
	// Duplicate for safety as the C code used 32 size array but init with 16 elements and logic uses /2
	0, 513, 828, 1239, 1923, 3238, 4926, 9110,
	10344, 17876, 24682, 30442, 38844, 47270, 56402, 65535
};

/* default equlaizer (layout) settings for AY, ABC stereo */
static const int default_layout_ay_abc[6] = {
	100, 33, 70, 70, 33, 100
};

AY8912Stream::AY8912Stream(int rate, int chipFreq) : _rate(rate), _chipFreq(chipFreq) {
	if (!_envGenInit)
		genEnv();

	// Reset state
	_bit_a = _bit_b = _bit_c = _bit_n = 0;
	_cnt_a = _cnt_b = _cnt_c = _cnt_n = _cnt_e = 0;
	_envPos = 0;
	_curSeed = 0xffff;

	// Reset registers
	_regs.tone_a = _regs.tone_b = _regs.tone_c = 0;
	_regs.noise = 0;
	_regs.R7_tone_a = _regs.R7_tone_b = _regs.R7_tone_c = 0;
	_regs.R7_noise_a = _regs.R7_noise_b = _regs.R7_noise_c = 0;
	_regs.vol_a = _regs.vol_b = _regs.vol_c = 0;
	_regs.env_a = _regs.env_b = _regs.env_c = 0;
	_regs.env_freq = _regs.env_style = 0;

	// Initialize table and eq
	for (int i = 0; i < 32; i++)
		_table[i] = Lion17_AY_table[i/2]; // AYEMU_AY style

	for (int i = 0; i < 6; i++)
		_eq[i] = default_layout_ay_abc[i]; // AYEMU_ABC style

	prepareGeneration();
}

AY8912Stream::~AY8912Stream() {
}

void AY8912Stream::genEnv() {
	int env;
	int pos;
	int hold;
	int dir;
	int vol;

	for (env = 0; env < 16; env++) {
		hold = 0;
		dir = (env & 4) ? 1 : -1;
		vol = (env & 4) ? -1 : 32;
		for (pos = 0; pos < 128; pos++) {
			if (!hold) {
				vol += dir;
				if (vol < 0 || vol >= 32) {
					if (env & 8) {
						if (env & 2)
							dir = -dir;
						vol = (dir > 0) ? 0 : 31;
						if (env & 1) {
							hold = 1;
							vol = (dir > 0) ? 31 : 0;
						}
					} else {
						vol = 0;
						hold = 1;
					}
				}
			}
			_envelope[env][pos] = vol;
		}
	}
	_envGenInit = true;
}

void AY8912Stream::prepareGeneration() {
	int vol, max_l, max_r;

	_chipTactsPerOutcount = _chipFreq / _rate / 8;

	// GenVols
	for (int n = 0; n < 32; n++) {
		vol = _table[n];
		for (int m = 0; m < 6; m++)
			_vols[m][n] = (int)(((double)vol * _eq[m]) / 100);
	}

	max_l = _vols[0][31] + _vols[2][31] + _vols[3][31];
	max_r = _vols[1][31] + _vols[3][31] + _vols[5][31];
	vol = (max_l > max_r) ? max_l : max_r;
	_ampGlobal = _chipTactsPerOutcount * vol / 24575; // AYEMU_MAX_AMP
}

void AY8912Stream::setRegs(const unsigned char *regs) {
	Common::StackLock lock(_mutex);

	_regs.tone_a = regs[0] + ((regs[1] & 0x0f) << 8);
	_regs.tone_b = regs[2] + ((regs[3] & 0x0f) << 8);
	_regs.tone_c = regs[4] + ((regs[5] & 0x0f) << 8);

	_regs.noise = regs[6] & 0x1f;

	_regs.R7_tone_a = !(regs[7] & 0x01);
	_regs.R7_tone_b = !(regs[7] & 0x02);
	_regs.R7_tone_c = !(regs[7] & 0x04);

	_regs.R7_noise_a = !(regs[7] & 0x08);
	_regs.R7_noise_b = !(regs[7] & 0x10);
	_regs.R7_noise_c = !(regs[7] & 0x20);

	_regs.vol_a = regs[8] & 0x0f;
	_regs.vol_b = regs[9] & 0x0f;
	_regs.vol_c = regs[10] & 0x0f;
	_regs.env_a = regs[8] & 0x10;
	_regs.env_b = regs[9] & 0x10;
	_regs.env_c = regs[10] & 0x10;
	_regs.env_freq = regs[11] + (regs[12] << 8);

	if (regs[13] != 0xff) {
		_regs.env_style = regs[13] & 0x0f;
		_envPos = _cnt_e = 0;
	}
}

void AY8912Stream::setReg(int reg, unsigned char value) {
	Common::StackLock lock(_mutex);

	switch (reg) {
	case 0:
		_regs.tone_a = (_regs.tone_a & 0x0f00) | value;
		break;
	case 1:
		_regs.tone_a = (_regs.tone_a & 0x00ff) | ((value & 0x0f) << 8);
		break;
	case 2:
		_regs.tone_b = (_regs.tone_b & 0x0f00) | value;
		break;
	case 3:
		_regs.tone_b = (_regs.tone_b & 0x00ff) | ((value & 0x0f) << 8);
		break;
	case 4:
		_regs.tone_c = (_regs.tone_c & 0x0f00) | value;
		break;
	case 5:
		_regs.tone_c = (_regs.tone_c & 0x00ff) | ((value & 0x0f) << 8);
		break;
	case 6:
		_regs.noise = value & 0x1f;
		break;
	case 7:
		_regs.R7_tone_a = !(value & 0x01);
		_regs.R7_tone_b = !(value & 0x02);
		_regs.R7_tone_c = !(value & 0x04);

		_regs.R7_noise_a = !(value & 0x08);
		_regs.R7_noise_b = !(value & 0x10);
		_regs.R7_noise_c = !(value & 0x20);
		break;
	case 8:
		_regs.vol_a = value & 0x0f;
		_regs.env_a = value & 0x10;
		break;
	case 9:
		_regs.vol_b = value & 0x0f;
		_regs.env_b = value & 0x10;
		break;
	case 10:
		_regs.vol_c = value & 0x0f;
		_regs.env_c = value & 0x10;
		break;
	case 11:
		_regs.env_freq = (_regs.env_freq & 0xff00) | value;
		break;
	case 12:
		_regs.env_freq = (_regs.env_freq & 0x00ff) | (value << 8);
		break;
	case 13:
		_regs.env_style = value & 0x0f;
		_envPos = _cnt_e = 0;
		break;
	}
}

int AY8912Stream::readBuffer(int16 *buffer, const int numSamples) {
	generateSamples(buffer, numSamples);
	return numSamples;
}

void AY8912Stream::generateSamples(int16 *buffer, int numSamples) {
	Common::StackLock lock(_mutex);

	int mix_l, mix_r;
	int tmpvol;
	int m;
	int frame_count = numSamples / 2; // Stereo samples

	int16 *bufPtr = buffer;

	while (frame_count-- > 0) {
		mix_l = mix_r = 0;

		for (m = 0; m < _chipTactsPerOutcount; m++) {
			if (++_cnt_a >= _regs.tone_a) {
				_cnt_a = 0;
				_bit_a = !_bit_a;
			}
			if (++_cnt_b >= _regs.tone_b) {
				_cnt_b = 0;
				_bit_b = !_bit_b;
			}
			if (++_cnt_c >= _regs.tone_c) {
				_cnt_c = 0;
				_bit_c = !_bit_c;
			}

			if (++_cnt_n >= (_regs.noise * 2)) {
				_cnt_n = 0;
				_curSeed = (_curSeed * 2 + 1) ^ (((_curSeed >> 16) ^ (_curSeed >> 13)) & 1);
				_bit_n = ((_curSeed >> 16) & 1);
			}

			if (++_cnt_e >= _regs.env_freq) {
				_cnt_e = 0;
				if (++_envPos > 127)
					_envPos = 64;
			}

			int envVol = _envelope[_regs.env_style][_envPos];

			if ((_bit_a | !_regs.R7_tone_a) & (_bit_n | !_regs.R7_noise_a)) {
				tmpvol = (_regs.env_a) ? envVol : _regs.vol_a * 2 + 1;
				mix_l += _vols[0][tmpvol];
				mix_r += _vols[1][tmpvol];
			}

			if ((_bit_b | !_regs.R7_tone_b) & (_bit_n | !_regs.R7_noise_b)) {
				tmpvol = (_regs.env_b) ? envVol : _regs.vol_b * 2 + 1;
				mix_l += _vols[2][tmpvol];
				mix_r += _vols[3][tmpvol];
			}

			if ((_bit_c | !_regs.R7_tone_c) & (_bit_n | !_regs.R7_noise_c)) {
				tmpvol = (_regs.env_c) ? envVol : _regs.vol_c * 2 + 1;
				mix_l += _vols[4][tmpvol];
				mix_r += _vols[5][tmpvol];
			}
		}

		if (_ampGlobal > 0) {
			mix_l /= _ampGlobal;
			mix_r /= _ampGlobal;
		}

		*bufPtr++ = mix_l;
		*bufPtr++ = mix_r;
	}
}

} // End of namespace Audio
