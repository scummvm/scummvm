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

#ifndef ACCESS_VIDEO_H
#define ACCESS_VIDEO_H

#include "common/scummsys.h"
#include "common/memstream.h"
#include "access/data.h"
#include "access/asurface.h"
#include "access/files.h"

namespace Access {

class VideoPlayer: public Manager {
private:
	ASurface *_vidSurface;
	Resource *_videoData;
	int _frameCount;
	int _width, _height;
	int _flags;
public:
	VideoPlayer(AccessEngine *vm);
	~VideoPlayer();

	void setVideo(ASurface *vidSurface, const Common::Point &pt, FileIdent &videoFile, int rate);

	void freeVideo();
};

} // End of namespace Access

#endif /* ACCESS_VIDEO_H */
