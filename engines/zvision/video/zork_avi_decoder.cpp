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

#include "common/scummsys.h"

#include "zvision/video/zork_avi_decoder.h"

#include "zvision/sound/zork_raw.h"

#include "common/stream.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace ZVision {

Video::AVIDecoder::AVIAudioTrack *ZorkAVIDecoder::createAudioTrack(Video::AVIDecoder::AVIStreamHeader sHeader, Video::AVIDecoder::PCMWaveFormat wvInfo) {
	ZorkAVIDecoder::ZorkAVIAudioTrack *audioTrack = new ZorkAVIDecoder::ZorkAVIAudioTrack(sHeader, wvInfo, _soundType);
	return (Video::AVIDecoder::AVIAudioTrack *)audioTrack;
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::queueSound(Common::SeekableReadStream *stream) {
	bool updateCurChunk = true;
	if (_audStream) {
		if (_wvInfo.tag == kWaveFormatZorkPCM) {
			assert(_wvInfo.size == 8);
			RawChunkStream::RawChunk chunk = decoder->readNextChunk(stream);
			delete stream;

			if (chunk.data) {
				byte flags = Audio::FLAG_16BITS | Audio::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
				// RawChunkStream produces native endianness int16
				flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
				_audStream->queueBuffer((byte *)chunk.data, chunk.size, DisposeAfterUse::YES, flags);
			}
		} else {
			updateCurChunk = false;
			AVIAudioTrack::queueSound(stream);
		}
	} else {
		delete stream;
	}

	// The superclass always updates _curChunk, whether or not audio has
	// been queued, so we should do that too. Unless the superclass already
	// has done it for us.
	if (updateCurChunk) {
		_curChunk++;
	}
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::resetStream() {
	decoder->init();
}

} // End of namespace ZVision
