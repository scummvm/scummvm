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

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, x, y, w, h, scale, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip)
	: RichTextWidget(boss, x, y, w, h, false, text, tooltip) {
}

RichTextWidget::RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, name, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

void RichTextWidget::init() {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_TRACK_MOUSE | WIDGET_DYN_TOOLTIP);

	_type = kRichTextWidget;

	_verticalScroll = new ScrollBarWidget(this, _w - 16, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;

	_innerMargin = g_gui.xmlEval()->getVar("Globals.RichTextWidget.InnerMargin", 0);
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	_textWidth = MAX(1, _w - _scrollbarWidth - 2 * _innerMargin);
	_textHeight = MAX(1, _h - 2 * _innerMargin);

	_limitH = 140;
}


RichTextWidget::~RichTextWidget() {
	delete _txtWnd;

	if (_surface)
		_surface->free();

	delete _surface;
}

void RichTextWidget::handleMouseWheel(int x, int y, int direction) {
	_verticalScroll->handleMouseWheel(x, y, direction);
}

void RichTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	_mouseDownY = _mouseDownStartY = y;
}

void RichTextWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// Allow some tiny finger slipping
	if (ABS(_mouseDownY - _mouseDownStartY) > 5) {
		_mouseDownY = _mouseDownStartY = 0;

		return;
	}

	_mouseDownY = _mouseDownStartY = 0;

	Common::String link = _txtWnd->getMouseLink(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY).encode();

	if (link.hasPrefixIgnoreCase("http"))
		g_system->openUrl(link);
}

void RichTextWidget::handleMouseMoved(int x, int y, int button) {
	if (_mouseDownStartY == 0 || _mouseDownY == y)
		return;

	int h = _txtWnd->getTextHeight();
	int prevScrolledY = _scrolledY;

	_scrolledY = CLIP(_scrolledY - (y - _mouseDownY), 0, h);

	_mouseDownY = y;

	if (_scrolledY == prevScrolledY)
		return;

	recalc();
	_verticalScroll->recalc();
	markAsDirty();
}

void RichTextWidget::handleTooltipUpdate(int x, int y) {
	_tooltip = _txtWnd->getMouseLink(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY);
}

void RichTextWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void RichTextWidget::recalc() {
	_innerMargin = g_gui.xmlEval()->getVar("Globals.RichTextWidget.InnerMargin", 0);
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_textWidth = MAX(1, _w - _scrollbarWidth - 2 * _innerMargin);
	_textHeight = MAX(1, _h - 2 * _innerMargin);
	_limitH = _textHeight;

	// Workaround: Currently Graphics::MacText::setMaxWidth does not work well.
	// There is a known limitation that the size is skipped when the text contains table,
	// and there is also an issue with the font.
	// So for now we recreate the widget.
//	if (_txtWnd) {
//		_txtWnd->setMaxWidth(_textWidth);
//		if (_surface->w != _textWidth || _surface->h != _textHeight)
//			_surface->create(_textWidth, _textHeight, g_gui.getWM()->_pixelformat);
//	} else {
//		createWidget();
//	}
	if (!_surface || _surface->w != _textWidth) {
		delete _txtWnd;
		createWidget();
	} else if (_surface->h != _textHeight)
		_surface->create(_textWidth, _textHeight, g_gui.getWM()->_pixelformat);

	int h = _txtWnd->getTextHeight();

	if (h <= _limitH) _scrolledY = 0;
	if (_scrolledY > h - _limitH) _scrolledY = MAX(0, h - _limitH);

	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = _h / 4;
	_verticalScroll->setPos(_w - _scrollbarWidth, 0);
	_verticalScroll->setSize(_scrollbarWidth, _h - 1);
}

void RichTextWidget::createWidget() {
	Graphics::MacWindowManager *wm = g_gui.getWM();

	uint32 bg = wm->_pixelformat.ARGBToColor(0, 0xff, 0xff, 0xff); // transparent
	TextColorData *normal = g_gui.theme()->getTextColorData(kTextColorNormal);
	uint32 fg = wm->_pixelformat.RGBToColor(normal->r, normal->g, normal->b);

	const int fontHeight = g_gui.xmlEval()->getVar("Globals.Font.Height", 25);

	Graphics::MacFont macFont(Graphics::kMacFontNewYork, fontHeight, Graphics::kMacFontRegular);

	_txtWnd = new Graphics::MacText(Common::U32String(), wm, &macFont, fg, bg, _textWidth, Graphics::kTextAlignLeft);

	if (!_imageArchive.empty())
		_txtWnd->setImageArchive(_imageArchive);

	_txtWnd->setMarkdownText(_text);

	if (_surface)
		_surface->create(_textWidth, _textHeight, g_gui.getWM()->_pixelformat);
	else
		_surface = new Graphics::ManagedSurface(_textWidth, _textHeight, wm->_pixelformat);
}

void RichTextWidget::reflowLayout() {
	Widget::reflowLayout();

	recalc();

	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
	_verticalScroll->recalc();
}

void RichTextWidget::drawWidget() {
	if (!_txtWnd)
		recalc();

	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), ThemeEngine::kWidgetBackgroundPlain);

	_surface->clear(g_gui.getWM()->_pixelformat.ARGBToColor(0, 0xff, 0xff, 0xff)); // transparent

	_txtWnd->draw(_surface, 0, _scrolledY, _textWidth, _textHeight, 0, 0);

	g_gui.theme()->drawManagedSurface(Common::Point(_x + _innerMargin, _y + _innerMargin), *_surface);
}

void RichTextWidget::draw() {
	Widget::draw();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->draw();
	}
}

void RichTextWidget::markAsDirty() {
	Widget::markAsDirty();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->markAsDirty();
	}
}

bool RichTextWidget::containsWidget(Widget *w) const {
	if (w == _verticalScroll || _verticalScroll->containsWidget(w))
		return true;
	return false;
}

Widget *RichTextWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w - _scrollbarWidth)
		return _verticalScroll;

	return this;
}

} // End of namespace GUI
