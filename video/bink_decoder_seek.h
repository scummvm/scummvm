/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "video/bink_decoder.h"

#ifdef USE_BINK

#ifndef VIDEO_BINK_DECODER_SEEK_H
#define VIDEO_BINK_DECODER_SEEK_H

namespace Video {

class SeekableBinkDecoder: public Video::BinkDecoder,
		public Video::SeekableVideoDecoder {
public:
	// SeekableVideoDecoder API
	void seekToFrame(uint32 frame);
	void seekToTime(Audio::Timestamp time);
	uint32 getDuration() const;

protected:
	/** Decode a video packet. */
	void videoPacket(VideoFrame &video);
	
	/** Find the keyframe needed to decode a frame */
	uint32 findKeyFrame(uint32 frame) const;
};

} /* namespace Video */

#endif // VIDEO_BINK_DECODER_SEEK_H

#endif // USE_BINK
