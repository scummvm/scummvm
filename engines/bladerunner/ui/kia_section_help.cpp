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

#include "bladerunner/ui/kia_section_help.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_scroll_box.h"

namespace BladeRunner {

KIASectionHelp::KIASectionHelp(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);
	_scrollBox   = new UIScrollBox(_vm, nullptr, this, 1024, 0, false, Common::Rect(135, 145, 461, 385), Common::Rect(506, 160, 506, 350));

	_uiContainer->add(_scrollBox);
}

KIASectionHelp::~KIASectionHelp() {
	_uiContainer->clear();
	delete _scrollBox;
	delete _uiContainer;
}

void KIASectionHelp::open() {
	TextResource textResource(_vm);
	if (!textResource.open("HELP")) {
		return;
	}

	_scrollBox->clearLines();

	for (int i = 0; i < textResource.getCount(); ++i) {
		Common::String textLine = textResource.getText(i);
		int flags = 0x04;
		if (textLine.firstChar() == ' ') {
			flags = 0x00;
		}
		_scrollBox->addLine(textLine, -1, flags);
	}

	_scrollBox->show();
}

void KIASectionHelp::close() {
	_scrollBox->hide();
}

void KIASectionHelp::draw(Graphics::Surface &surface) {
	_vm->_kia->_shapes->get(69)->draw(surface, 501, 123);
	_uiContainer->draw(surface);
}

void KIASectionHelp::handleMouseMove(int mouseX, int mouseY) {
	_uiContainer->handleMouseMove(mouseX, mouseY);
}

void KIASectionHelp::handleMouseDown(bool mainButton) {
	_uiContainer->handleMouseDown(!mainButton);
}

void KIASectionHelp::handleMouseUp(bool mainButton) {
	_uiContainer->handleMouseUp(!mainButton);
}

void KIASectionHelp::handleMouseScroll(int direction) {
	_uiContainer->handleMouseScroll(direction);
}

} // End of namespace BladeRunner
