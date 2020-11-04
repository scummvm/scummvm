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
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/sound/decoder/fm_towns_decoder_stream.h"

namespace Ultima {
namespace Nuvie {

inline sint16 convert_sample(uint16 raw_sample);

FMtownsDecoderStream::FMtownsDecoderStream(unsigned char *buf, uint32 len) {
	raw_audio_buf = buf;
	buf_len = len;
	buf_pos = 0;
	should_free_raw_data = false;
}

FMtownsDecoderStream::FMtownsDecoderStream(Std::string filename, uint16 sample_num, bool isCompressed) {
	unsigned char *item_data;
	uint32 decomp_size;
	U6Lib_n sam_file;
	U6Lzw lzw;

	sam_file.open(filename, 4);

	item_data = sam_file.get_item(sample_num, NULL);

	if (isCompressed) {
		raw_audio_buf = lzw.decompress_buffer(item_data, sam_file.get_item_size(sample_num), decomp_size);

		free(item_data);

		buf_len = decomp_size;
	} else {
		raw_audio_buf = item_data;
		buf_len = sam_file.get_item_size(sample_num);
	}

	buf_pos = 0;
	should_free_raw_data = true;
}

FMtownsDecoderStream::~FMtownsDecoderStream() {
	if (raw_audio_buf && should_free_raw_data)
		free(raw_audio_buf);
}

uint32 FMtownsDecoderStream::getLengthInMsec() {
	return (uint32)(buf_len / (getRate() / 1000.0f));
}

int FMtownsDecoderStream::readBuffer(sint16 *buffer, const int numSamples) {
	int j = 0;
	uint32 i = buf_pos;
	//DEBUG(0,LEVEL_INFORMATIONAL, "numSamples = %d. buf_pos = %d, buf_len = %d\n", numSamples, buf_pos, buf_len);

	for (; j < numSamples && i < buf_len;) {
		buffer[j] = convertSample(READ_LE_UINT16(&raw_audio_buf[i]));
		j++;
		i++;
	}

	buf_pos += j;
	//DEBUG(0,LEVEL_INFORMATIONAL, "read %d samples.\n", j);

	return j;
}

inline sint16 FMtownsDecoderStream::convertSample(uint16 rawSample) const {
	sint16 sample;

	if (rawSample & 128)
		sample = ((sint16)(ABS(128 - rawSample) * 256) ^ 0xffff)  + 1;
	else
		sample = rawSample * 256;

	return sample;
}

} // End of namespace Nuvie
} // End of namespace Ultima
