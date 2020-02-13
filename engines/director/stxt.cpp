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

#include "common/substream.h"

#include "director/director.h"
#include "director/stxt.h"

namespace Director {

Stxt::Stxt(Common::SeekableSubReadStreamEndian &textStream) {
	// TODO: Side effects on textStream make this a little hard to understand in context?
	uint32 unk1 = textStream.readUint32();
	uint32 strLen = textStream.readUint32();
	uint32 dataLen = textStream.readUint32();
	Common::String text;

	for (uint32 i = 0; i < strLen; i++) {
		byte ch = textStream.readByte();
		if (ch == 0x0d) {
			ch = '\n';
		}
		text += ch;
	}
	debugC(3, kDebugText, "Stxt init: unk1: %d strLen: %d dataLen: %d textlen: %u", unk1, strLen, dataLen, text.size());
	if (strLen < 200)
		debugC(3, kDebugText, "text: '%s'", Common::toPrintable(text).c_str());

	_ptext = text;

	uint16 formattingCount = textStream.readUint16();
	uint32 prevPos = 0;

	while (formattingCount) {
		uint32 formatStartOffset = textStream.readUint32();
		uint16 height = textStream.readUint16();
		uint16 ascent = textStream.readUint16();

		_fontId = textStream.readUint16();
		_textSlant = textStream.readByte();
		byte padding = textStream.readByte();
		_fontSize = textStream.readUint16();

		_palinfo1 = textStream.readUint16();
		_palinfo2 = textStream.readUint16();
		_palinfo3 = textStream.readUint16();

		debugC(3, kDebugText, "Stxt init: formattingCount: %u, formatStartOffset: %d, height: %d ascent: %d, fontId: %d, textSlant: %d padding: 0x%02x",
			   formattingCount, formatStartOffset, height, ascent, _fontId, _textSlant, padding);

		debugC(3, kDebugText, "        fontSize: %d, p0: %x p1: %x p2: %x", _fontSize, _palinfo1, _palinfo2, _palinfo3);

		assert(prevPos <= formatStartOffset);  // If this is triggered, we have to implement sorting

		while (prevPos != formatStartOffset) {
			char f = text.firstChar();
			_ftext += text.firstChar();
			text.deleteChar(0);

			if (f == '\001')    // Insert two \001s as a replacement
				_ftext += '\001';

			prevPos++;

			debugCN(4, kDebugText, "%c", f);
		}

		debugCN(4, kDebugText, "*");

		_ftext += Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", _fontId, _textSlant, _fontSize, _palinfo1, _palinfo2, _palinfo3);

		formattingCount--;
	}

	_ftext += text;

	debugC(4, kDebugText, "#### text:\n%s\n####", Common::toPrintable(_ftext).c_str());
}

} // End of namespace Director
