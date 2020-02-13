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

#ifndef NUVIE_SOUND_ADPLUG_EMU_OPL_H
#define NUVIE_SOUND_ADPLUG_EMU_OPL_H

#include "ultima/nuvie/sound/adplug/opl.h"
#include "audio/fmopl.h"

namespace Ultima {
namespace Nuvie {

class CEmuopl: public Copl {
public:
	CEmuopl(int rate, bool bit16, bool usestereo);  // rate = sample rate
	~CEmuopl() override;

	int getRate() {
		return oplRate;
	}

	void update(short *buf, int samples);   // fill buffer

	// template methods
	void write(int reg, int val) override;
	void init() override;

private:
	bool    use16bit, stereo;
	int oplRate;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
