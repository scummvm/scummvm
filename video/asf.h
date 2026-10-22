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

#ifndef VIDEO_ASF_H
#define VIDEO_ASF_H

#include "audio/audiostream.h"
#include "audio/decoders/wma.h"
#include "common/array.h"
#include "common/stream.h"

namespace Video {

class ASFDemuxer {
public:
	static const int64 kNoTimestamp = (-0x7fffffffffffffffLL - 1);
	enum StreamType { kAudio, kVideo, kData, kUnknown };
	enum ReadResult { kObject, kEnd, kError };

	struct Stream {
		uint8 id = 0;
		StreamType type = kUnknown;
		uint64 timeOffset = 0;
		uint64 frameTime = 0;
		uint32 aspectNum = 0, aspectDen = 0;
		Common::Array<byte> typeData;
		Common::Array<byte> errorData;
	};

	struct Object {
		uint8 stream = 0;
		uint32 sequence = 0;
		int64 pts = kNoTimestamp, dts = kNoTimestamp;
		int64 position = -1;
		bool keyframe = false;
		Common::Array<byte> data;
		Common::Array<uint32> palette;
	};

	ASFDemuxer();
	~ASFDemuxer();
	bool open(Common::SeekableReadStream *stream);
	void close();
	bool rewind();
	ReadResult readObject(Object &object);
	const Common::Array<Stream> &getStreams() const;
	uint64 getDuration() const;
	uint64 getSendDuration() const;
	uint64 getPreroll() const;
	bool endOfData() const;

private:
	ASFDemuxer(const ASFDemuxer &) = delete;
	ASFDemuxer &operator=(const ASFDemuxer &) = delete;
	struct Context;
	Context *_ctx;
};

Audio::SeekableAudioStream *makeASFAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

class ASFAudioCodec {
public:
	ASFAudioCodec() : _codec(nullptr), _rate(0), _channels(0) {}
	~ASFAudioCodec() { delete _codec; }
	bool open(const ASFDemuxer::Stream &stream);
	Audio::AudioStream *decode(const ASFDemuxer::Object &object);
	int getRate() const { return _rate; }
	bool isStereo() const { return _channels == 2; }
private:
	Audio::WMACodec *_codec;
	uint32 _rate;
	uint16 _channels;
};

} // End of namespace Video

#endif
