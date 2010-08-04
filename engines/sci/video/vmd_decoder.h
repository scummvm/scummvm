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

#ifdef ENABLE_SCI32

#ifndef GRAPHICS_VIDEO_VMD_DECODER_H
#define GRAPHICS_VIDEO_VMD_DECODER_H

#include "graphics/video/coktelvideo/coktelvideo.h"
#include "graphics/video/video_decoder.h"
#include "sound/mixer.h"

namespace Sci {

/**
 * Wrapper for the Coktel Vision VMD video decoder
 * for videos by Coktel Vision/Sierra.
 *
 * VMD videos were used in the following SCI21/SCI3
 * adventure games, developed by Sierra:
 * - Gabriel Knight 2: The Beast Within
 * - Leisure Suit Larry 7
 * - Lighthouse
 * - Phantasmagoria 1
 * - RAMA
 * - Shivers
 * - Shivers 2: Harvest of Souls
 * - Torin's Passage
 */
class VMDDecoder : public Graphics::FixedRateVideoDecoder {
public:
	VMDDecoder(Audio::Mixer *mixer);
	virtual ~VMDDecoder();

	uint32 getFrameWaitTime();

	bool load(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _surface->w; }
	uint16 getHeight() const { return _surface->h; }
	uint32 getFrameCount() const { return _vmdDecoder->getFramesCount(); }
	Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat::createFormatCLUT8(); }
	byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

protected:
	Common::Rational getFrameRate() const { return _vmdDecoder->getFrameRate(); }

private:
	Graphics::Vmd *_vmdDecoder;
	Audio::Mixer *_mixer;
	Graphics::Surface *_surface;
	Common::SeekableReadStream *_fileStream;
	byte _palette[256 * 3];
	bool _dirtyPalette;

	void loadPaletteFromVMD();
};

} // End of namespace Graphics

#endif

#endif
