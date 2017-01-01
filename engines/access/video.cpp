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

#include "access/video.h"
#include "access/access.h"

namespace Access {

VideoPlayer::VideoPlayer(AccessEngine *vm) : Manager(vm) {
	_vidSurface = nullptr;
	_videoData = nullptr;
	_startCoord = nullptr;

	_frameCount = 0;
	_xCount = 0;
	_scanCount = 0;
	_frameSize = 0;
	_videoFrame = 0;
	_soundFlag = false;
	_soundFrame = 0;
	_videoEnd = false;

	_header._frameCount = 0;
	_header._width = _header._height = 0;
	_header._flags = VIDEOFLAG_NONE;
}

VideoPlayer::~VideoPlayer() {
	closeVideo();
}

void VideoPlayer::setVideo(BaseSurface *vidSurface, const Common::Point &pt, int rate) {
	_vidSurface = vidSurface;
	vidSurface->_orgX1 = pt.x;
	vidSurface->_orgY1 = pt.y;
	_vm->_timers[31]._timer = rate;
	_vm->_timers[31]._initTm = rate;

	// Load in header
	_header._frameCount = _videoData->_stream->readUint16LE();
	_header._width = _videoData->_stream->readUint16LE();
	_header._height = _videoData->_stream->readUint16LE();
	_videoData->_stream->skip(1);
	_header._flags = (VideoFlags)_videoData->_stream->readByte();

	_startCoord = (byte *)vidSurface->getBasePtr(pt.x, pt.y);
	_frameCount = _header._frameCount - 2;
	_xCount = _header._width;
	_scanCount = _header._height;
	_videoFrame = 0;
	_videoBounds = Common::Rect(pt.x, pt.y, pt.x + _header._width, pt.y + _header._height);

	getFrame();

	if (_header._flags == VIDEOFLAG_BG) {
		// Draw the background
		for (int y = 0; y < _scanCount; ++y) {
			byte *pDest = (byte *)vidSurface->getBasePtr(pt.x, pt.y + y);
			_videoData->_stream->read(pDest, _xCount);
		}

		if (vidSurface == _vm->_screen)
			_vm->_newRects.push_back(Common::Rect(pt.x, pt.y, pt.x + _xCount, pt.y + _scanCount));

		getFrame();
	}

	_videoEnd = false;
}

void VideoPlayer::setVideo(BaseSurface *vidSurface, const Common::Point &pt, const Common::String filename, int rate) {
	// Open up video stream
	_videoData = _vm->_files->loadFile(filename);

	setVideo(vidSurface, pt, rate);
}

void VideoPlayer::setVideo(BaseSurface *vidSurface, const Common::Point &pt, const FileIdent &videoFile, int rate) {
	// Open up video stream
	_videoData = _vm->_files->loadFile(videoFile);

	setVideo(vidSurface, pt, rate);
}

void VideoPlayer::closeVideo() {
	delete _videoData;
	_videoData = nullptr;
}

void VideoPlayer::getFrame() {
	_frameSize = _videoData->_stream->readUint16LE();
}

void VideoPlayer::playVideo() {
	if (_vm->_timers[31]._flag)
		return;
	++_vm->_timers[31]._flag;

	byte *pDest = _startCoord;
	byte *pLine = _startCoord;
	uint32 frameEnd = _videoData->_stream->pos() + _frameSize;

	while ((uint32)_videoData->_stream->pos() < frameEnd) {
		int count = _videoData->_stream->readByte();

		if (count & 0x80) {
			count &= 0x7f;

			// Skip count number of pixels
			// Loop across lines if necessary
			while (count >= (pLine + _xCount - pDest)) {
				count -= (pLine + _xCount - pDest);
				pLine += _vidSurface->pitch;
				pDest = pLine;
			}

			// Skip any remaining pixels in the new line
			pDest += count;
		} else {
			// Read count number of pixels

			// Load across lines if necessary
			while (count >= (pLine + _xCount - pDest)) {
				int lineCount = (pLine + _xCount - pDest);
				_videoData->_stream->read(pDest, lineCount);
				count -= lineCount;
				pLine += _vidSurface->pitch;
				pDest = pLine;
			}

			// Load remainder of pixels on line
			if (count > 0) {
				_videoData->_stream->read(pDest, count);
				pDest += count;
			}
		}
	}

	// If the video is playing on the screen surface, add a dirty rect
	if (_vidSurface == _vm->_screen)
		_vm->_screen->markAllDirty();

	getFrame();
	if (++_videoFrame == _frameCount) {
		closeVideo();
		_videoEnd = true;
	}
}

void VideoPlayer::copyVideo() {
	_vm->_player->calcPlayer();

	// Figure out the dirty rect area for the video frame
	Common::Rect r = Common::Rect(_vm->_vidX - _vm->_screen->_bufferStart.x,
		_vm->_vidY - _vm->_screen->_bufferStart.y,
		_vm->_vidX - _vm->_screen->_bufferStart.x + _header._width,
		_vm->_vidY - _vm->_screen->_bufferStart.y + _header._height);
	if (!_vm->_screen->clip(r))
		return;
	_vm->_newRects.push_back(r);

	int vh = _header._height;
	int vw = _header._width;
	int destIdx = _vm->_vidX - _vm->_screen->_bufferStart.x;
	int srcIdx = _vm->_screen->_leftSkip;
	for (int i = 0; i < _vm->_screen->_topSkip; i++)
		destIdx += 160;

	const byte *srcP = (const byte *)_vm->_vidBuf.getPixels() + srcIdx;
	byte *destP = (byte *)_vm->_buffer2.getPixels() + destIdx;
	for (int i = 0; i < vh; i++) {
		Common::copy(srcP, srcP + vw, destP);
		srcP += _vm->_vidBuf.pitch;
		destP += _vm->_buffer2.pitch;
	}
}

} // End of namespace Access
