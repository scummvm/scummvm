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
#include "awe/graphics.h"
#include "awe/resource.h"
#include "awe/scaler.h"
#include "awe/script.h"
#include "awe/system_stub.h"
#include "awe/util.h"
#include "awe/video.h"

namespace Awe {

AweEngine *g_engine;


static const struct {
	const char *name;
	int type;
} GRAPHICS[] = {
	{ "original", GRAPHICS_ORIGINAL },
	{ "software", GRAPHICS_SOFTWARE },
	{ "gl", GRAPHICS_GL },
	{ 0,  -1 }
};

static const struct {
	const char *name;
	int difficulty;
} DIFFICULTIES[] = {
	{ "easy", DIFFICULTY_EASY },
	{ "normal", DIFFICULTY_NORMAL },
	{ "hard", DIFFICULTY_HARD },
	{ 0,  -1 }
};

bool Graphics::_is1991 = false;
bool Graphics::_use555 = false;
bool Video::_useEGA = false;
Difficulty Script::_difficulty = DIFFICULTY_NORMAL;
bool Script::_useRemasteredAudio = true;

static Graphics *createGraphics(int type) {
	switch (type) {
	case GRAPHICS_ORIGINAL:
		Graphics::_is1991 = true;
		// fall-through
	case GRAPHICS_SOFTWARE:
		debugC(kDebugInfo, "Using software graphics");
		return GraphicsSoft_create();
	case GRAPHICS_GL:
		debugC(kDebugInfo, "Using GL graphics");
#ifdef USE_GL
		return GraphicsGL_create();
#endif
	}
	return 0;
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

static const int DEFAULT_WINDOW_W = 640;
static const int DEFAULT_WINDOW_H = 400;


#ifdef TODO
static const char USAGE[] =
"raw(gl) - Another World Interpreter\n"
"Usage: %s [OPTIONS]...\n"
"  --datapath=PATH   Path to data files (default '.')\n"
"  --language=LANG   Language (fr,us,de,es,it)\n"
"  --part=NUM        Game part to start from (0-35 or 16001-16009)\n"
"  --render=NAME     Renderer (original,software,gl)\n"
"  --window=WxH      Windowed display size (default '640x400')\n"
"  --fullscreen      Fullscreen display (stretched)\n"
"  --fullscreen-ar   Fullscreen display (16:10 aspect ratio)\n"
"  --ega-palette     Use EGA palette with DOS version\n"
"  --demo3-joy       Use inputs from 'demo3.joy' (DOS demo)\n"
"  --difficulty=DIFF Difficulty (easy,normal,hard)\n"
"  --audio=AUDIO     Audio (original,remastered)\n"
;



int main(int argc, char *argv[]) {
	char *dataPath = 0;
	int part = 16001;
	Language lang = Common::FR_FRA;
	int graphicsType = GRAPHICS_GL;
	DisplayMode dm;
	dm.mode = DisplayMode::WINDOWED;
	dm.width = DEFAULT_WINDOW_W;
	dm.height = DEFAULT_WINDOW_H;
	dm.opengl = (graphicsType == GRAPHICS_GL);
	Scaler scaler;
	scaler.name[0] = 0;
	scaler.factor = 1;
	bool defaultGraphics = true;
	bool demo3JoyInputs = false;
	if (argc == 2) {
		// data path as the only command line argument
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode)) {
			dataPath = strdup(argv[1]);
		}
	}
	while (1) {
		static struct option options[] = {
			{ "datapath", required_argument, 0, 'd' },
			{ "language", required_argument, 0, 'l' },
			{ "part",     required_argument, 0, 'p' },
			{ "render",   required_argument, 0, 'r' },
			{ "window",   required_argument, 0, 'w' },
			{ "fullscreen", no_argument,     0, 'f' },
			{ "fullscreen-ar", no_argument,  0, 'a' },
			{ "scaler",   required_argument, 0, 's' },
			{ "ega-palette", no_argument,    0, 'e' },
			{ "demo3-joy",  no_argument,     0, 'j' },
			{ "difficulty", required_argument, 0, 'i' },
			{ "audio",    required_argument, 0, 'u' },
			{ "help",       no_argument,     0, 'h' },
			{ 0, 0, 0, 0 }
		};
		int index;
		const int c = getopt_long(argc, argv, "", options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'd':
			dataPath = strdup(optarg);
			break;
		case 'l':
			for (int i = 0; LANGUAGES[i].name; ++i) {
				if (strcmp(optarg, LANGUAGES[i].name) == 0) {
					lang = (Language)LANGUAGES[i].lang;
					break;
				}
			}
			break;
		case 'p':
			part = atoi(optarg);
			break;
		case 'r':
			for (int i = 0; GRAPHICS[i].name; ++i) {
				if (strcmp(optarg, GRAPHICS[i].name) == 0) {
					graphicsType = GRAPHICS[i].type;
					dm.opengl = (graphicsType == GRAPHICS_GL);
					defaultGraphics = false;
					break;
				}
			}
			break;
		case 'w':
			sscanf(optarg, "%dx%d", &dm.width, &dm.height);
			break;
		case 'f':
			dm.mode = DisplayMode::FULLSCREEN;
			break;
		case 'a':
			dm.mode = DisplayMode::FULLSCREEN_AR;
			break;
		case 's':
			parseScaler(optarg, &scaler);
			break;
		case 'e':
			Video::_useEGA = true;
			break;
		case 'j':
			demo3JoyInputs = true;
			break;
		case 'i':
			for (int i = 0; DIFFICULTIES[i].name; ++i) {
				if (strcmp(optarg, DIFFICULTIES[i].name) == 0) {
					Script::_difficulty = (Difficulty)DIFFICULTIES[i].difficulty;
					break;
				}
			}
			break;
		case 'u':
			if (strcmp(optarg, "remastered") == 0) {
				Script::_useRemasteredAudio = true;
			} else if (strcmp(optarg, "original") == 0) {
				Script::_useRemasteredAudio = false;
			}
			break;
		case 'h':
			// fall-through
		default:
			printf(USAGE, argv[0]);
			return 0;
		}
	}

	Engine *e = new Engine(dataPath, part);
	if (defaultGraphics) {
		// if not set, use original software graphics for 199x editions and GL for the anniversary and 3DO versions
		graphicsType = getGraphicsType(dataType);
		dm.opengl = (graphicsType == GRAPHICS_GL);
	}
	if (graphicsType != GRAPHICS_GL && dataType == DT_3DO) {
		graphicsType = GRAPHICS_SOFTWARE;
		Graphics::_use555 = true;
	}
	Graphics *graphics = createGraphics(graphicsType);
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
	SystemStub *stub = SystemStub_SDL_create();
	stub->init(e->getGameTitle(lang), &dm);
	e->setSystemStub(stub, graphics);
	if (demo3JoyInputs && dataType == DT_DOS) {
		e->_res.readDemo3Joy();
	}
	e->setup(lang, graphicsType, scaler.name, scaler.factor);
	while (!stub->_pi.quit) {
		e->run();
	}
	e->finish();
	delete e;
	stub->fini();
	delete stub;
	return 0;
}

#endif


AweEngine::AweEngine(OSystem *syst, const Awe::AweGameDescription *gameDesc)
		: Engine(syst), _gameDescription(gameDesc),
		_random("Awe") {
	g_engine = this;
}

AweEngine::~AweEngine() {
}

Common::Error AweEngine::run() {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	int part = 16001;
	const Language lang = getLanguage();
	const DataType dataType = getDataType();
	int graphicsType = GRAPHICS_SOFTWARE;

	DisplayMode dm;
	dm.mode = DisplayMode::WINDOWED;
	dm.width = DEFAULT_WINDOW_W;
	dm.height = DEFAULT_WINDOW_H;
	dm.opengl = (graphicsType == GRAPHICS_GL);

	bool defaultGraphics = true;
	bool demo3JoyInputs = false;

	Awe::Engine *e = new Awe::Engine(_mixer, dataType, part);
	if (defaultGraphics) {
		// if not set, use original software graphics for 199x editions and GL for the anniversary and 3DO versions
		graphicsType = getGraphicsType(dataType);
		dm.opengl = (graphicsType == GRAPHICS_GL);
	}
	if (graphicsType != GRAPHICS_GL && dataType == DT_3DO) {
		graphicsType = GRAPHICS_SOFTWARE;
		Graphics::_use555 = true;
	}
	Graphics *graphics = createGraphics(graphicsType);
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

	SystemStub *stub = SystemStub_create();
	stub->init(dm);
	e->setSystemStub(stub, graphics);

	if (demo3JoyInputs && dataType == DT_DOS) {
		e->_res.readDemo3Joy();
	}

	e->setup(lang, graphicsType, nullptr, 1);

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
