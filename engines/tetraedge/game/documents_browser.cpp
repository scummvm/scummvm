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

#include "tetraedge/game/documents_browser.h"

namespace Tetraedge {

DocumentsBrowser::DocumentsBrowser() {
	_timer.alarmSignal().add(this, &DocumentsBrowser::onQuitDocumentDoubleClickTimer);
}

void DocumentsBrowser::enter() {
	setVisible(true);
	currentPage(_curPage);
}

void DocumentsBrowser::leave() {
	_timer.stop();
	setVisible(false);
}

void DocumentsBrowser::load() {
	setVisible(false);
	setName("documentsBrowser");

	setSizeType(RELATIVE_TO_PARENT);
	const TeVector3f32 userSz = TeLayout::userSize();
	setSize(TeVector3f32(1.0f, 1.0f, userSz.z()));

	TeLuaGUI::load("DocumentsBrowser/DocumentsBrowser.lua");

	TeLayout *docBrowser = TeLuaGUI::layout("documentBrowser");
	if (docBrowser)
		addChild(docBrowser);

	TeButtonLayout *button = buttonLayout("previousPage");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onPreviousPage);
	button = buttonLayout("nextPage");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onNextPage);
	button = TeLuaGUI::buttonLayout("zoomed");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onZoomedButton);
	button = TeLuaGUI::buttonLayout("zoomed");
	button->setVisible(false);

	// Game tries to load a file that doesn't exist..
	debug("TODO?? DocumentsBrowser::load: Game opens Documents.xml here.");
	_timer.start();
}

void DocumentsBrowser::loadZoomed() {
	_zoomedLayout.setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 usersz = userSize();
	_zoomedLayout.setSize(TeVector3f32(1.0f, 1.0f, usersz.z()));
	TeLayout *zoomedChild = layout("zoomed");
	_zoomedLayout.addChild(zoomedChild);
}

void DocumentsBrowser::currentPage(long page) {
	const Common::String pageName = Common::String::format("page%ld", page);
	TeLayout *pageLayout = layout(pageName);
	if (!pageLayout)
		return;

	_curPage = page;

	error("TODO: Implement DocumentsBrowser::currentPage");
}

bool DocumentsBrowser::onQuitDocumentDoubleClickTimer() {
	error("TODO: Implement DocumentsBrowser::onQuitDocumentDoubleClickTimer");
}

bool DocumentsBrowser::onNextPage() {
	currentPage(_curPage + 1);
	return false;
}

bool DocumentsBrowser::onPreviousPage() {
	currentPage(_curPage - 1);
	return false;
}

bool DocumentsBrowser::onZoomedButton() {
	error("TODO: Implement DocumentsBrowser::onZoomedButton");
}

void DocumentsBrowser::showDocument(const Common::String &str, long n) {
	error("TODO: Implement DocumentsBrowser::showDocument");
}


// TODO: Add more functions here.

} // end namespace Tetraedge
