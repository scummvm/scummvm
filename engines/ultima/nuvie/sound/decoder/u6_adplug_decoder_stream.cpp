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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/sound/decoder/u6_adplug_decoder_stream.h"

namespace Ultima {
namespace Nuvie {

U6AdPlugDecoderStream::U6AdPlugDecoderStream(CEmuopl *o, Std::string filename, uint16 song_num) {
	is_midi_track = false;
	opl = o;
	samples_left = 0;
	if (has_file_extension(filename.c_str(), ".lzc")) {
		player = new CmidPlayer(opl);
		((CmidPlayer *)player)->load(filename, song_num);
		is_midi_track = true;
	} else {
		player = new Cu6mPlayer(opl);
		player->load(filename.c_str());
	}
	player_refresh_count = (int)(opl->getRate() / player->getrefresh());

	interrupt_rate = (int)(opl->getRate() / 60);
	interrupt_samples_left = interrupt_rate;
}

U6AdPlugDecoderStream::~U6AdPlugDecoderStream() {
}

int U6AdPlugDecoderStream::readBuffer(sint16 *buffer, const int numSamples) {
	sint32 i, j;
	short *data = (short *)buffer;

	int len = numSamples / 2;

//DEBUG(0, LEVEL_INFORMATIONAL, "Get here. numSamples = %d player refreshrate = %f refresh_count = %d\n", numSamples, player->getrefresh(), (int)(opl->getRate() / player->getrefresh()));

	if (samples_left > 0) {
		if (samples_left > len) {
			update_opl(data, len);
			samples_left -= len;
			return numSamples;
		}

		update_opl(data, samples_left);
		data += samples_left * 2;
		len -= samples_left;
		samples_left = 0;
	}

	for (i = len; i > 0;) {

		if (!player->update()) {
			player->rewind();
			//SoundManager::g_MusicFinished = true;
			DEBUG(0, LEVEL_DEBUGGING, "Music Finished!\n");
		}

		j = (int)(opl->getRate() / player->getrefresh());
		if (j > i) {
			samples_left = j - i;
			j = i;
		}
		update_opl(data, j);

		data += j * 2;
		i -= j;
	}

	return numSamples;
}

void U6AdPlugDecoderStream::update_opl(short *data, int len) {
	if (is_midi_track) {
		if (interrupt_samples_left > 0) {
			if (interrupt_samples_left > len) {
				opl->update(data, len);
				interrupt_samples_left -= len;
				return;
			}

			opl->update(data, interrupt_samples_left);
			data += interrupt_samples_left * 2;
			len -= interrupt_samples_left;
			interrupt_samples_left = 0;
		}

		for (int i = len; i > 0;) {
			((CmidPlayer *)player)->interrupt_vector();

			int j = interrupt_rate;
			if (j > i) {
				interrupt_samples_left = j - i;
				j = i;
			}
			opl->update(data, j);

			data += j * 2;
			i -= j;
		}
	} else {
		opl->update(data, len);
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
