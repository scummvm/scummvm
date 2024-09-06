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

#include "common/events.h"

#include "graphics/macgui/macbutton.h"
#include "graphics/macgui/mactextwindow.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/castmember/text.h"
#include "director/lingo/lingo-the.h"

namespace Director {

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
	_fgcolor = 0xff;

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

	// seems like the line spacing is default to 1 in D4
	_lineSpacing = g_director->getVersion() >= 400 ? 1 : 0;

	if (debugChannelSet(4, kDebugLoading)) {
		stream.hexdump(stream.size());
	}

	if (version < kFileVer400) {
		_flags1 = flags1; // region: 0 - auto, 1 - matte, 2 - disabled
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readUint16());
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();

		uint32 pad2;
		uint16 pad3;
		uint16 pad4 = 0;
		uint16 totalTextHeight;

		if (version < kFileVer300) {
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
			_textFlags = stream.readUint16(); // 1: editable, 2: auto tab, 4: don't wrap
			_editable = _textFlags & 0x1;
			totalTextHeight = stream.readUint16();
		}

		debugC(2, kDebugLoading, "TextCastMember(): flags1: %d, border: %d gutter: %d shadow: %d textType: %d align: %04x",
				_flags1, _borderSize, _gutterSize, _boxShadow, _textType, _textAlign);
		debugC(2, kDebugLoading, "TextCastMember(): background rgb: 0x%04x 0x%04x 0x%04x, pad2: %x pad3: %d pad4: %d shadow: %d flags: %d totHeight: %d",
				_bgpalinfo1, _bgpalinfo2, _bgpalinfo3, pad2, pad3, pad4, _textShadow, _textFlags, totalTextHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCastMember(): rect:");
		}
	} else if (version >= kFileVer400 && version < kFileVer600) {
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
		_textFlags = stream.readByte(); // 1: editable, 2: auto tab 4: don't wrap
		_editable = _textFlags & 0x1;

		_textHeight = stream.readUint16();
		_textSlant = 0;
		debugC(2, kDebugLoading, "TextCastMember(): flags1: %d, border: %d gutter: %d shadow: %d textType: %d align: %04x",
				_flags1, _borderSize, _gutterSize, _boxShadow, _textType, _textAlign);
		debugC(2, kDebugLoading, "TextCastMember(): background rgb: 0x%04x 0x%04x 0x%04x, shadow: %d flags: %d textHeight: %d",
				_bgpalinfo1, _bgpalinfo2, _bgpalinfo3, _textShadow, _textFlags, _textHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCastMember(): rect:");
		}
	} else {
		warning("Text/ButtonCastMember(): >D5 isn't handled");
	}

	if (asButton) {
		_type = kCastButton;
		_buttonType = static_cast<ButtonType>(stream.readUint16BE() - 1);
	}

	_bgcolor = g_director->_wm->findBestColor(_bgpalinfo1 & 0xff, _bgpalinfo2 & 0xff, _bgpalinfo3 & 0xff);

	_modified = true;
}

TextCastMember::TextCastMember(Cast *cast, uint16 castId, TextCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastText;
	// force a load so we can copy the cast resource information
	source.load();
	_loaded = true;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_children = source._children;

	_borderSize = source._borderSize;
	_gutterSize = source._gutterSize;
	_boxShadow = source._boxShadow;
	_maxHeight = source._maxHeight;
	_textHeight = source._textHeight;

	_fontId = source._fontId;
	_fontSize = source._fontSize;
	_textType = source._textType;
	_textAlign = source._textAlign;
	_textShadow = source._textShadow;
	_scroll = source._scroll;
	_textSlant = source._textSlant;
	_textFlags = source._textFlags;
	_bgpalinfo1 = source._bgpalinfo1;
	_bgpalinfo2 = source._bgpalinfo2;
	_bgpalinfo3 = source._bgpalinfo3;
	_fgpalinfo1 = source._fgpalinfo1;
	_fgpalinfo2 = source._fgpalinfo2;
	_fgpalinfo3 = source._fgpalinfo3;
	_buttonType = source._buttonType;
	_editable = source._editable;
	_lineSpacing = source._lineSpacing;

	_ftext = source._ftext;
	_ptext = source._ptext;
	_rtext = source._rtext;

	_bgcolor = source._bgcolor;
	_fgcolor = source._fgcolor;
}

void TextCastMember::setColors(uint32 *fgcolor, uint32 *bgcolor) {
	if (fgcolor)
		_fgcolor = *fgcolor;

	if (bgcolor)
		_bgcolor = *bgcolor;

	// if we want to keep the format unchanged, then we need to modify _ftext as well
	if (_widget)
		((Graphics::MacText *)_widget)->setColors(_fgcolor, _bgcolor);
	else
		_modified = true;
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

void TextCastMember::setBackColor(uint32 bgCol) {
	_bgcolor = bgCol;
	_modified = true;
}

void TextCastMember::setForeColor(uint32 fgCol) {
	_fgcolor = fgCol;
	_modified = true;
}

void TextCastMember::importStxt(const Stxt *stxt) {
	_fontId = stxt->_style.fontId;
	_textSlant = stxt->_style.textSlant;
	_fontSize = stxt->_style.fontSize;
	_fgpalinfo1 = stxt->_style.r;
	_fgpalinfo2 = stxt->_style.g;
	_fgpalinfo3 = stxt->_style.b;
	// The default color in the Stxt will override the fgcolor,
	// e.g. empty editable text boxes will use the Stxt color
	_fgcolor = g_director->_wm->findBestColor(_fgpalinfo1 >> 8, _fgpalinfo2 >> 8, _fgpalinfo3 >> 8);
	_ftext = stxt->_ftext;
	_ptext = stxt->_ptext;
	_rtext = stxt->_rtext;

	// Rectifying _fontId in case of a fallback font
	Graphics::MacFont macFont(_fontId, _fontSize, _textSlant);
	g_director->_wm->_fontMan->getFont(&macFont);
	_fontId = macFont.getId();

	// If the text is empty, that means we ignored the font and now
	// set the text height to a minimal one.
	//
	// This fixes `number of chars` in Lingo Workshop
	if (_textType == kTextTypeAdjustToFit && _ftext.empty())
		_initialRect.setHeight(macFont.getSize() + (2 * _borderSize) + _gutterSize + _boxShadow);
}

bool textWindowCallback(Graphics::WindowClick click, Common::Event &event, void *ptr) {
	return g_director->getCurrentMovie()->processEvent(event);
}

Graphics::MacWidget *TextCastMember::createWindowOrWidget(Common::Rect &bbox, Channel *channel, Common::Rect dims, Graphics::MacFont *macFont) {
	Graphics::MacWidget *widget = nullptr;

	if (_textType == kTextTypeScrolling) {
		Graphics::MacTextWindow *window = (Graphics::MacTextWindow *)g_director->_wm->addTextWindow(macFont, getForeColor(), getBackColor(), _initialRect.width(),
														  getAlignment(), nullptr, false);
		// Set callback so that we can process events like mouse clicks
		window->setCallback(textWindowCallback, window);
		// Set widget to this window!
		widget = window;

		// Set configuration
		window->setBorderType(Graphics::kWindowBorderMacOSNoBorderScrollbar);
		window->enableScrollbar(true);
		// window->setMode(Graphics::kWindowModeDynamicScrollbar);
		window->move(bbox.left, bbox.top);
		window->resize(dims.width(), dims.height());
		window->setEditable(false);
		window->setSelectable(false);
		window->appendText(_ftext);
		window->draw(true);
	} else {
		widget = new Graphics::MacText(g_director->getCurrentWindow(), bbox.left, bbox.top, dims.width(), dims.height(), g_director->_wm, _ftext, macFont, getForeColor(), getBackColor(), _initialRect.width(), getAlignment(), _lineSpacing, _borderSize, _gutterSize, _boxShadow, _textShadow, _textType == kTextTypeFixed);
		((Graphics::MacText *)widget)->setSelRange(g_director->getCurrentMovie()->_selStart, g_director->getCurrentMovie()->_selEnd);
		((Graphics::MacText *)widget)->setEditable(channel->_sprite->_editable);
		((Graphics::MacText *)widget)->draw();
	}

	return widget;
}

Graphics::MacWidget *TextCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);
	Graphics::MacWidget *widget = nullptr;
	Common::Rect dims(bbox);

	CastType type = _type;
	ButtonType buttonType = _buttonType;

	// WORKAROUND: In D2/D3 there can be text casts that have button
	// information set in the sprite.
	if (type == kCastText && isButtonSprite(spriteType)) {
		type = kCastButton;
		buttonType = ButtonType(spriteType - 8);
	}

	switch (type) {
	case kCastText:
		// for mactext, we can expand now, but we can't shrink. so we may pass the small size when we have adjustToFit text style
		if (_textType == kTextTypeAdjustToFit) {
			dims.right = MIN<int>(dims.right, dims.left + _initialRect.width());
			dims.bottom = MIN<int>(dims.bottom, dims.top + _initialRect.height());
		} else if (_textType == kTextTypeFixed) {
			// use initialRect to create widget for fixed style text, this maybe related to version.
			dims.right = MAX<int>(dims.right, dims.left + _initialRect.width());
			dims.bottom = MAX<int>(dims.bottom, dims.top + _initialRect.height());
		}
		widget = createWindowOrWidget(bbox, channel, dims, macFont);

		// since we disable the ability of setActive in setEdtiable, then we need to set active widget manually
		if (channel->_sprite->_editable) {
			Graphics::MacWidget *activeWidget = g_director->_wm->getActiveWidget();
			if (activeWidget == nullptr || !activeWidget->isEditable())
				g_director->_wm->setActiveWidget(widget);
		}
		break;

	case kCastButton:
		// note that we use _initialRect for the dimensions of the button;
		// the values provided in the sprite bounding box are ignored
		widget = new Graphics::MacButton(Graphics::MacButtonType(buttonType), getAlignment(), g_director->getCurrentWindow(), bbox.left, bbox.top, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, getForeColor(), g_director->_wm->_colorWhite);
		widget->_focusable = true;

		((Graphics::MacButton *)widget)->setHilite(_hilite);
		((Graphics::MacButton *)widget)->setCheckBoxType(g_director->getCurrentMovie()->_checkBoxType);
		((Graphics::MacButton *)widget)->draw();
		break;

	default:
		break;
	}

	delete macFont;
	return widget;
}

void TextCastMember::importRTE(byte *text) {
	//assert(rteList.size() == 3);
	//child0 is probably font data.
	//child1 is the raw text.
	_rtext = _ptext = _ftext = Common::String((char*)text);
	//child2 is positional?
}

void TextCastMember::setRawText(const Common::String &text) {
	// Do nothing if text did not change
	if (_ptext.equals(Common::U32String(text)))
		return;

	_rtext = text;
	_ptext = Common::U32String(text);

	// If text has changed, use the cached formatting from first STXT in this castmember.
	Common::U32String formatting = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", _fontId, _textSlant, _fontSize, _fgpalinfo1, _fgpalinfo2, _fgpalinfo3);
	_ftext = formatting + _ptext;
	_modified = true;
}

// D4 dictionary book said this is line spacing
int TextCastMember::getTextHeight() {
	if (_widget)
		return ((Graphics::MacText *)_widget)->getLineSpacing();
	else
		return _lineSpacing;
	return 0;
}

// this should be amend when we have some where using this function
int TextCastMember::getTextSize() {
	if (_widget)
		return ((Graphics::MacText *)_widget)->getTextSize();
	else
		return _fontSize;
	return 0;
}

Common::U32String TextCastMember::getText() {
	return _ptext;
}

Common::String TextCastMember::getRawText() {
	return _rtext;
}

void TextCastMember::setTextSize(int textSize) {
	if (_widget) {
		((Graphics::MacText *)_widget)->setTextSize(textSize);
		((Graphics::MacText *)_widget)->draw();
	} else {
		_fontSize = textSize;
		_modified = true;
	}
}

void TextCastMember::updateFromWidget(Graphics::MacWidget *widget) {
	if (widget && _type == kCastText) {
		_ptext = ((Graphics::MacText *)widget)->getEditedString();
	}
}

Common::String TextCastMember::formatInfo() {
	// need to pull the data from the STXT resource before the
	// debug output will be visible
	load();
	Common::String format = formatStringForDump(_ptext.encode());

	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, foreColor: %d, backColor: %d, editable: %d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		getForeColor(), getBackColor(),
		_editable, formatStringForDump(format).c_str()
	);
}

void TextCastMember::load() {
	if (_loaded)
		return;

	uint stxtid = 0;
	if (_cast->_version >= kFileVer400) {
		for (auto &it : _children) {
			if (it.tag == MKTAG('S', 'T', 'X', 'T')) {
				stxtid = it.index;
				break;
			}
		}
		if (!stxtid) {
			warning("TextCastMember::load(): No STXT resource found in %d children", _children.size());
		}
	} else {
		stxtid = _castId;
	}

	if (_cast->_loadedStxts.contains(stxtid)) {
		const Stxt *stxt = _cast->_loadedStxts.getVal(stxtid);
		importStxt(stxt);
		_size = stxt->_size;
	} else {
		warning("TextCastMember::load(): stxtid %i isn't loaded", stxtid);
	}

	_loaded = true;
}

void TextCastMember::unload() {
	// No unload necessary.
}

bool TextCastMember::hasField(int field) {
	switch (field) {
	case kTheHilite:
	case kTheText:
	case kTheTextAlign:
	case kTheTextFont:
	case kTheTextHeight:
	case kTheTextSize:
	case kTheTextStyle:
	case kTheScrollTop:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum TextCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheHilite:
		d = _hilite;
		break;
	case kTheText:
		d = getText().encode(Common::kUtf8);
		break;
	case kTheTextAlign:
		d.type = STRING;
		switch (_textAlign) {
		case kTextAlignLeft:
			d.u.s = new Common::String("left");
			break;
		case kTextAlignCenter:
			d.u.s = new Common::String("center");
			break;
		case kTextAlignRight:
			d.u.s = new Common::String("right");
			break;
		default:
			warning("TextCastMember::getField(): Invalid text align spec");
			break;
		}
		break;
	case kTheTextFont:
		d.type = STRING;
		d.u.s = new Common::String(g_director->_wm->_fontMan->getFontName(_fontId));
		break;
	case kTheTextHeight:
		d = getTextHeight();
		break;
	case kTheTextSize:
		d = getTextSize();
		break;
	case kTheTextStyle:
		d = (int)_textSlant;
		break;
	case kTheScrollTop:
		d = _scroll;
		break;
	default:
		d = CastMember::getField(field);
	}

	return d;
}

bool TextCastMember::setField(int field, const Datum &d) {
	Channel *toEdit = nullptr;

	if (field == kTheTextFont || field == kTheTextSize || field == kTheTextStyle) {
		Common::Array<Channel *> channels = g_director->getCurrentMovie()->getScore()->_channels;
		for (uint i = 0; i < channels.size(); i++) {
			if (channels[i]->_sprite->_cast == this) {
				toEdit = channels[i];
				break;
			}
		}
		if (toEdit) {
			Common::Rect bbox = toEdit->getBbox();
			if (!toEdit->_widget)
				toEdit->_widget = createWidget(bbox, toEdit, toEdit->_sprite->_spriteType);
		}
	}

	switch (field) {
	case kTheBackColor:
		{
			uint32 color = g_director->transformColor(d.asInt());
			setColors(nullptr, &color);
		}
		return true;
	case kTheForeColor:
		{
			uint32 color = g_director->transformColor(d.asInt());
			setColors(&color, nullptr);
		}
		return true;
	case kTheHilite:
		// TODO: Understand how texts can be selected programmatically as well.
		// since hilite won't affect text castmember, and we may have button info in text cast in D2/3. so don't check type here
		_hilite = (bool)d.asInt();
		_modified = true;
		return true;
		break;
	case kTheText:
		setRawText(d.asString());
		return true;
	case kTheTextAlign:
		{
			Common::String select = d.asString(true);
			select.toLowercase();

			TextAlignType align;
			if (select == "\"left\"") {
				align = kTextAlignLeft;
			} else if (select == "\"center\"") {
				align = kTextAlignCenter;
			} else if (select == "\"right\"") {
				align = kTextAlignRight;
			} else {
				warning("TextCastMember::setField(): Unknown text align spec: %s", d.asString(true).c_str());
				break;
			}

			_textAlign = align;
			_modified = true;
	}
		return true;
	case kTheTextFont:
		if (!toEdit) {
			warning("Channel containing this CastMember %d doesn't exist", (int) _castId);
			return false;
		}
		((Graphics::MacText *)toEdit->_widget)->enforceTextFont((uint16) g_director->_wm->_fontMan->getFontIdByName(d.asString()));
		_ptext = ((Graphics::MacText *)toEdit->_widget)->getPlainText();
		_ftext = ((Graphics::MacText *)toEdit->_widget)->getTextChunk(0, 0, -1, -1, true);
		return true;
	case kTheTextHeight:
		_lineSpacing = d.asInt();
		_modified = true;
		return false;
	case kTheTextSize:
		if (!toEdit) {
			warning("Channel containing this CastMember %d doesn't exist", (int) _castId);
			return false;
		}
		((Graphics::MacText *)toEdit->_widget)->setTextSize(d.asInt());
		_ptext = ((Graphics::MacText *)toEdit->_widget)->getPlainText();
		_ftext = ((Graphics::MacText *)toEdit->_widget)->getTextChunk(0, 0, -1, -1, true);
		return true;
	case kTheTextStyle:
		if (!toEdit) {
			warning("Channel containing this CastMember %d doesn't exist", (int) _castId);
			return false;
		}
		{
			int slant = g_director->_wm->_fontMan->parseSlantFromName(d.asString());
			((Graphics::MacText *)toEdit->_widget)->enforceTextSlant(slant);
		}
		_ptext = ((Graphics::MacText *)toEdit->_widget)->getPlainText();
		_ftext = ((Graphics::MacText *)toEdit->_widget)->getTextChunk(0, 0, -1, -1, true);
		return true;
	case kTheScrollTop:
		_scroll = d.asInt();
		return true;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

bool TextCastMember::hasChunkField(int field) {
	switch (field) {
	case kTheForeColor:
	case kTheTextFont:
	case kTheTextHeight:
	case kTheTextSize:
	case kTheTextStyle:
		return true;
	default:
		break;
	}
	return false;
}

Datum TextCastMember::getChunkField(int field, int start, int end) {
	Datum d;

	Graphics::MacText *macText = ((Graphics::MacText *)_widget);
	if (!_widget)
		warning("TextCastMember::getChunkField getting chunk field when there is no linked widget, returning the default value");

	switch (field) {
	case kTheForeColor:
		if (_widget)
			d.u.i = macText->getTextColor(start, end);
		else
			d.u.i = getForeColor();
		break;
	case kTheTextFont: {
		int fontId;
		if (_widget)
			fontId = macText->getTextFont(start, end);
		else
			fontId = _fontId;

		d.type = STRING;
		d.u.s = new Common::String(g_director->_wm->_fontMan->getFontName(fontId));
		break;
		}
	case kTheTextHeight:
		warning("TextCastMember::getChunkField getting text height(line spacing) is not implemented yet, returning the default one");
		d.u.i = _lineSpacing;
		break;
	case kTheTextSize:
		if (_widget)
			d.u.i = macText->getTextSize(start, end);
		else
			d.u.i = _fontSize;
		break;
	case kTheTextStyle:
		if (_widget)
			d.u.i = macText->getTextSlant(start, end);
		else
			d.u.i = _textSlant;
		break;
	default:
		break;
	}

	return d;
}

bool TextCastMember::setChunkField(int field, int start, int end, const Datum &d) {
	Graphics::MacText *macText = ((Graphics::MacText *)_widget);
	if (!_widget)
		warning("TextCastMember::setChunkField setting chunk field when there is no linked widget");

	switch (field) {
	case kTheForeColor:
		if (_widget)
			macText->setTextColor(d.asInt(), start, end);
		return true;
	case kTheTextFont:
		if (_widget)
			macText->setTextFont(d.asInt(), start, end);
		return true;
	case kTheTextHeight:
		warning("TextCastMember::setChunkField setting text height(line spacing) is not implemented yet");
		return false;
	case kTheTextSize:
		if (_widget)
			macText->setTextSize(d.asInt(), start, end);
		return true;
	case kTheTextStyle:
		if (_widget)
			macText->setTextSlant(d.asInt(), start, end);
		return true;
	default:
		break;
	}

	return false;
}

RTECastMember::RTECastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: TextCastMember(cast, castId, stream, version) {

	_type = kCastRTE;
}

void RTECastMember::load() {
	if (_loaded)
		return;

	uint rte0id = 0;
	uint rte1id = 0;
	uint rte2id = 0;
	for (auto &it : _children) {
		if (it.tag == MKTAG('R', 'T', 'E', '0')) {
			rte0id = it.index;
			break;
		} else if (it.tag == MKTAG('R', 'T', 'E', '1')) {
			rte1id = it.index;
			break;
		} else if (it.tag == MKTAG('R', 'T', 'E', '2')) {
			rte2id = it.index;
			break;
		}
	}

	if (_cast->_loadedRTE0s.contains(rte0id)) {
		// TODO: Copy the formatted text data
	} else {
		warning("RTECastMember::load(): rte0tid %i isn't loaded", rte0id);
	}
	if (_cast->_loadedRTE1s.contains(rte1id)) {
		// TODO: Copy the plain text data
	} else {
		warning("RTECastMember::load(): rte1tid %i isn't loaded", rte1id);
	}
	if (_cast->_loadedRTE2s.contains(rte2id)) {
		// TODO: Copy the bitmap data
	} else {
		warning("RTECastMember::load(): rte2tid %i isn't loaded", rte2id);
	}

	_loaded = true;
}

}
