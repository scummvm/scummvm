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

#include "access/video.h"
#include "access/access.h"

namespace Access {

VideoPlayer::VideoPlayer(AccessEngine *vm) : Manager(vm) {
	_vidSurface = nullptr;
}

VideoPlayer::~VideoPlayer() {
	freeVideo();
}


void VideoPlayer::setVideo(ASurface *vidSurface, const Common::Point &pt, FileIdent &videoFile, int rate) {
	_vidSurface = vidSurface;
	vidSurface->_orgX1 = pt.x;
	vidSurface->_orgY1 = pt.y;
	_vm->_timers[31]._timer = rate;
	_vm->_timers[31]._initTm = rate;

	// Open up video stream
	_videoData = _vm->_files->loadFile(videoFile);

	// Load in header
	
}

void VideoPlayer::freeVideo() {
	delete _videoData;
	_videoData = nullptr;
}


} // End of namespace Access
