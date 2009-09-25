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

#ifdef ENABLE_HE

#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"

#include "sound/audiostream.h"

namespace Scumm {

MoviePlayer::MoviePlayer(ScummEngine_v90he *vm, Audio::Mixer *mixer)
	: SmackerDecoder(mixer), _vm(vm), _mixer(mixer) {

	_flags = 0;
	_wizResNum = 0;
}

int MoviePlayer::getImageNum() {
	if (!isVideoLoaded())
		return 0;
	return _wizResNum;
}

int MoviePlayer::load(const char *filename, int flags, int image) {
	if (isVideoLoaded()) {
		closeFile();
	}

	if (!loadFile(filename)) {
		warning("Failed to load video file %s", filename);
		return -1;
	}
	debug(1, "Playing video %s", filename);

	if (flags & 2) {
		_vm->_wiz->createWizEmptyImage(image, 0, 0, getWidth(), getHeight());
	}

	_flags = flags;
	_wizResNum = image;

	return 0;
}

void MoviePlayer::copyFrameToBuffer(byte *dst, int dstType, uint x, uint y, uint pitch) {
	uint h = getHeight();
	uint w = getWidth();

	byte *src = _videoFrameBuffer;

	if (_vm->_game.features & GF_16BIT_COLOR) {
		dst += y * pitch + x * 2;
		do {
			for (uint i = 0; i < w; i++) {
				uint16 color = READ_LE_UINT16(_vm->_hePalettes + _vm->_hePaletteSlot + 768 + src[i] * 2);
				switch (dstType) {
				case kDstScreen:
					WRITE_UINT16(dst + i * 2, color);
					break;
				case kDstResource:
					WRITE_LE_UINT16(dst + i * 2, color);
					break;
				default:
					error("copyFrameToBuffer: Unknown dstType %d", dstType);
				}
			}
			dst += pitch;
			src += w;
		} while (--h);
	} else {
		dst += y * pitch + x;
		do {
			memcpy(dst, src, w);
			dst += pitch;
			src += w;
		} while (--h);
	}
}

void MoviePlayer::handleNextFrame() {
	if (!isVideoLoaded()) {
		return;
	}

	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];

	decodeNextFrame();

	if (_flags & 2) {
		uint8 *dstPtr = _vm->getResourceAddress(rtImage, _wizResNum);
		assert(dstPtr);
		uint8 *dst = _vm->findWrappedBlock(MKID_BE('WIZD'), dstPtr, 0, 0);
		assert(dst);
		copyFrameToBuffer(dst, kDstResource, 0, 0, _vm->_screenWidth * _vm->_bytesPerPixel);
	} else if (_flags & 1) {
		copyFrameToBuffer(pvs->getBackPixels(0, 0), kDstScreen, 0, 0, pvs->pitch);

		Common::Rect imageRect(getWidth(), getHeight());
		_vm->restoreBackgroundHE(imageRect);
	} else {
		copyFrameToBuffer(pvs->getPixels(0, 0), kDstScreen, 0, 0, pvs->pitch);

		Common::Rect imageRect(getWidth(), getHeight());
		_vm->markRectAsDirty(kMainVirtScreen, imageRect);
	}

	if (getCurFrame() == getFrameCount()) {
		closeFile();
	}
}

void MoviePlayer::setPalette(byte *pal) {
	_vm->setPaletteFromPtr(pal, 256);
}

} // End of namespace Scumm

#endif // ENABLE_HE
