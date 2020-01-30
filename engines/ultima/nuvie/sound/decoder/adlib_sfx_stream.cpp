/* Created by Eric Fry
 * Copyright (C) 2011 The Nuvie Team
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/adplug/opl_class.h"
#include "ultima/nuvie/sound/origin_fx_adib_driver.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/sound/decoder/adlib_sfx_stream.h"

namespace Ultima {
namespace Nuvie {

AdLibSfxStream::AdLibSfxStream(Configuration *cfg, int rate, uint8 channel, sint8 note, uint8 velocity, uint8 program_number, uint32 d) {
	interrupt_samples_left = 0;
	opl = new OplClass(rate, true, true); // 16bit stereo
	driver = new OriginFXAdLibDriver(cfg, opl);
	if (program_number != 0xff) {
		driver->program_change(channel, program_number);
	}
	driver->control_mode_change(channel, 0x7, 0x7f);
	driver->play_note(channel, note, velocity);
	duration = d;
	interrupt_rate = (int)(opl->getRate() / 60);
	total_samples_played = 0;
}

AdLibSfxStream::~AdLibSfxStream() {
	delete driver;
	delete opl;
}

int AdLibSfxStream::readBuffer(sint16 *buffer, const int numSamples) {

	//sint32 i, j;
	short *data = (short *)buffer;

	int len = numSamples / 2;
	total_samples_played += numSamples;

	if (interrupt_samples_left > 0) {
		if (interrupt_samples_left > len) {
			opl->update(data, len);
			interrupt_samples_left -= len;
			return numSamples;
		}

		opl->update(data, interrupt_samples_left);
		data += interrupt_samples_left * 2;
		len -= interrupt_samples_left;
		interrupt_samples_left = 0;
	}

	for (int i = len; i > 0;) {
		driver->interrupt_vector();

		int j = interrupt_rate;
		if (j > i) {
			interrupt_samples_left = j - i;
			j = i;
		}
		opl->update(data, j);

		data += j * 2;
		i -= j;
	}


//driver->play_note(8, 0x40, 0);
	return numSamples;
}

} // End of namespace Nuvie
} // End of namespace Ultima
