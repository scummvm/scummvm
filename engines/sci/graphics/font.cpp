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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/font.h"

namespace Sci {

GfxFontFromResource::GfxFontFromResource(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId)
	: _resourceId(resourceId), _screen(screen), _resMan(resMan) {
	assert(resourceId != -1);

	// Workaround: lsl1sci mixes its own internal fonts with the global
	// SCI ones, so we translate them here, by removing their extra bits
	if (!resMan->testResource(ResourceId(kResourceTypeFont, resourceId)))
		resourceId = resourceId & 0x7ff;

	_resource = resMan->findResource(ResourceId(kResourceTypeFont, resourceId), true);
	if (!_resource) {
		error("font resource %d not found", resourceId);
	}

	_numChars = _resource->getUint16SE32At(2);
	_fontHeight = _resource->getUint16SE32At(4);
	_chars = new Charinfo[_numChars];
	// filling info for every char
	for (int16 i = 0; i < _numChars; i++) {
		_chars[i].offset = _resource->getUint16SE32At(6 + i * 2);
		_chars[i].width = _resource->getUint8At(_chars[i].offset);
		_chars[i].height = _resource->getUint8At(_chars[i].offset + 1);
	}
}

GfxFontFromResource::~GfxFontFromResource() {
	delete[] _chars;
	_resMan->unlockResource(_resource);
}

GuiResourceId GfxFontFromResource::getResourceId() {
	return _resourceId;
}

uint8 GfxFontFromResource::getHeight() {
	return _fontHeight;
}
uint8 GfxFontFromResource::getCharWidth(uint16 chr) {
	return chr < _numChars ? _chars[chr].width : 0;
}
uint8 GfxFontFromResource::getCharHeight(uint16 chr) {
	return chr < _numChars ? _chars[chr].height : 0;
}
SciSpan<const byte> GfxFontFromResource::getCharData(uint16 chr) {
	if (chr >= _numChars) {
		return SciSpan<const byte>();
	}

	const uint32 size = (chr + 1 >= _numChars ? _resource->size() : _chars[chr + 1].offset) - _chars[chr].offset - 2;
	return _resource->subspan(_chars[chr].offset + 2, size);
}

void GfxFontFromResource::draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {
	if (chr >= _numChars) {
		// SSCI silently ignores attempts to draw characters that do not exist
		// in the font; for now, emit warnings if this happens, to learn if
		// it leads to any bugs
		warning("%s is missing glyph %d", _resource->name().c_str(), chr);
		return;
	}

	// Make sure we're comparing against the correct dimensions
	// If the font we're drawing is already upscaled, make sure we use the full screen width/height
	uint16 screenWidth = _screen->fontIsUpscaled() ? _screen->getDisplayWidth() : _screen->getWidth();
	uint16 screenHeight = _screen->fontIsUpscaled() ? _screen->getDisplayHeight() : _screen->getHeight();

	int charWidth = MIN<int>(getCharWidth(chr), screenWidth - left);
	int charHeight = MIN<int>(getCharHeight(chr), screenHeight - top);
	byte b = 0, mask = 0xFF;
	int y = 0;
	int16 greyedTop = top;

	SciSpan<const byte> charData = getCharData(chr);
	for (int i = 0; i < charHeight; i++, y++) {
		if (greyedOutput)
			mask = ((greyedTop++) % 2) ? 0xAA : 0x55;
		for (int done = 0; done < charWidth; done++) {
			if ((done & 7) == 0) // fetching next data byte
				b = *(charData++) & mask;
			if (b & 0x80) // if MSB is set - paint it
				_screen->putFontPixel(top, left + done, y, color);
			b = b << 1;
		}
	}
}

#ifdef ENABLE_SCI32

void GfxFontFromResource::drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 bufWidth, int16 bufHeight) {
	if (chr >= _numChars) {
		// SSCI silently ignores attempts to draw characters that do not exist
		// in the font; for now, emit warnings if this happens, to learn if
		// it leads to any bugs
		warning("%s is missing glyph %d", _resource->name().c_str(), chr);
		return;
	}

	int charWidth = MIN<int>(getCharWidth(chr), bufWidth - left);
	int charHeight = MIN<int>(getCharHeight(chr), bufHeight - top);
	byte b = 0, mask = 0xFF;
	int y = 0;
	int16 greyedTop = top;

	SciSpan<const byte> charData = getCharData(chr);
	for (int i = 0; i < charHeight; i++, y++) {
		if (greyedOutput)
			mask = ((greyedTop++) % 2) ? 0xAA : 0x55;
		for (int done = 0; done < charWidth; done++) {
			if ((done & 7) == 0) // fetching next data byte
				b = *(charData++) & mask;
			if (b & 0x80) {	// if MSB is set - paint it
				int offset = (top + y) * bufWidth + (left + done);
				buffer[offset] = color;
			}
			b = b << 1;
		}
	}
}

#endif

} // End of namespace Sci
