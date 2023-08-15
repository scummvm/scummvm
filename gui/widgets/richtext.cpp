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

#include "graphics/macgui/mactextwindow.h"

#include "gui/widgets/richtext.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

Graphics::MacWindowManager *_wm = nullptr;

void ensureWM() {
	if (_wm)
		return;

	uint32 wmMode = Graphics::kWMModeNoDesktop | Graphics::kWMModeForceBuiltinFonts
		| Graphics::kWMModeUnicode | Graphics::kWMMode32bpp;

	_wm = new Graphics::MacWindowManager(wmMode);
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, x, y, w, h, scale, tooltip)  {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kRichTextWidget;

	_text = text;

	ensureWM();
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip)
	: RichTextWidget(boss, x, y, w, h, false, text, tooltip) {
}

RichTextWidget::RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, name, tooltip) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kRichTextWidget;
	_text = text;

	ensureWM();
}

void RichTextWidget::createWidget() {
	_txtWnd = _wm->addTextWindow(FontMan.getFontByUsage(Graphics::FontManager::kGUIFont),
				0, 0xffffffff, _w, Graphics::kTextAlignLeft, nullptr, false);
	_txtWnd->setTextColorRGB(0xffffffff);
	_txtWnd->setBorderType(Graphics::kWindowBorderMacOSNoBorderScrollbar);
	_txtWnd->enableScrollbar(true);
	// it will hide the scrollbar when the text height is smaller than the window height
	_txtWnd->setMode(Graphics::kWindowModeDynamicScrollbar);
	_txtWnd->resize(_w, _h);
	_txtWnd->setEditable(false);
	_txtWnd->setSelectable(false);

	_txtWnd->appendText(_text);

	_surface = new Graphics::ManagedSurface(_w, _h, _wm->_pixelformat);

	warning("Pixel format: %s", _wm->_pixelformat.toString().c_str());
}

void RichTextWidget::reflowLayout() {
	Widget::reflowLayout();
}

void RichTextWidget::drawWidget() {
	if (!_txtWnd)
		createWidget();

	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    ThemeEngine::kWidgetBackgroundEditText);

	_txtWnd->draw(_surface);

	g_gui.theme()->drawManagedSurface(Common::Point(_x, _y), *_surface);

	warning("DRAW");
}

} // End of namespace GUI
