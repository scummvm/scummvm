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

#include "common/config-manager.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/video.h"

namespace Gob {

Init_v2::Init_v2(GobEngine *vm) : Init_v1(vm) {
}

Init_v2::~Init_v2() {
}

void Init_v2::initVideo() {
	if (_vm->_global->_videoMode)
		_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_draw->_frontSurface = _vm->_global->_primarySurfDesc;
	_vm->_video->initSurfDesc(_vm->_video->_surfWidth, _vm->_video->_surfHeight, PRIMARY_SURFACE);

	_vm->_global->_mousePresent = 1;

	_vm->_global->_colorCount = 16;
	if (!_vm->isEGA() &&
		!_vm->is16Colors() &&
		((_vm->getPlatform() == Common::kPlatformDOS) ||
		 (_vm->getPlatform() == Common::kPlatformMacintosh) ||
		 (_vm->getPlatform() == Common::kPlatformWindows)) &&
		((_vm->_global->_videoMode == 0x13) ||
		 (_vm->_global->_videoMode == 0x14)))
		_vm->_global->_colorCount = 256;

	_vm->_global->_pPaletteDesc = &_vm->_global->_paletteStruct;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_global->_unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _vm->_global->_unusedPalette2;

	_vm->_video->initSurfDesc(_vm->_video->_surfWidth, _vm->_video->_surfHeight, PRIMARY_SURFACE);

	_vm->_draw->_cursorWidth       = 16;
	_vm->_draw->_cursorHeight      = 16;
	_vm->_draw->_transparentCursor =  1;
}

void Init_v2::initGame() {
	if (_vm->getGameType() == kGameTypeAdibou1) {
		const Common::FSNode gameDataDir(ConfMan.getPath("path"));

		// Add additional applications directories (e.g. "Read/Count 4-5 years").
		Common::FSList subdirs;
		gameDataDir.getChildren(subdirs, Common::FSNode::kListDirectoriesOnly);
		for (const Common::FSNode &subdirNode : subdirs) {
			Common::FSDirectory subdir(subdirNode);
			if (subdir.hasFile("c51.stk") || subdir.hasFile("c61.stk") || subdir.hasFile("l51.stk") || subdir.hasFile("l61.stk")) {
				debugC(1, kDebugFileIO, "Found Adibou/Adi application subdirectory \"%s\", adding it to the search path", subdir.getFSNode().getName().c_str());
				SearchMan.addSubDirectoryMatching(gameDataDir, subdir.getFSNode().getName(), 0, 4, true);
			}
		}
	}

	Init::initGame();
}


} // End of namespace Gob
