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

#include "director/director.h"
#include "director/cachedmactext.h"
#include "director/cast.h"
#include "director/score.h"

namespace Director {

BitmapCast::BitmapCast(Common::ReadStreamEndian &stream, uint32 castTag, uint16 version) {
	if (version < 4) {
		flags = stream.readByte();
		someFlaggyThing = stream.readUint16();
		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);
		regY = stream.readUint16();
		regX = stream.readUint16();
		unk1 = unk2 = 0;

		if (someFlaggyThing & 0x8000) {
			unk1 = stream.readUint16();
			unk2 = stream.readUint16();
		}
	} else if (version == 4) {
		stream.readByte();
		stream.readByte();

		flags = 0;
		someFlaggyThing = 0;
		unk1 = unk2 = 0;

		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);
		regX = stream.readUint16();
		regY = stream.readUint16();

		bitsPerPixel = stream.readUint16();
		if (bitsPerPixel == 0)
			bitsPerPixel = 1;

		int tail = 0;

		while (!stream.eos()) {
			stream.readByte();
			tail++;
		}

		warning("BitmapCast: %d bytes left", tail);
	} else if (version == 5) {
		uint16 count = stream.readUint16();
		for (uint16 cc = 0; cc < count; cc++)
			stream.readUint32();

		uint32 stringLength = stream.readUint32();
		for (uint32 s = 0; s < stringLength; s++)
			stream.readByte();

		/*uint16 width =*/ stream.readUint16LE(); //maybe?
		initialRect = Score::readRect(stream);

		/*uint32 somethingElse =*/ stream.readUint32();
		boundingRect = Score::readRect(stream);

		bitsPerPixel = stream.readUint16();

		regX = 0;
		regY = 0;

		stream.readUint32();
	}
	modified = 0;
	tag = castTag;
}

TextCast::TextCast(Common::ReadStreamEndian &stream, uint16 version) {
	borderSize = kSizeNone;
	gutterSize = kSizeNone;
	boxShadow = kSizeNone;

	flags1 = 0;
	fontId = 0;
	fontSize = 12;
	textType = kTextTypeFixed;
	textAlign = kTextAlignLeft;
	textShadow = kSizeNone;
	textSlant = 0;
	palinfo1 = palinfo2 = palinfo3 = 0;

	if (version <= 3) {
		flags1 = stream.readByte();
		borderSize = static_cast<SizeType>(stream.readByte());
		gutterSize = static_cast<SizeType>(stream.readByte());
		boxShadow = static_cast<SizeType>(stream.readByte());
		textType = static_cast<TextType>(stream.readByte());
		textAlign = static_cast<TextAlignType>(stream.readUint16());
		palinfo1 = stream.readUint16();
		palinfo2 = stream.readUint16();
		palinfo3 = stream.readUint16();

		if (version == 2) {
			int t = stream.readUint16();
			if (t != 0) { // In D2 there are values
				warning("TextCast: t: %x", t);
			}

			initialRect = Score::readRect(stream);
			stream.readUint16();
		} else {
			int t = stream.readUint32();
			if (t != 0) { // In D2 there are values
				warning("TextCast: t: %x", t);
			}

			initialRect = Score::readRect(stream); 
		}

		textShadow = static_cast<SizeType>(stream.readByte());
		byte flags = stream.readByte();
		if (flags & 0x1)
			textFlags.push_back(kTextFlagEditable);
		if (flags & 0x2)
			textFlags.push_back(kTextFlagAutoTab);
		if (flags & 0x4)
			textFlags.push_back(kTextFlagDoNotWrap);
		if (flags & 0xf8)
			warning("Unprocessed text cast flags: %x", flags & 0xf8);

		// TODO: FIXME: guesswork
		fontId = stream.readByte();
		fontSize = stream.readByte();
		textSlant = 0;
	} else if (version == 4) {
		borderSize = static_cast<SizeType>(stream.readByte());
		gutterSize = static_cast<SizeType>(stream.readByte());
		boxShadow = static_cast<SizeType>(stream.readByte());
		textType = static_cast<TextType>(stream.readByte());
		textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();

		fontId = 1; // this is in STXT

		initialRect = Score::readRect(stream);
		stream.readUint16();
		textShadow = static_cast<SizeType>(stream.readByte());
		byte flags = stream.readByte();

		if (flags)
			warning("Unprocessed text cast flags: %x", flags);

		fontSize = stream.readUint16();
		textSlant = 0;
	} else {
		fontId = 1;
		fontSize = 12;

		stream.readUint32();
		stream.readUint32(); 
		stream.readUint32();
		stream.readUint32();
		uint16 skip = stream.readUint16();
		for (int i = 0; i < skip; i++) 
			stream.readUint32();

		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();

		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);

		stream.readUint32();
		stream.readUint16();
		stream.readUint16();
	}

	modified = 0;

	cachedMacText = new CachedMacText(this, version);
	// TODO Destroy me
}

void TextCast::importStxt(const Stxt *stxt) {
	fontId = stxt->_fontId;
	textSlant = stxt->_textSlant;
	fontSize = stxt->_fontSize;
	palinfo1 = stxt->_palinfo1;
	palinfo2 = stxt->_palinfo2;
	palinfo3 = stxt->_palinfo3;
	_ftext = stxt->_ftext;
}

void TextCast::importRTE(byte* text) 	{
	//assert(rteList.size() == 3);
	//child0 is probably font data.
	//child1 is the raw text.
	_ftext = Common::String((char*)text);
	//child2 is positional?
}

ShapeCast::ShapeCast(Common::ReadStreamEndian &stream, uint16 version) {
	if (version < 4) {
		/*byte flags = */ stream.readByte();
		/*unk1 = */ stream.readByte();
		shapeType = static_cast<ShapeType>(stream.readByte());
		initialRect = Score::readRect(stream);
		pattern = stream.readUint16BE();
		fgCol = stream.readByte();
		bgCol = stream.readByte();
		fillType = stream.readByte();
		lineThickness = stream.readByte();
		lineDirection = stream.readByte();
	} else {
		stream.readByte();
		stream.readByte();

		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);

		shapeType = kShapeRectangle;
		pattern = 0;
		fgCol = bgCol = 0;
		fillType = 0;
		lineThickness = 1;
		lineDirection = 0;
	}
	modified = 0;
}

ButtonCast::ButtonCast(Common::ReadStreamEndian &stream, uint16 version) : TextCast(stream, version) {
	if (version < 4) {
		buttonType = static_cast<ButtonType>(stream.readUint16BE());
	} else {
		stream.readByte();
		stream.readByte();

		// This has already been populated in the super TextCast constructor
		//initialRect = Score::readRect(stream);
		//boundingRect = Score::readRect(stream);

		buttonType = static_cast<ButtonType>(stream.readUint16BE());
	}
	modified = 0;
}

ScriptCast::ScriptCast(Common::ReadStreamEndian &stream, uint16 version) {
	if (version < 4) {
		error("Unhandled Script cast");
	} else if (version == 4) {
		stream.readByte();
		stream.readByte();

		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);

		id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d", id);

		stream.readByte(); // There should be no more data
		assert(stream.eos());
	} else if (version > 4) {
		stream.readByte();
		stream.readByte();

		initialRect = Score::readRect(stream);
		boundingRect = Score::readRect(stream);

		id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d", id);

		// WIP need to complete this!
	}
	modified = 0;
}

} // End of namespace Director
