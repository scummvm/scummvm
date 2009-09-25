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

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/sprite.h"
#include "draci/font.h"

#include <cmath>

namespace Draci {

/**
 *  @brief Transforms an image from column-wise to row-wise
 *	@param img pointer to the buffer containing the image data
 *		   width width of the image in the buffer
 *		   height height of the image in the buffer
 */
static void transformToRows(byte *img, uint16 width, uint16 height) {
	byte *buf = new byte[width * height];
	byte *tmp = buf;
	memcpy(buf, img, width * height);
	
	for (uint16 i = 0; i < width; ++i) {
		for (uint16 j = 0; j < height; ++j) {
			img[j * width + i] = *tmp++;
		}
	}
	
	delete[] buf;
}

/**
 *  Constructor for loading sprites from a raw data buffer, one byte per pixel.
 */
Sprite::Sprite(const byte *raw_data, uint16 width, uint16 height, int x, int y, 
	bool columnwise) : _data(NULL) {

	 _width = width;
	 _height = height;

	_scaledWidth = _width;
	_scaledHeight = _height;

	 _x = x;
	 _y = y;

	_delay = 0;

	_mirror = false;

	byte *data = new byte[width * height];
	
	memcpy(data, raw_data, width * height);

	// If the sprite is stored column-wise, transform it to row-wise
	if (columnwise) {
		transformToRows(data, width, height);
	}	
	_data = data;
}

/**
 *  Constructor for loading sprites from a sprite-formatted buffer, one byte per 
 *	pixel.
 */
Sprite::Sprite(const byte *sprite_data, uint16 length, int x, int y, bool columnwise) 
	: _data(NULL) {

	 _x = x;
	 _y = y;

	_delay = 0;

	_mirror = false;	

	Common::MemoryReadStream reader(sprite_data, length);

	_width = reader.readSint16LE();
	_height = reader.readSint16LE();

	_scaledWidth = _width;
	_scaledHeight = _height;

	byte *data = new byte[_width * _height];

	reader.read(data, _width * _height);

	// If the sprite is stored column-wise, transform it to row-wise
	if (columnwise) {
		transformToRows(data, _width, _height);
	}		
	_data = data;
}

Sprite::~Sprite() { 
	delete[] _data;
}

void Sprite::setMirrorOn() {
	_mirror = true;
}

void Sprite::setMirrorOff() {
	_mirror = false;
}


int Sprite::getPixel(int x, int y) const {
	
	Common::Rect rect = getRect();

	int dy = y - rect.top;
	int dx = x - rect.left;
	
	// Calculate scaling factors
	double scaleX = double(_scaledWidth) / _width;
	double scaleY = double(_scaledHeight) / _height;

	int sy = scummvm_lround(dy * scaleY);
	int sx = scummvm_lround(dx * scaleX);

	if (_mirror)
		return _data[sy * _width + (_width - sx)];
	else
		return _data[sy * _width + sx];
}


void Sprite::drawScaled(Surface *surface, bool markDirty) const {

	Common::Rect sourceRect(0, 0, _width, _height);
	Common::Rect destRect(_x, _y, _x + _scaledWidth, _y + _scaledHeight);
	Common::Rect surfaceRect(0, 0, surface->w, surface->h);
	Common::Rect clippedDestRect(destRect);

	clippedDestRect.clip(surfaceRect);

	// Calculate by how much we need to adjust the source rectangle to account for cropping
	const int adjustLeft = clippedDestRect.left - destRect.left;
	const int adjustRight = clippedDestRect.right - destRect.right;
	const int adjustTop = clippedDestRect.top - destRect.top;
	const int adjustBottom = clippedDestRect.bottom - destRect.bottom;

	// Resize source rectangle
	sourceRect.left += adjustLeft;
	sourceRect.right += adjustRight;
	sourceRect.top += adjustTop;
	sourceRect.bottom += adjustBottom;

	// Get pointers to source and destination buffers
	byte *dst = (byte *)surface->getBasePtr(clippedDestRect.left, clippedDestRect.top);
	const byte *src = _data;

	const int transparent = surface->getTransparentColour();

	// Calculate how many rows and columns we need to draw
	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	int *rowIndices = new int[rows];
	int *columnIndices = new int[columns];

	// Calculate scaling factors
	double scaleX = double(_scaledWidth) / _width;
	double scaleY = double(_scaledHeight) / _height;

	// Precalculate pixel indexes
	for (int i = 0; i < rows; ++i) {
		rowIndices[i] = scummvm_lround(i / scaleY);
	}

	for (int j = 0; j < columns; ++j) {
		columnIndices[j] = scummvm_lround(j / scaleX);
	}

	// Blit the sprite to the surface
	for (int i = 0; i < rows; ++i) {

		// Fetch index of current row to be drawn
		int row = rowIndices[i];
		
		for (int j = 0; j < columns; ++j) {
			
			// Fetch index of current column to be drawn
			int column = columnIndices[j];

			// Don't blit if the pixel is transparent on the target surface
			if (src[row * _width + column] != transparent) {

				// Draw the sprite mirrored if the _mirror flag is set						
				if (_mirror) {
					dst[sourceRect.left + columns - j - 1] = src[row * _width + column];
				} else {
					dst[sourceRect.left + j] = src[row * _width + column];
				}
			}
		}

		// Advance to next row
		dst += surface->pitch;
	}

	// Mark the sprite's rectangle dirty
	if (markDirty) {	
		surface->markDirtyRect(destRect);
	}

	delete[] rowIndices;
	delete[] columnIndices;
}
	

/**
 *  @brief Draws the sprite to a Draci::Surface
 *  @param surface Pointer to a Draci::Surface
 *
 *  Draws the sprite to a Draci::Surface and marks its rectangle on the surface as dirty.
 *  It is safe to call it for sprites that would overflow the surface.
 */
void Sprite::draw(Surface *surface, bool markDirty) const { 

	Common::Rect sourceRect(0, 0, _width, _height);
	Common::Rect destRect(_x, _y, _x + _width, _y + _height);
	Common::Rect surfaceRect(0, 0, surface->w, surface->h);
	Common::Rect clippedDestRect(destRect);

	clippedDestRect.clip(surfaceRect);

	// Calculate by how much we need to adjust the source rectangle to account for cropping
	const int adjustLeft = clippedDestRect.left - destRect.left;
	const int adjustRight = clippedDestRect.right - destRect.right;
	const int adjustTop = clippedDestRect.top - destRect.top;
 	const int adjustBottom = clippedDestRect.bottom - destRect.bottom;

	// Resize source rectangle
	sourceRect.left += adjustLeft;
	sourceRect.right += adjustRight;
	sourceRect.top += adjustTop;
	sourceRect.bottom += adjustBottom;

	// Get pointers to source and destination buffers
	byte *dst = (byte *)surface->getBasePtr(clippedDestRect.left, clippedDestRect.top);
	const byte *src = _data;

	const int transparent = surface->getTransparentColour();

	// Blit the sprite to the surface
	for (int i = sourceRect.top; i < sourceRect.bottom; ++i) {
		for (int j = sourceRect.left; j < sourceRect.right; ++j) {
			
			// Don't blit if the pixel is transparent on the target surface
			if (src[i * _width + j] != transparent) {

				// Draw the sprite mirrored if the _mirror flag is set						
				if (_mirror) {
					dst[sourceRect.right - j - 1] = src[i * _width + j];
				} else {	
					dst[j] = src[i * _width + j];
				}
			}		
		}

		// Advance to next row
		dst += surface->pitch;
	}

	// Mark the sprite's rectangle dirty
	if (markDirty) {	
		surface->markDirtyRect(destRect);
	}
}
	

Common::Rect Sprite::getRect(bool scaled) const {
	if (scaled) 
		return Common::Rect(_x, _y, _x + _scaledWidth, _y + _scaledHeight);
	else
		return Common::Rect(_x, _y, _x + _width, _y + _height);
}

Text::Text(const Common::String &str, Font *font, byte fontColour, 
				int x, int y, uint spacing) {
	_x = x;
	_y = y;
	_delay = 0;
	
	_text = str;

	_length = 0;
	for (uint i = 0; i < _text.size(); ++i) {
		if (_text[i] != '|') {
			++_length;
		}
	}
	
	_spacing = spacing;
	_colour = fontColour;
	
	_font = font;

	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getStringHeight(str);

	_scaledWidth = _width;
	_scaledHeight = _height;
} 

void Text::setText(const Common::String &str) {

	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getStringHeight(str);

	 _text = str;

	_length = 0;
	for (uint i = 0; i < _text.size(); ++i) {
		if (_text[i] != '|') {
			++_length;
		}
	}
}

void Text::setColour(byte fontColour) {
	_colour = fontColour;
}

void Text::setSpacing(uint spacing) {
	_spacing = spacing;
}

uint Text::getLength() const {
	return _length;
}

void Text::draw(Surface *surface, bool markDirty) const {
	_font->setColour(_colour);

	_font->drawString(surface, _text, _x, _y, _spacing);
}

// TODO: Handle scaled parameter properly by implementing Text scaling
Common::Rect Text::getRect(bool scaled) const {
	return Common::Rect(_x, _y, _x + _width, _y + _height);
}

void Text::setFont(Font *font) {
	_font = font;

	_width = _font->getStringWidth(_text, _spacing);
	_height = _font->getStringHeight(_text);
}
			
} // End of namespace Draci	
		
 
