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
#include "common/system.h"
#include "common/error.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "image/bmp.h"

#include "cryomni3d/font_manager.h"
#include "cryomni3d/dialogs_manager.h"
#include "cryomni3d/fixed_image.h"
#include "cryomni3d/omni3d.h"

#include "cryomni3d/versailles/engine.h"

#define DEBUG_FAST_START 1

namespace CryOmni3D {
namespace Versailles {

const FixedImageConfiguration CryOmni3DEngine_Versailles::kFixedImageConfiguration = {
	45, 223, 243, 238, 226, 198, 136, 145, 99, 113,
	470
};

CryOmni3DEngine_Versailles::CryOmni3DEngine_Versailles(OSystem *syst,
        const CryOmni3DGameDescription *gamedesc) : CryOmni3DEngine(syst, gamedesc),
	_mainPalette(nullptr), _cursorPalette(nullptr), _transparentPaletteMap(nullptr),
	_currentPlace(nullptr), _currentWarpImage(nullptr), _fixedImage(nullptr),
	_transitionAnimateWarp(true), _forceRedrawWarp(false), _forcePaletteUpdate(false),
	_fadedPalette(false), _loadedSave(-1), _dialogsMan(this),
	_musicVolumeFactor(1.), _musicCurrentFile(nullptr) {
}

CryOmni3DEngine_Versailles::~CryOmni3DEngine_Versailles() {
	delete _currentWarpImage;
	delete[] _mainPalette;
	delete[] _cursorPalette;
	delete[] _transparentPaletteMap;

	delete _fixedImage;
}

Common::Error CryOmni3DEngine_Versailles::run() {
	CryOmni3DEngine::run();

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/sc_trans", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/menu", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/basedoc/fonds", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/fonts", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/spr8col", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/spr8col/bmpok", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/wam", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/objets", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/gto", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/dial/flc_dial", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/dial/voix", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/textes", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/music", 1);
	SearchMan.addSubDirectoryMatching(gameDataDir, "datas_v/sound", 1);

	setupMessages();

	_dialogsMan.init(138, _messages[22]);
	_gameVariables.resize(GameVariables::kMax);
	_omni3dMan.init(75. / 180. * M_PI);

	_dialogsMan.loadGTO("DIALOG1.GTO");
	setupDialogVariables();
	setupDialogShows();

	setupPaintingsTitles();
	setupImgScripts();

	_mainPalette = new byte[3 * 256];
	setupFonts();
	setupSprites();
	loadCursorsPalette();

	// Objects need messages and sprites
	setupObjects();

	_transparentPaletteMap = new byte[256];
	_transparentSrcStart = 0;
	_transparentSrcStop = 240;
	_transparentDstStart = 0;
	_transparentDstStop = 248;
	_transparentNewStart = 248;
	_transparentNewStop = 254;

	// Inventory has a size of 50
	_inventory.init(50, new Common::Functor1Mem<unsigned int, void, Toolbar>(&_toolbar,
	                &Toolbar::inventoryChanged));

	// Init toolbar after we have setup sprites and fonts
	_toolbar.init(&_sprites, &_fontManager, &_messages, &_inventory, this);

	_fixedImage = new ZonFixedImage(*this, _inventory, _sprites, &kFixedImageConfiguration);

	// Documentation is needed by noone at init time, let's do it last
	initDocPeopleRecord();
	_docManager.init(&_sprites, &_fontManager, &_messages, this);

	initGraphics(640, 480);
	setMousePos(Common::Point(320, 200));

	_isPlaying = false;
	_isVisiting = false;

#if !defined(DEBUG_FAST_START) || DEBUG_FAST_START<1
	playTransitionEndLevel(-2);
	if (g_engine->shouldQuit()) {
		return Common::kNoError;
	}
	playTransitionEndLevel(-1);
	if (g_engine->shouldQuit()) {
		return Common::kNoError;
	}
#endif

	bool stopGame = false;
	while (!stopGame) {
		bool exitLoop = false;
		unsigned int nextStep = 0;
#if defined(DEBUG_FAST_START) && DEBUG_FAST_START>=2
		nextStep = 27;
		// Called in options
		syncOmni3DSettings();
#endif
		setCursor(181);
		while (!exitLoop) {
			_isPlaying = false;
			if (!nextStep) {
				nextStep = displayOptions();
			}
			if (nextStep == 40) {
				// Quit action
				exitLoop = true;
				stopGame = true;
			} else if (nextStep == 27 || nextStep == 28 || nextStep == 65) {
				// New game, Load game, Next level
				if (nextStep == 27) {
					// New game
#if !defined(DEBUG_FAST_START) || DEBUG_FAST_START<1
					playTransitionEndLevel(0);
					if (g_engine->shouldQuit()) {
						stopGame = true;
						exitLoop = true;
						break;
					}
#endif
					changeLevel(1);
				} else if (nextStep == 28) {
					// Load game
					loadGame(_isVisiting, _loadedSave);
				} else if (nextStep == 65) {
					changeLevel(_currentLevel + 1);
				}

				_isPlaying = true;
				_toolbar.setInventoryEnabled(!_isVisiting);
				nextStep = 0;
				_abortCommand = AbortNoAbort;

				gameStep();

				// We shouldn't return from gameStep without an abort command
				assert(_abortCommand != AbortNoAbort);
				switch (_abortCommand) {
				default:
					error("Invalid abortCommand: %d", _abortCommand);
					// Shouldn't return
					return Common::kUnknownError;
				case AbortFinished:
				case AbortGameOver:
					// Go back to menu
					exitLoop = true;
					break;
				case AbortQuit:
					exitLoop = true;
					stopGame = true;
					break;
				case AbortNewGame:
					nextStep = 27;
					break;
				case AbortLoadGame:
					nextStep = 28;
					break;
				case AbortNextLevel:
					nextStep = 65;
					break;
				}
			}
		}
	}
	return Common::kNoError;
}

void CryOmni3DEngine_Versailles::setupFonts() {
	Common::Array<Common::String> fonts;
	fonts.push_back("font01.CRF");
	fonts.push_back("font02.CRF");
	fonts.push_back("font03.CRF");
	fonts.push_back("font04.CRF");
	fonts.push_back("font05.CRF");
	fonts.push_back("font06.CRF");
	fonts.push_back("font07.CRF");
	fonts.push_back("font08.CRF");
	fonts.push_back("font09.CRF");
	fonts.push_back("font10.CRF");
	fonts.push_back("font11.CRF");

	_fontManager.loadFonts(fonts);
}

void CryOmni3DEngine_Versailles::setupSprites() {
	Common::File file;

	if (!file.open("all_spr.bin")) {
		error("Failed to open all_spr.bin file");
	}
	_sprites.loadSprites(file);

	for (unsigned int i = 0; i < _sprites.getSpritesCount(); i++) {
		const Graphics::Cursor &cursor = _sprites.getCursor(i);
		if (cursor.getWidth() != 32 || cursor.getHeight() != 32) {
			_sprites.setSpriteHotspot(i, 8, 8);
		} else {
			_sprites.setSpriteHotspot(i, 16, 16);
		}
	}
	_sprites.setupMapTable(kSpritesMapTable, kSpritesMapTableSize);


	_sprites.setSpriteHotspot(181, 4, 0);
	// Replace 2-keys, 3-keys and 4-keys icons with 1-key ones
	_sprites.replaceSprite(80, 64);
	_sprites.replaceSprite(84, 66);
	_sprites.replaceSprite(93, 78);
	_sprites.replaceSprite(97, 82);
	_sprites.replaceSprite(92, 64);
	_sprites.replaceSprite(96, 66);
	_sprites.replaceSprite(116, 78);
	_sprites.replaceSprite(121, 82);
	_sprites.replaceSprite(115, 64);
	_sprites.replaceSprite(120, 66);
	_sprites.replaceSprite(135, 78);
	_sprites.replaceSprite(140, 82);
}

void CryOmni3DEngine_Versailles::loadCursorsPalette() {
	Image::BitmapDecoder bmpDecoder;

	Common::File file;

	if (!file.open("bou1_cA.bmp")) {
		error("Failed to open BMP file");
	}

	if (!bmpDecoder.loadStream(file)) {
		error("Failed to load BMP file");
	}

	_cursorPalette = new byte[3 * (bmpDecoder.getPaletteColorCount() +
	                               bmpDecoder.getPaletteStartIndex())];
	memset(_cursorPalette, 0, 3 * (bmpDecoder.getPaletteColorCount() +
	                               bmpDecoder.getPaletteStartIndex()));
	memcpy(_cursorPalette + 3 * bmpDecoder.getPaletteStartIndex(), bmpDecoder.getPalette(),
	       3 * bmpDecoder.getPaletteColorCount());
}

void CryOmni3DEngine_Versailles::setupPalette(const byte *palette, uint start, uint num,
        bool commit) {
	memcpy(_mainPalette + 3 * start, palette, 3 * num);
	copySubPalette(_mainPalette, _cursorPalette, 240, 8);

	calculateTransparentMapping();
	if (commit) {
		setPalette(_mainPalette, 0, 256);
	}
}

struct transparentScore {
	unsigned int score;
	byte redScaled;
	byte greenScaled;

	int dScore(transparentScore &other) { return abs((int)score - (int)other.score); }
	int dRed(transparentScore &other) { return abs((int)redScaled - (int)other.redScaled); }
	int dGreen(transparentScore &other) { return abs((int)greenScaled - (int)other.greenScaled); }
};

static transparentScore transparentCalculateScore(byte red, byte green, byte blue) {
	transparentScore ret;
	ret.score = 10 * (blue + 3 * (red + 2 * green)) / 30;
	if (ret.score) {
		ret.redScaled = ((unsigned int)red) * 256 / ret.score;
		ret.greenScaled = ((unsigned int)green) * 256 / ret.score;
	} else {
		ret.redScaled = 0;
		ret.greenScaled = 0;
	}
	return ret;
}

void CryOmni3DEngine_Versailles::calculateTransparentMapping() {
	// Calculate colors proximity array
	transparentScore *proximities = new transparentScore[256];

	for (unsigned int i = _transparentSrcStart; i < _transparentSrcStop; i++) {
		proximities[i] = transparentCalculateScore(_mainPalette[3 * i + 0], _mainPalette[3 * i + 1],
		                 _mainPalette[3 * i + 2]);
	}

	unsigned int newColorsNextId = _transparentNewStart;
	unsigned int newColorsCount = 0;
	for (unsigned int i = _transparentDstStart; i < _transparentDstStop; i++) {
		byte transparentRed = ((unsigned int)_mainPalette[3 * i + 0]) * 60 / 128;
		byte transparentGreen = ((unsigned int)_mainPalette[3 * i + 1]) * 50 / 128;
		byte transparentBlue = ((unsigned int)_mainPalette[3 * i + 2]) * 35 / 128;

		// Find nearest color
		transparentScore newColorScore = transparentCalculateScore(transparentRed, transparentGreen,
		                                 transparentBlue);
		unsigned int distanceMin = -1u;
		unsigned int nearestId = -1u;
		for (unsigned int j = _transparentSrcStart; j < _transparentSrcStop; j++) {
			if (j != i && newColorScore.dScore(proximities[j]) < 15) {
				unsigned int distance = newColorScore.dRed(proximities[j]) + newColorScore.dGreen(proximities[j]);
				if (distance < distanceMin) {
					distanceMin = distance;
					nearestId = j;
				}
			}
		}

		if (nearestId == -1u) {
			// Couldn't find a good enough color, try to create one
			if (_transparentNewStart != -1u && newColorsNextId <= _transparentNewStop) {
				_mainPalette[3 * newColorsNextId + 0] = transparentRed;
				_mainPalette[3 * newColorsNextId + 1] = transparentGreen;
				_mainPalette[3 * newColorsNextId + 2] = transparentBlue;
				nearestId = newColorsNextId;

				newColorsCount++;
				newColorsNextId++;
			}
		}

		if (nearestId == -1u) {
			// Couldn't allocate a new color, return the original one
			nearestId = i;
		}

		_transparentPaletteMap[i] = nearestId;
	}

	delete[] proximities;
}

void CryOmni3DEngine_Versailles::makeTranslucent(Graphics::Surface &dst,
        const Graphics::Surface &src) const {
	assert(dst.w == src.w && dst.h == src.h);

	const byte *srcP = (const byte *) src.getPixels();
	byte *dstP = (byte *) dst.getPixels();
	for (unsigned int y = 0; y < dst.h; y++) {
		for (unsigned int x = 0; x < dst.w; x++) {
			dstP[x] = _transparentPaletteMap[srcP[x]];
		}
		dstP += dst.pitch;
		srcP += src.pitch;
	}
}

bool CryOmni3DEngine_Versailles::hasPlaceDocumentation() {
	return _placeStates[_currentPlaceId].docImage != nullptr;
}

bool CryOmni3DEngine_Versailles::displayPlaceDocumentation() {
	if (!_placeStates[_currentPlaceId].docImage) {
		return false;
	}

	_docManager.handleDocInGame(_placeStates[_currentPlaceId].docImage);
	return true;
}

void CryOmni3DEngine_Versailles::syncOmni3DSettings() {
	ConfMan.registerDefault("omni3d_speed", 0);
	int confOmni3DSpeed = ConfMan.getInt("omni3d_speed");
	if (confOmni3DSpeed == 0) {
		_omni3dSpeed = 0;
	} else if (confOmni3DSpeed == 1) {
		_omni3dSpeed = 2;
	} else if (confOmni3DSpeed == 2) {
		_omni3dSpeed = 4;
	} else if (confOmni3DSpeed == 3) {
		_omni3dSpeed = -1;
	} else if (confOmni3DSpeed == 4) {
		_omni3dSpeed = -2;
	} else {
		// Invalid value
		_omni3dSpeed = 0;
	}
}

void CryOmni3DEngine_Versailles::syncSoundSettings() {
	CryOmni3DEngine::syncSoundSettings();

	int soundVolumeMusic = ConfMan.getInt("music_volume") / _musicVolumeFactor;

	bool mute = false;
	if (ConfMan.hasKey("mute")) {
		mute = ConfMan.getBool("mute");
	}

	bool musicMute = mute  || ConfMan.getBool("music_mute");

	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, musicMute);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
}

void CryOmni3DEngine_Versailles::playTransitionEndLevel(int level) {
	musicStop();
	_gameVariables[GameVariables::kWarnedIncomplete] = 0;

	Common::String video;

	unlockPalette();
	switch (level) {
	case -2:
		video = "logo.hnm";
		break;
	case -1:
		video = "a0_vf.hns";
		break;
	case 0:
		video = "a1_vf.hns";
		break;
	case 1:
		video = "a2_vf.hns";
		break;
	case 2:
		video = "a3_vf.hns";
		_inventory.removeByNameId(96);
		_inventory.removeByNameId(104);
		break;
	case 3:
		video = "a4_vf.hns";
		break;
	case 4:
		video = "a5_vf.hns";
		_inventory.removeByNameId(101);
		_inventory.removeByNameId(127);
		_inventory.removeByNameId(129);
		_inventory.removeByNameId(130);
		_inventory.removeByNameId(131);
		_inventory.removeByNameId(132);
		_inventory.removeByNameId(126);
		break;
	case 5:
		video = "a6_vf.hns";
		_inventory.removeByNameId(115);
		break;
	case 6:
		video = "a7_vf.hns";
		break;
	case 7:
		video = "a9_vf.hns";
		break;
	case 8:
		video = "a8_vf.hns";
		break;
	default:
		error("Invalid level : %d", level);
	}

	fadeOutPalette();
	if (g_engine->shouldQuit()) {
		_abortCommand = AbortQuit;
		return;
	}

	fillSurface(0);

	// Videos are like music because if you mute music in game it will mute videos soundtracks
	playHNM(video, Audio::Mixer::kMusicSoundType);
	clearKeys();
	if (g_engine->shouldQuit()) {
		_abortCommand = AbortQuit;
		return;
	}

	fadeOutPalette();
	if (g_engine->shouldQuit()) {
		_abortCommand = AbortQuit;
		return;
	}

	fillSurface(0);

	if (level == 7 || level == 8) {
		_abortCommand = AbortFinished;
	} else {
		_abortCommand = AbortNextLevel;
	}
}

void CryOmni3DEngine_Versailles::changeLevel(int level) {
	_currentLevel = level;

	musicStop();
	_mixer->stopAll();

	if (_currentLevel == 1) {
		_dialogsMan.reinitVariables();
		for (Common::Array<unsigned int>::iterator it = _gameVariables.begin(); it != _gameVariables.end();
		        it++) {
			*it = 0;
		}
		// TODO: countdown
		_inventory.clear();
	} else {
		// TODO: to implement
		error("New level %d is not implemented (yet)", level);
	}

	_gameVariables[GameVariables::kCurrentTime] = 1;

	// keep back place state for level 2
	int place8_state_backup;
	if (level == 2) {
		place8_state_backup = _placeStates[8].state;
	}
	_nextPlaceId = -1;
	initNewLevel(_currentLevel);
	// restore place state for level 2
	if (level == 2) {
		_placeStates[8].state = place8_state_backup;
	}
}

void CryOmni3DEngine_Versailles::initNewLevel(int level) {
	// SearchMan can't add several times the same basename
	// We create several SearchSet with different names that we add to SearchMan instead

	// Visiting uses all levels
	SearchMan.remove("__visitFiles");

	SearchMan.remove("__levelFiles_animacti");
	SearchMan.remove("__levelFiles_warp");
	SearchMan.remove("__levelFiles_img_fix");

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	if (level >= 1 && level <= 7) {
		Common::SearchSet *levelFilesAnimacti = new Common::SearchSet();
		Common::SearchSet *levelFilesWarp = new Common::SearchSet();
		Common::SearchSet *levelFilesImgFix = new Common::SearchSet();

		levelFilesAnimacti->addSubDirectoryMatching(gameDataDir, Common::String::format(
		            "datas_v/animacti/level%d", level), 1);
		levelFilesWarp->addSubDirectoryMatching(gameDataDir, Common::String::format(
		        "datas_v/warp/level%d/cyclo", level), 1);
		levelFilesWarp->addSubDirectoryMatching(gameDataDir, Common::String::format(
		        "datas_v/warp/level%d/hnm", level), 1);
		levelFilesImgFix->addSubDirectoryMatching(gameDataDir, Common::String::format(
		            "datas_v/img_fix/level%d", level), 1);

		SearchMan.add("__levelFiles_animacti", levelFilesAnimacti);
		SearchMan.add("__levelFiles_warp", levelFilesWarp);
		SearchMan.add("__levelFiles_img_fix", levelFilesImgFix);
	} else if (level == 8 && _isVisiting) {
		// In visit mode, we take files from all levels, happily they have unique names
		// Create a first SearchSet in which we will add all others to easily cleanup the mess
		Common::SearchSet *visitFiles = new Common::SearchSet();

		for (unsigned int lvl = 1; lvl <= 7; lvl++) {
			Common::SearchSet *visitFilesAnimacti = new Common::SearchSet();
			Common::SearchSet *visitFilesWarp = new Common::SearchSet();
			Common::SearchSet *visitFilesImgFix = new Common::SearchSet();

			visitFilesAnimacti->addSubDirectoryMatching(gameDataDir, Common::String::format(
			            "datas_v/animacti/level%d", lvl), 1);
			visitFilesWarp->addSubDirectoryMatching(gameDataDir, Common::String::format(
			        "datas_v/warp/level%d/cyclo", lvl), 1);
			visitFilesWarp->addSubDirectoryMatching(gameDataDir, Common::String::format(
			        "datas_v/warp/level%d/hnm", lvl), 1);
			visitFilesImgFix->addSubDirectoryMatching(gameDataDir, Common::String::format(
			            "datas_v/img_fix/level%d", lvl), 1);

			visitFiles->add(Common::String::format("__visitFiles_animacti_%d", lvl), visitFilesAnimacti);
			visitFiles->add(Common::String::format("__visitFiles_warp_%d", lvl), visitFilesWarp);
			visitFiles->add(Common::String::format("__visitFiles_img_fix_%d", lvl), visitFilesImgFix);
		}

		SearchMan.add("__visitFiles", visitFiles);
	} else {
		error("Invalid level %d", level);
	}

	// TODO: countdown
	initPlacesStates();
	initWhoSpeaksWhere();
	setupLevelWarps(level);
	updateGameTimeDialVariables();
	_dialogsMan["{JOUEUR-ESSAYE-OUVRIR-PORTE-SALON}"] = 'Y';
	_dialogsMan["{JOUEUR-ESSAYE-OUVRIR-PORTE-CHAMBRE}"] = 'Y';
	setupLevelActionsMask();
}

void CryOmni3DEngine_Versailles::setupLevelWarps(int level) {
	Common::File wamFile;
	Common::String wamFName = Common::String::format("level%d.wam", level);
	if (!wamFile.open(wamFName)) {
		error("Can't open WAM file '%s'", wamFName.c_str());
	}
	_wam.loadStream(wamFile);

	const LevelInitialState &initialState = kLevelInitialStates[level - 1];

	if (_nextPlaceId == -1u) {
		_nextPlaceId = initialState.placeId;
	}
	_omni3dMan.setAlpha(initialState.alpha);
	_omni3dMan.setBeta(initialState.beta);
}

void CryOmni3DEngine_Versailles::setGameTime(unsigned int newTime, unsigned int level) {
	if (_currentLevel != level) {
		error("Level %u != current level %u", level, _currentLevel);
	}

	_gameVariables[GameVariables::kCurrentTime] = newTime;
	updateGameTimeDialVariables();

	if (_currentLevel == 1) {
		if (newTime == 2) {
			setPlaceState(1, 1);
			setPlaceState(2, 1);
			_whoSpeaksWhere[PlaceActionKey(2, 11201)] = "12E_HUI";
			setPlaceState(3, 1);
		} else if (newTime == 3) {
			setPlaceState(2, 2);
		}
	} else if (_currentLevel == 2) {
		if (newTime == 2) {
			setPlaceState(9, 1);
			_whoSpeaksWhere[PlaceActionKey(9, 52902)] = "22G_DAU";
		} else if (newTime == 4) {
			setPlaceState(10, 1);
			setPlaceState(11, 1);
			setPlaceState(12, 1);
			setPlaceState(13, 1);
		}
	} else if (_currentLevel == 3) {
		if (newTime == 2) {
			if (_placeStates[13].state) {
				setPlaceState(13, 3);
			} else {
				setPlaceState(13, 2);
			}
			setPlaceState(15, 1);
			setPlaceState(17, 1);
		} else if (newTime == 3) {
			setPlaceState(10, 1);
			setPlaceState(14, 1);
		}
	} else if (_currentLevel == 4) {
		if (newTime == 2) {
			setPlaceState(7, 1);
			setPlaceState(8, 1);
			setPlaceState(10, 1);
			setPlaceState(16, 1);
		} else if (newTime == 3) {
			setPlaceState(10, 2);
			setPlaceState(9, 1);
		} else if (newTime == 4) {
			setPlaceState(9, 2);
			_whoSpeaksWhere[PlaceActionKey(9, 54091)] = "4_MAI";
			_whoSpeaksWhere[PlaceActionKey(9, 14091)] = "4_MAI";
		}
	} else if (_currentLevel == 5) {
		if (newTime == 2) {
			setPlaceState(9, 1);
			setPlaceState(13, 1);
		} else if (newTime == 3) {
			if (!_placeStates[16].state) {
				setPlaceState(16, 2);
			}
		} else if (newTime == 4) {
			_whoSpeaksWhere[PlaceActionKey(9, 15090)] = "54I_BON";
		}
	} else if (_currentLevel == 6) {
		if (newTime == 2) {
			setPlaceState(14, 1);
			setPlaceState(19, 2);
		}
	}
}

void CryOmni3DEngine_Versailles::gameStep() {
	while (!_abortCommand) {
		if (_nextPlaceId != -1u) {
			// TODO: check selected object == -2 if needed
			if (_placeStates[_nextPlaceId].initPlace) {
				(this->*_placeStates[_nextPlaceId].initPlace)();
				// TODO: check selected object == -2 if needed
			}
			doPlaceChange();
			musicUpdate();
		}
		if (_forcePaletteUpdate) {
			redrawWarp();
		}
		unsigned int actionId = handleWarp();
		debug("handleWarp returned %u", actionId);
		// TODO: handle keyboard levels 4 and 5

		// Get selected object there to detect when it has just been deselected
		Object *selectedObject = _inventory.selectedObject();

		_nextPlaceId = -1;
		bool doEvent;
		if (_placeStates[_currentPlaceId].filterEvent && !_isVisiting) {
			doEvent = (this->*_placeStates[_currentPlaceId].filterEvent)(&actionId);
		} else {
			doEvent = true;
		}

		if (_abortCommand) {
			break;
		}

		if (!selectedObject) {
			// Execute only when no object was used before filtering event
			if (actionId >= 1 && actionId < 10000) {
				// Move to another place
				if (doEvent) {
					executeTransition(actionId);
				}
			} else if (actionId >= 10000 && actionId < 20000) {
				// Speak
				if (doEvent) {
					executeSpeakAction(actionId);
					// Force refresh of place
					if (_nextPlaceId == -1u) {
						_nextPlaceId = _currentPlaceId;
					}
				}
			} else if (actionId >= 20000 && actionId < 30000) {
				// Documentation
				executeDocAction(actionId);
			} else if (actionId >= 30000 && actionId < 40000) {
				// Use
				// In original game there is a handler for use actions but it's
				// only for some events, we will implement them in the filterEvent handler
				if (doEvent) {
					error("Not implemented yet");
				}
			} else if (actionId >= 40000 && actionId < 50000) {
				// See
				executeSeeAction(actionId);
			} else if (actionId >= 50000 && actionId < 60000) {
				// Listening
				// never filtered
				executeSpeakAction(actionId);
				// Force refresh of place
				if (_nextPlaceId == -1u) {
					_nextPlaceId = _currentPlaceId;
				}
			} else if (actionId == 66666) {
				// Abort
				assert(_abortCommand != AbortNoAbort);
				return;
			}
		} else if (!actionId) {
			// Click on nothing with an object: deselect it
			_inventory.setSelectedObject(nullptr);
		}
		// TODO: selected_object == -2 if needed
	}
}

void CryOmni3DEngine_Versailles::doGameOver() {
	musicStop();
	fadeOutPalette();
	fillSurface(0);
	// This test is not really relevant because it's for 2CDs edition but let's follow the code
	if (_currentLevel < 4) {
		playInGameVideo("1gameove");
	} else {
		playInGameVideo("4gameove");
	}
	fillSurface(0);
	_abortCommand = AbortGameOver;
}

void CryOmni3DEngine_Versailles::doPlaceChange() {
	const Place *nextPlace = _wam.findPlaceById(_nextPlaceId);
	unsigned int state = _placeStates[_nextPlaceId].state;
	if (state == -1u) {
		state = 0;
	}

	if (state >= nextPlace->warps.size()) {
		error("invalid warp %d/%d/%d", _currentLevel, _nextPlaceId, state);
	}

	Common::String warpFile = nextPlace->warps[state];
	warpFile.toUppercase();
	if (warpFile.size() > 0) {
		if (warpFile.hasPrefix("NOT_MOVE")) {
			// Don't move so do nothing but cancel place change
			_nextPlaceId = -1;
		} else {
			_currentPlace = nextPlace;
			if (!warpFile.hasPrefix("NOT_STOP")) {
				if (_currentWarpImage) {
					delete _currentWarpImage;
				}
				debug("Loading warp %s", warpFile.c_str());
				_currentWarpImage = loadHLZ(warpFile);
				if (!_currentWarpImage) {
					error("Can't load warp %s", warpFile.c_str());
				}
#if 0
				// This is not correct but to debug zones I think it's OK
				Graphics::Surface *tmpSurf = (Graphics::Surface *) _currentWarpImage->getSurface();
				for (Common::Array<Zone>::const_iterator it = _currentPlace->zones.begin();
				        it != _currentPlace->zones.end(); it++) {
					Common::Rect tmp = it->rct;
					tmp.bottom = tmpSurf->h - it->rct.top;
					tmp.top = tmpSurf->h - it->rct.bottom;
					tmpSurf->frameRect(tmp, 244);
				}
#endif
				_currentPlace->setupWarpConstraints(_omni3dMan);
				_omni3dMan.setSourceSurface(_currentWarpImage->getSurface());

				setupPalette(_currentWarpImage->getPalette(), _currentWarpImage->getPaletteStartIndex(),
				             _currentWarpImage->getPaletteColorCount(), !_fadedPalette);

				setMousePos(Common::Point(320, 240)); // Center of screen

				_currentPlaceId = _nextPlaceId;
				_nextPlaceId = -1;
			}
		}
	} else {
		error("invalid warp %d/%d/%d", _currentLevel, _nextPlaceId, state);
	}
}

void CryOmni3DEngine_Versailles::setPlaceState(unsigned int placeId, unsigned int newState) {
	unsigned int numStates = _wam.findPlaceById(placeId)->getNumStates();
	unsigned int oldState = _placeStates[placeId].state;

	if (newState > numStates) {
		warning("CryOmni3DEngine_Versailles::setPlaceState: newState '%d' > numStates '%d'",
		        newState, numStates);
		return;
	}
	_placeStates[placeId].state = newState;

	if (_currentPlaceId == placeId && oldState != newState) {
		// force reload of the place
		_nextPlaceId = _currentPlaceId;
	}
}

void CryOmni3DEngine_Versailles::executeTransition(unsigned int nextPlaceId) {
	const Transition *transition;
	unsigned int animationId = determineTransitionAnimation(_currentPlaceId, nextPlaceId, &transition);

	_nextPlaceId = nextPlaceId;

	Common::String animation = transition->animations[animationId];
	animation.toUppercase();
	debug("Transition animation: %s", animation.c_str());
	if (animation.hasPrefix("NOT_FLI")) {
		return;
	}

	if (_transitionAnimateWarp) {
		animateWarpTransition(transition);
	} else {
		_transitionAnimateWarp = true;
	}
	if (musicWouldChange(_currentLevel, _nextPlaceId)) {
		musicStop();
	}
	if (animation.hasPrefix("FADE_PAL")) {
		_fadedPalette = true;
		fadeOutPalette();
	} else if (animation != "") {
		_fadedPalette = false;
		// Normally transitions don't overwrite the cursors colors and game doesn't restore palette
		playInGameVideo(animation, false);
	}

	_omni3dMan.setAlpha(transition->dstAlpha);
	_omni3dMan.setBeta(-transition->dstBeta);

	unsigned int nextState = _placeStates[nextPlaceId].state;
	if (nextState == -1u) {
		nextState = 0;
	}
	const Place *nextPlace = _wam.findPlaceById(nextPlaceId);
	Common::String warpFile = nextPlace->warps[nextState];
	warpFile.toUppercase();
	if (warpFile.hasPrefix("NOT_STOP")) {
		unsigned int transitionNum;
		// Determine transition to take
		if (nextPlace->getNumTransitions() == 1) {
			// Only one
			transitionNum = 0;
		} else if (nextPlace->findTransition(_currentPlaceId) == &nextPlace->transitions[0]) {
			// Don't take the transition returning to where we come from
			transitionNum = 1;
		} else {
			transitionNum = 0;
		}
		unsigned int nextNextPlaceId = nextPlace->transitions[transitionNum].dstId;

		animationId = determineTransitionAnimation(nextPlaceId, nextNextPlaceId, &transition);
		animation = transition->animations[animationId];

		animation.toUppercase();
		if (animation.hasPrefix("NOT_FLI")) {
			return;
		}
		if (animation.hasPrefix("FADE_PAL")) {
			_fadedPalette = true;
			fadeOutPalette();
		} else if (animation != "") {
			_fadedPalette = false;
			// Normally transitions don't overwrite the cursors colors and game doesn't restore palette
			playInGameVideo(animation, false);
		}

		_nextPlaceId = nextNextPlaceId;

		_omni3dMan.setAlpha(transition->dstAlpha);
		_omni3dMan.setBeta(-transition->dstBeta);
	}
}

void CryOmni3DEngine_Versailles::fakeTransition(unsigned int dstPlaceId) {
	// No need of animation, caller will take care
	// We just setup the camera in good place for the caller
	const Place *srcPlace = _wam.findPlaceById(_currentPlaceId);
	const Transition *transition = srcPlace->findTransition(dstPlaceId);

	animateWarpTransition(transition);

	_omni3dMan.setAlpha(transition->dstAlpha);
	_omni3dMan.setBeta(-transition->dstBeta);
}

unsigned int CryOmni3DEngine_Versailles::determineTransitionAnimation(unsigned int srcPlaceId,
        unsigned int dstPlaceId, const Transition **transition_) {
	const Place *srcPlace = _wam.findPlaceById(srcPlaceId);
	const Place *dstPlace = _wam.findPlaceById(dstPlaceId);
	const Transition *transition = srcPlace->findTransition(dstPlaceId);

	if (transition_) {
		*transition_ = transition;
	}

	unsigned int srcNumStates = srcPlace->getNumStates();
	unsigned int dstNumStates = dstPlace->getNumStates();
	unsigned int animsNum = transition->getNumAnimations();

	unsigned int srcState = _placeStates[srcPlaceId].state;
	unsigned int dstState = _placeStates[dstPlaceId].state;

	if (srcState >= srcNumStates) {
		error("Invalid src state");
	}

	if (dstState >= dstNumStates) {
		error("Invalid dst state");
	}

	if (animsNum <= 1) {
		return 0;
	}

	if (srcNumStates == 2 && dstNumStates == 2) {
		if (animsNum == 2) {
			return dstState;
		} else if (animsNum == 4) {
			return srcState * 2 + dstState;
		}
	}

	if (animsNum == dstNumStates) {
		return dstState;
	}

	if (animsNum == srcNumStates) {
		return srcState;
	}

	// Other case
	return 0;
}

int CryOmni3DEngine_Versailles::handleWarp() {
	bool exit = false;
	bool leftButtonPressed = false;
	bool firstDraw = true;
	bool moving = true;
	unsigned int actionId;
	g_system->showMouse(true);
	while (!leftButtonPressed && !exit) {
		int xDelta = 0, yDelta = 0;
		unsigned int movingCursor = -1;

		pollEvents();
		Common::Point mouse = getMousePos();

		if (mouse.y < 100) {
			movingCursor = 245;
			yDelta = 100 - mouse.y;
		} else if (mouse.y > 380) {
			movingCursor = 224;
			yDelta = 380 - mouse.y;
		}
		if (mouse.x < 100) {
			movingCursor = 241;
			xDelta = 100 - mouse.x;
		} else if (mouse.x > 540) {
			movingCursor = 228;
			xDelta = 540 - mouse.x;
		}
		if (_omni3dSpeed > 0) {
			xDelta <<= _omni3dSpeed;
			yDelta <<= _omni3dSpeed;
		} else if (_omni3dSpeed < 0) {
			xDelta >>= -_omni3dSpeed;
			yDelta >>= -_omni3dSpeed;
		}
		leftButtonPressed = (getCurrentMouseButton() == 1);

		Common::Point mouseRev = _omni3dMan.mapMouseCoords(mouse);
		mouseRev.y = 768 - mouseRev.y;

		actionId = _currentPlace->hitTest(mouseRev);

		exit = handleWarpMouse(&actionId, movingCursor);
		if (g_engine->shouldQuit()) {
			_abortCommand = AbortQuit;
			exit = true;
		}
		if (exit) {
			actionId = 66666;
		}

		if (firstDraw || xDelta || yDelta || _omni3dMan.hasSpeed()) {
			bool useOldSpeed = false;
			if (_omni3dSpeed <= 2) {
				useOldSpeed = true;
			}
			_omni3dMan.updateCoords(xDelta, -yDelta, useOldSpeed);

			const Graphics::Surface *result = _omni3dMan.getSurface();
			g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
			if (!exit) {
				// TODO: countdown
				g_system->updateScreen();
				if (firstDraw && _fadedPalette) {
					fadeInPalette(_mainPalette);
					_fadedPalette = false;
				}
			}
			moving = true;
			firstDraw = false;
		} else if (moving) {
			const Graphics::Surface *result = _omni3dMan.getSurface();
			g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
			if (!exit) {
				// TODO: countdown
				g_system->updateScreen();
			}
			// TODO: cursorUseZones
			moving = false;
		} else {
			if (!exit) {
				// TODO: countdown
				g_system->updateScreen();
			}
		}
		if (!exit && !leftButtonPressed) {
			g_system->delayMillis(50);
		}
	}
	g_system->showMouse(false);
	return actionId;
}

bool CryOmni3DEngine_Versailles::handleWarpMouse(unsigned int *actionId,
        unsigned int movingCursor) {
	PlaceStateActionKey mask = PlaceStateActionKey(_currentPlaceId, _placeStates[_currentPlaceId].state,
	                           *actionId);
	*actionId = _actionMasks.getVal(mask, *actionId);

	if (getCurrentMouseButton() == 2 ||
	        getNextKey().keycode == Common::KEYCODE_SPACE) {
		// Prepare background using alpha
		const Graphics::Surface *original = _omni3dMan.getSurface();
		bool mustRedraw = displayToolbar(original);
		// Don't redraw if we abort game
		if (_abortCommand != AbortNoAbort) {
			return true;
		}
		if (mustRedraw) {
			_forceRedrawWarp = true;
			redrawWarp();
		}
		// Force a cycle to recalculate the correct mouse cursor
		return false;
	}

	// TODO: countdown

	const Object *selectedObj = _inventory.selectedObject();
	if (selectedObj) {
		if (*actionId != 0) {
			setCursor(selectedObj->idSA());
		} else {
			setCursor(selectedObj->idSl());
		}
	} else if (*actionId >= 1 && *actionId < 10000) {
		setCursor(243);
	} else if (*actionId >= 10000 && *actionId < 20000) {
		setCursor(113);
	} else if (*actionId >= 20000 && *actionId < 30000) {
		setCursor(198);
	} else if (*actionId >= 30000 && *actionId < 40000) {
		setCursor(99);
	} else if (*actionId >= 40000 && *actionId < 50000) {
		setCursor(145);
	} else if (*actionId >= 50000 && *actionId < 60000) {
		setCursor(136);
	} else if (movingCursor != -1u) {
		setCursor(movingCursor);
	} else {
		setCursor(45);
	}
	return false;
}

void CryOmni3DEngine_Versailles::animateWarpTransition(const Transition *transition) {
	double srcAlpha = transition->srcAlpha;
	double srcBeta = transition->srcBeta;

	double oldDeltaAlpha = 1000., oldDeltaBeta = 1000.;

	clearKeys();

	bool exit = false;
	while (!exit) {
		double deltaAlpha = 2.*M_PI - srcAlpha + _omni3dMan.getAlpha();
		if (deltaAlpha >= 2.*M_PI) {
			deltaAlpha -= 2.*M_PI;
		} else if (deltaAlpha < 0) {
			deltaAlpha += 2.*M_PI;
		}
		int deltaAlphaI;
		if (deltaAlpha < M_PI) {
			deltaAlphaI = -(deltaAlpha * 512.);
		} else {
			deltaAlphaI = (2.*M_PI - deltaAlpha) * 512.;
		}

		double deltaBeta = -srcBeta - _omni3dMan.getBeta();
		int deltaBetaI = -(deltaBeta * 512.);

		if (_omni3dSpeed > 0) {
			deltaAlphaI <<= 2;
			deltaBetaI <<= 2;
		} else if (_omni3dSpeed < 0) {
			deltaAlphaI >>= 2;
			deltaBetaI >>= 2;
		}

		_omni3dMan.updateCoords(deltaAlphaI, -deltaBetaI, false);

		const Graphics::Surface *result = _omni3dMan.getSurface();
		g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
		// TODO: countdown
		g_system->updateScreen();

		if (abs(oldDeltaAlpha - deltaAlpha) < 0.001 && abs(oldDeltaBeta - deltaBeta) < 0.001) {
			exit = true;
		}
		oldDeltaAlpha = deltaAlpha;
		oldDeltaBeta = deltaBeta;

		if (pollEvents() && checkKeysPressed(2, Common::KEYCODE_ESCAPE, Common::KEYCODE_SPACE)) {
			exit = true;
		}

		if (!exit) {
			g_system->delayMillis(50);
		}
	}
}

void CryOmni3DEngine_Versailles::redrawWarp() {
	setupPalette(_currentWarpImage->getPalette(), _currentWarpImage->getPaletteStartIndex(),
	             _currentWarpImage->getPaletteColorCount(), true);
	if (_forceRedrawWarp) {
		const Graphics::Surface *result = _omni3dMan.getSurface();
		g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
		// TODO: countdown
		g_system->updateScreen();
		_forceRedrawWarp = false;
	}
	_forcePaletteUpdate = false;
}

void CryOmni3DEngine_Versailles::warpMsgBoxCB() {
	pollEvents();
}

void CryOmni3DEngine_Versailles::animateCursor(const Object *obj) {
	if (obj == nullptr) {
		return;
	}

	g_system->showMouse(true);

	for (unsigned int i = 4; i > 0; i--) {
		// Wait 100ms
		for (unsigned int j = 10; j > 0; j--) {
			// pollEvents sleeps 10ms
			pollEvents();
			g_system->updateScreen();
		}
		setCursor(obj->idSA());
		g_system->updateScreen();
		// Wait 100ms
		for (unsigned int j = 10; j > 0; j--) {
			// pollEvents sleeps 10ms
			pollEvents();
			g_system->updateScreen();
		}
		setCursor(obj->idSl());
		g_system->updateScreen();
	}

	g_system->showMouse(false);
}

void CryOmni3DEngine_Versailles::collectObject(unsigned int nameID, const ZonFixedImage *fimg,
        bool showObject) {
	Object *obj = _objects.findObjectByNameID(nameID);
	_inventory.add(obj);
	Object::ViewCallback cb = obj->viewCallback();
	if (showObject && cb) {
		(*cb)();
		if (fimg) {
			fimg->display();
		} else {
			_forceRedrawWarp = true;
			redrawWarp();
		}
	}
	animateCursor(obj);
}

void CryOmni3DEngine_Versailles::displayObject(const Common::String &imgName,
        DisplayObjectHook hook) {
	Image::ImageDecoder *imageDecoder = loadHLZ(imgName);
	if (!imageDecoder) {
		error("Can't display object");
	}

	if (imageDecoder->hasPalette()) {
		// We don't need to calculate transparency but it's simpler to call this function
		setupPalette(imageDecoder->getPalette(), imageDecoder->getPaletteStartIndex(),
		             imageDecoder->getPaletteColorCount());
	}

	const Graphics::Surface *image = imageDecoder->getSurface();

	// Duplicate image to let hook modify it
	Graphics::ManagedSurface dstSurface(image->w, image->h, image->format);
	dstSurface.blitFrom(*image);

	delete imageDecoder;
	imageDecoder = nullptr;

	if (hook) {
		(this->*hook)(dstSurface);
	}

	g_system->copyRectToScreen(dstSurface.getPixels(), dstSurface.pitch, 0, 0,
	                           dstSurface.w, dstSurface.h);
	g_system->updateScreen();

	setMousePos(Common::Point(320, 240)); // Center of screen
	setCursor(181);

	g_system->showMouse(true);

	bool exitImg = false;
	while (!g_engine->shouldQuit() && !exitImg) {
		if (pollEvents()) {
			if (getCurrentMouseButton() == 1) {
				exitImg = true;
			}
		}
		g_system->updateScreen();
	}
	waitMouseRelease();
	clearKeys();

	g_system->showMouse(false);
	setMousePos(Common::Point(320, 240)); // Center of screen
}

void CryOmni3DEngine_Versailles::executeSeeAction(unsigned int actionId) {
	if (_currentLevel == 7 && _currentPlaceId != 20) {
		// Not enough time for paintings
		displayMessageBoxWarp(14);
		return;
	}

	const FixedImgCallback &cb = _imgScripts.getVal(actionId, nullptr);
	if (cb != nullptr) {
		handleFixedImg(cb);
	} else {
		warning("Image script %u not found", actionId);
	}
}

void CryOmni3DEngine_Versailles::executeSpeakAction(unsigned int actionId) {
	PlaceActionKey key(_currentPlaceId, actionId);
	Common::HashMap<PlaceActionKey, Common::String>::iterator it = _whoSpeaksWhere.find(key);
	g_system->showMouse(true);
	bool doneSth = false;
	if (it != _whoSpeaksWhere.end()) {
		doneSth = _dialogsMan.play(it->_value);
	}
	g_system->showMouse(false);
	_forcePaletteUpdate = true;
	if (doneSth) {
		setMousePos(Common::Point(320, 240)); // Center of screen
	}
}

void CryOmni3DEngine_Versailles::executeDocAction(unsigned int actionId) {
	if (_currentLevel == 7) {
		// Not enough time for doc
		displayMessageBoxWarp(13);
		return;
	}

	Common::HashMap<unsigned int, const char *>::iterator it = _docPeopleRecord.find(actionId);
	if (it == _docPeopleRecord.end() || !it->_value) {
		warning("Missing documentation record for action %u", actionId);
		return;
	}

	_docManager.handleDocInGame(it->_value);

	_forcePaletteUpdate = true;
	setMousePos(Common::Point(320, 240)); // Center of screen
}

void CryOmni3DEngine_Versailles::handleFixedImg(const FixedImgCallback &callback) {
	if (!callback) {
		return;
	}

	ZonFixedImage::CallbackFunctor *functor =
	    new Common::Functor1Mem<ZonFixedImage *, void, CryOmni3DEngine_Versailles>(this, callback);
	_fixedImage->run(functor);
	// functor is deleted in ZoneFixedImage
	functor = nullptr;

	if (_nextPlaceId == -1u) {
		_forcePaletteUpdate = true;
	}
}

unsigned int CryOmni3DEngine_Versailles::getFakeTransition(unsigned int actionId) const {
	for (const FakeTransitionActionPlace *ft = kFakeTransitions; ft->actionId != nullptr; ft++) {
		if (ft->actionId == actionId) {
			return ft->placeId;
		}
	}
	return 0;
}

void CryOmni3DEngine_Versailles::playInGameVideo(const Common::String &filename,
        bool restoreCursorPalette) {
	if (!_isPlaying) {
		return;
	}

	g_system->showMouse(false);
	lockPalette(0, 241);
	// Videos are like music because if you mute music in game it will mute videos soundtracks
	playHNM(filename, Audio::Mixer::kMusicSoundType);
	clearKeys();
	unlockPalette();
	if (restoreCursorPalette) {
		// Restore cursors colors as 2 first ones may have been erased by the video
		setPalette(&_cursorPalette[3 * 240], 240, 248);
	}
	g_system->showMouse(true);
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
