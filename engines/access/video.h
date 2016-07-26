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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ACCESS_VIDEO_H
#define ACCESS_VIDEO_H

#include "common/scummsys.h"
#include "common/memstream.h"
#include "access/data.h"
#include "access/asurface.h"
#include "access/files.h"

namespace Access {

enum VideoFlags { VIDEOFLAG_NONE = 0, VIDEOFLAG_BG = 1 };

class VideoPlayer : public Manager {
	struct VideoHeader {
		int _frameCount;
		int _width, _height;
		VideoFlags _flags;
	};
private:
	BaseSurface *_vidSurface;
	Resource *_videoData;
	VideoHeader _header;
	byte *_startCoord;
	int _frameCount;
	int _xCount;
	int _scanCount;
	int _frameSize;
	Common::Rect _videoBounds;

	void getFrame();
	void setVideo(BaseSurface *vidSurface, const Common::Point &pt, int rate);
public:
	int _videoFrame;
	bool _soundFlag;
	int _soundFrame;
	bool _videoEnd;
public:
	VideoPlayer(AccessEngine *vm);
	~VideoPlayer();

	/**
	 * Start up a video
	 */
	void setVideo(BaseSurface *vidSurface, const Common::Point &pt, const FileIdent &videoFile, int rate);
	void setVideo(BaseSurface *vidSurface, const Common::Point &pt, const Common::String filename, int rate);

	/**
	 * Decodes a frame of the video
	 */
	void playVideo();

	void copyVideo();
	/**
	 * Frees the data for a previously loaded video
	 */
	void closeVideo();
};

} // End of namespace Access

#endif /* ACCESS_VIDEO_H */
