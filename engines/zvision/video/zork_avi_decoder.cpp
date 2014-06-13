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
	if (_audStream) {
		if (_wvInfo.tag == kWaveFormatZorkPCM) {
			assert(_wvInfo.size == 8);
			RawChunkStream::RawChunk chunk = decoder->readNextChunk(stream);
			delete stream;

			if (chunk.data)
				_audStream->queueBuffer((byte *)chunk.data, chunk.size, DisposeAfterUse::YES, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO);
		}
	} else {
		warning("Got %d wave format in AVI\n", _wvInfo.tag);
		delete stream;
	}
}

} // End of namespace ZVision
