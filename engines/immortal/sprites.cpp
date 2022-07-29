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
	uint16 numFrames = f->readUint16LE();

	d->_numFrames = numFrames;
	debug("Number of Frames: %d", numFrames);

	// Only here for dragon, but just in case, it's a high number so it should catch others
	if (numFrames >= 0x0200) {
		debug("** Crazy large value, this isn't a frame number **");
		return;
	}

	Common::Array<Frame> frames;

	for (int i = 0; i < numFrames; i++) {
		Frame newFrame;
		f->seek(index + (i*2));
		int ptrFrame = f->readUint16LE();
		f->seek(ptrFrame);
		newFrame._deltaX = f->readUint16LE() << 1;      // the ASL might not be required, depending on how I translate the sprite drawing
		newFrame._deltaY = f->readUint16LE();
		newFrame._rectX = f->readUint16LE();
		newFrame._rectY = f->readUint16LE();
		frames.push_back(newFrame);
		// This is probably where we will get the bitmap when I know how to get it
	}

	d->_frames = frames;
}


void ImmortalEngine::superSprite(int s, uint16 x, uint16 y, Frame f, int bmw, byte *dst, int sT, int sB) {}

} // namespace Immortal
























