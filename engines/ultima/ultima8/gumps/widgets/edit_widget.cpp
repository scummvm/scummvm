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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/widgets/edit_widget.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/misc/encoding.h"
#include "common/system.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(EditWidget, Gump)

EditWidget::EditWidget(int x, int y, Std::string txt, bool gamefont_, int font,
                       int w, int h, unsigned int maxlength_, bool multiline_)
	: Gump(x, y, w, h), _text(txt), _gameFont(gamefont_), _fontNum(font),
	  _maxLength(maxlength_), _multiLine(multiline_),
	  _cursorChanged(0), _cursorVisible(true), _cachedText(nullptr) {
	_cursor = _text.size();
}

EditWidget::~EditWidget(void) {
	delete _cachedText;
}

// Init the gump, call after construction
void EditWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	Font *font = getFont();

	// Y offset is always baseline
	_dims.y = -font->getBaseline();

	// No X offset
	_dims.x = 0;

	if (_gameFont && getFont()->isHighRes()) {
		int32 x_ = 0, y_ = 0, w = 0;
		ScreenSpaceToGumpRect(x_, y_, w, _dims.y, ROUND_OUTSIDE);
	}
}

Font *EditWidget::getFont() const {
	if (_gameFont)
		return FontManager::get_instance()->getGameFont(_fontNum, true);
	else
		return FontManager::get_instance()->getTTFont(_fontNum);
}

void EditWidget::setText(const Std::string &t) {
	_text = t;
	_cursor = _text.size();
	FORGET_OBJECT(_cachedText);
}

void EditWidget::ensureCursorVisible() {
	_cursorVisible = true;
	_cursorChanged = g_system->getMillis();
}

bool EditWidget::textFits(Std::string &t) {
	Font *font = getFont();

	unsigned int remaining;
	int32 width, height;

	int32 max_width = _multiLine ? _dims.w : 0;
	int32 max_height = _dims.h;
	if (_gameFont && font->isHighRes()) {
		int32 x_ = 0, y_ = 0;
		GumpRectToScreenSpace(x_, y_, max_width, max_height, ROUND_INSIDE);
	}

	font->getTextSize(t, width, height, remaining,
	                  max_width, max_height,
	                  Font::TEXT_LEFT, false);

	if (_gameFont && font->isHighRes()) {
		int32 x_ = 0, y_ = 0;
		ScreenSpaceToGumpRect(x_, y_, width, height, ROUND_OUTSIDE);
	}

	if (_multiLine)
		return (remaining >= t.size());
	else
		return (width <= _dims.w);
}

void EditWidget::renderText() {
	bool cv = _cursorVisible;
	if (!IsFocus()) {
		cv = false;
	} else {
		uint32 now = g_system->getMillis();
		if (now > _cursorChanged + 750) {
			cv = !_cursorVisible;
			_cursorChanged = now;
		}
	}

	if (cv != _cursorVisible) {
		FORGET_OBJECT(_cachedText);
		_cursorVisible = cv;
	}

	if (!_cachedText) {
		Font *font = getFont();

		int32 max_width = _multiLine ? _dims.w : 0;
		int32 max_height = _dims.h;
		if (_gameFont && font->isHighRes()) {
			int32 x_ = 0, y_ = 0;
			GumpRectToScreenSpace(x_, y_, max_width, max_height, ROUND_INSIDE);
		}

		unsigned int remaining;
		_cachedText = font->renderText(_text, remaining,
		                               max_width, max_height,
		                               Font::TEXT_LEFT,
		                               false, cv ? _cursor : Std::string::npos);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void EditWidget::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	renderText();

	if (scaled && _gameFont && getFont()->isHighRes()) {
		surf->FillAlpha(0xFF, _dims.x, _dims.y, _dims.w, _dims.h);
		return;
	}

	_cachedText->draw(surf, 0, 0);
}

// Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
void EditWidget::PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy) {
	Font *font = getFont();

	if (!_gameFont || !font->isHighRes()) return;

	int32 x_ = 0, y_ = 0;
	GumpToScreenSpace(x_, y_, ROUND_BOTTOMRIGHT);

	_cachedText->draw(surf, x_, y_, true);

	x_ = _dims.x;
	y_ = _dims.y;
	int32 w = _dims.w, h = _dims.h;
	GumpRectToScreenSpace(x_, y_, w, h, ROUND_OUTSIDE);
	surf->FillAlpha(0x00, x_, y_, w, h);
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump *EditWidget::OnMouseMotion(int32 mx, int32 my) {
	return nullptr;
}

bool EditWidget::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		_parent->ChildNotify(this, EDIT_ENTER);
		break;
	case Common::KEYCODE_ESCAPE:
		_parent->ChildNotify(this, EDIT_ESCAPE);
		break;
	case Common::KEYCODE_BACKSPACE:
		if (_cursor > 0) {
			_text.erase(--_cursor, 1);
			FORGET_OBJECT(_cachedText);
			ensureCursorVisible();
		}
		break;
	case Common::KEYCODE_DELETE:
		if (_cursor != _text.size()) {
			_text.erase(_cursor, 1);
			FORGET_OBJECT(_cachedText);
		}
		break;
	case Common::KEYCODE_LEFT:
		if (_cursor > 0) {
			_cursor--;
			FORGET_OBJECT(_cachedText);
			ensureCursorVisible();
		}
		break;
	case Common::KEYCODE_RIGHT:
		if (_cursor < _text.size()) {
			_cursor++;
			FORGET_OBJECT(_cachedText);
			ensureCursorVisible();
		}
		break;
	default:
		break;
	}

	return true;
}

bool EditWidget::OnKeyUp(int key) {
	return true;
}


bool EditWidget::OnTextInput(int unicode) {
	if (_maxLength > 0 && _text.size() >= _maxLength)
		return true;

	char c = 0;
	if (unicode >= 0 && unicode < 256)
		c = reverse_encoding[unicode];
	if (!c) return true;

	Std::string newtext = _text;
	newtext.insert(_cursor, 1, c);

	if (textFits(newtext)) {
		_text = newtext;
		_cursor++;
		FORGET_OBJECT(_cachedText);
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
