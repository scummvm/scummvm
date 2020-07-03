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

#include "graphics/macgui/macbutton.h"
#include "image/image_decoder.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/movie.h"
#include "director/stage.h"
#include "director/stxt.h"

namespace Director {

CastMember::CastMember(Cast* cast, uint16 castId) {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_widget = nullptr;
	_hilite = false;

	_modified = true;
}

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint32 castTag, uint16 version)
		: CastMember(cast, castId) {
	_type = kCastBitmap;
	_img = nullptr;
	_matte = nullptr;
	_bytes = 0;
	_pitch = 0;
	_flags = 0;
	_clut = 0;
	_regX = _regY = 0;
	_bitsPerPixel = 1;

	if (version < 4) {
		_flags = stream.readByte();	// region: 0 - auto, 1 - matte, 2 - disabled
		_bytes = stream.readUint16();
		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
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
		_flags = stream.readByte();
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
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

		warning("BitmapCastMember: %d bytes left", tail);
	} else if (version == 5) {
		uint16 count = stream.readUint16();
		for (uint16 cc = 0; cc < count; cc++)
			stream.readUint32();

		uint32 stringLength = stream.readUint32();
		for (uint32 s = 0; s < stringLength; s++)
			stream.readByte();

		/*uint16 width =*/ stream.readUint16LE(); //maybe?
		_initialRect = Movie::readRect(stream);

		/*uint32 somethingElse =*/ stream.readUint32();
		_boundingRect = Movie::readRect(stream);

		_bitsPerPixel = stream.readUint16();

		stream.readUint32();
	}
	_tag = castTag;
}

BitmapCastMember::~BitmapCastMember() {
	if (_img)
		delete _img;

	if (_matte)
		delete _matte;
}

void BitmapCastMember::createWidget() {
	CastMember::createWidget();

	if (!_img) {
		warning("BitmapCastMember::createWidget: No image decoder");
		return;
	}

	_widget = new Graphics::MacWidget(g_director->getStage(), 0, 0, _initialRect.width(), _initialRect.height(), g_director->_wm, false);
	_widget->getSurface()->blitFrom(*_img->getSurface());
}

void BitmapCastMember::createMatte() {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels
	// are transparent
	Graphics::Surface tmp;
	tmp.create(_initialRect.width(), _initialRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	tmp.copyFrom(*_img->getSurface());

	// Searching white color in the corners
	int whiteColor = -1;

	for (int y = 0; y < tmp.h; y++) {
		for (int x = 0; x < tmp.w; x++) {
			byte color = *(byte *)tmp.getBasePtr(x, y);

			if (g_director->getPalette()[color * 3 + 0] == 0xff &&
					g_director->getPalette()[color * 3 + 1] == 0xff &&
					g_director->getPalette()[color * 3 + 2] == 0xff) {
				whiteColor = color;
				break;
			}
		}
	}

	if (whiteColor == -1) {
		debugC(1, kDebugImages, "BitmapCastMember::createMatte(): No white color for matte image");
	} else {
		delete _matte;

		_matte = new Graphics::FloodFill(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			_matte->addSeed(0, yy);
			_matte->addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			_matte->addSeed(xx, 0);
			_matte->addSeed(xx, tmp.h - 1);
		}

		_matte->fillMask();
	}

	tmp.free();
}

Graphics::Surface *BitmapCastMember::getMatte() {
	// Lazy loading of mattes
	if (!_matte) {
		createMatte();
	}

	return _matte ? _matte->getMask() : nullptr;
}

DigitalVideoCastMember::DigitalVideoCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId) {
	_type = kCastDigitalVideo;

	if (version < 4) {
		warning("STUB: DigitalVideoCastMember: unhandled properties data");
		for (int i = 0; i < 0xd; i++) {
			stream.readByte();
		}
		_frameRate = 12;
		_frameRateType = kFrameRateDefault;

		_preload = false;
		_enableVideo = false;
		_pauseAtStart = false;
		_showControls = false;
		_looping = false;
		_enableSound = false;
		_enableCrop = false;
		_center = false;
	} else {
		for (int i = 0; i < 5; i++) {
			stream.readUint16();
		}
		_frameRate = stream.readByte();
		stream.readByte();

		byte flags1 = stream.readByte();
		_frameRateType = kFrameRateDefault;
		if (flags1 & 0x08) {
			_frameRateType = (FrameRateType)((flags1 & 0x30) >> 4);
		}
		_preload = flags1 & 0x04;
		_enableVideo = !(flags1 & 0x02);
		_pauseAtStart = flags1 & 0x01;

		byte flags2 = stream.readByte();
		_showControls = flags2 & 0x40;
		_looping = flags2 & 0x10;
		_enableSound = flags2 & 0x08;
		_enableCrop = !(flags2 & 0x02);
		_center = flags2 & 0x01;
	}
}

SoundCastMember::SoundCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId) {
	_type = kCastSound;
	_audio = nullptr;
	_looping = 0;

	if (version == 4) {
		for (int i = 0; i < 0xf; i++) {
			stream.readByte();
		}
		_looping = stream.readByte() & 0x10 ? 0 : 1;
	}
}

TextCastMember::TextCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version, bool asButton)
		: CastMember(cast, castId) {
	_type = kCastText;

	_borderSize = kSizeNone;
	_gutterSize = kSizeNone;
	_boxShadow = kSizeNone;
	_buttonType = kTypeButton;

	_bgcolor = 0;
	_fgcolor = 0;

	_flags = 0;
	_textFlags = 0;
	_fontId = 0;
	_fontSize = 12;
	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_textSlant = 0;
	_bgpalinfo1 = _bgpalinfo2 = _bgpalinfo3 = 0;
	_fgpalinfo1 = _fgpalinfo2 = _fgpalinfo3 = 0xff;

	if (version <= 3) {
		_flags = stream.readByte(); // region: 0 - auto, 1 - matte, 2 - disabled
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		byte pad1 = stream.readByte();
		_textAlign = static_cast<TextAlignType>(stream.readUint16());
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();

		uint32 pad2;
		uint16 pad3;
		uint16 pad4 = 0;
		uint16 totalTextHeight;

		if (version == 2) {
			pad2 = stream.readUint16();
			if (pad2 != 0) { // In D2 there are values
				warning("TextCastMember: pad2: %x", pad2);
			}

			_initialRect = Movie::readRect(stream);
			pad3 = stream.readUint16();

			_textShadow = static_cast<SizeType>(stream.readByte());
			_textFlags = stream.readByte();
			if (_textFlags & 0xf8)
				warning("Unprocessed text cast flags: %x", _textFlags & 0xf8);

			totalTextHeight = stream.readUint16();
		} else {
			pad2 = stream.readUint16();
			_initialRect = Movie::readRect(stream);
			pad3 = stream.readUint16();
			pad4 = stream.readUint16();
			totalTextHeight = stream.readUint16();
		}

		debugC(2, kDebugLoading, "TextCastMember(): flags1: %d, border: %d gutter: %d shadow: %d pad1: %x align: %04x",
				_flags, _borderSize, _gutterSize, _boxShadow, pad1, _textAlign);
		debugC(2, kDebugLoading, "TextCastMember(): background rgb: 0x%04x 0x%04x 0x%04x, pad2: %x pad3: %d pad4: %d shadow: %d flags: %d totHeight: %d",
				_bgpalinfo1, _bgpalinfo2, _bgpalinfo3, pad2, pad3, pad4, _textShadow, _textFlags, totalTextHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCastMember(): rect:");
		}
	} else if (version == 4) {
		byte flags = stream.readByte();
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();
		stream.readUint16();

		_fontId = 1; // this is in STXT

		_initialRect = Movie::readRect(stream);
		stream.readUint16();
		_textShadow = static_cast<SizeType>(stream.readByte());
		byte flags2 = stream.readByte();

		if (flags || flags2)
			warning("Unprocessed text cast flags: %x, flags:2 %x", flags, flags2);

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

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		stream.readUint32();
		stream.readUint16();
		stream.readUint16();
	}

	if (asButton) {
		_type = kCastButton;

		if (version < 4) {
			_buttonType = static_cast<ButtonType>(stream.readUint16BE() - 1);
		} else {
			stream.readByte();
			stream.readByte();

			// This has already been populated in the super TextCastMember constructor
			//initialRect = Score::readRect(stream);
			//boundingRect = Score::readRect(stream);

			_buttonType = static_cast<ButtonType>(stream.readUint16BE());
		}
	}

	_bgcolor = g_director->_wm->findBestColor(_bgpalinfo1 & 0xff, _bgpalinfo2 & 0xff, _bgpalinfo3 & 0xff);

	_modified = true;
}

void TextCastMember::setColors(int *fgcolor, int *bgcolor) {
	if (!_widget)
		return;

	if (fgcolor)
		_fgcolor = *fgcolor;

	if (bgcolor)
		_bgcolor = *bgcolor;

	_widget->setColors(_fgcolor, _bgcolor);
	((Graphics::MacText *)_widget)->_fullRefresh = true;
}

void TextCastMember::getColors(int *fgcolor, int *bgcolor) {
	if (fgcolor)
		*fgcolor = _fgcolor;

	if (bgcolor)
		*bgcolor = _bgcolor;
}

Graphics::TextAlign TextCastMember::getAlignment() {
	switch (_textAlign) {
	case kTextAlignRight:
		return Graphics::kTextAlignRight;
	case kTextAlignCenter:
		return Graphics::kTextAlignCenter;
	case kTextAlignLeft:
	default:
		return Graphics::kTextAlignLeft;
	}
}

void TextCastMember::importStxt(const Stxt *stxt) {
	_fontId = stxt->_fontId;
	_textSlant = stxt->_textSlant;
	_fontSize = stxt->_fontSize;
	_fgpalinfo1 = stxt->_palinfo1;
	_fgpalinfo2 = stxt->_palinfo2;
	_fgpalinfo3 = stxt->_palinfo3;
	_ftext = stxt->_ftext;
	_ptext = stxt->_ptext;

	_fgcolor = g_director->_wm->findBestColor(_fgpalinfo1 & 0xff, _fgpalinfo2 & 0xff, _fgpalinfo3 & 0xff);
}

void TextCastMember::createWidget() {
	CastMember::createWidget();

	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);

	switch (_type) {
	case kCastText:
		_widget = new Graphics::MacText(g_director->getStage(), 0, 0, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, getForeColor(), getBackColor(), _initialRect.width(), getAlignment(), 0, _borderSize, _gutterSize, _boxShadow, _textShadow);

		((Graphics::MacText *)_widget)->draw();
		break;

	case kCastButton:
		_widget = new Graphics::MacButton(Graphics::MacButtonType(_buttonType), getAlignment(), g_director->getStage(), 0, 0, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, getForeColor(), 0xff);
		((Graphics::MacButton *)_widget)->draw();
		_widget->_focusable = true;

		((Graphics::MacButton *)(_widget))->draw();
		break;

	default:
		break;
	}

	delete macFont;
}

void TextCastMember::importRTE(byte *text) {
	//assert(rteList.size() == 3);
	//child0 is probably font data.
	//child1 is the raw text.
	_ptext = _ftext = Common::String((char*)text);
	//child2 is positional?
}

void TextCastMember::setText(const char *text) {
	// Do nothing if text did not change
	if (_ftext.equals(text))
		return;

	_ptext = _ftext = text;

	if (_widget) {
		Graphics::MacText *wtext = (Graphics::MacText *)_widget;
		wtext->clearText();
		wtext->appendTextDefault(_ftext);
		wtext->draw();
	}

	_modified = true;
}

Common::String TextCastMember::getText() {
	if (_widget)
		_ptext = ((Graphics::MacText *)_widget)->getEditedString().encode();

	return _ptext;
}

bool TextCastMember::isModified() {
	return _modified || (_widget ? ((Graphics::MacText *)_widget)->_contentIsDirty : false);
}

bool TextCastMember::isEditable() {
	if (!_widget) {
		warning("TextCastMember::setEditable: Attempt to set editable of null widget");
		return false;
	}

	return (Graphics::MacText *)_widget->_editable;
}

bool TextCastMember::setEditable(bool editable) {
	if (!_widget) {
		warning("TextCastMember::setEditable: Attempt to set editable of null widget");
		return false;
	}

	Graphics::MacText *text = (Graphics::MacText *)_widget;
	text->_focusable = editable;
	text->setEditable(editable);
	text->_selectable = editable;
	// text->setActive(editable);

	return true;
}

ShapeCastMember::ShapeCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId) {
	_type = kCastShape;

	byte flags, unk1;

	_ink = kInkTypeCopy;

	if (version < 4) {
		flags = stream.readByte();
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
		_pattern = stream.readUint16BE();
		// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
		_fgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_bgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else if (version == 4) {
		flags = stream.readByte();
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
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

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		_shapeType = kShapeRectangle;
		_pattern = 0;
		_fgCol = _bgCol = 0;
		_fillType = 0;
		_lineThickness = 1;
		_lineDirection = 0;
	}
	_modified = false;

	debugC(3, kDebugLoading, "ShapeCastMember: fl: %x unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		flags, unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCastMember: rect:");
}

ScriptCastMember::ScriptCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId) {
	_type = kCastLingoScript;
	_scriptType = kNoneScript;

	if (version < 4) {
		error("Unhandled Script cast");
	} else if (version == 4) {
		byte flags = stream.readByte();
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
			error("ScriptCastMember: Unprocessed script type: %d", type);
		}

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		_id = stream.readUint32();

		debugC(3, kDebugLoading, "CASt: Script id: %d type: %s (%d), flags: (%x), unk1: %d", _id, scriptType2str(_scriptType), type, flags, unk1);

		stream.readByte(); // There should be no more data
		assert(stream.eos());
	} else if (version > 4) {
		stream.readByte();
		stream.readByte();

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		_id = stream.readUint32();

		debugC(4, kDebugLoading, "CASt: Script id: %d", _id);

		// WIP need to complete this!
	}
}

RTECastMember::RTECastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version)
		: TextCastMember(cast, castId, stream, version) {

	_type = kCastRTE;
}

void RTECastMember::loadChunks() {
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
