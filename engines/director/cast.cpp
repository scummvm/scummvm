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
#include "director/cachedmactext.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/stxt.h"

namespace Director {

BitmapCast::BitmapCast(Common::ReadStreamEndian &stream, uint32 castTag, uint16 version) {
	if (version < 4) {
		_pitch = 0;
		_flags = stream.readByte();
		_bytes = stream.readUint16();
		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		if (_bytes & 0x8000) {
			_bitsPerPixel = stream.readUint16();
			_unk2 = stream.readUint16();
		} else {
			_bitsPerPixel = 1;
			_unk2 = 0;
		}

		_pitch = _initialRect.width();
		if (_pitch % 16)
			_pitch += 16 - (_initialRect.width() % 16);
	} else if (version == 4) {
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_flags = 0;
		_bytes = 0;
		_unk2 = 0;

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);
		_regX = stream.readUint16();
		_regY = stream.readUint16();

		_bitsPerPixel = stream.readUint16();
		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		int tail = 0;

		while (!stream.eos()) {
			stream.readByte();
			tail++;
		}

		warning("BitmapCast: %d bytes left", tail);
	} else if (version == 5) {
		_bytes = 0;
		_pitch = 0;
		uint16 count = stream.readUint16();
		for (uint16 cc = 0; cc < count; cc++)
			stream.readUint32();

		uint32 stringLength = stream.readUint32();
		for (uint32 s = 0; s < stringLength; s++)
			stream.readByte();

		/*uint16 width =*/ stream.readUint16LE(); //maybe?
		_initialRect = Score::readRect(stream);

		/*uint32 somethingElse =*/ stream.readUint32();
		_boundingRect = Score::readRect(stream);

		_bitsPerPixel = stream.readUint16();

		_regX = 0;
		_regY = 0;

		stream.readUint32();
	}
	_modified = 0;
	_tag = castTag;
}

TextCast::TextCast(Common::ReadStreamEndian &stream, uint16 version) {
	_borderSize = kSizeNone;
	_gutterSize = kSizeNone;
	_boxShadow = kSizeNone;

	_flags1 = 0;
	_fontId = 0;
	_fontSize = 12;
	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_textSlant = 0;
	_palinfo1 = _palinfo2 = _palinfo3 = 0;

	if (version <= 3) {
		_flags1 = stream.readByte();
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readUint16());
		_palinfo1 = stream.readUint16();
		_palinfo2 = stream.readUint16();
		_palinfo3 = stream.readUint16();

		if (version == 2) {
			int t = stream.readUint16();
			if (t != 0) { // In D2 there are values
				warning("TextCast: t: %x", t);
			}

			_initialRect = Score::readRect(stream);
			stream.readUint16();
		} else {
			int t = stream.readUint32();
			if (t != 0) { // In D2 there are values
				warning("TextCast: t: %x", t);
			}

			_initialRect = Score::readRect(stream);
		}

		_textShadow = static_cast<SizeType>(stream.readByte());
		byte flags = stream.readByte();
		if (flags & 0x1)
			_textFlags.push_back(kTextFlagEditable);
		if (flags & 0x2)
			_textFlags.push_back(kTextFlagAutoTab);
		if (flags & 0x4)
			_textFlags.push_back(kTextFlagDoNotWrap);
		if (flags & 0xf8)
			warning("Unprocessed text cast flags: %x", flags & 0xf8);

		// TODO: FIXME: guesswork
		_fontId = stream.readByte();
		_fontSize = stream.readByte();
		_textSlant = 0;
	} else if (version == 4) {
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();

		_fontId = 1; // this is in STXT

		_initialRect = Score::readRect(stream);
		stream.readUint16();
		_textShadow = static_cast<SizeType>(stream.readByte());
		byte flags = stream.readByte();

		if (flags)
			warning("Unprocessed text cast flags: %x", flags);

		_fontSize = stream.readUint16();
		_textSlant = 0;
	} else {
		_fontId = 1;
		_fontSize = 12;

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

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);

		stream.readUint32();
		stream.readUint16();
		stream.readUint16();
	}

	_modified = 0;

	_cachedMacText = new CachedMacText(this, version, -1, g_director->_wm);
	// TODO Destroy me
}

void TextCast::importStxt(const Stxt *stxt) {
	_fontId = stxt->_fontId;
	_textSlant = stxt->_textSlant;
	_fontSize = stxt->_fontSize;
	_palinfo1 = stxt->_palinfo1;
	_palinfo2 = stxt->_palinfo2;
	_palinfo3 = stxt->_palinfo3;
	_ftext = stxt->_ftext;
	_ptext = stxt->_ptext;

	_cachedMacText->setStxt(this);
}

void TextCast::importRTE(byte *text) 	{
	//assert(rteList.size() == 3);
	//child0 is probably font data.
	//child1 is the raw text.
	_ptext = _ftext = Common::String((char*)text);
	//child2 is positional?
}

void TextCast::setText(const char *text) {
	// Do nothing if text did not change
	if (_ftext.equals(text))
		return;

	_ptext = _ftext = text;

	_cachedMacText->forceDirty();
}

ShapeCast::ShapeCast(Common::ReadStreamEndian &stream, uint16 version) {
	byte flags, unk1;

	if (version < 4) {
		flags = stream.readByte();
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Score::readRect(stream);
		_pattern = stream.readUint16BE();
		_fgCol = (stream.readByte() + 128) & 0xff; // -128 -> 0, 127 -> 256
		_bgCol = (stream.readByte() + 128) & 0xff;
		_fillType = stream.readByte();
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else {
		flags = stream.readByte();
		unk1 = stream.readByte();

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);

		_shapeType = kShapeRectangle;
		_pattern = 0;
		_fgCol = _bgCol = 0;
		_fillType = 0;
		_lineThickness = 1;
		_lineDirection = 0;
	}
	_modified = 0;

	debugC(3, kDebugLoading, "ShapeCast: fl: %x unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		flags, unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCast: rect:");
}

ButtonCast::ButtonCast(Common::ReadStreamEndian &stream, uint16 version) : TextCast(stream, version) {
	if (version < 4) {
		_buttonType = static_cast<ButtonType>(stream.readUint16BE());
	} else {
		stream.readByte();
		stream.readByte();

		// This has already been populated in the super TextCast constructor
		//initialRect = Score::readRect(stream);
		//boundingRect = Score::readRect(stream);

		_buttonType = static_cast<ButtonType>(stream.readUint16BE());
	}
	_modified = 0;
}

ScriptCast::ScriptCast(Common::ReadStreamEndian &stream, uint16 version) {
	if (version < 4) {
		error("Unhandled Script cast");
	} else if (version == 4) {
		stream.readByte();
		stream.readByte();

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);

		_id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d", _id);

		stream.readByte(); // There should be no more data
		assert(stream.eos());
	} else if (version > 4) {
		stream.readByte();
		stream.readByte();

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);

		_id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d", _id);

		// WIP need to complete this!
	}
	_modified = 0;
}

} // End of namespace Director
