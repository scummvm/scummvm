/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "immortal/immortal.h"

/* -- How does image construction work --
 * One thing to note about this translation, is that the source
 * has a lot of address related stuff mixed in to it. This is
 * because 'Super Sprites' could use a screen buffer and sprite
 * data from anywhere in memory, including locations that cross
 * bank boundaries. This means that you don't just have
 * position -> relative position -> screen, you have
 * position -> relative position -> relative *address* position -> screen
 * With that out of the way, here's what a sprite is:
 * A 'Super Sprite' is several layers of structures combined.
 * This is both more and less complex in the source. It is structurally
 * less complicated, only being seen as a sprite + frame, and a cycle.
 * But in reality that comes with complicated indexing and routines
 * designed just to get relative indexes that are already in memory.
 * Instead, I have chosen to clean up the structure a little bit,
 * which in turns makes it slightly more complicated on a top level.
 * What we end up with, basically looks like this:
 * Cycle (ram/rom)
 *   |
 * Sprite (ram)
 *   |
 * DataSprite (rom)
 *   |
 * Frame (rom)
 *   |
 * Scanline (rom)
 *   |
 * Bitmap (rom)
 */

namespace Immortal {

/* 
 *
 * -----                -----
 * ----- Main Functions -----
 * -----                -----
 *
 */

// This function is basically setSpriteCenter + getSpriteInfo from the source
void ImmortalEngine::initDataSprite(Common::SeekableReadStream *f, DataSprite *d, int index, uint16 cenX, uint16 cenY) {
	// We set the center X and Y
	d->_cenX = cenX;
	d->_cenY = cenY;

	// But now we need to get the rest of the meta data for each frame
	// index is the index of the sprite within the file (not the same as the sprite name enum)
	index *= 8;
	f->seek(index);

	index = f->readUint16LE();
	uint16 numImages = f->readUint16LE();

	d->_numImages = numImages;

	// Only here for dragon, but just in case, it's a high number so it should catch others
	if (numImages >= 0x0200) {
		//debug("** Crazy large value, this isn't a frame number **");
		return;
	}

	Common::Array<Image> images;

	for (int i = 0; i < numImages; i++) {
		Image newImage;
		f->seek(index + (i * 2));
		int ptrFrame = f->readUint16LE();
		f->seek(ptrFrame);
		newImage._deltaX = f->readUint16LE() << 1;      // This member does not seem to be used in the actual game, and it is not clear whether it needs the << 1 or if that was fixed before release
		newImage._deltaY = f->readUint16LE();
		newImage._rectW  = f->readUint16LE();
		newImage._rectH  = f->readUint16LE();
		uint16 next = 0;
		for (int j = 0; j < newImage._rectH; j++) {
			next = f->readUint16LE();
			newImage._deltaPos.push_back(next);
			next = f->readUint16LE();
			newImage._scanWidth.push_back(next);
			Common::Array<byte> b;
			b.resize(newImage._scanWidth[j]);
			for (int k = 0; k < newImage._scanWidth[j]; k++) {
				b[k] = f->readByte();
			}
			newImage._bitmap.push_back(b);
		}
		images.push_back(newImage);
	}

	d->_images = images;
}

bool ImmortalEngine::clipSprite(uint16 &height, uint16 &pointIndex, uint16 &skipY, DataSprite *dSprite, uint16 &pointX, uint16 &pointY, int img, uint16 bmw, uint16 superTop, uint16 superBottom) {
	/* Something important to note here:
	 * In the source, bmw is not *2, and pointX is /2. However, the source
	 * was using a buffer of 2 pixels per byte. In ScummVM, the screen buffer
	 * is 1 pixel per byte. This means some calculations are slightly different.
	 */

	// This bit is to get the base index into the screen buffer, unless that's already been done, which is _lastPoint
	if ((pointY != _lastY) || (bmw != _lastBMW)) {
		_lastBMW = bmw;
		_lastY = pointY;
		if (pointY < kMaskNeg) {
			// For the Apple IIGS, pointY in pixels needed to be converted to bytes. For us, it's the other way around, we need bmw in pixels
			// This should probably use mult16() instead of *
			_lastPoint = pointY * (bmw);
		} else {
			// Screen wrapping?
			uint16 temp = (0 - pointY) + 1;
			_lastPoint = temp * bmw;
			_lastPoint = 0 - _lastPoint;
		}
	}

	pointIndex = _lastPoint;

	// Now we begin clipping, starting with totally offscreen
	if (pointY > superBottom) {
		return true;

	} else if ((pointY + height) < superTop) {
		return true;

	/* The actual clipping is pretty simple:
	 * Lower height = stop drawing the sprite early. Higher SkipY = start drawing the sprite late
	 * So we just determine the delta for each based on superTop and superBottom
	 */
	} else {

		// Starting with checking if any of the sprite is under the bottom of the screen
		if ((pointY + height) >= superBottom) {
			height = superBottom - pointY;
		}

		// Next we get the difference of overlap from the sprite if it is above the top
		if (uint16((superTop - pointY)) < kMaskNeg) {
			skipY = (superTop - pointY);
		}

		// The image is clipped, time to move the index to the sprite's first scanline base position
		pointIndex += pointX;// + dSprite->_images[img]._rectW;
	}
	return false;
}

void ImmortalEngine::spriteAligned(DataSprite *dSprite, Image &img, uint16 &skipY, uint16 &pointIndex, uint16 &height, uint16 bmw, byte *dst) {
	/* This is an approximation of the sprite drawing system in the source.
	 * It is an approximation because the source needed to do some things
	 * that aren't relevant anymore, and it had some....creative solutions.
	 * For example, transparency was handled with a 256 byte table of masks
	 * that was indexed by the pixel itself, and used to find what nyble needed
	 * to be masked. However we are using a slightly different kind of screen buffer,
	 * and so I chose a more traditional method. Likewise, alignement was
	 * relevant for the source, but is not relevant here, and the sprite drawing
	 * is not accomplished by indexed a set of code blocks.
	 */
	byte pixel1 = 0;
	byte pixel2 = 0;

	// For every scanline before height
	for (int y = 0; y < height; y++, pointIndex += (bmw)) {

		// We increase the position by one screen width
		if (img._deltaPos[y] < kMaskNeg) {
			pointIndex += (img._deltaPos[y] * 2);
		}

		// And if the delta X for the line is positive, we add it. If negative we subtract
		else {
			pointIndex -= ((0 - img._deltaPos[y]) * 2);
		}

		// For every pixel in the scanline
		for (int x = 0; x < img._scanWidth[y]; x++, pointIndex += 2) {
			// SkipY defines the lines we don't draw because they are clipped
			if (y >= skipY) {

				// For handling transparency, I chose to simply check if the pixel is 0,
				// as that is the transparent colour
				pixel1 = (img._bitmap[y][x] & kMask8High) >> 4;
				pixel2 = (img._bitmap[y][x] & kMask8Low);
				
				if (pixel1 != 0) {
					_screenBuff[pointIndex] = pixel1;
				}

				if (pixel2 != 0) {
					_screenBuff[pointIndex + 1] = pixel2;
				}

			}
		}
	}
}

void ImmortalEngine::superSprite(DataSprite *dSprite, uint16 pointX, uint16 pointY, int img, uint16 bmw, byte *dst, uint16 superTop, uint16 superBottom) {
	// Main sprite image construction routine

	// For the Apple IIGS, the bmw is in bytes, but for us it needs to be the reverse, in pixels
	bmw <<= 1;

	uint16 cenX   = dSprite->_cenX;
	uint16 cenY   = dSprite->_cenY;
	uint16 dY     = dSprite->_images[img]._deltaY;
	uint16 height = dSprite->_images[img]._rectH;

	uint16 skipY  = 0;
	uint16 pointIndex = 0;			// This is 'screen' and 'screen + 2' in the source

	pointX -= cenX;
	pointY -= cenY;
	pointY += dY;

	// Normally I would just make the return from clip be reversed, but the idea is that the return would be 'offscreen == true'
	if (!(clipSprite(height, pointIndex, skipY, dSprite, pointX, pointY, img, bmw, superTop, superBottom))) {
		
		// Alignment was a factor in the assembly because it was essentially 2 pixels per byte. However ScummVM is 1 pixel per byte
		spriteAligned(dSprite, dSprite->_images[img], skipY, pointIndex, height, bmw, dst);
	}

}

} // namespace Immortal
