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
 * $URL$
 * $Id$
 *
 */

#ifndef GRAPHICS_VIDEO_FlicDecoder_H
#define GRAPHICS_VIDEO_FlicDecoder_H

#include "graphics/video/video_player.h"
#include "common/list.h"
#include "common/rect.h"

namespace Common {
	class SeekableReadStream;
}

namespace Graphics {

/**
 *
 * Decoder for FLIC videos.
 * Video decoder used in engines:
 *  - tucker
 */
class FlicDecoder : public VideoDecoder {
public:
	FlicDecoder();
	virtual ~FlicDecoder();

	/**
	 * Load a FLIC encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *fileName);

	/**
	 * Close a FLIC encoded video file
	 */
	void closeFile();

	/**
	 * Decode the next frame
	 */
	bool decodeNextFrame();

	const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
	void clearDirtyRects() { _dirtyRects.clear(); }
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

	const byte *getPalette() { _paletteChanged = false; return _palette; }
	bool paletteChanged() { return _paletteChanged; }
	void reset();

private:
	uint16 _offsetFrame1;
	uint16 _offsetFrame2;
	byte *_palette;
	bool _paletteChanged;

	void decodeByteRun(uint8 *data);
	void decodeDeltaFLC(uint8 *data);
	void unpackPalette(uint8 *mem);

	Common::List<Common::Rect> _dirtyRects;

};

} // End of namespace Graphics

#endif
