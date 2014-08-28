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
	_frameCount = _videoData->_stream->readUint16LE();
	_header._width = _videoData->_stream->readUint16LE();
	_header._height = _videoData->_stream->readUint16LE();
	_videoData->_stream->skip(1);
	_header._flags = (VideoFlags)_videoData->_stream->readByte();
	
	_startCoord = (byte *)vidSurface->getBasePtr(pt.x, pt.y);
	_frameCount = _header._frameCount - 2;
	_xCount = _header._width;
	_scanCount = _header._height;
	_vidFrame = 0;

	getFrame();

	if (_header._flags == VIDEOFLAG_BG) {
		// Draw the background
		const byte *pSrc = _vm->_plotBuffer;
		for (int y = 0; y < _scanCount; ++y) {
			byte *pDest = (byte *)vidSurface->getBasePtr(pt.x, pt.y + y);
			Common::copy(pSrc, pSrc + _xCount, pDest);
			pSrc += _xCount;
		}

		if (vidSurface == _vm->_screen)
			_vm->_newRects.push_back(Common::Rect(pt.x, pt.y, pt.x + _xCount, pt.y + _scanCount));
	
	
		getFrame();
	}

	_videoEnd = false;
}

void VideoPlayer::freeVideo() {
	delete _videoData;
	_videoData = nullptr;
}

void VideoPlayer::getFrame() {
	_frameSize = _videoData->_stream->readUint16LE();
	_videoData->_stream->read(_vm->_plotBuffer, _frameSize);
}

void VideoPlayer::playVideo() {
	if (_vm->_timers[31]._flag)
		return;
	++_vm->_timers[31]._flag;


}


} // End of namespace Access
