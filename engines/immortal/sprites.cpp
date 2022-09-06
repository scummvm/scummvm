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
	// We set the center X and Y, for some reason
	d->_cenX = cenX;
	d->_cenY = cenY;

	// But now we need to get the rest of the meta data for each frame
	// index is the index of the sprite within the file (not the same as the sprite name enum)
	index *= 8;
	f->seek(index);

	index = f->readUint16LE();
	uint16 numImages = f->readUint16LE();

	d->_numImages = numImages;
	//debug("Number of Frames: %d", numFrames);

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
		newImage._deltaX = f->readUint16LE() << 1;      // the ASL might not be required, depending on whether the data is actually in bytes or pixels <-- this also might not be used in the game anyway? Lol
		newImage._deltaY = f->readUint16LE();
		newImage._rectW  = f->readUint16LE();
		newImage._rectH  = f->readUint16LE();
		uint16 next = 0;
		for (int j = 0; j < newImage._rectH; j++) {
			next = f->readUint16LE();
			//debug("First after RectH: %04X", next);
			newImage._deltaPos.push_back(next);
			next = f->readUint16LE();
			//debug("Second after RectH: %04X", next);
			newImage._scanWidth.push_back(next);
			Common::Array<byte> b;
			b.resize(newImage._scanWidth[j]);
			for (int k = 0; k < newImage._scanWidth[j]; k++) {
				b[k] = f->readByte();
				//debugN("%02X", b[k]);
			}
			//debug("");
			newImage._bitmap.push_back(b);
		}
		images.push_back(newImage);
	}

	d->_images = images;
}

bool ImmortalEngine::clipSprite(uint16 &height, uint16 &pointIndex, uint16 &skipY, DataSprite *dSprite, uint16 &pointX, uint16 &pointY, int img, uint16 bmw, int superTop, int superBottom) {
	// This bit is to get the base index into the screen buffer, unless that's already been done, which is _lastPoint
	if ((pointY != _lastY) || (bmw != _lastBMW)) {
		_lastBMW = bmw;
		_lastY = pointY;
		if (pointY < 0x80) {
			_lastPoint = pointY * bmw;
		} else {
			pointY = (pointY ^ 0xFF) + 1;
			_lastPoint = pointY * bmw;
			_lastPoint = 0 - _lastPoint;
		}
	}

	pointIndex = _lastPoint;
	return false;


	// Now we begin clipping, starting with totally offscreen
	// We do this by checking if the sprite is above the top of the screen, or below the bottom of it
	if (pointY > superBottom) {
		return true;

	} else if ((height + pointY) < superTop) {
		return true;

	// Now we actually clip top/bottom parts
	} else {

		// Starting with checking if any of the sprite is under the bottom of the screen
		if ((height + pointY) >= superBottom) {
			height = superBottom - pointY;
		}

		// Next we get the difference of overlap from the sprite if it is above the top
		if ((superTop - pointY) < 0x8000) {
			skipY = (superTop - pointY);
		}

		// The image is clipped, time to move the index to the sprite's first scanline base position
		pointIndex += (pointX / 2) + dSprite->_images[img]._rectW;
	}
	return false;
}

void ImmortalEngine::spriteAligned(DataSprite *dSprite, Image &img, uint16 &skipY, uint16 &pointIndex, uint16 &height, uint16 bmw, byte *dst) {
	//debug("draw the sprite");

	debug("%d, %d, %04X", height, skipY, pointIndex);

	byte pixel;
	// For debug currently, align to the word by default
	pointIndex &= 0xFFFE;

	// Position is weird
	pointIndex += 50;

	debug("SPRITE START ------");
	for (int y = 0; y < height; y++, pointIndex += (bmw * 2)) {

		//debug("%04X, %04X ", pointIndex, img._deltaPos[y]);
		
		if (img._deltaPos[y] < 0x8000) {
			pointIndex += (img._deltaPos[y] * 2);
		}
		else {
			pointIndex -= ((0 - img._deltaPos[y]) * 2);
		}

		for (int x = 0; x < img._scanWidth[y]; x++, pointIndex += 2) {

			//if (y > skipY) {
				pixel = img._bitmap[y][x];
				_screenBuff[pointIndex]     = (pixel & kMask8High) >> 4;
				_screenBuff[pointIndex + 1] =  pixel & kMask8Low;				
			//}
		}
	}
	debug("SPRITE END -------");
}

void ImmortalEngine::spriteNotAligned() {

}

void ImmortalEngine::superSprite(DataSprite *dSprite, uint16 pointX, uint16 pointY, int img, uint16 bmw, byte *dst, int superTop, int superBottom) {
	// Main image construction routine

	uint16 cenX   = dSprite->_cenX;
	uint16 cenY   = dSprite->_cenY;
	uint16 dY     = dSprite->_images[img]._deltaY;
	uint16 height = dSprite->_images[img]._rectH;

	uint16 skipY  = 0;
	uint16 pointIndex = 0;			// This is screen and screen + 2 in the source

	pointX -= cenX;
	pointY -= cenY;
	pointY += dY;

	// Normally I would just make the return from clip be reversed, but the idea is that the return would be 'offscreen == true'
	if (!(clipSprite(height, pointIndex, skipY, dSprite, pointX, pointY, img, bmw, superTop, superBottom))) {
		// Alignment is determined by whether the x position of the point is positive or negative
		if (pointX >= 0x8000) {
			spriteAligned(dSprite, dSprite->_images[img], skipY, pointIndex, height, bmw, dst);
		
		} else {
			spriteAligned(dSprite, dSprite->_images[img], skipY, pointIndex, height, bmw, dst);
		}
	}

}

} // namespace Immortal






































