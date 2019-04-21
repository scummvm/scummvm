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

#include "common/rect.h"
#include "common/system.h"

#include "chewy/resource.h"
#include "chewy/text.h"

namespace Chewy {

Text::Text() : Resource("atds.tap") {
}

Text::~Text() {
}

TextEntryList *Text::getDialog(uint dialogNum, uint entryNum) {
	if (dialogNum >= kADSTextMax)
		error("getDialog(): Invalid entry number requested, %d (max %d)", dialogNum, kADSTextMax - 1);

	TextEntryList *l = new TextEntryList();

	byte *data = getChunkData(dialogNum);
	byte *ptr = data;

	ptr += 2;	// entry number
	ptr += 2;	// number of persons
	ptr += 2;	// automove count
	ptr += 2;	// cursor number
	ptr += 13;	// misc data

	for (uint i = 0; i <= entryNum; i++) {
		do {
			TextEntry curDialog;
			ptr++;	// current entry
			ptr += 2;
			curDialog.speechId = READ_LE_UINT16(ptr) - VOICE_OFFSET;	ptr += 2;

			do {
				curDialog.text += *ptr++;

				if (*ptr == 0 && *(ptr + 1) != kEndText) {
					// TODO: Split lines
					*ptr = ' ';
				}
			} while (*ptr != kEndText);

			if (i == entryNum)
				l->push_back(curDialog);

		} while (*(ptr + 1) != kEndEntry);

		ptr += 2;	// kEndText, kEndEntry

		if (*ptr == kEndBlock)	// not found
			break;
	}

	delete[] data;

	return l;
}

TextEntry *Text::getText(uint dialogNum, uint entryNum) {
	if (dialogNum < kADSTextMax)
		error("getText(): Invalid entry number requested, %d (min %d)", dialogNum, kADSTextMax);

	TextEntry *d = new TextEntry();
	bool isText = (dialogNum >= kADSTextMax && dialogNum < kADSTextMax + kATSTextMax);
	bool isAutoDialog = (dialogNum >= kADSTextMax + kATSTextMax && dialogNum < kADSTextMax + kATSTextMax + kAADTextMax);
	//bool isInvText = (dialogNum >= kADSTextMax + kATSTextMax + kAADTextMax && dialogNum < kADSTextMax + kATSTextMax + kAADTextMax + kINVTextMax);

	byte *data = getChunkData(dialogNum);
	byte *ptr = data;

	if (isAutoDialog)
		ptr += 3;

	for (uint i = 0; i <= entryNum; i++) {
		ptr += 13;
		d->speechId = READ_LE_UINT16(ptr) - VOICE_OFFSET;	ptr += 2;

		do {
			if (i == entryNum)
				d->text += *ptr++;
			else
				ptr++;

			if (*ptr == 0 && *(ptr + 1) != kEndText) {
				// TODO: Split lines
				*ptr = ' ';
			}
		} while (*ptr);

		if (*(ptr + 1) != kEndText || *(ptr + 2) != kEndChunk) {
			warning("Invalid text resource - %d, %d", dialogNum, entryNum);

			delete[] data;
			delete d;

			return nullptr;
		}

		if (!isText)
			ptr += 3;	// 0, kEndText, kEndChunk
		if (isAutoDialog)
			ptr += 3;

		if (i == entryNum) {
			// Found
			delete[] data;
			return d;
		}
	}

	// Not found
	delete[] data;
	delete d;

	return nullptr;
}

Font::Font(Common::String filename) {
	const uint32 headerFont = MKTAG('T', 'F', 'F', '\0');
	Common::File stream;

	stream.open(filename);

	uint32 header = stream.readUint32BE();

	if (header != headerFont)
		error("Invalid resource - %s", filename.c_str());

	stream.skip(4);	// total memory
	_count = stream.readUint16LE();
	_first = stream.readUint16LE();
	_last = stream.readUint16LE();
	_width = stream.readUint16LE();
	_height = stream.readUint16LE();

	_fontSurface.create(_width * _count, _height, ::Graphics::PixelFormat::createFormatCLUT8());

	byte cur;
	int bitIndex = 7;
	byte *p;

	cur = stream.readByte();

	for (uint n = 0; n < _count; n++) {
		for (uint y = 0; y < _height; y++) {
			p = (byte *)_fontSurface.getBasePtr(n * _width, y);

			for (uint x = n * _width; x < n * _width + _width; x++) {
				*p++ = (cur & (1 << bitIndex)) ? 0 : 0xFF;

				bitIndex--;
				if (bitIndex < 0) {
					bitIndex = 7;
					cur = stream.readByte();
				}
			}
		}
	}
}

Font::~Font() {
	_fontSurface.free();
}

::Graphics::Surface *Font::getLine(const Common::String &text) {
	::Graphics::Surface *line = new ::Graphics::Surface();
	line->create(text.size() * _width, _height, ::Graphics::PixelFormat::createFormatCLUT8());

	for (uint i = 0; i < text.size(); i++) {
		uint x = (text[i] - _first) * _width;
		line->copyRectToSurface(_fontSurface, i * _width, 0, Common::Rect(x, 0, x + _width, _height));
	}

	return line;
}

Common::String ErrorMessage::getErrorMessage(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	Common::String str;
	byte *data = new byte[chunk->size];

	_stream.seek(chunk->pos, SEEK_SET);
	_stream.read(data, chunk->size);
	if (_encrypted)
		decrypt(data, chunk->size);

	str = (char *)data;
	delete[] data;

	return str;
}

} // End of namespace Chewy
