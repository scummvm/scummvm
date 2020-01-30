/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/adplug/fm_opl.h"

namespace Ultima {
namespace Nuvie {

CEmuopl::CEmuopl(int rate, bool bit16, bool usestereo)
	: use16bit(bit16), stereo(usestereo), oplRate(rate) {
	YM3812Init(1, 3579545, rate);
}

CEmuopl::~CEmuopl() {
	YM3812Shutdown();
}

void CEmuopl::update(short *buf, int samples) {
	int i;

	if (use16bit) {
		YM3812UpdateOne(0, buf, samples);

		if (stereo)
			for (i = samples - 1; i >= 0; i--) {
				buf[i * 2] = buf[i];
				buf[i * 2 + 1] = buf[i];
			}
	} else {
		short *tempbuf = new short[stereo ? samples * 2 : samples];

		YM3812UpdateOne(0, tempbuf, samples);

		if (stereo)
			for (i = samples - 1; i >= 0; i--) {
				tempbuf[i * 2] = tempbuf[i];
				tempbuf[i * 2 + 1] = tempbuf[i];
			}

		for (i = 0; i < (stereo ? samples * 2 : samples); i++)
			((char *)buf)[i] = (tempbuf[i] >> 8) ^ 0x80;

		delete [] tempbuf;
	}
}

void CEmuopl::write(int reg, int val) {
	YM3812Write(0, 0, reg);
	YM3812Write(0, 1, val);
}

void CEmuopl::init() {
	YM3812ResetChip(0);
}

} // End of namespace Nuvie
} // End of namespace Ultima
