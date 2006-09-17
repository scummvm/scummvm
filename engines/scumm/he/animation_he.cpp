/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"

namespace Scumm {

MoviePlayer::MoviePlayer(ScummEngine_v90he *vm)
	: DXAPlayer(), _vm(vm) {

	_flags = 0;
	_wizResNum = 0;
}

int MoviePlayer::getImageNum() {
	if (!_fd.isOpen())
		return 0;
	return _wizResNum;
}

int MoviePlayer::load(const char *filename, int flags, int image) {
	char videoName[100];

	if (_fd.isOpen()) {
		closeFile();
	}

	// Change file extension to dxa
	strcpy(videoName, filename);
	int len = strlen(videoName) - 3;
	videoName[len++] = 'd';
	videoName[len++] = 'x';
	videoName[len++] = 'a';
	
	if (!loadFile(videoName)) {
		warning("Failed to load video file %s", videoName);
		return -1;
	} 
	debug(1, "Playing video %s", videoName);

	// Skip sound tag
	_fd.readUint32BE();

	if (flags & 2) {
		_vm->_wiz->createWizEmptyImage(image, 0, 0, _width, _height);
	}

	_flags = flags;
	_wizResNum = image;

	return 0;
}

void MoviePlayer::handleNextFrame() {
	if (_fd.isOpen() == false) {
		return;
	}

	VirtScreen *pvs = &_vm->virtscr[kMainVirtScreen];

	decodeNextFrame();

	if (_flags & 2) {
		uint8 *dstPtr = _vm->getResourceAddress(rtImage, _wizResNum);
		assert(dstPtr);
		uint8 *dst = _vm->findWrappedBlock(MKID_BE('WIZD'), dstPtr, 0, 0);
		assert(dst);
		copyFrameToBuffer(dst, 0, 0, _vm->_screenWidth);
	} else if (_flags & 1) {
		copyFrameToBuffer(pvs->getBackPixels(0, 0), 0, 0, _vm->_screenWidth);
		
		Common::Rect imageRect(_width, _height);
		_vm->restoreBackgroundHE(imageRect);
	} else {
		copyFrameToBuffer(pvs->getPixels(0, 0), 0, 0, _vm->_screenWidth);

		_vm->markRectAsDirty(kMainVirtScreen, 0, 0, _width, _height);
	}

	_frameNum++;
	if (_frameNum == _framesCount) {
		closeFile();
	}
}

void MoviePlayer::setPalette(byte *pal) {
	_vm->setPaletteFromPtr(pal, 256);
}

} // End of namespace Simon
