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

#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/visual/image.h"

namespace Stark {

StaticProvider::StaticProvider(ArchiveLoader *archiveLoader, Global *global) :
		_archiveLoader(archiveLoader),
		_global(global),
		_level(nullptr) {
}

void StaticProvider::init() {
	// Load the static archive
	_archiveLoader->load("static/static.xarc");

	// Set the root tree
	_level = _archiveLoader->useRoot<Resources::Level>("static/static.xarc");

	// Resources lifecycle update
	_level->onAllLoaded();

	Resources::Item *staticItem = _level->findChild<Resources::Item>();
	_stockAnims = staticItem->listChildren<Resources::Anim>();
}

void StaticProvider::shutdown() {
	_level = nullptr;

	_archiveLoader->returnRoot("static/static.xarc");
	_archiveLoader->unloadUnused();
}

VisualImageXMG *StaticProvider::getCursorImage(uint32 stockAnim) {
	Resources::Anim *anim = _stockAnims[stockAnim];
	return anim->getVisual()->get<VisualImageXMG>();
}

Resources::Anim *StaticProvider::getUIItem(StaticItems stockAnim) {
	return _stockAnims[stockAnim];
}

} // End of namespace Stark
