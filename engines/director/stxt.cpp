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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/substream.h"

#include "director/director.h"
#include "director/cast.h"
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
	Common::String text = textStream.readString(0, strLen);
	debugC(3, kDebugText, "Stxt init: offset: %d strLen: %d dataLen: %d textlen: %u", offset, strLen, dataLen, text.size());

	uint16 formattingCount = textStream.readUint16();
	uint32 prevPos = 0;

	debugC(3, kDebugText, "Stxt init: formattingCount: %u", formattingCount);

	Common::U32String logText;

	while (formattingCount) {
		uint16 currentFont = _style.fontId;
		_style.read(textStream, _cast);

		assert(prevPos <= _style.formatStartOffset);  // If this is triggered, we have to implement sorting

		Common::String textPart;
		while (prevPos != _style.formatStartOffset) {
			char f = text.firstChar();
			textPart += f;
			text.deleteChar(0);

			if (f == '\001')	// Insert two \001s as a replacement
				_ftext += '\001';

			prevPos++;
		}
		Common::CodePage encoding = detectFontEncoding(cast->_platform, currentFont);
		Common::U32String u32TextPart(textPart, encoding);
		_ptext += u32TextPart;
		_ftext += u32TextPart;
		logText += Common::toPrintable(u32TextPart);

		Common::String format = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", _style.fontId, _style.textSlant, _style.fontSize, _style.r, _style.g, _style.b);
		_ftext += format;
		logText += Common::toPrintable(format);

		formattingCount--;
	}

	Common::CodePage encoding = detectFontEncoding(cast->_platform, _style.fontId);
	Common::U32String u32Text(text, encoding);
	_ptext += u32Text;
	_ftext += u32Text;
	logText += Common::toPrintable(u32Text);

	debugC(4, kDebugText, "#### text:\n%s\n####", logText.encode(Common::kUtf8).c_str());
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

void FontStyle::read(Common::ReadStreamEndian &stream, Cast *cast) {
	formatStartOffset = stream.readUint32();
	uint16 originalHeight = height = stream.readUint16();
	ascent = stream.readUint16();

	uint16 originalFontId = fontId = stream.readUint16();
	textSlant = stream.readByte();
	stream.readByte(); // padding
	fontSize = stream.readUint16();

	r = stream.readUint16();
	g = stream.readUint16();
	b = stream.readUint16();

	if (cast->_fontMap.contains(originalFontId)) {
		FontMapEntry *info = cast->_fontMap[originalFontId];
		fontId = info->toFont;
		if (info->sizeMap.contains(originalHeight)) {
			height = info->sizeMap[height];
		}
	}

	debugC(3, kDebugLoading, "FontStyle::read(): formatStartOffset: %d, height: %d -> %d ascent: %d, fontId: %d -> %d, textSlant: %d, fontSize: %d, r: %x g: %x b: %x",
			formatStartOffset, originalHeight, height, ascent, originalFontId, fontId, textSlant, fontSize, r, g, b);
}

} // End of namespace Director
