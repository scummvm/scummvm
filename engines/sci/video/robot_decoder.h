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

#ifndef SCI_VIDEO_ROBOT_DECODER_H
#define SCI_VIDEO_ROBOT_DECODER_H

#include "common/rational.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/substream.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"

namespace Sci {

#ifdef ENABLE_SCI32

struct RobotHeader {
	// 6 bytes, identifier bytes
	uint16 version;
	uint16 audioChunkSize;
	uint16 audioSilenceSize;
	// 2 bytes, unknown
	uint16 frameCount;
	uint16 paletteDataSize;
	uint16 unkChunkDataSize;
	// 5 bytes, unknown
	byte hasSound;
	// 34 bytes, unknown
};

class RobotDecoder : public Video::FixedRateVideoDecoder {
public:
	RobotDecoder(Audio::Mixer *mixer, bool isBigEndian);
	virtual ~RobotDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	bool load(GuiResourceId id);
	void close();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	uint32 getFrameCount() const { return _header.frameCount; }
	const Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat::createFormatCLUT8(); }
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }
	void setPos(uint16 x, uint16 y) { _pos = Common::Point(x, y); }
	Common::Point getPos() const { return _pos; }

protected:
	Common::Rational getFrameRate() const { return Common::Rational(60, 10); }

private:
	void readHeaderChunk();
	void readPaletteChunk(uint16 chunkSize);
	void readFrameSizesChunk();
	void calculateVideoDimensions();

	void freeData();

	RobotHeader _header;
	Common::Point _pos;
	bool _isBigEndian;

	Common::SeekableSubReadStreamEndian *_fileStream;

	uint16 _width;
	uint16 _height;
	uint32 *_frameTotalSize;
	byte _palette[256 * 3];
	bool _dirtyPalette;
	Graphics::Surface *_surface;
	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;
	Audio::Mixer *_mixer;
};
#endif

} // End of namespace Sci

#endif
