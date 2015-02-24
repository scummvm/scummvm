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
	_hand = inventory->findChildWithIndex<Resources::Item>(1);
	_eye = inventory->findChildWithIndex<Resources::Item>(2);
	_mouth = inventory->findChildWithIndex<Resources::Item>(3);

	Resources::Anim *handAnim = _hand->findChild<Resources::Anim>(false);

	_handEntry = new Gfx::RenderEntry(handAnim, handAnim->getName());
	_handEntry->setVisual(handAnim->getVisual());

	Resources::Anim *eyeAnim = _eye->findChild<Resources::Anim>(false);

	_eyeEntry = new Gfx::RenderEntry(eyeAnim, eyeAnim->getName());
	_eyeEntry->setVisual(eyeAnim->getVisual());
	
	Resources::Anim *mouthAnim = _mouth->findChild<Resources::Anim>(false);

	_mouthEntry = new Gfx::RenderEntry(mouthAnim, mouthAnim->getName());
	_mouthEntry->setVisual(mouthAnim->getVisual());
	
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	// TODO: Shouldn't use a function called getCursorImage for this, also unhardcode
	_background = staticProvider->getCursorImage(5);
}

ActionMenu::~ActionMenu() {
	_renderEntries.clear();
	delete _mouthEntry;
	delete _handEntry;
	delete _eyeEntry;
}

void ActionMenu::render(Common::Point pos) {
	_gfx->setScreenViewport(true); // Drawn unscaled
	_handEntry->setPosition(pos + Common::Point(90, -21));
	_eyeEntry->setPosition(pos + Common::Point(5, 40));
	_mouthEntry->setPosition(pos + Common::Point(43, 0));

	Scene *scene = StarkServices::instance().scene;
	_background->render(pos);
	scene->render(_renderEntries);
}

void ActionMenu::clearActions() {
	_renderEntries.clear();
}

void ActionMenu::enableAction(ActionMenuType action) {
	switch (action) {
		case kActionHand:
			_renderEntries.push_back(_handEntry);
			break;
		case kActionMouth:
			_renderEntries.push_back(_mouthEntry);
			break;
		case kActionEye:
			_renderEntries.push_back(_eyeEntry);
			break;
		default:
			error("Invalid action type in ActionMenu::enableAction");
	}
}

int ActionMenu::isThisYourButton(Resources::Object *object) {
	Resources::Item *item = object->findParent<Resources::Item>();
	warning("Item: %s", item->getName().c_str());
	if (item == _mouth) {
		return kActionHand;
	} else if (item == _eye) {
		return kActionEye;
	} else if (item == _hand) {
		return kActionHand;
	} else {
		return -1;
	}
}
 
} // End of namespace Stark
