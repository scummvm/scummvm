/* ScummVM - Graphic Adventure AweEngine
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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/stream.h"
#include "awe/awe.h"
#include "awe/engine.h"
#include "awe/gfx.h"
#include "awe/resource.h"
#include "awe/script.h"
#include "awe/system_stub.h"
#include "awe/video.h"

namespace Awe {

AweEngine *g_engine;


/* unused
static const struct {
	const char *name;
	int type;
} GRAPHICS[] = {
	{ "original", GRAPHICS_ORIGINAL },
	{ "software", GRAPHICS_SOFTWARE },
	{ "gl", GRAPHICS_GL },
	{ nullptr,  -1 }
};

static const struct {
	const char *name;
	int difficulty;
} DIFFICULTIES[] = {
	{ "easy", DIFFICULTY_EASY },
	{ "normal", DIFFICULTY_NORMAL },
	{ "hard", DIFFICULTY_HARD },
	{ nullptr,  -1 }
};
*/

bool Gfx::_is1991;
Graphics::PixelFormat Gfx::_format;
bool Video::_useEGA;
Difficulty Script::_difficulty;
bool Script::_useRemasteredAudio;

static Gfx *createGraphics(int type) {
	switch (type) {
	case GRAPHICS_ORIGINAL:
		Gfx::_is1991 = true;
		// fall-through
	case GRAPHICS_SOFTWARE:
		debugC(kDebugInfo, "Using software graphics");
		return GraphicsSoft_create();
	case GRAPHICS_GL:
		debugC(kDebugInfo, "Using GL graphics");
#ifdef USE_GL
		return GraphicsGL_create();
#else
		break;
#endif

	default:
		break;

	}

	return nullptr;
}

static int getGraphicsType(DataType type) {
	switch (type) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
	case DT_3DO:
		return GRAPHICS_GL;
	default:
		return GRAPHICS_ORIGINAL;
	}
}

AweEngine::AweEngine(OSystem *syst, const Awe::AweGameDescription *gameDesc)
		: Engine(syst), _gameDescription(gameDesc),
		_random("Awe") {
	g_engine = this;

	Gfx::_is1991 = false;
	Gfx::_format = Graphics::PixelFormat::createFormatCLUT8();
	Video::_useEGA = false;
	Script::_difficulty = DIFFICULTY_NORMAL;
	Script::_useRemasteredAudio = true;
}

AweEngine::~AweEngine() {
}

Common::Error AweEngine::run() {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	const int part = 16001;
	const Language lang = getLanguage();
	const DataType dataType = getDataType();
	int graphicsType = GRAPHICS_SOFTWARE;

	DisplayMode dm;
	dm.mode = DisplayMode::WINDOWED;
	dm.width = GFX_W;
	dm.height = GFX_H;
	dm.opengl = (graphicsType == GRAPHICS_GL);

	const bool defaultGraphics = true;
	const bool demo3JoyInputs = false;

	Sound snd(_mixer);
	Awe::Engine *e = new Awe::Engine(&snd, dataType, part);

	if (defaultGraphics) {
		// if not set, use original software graphics for 199x editions and GL for the anniversary and 3DO versions
		graphicsType = getGraphicsType(dataType);
		dm.opengl = (graphicsType == GRAPHICS_GL);
	}

	if (graphicsType != GRAPHICS_GL && dataType == DT_3DO) {
		graphicsType = GRAPHICS_SOFTWARE;
		Gfx::_format = Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0);
	}

	Gfx *graphics = createGraphics(graphicsType);

	if (!graphics)
		error("The Anniversary versions graphics is not currently supported");

	if (dataType == DT_20TH_EDITION) {
		switch (Script::_difficulty) {
		case DIFFICULTY_EASY:
			debugC(kDebugInfo, "Using easy difficulty");
			break;
		case DIFFICULTY_NORMAL:
			debugC(kDebugInfo, "Using normal difficulty");
			break;
		case DIFFICULTY_HARD:
			debugC(kDebugInfo, "Using hard difficulty");
			break;
		}
	}

	if (dataType == DT_15TH_EDITION || dataType == DT_20TH_EDITION) {
		if (Script::_useRemasteredAudio) {
			debugC(kDebugInfo, "Using remastered audio");
		} else {
			debugC(kDebugInfo, "Using original audio");
		}
	}

	SystemStub *stub = SystemStub_create(
		dataType == DT_15TH_EDITION ||
		dataType == DT_20TH_EDITION
	);
	stub->init(dm);
	e->setSystemStub(stub, graphics);

	if (demo3JoyInputs && dataType == DT_DOS) {
		e->_res.readDemo3Joy();
	}

	e->_res._copyProtection = ConfMan.getBool("copy_protection");
	e->setup(lang, graphicsType);

	while (!stub->_pi.quit) {
		e->run();
	}

	e->finish();
	delete e;
	stub->fini();
	delete stub;

	return Common::kNoError;
}

DataType AweEngine::getDataType() const {
	return (DataType)_gameDescription->_gameType;
}

Common::Language AweEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // namespace Awe
