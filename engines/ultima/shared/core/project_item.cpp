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

#include "common/file.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "ultima/shared/engine/game_manager.h"
#include "ultima/shared/engine/ultima_early.h"
#include "ultima/shared/core/project_item.h"

namespace Ultima {
namespace Shared {

#define CURRENT_SAVEGAME_VERSION 1
#define MAX_SAVEGAME_SLOTS 99
#define MINIMUM_SAVEGAME_VERSION 1

EMPTY_MESSAGE_MAP(ProjectItem, NamedItem);

ProjectItem::ProjectItem() : _gameManager(nullptr) {
}

void ProjectItem::setGameManager(GameManager *gameManager) {
	if (!_gameManager)
		_gameManager = gameManager;
}

void ProjectItem::resetGameManager() {
	_gameManager = nullptr;
}

void ProjectItem::loadGame(int slotId) {

}

void ProjectItem::saveGame(int slotId, const Common::String &desc) {

}

void ProjectItem::clear() {
	TreeItem *item;
	while ((item = getFirstChild()) != nullptr)
		item->destroyAll();
}

void ProjectItem::preLoad() {

}

void ProjectItem::postLoad() {

}

void ProjectItem::preSave() {

}

void ProjectItem::postSave() {

}

} // End of namespace Shared
} // End of namespace Ultima
