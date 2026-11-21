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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "engines/metaengine.h"
#include "graphics/cursorman.h"

#include "hollywood/hotspots.h"
#include "hollywood/runaway/console.h"
#include "hollywood/runaway/engine.h"
#include "hollywood/runaway/intro.h"
#include "hollywood/runaway/menu.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"
#include "hollywood/runaway/scene_registry.h"
#include "hollywood/runaway/scene5030.h"
#include "hollywood/runaway/strings.h"
#include "hollywood/runaway/ui.h"

namespace Hollywood {

RunawayEngine::RunawayEngine(OSystem *syst, Common::Language language, RunawayGameType game) :
	Engine(syst), _language(language), _game(game), _museum(game == kRunawayMuseumDemo), _strings(language), _audio(_mixer) {
	_state.game = game;
	_state.scene = _museum ? 4050 : game == kRunawayFullGame ? 2020 : 5030;
	_audio.setMuseum(_museum);
	_audio.setChapter(_state.chapter());
}

RunawayEngine::~RunawayEngine() {}

void RunawayEngine::registerDefaultSettings() {
	ConfMan.registerDefault("runaway_antialias", true);
	ConfMan.registerDefault("runaway_fast_scrolling", true);
	ConfMan.registerDefault("runaway_pointer_sound", true);
	ConfMan.registerDefault("runaway_last_chapter", 0);
}

RunawayMenu::Settings RunawayEngine::readSettings() const {
	RunawayMenu::Settings settings;
	settings.music = !ConfMan.getBool("music_mute");
	settings.subtitles = !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles");
	settings.antialias = ConfMan.getBool("runaway_antialias");
	settings.fastScrolling = ConfMan.getBool("runaway_fast_scrolling");
	settings.pointerSound = ConfMan.getBool("runaway_pointer_sound");
	settings.volume = MAX(ConfMan.getInt("music_volume"), MAX(ConfMan.getInt("sfx_volume"), ConfMan.getInt("speech_volume")));
	return settings;
}

void RunawayEngine::applySettings(const RunawayMenu::Settings &settings, bool restoreDefaults) {
	const RunawayMenu::Settings previous = readSettings();
	ConfMan.setBool("music_mute", !settings.music);
	ConfMan.setBool("subtitles", settings.subtitles);
	ConfMan.setBool("runaway_antialias", settings.antialias);
	ConfMan.setBool("runaway_fast_scrolling", settings.fastScrolling);
	ConfMan.setBool("runaway_pointer_sound", settings.pointerSound);
	// Preserve independent channel levels until the master dial is adjusted
	// or the player explicitly restores defaults.
	if (restoreDefaults || previous.volume != settings.volume) {
		ConfMan.setInt("music_volume", settings.volume);
		ConfMan.setInt("sfx_volume", settings.volume);
		ConfMan.setInt("speech_volume", settings.volume);
	}
	syncSoundSettings();
}

void RunawayEngine::present(const Graphics::ManagedSurface &screen) {
	_system->copyRectToScreen(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
	drawHotspots();
	_system->updateScreen();
}

void RunawayEngine::getHotspotPositions(Common::Array<Graphics::HotspotInfo> &hotspots) {
	if (_gameStarted && !_inPresentation && _scene && _ui && !sceneBusy() && !_scene->speaking() &&
			!_ui->busy() && !_ui->inventoryOpen()) {
		_scene->getHotspotPositions(hotspots, Common::Rect(0, kRunawayViewportTop,
			kRunawayScreenWidth, kRunawayViewportTop + kRunawayViewportHeight));
	}
}

Graphics::HotspotRenderer *RunawayEngine::createHotspotRenderer() {
	return createHollywoodHotspotRenderer();
}

void RunawayEngine::updateSoundListener() {
	// Full-screen films have their own coordinates; room sounds use the
	// same horizontal camera position as the original DirectSound listener.
	_audio.setListenerPosition((_scene->playingVideo() ? 0 : _scene->cameraX()) + kRunawayScreenWidth / 2);
}

Common::Point RunawayEngine::updateUI(uint32 now) {
	const bool presentation = _scene->playingVideo();
	const Common::Point mouse = _presentationMouse.update(presentation);
	if (!presentation) {
		const byte hotspot = _scene->hotspotAt(mouse);
		_ui->setDialogueChoices(_scene->dialogueChoices());
		_ui->update(now, mouse, hotspot, sceneBusy(), _scene->speaking(),
			_scene->inventoryPresentation(), _scene->speechFrame(), _scene->hasVerbControls(),
			_scene->actionCursor(hotspot), _scene->preferLook(hotspot));
	}
	return mouse;
}

bool RunawayEngine::canSaveGameStateCurrently(Common::U32String *) {
	return _gameStarted && !_inPresentation && !_pendingDoorClose && _scene && _ui && _scene->canSave() && !_ui->busy();
}

bool RunawayEngine::canLoadGameStateCurrently(Common::U32String *) {
	return _scene && _ui;
}

Common::Error RunawayEngine::saveGameStream(Common::WriteStream *stream, bool) {
	if (!canSaveGameStateCurrently())
		return Common::kWritingFailed;
	RunawaySaveState state = _state;
	_scene->captureState(state);
	_inventory->captureState(state);
	_ui->captureState(state);
	state.playTime = getTotalPlayTime();
	return state.write(*stream) ? Common::kNoError : Common::kWritingFailed;
}

Common::Error RunawayEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (!canLoadGameStateCurrently())
		return Common::kReadingFailed;
	RunawaySaveState state;
	if (!state.read(*stream) || state.game != _game)
		return Common::Error(Common::kReadingFailed, "Invalid or unsupported Runaway save");
	// Build and validate replacement resources before touching the live scene.
	// Failed reads leave the current puzzle, inventory, audio and UI intact.
	Common::ScopedPtr<RunawayScene> scene(createScene(state.scene));
	if (!scene || !scene->load(state.scene) || !scene->restoreState(state, state.playTime))
		return Common::kReadingFailed;
	if (state.chapter() != _state.chapter() && !_ui->loadChapter(state.chapter()))
		return Common::kReadingFailed;
	_audio.stopMusic();
	_audio.stopSpeech();
	_audio.stopEffect();
	_audio.setChapter(state.chapter());
	_scene.reset(scene.release());
	_state = state;
	_inventory->restoreState(state);
	_ui->restoreState(state, state.playTime);
	setTotalPlayTime(state.playTime);
	_pendingDoorClose = false;
	_loadedGame = _gameStarted = true;
	syncSoundSettings();
	updateSoundListener();
	_scene->resume();
	updateSoundListener();
	_scene->draw();
	unlockChapter(state.chapter());
	return Common::kNoError;
}

RunawayScene *RunawayEngine::createScene(uint16 entryScene) {
	const RunawayRoom *room = findRunawayRoom(entryScene);
	if (room && ((_museum && entryScene >= 4000) ||
			(_game == kRunawayFullGame && (entryScene / 1000 == 2 || entryScene / 1000 == 4 || entryScene / 1000 == 5 || entryScene / 1000 == 6 || entryScene / 1000 == 7 || entryScene / 1000 == 8 || entryScene / 1000 == 9)))) {
		return createRunawayRoomScene(*room, _audio, *_inventory, _strings, _game);
	}
	if ((_game == kRunawayCabinDemo && entryScene == 5030) ||
			(_game == kRunawayFullGame && entryScene >= 5030 && entryScene <= 5032))
		return new Runaway::Scene5030(_mixer, _audio, *_inventory, _strings, _game);
	// Retail scene scripts have their own entry substates. Sharing an archive
	// with a demo does not make its puzzle initialization interchangeable.
	return nullptr;
}

bool RunawayEngine::sceneBusy() const {
	return _pendingDoorClose || _scene->busy();
}

Common::Error RunawayEngine::changeScene() {
	const RunawayScene::Transition transition = _scene->transition();
	RunawaySaveState state = _state;
	_scene->captureState(state);
	_inventory->captureState(state);
	state.scene = transition.scene;
	state.x = transition.position.x;
	state.y = transition.position.y;
	state.facing = transition.facing;
	state.camera = state.scene >= 2020 && state.scene <= 2023 ? 156 : 0;
	state.cameraY = 0;
	Common::ScopedPtr<RunawayScene> scene(createScene(state.scene));
	if (!scene)
		return Common::Error(Common::kUnsupportedGameidError,
			Common::String::format("Runaway scene %u is not implemented", state.scene));
	if (!scene->load(state.scene) || !scene->restoreState(state, getTotalPlayTime(), true))
		return Common::Error(Common::kReadingFailed,
			Common::String::format("Cannot load Runaway scene %u", state.scene));
	if (state.chapter() != _state.chapter() && !_ui->loadChapter(state.chapter()))
		return Common::Error(Common::kReadingFailed, "Cannot load Runaway chapter graphics");
	_audio.stopSpeech();
	_audio.stopEffect();
	_audio.setChapter(state.chapter());
	_state = state;
	_inventory->setChapter(state.chapter());
	_scene.reset(scene.release());
	syncSoundSettings();
	updateSoundListener();
	_scene->resume();
	updateSoundListener();
	_pendingDoorClose = transition.door;
	if (transition.door) {
		_audio.playEffect(_museum ? 3 : 58);
	}
	return Common::kNoError;
}

bool RunawayEngine::copySaveThumbnail(Graphics::Surface &thumbnail) const {
	if (!_scene)
		return false;
	const Graphics::ManagedSurface &screen = _scene->screen();
	thumbnail.create(160, 100, screen.format);
	for (int y = 0; y < thumbnail.h; ++y) {
		uint16 *out = (uint16 *)thumbnail.getBasePtr(0, y);
		for (int x = 0; x < thumbnail.w; ++x)
			out[x] = *(const uint16 *)screen.getBasePtr(x * screen.w / thumbnail.w, kRunawayViewportTop + y * 600 / thumbnail.h);
	}
	return true;
}

void RunawayEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addSubDirectoryMatching(gamePath, "Resource");
	SearchMan.addSubDirectoryMatching(gamePath, "Dataa");
	SearchMan.addSubDirectoryMatching(gamePath, "Datav");
	SearchMan.addSubDirectoryMatching(gamePath, "Screenshots");
	// Also support targets added directly from the Resource directory.
	SearchMan.addSubDirectoryMatching(gamePath.getParent(), "Dataa");
	SearchMan.addSubDirectoryMatching(gamePath.getParent(), "Datav");
	SearchMan.addSubDirectoryMatching(gamePath.getParent(), "Screenshots");
}

bool RunawayEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher || (feature == kSupportsSubtitleOptions && !_museum) ||
		feature == kSupportsSavingDuringRuntime || feature == kSupportsLoadingDuringRuntime;
}

void RunawayEngine::syncSoundSettings() {
	Engine::syncSoundSettings();
	const bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute || ConfMan.getBool("music_mute"));
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute || ConfMan.getBool("sfx_mute"));
	const RunawayMenu::Settings settings = readSettings();
	if (_scene)
		_scene->setOptions(settings.antialias, settings.fastScrolling);
	if (_ui)
		_ui->setPointerSound(settings.pointerSound);
}

Common::Error RunawayEngine::playIntro(RunawayMenu &menu, byte chapter, RunawayIntro::Mode mode) {
	_inPresentation = true;
	_loadedGame = false;
	RunawayIntro intro(_mixer);
	if (!intro.load(_game == kRunawayFullGame, chapter, mode)) {
		_inPresentation = false;
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway opening resources");
	}
	_presentationMouse.update(true);
	intro.start(getTotalPlayTime());
	intro.draw();
	while (!shouldQuit() && !intro.finished() && !_loadedGame) {
		bool openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			openMenu = intro.handlePresentationEvent(event);
		}
		if (shouldQuit() || _loadedGame)
			break;
		if (openMenu) {
			pauseMenu(menu, intro.screen());
			continue;
		}
		const uint32 now = getTotalPlayTime();
		intro.processPresentationInput(now);
		intro.update(now);
		intro.draw();
		_presentationMouse.update(true);
		if (!ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles") || ConfMan.getBool("speech_mute"))
			_ui->drawSubtitle(intro.screen(), intro.subtitle());
		const Graphics::ManagedSurface &screen = intro.screen();
		present(screen);
		_system->delayMillis(10);
	}
	_presentationMouse.update(false);
	_inPresentation = false;
	if (intro.failed())
		return Common::Error(Common::kReadingFailed, "Cannot play the Runaway opening sequence");
	return Common::kNoError;
}

Common::Error RunawayEngine::startChapter(RunawayMenu &menu, byte chapter) {
	assert(chapter <= 6);
	const bool fromBeginning = chapter == 0;
	chapter = MAX<byte>(1, chapter);
	const uint16 entries[] = {2020, 4010, 5030, 6350, 7010, 8100};
	const uint16 entry = entries[chapter - 1];
	Common::ScopedPtr<RunawayScene> scene(createScene(entry));
	if (!scene || !scene->load(entry) || !_ui->loadChapter(chapter))
		return Common::Error(Common::kReadingFailed, "Cannot load Runaway chapter resources");
	_audio.stopSpeech();
	_audio.stopEffect();
	_audio.stopMusic();
	_audio.setChapter(chapter);
	_state = RunawaySaveState();
	_state.game = _game;
	_state.scene = entry;
	_inventory->startChapter(chapter);
	_ui->restoreState(_state, getTotalPlayTime());
	_scene.reset(scene.release());
	_pendingDoorClose = false;
	syncSoundSettings();
	const Common::Error result = playIntro(menu, fromBeginning ? 0 : chapter, RunawayIntro::kOpeningOnly);
	if (result.getCode() == Common::kNoError && !_loadedGame && !shouldQuit()) {
		updateSoundListener();
		_scene->start(getTotalPlayTime());
		_gameStarted = true;
		unlockChapter(chapter);
	}
	_loadedGame = false;
	return result;
}

byte RunawayEngine::unlockedChapter() const {
	return CLIP(ConfMan.getInt("runaway_last_chapter", _targetName), 0, 6);
}

void RunawayEngine::unlockChapter(byte chapter) {
	if (_game == kRunawayFullGame && chapter > unlockedChapter() && chapter <= 6) {
		// The original updates RESOURCE.000; keep progress in the target configuration.
		ConfMan.setInt("runaway_last_chapter", chapter, _targetName);
		ConfMan.flushToDisk();
	}
}

Common::Error RunawayEngine::selectChapter(RunawayMenu &menu) {
	_inPresentation = true;
	_audio.stopMusic();
	_audio.stopSpeech();
	_audio.stopEffect();
	const int chapter = pauseMenu(menu, _scene->screen(), MAX<byte>(1, unlockedChapter()));
	_inPresentation = false;
	if (chapter >= 0 && !_loadedGame && !shouldQuit()) {
		return startChapter(menu, chapter);
	}
	return Common::kNoError;
}

Common::Error RunawayEngine::run() {
	if (!_strings.load()) {
		return Common::Error(Common::kReadingFailed, "Cannot load Runaway text. Please use the matching hollywood.dat file.");
	}
	_inventory.reset(new RunawayInventory(_strings, _game));
	if (_museum && ConfMan.hasKey("boot_param") && ConfMan.getInt("boot_param") >= 4000 &&
			findRunawayRoom(ConfMan.getInt("boot_param")))
		_state.scene = ConfMan.getInt("boot_param");
	if (_game == kRunawayFullGame && ConfMan.hasKey("boot_param")) {
		const int scene = ConfMan.getInt("boot_param");
		if (scene == 4000) {
			_state.scene = 4010;
		} else if (scene == 6000) {
			_state.scene = 6350;
		} else if (scene == 7000) {
			_state.scene = 7010;
		} else if (scene == 8000) {
			_state.scene = 8100;
		} else if (scene == 5030 || findRunawayRoom(scene)) {
			_state.scene = scene;
		}
	}
	_inventory->setChapter(_state.chapter());
	_audio.setChapter(_state.chapter());
	_scene.reset(createScene(_state.scene));
	if (!_scene)
		return Common::Error(Common::kUnsupportedGameidError,
			Common::String::format("Runaway scene %u is not implemented yet", _state.scene));
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(kRunawayScreenWidth, kRunawayScreenHeight, &format);
	if (_system->getScreenFormat() != format)
		return Common::kUnsupportedColorMode;
	setDebugger(new RunawayConsole(this));
	syncSoundSettings();
	RunawayMenu menu(_mixer, _strings);
	if (!menu.load(_game))
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway menu");
	_ui.reset(new RunawayUI(_mixer, *_inventory, _strings));
	if (!_scene->load(_state.scene) || !_ui->load(_game, _state.chapter()))
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway resources");
	syncSoundSettings();
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0) {
		const Common::Error result = loadGameState(ConfMan.getInt("save_slot"));
		if (result.getCode() != Common::kNoError)
			return result;
	} else if (_game == kRunawayFullGame &&
			((!ConfMan.hasKey("boot_param") && unlockedChapter()) ||
			(ConfMan.hasKey("boot_param") && ConfMan.getInt("boot_param") == 1000))) {
		Common::Error result = playIntro(menu, 0, RunawayIntro::kLogoOnly);
		if (result.getCode() == Common::kNoError && !_loadedGame && !shouldQuit()) {
			result = selectChapter(menu);
		}
		if (result.getCode() != Common::kNoError || shouldQuit()) {
			return result;
		}
	} else if (_game == kRunawayCabinDemo || (_game == kRunawayFullGame && _state.chapter() == 1)) {
		const Common::Error result = playIntro(menu);
		if (result.getCode() != Common::kNoError || shouldQuit())
			return result;
	} else if (_game == kRunawayFullGame && (_state.scene == 4010 || _state.scene == 6350 ||
			_state.scene == 7010 || _state.scene == 8100)) {
		const Common::Error result = playIntro(menu, _state.chapter());
		if (result.getCode() != Common::kNoError || shouldQuit())
			return result;
	}
	if (!_gameStarted) {
		updateSoundListener();
		_scene->start(getTotalPlayTime());
		unlockChapter(_state.chapter());
	}
	_gameStarted = true;
	_loadedGame = false;
	_scene->draw();
	CursorMan.showMouse(true);
	_system->warpMouse(kRunawayScreenWidth / 2, kRunawayViewportTop + kRunawayViewportHeight / 2);

	bool leftPressed = false, rightPressed = false;
	while (!shouldQuit()) {
		const uint32 now = getTotalPlayTime();
		// Museum doors close once the opening sound finishes, across the room change.
		if (_pendingDoorClose && !_audio.effectPlaying()) {
			_pendingDoorClose = false;
			_audio.playEffect(_museum ? 4 : 59);
		}
		updateUI(now);
		if (_scene->playingVideo()) {
			leftPressed = rightPressed = false;
		}
		bool openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			if (_loadedGame)
				break;
			if (_scene->playingVideo()) {
				openMenu = _scene->handlePresentationEvent(event);
				continue;
			}
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				updateUI(now);
				const RunawayUI::Command command = _ui->click(event.type == Common::EVENT_RBUTTONDOWN);
				switch (command.type) {
				case RunawayUI::Command::kOpenMenu:
					openMenu = true;
					break;
				case RunawayUI::Command::kChooseDialogue:
					_scene->chooseDialogue(command.target);
					break;
				case RunawayUI::Command::kRoom:
					_scene->click(command.mouse, command.verb, command.item);
					break;
				case RunawayUI::Command::kExamineItem:
					_scene->sayInventory(_inventory->examine(command.target));
					break;
				case RunawayUI::Command::kCombine:
					_scene->sayInventory(_inventory->combine(command.item, command.target));
					break;
				case RunawayUI::Command::kSkipSpeech:
					_scene->skipSpeech();
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START || event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				updateUI(now);
				const bool pressed = event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START;
				switch (event.customType) {
				case kPauseMenu:
					openMenu = pressed;
					break;
				case kScrollLeft:
					leftPressed = pressed;
					break;
				case kScrollRight:
					rightPressed = pressed;
					break;
				case kScrollStart:
					if (pressed && !_ui->inventoryOpen() && !sceneBusy())
						_scene->setCameraX(0);
					break;
				case kScrollEnd:
					if (pressed && !_ui->inventoryOpen() && !sceneBusy())
						_scene->setCameraX(_scene->roomWidth() - kRunawayScreenWidth);
					break;
				case kCancelAction:
					if (pressed) {
						if (_scene->speaking())
							_scene->skipSpeech();
						else if (!_ui->cancel())
							openMenu = true;
					}
					break;
				case kToggleInventory:
					if (pressed)
						_ui->toggleInventory();
					break;
				case kCycleAction:
					if (pressed && _ui->click(true).type == RunawayUI::Command::kSkipSpeech)
						_scene->skipSpeech();
					break;
				case kSkip:
					if (pressed) {
						_scene->skipSpeech();
					}
					break;
				}
			}
		}
		if (shouldQuit())
			break;
		if (_loadedGame) {
			_loadedGame = false;
			leftPressed = rightPressed = false;
			continue;
		}
		if (openMenu) {
			pauseMenu(menu, _scene->screen());
			_loadedGame = false;
			leftPressed = rightPressed = false;
			// Refresh the engine clock and redraw the room after resuming.
			// The input which closed the menu must not reach a room action.
			continue;
		}
		if (_scene->playingVideo()) {
			_scene->processPresentationInput(now);
		}
		int direction = (int)rightPressed - (int)leftPressed;
		Common::Point mouse = _presentationMouse.update(_scene->playingVideo());
		if (!direction && mouse.y >= kRunawayViewportTop && mouse.y < kRunawayViewportTop + kRunawayViewportHeight) {
			if (mouse.x < 24)
				direction = -1;
			else if (mouse.x >= kRunawayScreenWidth - 24)
				direction = 1;
		}
		// The engine clock excludes time spent in ScummVM dialogs, keeping
		// animation frames and subtitle deadlines in sync with paused audio.
		updateSoundListener();
		_scene->update(now, _ui->inventoryOpen() || sceneBusy() ? 0 : direction, _ui->inventoryOpen());
		updateSoundListener();
		if (_scene->transition().scene) {
			// Brian brings Gina to Oscar before the chapter-six narration (8000).
			const Common::Error result = _state.scene == 8092 ? startChapter(menu, 6) : changeScene();
			if (result.getCode() != Common::kNoError)
				return result;
			leftPressed = rightPressed = false;
			continue;
		}
		if (_scene->escaped()) {
			if (_game == kRunawayFullGame) {
				const Common::Error result = _state.chapter() == 6 ?
					selectChapter(menu) : startChapter(menu, _state.chapter() + 1);
				if (result.getCode() != Common::kNoError || shouldQuit())
					return result;
				_loadedGame = false;
				leftPressed = rightPressed = false;
				continue;
			}
			// The extended French demo's later rooms are not implemented yet;
			// all three releases currently finish at the shared shack escape.
			const Common::Error result = playEnding(menu);
			if (!_loadedGame || result.getCode() != Common::kNoError || shouldQuit())
				return result;
			_loadedGame = false;
			leftPressed = rightPressed = false;
			continue;
		}
		mouse = updateUI(now);
		if (isPaused()) {
			// Movie preparation freezes the clock and audio, but still accepts input.
			_system->delayMillis(1);
			continue;
		}
		_scene->draw();
		Graphics::ManagedSurface &screen = _scene->screen();
		const bool subtitles = _museum || !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles") || ConfMan.getBool("speech_mute");
		if (_scene->playingVideo()) {
			if (subtitles) {
				_ui->drawSubtitle(screen, _scene->subtitle());
			}
		} else {
			_ui->draw(screen, _scene->actionCaption(_scene->hotspotAt(mouse), _ui->verb(), _inventory->selected()),
				_scene->committedCaption(), _scene->subtitle(), subtitles);
		}
		present(screen);
		_system->delayMillis(16);
	}
	return Common::kNoError;
}

Common::Error RunawayEngine::playEnding(RunawayMenu &menu) {
	_inPresentation = true;
	_loadedGame = false;
	RunawayEnding ending(_mixer, _language);
	if (!ending.load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo ending");
	Graphics::ManagedSurface screen(kRunawayScreenWidth, kRunawayScreenHeight, _system->getScreenFormat());
	_presentationMouse.update(true);
	ending.start(getTotalPlayTime());
	ending.draw(screen);
	while (!shouldQuit() && !ending.finished() && !_loadedGame) {
		bool openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			openMenu = ending.handlePresentationEvent(event);
		}
		if (shouldQuit() || _loadedGame)
			break;
		if (openMenu) {
			pauseMenu(menu, screen);
			continue;
		}
		const uint32 now = getTotalPlayTime();
		ending.processPresentationInput(now);
		ending.update(now);
		ending.draw(screen);
		_presentationMouse.update(true);
		present(screen);
		_system->delayMillis(16);
	}
	_presentationMouse.update(false);
	_inPresentation = false;
	return Common::kNoError;
}

int RunawayEngine::pauseMenu(RunawayMenu &menu, Graphics::ManagedSurface &screen, byte chapters) {
	// Existing gameplay channels and clocks stay paused; new menu preview
	// music is owned separately and stopped before gameplay resumes.
	PauseToken pauseToken = pauseEngine();
	RunawayMenu::Settings settings = readSettings();
	menu.open(screen, settings, canSaveGameStateCurrently(), canLoadGameStateCurrently(), chapters);
	menu.update(_eventMan->getMousePos());
	// Never draw options into the scene: its image is also the save thumbnail.
	Graphics::ManagedSurface menuScreen(screen.w, screen.h, screen.format);
	Common::Keymap *keymap = _eventMan->getKeymapper()->getKeymap("runaway");
	const bool keymapEnabled = keymap && keymap->isEnabled();
	const bool virtualKeyboard = _system->hasFeature(OSystem::kFeatureVirtualKeyboard) &&
		_system->getFeatureState(OSystem::kFeatureVirtualKeyboard);
	bool enteringName = false;
	bool finished = false, changed = false;
	int chapter = -1;
	while (!shouldQuit() && !finished) {
		if (chapters) {
			menu.setChapterCount(MAX<byte>(1, unlockedChapter()));
		}
		menu.tick(_system->getMillis());
		Common::Event event;
		while (!finished && _eventMan->pollEvent(event)) {
			if (_loadedGame) {
				finished = true;
				break;
			}
			RunawayMenu::Command command = RunawayMenu::kNone;
			if (event.type == Common::EVENT_MOUSEMOVE) {
				command = menu.update(event.mouse);
			} else if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				menu.update(event.mouse);
				command = menu.click(event.type == Common::EVENT_RBUTTONDOWN);
			} else if (event.type == Common::EVENT_LBUTTONUP) {
				command = menu.update(event.mouse);
				menu.release();
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				switch (event.customType) {
				case kPauseMenu: case kCancelAction:
					command = menu.cancel();
					break;
				case kSkip: command = menu.key(Common::KeyState(Common::KEYCODE_RETURN)); break;
				case kScrollLeft: command = menu.key(Common::KeyState(Common::KEYCODE_LEFT)); break;
				case kScrollRight: command = menu.key(Common::KeyState(Common::KEYCODE_RIGHT)); break;
				case kScrollStart: command = menu.key(Common::KeyState(Common::KEYCODE_HOME)); break;
				case kScrollEnd: command = menu.key(Common::KeyState(Common::KEYCODE_END)); break;
				default: break;
				}
			} else if (event.type == Common::EVENT_KEYDOWN) {
				if (!event.kbdRepeat || menu.enteringName())
					command = menu.key(event.kbd);
			} else if (event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN) {
				command = menu.adjust(event.type == Common::EVENT_WHEELUP ? 4 : -4);
			}
			if (settings != menu.settings() || command == RunawayMenu::kRestoreDefaults) {
				settings = menu.settings();
				applySettings(settings, command == RunawayMenu::kRestoreDefaults);
				changed = true;
			}
			if (command == RunawayMenu::kBrowseLoad || command == RunawayMenu::kBrowseSave || command == RunawayMenu::kBrowseDelete) {
				const SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());
				int freeSlot = -1;
				if (command == RunawayMenu::kBrowseSave) {
					for (int slot = 0; slot <= getMetaEngine()->getMaximumSaveSlot(); ++slot) {
						if (slot != getMetaEngine()->getAutosaveSlot() && !_saveFileMan->exists(getSaveStateName(slot))) {
							freeSlot = slot;
							break;
						}
					}
				}
				menu.browse(command, saves, freeSlot);
				if (command == RunawayMenu::kBrowseSave && freeSlot < 0)
					menu.showMessage(_strings.text(RunawayStrings::kNoFreeSlots));
			} else if (command == RunawayMenu::kSaveGame) {
				const Common::Error result = saveGameState(menu.selectedSlot(), menu.saveDescription());
				menu.operationDone(result.getCode() == Common::kNoError);
				if (result.getCode() != Common::kNoError)
					menu.showMessage(_strings.text(RunawayStrings::kSaveFailed));
			} else if (command == RunawayMenu::kLoadGame) {
				const Common::Error result = loadGameState(menu.selectedSlot());
				finished = result.getCode() == Common::kNoError;
				if (!finished)
					menu.showMessage(_strings.text(RunawayStrings::kLoadFailed));
			} else if (command == RunawayMenu::kDeleteGame) {
				const bool success = getMetaEngine()->removeSaveState(_targetName.c_str(), menu.selectedSlot());
				menu.operationDone(success);
				if (!success)
					menu.showMessage(_strings.text(RunawayStrings::kDeleteFailed));
			} else if (command == RunawayMenu::kStartChapter) {
				chapter = menu.selectedChapter();
				finished = true;
			} else if (command == RunawayMenu::kSaveSettings) {
				ConfMan.flushToDisk();
				menu.showMessage(_strings.text(RunawayStrings::kSettingsSaved));
			} else if (command == RunawayMenu::kResume || command == RunawayMenu::kQuit) {
				if (command == RunawayMenu::kQuit)
					quitGame();
				finished = true;
			}
			if (enteringName != menu.enteringName()) {
				enteringName = menu.enteringName();
				// Space, I, Tab and remapped game shortcuts must remain text
				// while entering a name, rather than triggering game actions.
				if (keymap)
					keymap->setEnabled(keymapEnabled && !enteringName);
				if (_system->hasFeature(OSystem::kFeatureVirtualKeyboard))
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, enteringName || virtualKeyboard);
			}
		}
		if (finished || shouldQuit() || _loadedGame) {
			break;
		}
		menu.draw(menuScreen);
		present(menuScreen);
		_system->delayMillis(16);
	}
	menu.close();
	if (keymap)
		keymap->setEnabled(keymapEnabled);
	if (_system->hasFeature(OSystem::kFeatureVirtualKeyboard))
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, virtualKeyboard);
	if (changed)
		ConfMan.flushToDisk();
	return chapter;
}

Common::Array<Common::Keymap *> RunawayEngine::initKeymaps() {
	using namespace Common;
	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "runaway", "Runaway: A Road Adventure");
	keymap->setPartialMatchAllowed(false);

	Action *action = new Action(kStandardActionLeftClick, _("Perform action"));
	action->setLeftClickEvent();
	action->addDefaultInputMapping("MOUSE_LEFT");
	action->addDefaultInputMapping("JOY_A");
	keymap->addAction(action);

	action = new Action(kStandardActionRightClick, _("Next action / Cancel"));
	action->setRightClickEvent();
	action->addDefaultInputMapping("MOUSE_RIGHT");
	action->addDefaultInputMapping("JOY_B");
	keymap->addAction(action);

	action = new Action("SCROLL_LEFT", _("Scroll left"));
	action->setCustomEngineActionEvent(kScrollLeft);
	action->addDefaultInputMapping("LEFT");
	keymap->addAction(action);

	action = new Action("SCROLL_RIGHT", _("Scroll right"));
	action->setCustomEngineActionEvent(kScrollRight);
	action->addDefaultInputMapping("RIGHT");
	keymap->addAction(action);

	action = new Action("SCROLL_START", _("Scroll to the left edge"));
	action->setCustomEngineActionEvent(kScrollStart);
	action->addDefaultInputMapping("HOME");
	keymap->addAction(action);

	action = new Action("SCROLL_END", _("Scroll to the right edge"));
	action->setCustomEngineActionEvent(kScrollEnd);
	action->addDefaultInputMapping("END");
	keymap->addAction(action);

	action = new Action(kStandardActionOpenMainMenu, _("Game menu"));
	action->setCustomEngineActionEvent(kPauseMenu);
	action->addDefaultInputMapping("F5");
	action->addDefaultInputMapping("JOY_START");
	keymap->addAction(action);

	action = new Action("CANCEL_ACTION", _("Skip cutscene / Cancel action / Menu"));
	action->setCustomEngineActionEvent(kCancelAction);
	action->addDefaultInputMapping("ESCAPE");
	keymap->addAction(action);

	action = new Action("INVENTORY", _("Inventory"));
	action->setCustomEngineActionEvent(kToggleInventory);
	action->addDefaultInputMapping("i");
	action->addDefaultInputMapping("JOY_Y");
	keymap->addAction(action);

	action = new Action("CYCLE_ACTION", _("Next action / Cancel"));
	action->setCustomEngineActionEvent(kCycleAction);
	action->addDefaultInputMapping("TAB");
	keymap->addAction(action);

	action = new Action(kStandardActionSkip, _("Skip line"));
	action->setCustomEngineActionEvent(kSkip);
	action->addDefaultInputMapping("SPACE");
	action->addDefaultInputMapping("RETURN");
	action->addDefaultInputMapping("JOY_X");
	keymap->addAction(action);

	return Keymap::arrayOf(keymap);
}

} // End of namespace Hollywood
