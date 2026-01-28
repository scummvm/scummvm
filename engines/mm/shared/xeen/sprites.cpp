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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"
#include "image/bmp.h"
#include "image/png.h"
#include "mm/shared/xeen/sprites.h"
#include "mm/mm.h"

namespace MM {
namespace Shared {
namespace Xeen {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)
#define SCENE_CLIP_LEFT 8
#define SCENE_CLIP_RIGHT 223

int SpriteResource::_clippedBottom;

SpriteResource::SpriteResource() {
	_filesize = 0;
	_data = nullptr;
}

SpriteResource::SpriteResource(const Common::Path &filename) {
	_data = nullptr;
	load(filename);
}

SpriteResource::SpriteResource(const SpriteResource &src) {
	copy(src);
}

SpriteResource::~SpriteResource() {
	clear();
}

void SpriteResource::copy(const SpriteResource &src) {
	_filesize = src._filesize;
	_data = new byte[_filesize];
	Common::copy(src._data, src._data + _filesize, _data);

	_index.resize(src._index.size());
	for (uint i = 0; i < src._index.size(); ++i)
		_index[i] = src._index[i];
}

SpriteResource &SpriteResource::operator=(const SpriteResource &src) {
	delete[] _data;
	_index.clear();

	copy(src);

	return *this;
}

void SpriteResource::load(const Common::Path &filename) {
	_filename = filename;
	Common::File f;
	if (g_engine->getGameID() == GType_MightAndMagic1 && f.open(filename)) {
		load(f);
	} else {
		File f2(filename);
		load(f2);
	}
}

void SpriteResource::load(Common::SeekableReadStream &f) {
	// Read in a copy of the file
	_filesize = f.size();
	delete[] _data;
	_data = new byte[_filesize];
	f.read(_data, _filesize);

	// Read in the index
	f.seek(0);
	int count = f.readUint16LE();
	_index.resize(count);

	for (int i = 0; i < count; ++i) {
		_index[i]._offset1 = f.readUint16LE();
		_index[i]._offset2 = f.readUint16LE();

		// Check for any images overriding the default Xeen sprites for M&M1 Enhanced mode
		const Common::String fnPNG = Common::String::format("gfx/%s/image%.2d.png", _filename.baseName().c_str(), i);
		const Common::String fnBitmap = Common::String::format("gfx/%s/image%.2d.bmp", _filename.baseName().c_str(), i);
		Image::PNGDecoder pngDecoder;
		Image::BitmapDecoder bmpDecoder;
		Common::File imgFile;

		if (Common::File::exists(fnPNG.c_str()) && imgFile.open(fnPNG.c_str()) && pngDecoder.loadStream(imgFile)) {
			_index[i]._override.copyFrom(*pngDecoder.getSurface());
		} else if (Common::File::exists(fnBitmap.c_str()) && imgFile.open(fnBitmap.c_str()) && bmpDecoder.loadStream(imgFile)) {
			_index[i]._override.copyFrom(*bmpDecoder.getSurface());
		}
	}
}

void SpriteResource::clear() {
	delete[] _data;
	_data = nullptr;
	_filesize = 0;
	_index.clear();
}

static inline uint getScaledVal(int xy, uint16 &scaleMask)
{
	if (!xy)
		return 0;

	uint result = 0;
	for (int idx = 0; idx < xy; ++idx) {
		uint bit = (scaleMask >> 15) & 1;
		scaleMask = ((scaleMask & 0x7fff) << 1) + bit;
		result += bit;
	}

	return result;
}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos,
		uint flags, int scale) const {
	draw(dest, frame, destPos, Common::Rect(0, 0, dest.w, dest.h), flags, scale);
}

static inline void DrawerPixel2(byte *dest, const byte pixel, byte *destLeft, byte *destRight, byte *destTop, byte *destBottom, int pitch, uint16 &random1, uint16 &random2, uint16 mask1, uint16 mask2)
{
	static const int8 DRAWER2_DELTA[64] = {
		-3, 3, 0, 0, 0, 0, 0, 0,
		-5, 5, 0, 0, 0, 0, 0, 0,
		-7, 7, 0, 0, 0, 0, 0, 0,
		-9, 9, 0, 0, 0, 0, 0, 0,
		-7, 7, 0, 0, 0, 0, 0, 0,
		-9, 9, 0, 0, 0, 0, 0, 0,
		-11, 11, 0, 0, 0, 0, 0, 0,
		-13, 13, 0, 0, 0, 0, 0, 0
	};

	bool flag = (random1 & 0x8000) != 0;
	random1 = (int)((uint16)random1 << 1) - random2 - (flag ? 1 : 0);
	
	bool newCf = (random2 & 1);
	random2 = (random2>>1) | (flag ? 0x8000 : 0);
	flag = newCf;

	newCf = (random2 & 1);
	random2 = (random2>>1) | (flag ? 0x8000 : 0);
	flag = newCf;

	random2 ^= random1;

	dest += DRAWER2_DELTA[(random2 & mask1 & mask2) / 2];
	if (dest >= destLeft && dest < destRight) {
		dest += pitch * DRAWER2_DELTA[((random2 >> 8) & mask1 & mask2) / 2];

		if (dest >= destTop && dest < destBottom) {
			*dest = pixel;
		}
	}
}

static inline void DrawerPixel3(byte *dest, const byte pixel, const uint16 mask, const uint16 offset, byte *palette)
{
	// WORKAROUND: This is slightly different then the original:
	// 1) The original has bunches of black pixels appearing. This does index increments to avoid such pixels
	// 2) It also prevents any pixels being drawn in the single initial frame until the palette is set
	const byte level = (pixel & mask) - offset + (*dest & 0xf);

	if (level >= 0x80) {
		*dest &= 0xf0;
	} else if (level <= 0xf) {
		*dest = (*dest & 0xf0) | level;
	} else {
		*dest |= 0xf;
	}

	//
	while (*dest < 0xff && !palette[*dest * 3] && !palette[*dest * 3 + 1] && !palette[*dest * 3 + 2])
	++ *dest;
}

static inline void DrawerPixel5(byte *dest, const byte pixel, const uint16 threshold, uint16 &random1, uint16 &random2)
{
	bool flag = (random1 & 0x8000) != 0;
	random1 = (int)((uint16)random1 << 1) - random2 - (flag ? 1 : 0);

	bool newCf = (random2 & 1);
	random2 = (random2>>1) | (flag ? 0x8000 : 0);
	flag = newCf;

	newCf = (random2 & 1);
	random2 = (random2>>1) | (flag ? 0x8000 : 0);
	flag = newCf;

	random2 ^= random1;

	if (random2 > threshold)
		*dest = pixel;
}

static inline void SpriteRender(byte *data, size_t filesize, XSurface &dest, uint16 offset, const Common::Point &pt, const Common::Rect &clipRect, uint flags, int scale)
{
	// regular drawer
	const SpriteFlags drawerFlag = static_cast<SpriteFlags>(flags & SPRFLAG_MODE_MASK);

	byte *destTop = nullptr, *destBottom = nullptr;
	byte *destLeft = nullptr, *destRight = nullptr;
	int pitch = 0;
	
	static const uint SCALE_TABLE[] = {
		0xFFFF, 0xFFEF, 0xEFEF, 0xEFEE, 0xEEEE, 0xEEAE, 0xAEAE, 0xAEAA,
		0xAAAA, 0xAA8A, 0x8A8A, 0x8A88, 0x8888, 0x8880, 0x8080, 0x8000
	};

	assert((scale & SCALE_MASK) < 16);
	uint16 scaleMask = SCALE_TABLE[scale & SCALE_MASK];
	uint16 scaleMaskX = scaleMask, scaleMaskY = scaleMask;
	const bool flipped = (flags & SPRFLAG_HORIZ_FLIPPED) != 0;
	int xInc = flipped ? -1 : 1;
	const bool enlarge = (scale & SCALE_ENLARGE) != 0;

	destTop = (byte *)dest.getBasePtr(clipRect.left, clipRect.top);
	destBottom = (byte *)dest.getBasePtr(clipRect.right, clipRect.bottom - 1);
	pitch = dest.pitch;

	// Get cell header
	Common::MemoryReadStream f(data, filesize);
	f.seek(offset);
	int xOffset = f.readUint16LE();
	int width = f.readUint16LE();
	int yOffset = f.readUint16LE();
	int height = f.readUint16LE();

	// Figure out drawing x, y
	Common::Point destPos;
	destPos.x = pt.x + getScaledVal(xOffset, scaleMaskX);
	destPos.x += (width - getScaledVal(width, scaleMaskX)) / 2;

	destPos.y = pt.y + getScaledVal(yOffset, scaleMaskY);

	// If the flags allow the dest surface to be resized, ensure dest surface is big enough
	Common::Rect bounds = clipRect;
	if (flags & SPRFLAG_RESIZE) {
		if (dest.w < (xOffset + width) || dest.h < (yOffset + height))
			dest.create(xOffset + width, yOffset + height);
		bounds = Common::Rect(0, 0, dest.w, dest.h);
	}
	if (flags & SPRFLAG_SCENE_CLIPPED) {
		bounds.clip(Common::Rect(8, 8, 223, 141));
	}

	uint16 scaleMaskXCopy = scaleMaskX;
	Common::Rect drawBounds;
	drawBounds.left = SCREEN_WIDTH;
	drawBounds.top = SCREEN_HEIGHT;
	drawBounds.right = drawBounds.bottom = 0;

	// Main loop
	for (int yCtr = height; yCtr > 0; --yCtr) {
		// The number of bytes in this scan line
		int lineLength = f.readByte();

		if (lineLength == 0) {
			// Skip the specified number of scan lines
			int numLines = f.readByte();
			destPos.y += getScaledVal(numLines + 1, scaleMaskY);
			yCtr -= numLines;
			continue;
		}

		// Roll the scale mask
		uint bit = (scaleMaskY >> 15) & 1;
		scaleMaskY = ((scaleMaskY & 0x7fff) << 1) + bit;

		if (!bit) {
			// Not a line to be drawn due to scaling down
			f.skip(lineLength);
		} else if (destPos.y < bounds.top || destPos.y >= bounds.bottom) {
			// Skip over the bytes of the line
			f.skip(lineLength);
			destPos.y++;
		} else {
			scaleMaskX = scaleMaskXCopy;
			xOffset = f.readByte();

			// Initialize the array to hold the temporary data for the line. We do this to make it simpler
			// to handle both deciding which pixels to draw in a scaled image, as well as when images
			// have been horizontally flipped. Note that we allocate an extra line for before and after our
			// work line, just in case the sprite is screwed up and overruns the line
			int tempLine[SCREEN_WIDTH * 3];
			Common::fill(&tempLine[SCREEN_WIDTH], &tempLine[SCREEN_WIDTH * 3], -1);
			int *lineP = flipped ? &tempLine[SCREEN_WIDTH + width - 1 - xOffset] : &tempLine[SCREEN_WIDTH + xOffset];

			// Build up the line
			int byteCount, opr1, opr2;
			int32 pos;
			for (byteCount = 1; byteCount < lineLength; ) {
				// The next byte is an opcode that determines what operators are to follow and how to interpret them.
				int opcode = f.readByte(); ++byteCount;

				// Decode the opcode
				int len = opcode & 0x1F;
				int cmd = (opcode & 0xE0) >> 5;

				switch (cmd) {
				case 0:   // The following len + 1 bytes are stored as indexes into the color table.
				case 1:   // The following len + 33 bytes are stored as indexes into the color table.
					for (int i = 0; i < opcode + 1; ++i, ++byteCount) {
						byte b = f.readByte();
						*lineP = b;
						lineP += xInc;
					}
					break;

				case 2:   // The following byte is an index into the color table, draw it len + 3 times.
					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i) {
						*lineP = opr1;
						lineP += xInc;
					}
					break;

				case 3:   // Stream copy command.
					opr1 = f.readUint16LE(); byteCount += 2;
					pos = f.pos();
					f.seek(-opr1, SEEK_CUR);

					for (int i = 0; i < len + 4; ++i) {
						*lineP = f.readByte();
						lineP += xInc;
					}

					f.seek(pos, SEEK_SET);
					break;

				case 4:   // The following two bytes are indexes into the color table, draw the pair len + 2 times.
					opr1 = f.readByte(); ++byteCount;
					opr2 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 2; ++i) {
						*lineP = opr1;
						lineP += xInc;
						*lineP = opr2;
						lineP += xInc;
					}
					break;

				case 5:   // Skip len + 1 pixels
					lineP += (len + 1) * xInc;
					break;

				case 6:   // Pattern command.
				case 7: {

					// The pattern command has a different opcode format
					static const int PATTERN_STEPS[] = {0, 1, 1, 1, 2, 2, 3, 3, 0, -1, -1, -1, -2, -2, -3, -3};

					len = opcode & 0x07;
					cmd = (opcode >> 2) & 0x0E;

					opr1 = f.readByte();
					++byteCount;
					for (int i = 0; i < len + 3; ++i) {
						*lineP = opr1;
						lineP += xInc;
						opr1 += PATTERN_STEPS[cmd + (i % 2)];
					}
				} break;
				default:
					break;
				}
			}
			assert(byteCount == lineLength);

			drawBounds.top = MIN(drawBounds.top, destPos.y);
			drawBounds.bottom = MAX((int)drawBounds.bottom, destPos.y + 1);

			// Handle drawing out the line
			byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y);
			destLeft = (byte *)dest.getBasePtr(
				(flags & SPRFLAG_SCENE_CLIPPED) ? SCENE_CLIP_LEFT : clipRect.left, destPos.y);
			destRight = (byte *)dest.getBasePtr(
				(flags & SPRFLAG_SCENE_CLIPPED) ? SCENE_CLIP_RIGHT : clipRect.right, destPos.y);
			int16 xp = destPos.x;
			lineP = &tempLine[SCREEN_WIDTH];

			const byte idx =  flags & 0x1F;

			for (int xCtr = 0; xCtr < width; ++xCtr, ++lineP)
			{
				bit = (scaleMaskX >> 15) & 1;
				scaleMaskX = ((scaleMaskX & 0x7fff) << 1) + bit;

				if (bit)
				{
					// Check whether there's a pixel to write, and we're within the allowable bounds. Note that for
					// the SPRFLAG_SCENE_CLIPPED or when enlarging, we also have an extra horizontal bounds check
					if (*lineP != -1 && xp >= bounds.left && xp < bounds.right)
					{
						drawBounds.left = MIN(drawBounds.left, xp);
						drawBounds.right = MAX((int)drawBounds.right, xp + 1);

						switch (drawerFlag)
						{
							case SPRFLAG_DRAWER1:
							{
								static const byte DRAWER1_OFFSET[24] = {
									0x30, 0xC0, 0xB0, 0x10, 0x41, 0x20, 0x40, 0x21, 0x48, 0x46, 0x43, 0x40,
									0xD0, 0xD3, 0xD6, 0xD8, 0x01, 0x04, 0x07, 0x0A, 0xEA, 0xEE, 0xF2, 0xF6
								};

								static const byte DRAWER1_MASK[24] = {
									0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x0F, 0x07, 0x07, 0x07, 0x07,
									0x07, 0x07, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x07, 0x07
								};

								const byte offset1 = DRAWER1_OFFSET[idx];
								const byte mask = DRAWER1_MASK[idx];
								const byte pixel = (((byte)*lineP) & mask) + offset1;

								(*destP) = pixel;

								if (enlarge)
								{
									*(destP + SCREEN_WIDTH) = pixel;
									*(destP + 1) = pixel;
									*(destP + 1 + SCREEN_WIDTH) = pixel;
								}	

							}break;
							case SPRFLAG_DRAWER2:
							{
								// drawer 2
   								static const byte DRAWER2_MASK1[32] = {
									3, 0, 3, 0, 3, 0, 3, 0, 2, 0, 2, 0, 2, 0, 2, 0,
									1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
								};

								static const byte DRAWER2_MASK2[16] = {
									0x7E, 0x7E, 0x7E, 0x7E, 0x3E, 0x3E, 0x3E, 0x3E,
									0x1E, 0x1E, 0x1E, 0x1E, 0x0E, 0x0E, 0x0E, 0x0E
								};

								const uint16 mask21 = DRAWER2_MASK1[idx];
								const uint16 mask22 = DRAWER2_MASK2[idx];

								MM::MMEngine *engine = static_cast<MM::MMEngine *>(g_engine);
								assert(engine!=nullptr);

								uint16 random1 = engine->getRandomNumber(0xffff);
								uint16 random2 = engine->getRandomNumber(0xffff);
								const byte pixel = (byte)*lineP;

								DrawerPixel2(destP, pixel, destLeft, destRight, destTop, destBottom, pitch, random1, random2, mask21, mask22);
									
								if (enlarge)
								{
									DrawerPixel2(destP + SCREEN_WIDTH, pixel, destLeft, destRight, destTop, destBottom, pitch, random1, random2, mask21, mask22);
									DrawerPixel2(destP + 1, pixel, destLeft, destRight, destTop, destBottom, pitch, random1, random2, mask21, mask22);
									DrawerPixel2(destP + 1 + SCREEN_WIDTH, pixel, destLeft, destRight, destTop, destBottom, pitch, random1, random2, mask21, mask22);
								}	

							} break;
							case SPRFLAG_DRAWER3:
							{
								static const uint16 DRAWER3_MASK[4] = { 1, 3, 7, 15 };
								static const uint16 DRAWER3_OFFSET[4] = { 1, 2, 4, 8 };
	
								const uint16 offsetLocal = DRAWER3_OFFSET[idx];
								const uint16 mask = DRAWER3_MASK[idx];

								byte palette[256 * 3];
								g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);

								bool hasPalette = false;
								for (byte *pal = palette; pal < palette + PALETTE_SIZE && !hasPalette; ++pal)
								{
									hasPalette = *pal != 0;
								}

								// nothing to do if hasPalette is true
								if(hasPalette)
								{
									const byte pixel = (byte)*lineP; 
									DrawerPixel3(destP, pixel, mask, offsetLocal, palette);
									
									if (enlarge)
									{
										DrawerPixel3(destP + SCREEN_WIDTH, pixel, mask, offsetLocal, palette);
										DrawerPixel3(destP + 1, pixel,  mask, offsetLocal, palette);
										DrawerPixel3(destP + 1 + SCREEN_WIDTH, pixel, mask, offsetLocal, palette );
									}	
								}
							}break;
							case SPRFLAG_DRAWER4:
							{
								static const byte DRAWER4_THRESHOLD[4] = { 4, 7, 10, 13 };
								const byte threshold = DRAWER4_THRESHOLD[idx];
								const byte pixel = (byte)*lineP;

								if((pixel & 0xf) >= threshold)
								{
									*destP = pixel;
									
									if (enlarge)
									{
										*(destP + SCREEN_WIDTH) = pixel;
										*(destP + 1) = pixel;
										*(destP + 1 + SCREEN_WIDTH) = pixel;
									}	
								}
							} break;
							case SPRFLAG_DRAWER5:
							{
								static const uint16 DRAWER5_THRESHOLD[4] = { 0x3333, 0x6666, 0x999A, 0xCCCD };
								static const uint16 threshold = DRAWER5_THRESHOLD[idx];

								MM::MMEngine *engine = static_cast<MM::MMEngine *>(g_engine);
								uint16 random1 = engine->getRandomNumber(0xffff);
								uint16 random2 = engine->getRandomNumber(0xffff);

								const byte pixel = (byte)*lineP; 
								DrawerPixel5(destP, pixel, threshold, random1, random2 );
									
								if (enlarge)
								{
									DrawerPixel5(destP + SCREEN_WIDTH, pixel, threshold, random1, random2 );
									DrawerPixel5(destP + 1, pixel, threshold, random1, random2 );
									DrawerPixel5(destP + 1 + SCREEN_WIDTH, pixel, threshold, random1, random2 );
								}	
							}break;
							case SPRFLAG_DRAWER6:
							{
								static const byte DRAWER6_MASK[16] = { 1, 2, 4, 8, 1, 3, 7, 15, 8, 12, 14, 15, 1, 2, 1, 2 };
								const byte mask = DRAWER6_MASK[idx];
								const byte pixel = (byte)*lineP ^ mask; 
								
								*destP = pixel;

								if (enlarge)
								{
									*(destP + SCREEN_WIDTH) = pixel;
									*(destP + 1) = pixel;
									*(destP + 1 + SCREEN_WIDTH) = pixel;
								}	
							}break;
							default:
							{
								const byte pixel = (byte)*lineP; 
							 	*destP = pixel;

								if (enlarge)
								{
									*(destP + SCREEN_WIDTH) = pixel;
									*(destP + 1) = pixel; 
									*(destP + 1 + SCREEN_WIDTH) = pixel;
								}	
							}break;
						};
					}

					++xp;
					++destP;
					if (enlarge) {
						++destP;
						++xp;
					}
				}
			}

			++destPos.y;
			if (enlarge)
				++destPos.y;
		}
	}

	if (drawBounds.isValidRect()) {
		drawBounds.clip(Common::Rect(0, 0, dest.w, dest.h));
		if (!drawBounds.isEmpty())
			dest.addDirtyRect(drawBounds);
	}

}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos, const Common::Rect &bounds, uint flags, int scale) const 
{
	Common::Rect r = bounds;
	
	if (flags & SPRFLAG_BOTTOM_CLIPPED)
	{
		r.clip(SCREEN_WIDTH, _clippedBottom);
	}

	// WORKAROUND: Crash clicking Vertigo well in Clouds
	if (frame < (int)_index.size()) {
		if (!_index[frame]._override.empty()) {
			const Graphics::ManagedSurface &src = _index[frame]._override;
			if (flags & Shared::Xeen::SPRFLAG_RESIZE)
				dest.create(src.w, src.h);
			dest.blitFrom(src, destPos);
		}
		else
		{
			// Sprites can consist of separate background & foreground
			SpriteRender(_data, _filesize, dest, _index[frame]._offset1, destPos, r, flags, scale);

			if (_index[frame]._offset2)
				SpriteRender(_data, _filesize, dest, _index[frame]._offset2, destPos, r, flags, scale);
		}
	}
}

void SpriteResource::draw(XSurface &dest, int frame) const {
	draw(dest, frame, Common::Point());
}

void SpriteResource::draw(Graphics::ManagedSurface *dest, int frame, const Common::Point &destPos) const {
	XSurface tmp;
	tmp.w = dest->w;
	tmp.h = dest->h;
	tmp.pitch = dest->pitch;
	tmp.format = dest->format;
	tmp.setPixels(dest->getPixels());

	draw(tmp, frame, destPos);
}


Common::Point SpriteResource::getFrameSize(int frame) const {
	Common::MemoryReadStream f(_data, _filesize);
	Common::Point frameSize;

	for (int idx = 0; idx < (_index[frame]._offset2 ? 2 : 1); ++idx) {
		f.seek((idx == 0) ? _index[frame]._offset1 : _index[frame]._offset2);
		int xOffset = f.readUint16LE();
		int width = f.readUint16LE();
		int yOffset = f.readUint16LE();
		int height = f.readUint16LE();

		frameSize.x = MAX((int)frameSize.x, xOffset + width);
		frameSize.y = MAX((int)frameSize.y, yOffset + height);
	}

	return frameSize;
}

} // End of namespace Xeen
} // End of namespace Shared
} // End of namespace MM
