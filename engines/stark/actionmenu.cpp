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

#include "engines/stark/actionmenu.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/knowledgeset.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/global.h"

#include "engines/stark/scene.h"

#include "engines/stark/visual/image.h"

namespace Stark {
 
ActionMenu::ActionMenu(Gfx::Driver *gfx) : _gfx(gfx) {
	Global *global = StarkServices::instance().global;
	Resources::Object *inventory = global->getLevel()->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory, true);
	_hand = inventory->findChildWithIndex<Resources::ItemSub2>(1);
	_eye = inventory->findChildWithIndex<Resources::ItemSub2>(2);
	_mouth = inventory->findChildWithIndex<Resources::ItemSub2>(3);

	// TODO: Should these be hardcoded?
	_hand->setPosition(Common::Point(90, -21));
	_eye->setPosition(Common::Point(5, 40));
	_mouth->setPosition(Common::Point(43, 0));

	_renderEntries.push_back(_hand->getRenderEntry(Common::Point(0, 0)));
	_renderEntries.push_back(_eye->getRenderEntry(Common::Point(0, 0)));
	_renderEntries.push_back(_mouth->getRenderEntry(Common::Point(0, 0)));

	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	// TODO: Shouldn't use a function called getCursorImage for this, also unhardcode
	_background = staticProvider->getCursorImage(5);
}

ActionMenu::~ActionMenu() {
	_renderEntries.clear();
}

void ActionMenu::render(Common::Point pos) {
	_gfx->setScreenViewport(true); // Drawn unscaled
	_renderEntries.clear();

	if (_handEnabled) {
		_renderEntries.push_back(_hand->getRenderEntry(pos));
	}
	if (_eyeEnabled) {
		_renderEntries.push_back(_eye->getRenderEntry(pos));
	}
	if (_mouthEnabled) {
		_renderEntries.push_back(_mouth->getRenderEntry(pos));
	}

	Scene *scene = StarkServices::instance().scene;
	_background->render(pos);
	scene->render(_renderEntries);
}

void ActionMenu::clearActions() {
	_handEnabled = _mouthEnabled = _eyeEnabled = false;
}

void ActionMenu::enableAction(ActionMenuType action) {
	switch (action) {
		case kActionHand:
			_handEnabled = true;
			break;
		case kActionMouth:
			_mouthEnabled = true;
			break;
		case kActionEye:
			_eyeEnabled = true;
			break;
		default:
			error("Invalid action type in ActionMenu::enableAction");
	}
}

int ActionMenu::isThisYourButton(Resources::Object *object) {
	Resources::Item *item = object->findParent<Resources::Item>();
	if (item == _mouth) {
		return kActionMouth;
	} else if (item == _eye) {
		return kActionEye;
	} else if (item == _hand) {
		return kActionHand;
	} else {
		return -1;
	}
}
 
} // End of namespace Stark
