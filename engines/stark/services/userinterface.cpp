/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/userinterface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/actionmenu.h"
#include "engines/stark/cursor.h"

namespace Stark {

UserInterface::UserInterface(Gfx::Driver *driver, const Cursor *cursor) {
	_gfx = driver;
	_cursor = cursor;
	_interfaceVisible = false;
}

UserInterface::~UserInterface() {
}

void UserInterface::init() {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	// TODO: Shouldn't use a function called getCursorImage for this, also unhardcode
	_exitButton = staticProvider->getCursorImage(8);

	_actionMenu = new ActionMenu(_gfx);
}

void UserInterface::skipCurrentSpeeches() {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	// Get all speeches
	Common::Array<Resources::Speech *> speeches;
	speeches.push_back(global->getLevel()->listChildrenRecursive<Resources::Speech>());
	speeches.push_back(current->getLevel()->listChildrenRecursive<Resources::Speech>());
	speeches.push_back(current->getLocation()->listChildrenRecursive<Resources::Speech>());

	// Stop them
	for (uint i = 0; i < speeches.size(); i++) {
		Resources::Speech *speech = speeches[i];
		if (speech->isPlaying()) {
			speech->stop();
		}
	}
}

void UserInterface::scrollLocation(int32 dX, int32 dY) {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	Resources::Location *location = current->getLocation();

	Common::Point scroll = location->getScrollPosition();
	scroll.x += dX;
	scroll.y += dY;
	location->setScrollPosition(scroll);
}

void UserInterface::update() {
	// TODO: Unhardcode
	if (_cursor->getMousePosition().y < 40) {
		_interfaceVisible = true;
	} else {
		_interfaceVisible = false;
	}
}

void UserInterface::activateActionMenu(Common::Point pos, bool eye, bool hand, bool mouth) {
	_actionMenuActive = true;
	_actionMenuPos = pos;
	_actionMenu->clearActions();
	if (eye) {
		_actionMenu->enableAction(ActionMenu::kActionEye);
	}
	if (hand) {
		_actionMenu->enableAction(ActionMenu::kActionHand);
	}
	if (mouth) {
		_actionMenu->enableAction(ActionMenu::kActionMouth);
	}
}

void UserInterface::render() {
	// TODO: Move this elsewhere

	Common::String debugStr;

	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	int32 chapter = global->getCurrentChapter();

	debugStr += Common::String::format("location: %02x %02x ", current->getLevel()->getIndex(), current->getLocation()->getIndex());
	debugStr += current->getLevel()->getName() + ", " + current->getLocation()->getName();
	debugStr += Common::String::format(" chapter: %d", chapter);

	
	Gfx::Texture *debugTexture = _gfx->createTextureFromString(debugStr, 0xF0FF0F00);

	_gfx->setScreenViewport(false);
	_gfx->drawSurface(debugTexture, Common::Point(0,0));

	if (_interfaceVisible) {
		// TODO: Unhardcode position
		_exitButton->render(Common::Point(600, 0));
	}

	if (_actionMenuActive) {
		_actionMenu->render(_actionMenuPos);
	}

	delete debugTexture;
}

} // End of namespace Stark
