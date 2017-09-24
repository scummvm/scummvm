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

/* Based on ffmpeg code (commit 039ebaa5f39ef45444f3cc42ab2ff71b0e9a1161)
 *
 * Interplay MVE File Demuxer
 * Copyright (c) 2003 The FFmpeg project
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "video/mve_decoder.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Video {

MveDecoder::MveDecoder() {
}

MveDecoder::~MveDecoder() {
	close();
}

static const char signature[] = "Interplay MVE File\x1A\0\x1A";

bool MveDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	byte signature_buffer[sizeof(signature)];

	stream->read(signature_buffer, sizeof(signature_buffer));

	while (memcmp(signature_buffer, signature, sizeof(signature))) {
		memmove(signature_buffer, signature_buffer + 1, sizeof(signature_buffer) - 1);
		signature_buffer[sizeof(signature_buffer) - 1] = stream->readByte();
		if (stream->eos()) {
			warning("MveDecoder::loadStream(): attempted to load non-MVE data");

			return false;
		}
	}

#if 0
	/* initialize private context members */
	ipmovie->video_pts = ipmovie->audio_frame_count = 0;
	ipmovie->audio_chunk_offset = ipmovie->video_chunk_offset =
	ipmovie->decode_map_chunk_offset = ipmovie->skip_map_chunk_offset = 0;
	ipmovie->decode_map_chunk_size = ipmovie->video_chunk_size =
	ipmovie->skip_map_chunk_size = 0;
	ipmovie->send_buffer = ipmovie->frame_format = 0;

	/* on the first read, this will position the stream at the first chunk */
	ipmovie->next_chunk_offset = avio_tell(pb) + 4;

	for (i = 0; i < 256; i++)
		ipmovie->palette[i] = 0xFFU << 24;

	/* process the first chunk which should be CHUNK_INIT_VIDEO */
	if (process_ipmovie_chunk(ipmovie, pb, &pkt) != CHUNK_INIT_VIDEO)
		return AVERROR_INVALIDDATA;

	/* peek ahead to the next chunk-- if it is an init audio chunk, process
	* it; if it is the first video chunk, this is a silent file */
	if (avio_read(pb, chunk_preamble, CHUNK_PREAMBLE_SIZE) !=
			CHUNK_PREAMBLE_SIZE)
		return AVERROR(EIO);
	chunk_type = AV_RL16(&chunk_preamble[2]);
	avio_seek(pb, -CHUNK_PREAMBLE_SIZE, SEEK_CUR);

	if (chunk_type == CHUNK_VIDEO)
		ipmovie->audio_type = AV_CODEC_ID_NONE;  /* no audio */
	else if (process_ipmovie_chunk(ipmovie, pb, &pkt) != CHUNK_INIT_AUDIO)
		return AVERROR_INVALIDDATA;

	/* initialize the stream decoders */
	st = avformat_new_stream(s, NULL);
	if (!st)
		return AVERROR(ENOMEM);
	avpriv_set_pts_info(st, 63, 1, 1000000);
	ipmovie->video_stream_index = st->index;
	st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	st->codecpar->codec_id = AV_CODEC_ID_INTERPLAY_VIDEO;
	st->codecpar->codec_tag = 0;  /* no fourcc */
	st->codecpar->width = ipmovie->video_width;
	st->codecpar->height = ipmovie->video_height;
	st->codecpar->bits_per_coded_sample = ipmovie->video_bpp;

	if (ipmovie->audio_type) {
		return init_audio(s);
	} else
		s->ctx_flags |= AVFMTCTX_NOHEADER;
	}

#endif

	return true;
}

const Common::List<Common::Rect> *MveDecoder::getDirtyRects() const {
	return 0;
}

void MveDecoder::clearDirtyRects() {
}

void MveDecoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
}

} // End of namespace Video
