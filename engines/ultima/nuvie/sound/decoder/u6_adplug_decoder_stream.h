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

#ifndef NUVIE_SOUND_ADPLUG_ADPLUG_DECODER_STREAM_H
#define NUVIE_SOUND_ADPLUG_ADPLUG_DECODER_STREAM_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/adplug/opl.h"
#include "ultima/nuvie/sound/adplug/u6m.h"
#include "ultima/nuvie/sound/adplug/mid.h"
#include "audio/audiostream.h"

namespace Ultima {
namespace Nuvie {

class U6Lib_n;
class U6Lzw;
class NuvieIOBuffer;
using Std::string;

class U6AdPlugDecoderStream : public Audio::RewindableAudioStream {
public:
	U6AdPlugDecoderStream() {
		opl = NULL;
		player = NULL;
		player_refresh_count = 0;
	}

	U6AdPlugDecoderStream(CEmuopl *o, Std::string filename, uint16 song_num);
	~U6AdPlugDecoderStream() override;

	int readBuffer(sint16 *buffer, const int numSamples) override;

	/** Is this a stereo stream? */
	bool isStereo() const override {
		return true;
	}

	/** Sample rate of the stream. */
	int getRate() const override {
		return opl->getRate();
	}

	bool rewind() override {
		if (player) {
			player->rewind();    //FIXME this would need to be locked if called outside mixer thread.
			return true;
		}
		return false;
	}

	/**
	 * End of data reached? If this returns true, it means that at this
	 * time there is no data available in the stream. However there may be
	 * more data in the future.
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or stop.
	 */
	bool endOfData() const override {
		return false;
	}
private:
	void update_opl(short *data, int num_samples);
protected:

	uint16 samples_left;
	CEmuopl *opl;
	CPlayer *player;
	int player_refresh_count;
	int interrupt_rate;
	int interrupt_samples_left;
	bool is_midi_track;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
