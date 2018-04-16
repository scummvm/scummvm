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
	if (wvInfo.tag != kWaveFormatZorkPCM)
		return new AVIAudioTrack(sHeader, wvInfo, getSoundType());

	assert(wvInfo.size == 8);
	return new ZorkAVIAudioTrack(sHeader, wvInfo, getSoundType());
}

ZorkAVIDecoder::ZorkAVIAudioTrack::ZorkAVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType) :
		Video::AVIDecoder::AVIAudioTrack(streamHeader, waveFormat, soundType), _queueStream(0), _decoder(waveFormat.channels == 2) {
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::createAudioStream() {
	_queueStream = Audio::makeQueuingAudioStream(_wvInfo.samplesPerSec, _wvInfo.channels == 2);
	_audioStream = _queueStream;
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::queueSound(Common::SeekableReadStream *stream) {
	RawChunkStream::RawChunk chunk = _decoder.readNextChunk(stream);
	delete stream;

	if (chunk.data) {
		byte flags = Audio::FLAG_16BITS;
		if (_wvInfo.channels == 2)
			flags |= Audio::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
		// RawChunkStream produces native endianness int16
		flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
		_queueStream->queueBuffer((byte *)chunk.data, chunk.size, DisposeAfterUse::YES, flags);
	}

	_curChunk++;
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::resetStream() {
	AVIAudioTrack::resetStream();
	_decoder.init();
}

} // End of namespace ZVision
