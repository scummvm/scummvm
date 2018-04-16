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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/assets.h"
#include "mads/compression.h"
#include "mads/events.h"
#include "mads/palette.h"

namespace MADS {

SpriteAsset::SpriteAsset(MADSEngine *vm, const Common::String &resourceName, int flags) : _vm(vm) {
	Common::String resName = resourceName;
	if (!resName.hasSuffix(".SS") && !resName.hasSuffix(".ss"))
		resName += ".SS";
	_srcSize = 0;

	File file(resName);
	load(&file, flags);

	file.close();
}

SpriteAsset::SpriteAsset(MADSEngine *vm, Common::SeekableReadStream *stream, int flags) : _vm(vm) {
	_srcSize = 0;

	load(stream, flags);
}

SpriteAsset::~SpriteAsset() {
	if (_usageIndex)
		_vm->_palette->_paletteUsage.resetPalFlags(_usageIndex);

	for (uint i = 0; i < _frames.size(); ++i)
		delete _frames[i]._frame;

	delete _charInfo;
}

void SpriteAsset::load(Common::SeekableReadStream *stream, int flags) {
	int curFrame = 0;
	uint32 frameOffset = 0;
	MadsPack sprite(stream);
	_frameRate = 0;
	_pixelSpeed = 0;
	_maxWidth = 0;
	_maxHeight = 0;
	_usageIndex = -1;

	Common::SeekableReadStream *spriteStream = sprite.getItemStream(0);
	_mode = spriteStream->readByte();
	spriteStream->skip(1);
	int type1 = spriteStream->readUint16LE();
	int type2 = spriteStream->readUint16LE();
	_isBackground = (type1 != 0) && (type2 < 4);
	spriteStream->skip(32);
	_frameCount = spriteStream->readUint16LE();

	if ((flags & ASSET_CHAR_INFO) == 0)
		_charInfo = nullptr;
	else
		_charInfo = new SpriteSetCharInfo(spriteStream);

	delete spriteStream;

	// Get the palette data
	Common::SeekableReadStream *palStream = sprite.getItemStream(2);
	Common::Array<RGB6> palette;

	int numColors = palStream->readUint16LE();
	assert(numColors <= 252);
	_colorCount = numColors;

	// Load in the palette
	palette.resize(numColors);
	for (int i = 0; i < numColors; ++i)
		palette[i].load(palStream);
	delete palStream;

	// Process the palette data
	if (flags & (ASSET_TRANSLATE | ASSET_SPINNING_OBJECT)) {
		_usageIndex = 0;

		if (flags & ASSET_SPINNING_OBJECT) {
			int newPalCtr = 0;

			for (uint i = 0; i < palette.size(); ++i) {
				RGB6 &rgb = palette[i];

				// Scan for existing rgb at beginning of the main palette
				bool found = false;
				for (int pIndex = 0; pIndex < 4 && !found; ++pIndex) {
					byte *palP = &_vm->_palette->_mainPalette[pIndex * 3];
					if (palP[0] == rgb.r && palP[1] == rgb.g && palP[2] == rgb.b) {
						rgb._palIndex = pIndex;
						found = true;
					}
				}

				if (!found) {
					// Existing palette entry not found, so need to add it in
					int palIndex = (0xF7F607 >> (8 * newPalCtr)) & 0xff;
					byte *palP = &_vm->_palette->_mainPalette[palIndex * 3];
					palP[0] = rgb.r;
					palP[1] = rgb.g;
					palP[2] = rgb.b;
					rgb._palIndex = palIndex;

					newPalCtr = MIN(newPalCtr + 1, 2);
				}
			}
		}
	} else {
		_usageIndex = _vm->_palette->_paletteUsage.process(palette, flags & 0xF800);
		assert(_usageIndex >= 0);
	}

	spriteStream = sprite.getItemStream(1);
	Common::SeekableReadStream *spriteDataStream = sprite.getItemStream(3);
	SpriteAssetFrame frame;
	Common::Array<int> frameSizes;
	for (curFrame = 0; curFrame < _frameCount; curFrame++) {
		frame._stream = 0;
		frame._comp = 0;
		frameOffset = spriteStream->readUint32LE();
		_frameOffsets.push_back(frameOffset);
		uint32 frameSize = spriteStream->readUint32LE();
		frameSizes.push_back(frameSize);

		frame._bounds.left = spriteStream->readSint16LE();
		frame._bounds.top  = spriteStream->readSint16LE();
		frame._bounds.setWidth(spriteStream->readUint16LE());
		frame._bounds.setHeight(spriteStream->readUint16LE());

		if (curFrame == 0)
			debugC(1, kDebugGraphics, "%i frames, x = %i, y = %i, w = %i, h = %i\n",
			_frameCount, frame._bounds.left, frame._bounds.top,
			frame._bounds.width(), frame._bounds.height());

		if (_mode == 0) {
			// Create a frame and decompress the raw pixel data
			uint32 currPos = (uint32)spriteDataStream->pos();
			frame._frame = new MSprite(spriteDataStream, palette, frame._bounds);
			assert((uint32)spriteDataStream->pos() == (currPos + frameSize));
		}

		_frames.push_back(frame);
	}

	if (_mode != 0) {
		// Handle decompressing Fab encoded data
		for (curFrame = 0; curFrame < _frameCount; curFrame++) {
			FabDecompressor fab;

			int srcSize = (curFrame == (_frameCount - 1)) ? spriteDataStream->size() - _frameOffsets[curFrame] :
				_frameOffsets[curFrame + 1] - _frameOffsets[curFrame];
			byte *srcData = new byte[srcSize];
			assert(srcData);
			spriteDataStream->read(srcData, srcSize);

			byte *destData = new byte[frameSizes[curFrame]];
			assert(destData);

			fab.decompress(srcData, srcSize, destData, frameSizes[curFrame]);

			// Load the frames
			Common::MemoryReadStream *rs = new Common::MemoryReadStream(destData, frameSizes[curFrame]);
			_frames[curFrame]._frame = new MSprite(rs, palette, _frames[curFrame]._bounds);
			delete rs;

			delete[] srcData;
			delete[] destData;
		}
	}

	delete spriteStream;
	delete spriteDataStream;
}

MSprite *SpriteAsset::getFrame(int frameIndex) {
	if ((uint)frameIndex < _frames.size()) {
		return _frames[frameIndex]._frame;
	} else {
		debugC(kDebugGraphics, "SpriteAsset::getFrame: Invalid frame %d, out of %d", frameIndex, _frames.size());
		return _frames[_frames.size() - 1]._frame;
	}
}

/*------------------------------------------------------------------------*/

SpriteSetCharInfo::SpriteSetCharInfo(Common::SeekableReadStream *s) {
	_totalFrames = s->readByte();
	s->skip(1);
	_numEntries = s->readUint16LE();

	for (int i = 0; i < 16; ++i)
		_startFrames[i] = s->readUint16LE();
	for (int i = 0; i < 16; ++i)
		_stopFrames[i] = s->readUint16LE();
	for (int i = 0; i < 16; ++i)
		_ticksList[i] = s->readUint16LE();

	_velocity = s->readUint16LE();
	_ticksAmount = s->readByte();
	_centerOfGravity = s->readByte();
}

} // End of namespace MADS
