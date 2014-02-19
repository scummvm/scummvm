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

#include "engines/util.h"
#include "mads/compression.h"
#include "mads/graphics.h"
#include "mads/mads.h"
#include "mads/msprite.h"
#include "mads/msurface.h"

namespace MADS {

MADSEngine *MSurface::_vm = nullptr;

MSurface *MSurface::init(bool isScreen) {
	if (_vm->getGameID() == GType_RexNebular) {
		return new MSurfaceNebular(isScreen);
	} else if (_vm->getGameFeatures() & GF_MADS) {
		return new MSurfaceMADS(isScreen);
	} else {
		return new MSurfaceM4(isScreen);
	}
}

MSurface *MSurface::init(int w, int h) {
	if (_vm->getGameID() == GType_RexNebular) {
		return new MSurfaceNebular(w, h);
	} else if (_vm->getGameFeatures() & GF_MADS) {
		return new MSurfaceMADS(w, h);
	} else {
		return new MSurfaceM4(w, h);
	}
}

MSurface::MSurface(bool isScreen) { 
	create(g_system->getWidth(), g_system->getHeight());
	_isScreen = isScreen;
}

MSurface::MSurface(int Width, int Height) { 
	create(Width, Height); 
	_isScreen = false; 
}

void MSurface::vLine(int x, int y1, int y2) {
	Graphics::Surface::vLine(x, y1, y2, _color);
}

void MSurface::hLine(int x1, int x2, int y) {
	Graphics::Surface::hLine(x1, y, x2, _color);
}

void MSurface::vLineXor(int x, int y1, int y2) {
	// Clipping
	if (x < 0 || x >= w)
		return;

	if (y2 < y1)
		SWAP(y2, y1);

	if (y1 < 0)
		y1 = 0;
	if (y2 >= h)
		y2 = h - 1;

	byte *ptr = (byte *)getBasePtr(x, y1);
	while (y1++ <= y2) {
		*ptr ^= 0xFF;
		ptr += pitch;
	}

}

void MSurface::hLineXor(int x1, int x2, int y) {
	// Clipping
	if (y < 0 || y >= h)
		return;

	if (x2 < x1)
		SWAP(x2, x1);

	if (x1 < 0)
		x1 = 0;
	if (x2 >= w)
		x2 = w - 1;

	if (x2 < x1)
		return;

	byte *ptr = (byte *)getBasePtr(x1, y);
	while (x1++ <= x2)
		*ptr++ ^= 0xFF;

}

void MSurface::line(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::drawLine(x1, y1, x2, y2, color);
}


void MSurface::frameRect(int x1, int y1, int x2, int y2) {
	Graphics::Surface::frameRect(Common::Rect(x1, y1, x2, y2), _color);
}

void MSurface::fillRect(int x1, int y1, int x2, int y2) {
	Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), _color);
}

int MSurface::scaleValue(int value, int scale, int err) {
	int scaled = 0;
	while (value--) {
		err -= scale;
		while (err < 0) {
			scaled++;
			err += 100;
		}
	}
	return scaled;
}

void MSurface::drawSprite(int x, int y, SpriteInfo &info, const Common::Rect &clipRect) {

	enum {
		kStatusSkip,
		kStatusScale,
		kStatusDraw
	};

	// NOTE: The current clipping code assumes that the top left corner of the clip
	// rectangle is always 0, 0
	assert(clipRect.top == 0 && clipRect.left == 0);

	// TODO: Put err* and scaled* into SpriteInfo
	int errX = info.hotX * info.scaleX % 100;
	int errY = info.hotY * info.scaleY % 100;
	int scaledWidth = scaleValue(info.width, info.scaleX, errX);
	int scaledHeight = scaleValue(info.height, info.scaleY, errY);

	/*
	printf("MSurface::drawSprite() info.width = %d; info.scaleX = %d; info.height = %d; info.scaleY = %d; scaledWidth = %d; scaledHeight = %d\n",
		info.width, info.scaleX, info.height, info.scaleY, scaledWidth, scaledHeight); fflush(stdout);
	*/
	
	int clipX = 0, clipY = 0;
	// Clip the sprite's width and height according to the clip rectangle's dimensions
	// This clips the sprite to the bottom and right
	if (x >= 0) {
		scaledWidth = MIN<int>(x + scaledWidth, clipRect.right) - x;
	} else {
		clipX = x;
		scaledWidth = x + scaledWidth;
	}
	if (y >= 0) {
		scaledHeight = MIN<int>(y + scaledHeight, clipRect.bottom) - y;
	} else {
		clipY = y;
		scaledHeight = y + scaledHeight;
	}

	//printf("MSurface::drawSprite() width = %d; height = %d; scaledWidth = %d; scaledHeight = %d\n", info.width, info.height, scaledWidth, scaledHeight); fflush(stdout);

	// Check if sprite is inside the screen. If it's not, there's no need to draw it
	if (scaledWidth + x <= 0 || scaledHeight + y <= 0)	// check left and top (in case x,y are negative)
		return;
	if (scaledWidth <= 0 || scaledHeight <= 0)			// check right and bottom
		return;
	int heightAmt = scaledHeight;

	byte *src = info.sprite->_surface.getData();
	byte *dst = getBasePtr(x - info.hotX - clipX, y - info.hotY - clipY);

	int status = kStatusSkip;
	byte *scaledLineBuf = new byte[scaledWidth];

	while (heightAmt > 0) {

		if (status == kStatusSkip) {
			// Skip line
			errY -= info.scaleY;
			if (errY < 0)
				status = kStatusScale;
			else
				src += info.width;
		} else {

			if (status == kStatusScale) {
				// Scale current line
				byte *lineDst = scaledLineBuf;
				int curErrX = errX;
				int widthVal = scaledWidth;
				byte *tempSrc = src;
				int startX = clipX;
				while (widthVal > 0) {
					byte pixel = *tempSrc++;
					curErrX -= info.scaleX;
					while (curErrX < 0) {
						if (startX == 0) {
							*lineDst++ = pixel;
							widthVal--;
						} else {
							startX++;
						}
						curErrX += 100;
					}
				}
				src += info.width;
				status = kStatusDraw;
			}

			if (status == kStatusDraw && clipY == 0) {
				// Draw previously scaled line
				// TODO Implement different drawing types (depth, shadow etc.)
				byte *tempDst = dst;
				for (int lineX = 0; lineX < scaledWidth; lineX++) {
					byte pixel = scaledLineBuf[lineX];

					if (info.encoding & 0x80) {

						if (pixel == 0x80) {
							pixel = 0;
						} else {
							byte destPixel = *tempDst;
							byte r, g, b;
							r = CLIP((info.palette[destPixel].r * pixel) >> 10, 0, 31);
							g = CLIP((info.palette[destPixel].g * pixel) >> 10, 0, 31);
							b = CLIP((info.palette[destPixel].b * pixel) >> 10, 0, 31);
							pixel = info.inverseColorTable[(b << 10) | (g << 5) | r];
						}
					}

					if (pixel)
						*tempDst = pixel;

					tempDst++;
				}
				dst += pitch;
				heightAmt--;
				// TODO depth etc.
				//depthAddress += Destination -> Width;

				errY += 100;
				if (errY >= 0)
					status = kStatusSkip;
			} else if (status == kStatusDraw && clipY < 0) {
				clipY++;

				errY += 100;
				if (errY >= 0)
					status = kStatusSkip;
			}

		}

	}
	
	delete[] scaledLineBuf;

}

// Surface methods

byte *MSurface::getData() {
	return (byte *)Graphics::Surface::getPixels();
}

byte *MSurface::getBasePtr(int x, int y) {
	return (byte *)Graphics::Surface::getBasePtr(x, y);
}

void MSurface::freeData() {
}

void MSurface::empty() {
	Common::fill(getBasePtr(0, 0), getBasePtr(w, h), _vm->_palette->BLACK);
}

void MSurface::frameRect(const Common::Rect &r, uint8 color) {
	Graphics::Surface::frameRect(r, color);
}

void MSurface::fillRect(const Common::Rect &r, uint8 color) {
	Graphics::Surface::fillRect(r, color);
}

void MSurface::copyFrom(MSurface *src, const Common::Rect &srcBounds, int destX, int destY,
						 int transparentColor) {
	// Validation of the rectangle and position
	if ((destX >= w) || (destY >= h))
		return;

	Common::Rect copyRect = srcBounds;
	if (destX < 0) {
		copyRect.left += -destX;
		destX = 0;
	} else if (destX + copyRect.width() > w) {
		copyRect.right -= destX + copyRect.width() - w;
	}
	if (destY < 0) {
		copyRect.top += -destY;
		destY = 0;
	} else if (destY + copyRect.height() > h) {
		copyRect.bottom -= destY + copyRect.height() - h;
	}

	if (!copyRect.isValidRect())
		return;

	// Copy the specified area

	byte *data = src->getData();
	byte *srcPtr = data + (src->width() * copyRect.top + copyRect.left);
	byte *destPtr = (byte *)pixels + (destY * width()) + destX;

	for (int rowCtr = 0; rowCtr < copyRect.height(); ++rowCtr) {
		if (transparentColor == -1)
			// No transparency, so copy line over
			Common::copy(srcPtr, srcPtr + copyRect.width(), destPtr);
		else {
			// Copy each byte one at a time checking for the transparency color
			for (int xCtr = 0; xCtr < copyRect.width(); ++xCtr)
				if (srcPtr[xCtr] != transparentColor) destPtr[xCtr] = srcPtr[xCtr];
		}

		srcPtr += src->width();
		destPtr += width();
	}

	src->freeData();
}

#undef COL_TRANS

void MSurface::translate(RGBList *list, bool isTransparent) {
	byte *p = getBasePtr(0, 0);
	byte *palIndexes = list->palIndexes();

	for (int i = 0; i < width() * height(); ++i, ++p) {
		if (!isTransparent || (*p != 0)) {
			assert(*p < list->size());
			*p = palIndexes[*p];
		}
	}

	freeData();
}

/*------------------------------------------------------------------------*/

void MSurfaceMADS::loadCodes(Common::SeekableReadStream *source) {
	if (!source) {
		free();
		return;
	}

	uint16 widthVal = MADS_SCREEN_WIDTH;
	uint16 heightVal = MADS_SCREEN_HEIGHT - MADS_INTERFACE_HEIGHT;
	byte *walkMap = new byte[source->size()];

	create(widthVal, heightVal);
	source->read(walkMap, source->size());

	byte *ptr = (byte *)getBasePtr(0, 0);

	for (int y = 0; y < heightVal; y++) {
		for (int x = 0; x < widthVal; x++) {
			int ofs = x + (y * widthVal);
			if ((walkMap[ofs / 8] << (ofs % 8)) & 0x80)
				*ptr++ = 1;		// walkable
			else
				*ptr++ = 0;
		}
	}
}

void MSurfaceMADS::loadBackground(int roomNumber, RGBList **palData) {
	// clear previous data
	empty();

	// Get a MadsPack reference to the tile set and mapping
	char resourceName[20];
	int i;

	// Uncompressed tile map resource
	sprintf(resourceName, "rm%d.mm", roomNumber);
	MadsPack tileMapFile(resourceName, _vm);
	Common::SeekableReadStream *mapStream = tileMapFile.getItemStream(0);

	// Get the details of the tiles and map
	mapStream->readUint32LE();
	int tileCountX = mapStream->readUint16LE();
	int tileCountY = mapStream->readUint16LE();
	int tileWidthMap = mapStream->readUint16LE();
	int tileHeightMap = mapStream->readUint16LE();
	int screenWidth = mapStream->readUint16LE();
	int screenHeight = mapStream->readUint16LE();
	int tileCountMap = tileCountX * tileCountY;
	delete mapStream;

	// Obtain tile map information
	typedef Common::List<Common::SharedPtr<MSurface> > TileSetList;
	typedef TileSetList::iterator TileSetIterator;
	TileSetList tileSet;
	uint16 *tileMap = new uint16[tileCountMap];
	mapStream = tileMapFile.getItemStream(1);
	for (i = 0; i < tileCountMap; ++i)
		tileMap[i] = mapStream->readUint16LE();
	delete mapStream;

	_vm->_resources->toss(resourceName);

	// --------------------------------------------------------------------------------

	// Tile map data, which needs to be kept compressed, as the tile offsets refer to
	// the compressed data. Each tile is then uncompressed separately
	sprintf(resourceName, "rm%d.tt", roomNumber);
	Common::SeekableReadStream *tileDataComp = _vm->_resources->get(resourceName);
	MadsPack tileData(tileDataComp);
	Common::SeekableReadStream *tileDataUncomp = tileData.getItemStream(0);

	// Validate that the data matches between the tiles and tile map file and is valid
	int tileCount = tileDataUncomp->readUint16LE();
	int tileWidth = tileDataUncomp->readUint16LE();
	int tileHeight = tileDataUncomp->readUint16LE();
	delete tileDataUncomp;
	assert(tileCountMap == tileCount);
	assert(tileWidth == tileWidthMap);
	assert(tileHeight == tileHeightMap);
	assert(screenWidth == _vm->_screen->width());
	assert(screenHeight <= _vm->_screen->height());

	// --------------------------------------------------------------------------------

	// Get the palette to use
	tileDataUncomp = tileData.getItemStream(2);
	// Set palette
	if (!palData) {
		_vm->_palette->setMadsPalette(tileDataUncomp, 4);
	} else {
		int numColors;
		RGB8 *rgbList = _vm->_palette->decodeMadsPalette(tileDataUncomp, &numColors);
		*palData = new RGBList(numColors, rgbList, true);
	}
	delete tileDataUncomp;

	// --------------------------------------------------------------------------------

	// Get tile data

	tileDataUncomp = tileData.getItemStream(1);
	FabDecompressor fab;
	uint32 compressedTileDataSize = 0;

	for (i = 0; i < tileCount; i++) {
		tileDataUncomp->seek(i * 4, SEEK_SET);
		uint32 tileOfs = tileDataUncomp->readUint32LE();
		MSurface *newTile = MSurface::init(tileWidth, tileHeight);

		if (i == tileCount - 1)
			compressedTileDataSize = tileDataComp->size() - tileOfs;
		else
			compressedTileDataSize = tileDataUncomp->readUint32LE() - tileOfs;

		//printf("Tile: %i, compressed size: %i\n", i, compressedTileDataSize);

		newTile->empty();

		byte *compressedTileData = new byte[compressedTileDataSize];

		tileDataComp->seek(tileData.getDataOffset() + tileOfs, SEEK_SET);
		tileDataComp->read(compressedTileData, compressedTileDataSize);

		fab.decompress(compressedTileData, compressedTileDataSize, newTile->getData(), 
			tileWidth * tileHeight);
		tileSet.push_back(TileSetList::value_type(newTile));
		delete[] compressedTileData;
	}

	delete tileDataUncomp;

	// --------------------------------------------------------------------------------

	// Loop through the mapping data to place the tiles on the screen
	
	uint16 *tIndex = &tileMap[0];
	for (int y = 0; y < tileCountY; y++) {
		for (int x = 0; x < tileCountX; x++) {
			int tileIndex = *tIndex++;
			assert(tileIndex < tileCount);
			TileSetIterator tile = tileSet.begin();
			for (i = 0; i < tileIndex; i++)
				++tile;
			((*tile).get())->copyTo(this, x * tileWidth, y * tileHeight);
		}
	}
	tileSet.clear();
	_vm->_resources->toss(resourceName);
}

void MSurfaceMADS::loadInterface(int index, RGBList **palData) {
	char resourceName[20];
	sprintf(resourceName, "i%d.int", index);
	MadsPack intFile(resourceName, _vm);
	RGB8 *palette = new RGB8[16];

	// Chunk 0, palette
	Common::SeekableReadStream *intStream = intFile.getItemStream(0);
	
	for (int i = 0; i < 16; i++) {
		palette[i].r = intStream->readByte() << 2;
		palette[i].g = intStream->readByte() << 2;
		palette[i].b = intStream->readByte() << 2;
		intStream->readByte();
		intStream->readByte();
		intStream->readByte();
	}
	*palData = new RGBList(16, palette, true);
	delete intStream;

	// Chunk 1, data
	intStream = intFile.getItemStream(1);
	create(MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);
	intStream->read(pixels, MADS_SCREEN_WIDTH * MADS_INTERFACE_HEIGHT);
	delete intStream;
}

/*------------------------------------------------------------------------*/

void MSurfaceNebular::loadBackground(int roomNumber, RGBList **palData) {
	// clear previous data
	empty();

	Common::String resourceName = Common::String::format("rm%d.art", roomNumber);
	Common::SeekableReadStream *stream = _vm->_resources->get(resourceName);	
	loadBackgroundStream(stream, palData);

	_vm->_resources->toss(resourceName);
}

void MSurfaceNebular::loadBackgroundStream(Common::SeekableReadStream *source, RGBList **palData) {
	MadsPack packData(source);
	Common::MemoryReadStream *sourceUnc = packData.getItemStream(0);

	int sceneWidth = sourceUnc->readUint16LE();
	int sceneHeight = sourceUnc->readUint16LE();
	int sceneSize = sceneWidth * sceneHeight;
	if (sceneWidth > this->width()) {
		warning("Background width is %i, too large to fit in screen. Setting it to %i", sceneWidth, this->width());
		sceneWidth = this->width();
		sceneSize = sceneWidth * sceneHeight;
	}
	if (sceneHeight > this->height()) {
		warning("Background height is %i, too large to fit in screen.Setting it to %i", sceneHeight, this->height());
		sceneHeight = this->height();
		sceneSize = sceneWidth * sceneHeight;
	}

	// Set palette
	if (!palData) {
		_vm->_palette->setMadsPalette(sourceUnc, 4);
	} else {
		int numColors;
		RGB8 *rgbList = _vm->_palette->decodeMadsPalette(sourceUnc, &numColors);
		*palData = new RGBList(numColors, rgbList, true);
	}
	delete sourceUnc;

	// Get the raw data for the background
	sourceUnc = packData.getItemStream(1);
	assert((int)sourceUnc->size() >= sceneSize);

	byte *pData = (byte *)pixels;
	sourceUnc->read(pData, sceneSize);
	
	freeData();
	delete sourceUnc;
}

/*------------------------------------------------------------------------*/

void MSurfaceM4::loadCodes(Common::SeekableReadStream *source) {
	if (!source) {
		free();
		return;
	}

	uint16 widthVal = source->readUint16LE();
	uint16 heightVal = source->readUint16LE();

	create(widthVal, heightVal);
	source->read(pixels, widthVal * heightVal);
}

void MSurfaceM4::loadBackground(int roomNumber, RGBList **palData) {
	if (palData)
		*palData = NULL;
	Common::String resourceName = Common::String::format("%i.tt", roomNumber);
	Common::SeekableReadStream *stream = _vm->_resources->get(resourceName);	
	loadBackgroundStream(stream);

	_vm->_resources->toss(resourceName);
}

void MSurfaceM4::loadBackgroundStream(Common::SeekableReadStream *source) {
	MSurface *tileBuffer = MSurface::init();
	uint curTileX = 0, curTileY = 0;
	int clipX = 0, clipY = 0;
	RGB8 palette[256];

	source->skip(4);
	/*uint32 size =*/ source->readUint32LE();
	uint32 widthVal = source->readUint32LE();
	uint32 heightVal = source->readUint32LE();
	uint32 tilesX = source->readUint32LE();
	uint32 tilesY = source->readUint32LE();
	uint32 tileWidth = source->readUint32LE();
	uint32 tileHeight = source->readUint32LE();
	uint8 blackIndex = 0;

	// BGR data, which is converted to RGB8
	for (uint i = 0; i < 256; i++) {
		palette[i].b = source->readByte() << 2;
		palette[i].g = source->readByte() << 2;
		palette[i].r = source->readByte() << 2;
		palette[i].u = source->readByte() << 2;

		if ((blackIndex == 0) && !palette[i].r && !palette[i].g && !palette[i].b)
			blackIndex = i;
	}
	
	_vm->_palette->setPalette(palette, 0, 256);

	// resize or create the surface
	// Note that the height of the scene in game scenes is smaller than the screen height, 
	// as the bottom part of the screen is the inventory
	assert(width() == (int)widthVal);

	tileBuffer->create(tileWidth, tileHeight);

	for (curTileY = 0; curTileY < tilesY; curTileY++) {
		clipY = MIN(heightVal, (1 + curTileY) * tileHeight) - (curTileY * tileHeight);

		for (curTileX = 0; curTileX < tilesX; curTileX++) {
			clipX = MIN(widthVal, (1 + curTileX) * tileWidth) - (curTileX * tileWidth);

			// Read a tile and copy it to the destination surface
			source->read(tileBuffer->getData(), tileWidth * tileHeight);
			Common::Rect srcBounds(0, 0, clipX, clipY);
			copyFrom(tileBuffer, srcBounds, curTileX * tileWidth, curTileY * tileHeight);
		}
	}

	if (heightVal < (uint)height())
		fillRect(Common::Rect(0, heightVal, width(), height()), blackIndex);

	delete tileBuffer;
}

/*------------------------------------------------------------------------*/

void MSurfaceRiddle::loadBackground(const Common::String &sceneName) {
	char resourceName[20];
	Common::SeekableReadStream *stream;
	// Loads a Riddle scene
	Common::String resName = Common::String::format("%s.tt", sceneName.c_str());
	stream = _vm->_resources->get(resourceName);

	loadBackgroundStream(stream);

	_vm->_resources->toss(resourceName);
}

} // End of namespace MADS
