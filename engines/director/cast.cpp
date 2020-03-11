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
#include "graphics/surface.h"

#include "director/director.h"
#include "director/cachedmactext.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/stxt.h"

namespace Director {

Cast::Cast() {
	_type = kCastTypeNull;
	_surface = nullptr;

	_modified = true;
}

Cast::~Cast() {
	delete _surface;
}

BitmapCast::BitmapCast(Common::ReadStreamEndian &stream, uint32 castTag, uint16 version) {
	_type = kCastBitmap;

	if (version < 4) {
		_pitch = 0;
		_flags = stream.readByte();	// region: 0 - auto, 1 - matte, 2 - disabled
		_bytes = stream.readUint16();
		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		if (_bytes & 0x8000) {
			_bitsPerPixel = stream.readUint16();
			_clut = stream.readUint16();
		} else {
			_bitsPerPixel = 1;
			_clut = 0;
		}

		_pitch = _initialRect.width();
		if (_pitch % 16)
			_pitch += 16 - (_initialRect.width() % 16);
	} else if (version == 4) {
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_flags = 0;
		_bytes = 0;
		_clut = 0;

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_bitsPerPixel = stream.readUint16();
		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		if (_bitsPerPixel == 1)
			_pitch *= 8;

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
		_clut = 0;

		stream.readUint32();
	}
	_tag = castTag;
}

TextCast::TextCast(Common::ReadStreamEndian &stream, uint16 version, int32 bgcolor) {
	_type = kCastText;

	_bgcolor = bgcolor;
	_borderSize = kSizeNone;
	_gutterSize = kSizeNone;
	_boxShadow = kSizeNone;

	_flags = 0;
	_textFlags = 0;
	_fontId = 0;
	_fontSize = 12;
	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_textSlant = 0;
	_palinfo1 = _palinfo2 = _palinfo3 = 0;

	if (version <= 3) {
		_flags = stream.readByte(); // region: 0 - auto, 1 - matte, 2 - disabled
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		byte pad1 = stream.readByte();
		_textAlign = static_cast<TextAlignType>(stream.readUint16());
		_palinfo1 = stream.readUint16();
		_palinfo2 = stream.readUint16();
		_palinfo3 = stream.readUint16();

		uint32 pad2;
		uint16 pad3;
		uint16 pad4 = 0;
		uint16 totalTextHeight;

		if (version == 2) {
			pad2 = stream.readUint16();
			if (pad2 != 0) { // In D2 there are values
				warning("TextCast: pad2: %x", pad2);
			}

			_initialRect = Score::readRect(stream);
			pad3 = stream.readUint16();

			_textShadow = static_cast<SizeType>(stream.readByte());
			_textFlags = stream.readByte();
			if (_textFlags & 0xf8)
				warning("Unprocessed text cast flags: %x", _textFlags & 0xf8);

			totalTextHeight = stream.readUint16();
		} else {
			pad2 = stream.readUint16();
			_initialRect = Score::readRect(stream);
			pad3 = stream.readUint16();
			pad4 = stream.readUint16();
			totalTextHeight = stream.readUint16();
		}

		debugC(2, kDebugLoading, "TextCast(): flags1: %d, border: %d gutter: %d shadow: %d pad1: %x align: %04x",
				_flags, _borderSize, _gutterSize, _boxShadow, pad1, _textAlign);
		debugC(2, kDebugLoading, "TextCast(): rgb: 0x%04x 0x%04x 0x%04x, pad2: %x pad3: %d pad4: %d shadow: %d flags: %d totHeight: %d",
				_palinfo1, _palinfo2, _palinfo3, pad2, pad3, pad4, _textShadow, _textFlags, totalTextHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCast(): rect:");
		}
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

	_cachedMacText = new CachedMacText(this, _bgcolor, version, -1, g_director->_wm);
	// TODO Destroy me

	_modified = false;
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
	_type = kCastShape;

	byte flags, unk1;

	_ink = kInkTypeCopy;

	if (version < 4) {
		flags = stream.readByte();
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Score::readRect(stream);
		_pattern = stream.readUint16BE();
		_fgCol = (127 - stream.readByte()) & 0xff; // -128 -> 0, 127 -> 256
		_bgCol = (127 - stream.readByte()) & 0xff;
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else if (version == 4) {
		flags = 0;
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Score::readRect(stream);
		_pattern = stream.readUint16BE();
		_fgCol = 0xff - (uint8)stream.readByte();
		_bgCol = 0xff - (uint8)stream.readByte();
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
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
	_modified = false;

	debugC(3, kDebugLoading, "ShapeCast: fl: %x unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		flags, unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCast: rect:");
}

ButtonCast::ButtonCast(Common::ReadStreamEndian &stream, uint16 version) : TextCast(stream, version, 0xff) {
	_type = kCastButton;

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
}

ScriptCast::ScriptCast(Common::ReadStreamEndian &stream, uint16 version) {
	_type = kCastLingoScript;
	_scriptType = kNoneScript;

	if (version < 4) {
		error("Unhandled Script cast");
	} else if (version == 4) {
		byte unk1 = stream.readByte();
		byte type = stream.readByte();

		switch (type) {
		case 1:
			_scriptType = kScoreScript;
			break;
		case 3:
			_scriptType = kMovieScript;
			break;
		default:
			error("ScriptCast: Unprocessed script type: %d", type);
		}

		_initialRect = Score::readRect(stream);
		_boundingRect = Score::readRect(stream);

		_id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d type: %s (%d) unk1: %d", _id, scriptType2str(_scriptType), type, unk1);

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
}

RTECast::RTECast(Common::ReadStreamEndian &stream, uint16 version, int32 bgcolor) : TextCast(stream, version, bgcolor) {

	_type = kCastRTE;
}

void RTECast::loadChunks() {
	//TODO: Actually load RTEs correctly, don't just make fake STXT.
#if 0
	Common::SeekableReadStream *rte1 = _movieArchive->getResource(res->children[child].tag, res->children[child].index);
	byte *buffer = new byte[rte1->size() + 2];
	rte1->read(buffer, rte1->size());
	buffer[rte1->size()] = '\n';
	buffer[rte1->size() + 1] = '\0';
	_loadedText->getVal(id)->importRTE(buffer);
#endif
}

} // End of namespace Director
