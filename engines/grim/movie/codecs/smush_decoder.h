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

#ifndef GRIM_SMUSH_DECODER_H
#define GRIM_SMUSH_DECODER_H

#include "common/stream.h"

#include "video/video_decoder.h"

#include "graphics/surface.h"

namespace Grim {

class Blocky16;
class SmushWave;

class SmushDecoder : public Video::VideoDecoder {
public:
	SmushDecoder();
	~SmushDecoder();

	void setWave(SmushWave *wave) { _wave = wave; }
	int getFrameDelay() const { return _frameDelay; }
	Common::SeekableReadStream *getFile() { return _file; }
	void decodeFrameBlock(const byte *block, uint32 size, int flags);
	const Graphics::Surface *getFrameSurface();
	void resetBlocky16Frames() { _blocky16Frames = 0; }
	int getBlocky16Frames() const { return _blocky16Frames; }
	bool isRewindable() const override { return true; }
	bool isSeekable() const override { return true; }
	bool rewind() override;
	bool seekIntern(const Audio::Timestamp &time) override;
	bool loadStream(Common::SeekableReadStream *stream) override;

protected:
	bool readHeader();
	bool handleFramesHeader();
	void init();
	void close() override;
	class SmushVideoTrack : public FixedRateVideoTrack {
	public:
		SmushVideoTrack(int width, int height, int fps, int numFrames);
		~SmushVideoTrack();

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return _format; }
		int getCurFrame() const override { return _curFrame; }
		void setCurFrame(int frame) { _curFrame = frame; }
		int getFrameCount() const override { return _nbframes; }
		Common::Rational getFrameRate() const override { return _frameRate; }

		void finishFrame();
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &time) override { return true; }

		void handleBlocky16(Common::SeekableReadStream *stream, uint32 size, int flags);
		void init();
		Graphics::Surface *decodeNextFrame() override;

	private:
		int32 _curFrame;
		int _width, _height;
		Graphics::Surface _surface;
		Graphics::PixelFormat _format;
		Common::Rational _frameRate;
		Blocky16 *_blocky16;
		int32 _nbframes;
	};

private:
	SmushVideoTrack *_videoTrack;
	SmushWave *_wave;
	int _blocky16Frames;
	int _frameDelay;

	Common::SeekableReadStream *_file;

	uint32 _startPos;
};

} // end of namespace Grim

#endif
