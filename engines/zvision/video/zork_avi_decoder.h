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
 *
 */

#ifndef ZORK_AVI_DECODER_H
#define ZORK_AVI_DECODER_H

#include "video/avi_decoder.h"
#include "zvision/sound/zork_raw.h"

namespace ZVision {

class ZorkAVIDecoder : public Video::AVIDecoder {
public:
	ZorkAVIDecoder() {}

	~ZorkAVIDecoder() override {}

private:
	class ZorkAVIAudioTrack : public Video::AVIDecoder::AVIAudioTrack {
	public:
		ZorkAVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType);

		void createAudioStream() override;
		void queueSound(Common::SeekableReadStream *stream) override;
		void resetStream() override;

	private:
		Audio::QueuingAudioStream *_queueStream;
		RawChunkStream _decoder;
	};

	Video::AVIDecoder::AVIAudioTrack *createAudioTrack(Video::AVIDecoder::AVIStreamHeader sHeader, Video::AVIDecoder::PCMWaveFormat wvInfo) override;

private:
	// Audio Codecs
	enum {
		kWaveFormatZorkPCM = 17 // special Zork PCM audio format (clashes with MS IMA ADPCM)
	};
};

} // End of namespace ZVision

#endif
