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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/rendermode.h"
#include "common/system.h"

#include "engines/util.h"

#include "common/gui_options.h"

#include "chamber/chamber.h"
#include "chamber/renderer.h"
#include "chamber/script.h"
#include "chamber/resdata.h"
#include "chamber/room.h"
#include "chamber/dialog.h"
#include "chamber/cga.h"
#include "chamber/cursor.h"

namespace Chamber {

ChamberEngine *g_vm;

ChamberEngine::ChamberEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst) {
	g_vm = this;
	_gameDescription = desc;

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("chamber");

	_shouldQuit = false;
	_shouldRestart = false;
	_prioritycommand_1 = false;
	_prioritycommand_2 = false;
	_pxiData = NULL;

	_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	if (_renderMode == Common::kRenderEGA || _renderMode == Common::kRenderHercG || _renderMode == Common::kRenderHercA)
		_videoMode = _renderMode;
	else if (_renderMode == Common::kRenderDefault && Common::checkGameGUIOption(GUIO_RENDEREGA, Common::parseGameGUIOptions(ConfMan.get("guioptions"))))
		_videoMode = Common::kRenderEGA;
	else
		_videoMode = Common::kRenderCGA;

	if (_renderMode == Common::kRenderHercA)
		_videoMode = Common::kRenderHercG;

	_screenH = _screenW = _screenBits = _screenBPL = _screenPPB = 0;
	_line_offset = _line_offset2 = _fontHeight = _fontWidth = 0;

	if (_videoMode == Common::kRenderEGA)
		_renderer = new EGARenderer();
	else
		_renderer = new CGARenderer();
}

ChamberEngine::~ChamberEngine() {
	// Dispose your resources here
	delete _rnd;
	delete[] _pxiData;
	delete _renderer;

	deinitSound();
}

bool ChamberEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

int ChamberEngine::getX(int original_x) {
	return original_x;
}

int ChamberEngine::getY(int original_y) {
	return original_y;
}

} // End of namespace Chamber
