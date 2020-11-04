#ifndef __FMtownsDecoderStream_h__
#define __FMtownsDecoderStream_h__
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

#include "ultima/nuvie/sound/decoder/random_collection_audio_stream.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

class U6Lib_n;
class U6Lzw;
class NuvieIOBuffer;

using Std::string;

class FMtownsDecoderStream : public Audio::RewindableAudioStream {
public:
	FMtownsDecoderStream() {
		should_free_raw_data = false;
		raw_audio_buf = NULL;
	}

	FMtownsDecoderStream(unsigned char *buf, uint32 len);
	FMtownsDecoderStream(Std::string filename, uint16 sample_num, bool isCompressed = true);
	~FMtownsDecoderStream() override;

	uint32 getLengthInMsec();

	int readBuffer(sint16 *buffer, const int numSamples) override;

	/** Is this a stereo stream? */
	bool isStereo() const override {
		return false;
	}

	/** Sample rate of the stream. */
	int getRate() const override {
		return 14700;
	}

	/**
	 * End of data reached? If this returns true, it means that at this
	 * time there is no data available in the stream. However there may be
	 * more data in the future.
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or stop.
	 */
	bool endOfData() const override {
		return (buf_pos >= buf_len);
	}

	bool rewind() override {
		buf_pos = 0;
		return true;
	}

protected:

	bool should_free_raw_data;
	unsigned char *raw_audio_buf;
	uint32 buf_len;
	uint32 buf_pos;

private:
	inline sint16 convertSample(uint16 rawSample) const;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
