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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/assets.h"
#include "mads/compression.h"
#include "mads/events.h"

namespace MADS {

SpriteAsset::SpriteAsset(MADSEngine *vm, const Common::String &resourceName, int flags):
		_vm(vm) {
	Common::String resName = resourceName;
	if (!resName.hasSuffix(".SS"))
		resName += ".SS";
	
	File file(resName);
	MadsPack sprites(&file);

	int curFrame = 0;
	uint32 frameOffset = 0;
	_frameRate = 0;
	_pixelSpeed = 0;
	_maxWidth = 0;
	_maxHeight = 0;

	Common::SeekableReadStream *spriteStream = sprites.getItemStream(0);
	for (int i = 0; i < 19; i++) {
		spriteStream->readUint16LE();
	}
	_frameCount = spriteStream->readUint16LE();
	// we skip the rest of the data
	delete spriteStream;

	// Get the palette data
	spriteStream = sprites.getItemStream(2);
	int numColors = 0;
	byte *palData = _vm->_palette->decodePalette(spriteStream, &numColors);

	Common::copy(palData, &palData[numColors], &_palette[0]);
	if (numColors < 256) 
		Common::fill(&_palette[numColors * 3], &_palette[PALETTE_SIZE], 0);
	_colorCount = numColors;
	delete[] palData;
	delete spriteStream;

	spriteStream = sprites.getItemStream(1);
	Common::SeekableReadStream *spriteDataStream = sprites.getItemStream(3);

	SpriteAssetFrame frame;
	for (curFrame = 0; curFrame < _frameCount; curFrame++) {
		frame.comp = 0;
		frameOffset = spriteStream->readUint32LE();
		_frameOffsets.push_back(frameOffset);
		spriteStream->readUint32LE();	// frame size
		frame.x = spriteStream->readUint16LE();
		frame.y = spriteStream->readUint16LE();
		frame.w = spriteStream->readUint16LE();
		frame.h = spriteStream->readUint16LE();
		if (curFrame == 0) {
			debugN(kDebugGraphics, "%i frames, x = %i, y = %i, w = %i, h = %i\n", 
				_frameCount, frame.x, frame.y, frame.w, frame.h);
		}

		frame.frame = new MSprite(spriteDataStream, Common::Point(frame.x, frame.y), 
			frame.w, frame.h, false);
		_frames.push_back(frame);
	}

	delete spriteStream;
	delete spriteDataStream;
	file.close();
}

} // End of namespace MADS
