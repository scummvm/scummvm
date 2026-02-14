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

#include "video/video_decoder.h"

namespace Video {

/**
 * Decoder for 4XM videos.
 *
 * Video decoder used in engines:
 *  - phoenixvr
 */
class FourXMDecoder : public Video::VideoDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	bool useAudioSync() const override { return false; }

private:
	struct Frame {
		int64 offset;
		int64 end;
	};

	class FourXMVideoTrack;
	class FourXMAudioTrack;

	void readList(uint32 size);
	void decodeNextFrameImpl();

	uint32 _dataRate = 0;
	Common::Rational _frameRate;
	Common::SeekableReadStream *_stream;
	Common::Array<Frame> _frames;
	uint _curFrame = 0;
	FourXMVideoTrack *_video = nullptr;
	FourXMAudioTrack *_audio = nullptr;
};

} // namespace Video
