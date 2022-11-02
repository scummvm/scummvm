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

#include "common/debug-channels.h"
#include "common/events.h"
#include "common/error.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/debugger.h"
#include "gui/error.h"

#include "engines/engine.h"

#include "engines/myst3/archive.h"
#include "engines/myst3/console.h"
#include "engines/myst3/database.h"
#include "engines/myst3/effects.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/nodecube.h"
#include "engines/myst3/nodeframe.h"
#include "engines/myst3/scene.h"
#include "engines/myst3/state.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/script.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/movie.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/ambient.h"
#include "engines/myst3/transition.h"

#include "image/jpeg.h"

#include "graphics/conversion.h"
#include "graphics/renderer.h"
#include "graphics/yuv_to_rgb.h"
#include "graphics/framelimiter.h"

#include "math/vector2d.h"

namespace Myst3 {

Myst3Engine::Myst3Engine(OSystem *syst, const Myst3GameDescription *version) :
		Engine(syst), _system(syst), _gameDescription(version),
		_db(nullptr), _scriptEngine(nullptr),
		_state(nullptr), _node(nullptr), _scene(nullptr), _archiveNode(nullptr),
		_cursor(nullptr), _inventory(nullptr), _gfx(nullptr), _menu(nullptr),
		_rnd(nullptr), _sound(nullptr), _ambient(nullptr),
		_inputSpacePressed(false), _inputEnterPressed(false),
		_inputEscapePressed(false), _inputTildePressed(false),
		_inputEscapePressedNotConsumed(false),
		_interactive(false),
		_menuAction(0), _projectorBackground(nullptr),
		_shakeEffect(nullptr), _rotationEffect(nullptr),
		_backgroundSoundScriptLastRoomId(0),
		_backgroundSoundScriptLastAgeId(0),
		_transition(nullptr), _frameLimiter(nullptr), _inventoryManualHide(false) {

	// Add subdirectories to the search path to allow running from a full HDD install
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "bin");
	SearchMan.addSubDirectoryMatching(gameDataDir, "M3Data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "M3Data/TEXT");
	SearchMan.addSubDirectoriesMatching(gameDataDir, "EXILE Disc ?/Data", true);

	// Win DVD version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "English");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Data");

	// Mac DVD version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "Exile DVD");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Exile DVD/data");

	// PS2 version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA/WORLD");
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA/WORLD/SOUND");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/DISCS");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/DISCS/DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT/NTSC");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT/PAL");
}

Myst3Engine::~Myst3Engine() {
	closeArchives();

	delete _menu;
	delete _inventory;
	delete _cursor;
	delete _scene;
	delete _archiveNode;
	delete _db;
	delete _scriptEngine;
	delete _state;
	delete _rnd;
	delete _sound;
	delete _ambient;
	delete _frameLimiter;
	delete _gfx;
}

bool Myst3Engine::hasFeature(EngineFeature f) const {
	// The TinyGL renderer does not support arbitrary resolutions for now
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingAvailableType(desiredRendererType,
#if defined(USE_OPENGL_GAME)
	                Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
	                Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
	                Graphics::kRendererTypeTinyGL |
#endif
	                0);
	bool softRenderer = matchingRendererType == Graphics::kRendererTypeTinyGL;

	return (f == kSupportsReturnToLauncher) ||
	       (f == kSupportsLoadingDuringRuntime) ||
	       (f == kSupportsSavingDuringRuntime) ||
	       (f == kSupportsArbitraryResolutions && !softRenderer);
}

Common::Error Myst3Engine::run() {
	if (!checkDatafiles()) {
		// An error message has already been displayed
		return Common::kUserCanceled;
	}

	_gfx = createRenderer(_system);
	_gfx->init();
	_gfx->clear();

	_frameLimiter = new Graphics::FrameLimiter(_system, ConfMan.getInt("engine_speed"));
	_sound = new Sound(this);
	_ambient = new Ambient(this);
	_rnd = new Common::RandomSource("sprint");
	setDebugger(new Console(this));
	_scriptEngine = new Script(this);
	_db = new Database(getPlatform(), getGameLanguage(), getGameLocalizationType());
	_state = new GameState(getPlatform(), _db);
	_scene = new Scene(this);
	if (getPlatform() == Common::kPlatformXbox) {
		_menu = new AlbumMenu(this);
	} else {
		_menu = new PagingMenu(this);
	}
	_archiveNode = new Archive();

	_system->showMouse(false);

	settingsInitDefaults();
	syncSoundSettings();
	openArchives();

	_cursor = new Cursor(this);
	_inventory = new Inventory(this);

	// Init the font
	Graphics::Surface *font = loadTexture(1206);
	_gfx->initFont(font);
	font->free();
	delete font;

	if (ConfMan.hasKey("save_slot")) {
		// Load game from specified slot, if any
		Common::Error loadError = loadGameState(ConfMan.getInt("save_slot"));
		if (loadError.getCode() != Common::kNoError) {
			return loadError;
		}
	} else {
		if (getPlatform() == Common::kPlatformXbox) {
			// Play the logo videos
			loadNode(kNodeLogoPlay, kLogo, 11);
		}

		// Game init script, loads the menu
		loadNode(kNodeSharedInit, kRoomShared, 1);
	}

	while (!shouldQuit()) {
		runNodeBackgroundScripts();
		processInput(true);
		updateCursor();

		if (_menuAction) {
			_menu->updateMainMenu(_menuAction);
			_menuAction = 0;
		}

		drawFrame();
	}

	unloadNode();

	_archiveNode->close();
	_gfx->freeFont();

	// Make sure the mouse is unlocked
	_system->lockMouse(false);

	return Common::kNoError;
}

bool Myst3Engine::addArchive(const Common::String &file, bool mandatory) {
	Archive *archive = new Archive();
	bool opened = archive->open(file.c_str(), nullptr);

	if (opened) {
		_archivesCommon.push_back(archive);
	} else {
		delete archive;
		if (mandatory)
			error("Unable to open archive %s", file.c_str());
	}

	return opened;
}

void Myst3Engine::openArchives() {
	// The language of the menus is always the same as the executable
	// The English CD version can only display English text
	// The non English CD versions can display their localized language and English
	// The DVD version can display 6 different languages

	Common::String menuLanguage;
	Common::String textLanguage;

	switch (getGameLanguage()) {
	case Common::NL_NLD:
		menuLanguage = "DUTCH";
		break;
	case Common::FR_FRA:
		menuLanguage = "FRENCH";
		break;
	case Common::DE_DEU:
		menuLanguage = "GERMAN";
		break;
	case Common::HE_ISR:
		menuLanguage = "HEBREW";
		break;
	case Common::IT_ITA:
		menuLanguage = "ITALIAN";
		break;
	case Common::ES_ESP:
		menuLanguage = "SPANISH";
		break;
	case Common::JA_JPN:
		menuLanguage = "JAPANESE";
		break;
	case Common::PL_POL:
		menuLanguage = "POLISH";
		break;
	case Common::EN_ANY:
	case Common::RU_RUS:
	default:
		menuLanguage = "ENGLISH";
		break;
	}

	if (getGameLocalizationType() == kLocMulti6) {
		switch (ConfMan.getInt("text_language")) {
		case kDutch:
			textLanguage = "DUTCH";
			break;
		case kFrench:
			textLanguage = "FRENCH";
			break;
		case kGerman:
			textLanguage = "GERMAN";
			break;
		case kItalian:
			textLanguage = "ITALIAN";
			break;
		case kSpanish:
			textLanguage = "SPANISH";
			break;
		case kEnglish:
		default:
			textLanguage = "ENGLISH";
			break;
		}
	} else if (getGameLanguage() == Common::HE_ISR) {
		textLanguage = "ENGLISH"; // The Hebrew version does not have a "HEBREW.m3t" file
	} else {
		if (getGameLocalizationType() == kLocMonolingual || ConfMan.getInt("text_language")) {
			textLanguage = menuLanguage;
		} else {
			textLanguage = "ENGLISH";
		}
	}

	if (getGameLocalizationType() != kLocMonolingual && getPlatform() != Common::kPlatformXbox && textLanguage == "ENGLISH") {
		textLanguage = "ENGLISHjp";
	}

	if (getPlatform() == Common::kPlatformXbox) {
		menuLanguage += "X";
		textLanguage += "X";
	}

	// Load all the override files in the search path
	Common::ArchiveMemberList overrides;
	SearchMan.listMatchingMembers(overrides, "*.m3o");
	for (Common::ArchiveMemberList::const_iterator it = overrides.begin(); it != overrides.end(); it++) {
		addArchive(it->get()->getName(), false);
	}

	addArchive(textLanguage + ".m3t", true);

	if (getGameLocalizationType() != kLocMonolingual || getPlatform() == Common::kPlatformXbox || getGameLanguage() == Common::HE_ISR) {
		addArchive(menuLanguage + ".m3u", true);
	}

	addArchive("RSRC.m3r", true);
}

bool Myst3Engine::isTextLanguageEnglish() const {
	if (getGameLocalizationType() == kLocMonolingual && getGameLanguage() == Common::EN_ANY) {
		return true;
	}

	return getGameLocalizationType() != kLocMonolingual && ConfMan.getInt("text_language") == kEnglish;
}

void Myst3Engine::closeArchives() {
	for (uint i = 0; i < _archivesCommon.size(); i++)
		delete _archivesCommon[i];

	_archivesCommon.clear();
}

bool Myst3Engine::checkDatafiles() {
	if (!SearchMan.hasFile("OVER101.m3o")) {
		const char* urlForPatchesDownload = "https://www.scummvm.org/frs/extras/patches/";
		warning("Unable to open the update game archive 'OVER101.m3o'");
		Common::U32String updateMessage =
				Common::U32String::format(_("This version of Myst III has not been updated with the latest official patch.\n"
						  "Please install the official update corresponding to your game's language.\n"
						  "The updates can be downloaded from:\n"
						  "%s"), urlForPatchesDownload);
		warning("%s", updateMessage.encode().c_str());
		GUIErrorMessageWithURL(updateMessage, urlForPatchesDownload);
		return false;
	}

	return true;
}

HotSpot *Myst3Engine::getHoveredHotspot(NodePtr nodeData, uint16 var) {
	_state->setHotspotHovered(false);
	_state->setHotspotActiveRect(0);

	if (_state->getViewType() == kCube) {
		float pitch, heading;
		_cursor->getDirection(pitch, heading);

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			int32 hitRect = nodeData->hotspots[j].isPointInRectsCube(pitch, heading);
			if (hitRect >= 0 && nodeData->hotspots[j].isEnabled(_state, var)) {
				if (nodeData->hotspots[j].rects.size() > 1) {
					_state->setHotspotHovered(true);
					_state->setHotspotActiveRect(hitRect);
				}
				return &nodeData->hotspots[j];
			}
		}
	} else {
		// get the mouse position in original game window coordinates
		Common::Point mouse = _cursor->getPosition(false);
		mouse = _scene->scalePoint(mouse);

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			int32 hitRect = nodeData->hotspots[j].isPointInRectsFrame(_state, mouse);
			if (hitRect >= 0 && nodeData->hotspots[j].isEnabled(_state, var)) {
				if (nodeData->hotspots[j].rects.size() > 1) {
					_state->setHotspotHovered(true);
					_state->setHotspotActiveRect(hitRect);
				}
				return &nodeData->hotspots[j];
			}
		}
	}

	return nullptr;
}

void Myst3Engine::updateCursor() {
	if (!_inventory->isMouseInside()) {
		_inventoryManualHide = false;
	}

	if (isInventoryVisible() && _inventory->isMouseInside()) {
		_inventory->updateCursor();
		return;
	}

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom(), _state->getLocationAge());

	_state->setHotspotIgnoreClick(true);
	HotSpot *hovered = getHoveredHotspot(nodeData);
	_state->setHotspotIgnoreClick(false);

	if (hovered) {
		_cursor->changeCursor(hovered->cursor);
	} else {
		_cursor->changeCursor(8);
	}
}

void Myst3Engine::processInput(bool interactive) {
	_interactive = interactive;

	if (_state->hasVarGamePadUpPressed()) {
		// Reset the gamepad directions once they had a chance to be read by the scripts
		// This combined with keyboard repeat ensures the menu does not scroll too fast
		_state->setGamePadUpPressed(false);
		_state->setGamePadDownPressed(false);
		_state->setGamePadLeftPressed(false);
		_state->setGamePadRightPressed(false);
	}

	bool shouldInteractWithHoveredElement = false;

	// Process events
	Common::Event event;
	while (getEventManager()->pollEvent(event)) {
		if (_state->hasVarGamePadUpPressed()) {
			processEventForGamepad(event);
		}

		processEventForKeyboardState(event);

		if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_state->getViewType() == kCube
					&& _cursor->isPositionLocked()) {
				_scene->updateCamera(event.relMouse);
			}

			_cursor->updatePosition(event.mouse);

		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			shouldInteractWithHoveredElement = true;
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			// Skip the event when in non-interactive mode
			if (!interactive)
				continue;
			// Nothing to do if not in cube view
			if (_state->getViewType() != kCube)
				continue;
			// Don't unlock if the cursor is transparent
			if (!_state->getCursorTransparency())
				continue;

			bool cursorLocked = _cursor->isPositionLocked();
			_cursor->lockPosition(!cursorLocked);

		} else if (event.type == Common::EVENT_KEYDOWN) {
			// Save file name input
			if (_menu->handleInput(event.kbd)) {
				continue;
			}

			if (event.kbdRepeat) {
				// Ignore keyboard repeat except when entering save names
				continue;
			}

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_inputEscapePressedNotConsumed = true;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				shouldInteractWithHoveredElement = true;
				break;
			case Common::KEYCODE_F5:
				// Open main menu
				if (_cursor->isVisible() && interactive) {
					if (_state->getLocationRoom() != kRoomMenu)
						_menu->goToNode(kNodeMenuMain);
				}
				break;
			case Common::KEYCODE_i:
				if (event.kbd.flags & Common::KBD_CTRL) {
					bool mouseInverted = ConfMan.getBool("mouse_inverted");
					mouseInverted = !mouseInverted;
					ConfMan.setBool("mouse_inverted", mouseInverted);
				}
				break;
			default:
				break;
			}
		} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
			_gfx->computeScreenViewport();
			_cursor->updatePosition(_eventMan->getMousePos());
			_inventory->reflow();
		}
	}

	// The input state variables need to be set before calling the scripts
	updateInputState();

	if (shouldInteractWithHoveredElement && interactive) {
		interactWithHoveredElement();
	}

	// Open main menu
	// This is not checked directly in the event handling code
	// because menu open requests done while in lookOnly mode
	// need to be honored after leaving the inner script loop,
	// especially when the script loop was cancelled due to pressing
	// escape.
	if (_inputEscapePressedNotConsumed && interactive) {
		_inputEscapePressedNotConsumed = false;
		if (_cursor->isVisible() && _state->hasVarMenuEscapePressed()) {
			if (_state->getLocationRoom() != kRoomMenu)
				_menu->goToNode(kNodeMenuMain);
			else
				_state->setMenuEscapePressed(1);
		}
	}
}

void Myst3Engine::processEventForKeyboardState(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		if (event.kbdRepeat) {
			// Ignore keyboard repeat except when entering save names
			return;
		}

		switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_inputEscapePressed = true;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (!event.kbd.hasFlags(Common::KBD_ALT)) {
					_inputEnterPressed = true;
				}
				break;
			case Common::KEYCODE_SPACE:
				_inputSpacePressed = true;
				break;
			case Common::KEYCODE_BACKQUOTE: // tilde, used to trigger the easter eggs
				_inputTildePressed = true;
				break;
			default:
				break;
		}
	} else if (event.type == Common::EVENT_KEYUP) {
		switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_inputEscapePressed = false;
				_inputEscapePressedNotConsumed = false;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				_inputEnterPressed = false;
				break;
			case Common::KEYCODE_SPACE:
				_inputSpacePressed = false;
				break;
			case Common::KEYCODE_BACKQUOTE:
				_inputTildePressed = false;
				break;
			default:
				break;
		}
	}
}

void Myst3Engine::processEventForGamepad(const Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		_state->setGamePadActionPressed(true);
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		_state->setGamePadActionPressed(false);
	} else if (event.type == Common::EVENT_KEYDOWN) {
		if (event.kbdRepeat) return;

		switch (event.kbd.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			_state->setGamePadActionPressed(true);
			break;
		case Common::KEYCODE_UP:
			_state->setGamePadUpPressed(true);
			break;
		case Common::KEYCODE_DOWN:
			_state->setGamePadDownPressed(true);
			break;
		case Common::KEYCODE_LEFT:
			_state->setGamePadLeftPressed(true);
			break;
		case Common::KEYCODE_RIGHT:
			_state->setGamePadRightPressed(true);
			break;
		case Common::KEYCODE_ESCAPE:
			_state->setGamePadCancelPressed(true);
			break;
		default:
			break;
		}
	} else if (event.type == Common::EVENT_KEYUP) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			_state->setGamePadActionPressed(false);
			break;
		case Common::KEYCODE_ESCAPE:
			_state->setGamePadCancelPressed(false);
			break;
		default:
			break;
		}
	}
}

void Myst3Engine::updateInputState() {
	_state->setInputMousePressed(inputValidatePressed());
	_state->setInputTildePressed(_inputTildePressed);
	_state->setInputSpacePressed(_inputSpacePressed);
	_state->setInputEscapePressed(_inputEscapePressed);
}

void Myst3Engine::interactWithHoveredElement() {
	if (isInventoryVisible() && _inventory->isMouseInside()) {
		uint16 hoveredInventory = _inventory->hoveredItem();
		if (hoveredInventory > 0) {
			_inventory->useItem(hoveredInventory);
		} else {
			if (isWideScreenModEnabled()) {
				_inventoryManualHide = true;
			}
		}
		return;
	}

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom(), _state->getLocationAge());
	HotSpot *hovered = getHoveredHotspot(nodeData);

	if (hovered) {
		_scriptEngine->run(&hovered->script);
		return;
	}

	// Bad click
	_sound->playEffect(697, 5);
}

void Myst3Engine::drawFrame(bool noSwap) {
	_sound->update();
	_gfx->clear();

	if (_state->getViewType() == kCube) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		float fov = _state->getLookAtFOV();

		// Apply the rotation effect
		if (_rotationEffect) {
			_rotationEffect->update();

			heading += _rotationEffect->getHeadingOffset();
			_state->lookAt(pitch, heading);
		}

		// Apply the shake effect
		if (_shakeEffect) {
			_shakeEffect->update();
			pitch += _shakeEffect->getPitchOffset();
			heading += _shakeEffect->getHeadingOffset();
		}

		_gfx->setupCameraPerspective(pitch, heading, fov);
	}

	if (_node) {
		_node->update();
		_gfx->renderDrawable(_node, _scene);
	}

	for (int i = _movies.size() - 1; i >= 0 ; i--) {
		_movies[i]->update();
		_gfx->renderDrawable(_movies[i], _scene);
	}

	if (_state->getViewType() == kMenu) {
		_gfx->renderDrawable(_menu, _scene);
	}

	for (uint i = 0; i < _drawables.size(); i++) {
		_gfx->renderDrawable(_drawables[i], _scene);
	}

	if (_state->getViewType() != kMenu) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		SunSpot flare = computeSunspotsIntensity(pitch, heading);
		if (flare.intensity >= 0)
			_scene->drawSunspotFlare(flare);
	}

	if (isInventoryVisible()) {
		_gfx->renderWindow(_inventory);
	}

	// Draw overlay 2D movies
	for (int i = _movies.size() - 1; i >= 0 ; i--) {
		_gfx->renderDrawableOverlay(_movies[i], _scene);
	}

	for (uint i = 0; i < _drawables.size(); i++) {
		_gfx->renderDrawableOverlay(_drawables[i], _scene);
	}

	// Draw spot subtitles
	if (_node) {
		_gfx->renderDrawableOverlay(_node, _scene);
	}

	bool cursorVisible = _cursor->isVisible();

	if (getPlatform() == Common::kPlatformXbox) {
		// The cursor is not drawn in the Xbox version menus and journals
		cursorVisible &= !(_state->getLocationRoom() == kRoomMenu || _state->getLocationRoom() == kRoomJournals);
	}

	if (cursorVisible)
		_gfx->renderDrawable(_cursor, _scene);

	_gfx->flipBuffer();

	if (!noSwap) {
		_frameLimiter->delayBeforeSwap();
		_system->updateScreen();
		_state->updateFrameCounters();
		_frameLimiter->startFrame();
	}
}

bool Myst3Engine::isInventoryVisible() {
	if (_state->getViewType() == kMenu)
		return false;

	if (_node && _node->hasSubtitlesToDraw())
		return false;

	if (_inventoryManualHide) {
		return false;
	}

	// Only draw the inventory when the mouse is inside its area
	if (isWideScreenModEnabled() && !_inventory->isMouseInside()) {
		return false;
	}

	return true;
}

void Myst3Engine::setupTransition() {
	delete _transition;
	_transition = new Transition(this);
}

void Myst3Engine::drawTransition(TransitionType transitionType) {
	if (_transition) {
		_interactive = false; // Don't allow loading while drawing transitions
		_transition->draw(transitionType);
		delete _transition;
		_transition = nullptr;
	}
}

void Myst3Engine::goToNode(uint16 nodeID, TransitionType transitionType) {
	uint16 node = _state->getLocationNextNode();
	if (!node)
		node = nodeID;

	uint16 room = _state->getLocationNextRoom();
	uint16 age = _state->getLocationNextAge();

	setupTransition();

	ViewType sourceViewType = _state->getViewType();
	if (sourceViewType == kCube) {
		// The lookat direction in the next node should be
		// the direction of the mouse cursor
		float pitch, heading;
		_cursor->getDirection(pitch, heading);
		_state->lookAt(pitch, heading);
	}

	loadNode(node, room, age);

	_state->setLocationNextNode(0);
	_state->setLocationNextRoom(0);
	_state->setLocationNextAge(0);

	if (_state->getAmbiantPreviousFadeOutDelay() > 0) {
		_ambient->playCurrentNode(100, _state->getAmbiantPreviousFadeOutDelay());
	}

	drawTransition(transitionType);
}

void Myst3Engine::loadNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	unloadNode();

	_scriptEngine->run(&_db->getNodeInitScript());

	if (nodeID)
		_state->setLocationNode(_state->valueOrVarValue(nodeID));

	if (roomID)
		_state->setLocationRoom(_state->valueOrVarValue(roomID));
	else
		roomID = _state->getLocationRoom();

	if (ageID)
		_state->setLocationAge(_state->valueOrVarValue(ageID));
	else
		ageID = _state->getLocationAge();

	_db->cacheRoom(roomID, ageID);

	Common::String newRoomName = _db->getRoomName(roomID, ageID);
	if ((!_archiveNode || _archiveNode->getRoomName() != newRoomName) && !_db->isCommonRoom(roomID, ageID)) {

		Common::String nodeFile = Common::String::format("%snodes.m3a", newRoomName.c_str());

		_archiveNode->close();
		if (!_archiveNode->open(nodeFile.c_str(), newRoomName.c_str())) {
			error("Unable to open archive %s", nodeFile.c_str());
		}
	}

	runNodeInitScripts();
	if (!_node) {
		return; // The main init script does not load a node
	}

	// The effects can only be created after running the node init scripts
	_node->initEffects();
	_shakeEffect = ShakeEffect::create(this);
	_rotationEffect = RotationEffect::create(this);

	// WORKAROUND: In Narayan, the scripts in node NACH 9 test on var 39
	// without first reinitializing it leading to Saavedro not always giving
	// Releeshan to the player when he is trapped between both shields.
	if (nodeID == 9 && roomID == kRoomNarayan)
		_state->setVar(39, 0);
}

void Myst3Engine::unloadNode() {
	if (!_node)
		return;

	// Delete all movies
	removeMovie(0);

	// Remove all sunspots
	for (uint i = 0; i < _sunspots.size(); i++)
		delete _sunspots[i];

	_sunspots.clear();

	// Clean up the effects
	delete _shakeEffect;
	_shakeEffect = nullptr;
	_state->setShakeEffectAmpl(0);
	delete _rotationEffect;
	_rotationEffect = nullptr;

	delete _node;
	_node = nullptr;
}

void Myst3Engine::runNodeInitScripts() {
	NodePtr nodeData = _db->getNodeData(
			_state->getLocationNode(),
			_state->getLocationRoom(),
			_state->getLocationAge());

	NodePtr nodeDataInit = _db->getNodeData(
			32765,
			_state->getLocationRoom(),
			_state->getLocationAge());

	if (nodeDataInit)
		runScriptsFromNode(32765);

	if (!nodeData)
		error("Node %d unknown in the database", _state->getLocationNode());

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_state->evaluate(nodeData->scripts[j].condition)) {
			_scriptEngine->run(&nodeData->scripts[j].script);
		}
	}

	// Mark the node as a reachable zip destination
	_state->markNodeAsVisited(
			_state->getLocationNode(),
			_state->getLocationRoom(),
			_state->getLocationAge());
}

void Myst3Engine::runNodeBackgroundScripts() {
	NodePtr nodeDataRoom = _db->getNodeData(32765, _state->getLocationRoom(), _state->getLocationAge());

	if (nodeDataRoom) {
		for (uint j = 0; j < nodeDataRoom->hotspots.size(); j++) {
			if (nodeDataRoom->hotspots[j].condition == -1) {
				if (!_scriptEngine->run(&nodeDataRoom->hotspots[j].script))
					break;
			}
		}
	}

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom(), _state->getLocationAge());

	for (uint j = 0; j < nodeData->hotspots.size(); j++) {
		if (nodeData->hotspots[j].condition == -1) {
			if (!_scriptEngine->run(&nodeData->hotspots[j].script))
				break;
		}
	}
}

void Myst3Engine::loadNodeCubeFaces(uint16 nodeID) {
	_state->setViewType(kCube);

	_cursor->lockPosition(true);
	updateCursor();

	_node = new NodeCube(this, nodeID);
}

void Myst3Engine::loadNodeFrame(uint16 nodeID) {
	_state->setViewType(kFrame);

	_cursor->lockPosition(false);
	updateCursor();

	_node = new NodeFrame(this, nodeID);
}

void Myst3Engine::loadNodeMenu(uint16 nodeID) {
	_state->setViewType(kMenu);

	_cursor->lockPosition(false);
	updateCursor();

	_node = new NodeFrame(this, nodeID);
}

void Myst3Engine::runScriptsFromNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (roomID == 0)
		roomID = _state->getLocationRoom();

	if (ageID == 0)
		ageID = _state->getLocationAge();

	NodePtr nodeData = _db->getNodeData(nodeID, roomID, ageID);

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_state->evaluate(nodeData->scripts[j].condition)) {
			if (!_scriptEngine->run(&nodeData->scripts[j].script))
				break;
		}
	}
}

void Myst3Engine::runBackgroundSoundScriptsFromNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (_state->getSoundScriptsSuspended())
		return;

	if (roomID == 0)
		roomID = _state->getLocationRoom();

	if (ageID == 0)
		ageID = _state->getLocationAge();

	NodePtr nodeData = _db->getNodeData(nodeID, roomID, ageID);

	if (!nodeData) return;

	if (_backgroundSoundScriptLastRoomId != roomID || _backgroundSoundScriptLastAgeId != ageID) {
		bool sameScript;
		if (   _backgroundSoundScriptLastRoomId != 0 && roomID != 0
		    && _backgroundSoundScriptLastAgeId  != 0 && ageID  != 0) {
			sameScript = _db->areRoomsScriptsEqual(_backgroundSoundScriptLastRoomId, _backgroundSoundScriptLastAgeId,
			                                       roomID, ageID, kScriptTypeBackgroundSound);
		} else {
			sameScript = false;
		}

		// Stop previous music when the music script changes
		if (!sameScript
		    && _backgroundSoundScriptLastRoomId != kRoomMenu
		    && _backgroundSoundScriptLastRoomId != kRoomJournals
		    && roomID != kRoomMenu
		    && roomID != kRoomJournals) {

			_sound->stopMusic(_state->getSoundScriptFadeOutDelay());

			if (!nodeData->backgroundSoundScripts.empty()) {
				_state->setSoundScriptsPaused(1);
				_state->setSoundScriptsTimer(0);
			}
		}

		_backgroundSoundScriptLastRoomId = roomID;
		_backgroundSoundScriptLastAgeId  = ageID;
	}

	for (uint j = 0; j < nodeData->backgroundSoundScripts.size(); j++) {
		if (_state->evaluate(nodeData->backgroundSoundScripts[j].condition)) {
			if (!_scriptEngine->run(&nodeData->backgroundSoundScripts[j].script))
				break;
		}
	}
}

void Myst3Engine::runAmbientScripts(uint32 node) {
	uint32 room = _ambient->_scriptRoom;
	uint32 age = _ambient->_scriptAge;

	if (room == 0)
		room = _state->getLocationRoom();

	if (age == 0)
		age = _state->getLocationAge();

	NodePtr nodeData = _db->getNodeData(node, room, age);

	if (!nodeData) return;

	for (uint j = 0; j < nodeData->soundScripts.size(); j++)
		if (_state->evaluate(nodeData->soundScripts[j].condition))
			_scriptEngine->run(&nodeData->soundScripts[j].script);
}

void Myst3Engine::loadMovie(uint16 id, uint16 condition, bool resetCond, bool loop) {
	ScriptedMovie *movie;

	if (!_state->getMovieUseBackground()) {
		movie = new ScriptedMovie(this, id);
	} else {
		movie = new ProjectorMovie(this, id, _projectorBackground);
		_projectorBackground = nullptr;
		_state->setMovieUseBackground(0);
	}

	movie->setCondition(condition);
	movie->setDisableWhenComplete(resetCond);
	movie->setLoop(loop);

	if (_state->getMovieScriptDriven()) {
		movie->setScriptDriven(_state->getMovieScriptDriven());
		_state->setMovieScriptDriven(0);
	}

	if (_state->getMovieStartFrameVar()) {
		movie->setStartFrameVar(_state->getMovieStartFrameVar());
		_state->setMovieStartFrameVar(0);
	}

	if (_state->getMovieEndFrameVar()) {
		movie->setEndFrameVar(_state->getMovieEndFrameVar());
		_state->setMovieEndFrameVar(0);
	}

	if (_state->getMovieStartFrame()) {
		movie->setStartFrame(_state->getMovieStartFrame());
		_state->setMovieStartFrame(0);
	}

	if (_state->getMovieEndFrame()) {
		movie->setEndFrame(_state->getMovieEndFrame());
		_state->setMovieEndFrame(0);
	}

	if (_state->getMovieNextFrameGetVar()) {
		movie->setNextFrameReadVar(_state->getMovieNextFrameGetVar());
		_state->setMovieNextFrameGetVar(0);
	}

	if (_state->getMovieNextFrameSetVar()) {
		movie->setNextFrameWriteVar(_state->getMovieNextFrameSetVar());
		_state->setMovieNextFrameSetVar(0);
	}

	if (_state->getMoviePlayingVar()) {
		movie->setPlayingVar(_state->getMoviePlayingVar());
		_state->setMoviePlayingVar(0);
	}

	if (_state->getMovieOverridePosition()) {
		movie->setPosU(_state->getMovieOverridePosU());
		movie->setPosV(_state->getMovieOverridePosV());
		_state->setMovieOverridePosition(0);
	}

	if (_state->getMovieUVar()) {
		movie->setPosUVar(_state->getMovieUVar());
		_state->setMovieUVar(0);
	}

	if (_state->getMovieVVar()) {
		movie->setPosVVar(_state->getMovieVVar());
		_state->setMovieVVar(0);
	}

	if (_state->getMovieOverrideCondition()) {
		movie->setCondition(_state->getMovieOverrideCondition());
		_state->setMovieOverrideCondition(0);
	}

	if (_state->getMovieConditionBit()) {
		movie->setConditionBit(_state->getMovieConditionBit());
		_state->setMovieConditionBit(0);
	}

	if (_state->getMovieForce2d()) {
		movie->setForce2d(_state->getMovieForce2d());
		_state->setMovieForce2d(0);
	}

	if (_state->getMovieVolume1()) {
		movie->setVolume(_state->getMovieVolume1());
		_state->setMovieVolume1(0);
	} else {
		movie->setVolume(_state->getMovieVolume2());
	}

	if (_state->getMovieVolumeVar()) {
		movie->setVolumeVar(_state->getMovieVolumeVar());
		_state->setMovieVolumeVar(0);
	}

	if (_state->getMovieSoundHeading()) {
		movie->setSoundHeading(_state->getMovieSoundHeading());
		_state->setMovieSoundHeading(0);
	}

	if (_state->getMoviePanningStrenght()) {
		movie->setSoundAttenuation(_state->getMoviePanningStrenght());
		_state->setMoviePanningStrenght(0);
	}

	if (_state->getMovieAdditiveBlending()) {
		movie->setAdditiveBlending(true);
		_state->setMovieAdditiveBlending(0);
	}

	if (_state->getMovieTransparency()) {
		movie->setTransparency(_state->getMovieTransparency());
		_state->setMovieTransparency(0);
	} else {
		movie->setTransparency(100);
	}

	if (_state->getMovieTransparencyVar()) {
		movie->setTransparencyVar(_state->getMovieTransparencyVar());
		_state->setMovieTransparencyVar(0);
	}

	_movies.push_back(movie);
}

void Myst3Engine::playSimpleMovie(uint16 id, bool fullframe, bool refreshAmbientSounds) {
	SimpleMovie movie(this, id);

	if (!movie.isVideoLoaded()) {
		// The video was not loaded and it was optional, just do nothing
		return;
	}

	if (_state->getMovieSynchronized()) {
		movie.setSynchronized(_state->getMovieSynchronized());
		_state->setMovieSynchronized(0);
	}

	if (_state->getMovieStartFrame()) {
		movie.setStartFrame(_state->getMovieStartFrame());
		_state->setMovieStartFrame(0);
	}

	if (_state->getMovieEndFrame()) {
		movie.setEndFrame(_state->getMovieEndFrame());
		_state->setMovieEndFrame(0);
	}

	if (_state->getMovieVolume1()) {
		movie.setVolume(_state->getMovieVolume1());
		_state->setMovieVolume1(0);
	} else {
		movie.setVolume(_state->getMovieVolume2());
	}

	if (fullframe) {
		movie.setForce2d(_state->getViewType() == kCube);
		movie.setForceOpaque(true);
		movie.setPosU(0);
		movie.setPosV(_state->getViewType() == kMenu ? Renderer::kTopBorderHeight : 0);
	}

	movie.play();

	if (refreshAmbientSounds) {
		movie.refreshAmbientSounds();
	}

	_drawables.push_back(&movie);

	while (!shouldQuit() && !movie.endOfVideo()) {
		movie.update();

		// Process events
		processInput(false);

		// Handle skipping
		if (_inputSpacePressed || _inputEscapePressed) {
			// Consume the escape key press so the menu does not open
			_inputEscapePressedNotConsumed = false;
			break;
		}

		drawFrame();
	}

	_drawables.pop_back();

	// Reset the movie script so that the next movie will not try to run them
	// when the user has skipped this one before the script is triggered.
	_state->setMovieScriptStartFrame(0);
	_state->setMovieScript(0);
	_state->setMovieAmbiantScriptStartFrame(0);
	_state->setMovieAmbiantScript(0);
}

void Myst3Engine::removeMovie(uint16 id) {
	if (id == 0) {
		for (uint i = 0; i < _movies.size(); i++)
			delete _movies[i];

		_movies.clear();
		return;
	} else {
		for (uint i = 0; i < _movies.size(); i++)
			if (_movies[i]->getId() == id) {
				delete _movies[i];
				_movies.remove_at(i);
				break;
			}
	}
}

void Myst3Engine::setMovieLooping(uint16 id, bool loop) {
	for (uint i = 0; i < _movies.size(); i++) {
		if (_movies[i]->getId() == id) {
			// Enable or disable looping
			_movies[i]->setLoop(loop);
			_movies[i]->setDisableWhenComplete(!loop);
			break;
		}
	}
}

void Myst3Engine::addSpotItem(uint16 id, int16 condition, bool fade) {
	assert(_node);

	_node->loadSpotItem(id, condition, fade);
}

SpotItemFace *Myst3Engine::addMenuSpotItem(uint16 id, int16 condition, const Common::Rect &rect) {
	assert(_node);

	SpotItemFace *face = _node->loadMenuSpotItem(condition, rect);

	_menu->setSaveLoadSpotItem(id, face);

	return face;
}

void Myst3Engine::loadNodeSubtitles(uint32 id) {
	assert(_node);

	_node->loadSubtitles(id);
}

ResourceDescription Myst3Engine::getFileDescription(const Common::String &room, uint32 index, uint16 face,
	                                            Archive::ResourceType type) {
	Common::String archiveRoom = room;
	if (archiveRoom == "") {
		archiveRoom = _db->getRoomName(_state->getLocationRoom(), _state->getLocationAge());
	}

	ResourceDescription desc;

	// Search common archives
	uint i = 0;
	while (!desc.isValid() && i < _archivesCommon.size()) {
		desc = _archivesCommon[i]->getDescription(archiveRoom, index, face, type);
		i++;
	}

	// Search currently loaded node archive
	if (!desc.isValid() && _archiveNode)
		desc = _archiveNode->getDescription(archiveRoom, index, face, type);

	return desc;
}

ResourceDescriptionArray Myst3Engine::listFilesMatching(const Common::String &room, uint32 index, uint16 face,
	                                                Archive::ResourceType type) {
	Common::String archiveRoom = room;
	if (archiveRoom == "") {
		archiveRoom = _db->getRoomName(_state->getLocationRoom(), _state->getLocationAge());
	}

	for (uint i = 0; i < _archivesCommon.size(); i++) {
		ResourceDescriptionArray list = _archivesCommon[i]->listFilesMatching(archiveRoom, index, face, type);
		if (!list.empty()) {
			return list;
		}
	}

	return _archiveNode->listFilesMatching(archiveRoom, index, face, type);
}

Graphics::Surface *Myst3Engine::loadTexture(uint16 id) {
	ResourceDescription desc = getFileDescription("GLOB", id, 0, Archive::kRawData);

	if (!desc.isValid())
		error("Texture %d does not exist", id);

	Common::SeekableReadStream *data = desc.getData();

	uint32 magic = data->readUint32LE();
	if (magic != MKTAG('.', 'T', 'E', 'X'))
		error("Wrong texture format %d", id);

	data->readUint32LE(); // unk 1
	uint32 width = data->readUint32LE();
	uint32 height = data->readUint32LE();
	data->readUint32LE(); // unk 2
	data->readUint32LE(); // unk 3

#ifdef SCUMM_BIG_ENDIAN
	Graphics::PixelFormat onDiskFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 24, 16, 8);
#else
	Graphics::PixelFormat onDiskFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);
#endif

	Graphics::Surface *s = new Graphics::Surface();
	s->create(width, height, onDiskFormat);

	data->read(s->getPixels(), height * s->pitch);
	delete data;

	s->convertToInPlace(Texture::getRGBAPixelFormat());

	return s;
}

Graphics::Surface *Myst3Engine::decodeJpeg(const ResourceDescription *jpegDesc) {
	Common::SeekableReadStream *jpegStream = jpegDesc->getData();

	Image::JPEGDecoder jpeg;
	jpeg.setOutputPixelFormat(Texture::getRGBAPixelFormat());

	if (!jpeg.loadStream(*jpegStream))
		error("Could not decode Myst III JPEG");
	delete jpegStream;

	const Graphics::Surface *bitmap = jpeg.getSurface();
	assert(bitmap->format == Texture::getRGBAPixelFormat());

	// JPEGDecoder owns the decoded surface, we have to make a copy...
	Graphics::Surface *rgbaSurface = new Graphics::Surface();
	rgbaSurface->copyFrom(*bitmap);
	return rgbaSurface;
}

int16 Myst3Engine::openDialog(uint16 id) {
	Dialog *dialog;

	if (getPlatform() == Common::kPlatformXbox) {
		dialog = new GamepadDialog(this, id);
	} else {
		dialog = new ButtonsDialog(this, id);
	}

	_drawables.push_back(dialog);

	int16 result = -2;

	while (result == -2 && !shouldQuit()) {
		result = dialog->update();
		drawFrame();
	}

	_drawables.pop_back();

	delete dialog;

	return result;
}

void Myst3Engine::dragSymbol(uint16 var, uint16 id) {
	DragItem drag(this, id);

	_drawables.push_back(&drag);

	_cursor->changeCursor(2);
	_state->setVar(var, -1);

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom(), _state->getLocationAge());

	while (inputValidatePressed() && !shouldQuit()) {
		processInput(false);

		HotSpot *hovered = getHoveredHotspot(nodeData, var);
		drag.setFrame(hovered ? 2 : 1);

		drawFrame();
	}

	_state->setVar(var, 1);
	_drawables.pop_back();

	HotSpot *hovered = getHoveredHotspot(nodeData, var);
	if (hovered) {
		_cursor->setVisible(false);
		_scriptEngine->run(&hovered->script);
		_cursor->setVisible(true);
	}
}

void Myst3Engine::dragItem(uint16 statusVar, uint16 movie, uint16 frame, uint16 hoverFrame, uint16 itemVar) {
	DragItem drag(this, movie);

	_drawables.push_back(&drag);

	_cursor->changeCursor(2);
	_state->setVar(statusVar, 0);
	_state->setVar(itemVar, 1);

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom(), _state->getLocationAge());

	while (inputValidatePressed() && !shouldQuit()) {
		processInput(false);

		HotSpot *hovered = getHoveredHotspot(nodeData, itemVar);
		drag.setFrame(hovered ? hoverFrame : frame);

		drawFrame();
	}

	_drawables.pop_back();

	HotSpot *hovered = getHoveredHotspot(nodeData, itemVar);
	if (hovered) {
		_cursor->setVisible(false);
		_scriptEngine->run(&hovered->script);
		_cursor->setVisible(true);
	} else {
		_state->setVar(statusVar, 1);
		_state->setVar(itemVar, 0);
	}
}

bool Myst3Engine::canSaveGameStateCurrently() {
	bool inMenuWithNoGameLoaded = _state->getLocationRoom() == kRoomMenu && _state->getMenuSavedAge() == 0;
	return canLoadGameStateCurrently() && !inMenuWithNoGameLoaded && _cursor->isVisible();
}

bool Myst3Engine::canLoadGameStateCurrently() {
	// Loading from the GMM is only possible when the game is interactive
	// This is to prevent loading from inner loops. Loading while
	// in an inner loop can cause the exit condition to never happen,
	// or can unload required resources.
	return _interactive;
}

Common::Error Myst3Engine::loadGameState(int slot) {
	Common::StringArray filenames = Saves::list(_saveFileMan, getPlatform());
	return loadGameState(filenames[slot], kTransitionNone);
}

Common::Error Myst3Engine::loadGameState(Common::String fileName, TransitionType transition) {
	Common::SharedPtr<Common::InSaveFile> saveFile = Common::SharedPtr<Common::InSaveFile>(_saveFileMan->openForLoading(fileName));
	if (!saveFile) {
		return Common::kReadingFailed;
	}

	Common::Error loadError = _state->load(saveFile.get());
	if (loadError.getCode() != Common::kNoError) {
		return loadError;
	}

	if (saveFile->eos()) {
		warning("Unexpected end of file reached when reading '%s'", fileName.c_str());
		return Common::kReadingFailed;
	}

	if (saveFile->err()) {
		warning("An error occrured when reading '%s'", fileName.c_str());
		return Common::kReadingFailed;
	}

	_inventory->loadFromState();

	// Leave the load menu
	_state->setViewType(kMenu);
	_state->setLocationNextAge(_state->getMenuSavedAge());
	_state->setLocationNextRoom(_state->getMenuSavedRoom());
	_state->setLocationNextNode(_state->getMenuSavedNode());
	_state->setMenuSavedAge(0);
	_state->setMenuSavedRoom(0);
	_state->setMenuSavedNode(0);
	_sound->resetSoundVars();
	_sound->stopMusic(15);
	_state->setSoundScriptsSuspended(0);
	_sound->playEffect(696, 60);

	goToNode(0, transition);

	return Common::kNoError;
}

static bool isValidSaveFileChar(char c) {
	// Limit it to letters, digits, and a few other characters that should be safe
	return Common::isAlnum(c) || c == ' ' || c == '_' || c == '+' || c == '-' || c == '.';
}

static bool isValidSaveFileName(const Common::String &desc) {
	for (uint32 i = 0; i < desc.size(); i++)
		if (!isValidSaveFileChar(desc[i]))
			return false;

	return true;
}

Common::Error Myst3Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	assert(!desc.empty());

	if (!isValidSaveFileName(desc)) {
		return Common::Error(Common::kCreatingFileFailed, _("Invalid file name for saving"));
	}

	// Try to use an already generated thumbnail
	const Graphics::Surface *thumbnail = _menu->borrowSaveThumbnail();
	if (!thumbnail) {
		_menu->generateSaveThumbnail();
	}
	thumbnail = _menu->borrowSaveThumbnail();
	assert(thumbnail);

	return saveGameState(desc, thumbnail, isAutosave);
}

Common::Error Myst3Engine::saveGameState(const Common::String &desc, const Graphics::Surface *thumbnail, bool isAutosave) {
	// Strip extension
	Common::String saveName = desc;
	if (desc.hasSuffixIgnoreCase(".M3S") || desc.hasSuffixIgnoreCase(".M3X")) {
		saveName.erase(desc.size() - 4, desc.size());
	}

	Common::String fileName = Saves::buildName(saveName.c_str(), getPlatform());

	// Save the state and the thumbnail
	Common::SharedPtr<Common::OutSaveFile> save = Common::SharedPtr<Common::OutSaveFile>(_saveFileMan->openForSaving(fileName));
	if (!save) {
		return Common::kCreatingFileFailed;
	}

	Common::Error saveError = _state->save(save.get(), saveName, thumbnail, isAutosave);
	if (saveError.getCode() != Common::kNoError) {
		return saveError;
	}

	if (save->err()) {
		warning("An error occurred when writing '%s'", fileName.c_str());
		return Common::kWritingFailed;
	}

	return saveError;
}

void Myst3Engine::animateDirectionChange(float targetPitch, float targetHeading, uint16 scriptTicks) {
	float startPitch = _state->getLookAtPitch();
	float startHeading = _state->getLookAtHeading();

	if (startPitch == targetPitch && startHeading == targetHeading)
		return; // Fast path

	float pitchDistance = targetPitch - startPitch;
	float headingDistance = targetHeading - startHeading;

	// Make sure to use the shortest direction
	while (ABS(headingDistance) > 180) {
		if (headingDistance > 0) {
			headingDistance -= 360;
		} else {
			headingDistance += 360;
		}
	}

	// Compute animation duration in frames
	float numTicks;
	if (scriptTicks) {
		numTicks = scriptTicks;
	} else {
		numTicks = sqrt(pitchDistance * pitchDistance + headingDistance * headingDistance)
				* 30.0f / _state->getCameraMoveSpeed();

		if (numTicks > 0.0f)
			numTicks += 10.0f;
	}

	uint startTick = _state->getTickCount();

	// Draw animation
	if (numTicks != 0.0f) {
		while (1) {
			uint elapsedTicks = _state->getTickCount() - startTick;
			if (elapsedTicks >= numTicks || shouldQuit())
				break;

			float step;
			if (numTicks >= 15) {
				// Fast then slow movement
				if (elapsedTicks > numTicks / 2.0f)
					step = 1.0f - (numTicks - elapsedTicks) * (numTicks - elapsedTicks) / (numTicks / 2.0f * numTicks / 2.0f) / 2.0f;
				else
					step = elapsedTicks * elapsedTicks / (numTicks / 2.0f * numTicks / 2.0f) / 2.0f;

			} else {
				// Constant speed movement
				step = elapsedTicks / numTicks;
			}

			float nextPitch = startPitch + pitchDistance * step;
			float nextHeading = startHeading + headingDistance * step;

			_state->lookAt(nextPitch, nextHeading);
			drawFrame();
		}
	}

	_state->lookAt(targetPitch, targetHeading);
	drawFrame();
}

void Myst3Engine::getMovieLookAt(uint16 id, bool start, float &pitch, float &heading) {
	ResourceDescription desc = getFileDescription("", id, 0, Archive::kMovie);

	if (!desc.isValid())
		desc = getFileDescription("", id, 0, Archive::kMultitrackMovie);

	if (!desc.isValid())
		error("Movie %d does not exist", id);

	Math::Vector3d v;
	if (start)
		v = desc.getVideoData().v1;
	else
		v = desc.getVideoData().v2;

	Math::Vector2d horizontalProjection(v.x(), v.z());
	horizontalProjection.normalize();

	pitch = 90 - Math::Angle::arcCosine(v.y()).getDegrees();
	heading = Math::Angle::arcCosine(horizontalProjection.getY()).getDegrees();

	if (horizontalProjection.getX() < 0.0) {
		heading = 360 - heading;
	}
}

void Myst3Engine::playMovieGoToNode(uint16 movie, uint16 node) {
	uint16 room = _state->getLocationNextRoom();
	uint16 age = _state->getLocationNextAge();

	if (_state->getLocationNextNode()) {
		node = _state->getLocationNextNode();
	}

	if (_state->getViewType() == kCube && !_state->getCameraSkipAnimation()) {
		float startPitch, startHeading;
		getMovieLookAt(movie, true, startPitch, startHeading);
		animateDirectionChange(startPitch, startHeading, 0);
	}
	_state->setCameraSkipAnimation(0);

	loadNode(node, room, age);

	playSimpleMovie(movie, true, true);

	_state->setLocationNextNode(0);
	_state->setLocationNextRoom(0);
	_state->setLocationNextAge(0);

	if (_state->getViewType() == kCube) {
		float endPitch, endHeading;
		getMovieLookAt(movie, false, endPitch, endHeading);
		_state->lookAt(endPitch, endHeading);
	}

	setupTransition();
}

void Myst3Engine::playMovieFullFrame(uint16 movie) {
	if (_state->getViewType() == kCube && !_state->getCameraSkipAnimation()) {
		float startPitch, startHeading;
		getMovieLookAt(movie, true, startPitch, startHeading);
		animateDirectionChange(startPitch, startHeading, 0);
	}
	_state->setCameraSkipAnimation(0);

	playSimpleMovie(movie, true, false);

	if (_state->getViewType() == kCube) {
		float endPitch, endHeading;
		getMovieLookAt(movie, false, endPitch, endHeading);
		_state->lookAt(endPitch, endHeading);
	}

	setupTransition();
}

bool Myst3Engine::inputValidatePressed() {
	return _inputEnterPressed ||
	       _inputSpacePressed ||
	       getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
}

bool Myst3Engine::inputEscapePressed() {
	return _inputEscapePressed;
}

bool Myst3Engine::inputSpacePressed() {
	return _inputSpacePressed;
}

bool Myst3Engine::inputTilePressed() {
	return _inputTildePressed;
}

void Myst3Engine::addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
	                     uint16 color, uint16 var, bool varControlledIntensity, uint16 radius) {

	SunSpot *s = new SunSpot();

	s->pitch = pitch;
	s->heading = heading;
	s->intensity = intensity * 2.55;
	s->color = (color & 0xF) | 16
	           * ((color & 0xF) | 16
	           * (((color >> 4) & 0xF) | 16
	           * (((color >> 4) & 0xF) | 16
	           * (((color >> 8) & 0xF) | 16
	           * (((color >> 8) & 0xF))))));
	s->var = var;
	s->variableIntensity = varControlledIntensity;
	s->radius = radius;

	_sunspots.push_back(s);
}

SunSpot Myst3Engine::computeSunspotsIntensity(float pitch, float heading) {
	SunSpot result;
	result.intensity = -1;
	result.color = 0;
	result.radius = 0;

	for (uint i = 0; i < _sunspots.size(); i++) {
		SunSpot *s = _sunspots[i];

		uint32 value = _state->getVar(s->var);

		// Skip disabled items
		if (value == 0) continue;

		float distance = _scene->distanceToZone(s->heading, s->pitch, s->radius, heading, pitch);

		if (distance > result.radius) {
			result.radius = distance;
			result.color = s->color;
			result.intensity = s->intensity;
			result.variableIntensity = s->variableIntensity;

			if (result.variableIntensity) {
				result.radius = value * distance / 100;
			}
		}
	}

	return result;
}

void Myst3Engine::settingsInitDefaults() {
	int defaultLanguage = _db->getGameLanguageCode();

	int defaultTextLanguage;
	if (getGameLocalizationType() == kLocMulti6)
		defaultTextLanguage = defaultLanguage;
	else
		defaultTextLanguage = getGameLanguage() != Common::EN_ANY;

	ConfMan.registerDefault("overall_volume", Audio::Mixer::kMaxMixerVolume);
	ConfMan.registerDefault("music_volume", Audio::Mixer::kMaxMixerVolume / 2);
	ConfMan.registerDefault("music_frequency", 75);
	ConfMan.registerDefault("audio_language", defaultLanguage);
	ConfMan.registerDefault("text_language", defaultTextLanguage);
	ConfMan.registerDefault("water_effects", true);
	ConfMan.registerDefault("transition_speed", 50);
	ConfMan.registerDefault("mouse_speed", 50);
	ConfMan.registerDefault("mouse_inverted", false);
	ConfMan.registerDefault("zip_mode", false);
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("vibrations", true); // Xbox specific
}

void Myst3Engine::settingsLoadToVars() {
	_state->setWaterEffects(ConfMan.getBool("water_effects"));
	_state->setTransitionSpeed(ConfMan.getInt("transition_speed"));
	_state->setMouseSpeed(ConfMan.getInt("mouse_speed"));
	_state->setZipModeEnabled(ConfMan.getBool("zip_mode"));
	_state->setSubtitlesEnabled(ConfMan.getBool("subtitles"));

	if (getPlatform() != Common::kPlatformXbox) {
		_state->setOverallVolume(CLIP<uint>(ConfMan.getInt("overall_volume") * 100 / 256, 0, 100));
		_state->setMusicVolume(CLIP<uint>(ConfMan.getInt("music_volume") * 100 / 256, 0, 100));
		_state->setMusicFrequency(ConfMan.getInt("music_frequency"));
		_state->setLanguageAudio(ConfMan.getInt("audio_language"));
		_state->setLanguageText(ConfMan.getInt("text_language"));
	} else {
		_state->setVibrationEnabled(ConfMan.getBool("vibrations"));
	}
}

void Myst3Engine::settingsApplyFromVars() {
	int32 oldTextLanguage = ConfMan.getInt("text_language");

	ConfMan.setInt("transition_speed", _state->getTransitionSpeed());
	ConfMan.setInt("mouse_speed", _state->getMouseSpeed());
	ConfMan.setBool("zip_mode", _state->getZipModeEnabled());
	ConfMan.setBool("subtitles", _state->getSubtitlesEnabled());

	if (getPlatform() != Common::kPlatformXbox) {
		ConfMan.setInt("overall_volume", _state->getOverallVolume() * 256 / 100);
		ConfMan.setInt("music_volume", _state->getMusicVolume() * 256 / 100);
		ConfMan.setInt("music_frequency", _state->getMusicFrequency());
		ConfMan.setInt("audio_language", _state->getLanguageAudio());
		ConfMan.setInt("text_language", _state->getLanguageText());
		ConfMan.setBool("water_effects", _state->getWaterEffects());

		// The language changed, reload the correct archives
		if (_state->getLanguageText() != oldTextLanguage) {
			closeArchives();
			openArchives();
		}
	} else {
		ConfMan.setBool("vibrations", _state->getVibrationEnabled());
	}

	// Mouse speed may have changed, refresh it
	_scene->updateMouseSpeed();

	syncSoundSettings();
}

void Myst3Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	uint soundOverall = ConfMan.getInt("overall_volume");
	uint soundVolumeMusic = ConfMan.getInt("music_volume");

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundOverall);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic * soundOverall / 256);
}

bool Myst3Engine::isWideScreenModEnabled() const {
	return ConfMan.getBool("widescreen_mod");
}

void Myst3Engine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (!_state || !_cursor) {
		// This method may be called before the engine is fully initialized
		return;
	}

	for (uint i = 0; i < _movies.size(); i++) {
		_movies[i]->pause(pause);
	}

	_state->pauseEngine(pause);

	// Grab a game screen thumbnail in case we need one when writing a save file
	if (pause && !_menu->isOpen()) {
		// Opening the menu generates a save thumbnail so we only generate it if the menu is not open
		_menu->generateSaveThumbnail();
	}

	// Unlock the mouse so that the cursor is visible when the GMM opens
	if (_state->getViewType() == kCube && _cursor->isPositionLocked()) {
		_system->lockMouse(!pause);
	}

	// The user may have moved the mouse or resized the screen while the engine was paused
	if (!pause) {
		_gfx->computeScreenViewport();
		_cursor->updatePosition(_eventMan->getMousePos());
		_inventory->reflow();
	}
}

} // end of namespace Myst3
