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

#include "graphics/video/coktelvideo/coktelvideo.h"

#ifdef GRAPHICS_VIDEO_COKTELVIDEO_H

#ifndef GRAPHICS_VIDEO_VMD_DECODER_H
#define GRAPHICS_VIDEO_VMD_DECODER_H

#include "graphics/video/video_player.h"
#include "sound/mixer.h"

namespace Graphics {

/**
 * Wrapper for the Coktel Vision VMD video decoder
 * for videos by Coktel Vision/Sierra.
 *
 * Video decoder used in engines:
 *  - gob (without this wrapper)
 *  - sci
 */
class VMDDecoder : public VideoDecoder {
public:
	VMDDecoder(Audio::Mixer *mixer);
	virtual ~VMDDecoder();

	uint32 getFrameWaitTime();

	/**
	 * Load a VMD encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *filename);

	/**
	 * Close a VMD encoded video file
	 */
	void closeFile();

	bool decodeNextFrame();

private:
	Vmd *_vmdDecoder;
	Audio::Mixer *_mixer;
	byte _palette[256 * 3];

	void getPalette();
};

} // End of namespace Graphics

#endif

#endif
