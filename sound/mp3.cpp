/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "sound/mp3.h"
#include "common/file.h"
#include "common/util.h"

#ifdef USE_MAD
MP3TrackInfo::MP3TrackInfo(File *file) {
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0;

	// Check the format and bitrate
	mad_stream_init(&stream);
	mad_frame_init(&frame);

	while (1) {
		if (buflen < sizeof(buffer)) {
			int bytes;

			bytes = file->read(buffer + buflen, sizeof(buffer) - buflen);
			if (bytes <= 0) {
				if (bytes == -1) {
					warning("Invalid file format");
					goto error;
				}
				break;
			}

			buflen += bytes;
		}

		mad_stream_buffer(&stream, buffer, buflen);

		while (1) {
			if (mad_frame_decode(&frame, &stream) == -1) {
				if (!MAD_RECOVERABLE(stream.error))
					break;

				if (stream.error != MAD_ERROR_BADCRC)
					continue;
			}

			if (count++)
				break;
		}

		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;

		memmove(buffer, stream.next_frame,
		        buflen = &buffer[buflen] - stream.next_frame);
	}

	if (count)
		memcpy(&_mad_header, &frame.header, sizeof(mad_header));
	else {
		warning("Invalid file format");
		goto error;
	}

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	// Get file size
	_size = file->size();
	_file = file;
	_error_flag = false;
	return;

error:
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	_error_flag = true;
	delete file;
}

int MP3TrackInfo::play(SoundMixer *mixer, PlayingSoundHandle *handle, int startFrame, int duration) {
	unsigned int offset;
	mad_timer_t durationTime;

	// Calc offset. As all bitrates are in kilobit per seconds, the division by 200 is always exact
	offset = (startFrame * (_mad_header.bitrate / (8 * 25))) / 3;
	_file->seek(offset, SEEK_SET);

	// Calc delay
	if (!duration) {
		// FIXME: Using _size here is a problem if offset (or equivalently
		// startFrame) is non-zero.
		mad_timer_set(&durationTime, (_size * 8) / _mad_header.bitrate,
					(_size * 8) % _mad_header.bitrate, _mad_header.bitrate);
	} else {
		mad_timer_set(&durationTime, duration / 75, duration % 75, 75);
	}

	// Play it
	return mixer->playMP3CDTrack(handle, _file, durationTime);
}

MP3TrackInfo::~MP3TrackInfo() {
	if (! _error_flag)
		_file->close();
}

#endif
