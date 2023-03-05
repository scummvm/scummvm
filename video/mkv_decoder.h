/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"	// for USE_VPX

#ifdef USE_VPX

#ifndef VIDEO_MKV_DECODER_H
#define VIDEO_MKV_DECODER_H

#include "common/queue.h"
#include "video/video_decoder.h"
#include "audio/mixer.h"
#include "graphics/surface.h"

#ifdef USE_TREMOR
#include <tremor/ivorbiscodec.h>
#else
#include <vorbis/codec.h>
#endif

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>


namespace Common {
class SeekableReadStream;
}

namespace Audio {
class AudioStream;
class QueuingAudioStream;
}

namespace mkvparser {
class MkvReader;
class Cluster;
class Track;
class Tracks;
class Block;
class BlockEntry;
class Segment;
}

namespace Video {

class MkvReader;

/**
 *
 * Decoder for MKV/Webm videos.
 * Video decoder used in engines:
 *  - sludge
 */
class MKVDecoder : public VideoDecoder {
public:
	MKVDecoder();
	virtual ~MKVDecoder();

	/**
	 * Load a video file
	 * @param stream  the stream to load
	 */
	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();

private:
	class VPXVideoTrack : public VideoTrack {
	public:
		VPXVideoTrack(const Graphics::PixelFormat &format, const mkvparser::Track *const pTrack);
		~VPXVideoTrack();

		bool endOfTrack() const;
		uint16 getWidth() const { return _displaySurface.w; }
		uint16 getHeight() const { return _displaySurface.h; }
		Graphics::PixelFormat getPixelFormat() const { return _displaySurface.format; }
		int getCurFrame() const { return _curFrame; }
		uint32 getNextFrameStartTime() const { return (uint32)(_nextFrameStartTime * 1000); }
		const Graphics::Surface *decodeNextFrame();
		bool decodeFrame(byte *frame, long size);
		void setEndOfVideo() { _endOfVideo = true; }

	private:
		int _curFrame;
		bool _endOfVideo;
		double _nextFrameStartTime;

		Graphics::Surface _surface;
		Graphics::Surface _displaySurface;
		Common::Queue<Graphics::Surface> _displayQueue;

		vpx_codec_ctx_t *_codec = nullptr;
	};

	class VorbisAudioTrack : public AudioTrack {
	public:
		VorbisAudioTrack(const mkvparser::Track *const pTrack);
		~VorbisAudioTrack();

		bool decodeSamples(byte *frame, long size);
		bool hasAudio() const;
		bool needsAudio() const;
		bool synthesizePacket(byte *frame, long size);
		void setEndOfAudio() { _endOfAudio = true; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audStream;

		vorbis_block _vorbisBlock;
		vorbis_dsp_state _vorbisDSP;

		vorbis_info _vorbisInfo;
		bool _endOfAudio;

		ogg_packet oggPacket;
	};

	bool queueAudio(long size);

	Common::SeekableReadStream *_fileStream;

	bool _hasVideo, _hasAudio;

	VPXVideoTrack *_videoTrack = nullptr;
	VorbisAudioTrack *_audioTrack = nullptr;

	mkvparser::MkvReader *_reader = nullptr;

	const mkvparser::Cluster *_cluster = nullptr;
	const mkvparser::Tracks *_pTracks = nullptr;
	const mkvparser::BlockEntry *_pBlockEntry = nullptr;
	mkvparser::Segment *_pSegment = nullptr;

	byte *_frame = nullptr;
	int _frameCounter = 0;

	int _vTrack = -1;
	int _aTrack = -1;

	const mkvparser::Block *_pBlock;
	long long _trackNum;
	int _frameCount;
};

} // End of namespace Video

#endif

#endif
