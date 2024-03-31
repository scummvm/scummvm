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

#include "dgds/dialog.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/request.h"
#include "dgds/scripts.h"
#include "dgds/font.h"

namespace Dgds {

// TODO: This is repeated here and in scene.cpp
template<class S> static Common::String _dumpStructList(const Common::String &indent, const Common::String &name, const Common::Array<S> &list) {
	if (list.empty())
		return "";

	const Common::String nextind = indent + "    ";
	Common::String str = Common::String::format("\n%s%s=", Common::String(indent + "  ").c_str(), name.c_str());
	for (const auto &s : list) {
		str += "\n";
		str += s.dump(nextind);
	}
	return str;
}


int Dialog::_lastSelectedDialogItemNum = 0;
Dialog *Dialog::_lastDialogSelectionChangedFor = nullptr;


Dialog::Dialog() : _num(0), _bgColor(0), _fontColor(0), _selectionBgCol(0), _selectonFontCol(0),
	_fontSize(0), _flags(kDlgFlagNone), _frameType(kDlgFramePlain), _time(0), _nextDialogNum(0),
	_field18_0x28(0)
{}


void Dialog::draw(Graphics::ManagedSurface *dst, DialogDrawStage stage) {
	if (!_state)
		_state.reset(new DialogState());

	switch (_frameType) {
	case kDlgFramePlain: 	return drawType1(dst, stage);
	case kDlgFrameBorder: 	return drawType2(dst, stage);
	case kDlgFrameThought: 	return drawType3(dst, stage);
	case kDlgFrameRounded: 	return drawType4(dst, stage);
	default: error("unexpected frame type %d for dialog %d", _frameType, _num);
	}
}

static void _drawPixel(int x, int y, int color, void *data) {
	Graphics::ManagedSurface *surface = (Graphics::ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
		*((byte *)surface->getBasePtr(x, y)) = (byte)color;
}


const Font *Dialog::getDlgTextFont() const {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const FontManager *fontman = engine->getFontMan();
	FontManager::FontType fontType = FontManager::kGameDlgFont;
	if (_fontSize == 1)
		fontType = FontManager::k8x8Font;
	else if (_fontSize == 3)
		fontType = FontManager::k4x5Font;
	return fontman->getFont(fontType);
}

//  box with simple frame
void Dialog::drawType1(Graphics::ManagedSurface *dst, DialogDrawStage stage) {
	if (!_state)
		return;
	int x = _rect.x;
	int y = _rect.y;
	int w = _rect.width;
	int h = _rect.height;

	if (stage == kDlgDrawStageBackground) {
		dst->fillRect(Common::Rect(x, y, x + w, y + h), _bgColor);
		dst->fillRect(Common::Rect(x + 1, y + 1, x + w - 1, y + h - 1), _fontColor);
	} else if (stage == kDlgDrawFindSelectionPointXY) {
		drawFindSelectionXY();
	} else if (stage == kDlgDrawFindSelectionTxtOffset) {
		drawFindSelectionTxtOffset();
	} else {
		_state->_loc = DgdsRect(x + 3, y + 3, w - 6, h - 6);
		drawForeground(dst, _bgColor, _str);
	}
}

// box with fancy frame and optional title (everything before ":")
void Dialog::drawType2(Graphics::ManagedSurface *dst, DialogDrawStage stage) {
	if (!_state)
		return;

	Common::String title;
	Common::String txt;
	uint32 colonpos = _str.find(':');
	if (colonpos != Common::String::npos) {
		title = _str.substr(0, colonpos);
		txt = _str.substr(colonpos + 1);
	} else {
		txt = _str;
	}

	if (stage == kDlgDrawStageBackground) {
		_state->_loc = DgdsRect(_rect.x + 6, _rect.y + 6, _rect.width - 12, _rect.height - 12);
		Common::Rect drawRect(_rect.x, _rect.y, _rect.x + _rect.width, _rect.y + _rect.height);
		RequestData::fillBackground(dst, _rect.x, _rect.y, _rect.width, _rect.height, 0);
		RequestData::drawCorners(dst, 11, _rect.x, _rect.y, _rect.width, _rect.height);
		if (!title.empty()) {
			// TODO: Maybe should measure the font?
			_state->_loc.y += 10;
			RequestData::drawHeader(dst, _rect.x, _rect.y, _rect.width, 4, title);
		}

		if (hasFlag(kDlgFlagFlatBg)) {
			Common::Rect fr = _state->_loc.toCommonRect();
			dst->fillRect(fr, 0);
		} else {
			RequestData::fillBackground(dst, _state->_loc.x, _state->_loc.y, _state->_loc.width, _state->_loc.height, 6);
		}

		RequestData::drawCorners(dst, 19, _state->_loc.x - 2, _state->_loc.y - 2,
								_state->_loc.width + 4, _state->_loc.height + 4);

		_state->_loc.x += 8;
		_state->_loc.y -= 8;
	} else if (stage == kDlgDrawFindSelectionPointXY) {
		drawFindSelectionXY();
	} else if (stage == kDlgDrawFindSelectionTxtOffset) {
		drawFindSelectionTxtOffset();
	} else {
		drawForeground(dst, _fontColor, txt);
	}
}

static void _filledCircle(int x, int y, int xr, int yr, Graphics::ManagedSurface *dst, byte fgcol, byte bgcol) {
	Graphics::drawEllipse(x - xr, y - yr, x + xr, y + yr, bgcol, true, _drawPixel, dst);
	Graphics::drawEllipse(x - xr, y - yr, x + xr, y + yr, fgcol, false, _drawPixel, dst);
}

// Comic tought box made up of circles with 2 circles going up to it.
// Draw circles with 5/4 more pixels in x because the pixels are not square.
void Dialog::drawType3(Graphics::ManagedSurface *dst, DialogDrawStage stage) {
	if (!_state)
		return;

	if (stage == kDlgDrawStageBackground) {
		uint16 xradius = 9999;
		uint16 yradius = 40;
		const int16 usabley = _rect.height - 31;
		const int16 usablex = _rect.width - 30;
		for (uint16 testyradius = 40; testyradius != 0; testyradius--) {
			int16 testxradius = (testyradius * 5) / 4;
			if ((usablex / testxradius > 2) && (usabley / testyradius > 2)) {
				testxradius = usablex % testxradius + usabley % testyradius;
				if (testxradius < xradius) {
					yradius = testyradius;
					xradius = testxradius;
				}
			}
			if (testyradius < 20 && xradius != 9999)
				break;
		}

		xradius = (yradius * 5) / 4;
		const int16 circlesAcross = usablex / xradius - 1;
		const int16 circlesDown = usabley / yradius - 1;

		uint16 x = _rect.x + xradius;
		uint16 y = _rect.y + yradius;

		bool isbig = _rect.x + _rect.width / 2 > 160;
		if (isbig)
			x = x + 30;

		byte fgcol = 0;
		byte bgcol = 15;
		if (hasFlag(kDlgFlagFlatBg)) {
			bgcol = _bgColor;
			fgcol = _fontColor;
		}

		for (int i = 1; i < circlesDown; i++) {
			_filledCircle(x, y, xradius, yradius, dst, fgcol, bgcol);
			y += yradius;
		}
		for (int i = 1; i < circlesAcross; i++) {
			_filledCircle(x, y, xradius, yradius, dst, fgcol, bgcol);
			x += xradius;
		}
		for (int i = 1; i < circlesDown; i++) {
			_filledCircle(x, y, xradius, yradius, dst, fgcol, bgcol);
			y -= yradius;
		}
		for (int i = 1; i < circlesAcross; i++) {
			_filledCircle(x, y, xradius, yradius, dst, fgcol, bgcol);
			x -= xradius;
		}

		uint16 smallCircleX;
		if (isbig) {
			_filledCircle((x - xradius) - 5, y + circlesDown * yradius + 5, 10, 8, dst, fgcol, bgcol);
			smallCircleX = (x - xradius) - 20;
		} else {
			_filledCircle(x + circlesAcross * xradius + 5, y + circlesDown * yradius + 5, 10, 8, dst, fgcol, bgcol);
			smallCircleX = x + circlesAcross * xradius + 20;
		}

		_filledCircle(smallCircleX, y + circlesDown * yradius + 25, 5, 4, dst, fgcol, bgcol);

		int16 yoff = (yradius * 27) / 32;
		dst->fillRect(Common::Rect(x, y - yoff,
					x + (circlesAcross - 1) * xradius + 1,
					y + (circlesDown - 1) * yradius + yoff + 1), bgcol);
		int16 xoff = (xradius * 27) / 32;
		dst->fillRect(Common::Rect(x - xoff, y,
					x + (circlesAcross - 1) * xradius + xoff + 1,
					y + (circlesDown - 1) * yradius + 1), bgcol);

		int16 textRectX = x - xradius / 2;
		int16 textRectY = y - yradius / 2;
		assert(_state);
		_state->_loc = DgdsRect(textRectX, textRectY, circlesAcross * xradius , circlesDown * yradius);
	} else if (stage == kDlgDrawFindSelectionPointXY) {
		drawFindSelectionXY();
	} else if (stage == kDlgDrawFindSelectionTxtOffset) {
		drawFindSelectionTxtOffset();
	} else {
		drawForeground(dst, _fontColor, _str);
	}
}

// ellipse
void Dialog::drawType4(Graphics::ManagedSurface *dst, DialogDrawStage stage) {
	if (!_state)
		return;

	int x = _rect.x;
	int y = _rect.y;
	int w = _rect.width;
	int h = _rect.height;

	int midy = (h - 1) / 2;
	byte fillcolor;
	byte fillbgcolor;
	if (!hasFlag(kDlgFlagFlatBg)) {
		fillcolor = 0;
		fillbgcolor = 15;
	} else {
		fillcolor = _fontColor;
		fillbgcolor = _bgColor;
	}

	if (stage == kDlgDrawStageBackground) {
		//int radius = (midy * 5) / 4;

		// This is not exactly the same as the original - might need some work to get pixel-perfect
		Common::Rect drawRect(x, y, x + w, y + h);
		Graphics::drawRoundRect(drawRect, midy, fillbgcolor, true, _drawPixel, dst);
		Graphics::drawRoundRect(drawRect, midy, fillcolor, false, _drawPixel, dst);
	} else if (stage == kDlgDrawFindSelectionPointXY) {
		drawFindSelectionXY();
	} else if (stage == kDlgDrawFindSelectionTxtOffset) {
		drawFindSelectionTxtOffset();
	} else {
		assert(_state);
		_state->_loc = DgdsRect(x + midy, y + 1, w - midy, h - 1);
		drawForeground(dst, fillcolor, _str);
	}
}

void Dialog::drawFindSelectionXY() {
	if (!_state)
		return;

	const Font *font = getDlgTextFont();

	// Find the appropriate _lastMouseX/lastMouseY value given the last _strMouseLoc.

	int x = _state->_loc.x;
	_state->_lastMouseX = x;
	int y = _state->_loc.y + 1;
	_state->_lastMouseY = y;
	_state->_charWidth = font->getMaxCharWidth();
	_state->_charHeight = font->getFontHeight();
	if (_state->_strMouseLoc) {
		Common::Array<Common::String> lines;
		int maxWidth = font->wordWrapText(_str, _state->_loc.width, lines);

		if (hasFlag(kDlgFlagLeftJust)) {
			x = x + (_state->_loc.width - maxWidth - 1) / 2;
			_state->_lastMouseX = x;
			y = y + (_state->_loc.height - (lines.size() * _state->_charHeight) - 1) / 2;
			_state->_lastMouseY = y;
		}

		if (_state->_strMouseLoc >= (int)_str.size())
			_state->_strMouseLoc = _str.size() - 1;

		// Find the location of the mouse loc in the wrapped string.
		int totalchars = 0;
		for (uint lineno = 0; lineno < lines.size(); lineno++) {
			// +1 char for the space or CR that caused the wrap.
			int nexttotalchars = totalchars + lines[lineno].size() + 1;
			if (nexttotalchars > _state->_strMouseLoc)
				break;
			totalchars = nexttotalchars;
			y += _state->_charHeight;
		}

		// now get width of the remaining string to the mouse str offset
		x += font->getStringWidth(_str.substr(totalchars, _state->_strMouseLoc - totalchars));

		// TODO: does this make sense?
		if (_state->_loc.x + _state->_loc.width < (x + font->getCharWidth(_state->_strMouseLoc))) {
			if (_str[_state->_strMouseLoc] < '!') {
				_state->_charHeight = 0;
				_state->_charWidth = 0;
				_state->_lastMouseY = 0;
				_state->_lastMouseX = 0;
				return;
			}
			x = _state->_loc.x;
			y += _state->_charHeight;
		}

		_state->_lastMouseX = x;
		_state->_lastMouseY = y;
		_state->_charWidth = font->getCharWidth(_state->_strMouseLoc);
	}
}

void Dialog::drawFindSelectionTxtOffset() {
	if (!_state)
		return;

	// Find the appropriate _strMouseLoc value given the last x/y position.

	const Font *font = getDlgTextFont();
	int lastMouseX = _state->_lastMouseX;
	int lastMouseY = _state->_lastMouseY;
	int lineHeight = font->getFontHeight();
	int dlgx = _state->_loc.x;
	int dlgy = _state->_loc.y;

	Common::Array<Common::String> lines;
	int maxWidth = font->wordWrapText(_str, dlgy, lines);

	if (hasFlag(kDlgFlagLeftJust)) {
		dlgx += (_state->_loc.width - maxWidth - 1) / 2;
		dlgy += (_state->_loc.height - (lines.size() * lineHeight) - 1) / 2;
	}

	uint lineno;
	uint totalchars = 0;
	for (lineno = 0; lineno < lines.size() && dlgy + lineHeight < lastMouseY; lineno++) {
		totalchars += lines[lineno].size() + 1;
		dlgy = dlgy + lineHeight;
	}

	int startx = dlgx;
	while (lineno < lines.size()) {
		const Common::String &line = lines[lineno];
		for (uint charno = 0; charno < line.size(); charno++) {
			int charwidth = font->getCharWidth(line[charno]);
			if (lastMouseX <= dlgx + charwidth) {
				_state->_strMouseLoc = totalchars + charno;
				return;
			}
			dlgx += charwidth;
		}
		dlgx = startx;
		totalchars += line.size() + 1;
		lineno++;
	}

	_state->_strMouseLoc = _str.size();
	return;
}

void Dialog::drawForeground(Graphics::ManagedSurface *dst, uint16 fontcol, const Common::String &txt) {
	// TODO: some more text calcuations happen here.
	// This is where we actually draw the text.
	// For now do the simplest wrapping, no highlighting.
	assert(_state);

	Common::StringArray lines;
	const Font *font = getDlgTextFont();
	const int h = font->getFontHeight();
	font->wordWrapText(txt, _state->_loc.width, lines);

	int ystart = _state->_loc.y + (_state->_loc.height - lines.size() * h) / 2;

	int x = _state->_loc.x;
	if (hasFlag(kDlgFlagLeftJust)) {
		// each line left-aligned, but overall block is still centered
		int maxlen = -1;
		for (const auto &line : lines)
			maxlen = MAX(maxlen, font->getStringWidth(line));

		x += (_state->_loc.width - maxlen) / 2;

		for (uint i = 0; i < lines.size(); i++)
			font->drawString(dst, lines[i], x, ystart + i * h, maxlen, fontcol, Graphics::kTextAlignLeft);
	} else {
		// center each line
		for (uint i = 0; i < lines.size(); i++)
			font->drawString(dst, lines[i], x, ystart + i * h, _state->_loc.width, fontcol, Graphics::kTextAlignCenter);
	}

	if (_state->_selectedAction) {
		warning("TODO: Draw highlight on selected action.");
	}
}

void Dialog::setFlag(DialogFlags flg) {
	_flags = static_cast<DialogFlags>(_flags | flg);
}

void Dialog::clearFlag(DialogFlags flg) {
	_flags = static_cast<DialogFlags>(_flags & ~flg);
}

void Dialog::flipFlag(DialogFlags flg) {
	_flags = static_cast<DialogFlags>(_flags ^ flg);
}

bool Dialog::hasFlag(DialogFlags flg) const {
	return _flags & flg;
}

void Dialog::clear() {
	clearFlag(kDlgFlagHiFinished);
	clearFlag(kDlgFlagRedrawSelectedActionChanged);
	clearFlag(kDlgFlagHi10);
	clearFlag(kDlgFlagHi20);
	clearFlag(kDlgFlagHi40);
	clearFlag(kDlgFlagVisible);
	_state.reset();
}

void Dialog::updateSelectedAction(int delta) {
	if (!_lastDialogSelectionChangedFor)
		_lastDialogSelectionChangedFor = this;
	else
		_lastSelectedDialogItemNum = 0;

	if (!_state)
		return;

	if (_state->_selectedAction) {
		for (uint i = 0; i < _action.size(); i++) {
			if (_state->_selectedAction == &_action[i]) {
				_lastSelectedDialogItemNum = i;
				break;
			}
		}
	}
	_lastSelectedDialogItemNum += delta;
	if (!_action.empty()) {
		while (_lastSelectedDialogItemNum < 0)
			_lastSelectedDialogItemNum += _action.size();
		_lastSelectedDialogItemNum = _lastSelectedDialogItemNum % _action.size();
	}
	_lastDialogSelectionChangedFor = this;

	int mouseX = _state->_loc.x + _state->_loc.width;
	int mouseY = _state->_loc.y + _state->_loc.height - 2;
	if (_action.size() > 1) {
		_state->_strMouseLoc = _action[_lastSelectedDialogItemNum].strStart;
		draw(nullptr, kDlgDrawFindSelectionPointXY);
		// Move the mouse over the selected item
		mouseY = _state->_lastMouseY + _state->_charHeight / 2;
	}

	if (_action.size() > 1 || !delta) {
		g_system->warpMouse(mouseX, mouseY);
	}
}

struct DialogAction *Dialog::pickAction(bool isClosing) {
	struct DialogAction *retval = nullptr;
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (/* some game flags?? && */isClosing) {
		return &_action[engine->getRandom().getRandomNumber(_action.size() - 1)];
	}
	assert(_state);
	const Common::Point lastMouse = engine->getLastMouse();
	if (_state->_loc.x <= lastMouse.x &&
		_state->_loc.x + _state->_loc.width <= lastMouse.x &&
		_state->_loc.y <= lastMouse.y &&
		_state->_loc.y + _state->_loc.height <= lastMouse.y) {
		_state->_lastMouseX = lastMouse.x;
		_state->_lastMouseY = lastMouse.y;
		draw(nullptr, kDlgDrawFindSelectionTxtOffset);

		char underMouse;
		if (_state->_strMouseLoc >= 0 && _state->_strMouseLoc < (int)_str.size())
			underMouse = _str[_state->_strMouseLoc];
		else
			underMouse = '\0';

		for (auto &action : _action) {
			if ((action.strStart <= _state->_strMouseLoc && _state->_strMouseLoc <= action.strEnd) ||
				(_state->_strMouseLoc == action.strEnd + 1 && underMouse == '\r' && _str[action.strEnd] != '\r')) {
				return &action;
			}
		}
	}
	return retval;
}


Common::String Dialog::dump(const Common::String &indent) const {
	Common::String str = Common::String::format(
			"%sDialog<num %d %s bgcol %d fcol %d selbgcol %d selfontcol %d fntsz %d flags 0x%02x frame %d delay %d next %d unk18 %d",
			indent.c_str(), _num, _rect.dump("").c_str(), _bgColor, _fontColor, _selectionBgCol, _selectonFontCol, _fontSize,
			_flags, _frameType, _time, _nextDialogNum, _field18_0x28);
	str += indent + "state=" + (_state ? _state->dump("") : "null");
	str += "\n";
	str += _dumpStructList(indent, "actions", _action);
	str += "\n";
	str += indent + "  str='" + _str + "'>";
	return str;
}

Common::String DialogState::dump(const Common::String &indent) const {
	return Common::String::format("%sDialogState<hide %d loc %s lastmouse %d %d charsz %d %d mousestr %d selaction %p>",
			indent.c_str(), _hideTime, _loc.dump("").c_str(), _lastMouseX, _lastMouseY, _charWidth,
			_charHeight, _strMouseLoc, (void *)_selectedAction);
}

Common::String DialogAction::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sDialogueAction<%d span: %d-%d", indent.c_str(), val, strStart, strEnd);
	str += _dumpStructList(indent, "opList", sceneOpList);
	if (!sceneOpList.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}

} // End of namespace Dgds
