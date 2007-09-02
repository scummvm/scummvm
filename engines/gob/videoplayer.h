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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_VIDEOPLAYER_H
#define GOB_VIDEOPLAYER_H

#include "gob/coktelvideo.h"
#include "gob/dataio.h"

namespace Gob {

class GobEngine;

class VideoPlayer {
public:
	enum Flags {
		kFlagNone = 0,
		kFlagFrontSurface = 0x80,
		kFlagNoVideo = 0x100
	};

	enum Type {
		kVideoTypeTry = -1,
		kVideoTypeIMD = 0,
		kVideoTypeVMD = 1
	};

	VideoPlayer(GobEngine *vm);
	~VideoPlayer();

	bool openVideo(const char *video, int16 x = -1, int16 y = -1,
			int16 flags = kFlagFrontSurface, Type which = kVideoTypeTry);

	void play(int16 startFrame = -1, int16 lastFrame = -1, int16 breakKey = 27,
			uint16 palCmd = 8, int16 palStart = 0, int16 palEnd = 255,
			int16 palFrame = -1, int16 endFrame = -1, bool fade = false,
			int16 reverseTo = -1);

	int16 getFramesCount() const;
	int16 getCurrentFrame() const;
	void writeVideoInfo(const char *video, int16 varX, int16 varY,
			int16 varFrames, int16 varWidth, int16 varHeight);

	void closeVideo();

private:
	static const char *_extensions[];

	GobEngine *_vm;

	char _curFile[256];
	DataStream *_stream;
	CoktelVideo *_video;
	bool _backSurf;

	void copyPalette(int16 palStart = -1, int16 palEnd = -1);
	bool doPlay(int16 frame, int16 breakKey,
			uint16 palCmd, int16 palStart, int16 palEnd,
			int16 palFrame, int16 endFrame);
};

} // End of namespace Gob

#endif // GOB_VIDEOPLAYER_H
