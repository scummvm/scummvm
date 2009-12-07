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

#include "common/file.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/util.h"
#include "common/ptr.h"

#include "m4/globals.h"
#include "m4/graphics.h"
#include "m4/sprite.h"
#include "m4/m4.h"
#include "m4/compression.h"

namespace M4 {

RGBList::RGBList(int numEntries, RGB8 *srcData, bool freeData) {
	_size = numEntries;
	assert(numEntries <= 256);

	if (srcData == NULL) {
		_data = new RGB8[numEntries];
		_freeData = true;
	} else {
		_data = srcData;
		_freeData = freeData;
	}

	_palIndexes = new byte[numEntries];
	Common::set_to(&_palIndexes[0], &_palIndexes[numEntries], 0);
}

RGBList::~RGBList() {
	if (_freeData)
		delete[] _data;
	delete[] _palIndexes;
}

//--------------------------------------------------------------------------

#define VGA_COLOR_TRANS(x) (x == 0x3f ? 255 : x << 2)

void M4Surface::loadCodesM4(Common::SeekableReadStream *source) {
	if (!source) {
		free();
		return;
	}

	uint16 widthVal = source->readUint16LE();
	uint16 heightVal = source->readUint16LE();

	create(widthVal, heightVal, 1);
	source->read(pixels, widthVal * heightVal);
}

void M4Surface::loadCodesMads(Common::SeekableReadStream *source) {
	if (!source) {
		free();
		return;
	}

	uint16 widthVal = 320;
	uint16 heightVal = 156;
	byte *walkMap = new byte[source->size()];

	create(widthVal, heightVal, 1);
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

// Sprite related methods

void M4Surface::vLine(int x, int y1, int y2) {
	Graphics::Surface::vLine(x, y1, y2, _color);
}

void M4Surface::hLine(int x1, int x2, int y) {
	Graphics::Surface::hLine(x1, y, x2, _color);
}

void M4Surface::vLineXor(int x, int y1, int y2) {
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

void M4Surface::hLineXor(int x1, int x2, int y) {
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

void M4Surface::line(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::drawLine(x1, y1, x2, y2, color);
}


void M4Surface::frameRect(int x1, int y1, int x2, int y2) {
	Graphics::Surface::frameRect(Common::Rect(x1, y1, x2, y2), _color);
}

void M4Surface::fillRect(int x1, int y1, int x2, int y2) {
	Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), _color);
}

void M4Surface::drawSprite(int x, int y, SpriteInfo &info, const Common::Rect &clipRect) {

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
	printf("M4Surface::drawSprite() info.width = %d; info.scaleX = %d; info.height = %d; info.scaleY = %d; scaledWidth = %d; scaledHeight = %d\n",
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

	//printf("M4Surface::drawSprite() width = %d; height = %d; scaledWidth = %d; scaledHeight = %d\n", info.width, info.height, scaledWidth, scaledHeight); fflush(stdout);

	// Check if sprite is inside the screen. If it's not, there's no need to draw it
	if (scaledWidth + x <= 0 || scaledHeight + y <= 0)	// check left and top (in case x,y are negative)
		return;
	if (scaledWidth <= 0 || scaledHeight <= 0)			// check right and bottom
		return;
	int heightAmt = scaledHeight;

	byte *src = info.sprite->getBasePtr();
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

void M4Surface::freeData() {
}

void M4Surface::clear() {
	Common::set_to((byte *)pixels, (byte *)pixels + w * h, _vm->_palette->BLACK);
}

void M4Surface::frameRect(const Common::Rect &r, uint8 color) {
	Graphics::Surface::frameRect(r, color);
}

void M4Surface::fillRect(const Common::Rect &r, uint8 color) {
	Graphics::Surface::fillRect(r, color);
}

void M4Surface::copyFrom(M4Surface *src, const Common::Rect &srcBounds, int destX, int destY,
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

	byte *data = src->getBasePtr();
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

void M4Surface::loadBackgroundRiddle(const char *sceneName) {
	char resourceName[20];
	Common::SeekableReadStream *stream;
	// Loads a Riddle scene
	sprintf(resourceName, "%s.tt", sceneName);
	stream = _vm->_resourceManager->get(resourceName);
	m4LoadBackground(stream);
	_vm->_resourceManager->toss(resourceName);
}

void M4Surface::loadBackground(int sceneNumber, RGBList **palData) {
	clear();		// clear previous scene

	if (_vm->isM4() || (_vm->getGameType() == GType_RexNebular)) {
		char resourceName[20];
		Common::SeekableReadStream *stream;

		if (_vm->getGameType() == GType_RexNebular) {
			// Load Rex Nebular screen
			sprintf(resourceName, "rm%d.art", sceneNumber);
			stream = _vm->_resourceManager->get(resourceName);
			rexLoadBackground(stream, palData);
		} else {
			// Loads M4 game scene
			if (palData)
				*palData = NULL;
			sprintf(resourceName, "%i.tt", sceneNumber);
			stream = _vm->_resourceManager->get(resourceName);
			m4LoadBackground(stream);
		}

		_vm->_resourceManager->toss(resourceName);

	} else {
		madsLoadBackground(sceneNumber, palData);
	}
}

void M4Surface::madsLoadBackground(int roomNumber, RGBList **palData) {
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
	typedef Common::List<Common::SharedPtr<M4Surface> > TileSetList;
	typedef TileSetList::iterator TileSetIterator;
	TileSetList tileSet;
	uint16 *tileMap = new uint16[tileCountMap];
	mapStream = tileMapFile.getItemStream(1);
	for (i = 0; i < tileCountMap; ++i)
		tileMap[i] = mapStream->readUint16LE();
	delete mapStream;
	_vm->res()->toss(resourceName);

	// --------------------------------------------------------------------------------

	// Tile map data, which needs to be kept compressed, as the tile offsets refer to
	// the compressed data. Each tile is then uncompressed separately
	sprintf(resourceName, "rm%d.tt", roomNumber);
	Common::SeekableReadStream *tileDataComp = _vm->_resourceManager->get(resourceName);
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
		M4Surface* newTile = new M4Surface(tileWidth, tileHeight);

		if (i == tileCount - 1)
			compressedTileDataSize = tileDataComp->size() - tileOfs;
		else
			compressedTileDataSize = tileDataUncomp->readUint32LE() - tileOfs;

		//printf("Tile: %i, compressed size: %i\n", i, compressedTileDataSize);

		newTile->clear();

		byte *compressedTileData = new byte[compressedTileDataSize];

		tileDataComp->seek(tileData.getDataOffset() + tileOfs, SEEK_SET);
		tileDataComp->read(compressedTileData, compressedTileDataSize);

		fab.decompress(compressedTileData, compressedTileDataSize, (byte*)newTile->pixels, tileWidth * tileHeight);
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
	_vm->res()->toss(resourceName);
}

void M4Surface::rexLoadBackground(Common::SeekableReadStream *source, RGBList **palData) {
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

#undef COL_TRANS

void M4Surface::m4LoadBackground(Common::SeekableReadStream *source) {
	M4Surface *tileBuffer = new M4Surface();
	uint curTileX = 0, curTileY = 0;
	int clipX = 0, clipY = 0;
	RGB8 palette[256];

	source->readUint32LE();			// magic, unused
	/*uint32 size =*/ source->readUint32LE();
	uint32 widthVal = source->readUint32LE();
	uint32 heightVal = source->readUint32LE();
	uint32 tilesX = source->readUint32LE();
	uint32 tilesY = source->readUint32LE();
	uint32 tileWidth = source->readUint32LE();
	uint32 tileHeight = source->readUint32LE();
	uint8 blackIndex = 0;

	// Debug
	//printf("loadBackground(): %dx%d picture (%d bytes) - %dx%d tiles of size %dx%d\n",
	//	   widthVal, heightVal, size, tilesX, tilesY, tileWidth, tileHeight);

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
	// note that the height of the scene in game scenes is smaller than 480, as the bottom part of the
	// screen is the inventory
	assert(width() == (int)widthVal);
	//printf("width(): %d, widthVal: %d, height(): %d, heightVal: %d\n", width(), widthVal, height(), heightVal);

	tileBuffer->create(tileWidth, tileHeight, 1);

	for (curTileY = 0; curTileY < tilesY; curTileY++) {
		clipY = MIN(heightVal, (1 + curTileY) * tileHeight) - (curTileY * tileHeight);

		for (curTileX = 0; curTileX < tilesX; curTileX++) {
			clipX = MIN(widthVal, (1 + curTileX) * tileWidth) - (curTileX * tileWidth);

			// Read a tile and copy it to the destination surface
			source->read(tileBuffer->pixels, tileWidth * tileHeight);
			Common::Rect srcBounds(0, 0, clipX, clipY);
			copyFrom(tileBuffer, srcBounds, curTileX * tileWidth, curTileY * tileHeight);
		}
	}

	if (heightVal < (uint)height())
		fillRect(Common::Rect(0, heightVal, width(), height()), blackIndex);

	delete tileBuffer;
}

void M4Surface::madsloadInterface(int index, RGBList **palData) {
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
	create(320, 44, 1);
	intStream->read(pixels, 320 * 44);
	delete intStream;
}

void M4Surface::translate(RGBList *list, bool isTransparent) {
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

//--------------------------------------------------------------------------
// Palette class
//

#define GREEN_START 32
#define NUM_GREENS 32
#define GREEN_END (GREEN_START + NUM_GREENS - 1)
#define NORMAL_START 64
#define NORMAL_END 255
#define NUM_NORMAL (NORMAL_END - NORMAL_START + 1)

// Support function for creating a list of palette indexes to change entries in the shaded range to

static void makeTranslationList(RGB8 *palData, byte transList[NUM_GREENS]) {
	int i, j, minDistance;
	byte bestIndex;

	for (i = 0; i < NUM_GREENS; ++i) {
		bestIndex = NORMAL_START;
		minDistance = 255;

		uint8 findCol = palData[GREEN_START + i].g;

		// Find the closest matching palette color
		for (j = NORMAL_START; j <= NORMAL_END; ++j) {
			int greenVal = palData[j].g;
			if (ABS(findCol - greenVal) < minDistance) {
				minDistance = ABS(findCol - greenVal);
				bestIndex = j;
			}

			if (minDistance == 0)
				break;
		}

		transList[i] = bestIndex;
	}
}

// Support function for fading in or out

static void fadeRange(M4Engine *vm, RGB8 *srcPal, RGB8 *destPal,  int startIndex, int endIndex,
					 int numSteps, uint delayAmount) {
	RGB8 tempPal[256];

	// perform the fade
	for (int stepCtr = 1; stepCtr <= numSteps; ++stepCtr) {
		// Delay the specified amount
		uint32 startTime = g_system->getMillis();
		while ((g_system->getMillis() - startTime) < delayAmount) {
			vm->_events->handleEvents();
			g_system->delayMillis(10);
		}

		for (int i = startIndex; i <= endIndex; ++i) {
			// Handle the intermediate rgb values for fading
			tempPal[i].r = (byte) (srcPal[i].r + (destPal[i].r - srcPal[i].r) * stepCtr / numSteps);
			tempPal[i].g = (byte) (srcPal[i].g + (destPal[i].g - srcPal[i].g) * stepCtr / numSteps);
			tempPal[i].b = (byte) (srcPal[i].b + (destPal[i].b - srcPal[i].b) * stepCtr / numSteps);
		}

		vm->_palette->setPalette(&tempPal[startIndex], startIndex, endIndex - startIndex + 1);
		vm->_viewManager->refreshAll();
	}

	// Make sure the end palette exactly matches what is wanted
	vm->_palette->setPalette(&destPal[startIndex], startIndex, endIndex - startIndex + 1);
}

Palette::Palette(M4Engine *vm) : _vm(vm) {
	reset();
	_fading_in_progress = false;
	Common::set_to(&_usageCount[0], &_usageCount[256], 0);
}

void Palette::setPalette(const byte *colors, uint start, uint num) {
	g_system->setPalette(colors, start, num);
	reset();
}

void Palette::setPalette(const RGB8 *colors, uint start, uint num) {
	g_system->setPalette((const byte *)colors, start, num);
	reset();
}

void Palette::grabPalette(byte *colors, uint start, uint num) {
	g_system->grabPalette(colors, start, num);
	reset();
}

uint8 Palette::palIndexFromRgb(byte r, byte g, byte b, RGB8 *paletteData) {
	byte index = 0;
	int32 minDist = 0x7fffffff;
	RGB8 palData[256];
	int Rdiff, Gdiff, Bdiff;

	if (paletteData == NULL) {
		g_system->grabPalette((byte *)palData, 0, 256);
		paletteData = &palData[0];
	}

	for (int palIndex = 0; palIndex < 256; ++palIndex) {
		Rdiff = r - paletteData[palIndex].r;
		Gdiff = g - paletteData[palIndex].g;
		Bdiff = b - paletteData[palIndex].b;

		if (Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff < minDist) {
			minDist = Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff;
			index = (uint8)palIndex;
		}
	}

	return (uint8)index;
}

void Palette::reset() {
	RGB8 palData[256];
	g_system->grabPalette((byte *)palData, 0, 256);

	BLACK = palIndexFromRgb(0, 0, 0, palData);
	BLUE = palIndexFromRgb(0, 0, 255, palData);
	GREEN = palIndexFromRgb(0, 255, 0, palData);
	CYAN = palIndexFromRgb(0, 255, 255, palData);
	RED = palIndexFromRgb(255, 0, 0, palData);
	VIOLET = palIndexFromRgb(255, 0, 255, palData);
	BROWN = palIndexFromRgb(168, 84, 84, palData);
	LIGHT_GRAY = palIndexFromRgb(168, 168, 168, palData);
	DARK_GRAY = palIndexFromRgb(84, 84, 84, palData);
	LIGHT_BLUE = palIndexFromRgb(0, 0, 127, palData);
	LIGHT_GREEN = palIndexFromRgb(0, 127, 0, palData);
	LIGHT_CYAN = palIndexFromRgb(0, 127, 127, palData);
	LIGHT_RED = palIndexFromRgb(84, 0, 0, palData);
	PINK = palIndexFromRgb(84, 0, 0, palData);
	YELLOW = palIndexFromRgb(0, 84, 84, palData);
	WHITE = palIndexFromRgb(255, 255, 255, palData);
}

void Palette::fadeToGreen(int numSteps, uint delayAmount) {
	if (_fading_in_progress)
		return;
	_fading_in_progress = true;
	byte translationList[NUM_GREENS];

	int i;
	byte *tempP;
	uint8 greenAmount = 0;
	RGB8 *srcPalette = (RGB8 *) &_originalPalette[0];
	RGB8 *destPalette = (RGB8 *) &_fadedPalette[0];

	_vm->_palette->grabPalette(srcPalette, 0, 256);

	// Create the destination 'greenish' palette to fade to by setting the green component
	// to the average of the RGB bytes, and leaving the Red and Blue parts as 0

	Common::copy(&srcPalette[0], &srcPalette[256], &destPalette[0]);
	for (i = 32; i < 256; ++i) {
		byte luminance = (byte)((destPalette[i].r + destPalette[i].g + destPalette[i].b) / 3);
		destPalette[i].g = MIN((byte)255, luminance);
		destPalette[i].r = destPalette[i].b = 0;
	}

	// Handle the actual fading
	fadeRange(_vm, srcPalette, destPalette, 21, 255, numSteps, delayAmount);

	// Create a translation table to be used in translating pixels in the game surface
	// using palette indexes in the range the range #32-63 into values from #64-255

	makeTranslationList(destPalette, translationList);

	// Use palette indexes from #32-63 for the range of possible shades

	for (i = GREEN_START; i <= GREEN_END; ++i, greenAmount += 8) {
		destPalette[i].g = greenAmount;
		destPalette[i].r = destPalette[i].b = 0;
	}

	// Remap all pixels into the #32-63 range

	tempP = _vm->_scene->getBasePtr();
	for (int pixelCtr = 0; pixelCtr < _vm->_scene->width() * _vm->_scene->height();
			++pixelCtr, ++tempP) {
		// If pixel is in #32-63 range already, remap to higher palette entries
		if ((*tempP >= GREEN_START) && (*tempP <= GREEN_END))
			*tempP = translationList[*tempP - GREEN_START];

		*tempP = (uint8) (GREEN_START + (destPalette[*tempP].g >> 3));
	}

	_vm->_palette->setPalette(&destPalette[GREEN_START], GREEN_START, NUM_GREENS);
	_vm->_viewManager->refreshAll();
	_fading_in_progress = false;
}

void Palette::fadeFromGreen(int numSteps, uint delayAmount, bool fadeToBlack) {
	if (_fading_in_progress)
		return;
	_fading_in_progress = true;
	RGB8 blackPalette[256];
	RGB8 *fadedPalette = (RGB8 *) &_fadedPalette[0];
	RGB8 *destPalette = (RGB8 *) &_originalPalette[0];

	if (fadeToBlack) {
		Common::set_to((byte *)&blackPalette[0], (byte *)&blackPalette[256], 0);
		destPalette = &blackPalette[0];
	}

	// Initially restore the faded palette
	_vm->_palette->setPalette(fadedPalette, 0, 256);
	_vm->_viewManager->refreshAll();

	// Restore the pixel data from the original screen
	_vm->_scene->update();

	// Handle the actual fading
	fadeRange(_vm, fadedPalette, destPalette, GREEN_START, NORMAL_END, numSteps, delayAmount);

	_fading_in_progress = false;
}

void Palette::fadeIn(int numSteps, uint delayAmount, RGBList *destPalette) {
	fadeIn(numSteps, delayAmount, destPalette->data(), destPalette->size());
}

void Palette::fadeIn(int numSteps, uint delayAmount, RGB8 *destPalette, int numColors) {
	if (_fading_in_progress)
		return;

	_fading_in_progress = true;
	RGB8 blackPalette[256];
	Common::set_to((byte *)&blackPalette[0], (byte *)&blackPalette[256], 0);

	// Initially set the black palette
	_vm->_palette->setPalette(blackPalette, 0, numColors);

	// Handle the actual fading
	fadeRange(_vm, blackPalette, destPalette, 0, numColors - 1, numSteps, delayAmount);

	_fading_in_progress = false;
}

RGB8 *Palette::decodeMadsPalette(Common::SeekableReadStream *palStream, int *numColors) {
	*numColors = palStream->readUint16LE();
	assert(*numColors <= 252);

	RGB8 *palData = new RGB8[*numColors];
	Common::set_to((byte *)&palData[0], (byte *)&palData[*numColors], 0);

	for (int i = 0; i < *numColors; ++i) {
		byte r = palStream->readByte();
		byte g = palStream->readByte();
		byte b = palStream->readByte();
		palData[i].r = VGA_COLOR_TRANS(r);
		palData[i].g = VGA_COLOR_TRANS(g);
		palData[i].b = VGA_COLOR_TRANS(b);

		// The next 3 bytes are unused
		palStream->skip(3);
	}

	return palData;
}

int Palette::setMadsPalette(Common::SeekableReadStream *palStream, int indexStart) {
	int colorCount;
	RGB8 *palData = Palette::decodeMadsPalette(palStream, &colorCount);
	_vm->_palette->setPalette(palData, indexStart, colorCount);
	delete palData;
	return colorCount;
}

void Palette::setMadsSystemPalette() {
	// Rex Nebular default system palette
	resetColorCounts();

	RGB8 palData[4];
	palData[0].r = palData[0].g = palData[0].b = 0;
	palData[1].r = palData[1].g = palData[1].b = 0x54;
	palData[2].r = palData[2].g = palData[2].b = 0xb4;
	palData[3].r = palData[3].g = palData[3].b = 0xff;

	setPalette(palData, 0, 4);
	blockRange(0, 4);
}

void Palette::resetColorCounts() {
	Common::set_to(&_usageCount[0], &_usageCount[256], 0);
}

void Palette::blockRange(int startIndex, int size) {
	// Use a reference count of -1 to signal a palette index shouldn't be used
	Common::set_to(&_usageCount[startIndex], &_usageCount[startIndex + size], -1);
}

void Palette::addRange(RGBList *list) {
	RGB8 *data = list->data();
	byte *palIndexes = list->palIndexes();
	RGB8 palData[256];
	g_system->grabPalette((byte *)&palData[0], 0, 256);
	bool paletteChanged = false;

	for (int colIndex = 0; colIndex < list->size(); ++colIndex) {
		// Scan through for an existing copy of the RGB value
		int palIndex = -1;
		while (++palIndex < 256) {
			if (_usageCount[palIndex] <= 0)
				// Palette index is to be skipped
				continue;

			if ((palData[palIndex].r == data[colIndex].r) &&
				(palData[palIndex].g == data[colIndex].g) &&
				(palData[palIndex].b == data[colIndex].b))
				// Match found
				break;
		}

		if (palIndex == 256) {
			// No match found, so find a free slot to use
			palIndex = -1;
			while (++palIndex < 256) {
				if (_usageCount[palIndex] == 0)
					break;
			}

			if (palIndex == 256)
				error("addRange - Ran out of palette space to allocate");

			palData[palIndex].r = data[colIndex].r;
			palData[palIndex].g = data[colIndex].g;
			palData[palIndex].b = data[colIndex].b;
			paletteChanged = true;
		}

		palIndexes[colIndex] = palIndex;
		++_usageCount[palIndex];
	}

	if (paletteChanged) {
		g_system->setPalette((byte *)&palData[0], 0, 256);
		reset();
	}
}

void Palette::deleteRange(RGBList *list) {
	// Release the reference count on each of the palette entries
	for (int colIndex = 0; colIndex < list->size(); ++colIndex) {
		int palIndex = list->palIndexes()[colIndex];
		assert(_usageCount[palIndex] > 0);
		--_usageCount[palIndex];
	}
}

void Palette::deleteAllRanges() {
	for (int colIndex = 0; colIndex < 255; ++colIndex)
		_usageCount[colIndex] = 0;
}

//--------------------------------------------------------------------------
// Support methods

void decompressRle(byte *rleData, int rleSize, byte *celData, int w, int h) {
	byte *src = rleData;
	byte *dst = celData;
	byte len;
	while (1) {
		len = *src++;
		if (len == 0) {
			len = *src++;
			if (len <= 2) {
				if (len == 1) // end of sprite marker
					break;
			} else {
				while (len--)
					*dst++ = *src++;
			}
		} else {
			while (len--)
				*dst++ = *src;
			*src++;
		}
	}
}

int scaleValue(int value, int scale, int err) {
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

} // End of namespace M4
