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

#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/object.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/actionmenu.h"
#include "engines/stark/cursor.h"
#include "engines/stark/scene.h"
#include "engines/stark/ui.h"

namespace Stark {

UserInterface::UserInterface(Gfx::Driver *driver, const Cursor *cursor) {
	_gfx = driver;
	_cursor = cursor;
	_actionMenuActive = false;
}

UserInterface::~UserInterface() {
}

void UserInterface::init() {
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

void UserInterface::walkTo(const Common::Point &mouse) {
	Global *global = StarkServices::instance().global;
	Scene *scene = StarkServices::instance().scene;

	Resources::Floor *floor = global->getCurrent()->getFloor();
	Resources::ItemSub10 *april = global->getCurrent()->getInteractive();

	Math::Vector3d origin, direction, intersection;
	scene->makeRayFromMouse(mouse, origin, direction);
	int32 floorFace = floor->findFaceHitByRay(origin, direction, intersection);
	if (april && floorFace >= 0) {
		// TODO: Complete, for now we just teleport to the target location
		april->setPosition3D(intersection);
		april->setFloorFaceIndex(floorFace);
	}
}

void UserInterface::update() {
}

void UserInterface::activateActionMenuOn(Common::Point pos, Resources::Object *activeObject) {
	_actionMenuActive = true;
	_actionMenuPos = pos;
	_actionMenu->clearActions();
	int possible = getActionsPossibleForObject(activeObject);
	if (possible & kActionLookPossible) {
		_actionMenu->enableAction(ActionMenu::kActionEye);
	}
	if (possible & kActionUsePossible) {
		_actionMenu->enableAction(ActionMenu::kActionHand);
	}
	if (possible & kActionTalkPossible) {
		_actionMenu->enableAction(ActionMenu::kActionMouth);
	}
}

Gfx::RenderEntry *UserInterface::getEntryAtPosition(Common::Point pos, Gfx::RenderEntryArray entries) {
	Gfx::RenderEntryArray::iterator element = entries.begin();
	Gfx::RenderEntry *objectAtPos = nullptr;
	// We need this scaled. (Optionally, if we want to scale the cursor, we can move the scaling to the setMousePosition-function)
	while (element != entries.end()) {
		if ((*element)->containsPoint(pos)) {
			if (!objectAtPos) {
				objectAtPos = *element;
			// This assumes that lower sort keys are more important than higher sortkeys.
			} else if (Gfx::RenderEntry::compare(*element, objectAtPos)) {
				objectAtPos = *element;
			}
		}
		++element;
	}
	return objectAtPos;
}

// To be specific, this returns the PATTable, so that the distinction on index is resolved,
// any code that needs the item will thus have to traverse back up.
Resources::Object *UserInterface::getObjectForRenderEntryAtPosition(Common::Point pos, Gfx::RenderEntry *entry) {
	if (entry == nullptr) {
		return nullptr;
	}
	Resources::Object *owner = (Resources::Object*)entry->getOwner();
	if (owner->getType() != Resources::Type::kItem) {
		// HACK: We don't have ItemSub2 yet.
		if (owner->getType() != Resources::Type::kAnim) {
			error("Owner of render entry should be an item, was: %s", owner->getType().getName());
		} else if (owner->getType() == Resources::Type::kAnim) {
			// HACK
			owner = owner->findParent<Resources::Item>();
		}
	}
	int index = entry->indexForPoint(pos);
	// No table index
	if (index == -1) {
		return nullptr;
	}
	Resources::PATTable *table = owner->findChildWithIndex<Resources::PATTable>(index);
	if (table) {
		return table;
	}
	return nullptr;
}

Common::String UserInterface::getMouseHintForObject(Resources::Object *object) {
	if (object) {
		Resources::Item *item = object->findParent<Resources::Item>();
		if (item) {
			if (item->listChildrenRecursive<Resources::PATTable>().size() > 1) {
				// Use the PAT Table name if more than one defined in item.
				return object->getName();
			} else {
				return item->getName();
			}
		} else {
			return object->getName();
		}
	} else {
		return "";
	}
}

int UserInterface::getActionsPossibleForObject(Resources::Object *object) {
	if (object == nullptr) {
		return kActionNonePossible;
	}
	if (object->getType() != Resources::Type::kPATTable) {
		error("getActionsPossibleForObject requires a PATTable");
	}
	Resources::PATTable *table = (Resources::PATTable*)object;
	int possible = UserInterface::kActionNonePossible;
	if (table->canPerformAction(Resources::PATTable::kActionLook)) {
		possible |= kActionLookPossible;
	}
	if (table->canPerformAction(Resources::PATTable::kActionUse) || isInventoryObject(object)) {
		possible |= kActionUsePossible;
	}
	if (table->canPerformAction(Resources::PATTable::kActionTalk)) {
		possible |= kActionTalkPossible;
	}
	if (table->canPerformAction(Resources::PATTable::kActionExit)) {
		possible |= kActionExitPossible;
	}
	return possible;
}

bool UserInterface::isInventoryObject(Resources::Object *object) {
	if (object->getType() != Resources::Type::kPATTable) {
		error("isInventoryObject requires a PATTable");
	}
	Resources::PATTable *table = (Resources::PATTable*)object;
	Resources::ItemSub2 *inventoryParent = table->findParent<Resources::ItemSub2>();
	if (_actionMenu->isThisYourButton(object) != -1 || inventoryParent->getSubType() != Resources::Item::kItemSub2) {
		// Do not explicitly add use on action-menu buttons.
		inventoryParent = nullptr;
	}
	return inventoryParent != nullptr;
}

bool UserInterface::performActionOnObject(Resources::Object *object, Resources::Object *activeObject, int action) {
	if (object->getType() != Resources::Type::kPATTable) {
		error("performActionOnObject requires a PATTable");
	}
	// PATTable of object under cursor
	Resources::PATTable *table = (Resources::PATTable*)object;
	// Possibilites:
	// * Click on something that doesn't take an action
	// * Click on something that takes exactly 1 action.
	// * Click on something that takes more than 1 action (open action menu)
	// * Click in the action menu, which has 0 available actions (TODO)
	if (action != -1) {
		Resources::Script *script = table->getScriptForAction(action);
		if (script != nullptr) {
			script->execute(Resources::Script::kCallModePlayerAction);
			return true;
		} else {
			warning("Could not perform action %d on %s", action, table->getName().c_str());
		}
	}
	// Assume all inventory objects need action menu.
	if (isInventoryObject(object)) {
		return false;
	}
	if (table->getNumActions() == 0) {
		
		if (activeObject) {
			// HACK: presumably this can be resolved by adding SubItem2, and hooking up the item to the actionMenu directly.
			int menuResult = _actionMenu->isThisYourButton(object);
			Resources::Script *script = nullptr;
			if (menuResult != -1 && activeObject->getType() == Resources::Type::kPATTable) {
				Resources::PATTable *activeObjectTable = (Resources::PATTable *)activeObject;
				if (menuResult == ActionMenu::kActionHand) {
					if (isInventoryObject(activeObjectTable)) {
						StarkServices::instance().ui->notifySelectedInventoryItem(activeObject);
						return true;
					}
					script = activeObjectTable->getScriptForAction(Resources::PATTable::kActionUse);
				} else if (menuResult == ActionMenu::kActionEye) {
					script = activeObjectTable->getScriptForAction(Resources::PATTable::kActionLook);
				} else if (menuResult == ActionMenu::kActionMouth) {
					script = activeObjectTable->getScriptForAction(Resources::PATTable::kActionTalk);
				}
				if (script != nullptr) {
					script->execute(Resources::Script::kCallModePlayerAction);
					return true;
				} else {
					warning("No script, did the action menu buttons misalign again?");
					// Return true here too to clear the event as handled, even though we had a wrong PAT.
					return true;
				}
			}
		}
		return true;
	} else if (table->getNumActions() == 1) {
		if (table->canPerformAction(Resources::PATTable::kActionLook)) {
			table->getScriptForAction(Resources::PATTable::kActionLook)->execute(Resources::Script::kCallModePlayerAction);
		}
		if (table->canPerformAction(Resources::PATTable::kActionUse)) {
			table->getScriptForAction(Resources::PATTable::kActionUse)->execute(Resources::Script::kCallModePlayerAction);
		}
		if (table->canPerformAction(Resources::PATTable::kActionTalk)) {
			table->getScriptForAction(Resources::PATTable::kActionTalk)->execute(Resources::Script::kCallModePlayerAction);
		}
		if (table->canPerformAction(Resources::PATTable::kActionExit)) {
			table->getScriptForAction(Resources::PATTable::kActionExit)->execute(Resources::Script::kCallModePlayerAction);
		}
		return true;
	} else {
		// This is where we should trigger the Action Menu
		return false;
	}
}

Gfx::RenderEntryArray UserInterface::getRenderEntries() {
	if (_actionMenuActive) {
		return _actionMenu->getRenderEntries();
	} else {
		return Gfx::RenderEntryArray();
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

	if (_actionMenuActive) {
		_actionMenu->render(_actionMenuPos);
	}

	delete debugTexture;
}

} // End of namespace Stark
