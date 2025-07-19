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
#include "common/substream.h"
#include "common/macresman.h"
#include "common/memstream.h"

#include "graphics/macgui/macbutton.h"
#include "graphics/macgui/macwindow.h"

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

	_borderSize = 0;
	_gutterSize = 0;
	_boxShadow = 0;
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
	_textShadow = 0;
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
		_borderSize = stream.readByte();
		_gutterSize = stream.readByte();
		_boxShadow = stream.readByte();
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

			_textShadow = stream.readByte();
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
		_borderSize = stream.readByte();
		_gutterSize = stream.readByte();
		_boxShadow = stream.readByte();
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();
		_scroll = stream.readUint16();

		_fontId = 1; // this is in STXT

		_initialRect = Movie::readRect(stream);
		_maxHeight = stream.readUint16();
		_textShadow = stream.readByte();
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
	if (cast == source._cast)
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
	Graphics::MacText *target = getWidget();
	if (target) {
		target->setColors(_fgcolor, _bgcolor);
	} else {
		_modified = true;
	}
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

uint32 TextCastMember::getForeColor(int start, int end) {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getTextColor(start, end);
	}
	return _fgcolor;
}

void TextCastMember::setForeColor(uint32 fgCol) {
	_fgcolor = fgCol;
	_modified = true;
}

void TextCastMember::setForeColor(uint32 fgCol, int start, int end) {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->setTextColor(fgCol, start, end);
	}
	_modified = true;
}


void TextCastMember::importStxt(const Stxt *stxt) {
	_fontId = stxt->_style.fontId;
	_height = stxt->_style.height;
	_ascent = stxt->_style.ascent;
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

Graphics::MacWidget *TextCastMember::createWindowOrWidget(Common::Rect &bbox, Common::Rect dims, Graphics::MacFont *macFont) {
	Graphics::MacText *widget = nullptr;

	widget = new Graphics::MacText(g_director->getCurrentWindow(), bbox.left, bbox.top, dims.width(), dims.height(), g_director->_wm, _ftext, macFont, getForeColor(), getBackColor(), _initialRect.width(), getAlignment(), _lineSpacing, _borderSize, _gutterSize, _boxShadow, _textShadow, _textType == kTextTypeFixed || _textType == kTextTypeScrolling, _textType == kTextTypeScrolling);
	widget->setSelRange(g_director->getCurrentMovie()->_selStart, g_director->getCurrentMovie()->_selEnd);
	widget->draw();

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
		} else if (_textType == kTextTypeFixed || _textType == kTextTypeScrolling) {
			// use initialRect to create widget for fixed style text, this maybe related to version.
			dims.right = MIN<int>(dims.right, dims.left + _initialRect.width());
			dims.bottom = MIN<int>(dims.bottom, dims.top + MAX<int>(_initialRect.height(), _maxHeight));
		}
		widget = createWindowOrWidget(bbox, dims, macFont);
		if (_textType != kTextTypeScrolling) {
			((Graphics::MacText *)widget)->setEditable(channel->_sprite->_editable);
		}

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

Graphics::MacText *TextCastMember::getWidget() {
	// FIXME: The cast member should be the source of truth for the widget.
	// You don't have the issue you have with e.g. bitmaps where the channel
	// can stretch: all sprites of the cast member have the same dimensions.
	// There is technically a small window between typing something in and hitting
	// enter/defocusing where other copies of the widget are out of sync,
	// but they will resync pretty quickly.
	Channel *toEdit = nullptr;
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
		return (Graphics::MacText *)toEdit->_widget;
	}
	return (Graphics::MacText *)_widget;
}

CollisionTest TextCastMember::isWithin(const Common::Rect &bbox, const Common::Point &pos, InkType ink) {
	if (!bbox.contains(pos))
		return kCollisionNo;

	Graphics::MacText *target = getWidget();
	if (!target)
		return kCollisionYes;

	Graphics::MacWindowConstants::WindowClick result = target->isInScrollBar(pos.x, pos.y);
	if (result == Graphics::MacWindowConstants::kBorderScrollDown ||
			result == Graphics::MacWindowConstants::kBorderScrollUp)
		return kCollisionHole;

	return kCollisionYes;
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

int TextCastMember::getLineCount() {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getRowCount();
	}
	warning("TextCastMember::getLineCount(): no widget available, returning 0");
	return 0;
}

int TextCastMember::getLineHeight(int line) {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getLineHeight(line);
	}
	warning("TextCastMember::getLineHeight(): no widget available, returning 0");
	return 0;
}

// D4 dictionary book said this is line spacing
int TextCastMember::getTextHeight() {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getLineSpacing();
	}
	return _lineSpacing;
}

Common::String TextCastMember::getTextFont() {
	Graphics::MacText *target = getWidget();
	if (target) {
		int fontId = target->getTextFont();
		return g_director->_wm->_fontMan->getFontName(fontId);
	}
	return g_director->_wm->_fontMan->getFontName(_fontId);
}

Common::String TextCastMember::getTextFont(int start, int end) {
	Graphics::MacText *target = getWidget();
	if (target) {
		int fontId = target->getTextFont(start, end);
		return g_director->_wm->_fontMan->getFontName(fontId);
	}
	return g_director->_wm->_fontMan->getFontName(_fontId);
}

void TextCastMember::setTextFont(const Common::String &fontName) {
	Graphics::MacText *target = getWidget();
	if (!target)
		return;
	target->enforceTextFont((uint16) g_director->_wm->_fontMan->getFontIdByName(fontName));
	_ptext = target->getPlainText();
	_ftext = target->getTextChunk(0, 0, -1, -1, true);
}

void TextCastMember::setTextFont(const Common::String &fontName, int start, int end) {
	Graphics::MacText *target = getWidget();
	if (!target)
		return;
	target->setTextFont((uint16) g_director->_wm->_fontMan->getFontIdByName(fontName), start, end);
	_ptext = target->getPlainText();
	_ftext = target->getTextChunk(0, 0, -1, -1, true);
}

Common::U32String TextCastMember::getText() {
	return _ptext;
}

Common::String TextCastMember::getRawText() {
	return _rtext;
}

int TextCastMember::getTextSize() {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getTextSize();
	}

	return _fontSize;
}

int TextCastMember::getTextSize(int start, int end) {
	Graphics::MacText *target = getWidget();
	if (target) {
		return target->getTextSize(start, end);
	}

	return _fontSize;
}

void TextCastMember::setTextSize(int textSize) {
	Graphics::MacText *target = getWidget();
	if (target) {
		target->setTextSize(textSize);
		_ptext = target->getPlainText();
		_ftext = target->getTextChunk(0, 0, -1, -1, true);
		target->draw();
	}
	_fontSize = textSize;
	_modified = true;
}

void TextCastMember::setTextSize(int textSize, int start, int end) {
	Graphics::MacText *target = getWidget();
	if (target) {
		target->setTextSize(textSize, start, end);
		_ptext = target->getPlainText();
		_ftext = target->getTextChunk(0, 0, -1, -1, true);
		target->draw();
	}
	_modified = true;
}

Common::String TextCastMember::getTextStyle() {
	int slantVal = _textSlant;
	Graphics::MacText *target = getWidget();
	if (target) {
		slantVal = target->getTextSlant();
	}
	return g_director->_wm->_fontMan->getNameFromSlant(slantVal);
}

Common::String TextCastMember::getTextStyle(int start, int end) {
	int slantVal = _textSlant;
	Graphics::MacText *target = getWidget();
	if (target) {
		slantVal = target->getTextSlant(start, end);
	}
	return g_director->_wm->_fontMan->getNameFromSlant(slantVal);
}

void TextCastMember::setTextStyle(const Common::String &textStyle) {
	Graphics::MacText *target = getWidget();
	int slant = g_director->_wm->_fontMan->parseSlantFromName(textStyle);
	if (target) {
		target->enforceTextSlant(slant);
		_ptext = target->getPlainText();
		_ftext = target->getTextChunk(0, 0, -1, -1, true);
		target->draw();
	}
	_modified = true;
}

void TextCastMember::scrollByLine(int count) {
	Graphics::MacText *target = getWidget();
	target->scroll(count);
}

void TextCastMember::setTextStyle(const Common::String &textStyle, int start, int end) {
	Graphics::MacText *target = getWidget();
	int slant = g_director->_wm->_fontMan->parseSlantFromName(textStyle);
	if (target) {
		target->setTextSlant(slant, start, end);
		_ptext = target->getPlainText();
		_ftext = target->getTextChunk(0, 0, -1, -1, true);
		target->draw();
	}
	_modified = true;
}

void TextCastMember::updateFromWidget(Graphics::MacWidget *widget, bool spriteEditable) {
	if (widget && (spriteEditable || _editable)) {
		Common::String content = ((Graphics::MacText *)widget)->getEditedString();
		content.replace('\n', '\r');
		_ptext = content;
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
	case kTheText:
	case kTheTextAlign:
	case kTheTextFont:
	case kTheTextHeight:
	case kTheTextSize:
	case kTheTextStyle:
		return true;
	case kTheAutoTab:
	case kTheBorder:
	case kTheBoxDropShadow:
	case kTheBoxType:
	case kTheDropShadow:
	case kTheEditable:
	case kTheLineCount:
	case kTheMargin:
	case kThePageHeight:
	case kTheScrollTop:
	case kTheWordWrap:
		return _type == kCastText;
	case kTheButtonType:
		return _type == kCastButton;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum TextCastMember::getField(int field) {
	Datum d;

	switch (field) {
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
		d = getTextFont();
		break;
	case kTheTextHeight:
		d = getTextHeight();
		break;
	case kTheTextSize:
		d = getTextSize();
		break;
	case kTheTextStyle:
		d = getTextStyle();
		break;
	case kTheAutoTab:
		warning("STUB: TextCastMember::getField(): autoTab not implemented");
		d = 1;
		break;
	case kTheBorder:
		d = _borderSize;
		break;
	case kTheBoxDropShadow:
		warning("STUB: TextCastMember::getField(): boxDropShadow not implemented");
		d = 1;
		break;
	case kTheDropShadow:
		warning("STUB: TextCastMember::getField(): dropShadow not implemented");
		d = 1;
		break;
	case kTheEditable:
		d = (int)_editable;
		break;
	case kTheLineCount:
		d = getLineCount();
		break;
	case kTheMargin:
		warning("STUB: TextCastMember::getField(): margin not implemented");
		d = 0;
		break;
	case kThePageHeight:
		warning("STUB: TextCastMember::getField(): pageHeight not implemented");
		d = 0;
		break;
	case kTheScrollTop:
		d = _scroll;
		break;
	case kTheWordWrap:
		warning("STUB: TextCastMember::getField(): wordWrap not implemented");
		d = 1;
		break;
	case kTheButtonType:
		switch (_buttonType) {
		case kTypeCheckBox:
			d = Datum("checkBox");
			d.type = SYMBOL;
			break;
		case kTypeRadio:
			d = Datum("radioButton");
			d.type = SYMBOL;
			break;
		case kTypeButton:
		default:
			d = Datum("pushButton");
			d.type = SYMBOL;
			break;
		}
		break;
	default:
		d = CastMember::getField(field);
	}

	return d;
}

bool TextCastMember::setField(int field, const Datum &d) {
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
	case kTheText:
		setRawText(d.asString());
		return true;
	case kTheTextAlign:
		{
			Common::String select = d.asString();
			TextAlignType align;
			if (select.equalsIgnoreCase("left")) {
				align = kTextAlignLeft;
			} else if (select.equalsIgnoreCase("center")) {
				align = kTextAlignCenter;
			} else if (select.equalsIgnoreCase("right")) {
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
		setTextFont(d.asString());
		return true;
	case kTheTextHeight:
		_lineSpacing = d.asInt();
		_modified = true;
		return false;
	case kTheTextSize:
		setTextSize(d.asInt());
		return true;
	case kTheTextStyle:
		setTextStyle(d.asString());
		return true;
	case kTheAutoTab:
		warning("STUB: TextCastMember::setField(): autoTab not implemented");
		return false;
	case kTheBorder:
		_borderSize = d.asInt();
		setModified(true);
		return true;
	case kTheBoxDropShadow:
		warning("STUB: TextCastMember::setField(): boxDropShadow not implemented");
		return false;
	case kTheBoxType:
		warning("STUB: TextCastMember::setField(): boxType not implemented");
		return false;
	case kTheDropShadow:
		warning("STUB: TextCastMember::setField(): dropShadow not implemented");
		return false;
	case kTheEditable:
		_editable = d.asInt();
		setModified(true);
		return true;
	case kTheLineCount:
		warning("BUILDBOT: TextCastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->entity2str(field), _castId);
		return false;
	case kTheMargin:
		warning("STUB: TextCastMember::setField(): margin not implemented");
		return false;
	case kThePageHeight:
		warning("BUILDBOT: TextCastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->entity2str(field), _castId);
		return false;
	case kTheScrollTop:
		_scroll = d.asInt();
		setModified(true);
		return true;
	case kTheWordWrap:
		warning("STUB: TextCastMember::setField(): wordWrap not implemented");
		return false;
	case kTheButtonType:
		if (d.type == SYMBOL) {
			if (d.u.s->equalsIgnoreCase("pushButton")) {
				_buttonType = kTypeButton;
				setModified(true);
				return true;
			} else if (d.u.s->equalsIgnoreCase("radioButton")) {
				_buttonType = kTypeRadio;
				setModified(true);
				return true;
			} else if (d.u.s->equalsIgnoreCase("checkBox")) {
				_buttonType = kTypeCheckBox;
				setModified(true);
				return true;
			}
		}
		warning("TextCastMember: invalid button type %s", d.asString(true).c_str());
		return false;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

// This isn't documented particularly well by the Lingo Dictionary;
// as well as letting you read/write properties on the cast member,
// Director allows you to read/write some properties to a subset of the text
// within the cast member defined by a chunk expression, e.g.:
//
// set the textStyle of char 2 to 4 of field "Pudge" to "bold"

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

	switch (field) {
	case kTheForeColor:
		d = (int)getForeColor(start, end);
		break;
	case kTheTextFont:
		d = getTextFont(start, end);
		break;
	case kTheTextHeight:
		warning("TextCastMember::getChunkField(): getting text height(line spacing) is not implemented yet, returning the default one");
		d = (int)_lineSpacing;
		break;
	case kTheTextSize:
		d = getTextSize(start, end);
		break;
	case kTheTextStyle:
		d = getTextStyle(start, end);
		break;
	default:
		break;
	}

	return d;
}

bool TextCastMember::setChunkField(int field, int start, int end, const Datum &d) {

	switch (field) {
	case kTheForeColor:
		setForeColor(d.asInt(), start, end);
		return true;
	case kTheTextFont:
		setTextFont(d.asString(), start, end);
		return true;
	case kTheTextHeight:
		warning("TextCastMember::setChunkField(): setting text height(line spacing) is not implemented yet");
		return false;
	case kTheTextSize:
		setTextSize(d.asInt(), start, end);
		return true;
	case kTheTextStyle:
		setTextStyle(d.asString(), start, end);
		return true;
	default:
		break;
	}

	return false;
}

void TextCastMember::writeCastData(Common::MemoryWriteStream *writeStream) {
	writeStream->writeByte(_borderSize);	// 1 byte
	writeStream->writeByte(_gutterSize);	// 2 bytes
	writeStream->writeByte(_boxShadow);		// 3 bytes
	writeStream->writeByte(_textType);		// 4 bytes
	writeStream->writeSint16LE(_textAlign);		// 6 bytes
	writeStream->writeUint16LE(_bgpalinfo1);	// 8 bytes
	writeStream->writeUint16LE(_bgpalinfo2);	// 10 bytes
	writeStream->writeUint16LE(_bgpalinfo3);	// 12 bytes
	writeStream->writeUint16LE(_scroll);		// 14 bytes

	Movie::writeRect(writeStream, _initialRect);	// (+8) 22 bytes
	writeStream->writeUint16LE(_maxHeight);			// 24 bytes
	writeStream->writeByte(_textShadow);			// 25 bytes
	writeStream->writeByte(_textFlags);				// 26 bytes

	writeStream->writeUint16LE(_textHeight);		// 28 bytes

	if (_type == kCastButton) {
		writeStream->writeUint16LE(_buttonType + 1);		// 30 bytes
	}
}

uint32 TextCastMember::getCastDataSize() {
	// In total 30 bytes for text and 28 for button
	return (_type == kCastButton) ? 30 : 28;
}

}	// End of namespace Director
