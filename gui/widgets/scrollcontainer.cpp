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

#include "common/util.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

const int ScrollContainerWidget::kDragThreshold = 5;
ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, int x, int y, int w, int h, uint32 reflowCmd)
	: Widget(boss, x, y, w, h), CommandSender(nullptr), _reflowCmd(reflowCmd) {
	init();
}

ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &dialogName, uint32 reflowCmd)
	: Widget(boss, name), CommandSender(nullptr), _reflowCmd(reflowCmd), _dialogName(dialogName) {
	init();
}

void ScrollContainerWidget::init() {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_WANT_TICKLE | WIDGET_RETAIN_FOCUS);
	_type = kScrollContainerWidget;
	_backgroundType = ThemeEngine::kWidgetBackgroundPlain;
	_verticalScroll = new ScrollBarWidget(this, _w, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;
	_scrollPos = 0.0f;
	_limitH = 140;
	_fluidScroller = new FluidScroller();
	recalc();
}

void ScrollContainerWidget::handleMouseWheel(int x, int y, int direction) {
	if (!isEnabled())
		return;

	_fluidScroller->handleMouseWheel(direction);
}

void ScrollContainerWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	_mouseDownY = _mouseDownStartY = y;
	_isMouseDown = true;
	_fluidScroller->stopAnimation();
	Widget *child = _childUnderMouse;
	if (child) {
		int childX = (x + _scrolledX) - child->getRelX();
		int childY = (y + _scrolledY) - child->getRelY();
		child->handleMouseDown(childX, childY, button, clickCount);
	}
}

void ScrollContainerWidget::handleMouseMoved(int x, int y, int button) {
	if (!_isMouseDown || _mouseDownY == y)
		return;

	if (!_isDragging && ABS(y - _mouseDownStartY) > kDragThreshold)
		_isDragging = true;

	if (_isDragging) {
		int deltaY = _mouseDownY - y;
		_mouseDownY = y;
		_childUnderMouse = nullptr;

		if (deltaY != 0) {
			_fluidScroller->feedDrag(g_system->getMillis(), deltaY);
			_scrollPos = _fluidScroller->getVisualPosition();
			applyScrollPos();
		}
	}
}

void ScrollContainerWidget::handleTickle() {
	if (_fluidScroller->update(g_system->getMillis(), _scrollPos))
		applyScrollPos();
}

void ScrollContainerWidget::applyScrollPos() {
	_scrolledY = (int16)_scrollPos;
	int h = _verticalScroll->_numEntries;
	int maxScroll = MAX(0, h - _limitH);
	_verticalScroll->_currentPos = CLIP<int16>(_scrolledY, 0, maxScroll);
	_verticalScroll->setPos(_w, _scrolledY);
	_verticalScroll->recalc();
	markAsDirty();
	g_gui.scheduleTopDialogRedraw();
}

void ScrollContainerWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	Widget *child = _childUnderMouse;
	bool isDragging = _isDragging;

	if (_isMouseDown && _isDragging)
		_fluidScroller->startFling();

	_mouseDownY = _mouseDownStartY = 0;
	_isMouseDown = false;
	_isDragging = false;
	_childUnderMouse = nullptr;

	if (!isDragging && child) {
		int childX = (x + _scrolledX) - child->getRelX();
		int childY = (y + _scrolledY) - child->getRelY();
		child->handleMouseUp(childX, childY, button, clickCount);
	}
}

void ScrollContainerWidget::recalc() {
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_limitH = _h;

	//calculate virtual height
	const int spacing = g_gui.xmlEval()->getVar("Globals.Font.Height", 16); //on the bottom
	int min = spacing, max = 0;
	Widget *ptr = _firstWidget;
	while (ptr) {
		if (ptr != _verticalScroll && ptr->isVisible()) {
			int y = ptr->getAbsY() - getChildY();
			min = MIN(min, y - spacing);
			max = MAX(max, y + ptr->getHeight() + spacing);
		}
		ptr = ptr->next();
	}
	int h = max - min;

	if (h <= _limitH) _scrolledY = 0;
	else if (!_fluidScroller->isAnimating() && !_isMouseDown)
		_scrolledY = CLIP<int16>(_scrolledY, 0, h - _limitH);

	_verticalScroll->_numEntries = h;
	int maxScroll = MAX(0, h - _limitH);
	_verticalScroll->_currentPos = CLIP<int16>(_scrolledY, 0, maxScroll);
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = kLineHeight;
	_verticalScroll->setPos(_w, _scrolledY);
	_verticalScroll->setSize(_scrollbarWidth, _limitH-1);
	_verticalScroll->recalc();
	_fluidScroller->setBounds((float)maxScroll, _limitH, (float)_verticalScroll->_singleStep);
}


ScrollContainerWidget::~ScrollContainerWidget() {
	delete _fluidScroller;
}

int16 ScrollContainerWidget::getChildX() const {
	return getAbsX() - _scrolledX;
}

int16 ScrollContainerWidget::getChildY() const {
	return getAbsY() - _scrolledY;
}

uint16 ScrollContainerWidget::getWidth() const {
	// NOTE: if you change that, make sure to do the same
	// changes in the ThemeLayoutScrollContainerWidget (gui/ThemeLayout.cpp)
	// NOTE: this width is used for clipping, so it *includes*
	// scrollbars, because it starts from getAbsX(), not getChildX()
	return _w + _scrollbarWidth;
}

uint16 ScrollContainerWidget::getHeight() const {
	return _limitH;
}

void ScrollContainerWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		_scrollPos = _fluidScroller->setPosition((float)_scrolledY, false);
		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void ScrollContainerWidget::reflowLayout() {
	_childUnderMouse = nullptr;
	Widget::reflowLayout();

	if (!_dialogName.empty()) {
		g_gui.xmlEval()->reflowDialogLayout(_dialogName, _firstWidget);
	}

	//reflow layout of inner widgets
	Widget *ptr = _firstWidget;
	while (ptr) {
		ptr->reflowLayout();
		ptr = ptr->next();
	}

	//hide and move widgets, if needed
	sendCommand(_reflowCmd, 0);

	//recalculate height
	recalc();

	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
	_verticalScroll->recalc();
}

void ScrollContainerWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + getHeight()), _backgroundType);
}

void ScrollContainerWidget::draw() {
	Widget::draw();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->draw();
	}
}

void ScrollContainerWidget::markAsDirty() {
	Widget::markAsDirty();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->markAsDirty();
	}
}

bool ScrollContainerWidget::containsWidget(Widget *w) const {
	if (w == _verticalScroll || _verticalScroll->containsWidget(w))
		return true;
	return containsWidgetInChain(_firstWidget, w);
}

Widget *ScrollContainerWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w) {
		_childUnderMouse = nullptr;
		return _verticalScroll;
	}
	_childUnderMouse = Widget::findWidgetInChain(_firstWidget, x + _scrolledX, y + _scrolledY);
	if (_childUnderMouse == _verticalScroll) 
		_childUnderMouse = nullptr;
	return this;
}

void ScrollContainerWidget::setBackgroundType(ThemeEngine::WidgetBackground backgroundType) {
	_backgroundType = backgroundType;
}

} // End of namespace GUI
