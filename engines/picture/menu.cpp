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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://www.switchlink.se/svn/picture/resource.cpp $
 * $Id: resource.cpp 2 2008-08-04 12:18:15Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/menu.h"
#include "picture/resource.h"
#include "picture/screen.h"

namespace Picture {

// This code is very experimental.

Widget::Widget(PictureEngine *vm, int x, int y) : _vm(vm) {
	_rect.left = x;
	_rect.top = y;
}

Widget::~Widget() {
}

void Widget::redraw() {
}

Widget *Widget::getHoveredWidget(int mouseX, int mouseY) {
	if (_rect.contains(mouseX, mouseY))
		return this;
	else
		return NULL;
}

void Widget::calcDimensions() {
}

void Widget::setRect(Common::Rect rect) {
	_rect = rect;
}

void Widget::onMouseEnter() {
}

void Widget::onMouseLeave() {
}

void Widget::onMouseMove(int mouseX, int mouseY) {
}

LabelWidget::LabelWidget(PictureEngine *vm, int x, int y, Common::String caption, uint flags) :
	Widget(vm, x, y), _caption(caption), _flags(flags), _fontColor(kFontColorMenuDefault) {
	
	calcDimensions();
	
}

LabelWidget::~LabelWidget() {
}

void LabelWidget::redraw() {
	_vm->_screen->drawString(_rect.left, _rect.top, _fontColor, 14, (byte*)_caption.c_str(), -1, NULL, true);
}

void LabelWidget::calcDimensions() {
	Font font(_vm->_res->load(14)->data);
	_rect.setWidth(font.getTextWidth((byte*)_caption.c_str()));
	_rect.setHeight(font.getHeight());
}

void LabelWidget::setCaption(Common::String caption) {
	_caption = caption;
	calcDimensions();
}

void LabelWidget::setFontColor(byte fontColor) {
	_fontColor = fontColor;
}

void LabelWidget::onMouseEnter() {
	setFontColor(kFontColorMenuActive);
}

void LabelWidget::onMouseLeave() {
	setFontColor(kFontColorMenuDefault);
}


VolumeControlWidget::VolumeControlWidget(PictureEngine *vm, int x, int y, Common::String caption, uint flags) :
	Widget(_vm, x, y), _activeWidget(NULL) {
	
	_label = new LabelWidget(vm, x, y, caption, flags);
	_up = new LabelWidget(vm, x + 350, y + 4, "[", flags);
	_down = new LabelWidget(vm, x + 350 + 24, y + 4, "]", flags);
	_indicator = new LabelWidget(vm, x + 350 + 24 + 24 + 8, y, "||||||||||", flags);
	
	_rect.setWidth(350 + 24 + 24 + 8 + 50);
	_rect.setHeight(20);
	
}

VolumeControlWidget::~VolumeControlWidget() {
	delete _label;
	delete _up;
	delete _down;
	delete _indicator;
}

void VolumeControlWidget::redraw() {
	_label->redraw();
	_up->redraw();
	_down->redraw();
	_indicator->redraw();
}

Widget *VolumeControlWidget::getHoveredWidget(int mouseX, int mouseY) {
	return Widget::getHoveredWidget(mouseX, mouseY);
}

void VolumeControlWidget::calcDimensions() {
}

void VolumeControlWidget::onMouseEnter() {
	_label->setFontColor(kFontColorMenuActive);
	_indicator->setFontColor(kFontColorMenuActive);
	_activeWidget = NULL;
}

void VolumeControlWidget::onMouseLeave() {
	_label->setFontColor(kFontColorMenuDefault);
	_up->setFontColor(kFontColorMenuDefault);
	_down->setFontColor(kFontColorMenuDefault);
	_indicator->setFontColor(kFontColorMenuDefault);
}

void VolumeControlWidget::onMouseMove(int mouseX, int mouseY) {

	Widget *hoveredWidget = NULL;

	hoveredWidget = _up->getHoveredWidget(mouseX, mouseY);
	if (!hoveredWidget)
		hoveredWidget = _down->getHoveredWidget(mouseX, mouseY);

	if (_activeWidget != hoveredWidget) {
		_activeWidget = hoveredWidget;
		if (!_activeWidget) {
			_up->setFontColor(kFontColorMenuDefault);
			_down->setFontColor(kFontColorMenuDefault);
		} else if (_activeWidget == _up) {
			_up->setFontColor(kFontColorMenuActive);
			_down->setFontColor(kFontColorMenuDefault);
		} else if (_activeWidget == _down) {
			_up->setFontColor(kFontColorMenuDefault);
			_down->setFontColor(kFontColorMenuActive);
		}
	}

}

MenuPage::MenuPage(Common::String caption) {
}

MenuPage::~MenuPage() {
	// TODO: Delete widgets
}

void MenuPage::addWidget(Widget *widget) {
	_widgets.push_back(widget);
}

void MenuPage::redraw() {
	for (WidgetArray::iterator iter = _widgets.begin(); iter != _widgets.end(); iter++) {
		(*iter)->redraw();
	}
}

Widget *MenuPage::getHoveredWidget(int mouseX, int mouseY) {
	Widget *hoveredWidget = NULL;
	for (WidgetArray::iterator iter = _widgets.begin(); iter != _widgets.end() && !hoveredWidget; iter++) {
		hoveredWidget = (*iter)->getHoveredWidget(mouseX, mouseY);
	}
	return hoveredWidget;
}

MenuSystem::MenuSystem(PictureEngine *vm) : _vm(vm), _activeWidget(NULL), _oldMouseX(-1), _oldMouseY(-1) {
	_page = new MenuPage("Welcome");
	_page->addWidget(new LabelWidget(_vm, 10, 10, "Load game", 0));
	_page->addWidget(new LabelWidget(_vm, 10, 35, "Save game", 0));
	_page->addWidget(new VolumeControlWidget(_vm, 10, 60, "Master volume", 0));
	_page->addWidget(new VolumeControlWidget(_vm, 10, 90, "Some other volume", 0));
}

MenuSystem::~MenuSystem() {
	delete _page;
}

void MenuSystem::update() {

	_page->redraw();

	if (_vm->_mouseX != _oldMouseX || _vm->_mouseY != _oldMouseY) {

		_oldMouseX = _vm->_mouseX;
		_oldMouseY = _vm->_mouseY;

		Widget *hoveredWidget = _page->getHoveredWidget(_vm->_mouseX, _vm->_mouseY);
		if (_activeWidget != hoveredWidget) {
			if (_activeWidget)
				_activeWidget->onMouseLeave();
			if (hoveredWidget)
				hoveredWidget->onMouseEnter();
			_activeWidget = hoveredWidget;
		}

		if (_activeWidget) {
			_activeWidget->onMouseMove(_vm->_mouseX, _vm->_mouseY);
		}
		
	}

}

} // End of namespace Picture
