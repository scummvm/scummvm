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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/fs.h"
#include "common/config-manager.h"

#include "gob/init.h"

namespace Gob {

Init_v7::Init_v7(GobEngine *vm) : Init_v2(vm) {
}

Init_v7::~Init_v7() {
}

void Init_v7::initGame() {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	// Add the environment directory
	SearchMan.addSubDirectoryMatching(gameDataDir, "envir");

	// Add the application list directory
	SearchMan.addSubDirectoryMatching(gameDataDir, "applis");

	// Add the "ADIBODEM" directory sometimes found in demos
	SearchMan.addSubDirectoryMatching(gameDataDir, "adibodem");

	// Add additional applications directories (e.g. "Read/Count 4-5 years").
	// We rely on the presence of an "intro_ap.itk" to determinate whether a subdirectory contains an application.
	Common::FSList subdirs;
	gameDataDir.getChildren(subdirs, Common::FSNode::kListDirectoriesOnly);
	for (const Common::FSNode &subdirNode : subdirs) {
		Common::FSDirectory subdir(subdirNode);
		if (subdir.hasFile("intro_ap.stk")) {
			debugC(1, kDebugFileIO, "Found Adibou/Adi application subdirectory \"%s\", adding it to the search path", subdir.getFSNode().getName().c_str());
			SearchMan.addSubDirectoryMatching(gameDataDir, subdir.getFSNode().getName(), 0, 4, true);
		}
	}

	Init::initGame();
}

} // End of namespace Gob
