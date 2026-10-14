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
#include "gui/debugger.h"

#include "hollywood/runaway/engine.h"
#include "hollywood/runaway/intro.h"
#include "hollywood/runaway/menu.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene5030.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/strings.h"
#include "hollywood/runaway/ui.h"

namespace Hollywood {
namespace {

enum RunawayAction {
	kScrollLeft,
	kScrollRight,
	kScrollStart,
	kScrollEnd,
	kCancelAction,
	kSkip,
	kToggleInventory,
	kCycleAction,
	kPauseMenu
};

} // End of anonymous namespace

RunawayEngine::RunawayEngine(OSystem *syst, Common::Language language) :
	Engine(syst), _language(language), _strings(language) {}

RunawayEngine::~RunawayEngine() {}

void RunawayEngine::registerDefaultSettings() {
	ConfMan.registerDefault("runaway_antialias", true);
	ConfMan.registerDefault("runaway_fast_scrolling", true);
	ConfMan.registerDefault("runaway_pointer_sound", true);
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
	_system->updateScreen();
}

bool RunawayEngine::canSaveGameStateCurrently(Common::U32String *) {
	return _gameStarted && !_inPresentation && _scene && _ui && _scene->canSave() && !_ui->busy();
}

bool RunawayEngine::canLoadGameStateCurrently(Common::U32String *) {
	return _scene && _ui;
}

Common::Error RunawayEngine::saveGameStream(Common::WriteStream *stream, bool) {
	if (!canSaveGameStateCurrently())
		return Common::kWritingFailed;
	RunawaySaveState state;
	_scene->captureState(state);
	_ui->captureState(state);
	state.playTime = getTotalPlayTime();
	return state.write(*stream) ? Common::kNoError : Common::kWritingFailed;
}

Common::Error RunawayEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (!canLoadGameStateCurrently())
		return Common::kReadingFailed;
	RunawaySaveState state;
	if (!state.read(*stream))
		return Common::Error(Common::kReadingFailed, "Invalid or unsupported Runaway save");
	// Build and validate replacement resources before touching the live scene.
	// Failed reads leave the current puzzle, inventory, audio and UI intact.
	Common::ScopedPtr<RunawayScene5030> scene(new RunawayScene5030(_mixer, *_inventory, _strings));
	if (!scene->load() || !scene->restoreState(state, state.playTime))
		return Common::kReadingFailed;
	_scene.reset(scene.release());
	_ui->restoreState(state, state.playTime);
	setTotalPlayTime(state.playTime);
	_loadedGame = _gameStarted = true;
	syncSoundSettings();
	_scene->draw();
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
	return feature == kSupportsReturnToLauncher || feature == kSupportsSubtitleOptions ||
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

Common::Error RunawayEngine::playIntro(RunawayMenu &menu) {
	_inPresentation = true;
	_loadedGame = false;
	RunawayIntro intro(_mixer);
	if (!intro.load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo logo or story card");
	const bool cursorWasVisible = CursorMan.showMouse(false);
	intro.start(getTotalPlayTime());
	intro.draw();
	while (!shouldQuit() && !intro.finished() && !_loadedGame) {
		bool skip = false, openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				skip = true;
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (event.customType == kSkip || event.customType == kCancelAction)
					skip = true;
				else if (event.customType == kPauseMenu)
					openMenu = true;
			}
		}
		if (shouldQuit() || _loadedGame)
			break;
		if (openMenu) {
			pauseMenu(menu, intro.screen());
			continue;
		}
		const uint32 now = getTotalPlayTime();
		intro.update(now);
		if (skip)
			intro.skip(now);
		intro.draw();
		const Graphics::ManagedSurface &screen = intro.screen();
		present(screen);
		_system->delayMillis(10);
	}
	CursorMan.showMouse(cursorWasVisible);
	_inPresentation = false;
	return Common::kNoError;
}

Common::Error RunawayEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(kRunawayScreenWidth, kRunawayScreenHeight, &format);
	if (_system->getScreenFormat() != format)
		return Common::kUnsupportedColorMode;
	setDebugger(new GUI::Debugger());
	syncSoundSettings();
	RunawayMenu menu(_mixer, _strings);
	if (!menu.load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo menu");
	_inventory.reset(new RunawayInventory(_strings));
	_scene.reset(new RunawayScene5030(_mixer, *_inventory, _strings));
	_ui.reset(new RunawayUI(_mixer, *_inventory, _strings));
	if (!_scene->load() || !_ui->load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo room, opening movie or interface data");
	syncSoundSettings();
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0) {
		const Common::Error result = loadGameState(ConfMan.getInt("save_slot"));
		if (result.getCode() != Common::kNoError)
			return result;
	} else {
		const Common::Error result = playIntro(menu);
		if (result.getCode() != Common::kNoError || shouldQuit())
			return result;
	}
	if (!_loadedGame)
		_scene->start(getTotalPlayTime());
	_gameStarted = true;
	_loadedGame = false;
	_scene->draw();
	CursorMan.showMouse(true);
	_system->warpMouse(RunawayScene5030::kScreenWidth / 2, RunawayScene5030::kRoomTop + RunawayScene5030::kRoomHeight / 2);

	bool leftPressed = false, rightPressed = false;
	while (!shouldQuit()) {
		const uint32 now = getTotalPlayTime();
		Common::Point mouse = _eventMan->getMousePos();
		_ui->update(now, mouse, _scene->hotspotAt(mouse), _scene->busy(), _scene->speaking(), _scene->inventoryPresentation(), _scene->speechFrame());
		bool openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			if (_loadedGame)
				break;
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				_ui->update(now, event.mouse, _scene->hotspotAt(event.mouse), _scene->busy(), _scene->speaking(), _scene->inventoryPresentation(), _scene->speechFrame());
				const RunawayUI::Command command = _ui->click(event.type == Common::EVENT_RBUTTONDOWN);
				switch (command.type) {
				case RunawayUI::Command::kOpenMenu:
					openMenu = true;
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
					if (_scene->playingVideo())
						_scene->skipVideo();
					break;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START || event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				mouse = _eventMan->getMousePos();
				_ui->update(now, mouse, _scene->hotspotAt(mouse), _scene->busy(), _scene->speaking(), _scene->inventoryPresentation(), _scene->speechFrame());
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
					if (pressed && !_ui->inventoryOpen() && !_scene->busy())
						_scene->setCameraX(0);
					break;
				case kScrollEnd:
					if (pressed && !_ui->inventoryOpen() && !_scene->busy())
						_scene->setCameraX(RunawayScene5030::kRoomWidth - RunawayScene5030::kScreenWidth);
					break;
				case kCancelAction:
					if (pressed) {
						if (_scene->playingVideo())
							_scene->skipVideo();
						else if (_scene->speaking())
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
						if (_scene->playingVideo())
							_scene->skipVideo();
						else
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
		int direction = (int)rightPressed - (int)leftPressed;
		mouse = _eventMan->getMousePos();
		if (!direction && mouse.y >= RunawayScene5030::kRoomTop && mouse.y < RunawayScene5030::kRoomTop + RunawayScene5030::kRoomHeight) {
			if (mouse.x < 24)
				direction = -1;
			else if (mouse.x >= RunawayScene5030::kScreenWidth - 24)
				direction = 1;
		}
		// The engine clock excludes time spent in ScummVM dialogs, keeping
		// animation frames and subtitle deadlines in sync with paused audio.
		_scene->update(now, _ui->inventoryOpen() || _scene->busy() ? 0 : direction, _ui->inventoryOpen());
		// The extended French demo's later rooms are not implemented yet;
		// all three releases currently finish at the shared shack escape.
		if (_scene->escaped()) {
			const Common::Error result = playEnding(menu);
			if (!_loadedGame || result.getCode() != Common::kNoError || shouldQuit())
				return result;
			_loadedGame = false;
			leftPressed = rightPressed = false;
			continue;
		}
		_ui->update(now, mouse, _scene->hotspotAt(mouse), _scene->busy(), _scene->speaking(), _scene->inventoryPresentation(), _scene->speechFrame());
		_scene->draw();
		Graphics::ManagedSurface &screen = _scene->screen();
		const bool subtitles = !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles") || ConfMan.getBool("speech_mute");
		_ui->draw(screen, _scene->actionCaption(_scene->hotspotAt(mouse), _ui->verb(), _inventory->selected()),
			_scene->committedCaption(), _scene->subtitle(), subtitles);
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
	const bool cursorWasVisible = CursorMan.showMouse(false);
	ending.start(getTotalPlayTime());
	ending.draw(screen);
	while (!shouldQuit() && !ending.finished() && !_loadedGame) {
		bool skip = false, openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN)
				skip = true;
			else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (event.customType == kSkip || event.customType == kCancelAction)
					skip = true;
				else if (event.customType == kPauseMenu)
					openMenu = true;
			}
		}
		if (shouldQuit() || _loadedGame)
			break;
		if (openMenu) {
			pauseMenu(menu, screen);
			continue;
		}
		const uint32 now = getTotalPlayTime();
		ending.update(now);
		if (skip)
			ending.skip(now);
		ending.draw(screen);
		present(screen);
		_system->delayMillis(16);
	}
	CursorMan.showMouse(cursorWasVisible);
	_inPresentation = false;
	return Common::kNoError;
}

void RunawayEngine::pauseMenu(RunawayMenu &menu, Graphics::ManagedSurface &screen) {
	// Existing gameplay channels and clocks stay paused; new menu preview
	// music is owned separately and stopped before gameplay resumes.
	PauseToken pauseToken = pauseEngine();
	RunawayMenu::Settings settings = readSettings();
	menu.open(screen, settings, canSaveGameStateCurrently(), canLoadGameStateCurrently());
	menu.update(_eventMan->getMousePos());
	// Never draw options into the scene: its image is also the save thumbnail.
	Graphics::ManagedSurface menuScreen(screen.w, screen.h, screen.format);
	Common::Keymap *keymap = _eventMan->getKeymapper()->getKeymap("runaway");
	const bool keymapEnabled = keymap && keymap->isEnabled();
	const bool virtualKeyboard = _system->hasFeature(OSystem::kFeatureVirtualKeyboard) &&
		_system->getFeatureState(OSystem::kFeatureVirtualKeyboard);
	bool enteringName = false;
	bool finished = false, changed = false;
	while (!shouldQuit() && !finished) {
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
		if (finished || shouldQuit())
			break;
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

	action = new Action("CANCEL_ACTION", _("Cancel action / Menu"));
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

	action = new Action(kStandardActionSkip, _("Skip intro / Skip line"));
	action->setCustomEngineActionEvent(kSkip);
	action->addDefaultInputMapping("SPACE");
	action->addDefaultInputMapping("RETURN");
	action->addDefaultInputMapping("JOY_X");
	keymap->addAction(action);

	return Keymap::arrayOf(keymap);
}

} // End of namespace Hollywood
