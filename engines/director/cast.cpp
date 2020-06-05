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
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/maceditabletext.h"
#include "graphics/macgui/macbutton.h"
#include "image/image_decoder.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/stxt.h"

namespace Director {

Cast::Cast() {
	_type = kCastTypeNull;
	_surface = nullptr;
	_widget = nullptr;
	_hilite = false;

	_img = nullptr;

	_modified = true;
}

Cast::~Cast() {
	if (_img)
		delete _img;

	if (_widget)
		delete _widget;
}

bool Cast::isEditable() {
	return false;
}

bool Cast::setEditable(bool editable) {
	warning("Cast::setEditable: Attempted to set editable of non-editable cast");
	return false;
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

SoundCast::SoundCast(Common::ReadStreamEndian &stream, uint16 version) {
	_type = kCastSound;
	_audio = nullptr;
	_looping = 0;

	if (version == 4) {
		for (int i = 0; i < 0xe; i++) {
			stream.readByte();
		}
		_looping = stream.readByte() & 0x10 ? 0 : 1;
	}
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

	_modified = false;
}

Graphics::TextAlign TextCast::getAlignment() {
	switch (_textAlign) {
	case kTextAlignRight:
		return Graphics::kTextAlignRight;
		break;
	case kTextAlignCenter:
		return Graphics::kTextAlignCenter;
		break;
	case kTextAlignLeft:
	default:
		return Graphics::kTextAlignLeft;
		break;
	}
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
}

void TextCast::createWidget() {
	uint fgcolor = g_director->_wm->findBestColor(_palinfo1 & 0xff, _palinfo2 & 0xff, _palinfo3 & 0xff);
	warning("TextCast::createWidget: bgcolor: %d, fgcolor: %d", _bgcolor, fgcolor);

	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);

	_widget = new Graphics::MacEditableText(g_director->getCurrentScore()->_window, 0, 0, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, fgcolor, _bgcolor, _initialRect.width(), getAlignment(), 1, _borderSize, _gutterSize, _boxShadow, _textShadow);

	((Graphics::MacEditableText *)_widget)->draw();
	delete macFont;
}

void TextCast::importRTE(byte *text) {
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

	if (_widget) {
		Graphics::MacEditableText *wtext = (Graphics::MacEditableText *)_widget;
		wtext->clearText();
		wtext->appendTextDefault(_ftext);
		wtext->draw();
	}

	_modified = true;
}

Common::String TextCast::getText() {
	if (_widget)
		_ptext = ((Graphics::MacEditableText *)_widget)->getEditedString().encode();

	return _ptext;
}

bool TextCast::isModified() {
	return _modified || (_widget ? ((Graphics::MacEditableText *)_widget)->_contentIsDirty : false);
}

bool TextCast::isEditable() {
	if (!_widget) {
		warning("TextCast::setEditable: Attempt to set editable of null widget");
		return false;
	}

	return (Graphics::MacEditableText *)_widget->_editable;
}

bool TextCast::setEditable(bool editable) {
	if (!_widget) {
		warning("TextCast::setEditable: Attempt to set editable of null widget");
		return false;
	}

	Graphics::MacEditableText *text = (Graphics::MacEditableText *)_widget;
	text->_focusable = editable;
	text->setEditable(editable);
	text->_selectable = editable;
	// text->setActive(editable);

	return true;
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
		// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
		_fgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_bgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
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
		_fgCol = g_director->transformColor((uint8)stream.readByte());
		_bgCol = g_director->transformColor((uint8)stream.readByte());
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
		_buttonType = static_cast<ButtonType>(stream.readUint16BE() - 1);
	} else {
		stream.readByte();
		stream.readByte();

		// This has already been populated in the super TextCast constructor
		//initialRect = Score::readRect(stream);
		//boundingRect = Score::readRect(stream);

		_buttonType = static_cast<ButtonType>(stream.readUint16BE());
	}
}

void ButtonCast::createWidget() {
	uint fgcolor = g_director->_wm->findBestColor(_palinfo1 & 0xff, _palinfo2 & 0xff, _palinfo3 & 0xff);

	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);

	_widget = new Graphics::MacButton(Graphics::MacButtonType(_buttonType), getAlignment(), g_director->getCurrentScore()->_window, 0, 0, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, fgcolor, _bgcolor);

	_widget->_focusable = true;

	((Graphics::MacButton *)(_widget))->draw();
	delete macFont;
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

	delete rte1;
#endif
}

} // End of namespace Director
