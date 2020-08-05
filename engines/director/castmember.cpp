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
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/stxt.h"

namespace Director {

CastMember::CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream) {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_autoHilite = false;
	_purgePriority = 3;
	_size = stream.size();
	_flags1 = 0;

	_modified = true;
}

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1)
		: CastMember(cast, castId, stream) {
	_type = kCastBitmap;
	_img = nullptr;
	_matte = nullptr;
	_noMatte = false;
	_bytes = 0;
	_pitch = 0;
	_flags2 = 0;
	_regX = _regY = 0;
	_clut = kClutSystemMac;
	_bitsPerPixel = 0;

	if (version < 4) {
		_flags1 = flags1;	// region: 0 - auto, 1 - matte, 2 - disabled, 8 - no auto
		if (_flags1 >> 4 == 0x0)
			_autoHilite = true;

		_bytes = stream.readUint16();
		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		if (_bytes & 0x8000) {
			_bitsPerPixel = stream.readUint16();
			_clut = stream.readSint16() - 1;
		} else {
			_bitsPerPixel = 1;
			_clut = kClutSystemMac;
		}

		_pitch = _initialRect.width();
		if (_pitch % 16)
			_pitch += 16 - (_initialRect.width() % 16);

	} else if (version == 4) {
		_flags1 = flags1;
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_bitsPerPixel = stream.readUint16();

		if (stream.eos()) {
			_bitsPerPixel = 0;
		} else {
			_clut = stream.readSint16() - 1;
			stream.readUint16();
			/* uint16 unk1 = */ stream.readUint16();
			stream.readUint16();

			stream.readUint32();
			stream.readUint32();

			_flags2 = stream.readUint16();
		}

		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		if (_bitsPerPixel == 1)
			_pitch *= 8;

		_autoHilite = (_flags2 % 4 != 0);

		int tail = 0;
		byte buf[256];

		while (!stream.eos()) {
			byte c = stream.readByte();
			if (tail < 256)
				buf[tail] = c;
			tail++;
		}

		if (tail)
			warning("BUILDBOT: BitmapCastMember: %d bytes left", tail);

		if (tail && debugChannelSet(2, kDebugLoading)) {
			debug("BitmapCastMember: tail");
			Common::hexdump(buf, tail);
		}
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

Graphics::MacWidget *BitmapCastMember::createWidget(Common::Rect &bbox) {
	if (!_img) {
		warning("BitmapCastMember::createWidget: No image decoder");
		return nullptr;
	}

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);
	widget->getSurface()->blitFrom(*_img->getSurface());
	return widget;
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
		_noMatte = true;
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
		_noMatte = false;
	}

	tmp.free();
}

Graphics::Surface *BitmapCastMember::getMatte() {
	// Lazy loading of mattes
	if (!_matte && !_noMatte) {
		createMatte();
	}

	return _matte ? _matte->getMask() : nullptr;
}

DigitalVideoCastMember::DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastDigitalVideo;
	_video = nullptr;

	if (version < 4) {
		warning("STUB: DigitalVideoCastMember: unhandled rect data");
		if (debugChannelSet(5, kDebugLoading))
			stream.hexdump(stream.size());
		for (int i = 0; i < 0x4; i++)
			stream.readByte(); // 0

		for (int i = 0; i < 0x4; i++)
			stream.readByte(); // looks rect like, but as bytes not uint16.

		for ( int i = 0; i < 0x3; i++)
			stream.readByte(); // 0

		_frameRate = 12;
		_frameRateType = kFrameRateDefault;

		_preload = false;
		_enableVideo = false;
		_pausedAtStart = false;

		byte flag = stream.readByte();

		_showControls = flag & 0x40;
		_looping = flag & 0x10;
		_enableSound = flag & 0x08;
		_crop = !(flag & 0x02);
		_center = flag & 0x01;
		_directToStage = flag & 0x20;
	} else {
		stream.readByte();
		_initialRect = Movie::readRect(stream);
		_frameRate = stream.readByte();
		stream.readByte();

		byte flags1 = stream.readByte();
		_frameRateType = kFrameRateDefault;
		if (flags1 & 0x08) {
			_frameRateType = (FrameRateType)((flags1 & 0x30) >> 4);
		}
		_preload = flags1 & 0x04;
		_enableVideo = !(flags1 & 0x02);
		_pausedAtStart = flags1 & 0x01;

		byte flags2 = stream.readByte();
		_showControls = flags2 & 0x40;
		_looping = flags2 & 0x10;
		_enableSound = flags2 & 0x08;
		_crop = !(flags2 & 0x02);
		_center = flags2 & 0x01;
		_directToStage = true;
		debugC(2, kDebugLoading, "DigitalVideoCastMember(): flags1: (%d 0x%x)", flags1, flags1);

		debugC(2, kDebugLoading, "DigitalVideoCastMember(): flags2: (%d 0x%x)", flags2, flags2);
	}
	debugC(2, kDebugLoading, "_frameRate: %d", _frameRateType);
	debugC(2, kDebugLoading, "_frameRateType: %d, _preload: %d, _enableVideo %d, _pausedAtStart %d",
			_frameRateType, _preload, _enableVideo, _pausedAtStart);
	debugC(2, kDebugLoading, "_showControls: %d, _looping: %d, _enableSound: %d, _crop %d, _center: %d, _directToStage: %d",
			_showControls, _looping, _enableSound, _crop, _center, _directToStage);

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "DigitalVideoCastMember(): rect:");
}

DigitalVideoCastMember::~DigitalVideoCastMember() {

}

SoundCastMember::SoundCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastSound;
	_audio = nullptr;
	_looping = 0;
}

TextCastMember::TextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version, uint8 flags1, bool asButton)
		: CastMember(cast, castId, stream) {
	_type = kCastText;

	_borderSize = kSizeNone;
	_gutterSize = kSizeNone;
	_boxShadow = kSizeNone;
	_buttonType = kTypeButton;
	_editable = false;
	_maxHeight = _textHeight = 0;

	_bgcolor = 0;
	_fgcolor = 0;

	_textFlags = 0;
	_scroll = 0;
	_fontId = 1;
	_fontSize = 12;
	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_textSlant = 0;
	_bgpalinfo1 = _bgpalinfo2 = _bgpalinfo3 = 0;
	_fgpalinfo1 = _fgpalinfo2 = _fgpalinfo3 = 0xff;

	if (version <= 3) {
		_flags1 = flags1; // region: 0 - auto, 1 - matte, 2 - disabled
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
				_flags1, _borderSize, _gutterSize, _boxShadow, pad1, _textAlign);
		debugC(2, kDebugLoading, "TextCastMember(): background rgb: 0x%04x 0x%04x 0x%04x, pad2: %x pad3: %d pad4: %d shadow: %d flags: %d totHeight: %d",
				_bgpalinfo1, _bgpalinfo2, _bgpalinfo3, pad2, pad3, pad4, _textShadow, _textFlags, totalTextHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCastMember(): rect:");
		}
	} else if (version == 4) {
		_flags1 = flags1;
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();
		_scroll = stream.readUint16();

		_fontId = 1; // this is in STXT

		_initialRect = Movie::readRect(stream);
		_maxHeight = stream.readUint16();
		_textShadow = static_cast<SizeType>(stream.readByte());
		_textFlags = stream.readByte();

		_textHeight = stream.readUint16();
		_textSlant = 0;
	} else {
		_fontId = 1;

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

		if (version <= 4) {
			_buttonType = static_cast<ButtonType>(stream.readUint16BE() - 1);
		} else {
			warning("TextCastMember(): Attempting to initialize >D4 button castmember");
			_buttonType = kTypeButton;
		}
	}

	_bgcolor = g_director->_wm->findBestColor(_bgpalinfo1 & 0xff, _bgpalinfo2 & 0xff, _bgpalinfo3 & 0xff);

	_modified = true;
}

void TextCastMember::setColors(int *fgcolor, int *bgcolor) {
	if (fgcolor)
		_fgcolor = *fgcolor;

	if (bgcolor)
		_bgcolor = *bgcolor;
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
	_fontId = stxt->_style.fontId;
	_textSlant = stxt->_style.textSlant;
	_fontSize = stxt->_style.fontSize;
	_fgpalinfo1 = stxt->_style.r;
	_fgpalinfo2 = stxt->_style.g;
	_fgpalinfo3 = stxt->_style.b;
	_ftext = stxt->_ftext;
	_ptext = stxt->_ptext;
}

Graphics::MacWidget *TextCastMember::createWidget(Common::Rect &bbox) {
	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);
	Graphics::MacWidget *widget = nullptr;

	switch (_type) {
	case kCastText:
		widget = new Graphics::MacText(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, _ftext, macFont, getForeColor(), getBackColor(), bbox.width(), getAlignment(), 0, _borderSize, _gutterSize, _boxShadow, _textShadow);
		((Graphics::MacText *)widget)->draw();
		((Graphics::MacText *)widget)->_focusable = _editable;
		((Graphics::MacText *)widget)->setEditable(_editable);
		((Graphics::MacText *)widget)->_selectable = _editable;
		break;

	case kCastButton:
		// note that we use _initialRect for the dimensions of the button;
		// the values provided in the sprite bounding box are ignored
		widget = new Graphics::MacButton(Graphics::MacButtonType(_buttonType), getAlignment(), g_director->getCurrentWindow(), bbox.left, bbox.top, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, getForeColor(), 0xff);
		((Graphics::MacButton *)widget)->draw();
		widget->_focusable = true;

		((Graphics::MacButton *)widget)->draw();
		break;

	default:
		break;
	}

	delete macFont;
	return widget;
}

Common::Rect TextCastMember::getWidgetRect() {
	Graphics::MacWidget *widget = createWidget(_initialRect);
	Common::Rect result = _initialRect;
	if (widget) {
		result = widget->_dims;
		delete widget;
	}
	return result;
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
	_modified = true;
}

Common::String TextCastMember::getText() {
	return _ptext;
}

bool TextCastMember::isModified() {
	return _modified;
}

bool TextCastMember::isEditable() {
	return _editable;
}

void TextCastMember::setEditable(bool editable) {
	_editable = editable;
}

void TextCastMember::updateFromWidget(Graphics::MacWidget *widget) {
	if (widget && _type == kCastText) {
		_ptext = ((Graphics::MacText *)widget)->getEditedString().encode();
	}
}

ShapeCastMember::ShapeCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastShape;

	byte unk1;

	_ink = kInkTypeCopy;

	if (version < 4) {
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
		stream.readByte(); // FIXME: Was this copied from D4 by mistake?
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

	debugC(3, kDebugLoading, "ShapeCastMember: unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCastMember: rect:");
}

ScriptCastMember::ScriptCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
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
			error("ScriptCastMember: Unprocessed script type: %d", type);
		}

		debugC(3, kDebugLoading, "CASt: Script type: %s (%d), unk1: %d", scriptType2str(_scriptType), type, unk1);

		stream.readByte(); // There should be no more data
		assert(stream.eos());
	} else if (version > 4) {
		stream.readByte();
		stream.readByte();

		debugC(4, kDebugLoading, "CASt: Script");

		// WIP need to complete this!
	}
}

RTECastMember::RTECastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
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

PaletteCastMember::PaletteCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
	: CastMember(cast, castId, stream) {
	_type = kCastPalette;
	_palette = nullptr;
}

} // end of namespace Director
