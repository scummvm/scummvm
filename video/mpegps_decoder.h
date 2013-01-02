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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VIDEO_MPEG_DECODER_H
#define VIDEO_MPEG_DECODER_H

#include "common/hashmap.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

#ifdef USE_MAD
#include <mad.h>
#endif

#ifdef USE_MPEG2

#if defined(__PLAYSTATION2__)
	typedef uint8 uint8_t;
	typedef uint16 uint16_t;
	typedef uint32 uint32_t;
#elif defined(_WIN32_WCE)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
#elif defined(_MSC_VER)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	#if !defined(SDL_COMPILEDVERSION) || (SDL_COMPILEDVERSION < 1210)
	typedef signed long int32_t;
	typedef unsigned long uint32_t;
	#endif
#else
#	include <inttypes.h>
#endif

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

#endif

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class BitStream;
class Huffman;
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

class MPEGPSDecoder : public VideoDecoder {
public:
	MPEGPSDecoder();
	virtual ~MPEGPSDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();
	bool useAudioSync() const { return false; }

private:
	class MPEGStream {
	public:
		virtual ~MPEGStream() {}

		enum StreamType {
			kStreamTypeVideo,
			kStreamTypeAudio
		};

		virtual bool sendPacket(Common::SeekableReadStream *firstPacket, uint32 pts, uint32 dts) = 0;
		virtual StreamType getStreamType() const = 0;
	};

	class MPEGVideoTrack : public VideoTrack, public MPEGStream {
	public:
		MPEGVideoTrack(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format);
		~MPEGVideoTrack();

		bool endOfTrack() const { return _endOfTrack; }
		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime.msecs(); }
		const Graphics::Surface *decodeNextFrame();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeVideo; }

		void setEndOfTrack() { _endOfTrack = true; }

	private:
		bool _endOfTrack;
		int _curFrame;
		Audio::Timestamp _nextFrameStartTime;
		Graphics::Surface *_surface;

		void findDimensions(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format);

#ifdef USE_MPEG2
		enum {
			BUFFER_SIZE = 4096
		};

		byte _buffer[BUFFER_SIZE];
		mpeg2dec_t *_mpegDecoder;
		const mpeg2_info_t *_mpegInfo;
		bool _hasData;
#endif
	};

#ifdef USE_MAD
	class MPEGAudioTrack : public AudioTrack, public MPEGStream {
	public:
		MPEGAudioTrack(Common::SeekableReadStream *firstPacket);
		~MPEGAudioTrack();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeAudio; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audStream;

		enum State {
			MP3_STATE_INIT,  // Need to init the decoder
			MP3_STATE_READY, // ready for processing data
			MP3_STATE_EOS    // end of data reached (may need to loop)
		};

		State _state;

		mad_stream _stream;
		mad_frame _frame;
		mad_synth _synth;

		enum {
			BUFFER_SIZE = 5 * 8192
		};

		// This buffer contains a slab of input data
		byte _buf[BUFFER_SIZE + MAD_BUFFER_GUARD];

		void initStream(Common::SeekableReadStream *packet);
		void deinitStream();
		void readMP3Data(Common::SeekableReadStream *packet);
		void readHeader(Common::SeekableReadStream *packet);
		void decodeMP3Data(Common::SeekableReadStream *packet);
	};
#endif

	class PS2AudioTrack : public AudioTrack, public MPEGStream {
	public:
		PS2AudioTrack(Common::SeekableReadStream *firstPacket);
		~PS2AudioTrack();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeAudio; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audStream;

		enum {
			PS2_PCM = 0x01,
			PS2_ADPCM = 0x10
		};

		uint32 _channels;
		uint32 _soundType;
		uint32 _interleave;
		bool _isFirstPacket;

		byte *_blockBuffer;
		uint32 _blockPos, _blockUsed;

		uint32 calculateSampleCount(uint32 packetSize) const;
	};

	enum PrivateStreamType {
		kPrivateStreamUnknown,
		kPrivateStreamPS2Audio
	};

	bool addFirstVideoTrack();

	int readNextPacketHeader(int32 &startCode, uint32 &pts, uint32 &dts);
	int findNextStartCode(uint32 &size);
	uint32 readPTS(int c);

	void parseProgramStreamMap(int length);
	byte _psmESType[256];

	PrivateStreamType detectPrivateStreamType(Common::SeekableReadStream *packet);

	typedef Common::HashMap<int, MPEGStream *> StreamMap;
	StreamMap _streamMap;

	Common::SeekableReadStream *_stream;
};

} // End of namespace Video

#endif
