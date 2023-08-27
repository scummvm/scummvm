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

#include "common/system.h"
#include "common/unicode-bidi.h"

#include "graphics/macgui/mactext.h"

#include "gui/gui-manager.h"

#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"

#include "gui/widgets/richtext.h"
#include "gui/widgets/scrollbar.h"

namespace GUI {

Graphics::MacWindowManager *_wm = nullptr;

void ensureWM() {
	if (_wm)
		return;

	if (ConfMan.hasKey("extrapath")) {
		Common::FSNode dir(ConfMan.get("extrapath"));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	uint32 wmMode = Graphics::kWMModeNoDesktop | Graphics::kWMMode32bpp | Graphics::kWMModeNoCursorOverride;

	_wm = new Graphics::MacWindowManager(wmMode);
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, x, y, w, h, scale, tooltip)  {

	_text = text;

	init();
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip)
	: RichTextWidget(boss, x, y, w, h, false, text, tooltip) {
}

RichTextWidget::RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, name, tooltip) {

	_text = text;

	init();
}

void RichTextWidget::init() {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);

	_type = kRichTextWidget;

	_verticalScroll = new ScrollBarWidget(this, _w, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;

	ensureWM();

	_limitH = 140;
}

void RichTextWidget::recalc() {
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_limitH = _h;

	//calculate virtual height
	const int spacing = g_gui.xmlEval()->getVar("Global.Font.Height", 16); //on the bottom
	int min = spacing, max = 0;

	int h = max - min;

	if (h <= _limitH) _scrolledY = 0;
	if (_scrolledY > h - _limitH) _scrolledY = 0;

	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = kLineHeight;
	_verticalScroll->setPos(_w, _scrolledY);
	_verticalScroll->setSize(_scrollbarWidth, _limitH-1);
}

void RichTextWidget::createWidget() {
	uint32 white = _wm->_pixelformat.RGBToColor(0xff, 0xff, 0xff);
	TextColorData *normal = g_gui.theme()->getTextColorData(kTextColorNormal);
	uint32 black = _wm->_pixelformat.RGBToColor(normal->r, normal->g, normal->b);

	Graphics::MacFont macFont(Graphics::kMacFontNewYork, 30, Graphics::kMacFontRegular);

	_txtWnd = new Graphics::MacText(Common::U32String(), _wm, &macFont, black, white, _w, Graphics::kTextAlignLeft);
	_txtWnd->setMarkdownText(_text);

	_surface = new Graphics::ManagedSurface(_w, _h, _wm->_pixelformat);

	recalc();
}

void RichTextWidget::reflowLayout() {
	Widget::reflowLayout();
}

void RichTextWidget::drawWidget() {
	if (!_txtWnd)
		createWidget();

	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    ThemeEngine::kWidgetBackgroundEditText);

	_txtWnd->draw(_surface, 0, 0, _w, _h, 0, 0);

	g_gui.theme()->drawManagedSurface(Common::Point(_x, _y), *_surface);
}

void RichTextWidget::draw() {
	Widget::draw();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->draw();
	}
}

} // End of namespace GUI
