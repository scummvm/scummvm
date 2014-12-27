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

#include "engines/stark/resourceprovider.h"

#include "engines/stark/archiveloader.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/level.h"

namespace Stark {

ResourceProvider::ResourceProvider(ArchiveLoader *archiveLoader, Global *global) :
		_archiveLoader(archiveLoader),
		_global(global) {
}


void ResourceProvider::initGlobal() {
	// Load the root archive
	_archiveLoader->load("x.xarc");

	// Set the root tree
	Root *root = static_cast<Root *>(_archiveLoader->getRoot("x.xarc"));
	_global->setRoot(root);

	// Find the global level node
	Level *global = static_cast<Level *>(root->findChild(ResourceType::kLevel, 1));

	// Load the global archive
	Common::String globalArchiveName = global->getArchive();
	_archiveLoader->load(globalArchiveName);

	// Set the global tree
	global = static_cast<Level *>(_archiveLoader->getRoot(globalArchiveName));
	_global->setLevel(global);

	//TODO: Retrieve the inventory and April from the global tree
}

void ResourceProvider::shutdown() {
	_archiveLoader->unload(_global->getLevel()->getArchive());
	_global->setLevel(nullptr);
	_archiveLoader->unload("x.xarc");
	_global->setRoot(nullptr);
}

} // End of namespace Stark
