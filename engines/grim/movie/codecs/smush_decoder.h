/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_SMUSH_DECODER_H
#define GRIM_SMUSH_DECODER_H

#include "common/rational.h"

#include "audio/mixer.h"

#include "video/video_decoder.h"

#include "graphics/surface.h"

namespace Audio {
	class QueuingAudioStream;
}

namespace Grim {

class Blocky8;
class Blocky16;

class SmushDecoder : public virtual Video::SeekableVideoDecoder, public virtual Video::FixedRateVideoDecoder {
private:
	int32 _nbframes;
	int _width, _height;
	int _x, _y;
	Blocky8 *_blocky8;
	Blocky16 *_blocky16;
	Common::SeekableReadStream *_file;
	Common::Rational _frameRate;
	Graphics::Surface _surface;
	Graphics::PixelFormat _format;

	byte _pal[0x300];
	int16 _deltaPal[0x300];
	byte _IACToutput[4096];
	int32 _IACTpos;

	Audio::SoundHandle _soundHandle;
	Audio::QueuingAudioStream *_stream;

	uint32 _startPos;
	int _channels;
	int _freq;
	bool _videoPause;
	bool _videoLooping;
	bool _demo;

public:
	SmushDecoder();
	~SmushDecoder();

	int getX() { return _x; }
	int getY() { return _y; }
	void setLooping(bool l);
	void setDemo(bool demo) { _demo = demo; }

	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }

	Graphics::PixelFormat getPixelFormat() const { return _surface.format; }
	bool isVideoLoaded() const { return _file != 0; }

	bool loadStream(Common::SeekableReadStream *stream);
	const Graphics::Surface *decodeNextFrame();

	uint32 getFrameCount() const;
	void close();

	// Seekable
	void seekToTime(const Audio::Timestamp &time);
	uint32 getDuration() const;

	uint32 getTimeToNextFrame() const;

private:
	void pauseVideoIntern(bool p);
	void parseNextFrame();
	void init();
	void handleDeltaPalette(byte *src, int32 size);
	void handleFramesHeader();
	void handleFrameDemo();
	void handleFrame();
	void handleBlocky16(byte *src);
	void handleWave(const byte *src, uint32 size);
	void handleIACT(const byte *src, int32 size);
	bool setupAnim();
	bool setupAnimDemo();
	void setMsPerFrame(int ms);
protected:
// Fixed Rate:
	Common::Rational getFrameRate() const {	return _frameRate; }
};

} // end of namespace Grim

#endif
