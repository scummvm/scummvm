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

#include "common/scummsys.h"

#include "common/error.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "engines/util.h"

#include "trecision/trecision.h"

#include <common/archive.h>
#include <common/config-manager.h>
#include <common/fs.h>

#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"

namespace Trecision {

// TODO: Move to main engine?
void NlInit();

TrecisionEngine *g_vm;

TrecisionEngine::TrecisionEngine(OSystem *syst) : Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "AUTORUN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "FMV");

	g_vm = this;
}

TrecisionEngine::~TrecisionEngine() {
}

Common::Error TrecisionEngine::run() {
	const Graphics::PixelFormat kVideoFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); // RGB565
	initGraphics(MAXX, MAXY, &kVideoFormat);
	
	NlInit();

	return Common::kNoError;
}

} // End of namespace Trecision
