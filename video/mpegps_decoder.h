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

#ifndef VIDEO_MPEGPS_DECODER_H
#define VIDEO_MPEGPS_DECODER_H

#include "common/hashmap.h"
#include "common/queue.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Audio {
class PacketizedAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Image {
class MPEGDecoder;
}

namespace Video {

/**
 * Decoder for MPEG Program Stream videos.
 * Video decoder used in engines:
 *  - mtropolis
 *  - qdengine
 *  - zvision
 */
class MPEGPSDecoder : public VideoDecoder {
public:
	MPEGPSDecoder(double decibel = 0.0);
	virtual ~MPEGPSDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	// Set the number of prebuffered packets in demuxer
	// Used only by qdEngine
	void setPrebufferedPackets(int packets);

protected:
	void readNextPacket();
	bool useAudioSync() const { return false; }

private:
	class MPEGPSDemuxer {
	public:
		MPEGPSDemuxer();
		~MPEGPSDemuxer();

		bool loadStream(Common::SeekableReadStream *stream);
		void close();

		Common::SeekableReadStream *getFirstVideoPacket(int32 &startCode, uint32 &pts, uint32 &dts);
		Common::SeekableReadStream *getNextPacket(uint32 currentTime, int32 &startCode, uint32 &pts, uint32 &dts);

		void setPrebufferedPackets(int packets) { _prebufferedPackets = packets; }

	private:
		class Packet {
		public:
			Packet(Common::SeekableReadStream *stream, int32 startCode, uint32 pts, uint32 dts) : _stream(stream), _startCode(startCode), _pts(pts), _dts(dts) {}

			Common::SeekableReadStream *_stream;
			int32 _startCode;
			uint32 _pts;
			uint32 _dts;
		};
		bool queueNextPacket();
		bool fillQueues();
		int readNextPacketHeader(int32 &startCode, uint32 &pts, uint32 &dts);
		int findNextStartCode(uint32 &size);
		uint32 readPTS(int c);
		void parseProgramStreamMap(int length);

		Common::SeekableReadStream *_stream;
		Common::Queue<Packet> _videoQueue;
		Common::Queue<Packet> _audioQueue;
		// If we come across a non-packetized elementary stream
		bool _isESStream;

		uint32 _firstAudioPacketPts = 0xFFFFFFFF;
		uint32 _firstVideoPacketPts = 0xFFFFFFFF;

		int _prebufferedPackets = 150;
	};

	// Base class for handling MPEG streams
	class MPEGStream {
	public:
		virtual ~MPEGStream() {}

		enum StreamType {
			kStreamTypeVideo,
			kStreamTypeAudio
		};

		virtual bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) = 0;
		virtual StreamType getStreamType() const = 0;
	};

	// An MPEG 1/2 video track
	class MPEGVideoTrack : public VideoTrack, public MPEGStream {
	public:
		MPEGVideoTrack(Common::SeekableReadStream *firstPacket);
		~MPEGVideoTrack();

		bool endOfTrack() const { return _endOfTrack; }
		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		bool setOutputPixelFormat(const Graphics::PixelFormat &format);
		int getCurFrame() const { return _curFrame; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime.msecs(); }
		const Graphics::Surface *decodeNextFrame();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeVideo; }

		void setEndOfTrack() { _endOfTrack = true; }

	private:
		bool _endOfTrack;
		int _curFrame;
		uint32 _framePts;
		Audio::Timestamp _nextFrameStartTime;
		Graphics::Surface *_surface;

		uint16 _width;
		uint16 _height;
		Graphics::PixelFormat _pixelFormat;

		void findDimensions(Common::SeekableReadStream *firstPacket);

#ifdef USE_MPEG2
		Image::MPEGDecoder *_mpegDecoder;
#endif
	};

#ifdef USE_MAD
	// An MPEG audio track
	class MPEGAudioTrack : public AudioTrack, public MPEGStream {
	public:
		MPEGAudioTrack(Common::SeekableReadStream &firstPacket, Audio::Mixer::SoundType soundType);
		~MPEGAudioTrack();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeAudio; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::PacketizedAudioStream *_audStream;
	};
#endif

#ifdef USE_A52
	class AC3AudioTrack : public AudioTrack, public MPEGStream {
	public:
		AC3AudioTrack(Common::SeekableReadStream &firstPacket, double decibel, Audio::Mixer::SoundType soundType);
		~AC3AudioTrack();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeAudio; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::PacketizedAudioStream *_audStream;
	};
#endif

	class PS2AudioTrack : public AudioTrack, public MPEGStream {
	public:
		PS2AudioTrack(Common::SeekableReadStream *firstPacket, Audio::Mixer::SoundType soundType);
		~PS2AudioTrack();

		bool sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts);
		StreamType getStreamType() const { return kStreamTypeAudio; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::PacketizedAudioStream *_audStream;

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

	// The different types of private streams we can detect at the moment
	enum PrivateStreamType {
		kPrivateStreamUnknown,
		kPrivateStreamAC3,
		kPrivateStreamDTS,
		kPrivateStreamDVDPCM,
		kPrivateStreamPS2Audio
	};

	PrivateStreamType detectPrivateStreamType(Common::SeekableReadStream *packet);

	bool addFirstVideoTrack();
	MPEGStream *getStream(uint32 startCode, Common::SeekableReadStream *packet);

	MPEGPSDemuxer *_demuxer;

	// A map from stream types to stream handlers
	typedef Common::HashMap<int, MPEGStream *> StreamMap;
	StreamMap _streamMap;

	double _decibel;
};

} // End of namespace Video

#endif
