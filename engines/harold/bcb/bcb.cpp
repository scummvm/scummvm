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
 */

#include "harold/bcb/bcb.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/sjis.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace BCB {

static const int kCaptionTextNativeCharacterHeight = 18;
static const int kOwnerDrawUiTextNativeCharacterHeight = 16;

BCBEngine::BCBEngine(OSystem *syst, const HaroldGameDescription *desc)
	: Engine(syst), _gameDescription(desc) {
}

BCBEngine::~BCBEngine() {
}

Common::Error BCBEngine::run() {
	const Graphics::PixelFormat screenFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);

	initGraphics(640, 480, &screenFormat);
	CursorMan.lock(false);
	CursorMan.setDefaultArrowCursor();
	CursorMan.showMouse(true);
	g_system->updateScreen();
	_lastCursorRefreshMillis = g_system->getMillis();

	mainLoop();
	return Common::kNoError;
}

void BCBEngine::mainLoop() {
	while (!shouldQuit()) {
		bool screenUpdated = false;

		if (!pollEvents())
			return;

		serviceCursorRefresh(screenUpdated);

		delayWithEventPolling(10);
	}
}

} // End of namespace BCB
