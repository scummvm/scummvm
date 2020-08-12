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

Stxt::Stxt(Cast *cast, Common::SeekableReadStreamEndian &textStream) : _cast(cast) {
	// TODO: Side effects on textStream make this a little hard to understand in context?

	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_unk1f = _unk2f = 0;
	_unk3f = 0;
	_size = textStream.size();

	// D4+ variant
	if (textStream.size() == 0)
		return;

	uint32 offset = textStream.readUint32();
	if (offset != 12) {
		error("Stxt init: unhandlef offset");
		return;
	}
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
	debugC(3, kDebugText, "Stxt init: offset: %d strLen: %d dataLen: %d textlen: %u", offset, strLen, dataLen, text.size());
	if (strLen < 200)
		debugC(3, kDebugText, "text: '%s'", Common::toPrintable(text).c_str());

	_ptext = text;

	uint16 formattingCount = textStream.readUint16();
	uint32 prevPos = 0;

	while (formattingCount) {
		debugC(3, kDebugText, "Stxt init: formattingCount: %u", formattingCount);
		_style.read(textStream);

		assert(prevPos <= _style.formatStartOffset);  // If this is triggered, we have to implement sorting

		while (prevPos != _style.formatStartOffset) {
			char f = text.firstChar();
			_ftext += text.firstChar();
			text.deleteChar(0);

			if (f == '\001')	// Insert two \001s as a replacement
				_ftext += '\001';

			prevPos++;

			debugCN(4, kDebugText, "%c", f);
		}

		debugCN(4, kDebugText, "*");

		_ftext += Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", _style.fontId, _style.textSlant, _style.fontSize, _style.r, _style.g, _style.b);

		formattingCount--;
	}

	_ftext += text;

	debugC(4, kDebugText, "#### text:\n%s\n####", Common::toPrintable(_ftext).c_str());
}

FontStyle::FontStyle() {
	formatStartOffset = 0;
	height = 0;
	ascent = 0;

	fontId = 0;
	textSlant = 0;

	fontSize = 12;

	r = g = b = 0;
}

void FontStyle::read(Common::ReadStreamEndian &stream) {
	formatStartOffset = stream.readUint32();
	height = stream.readUint16();
	ascent = stream.readUint16();

	fontId = stream.readUint16();
	textSlant = stream.readByte();
	stream.readByte(); // padding
	fontSize = stream.readUint16();

	r = stream.readUint16();
	g = stream.readUint16();
	b = stream.readUint16();

	debugC(3, kDebugLoading, "FontStyle::read(): formatStartOffset: %d, height: %d ascent: %d, fontId: %d, textSlant: %d, fontSize: %d, r: %x g: %x b: %x",
			formatStartOffset, height, ascent, fontId, textSlant, fontSize, r, g, b);
}

} // End of namespace Director
