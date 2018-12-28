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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "graphics/pixelformat.h"

#include "engines/util.h"

#include "griffon/griffon.h"

namespace Griffon {

GriffonEngine::GriffonEngine(OSystem *syst) : Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	_rnd = new Common::RandomSource("griffon");
}

GriffonEngine::~GriffonEngine() {
	delete _rnd;
}

Common::Error GriffonEngine::run() {
	initGraphics(320, 240, new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));

	griffon_main();

	return Common::kNoError;
}

}
