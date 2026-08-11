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

#include "common/endian.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/textxtra.h"
#include "director/window.h"

namespace Director {

namespace TextXtra {

static bool isHexDigit(byte c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint hexValue(byte c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return c - 'a' + 10;
}

// XMED payload is an ASCII-hex serialized Hermes-Paige document:
// "FFFF" header, then 0x00 <hex length> ',' <text>, then style runs.
bool decodeXMED(const Common::Array<byte> &data, Common::String &text) {
	uint32 size = data.size();
	if (size < 4 || memcmp(data.data(), "FFFF", 4) != 0)
		return false;

	for (uint32 i = 0; i + 2 < size; i++) {
		if (data[i] != 0x00)
			continue;

		uint32 j = i + 1;
		uint32 len = 0;
		int digits = 0;
		while (j < size && isHexDigit(data[j]) && digits < 6) {
			len = len * 16 + hexValue(data[j]);
			j++;
			digits++;
		}

		if (digits == 0 || len == 0 || j >= size || data[j] != ',')
			continue;
		j++;

		if (j + len > size)
			continue;

		uint32 printable = 0;
		for (uint32 k = 0; k < len; k++) {
			byte c = data[j + k];
			if ((c >= 0x20 && c <= 0x7e) || c >= 0x80 || c == '\r' || c == '\n' || c == '\t')
				printable++;
		}

		if (printable * 5 < len * 4)
			continue;

		text = Common::String((const char *)&data[j], len);
		return true;
	}
	return false;
}

// 76-byte "text" payload observed in Physikus (D7): BE32 height at
// offset 36, width at offset 40, mirroring the authored xtraRect.
bool parseXtraData(const Common::Array<byte> &data, Common::Rect &rect) {
	if (data.size() < 44)
		return false;

	int32 h = READ_BE_INT32(&data[36]);
	int32 w = READ_BE_INT32(&data[40]);
	if (w <= 0 || h <= 0 || w > 0x4000 || h > 0x4000)
		return false;

	rect = Common::Rect((int16)w, (int16)h);
	return true;
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new TextXtraCastMember(cast, castId, *xtra);
}

} // End of namespace TextXtra

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	// Not kCastText: the engine casts kCastText members to TextCastMember
	_type = kCastXtra;

	Common::Rect rect;
	if (TextXtra::parseXtraData(source.getXtraData(), rect))
		_initialRect = rect;
}

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, TextXtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_text = source._text;
	_loaded = source._loaded;
	if (cast == source._cast)
		_children = source._children;
}

void TextXtraCastMember::load() {
	if (_loaded)
		return;

	for (auto &it : _children) {
		if (it.tag != MKTAG('X', 'M', 'E', 'D'))
			continue;

		if (!_cast->getArchive()->hasResource(it.tag, it.index)) {
			warning("TextXtraCastMember::load(): XMED %d not found", it.index);
			break;
		}
		Common::SeekableReadStreamEndian *r = _cast->getArchive()->getResource(it.tag, it.index);
		Common::Array<byte> data(r->size());
		r->read(data.data(), r->size());
		delete r;

		Common::String text;
		if (TextXtra::decodeXMED(data, text)) {
			_text = Common::U32String(text, g_director->getPlatformEncoding());
			debugC(3, kDebugText, "TextXtraCastMember::load(): XMED %d: '%s'", it.index, text.c_str());
		} else {
			warning("TextXtraCastMember::load(): XMED %d not decoded", it.index);
		}
		break;
	}

	_loaded = true;
}

Graphics::MacWidget *TextXtraCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// TODO: Parse the authored Paige style runs (font, size, colours)
	Graphics::MacFont macFont(1, 12);
	Graphics::MacText *widget = new Graphics::MacText(g_director->getCurrentWindow()->getMacWindow(),
			bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, _text, &macFont,
			g_director->_wm->_colorWhite, g_director->_wm->_colorBlack, _initialRect.width(),
			Graphics::kTextAlignLeft);
	widget->draw();
	return widget;
}

bool TextXtraCastMember::hasField(int field) {
	switch (field) {
	case kTheText:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum TextXtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCastType:
	case kTheType:
		d = Common::String("text");
		d.type = SYMBOL;
		break;
	case kTheText:
		load();
		d = _text.encode(Common::kUtf8);
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void TextXtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheText:
		_text = Common::U32String(d.asString(), Common::kUtf8);
		_loaded = true;
		setModified(true);
		return;
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String TextXtraCastMember::formatInfo() {
	return Common::String::format("initialRect: %dx%d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(), Common::toPrintable(_text.encode()).c_str());
}

} // End of namespace Director
