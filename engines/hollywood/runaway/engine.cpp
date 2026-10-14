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
#include "backends/keymapper/standard-actions.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "gui/debugger.h"

#include "hollywood/runaway/engine.h"
#include "hollywood/runaway/intro.h"
#include "hollywood/runaway/menu.h"
#include "hollywood/runaway/scene5030.h"
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

void RunawayEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addSubDirectoryMatching(gamePath, "Resource");
	SearchMan.addSubDirectoryMatching(gamePath, "Dataa");
	SearchMan.addSubDirectoryMatching(gamePath, "Datav");
	// Also support targets added directly from the Resource directory.
	SearchMan.addSubDirectoryMatching(gamePath.getParent(), "Dataa");
	SearchMan.addSubDirectoryMatching(gamePath.getParent(), "Datav");
}

bool RunawayEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher || feature == kSupportsSubtitleOptions;
}

void RunawayEngine::syncSoundSettings() {
	Engine::syncSoundSettings();
	const bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute || ConfMan.getBool("music_mute"));
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute || ConfMan.getBool("sfx_mute"));
}

Common::Error RunawayEngine::playIntro() {
	RunawayIntro intro(_mixer);
	if (!intro.load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo logo or story card");
	const bool cursorWasVisible = CursorMan.showMouse(false);
	intro.start(getTotalPlayTime());
	while (!shouldQuit() && !intro.finished()) {
		bool skip = false;
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				skip = true;
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
					(event.customType == kSkip || event.customType == kCancelAction)) {
				skip = true;
			}
		}
		if (shouldQuit())
			break;
		const uint32 now = getTotalPlayTime();
		intro.update(now);
		if (skip)
			intro.skip(now);
		intro.draw();
		const Graphics::ManagedSurface &screen = intro.screen();
		_system->copyRectToScreen(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
		_system->updateScreen();
		_system->delayMillis(10);
	}
	CursorMan.showMouse(cursorWasVisible);
	return Common::kNoError;
}

Common::Error RunawayEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(kRunawayScreenWidth, kRunawayScreenHeight, &format);
	if (_system->getScreenFormat() != format)
		return Common::kUnsupportedColorMode;
	setDebugger(new GUI::Debugger());
	syncSoundSettings();
	const Common::Error introResult = playIntro();
	if (introResult.getCode() != Common::kNoError || shouldQuit())
		return introResult;

	const RunawayStrings strings(_language);
	RunawayInventory inventory(strings);
	RunawayScene5030 scene(_mixer, inventory, strings);
	RunawayUI ui(_mixer, inventory, strings);
	RunawayMenu menu(strings);
	if (!scene.load() || !ui.load() || !menu.load())
		return Common::Error(Common::kReadingFailed, "Cannot load the Runaway demo room, actor or interface data");
	scene.start(getTotalPlayTime());
	CursorMan.showMouse(true);
	_system->warpMouse(RunawayScene5030::kScreenWidth / 2, RunawayScene5030::kRoomTop + RunawayScene5030::kRoomHeight / 2);

	bool leftPressed = false, rightPressed = false;
	while (!shouldQuit()) {
		const uint32 now = getTotalPlayTime();
		Common::Point mouse = _eventMan->getMousePos();
		ui.update(now, mouse, scene.hotspotAt(mouse), scene.busy(), scene.speaking());
		bool openMenu = false;
		Common::Event event;
		while (!openMenu && _eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				ui.update(now, event.mouse, scene.hotspotAt(event.mouse), scene.busy(), scene.speaking());
				const RunawayUI::Command command = ui.click(event.type == Common::EVENT_RBUTTONDOWN);
				switch (command.type) {
				case RunawayUI::Command::kOpenMenu:
					openMenu = true;
					break;
				case RunawayUI::Command::kRoom:
					scene.click(command.mouse, command.verb, command.item);
					break;
				case RunawayUI::Command::kExamineItem:
					scene.sayInventory(inventory.examine(command.target));
					break;
				case RunawayUI::Command::kCombine:
					scene.sayInventory(inventory.combine(command.item, command.target));
					break;
				case RunawayUI::Command::kSkipSpeech:
					scene.skipSpeech();
					break;
				default:
					if (scene.playingVideo())
						scene.skipVideo();
					break;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START || event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				mouse = _eventMan->getMousePos();
				ui.update(now, mouse, scene.hotspotAt(mouse), scene.busy(), scene.speaking());
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
					if (pressed && !ui.inventoryOpen() && !scene.busy())
						scene.setCameraX(0);
					break;
				case kScrollEnd:
					if (pressed && !ui.inventoryOpen() && !scene.busy())
						scene.setCameraX(RunawayScene5030::kRoomWidth - RunawayScene5030::kScreenWidth);
					break;
				case kCancelAction:
					if (pressed) {
						if (scene.playingVideo())
							scene.skipVideo();
						else if (scene.speaking())
							scene.skipSpeech();
						else if (!ui.cancel())
							openMenu = true;
					}
					break;
				case kToggleInventory:
					if (pressed)
						ui.toggleInventory();
					break;
				case kCycleAction:
					if (pressed && ui.click(true).type == RunawayUI::Command::kSkipSpeech)
						scene.skipSpeech();
					break;
				case kSkip:
					if (pressed) {
						if (scene.playingVideo())
							scene.skipVideo();
						else
							scene.skipSpeech();
					}
					break;
				}
			}
		}
		if (shouldQuit())
			break;
		if (openMenu) {
			pauseMenu(menu, scene.screen());
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
		scene.update(now, ui.inventoryOpen() || scene.busy() ? 0 : direction, ui.inventoryOpen());
		ui.update(now, mouse, scene.hotspotAt(mouse), scene.busy(), scene.speaking());
		scene.draw();
		Graphics::ManagedSurface &screen = scene.screen();
		const bool subtitles = !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles") || ConfMan.getBool("speech_mute");
		ui.draw(screen, scene.actionCaption(scene.hotspotAt(mouse), ui.verb(), inventory.selected()),
			scene.committedCaption(), scene.subtitle(), subtitles);
		_system->copyRectToScreen(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
		_system->updateScreen();
		_system->delayMillis(16);
	}
	return Common::kNoError;
}

void RunawayEngine::pauseMenu(RunawayMenu &menu, Graphics::ManagedSurface &screen) {
	// PauseToken freezes the mixer and excludes menu time from the scene/UI
	// clocks, including walking, speech deadlines and pickup sound cues.
	PauseToken pauseToken = pauseEngine();
	RunawayMenu::Settings settings;
	settings.music = !ConfMan.getBool("music_mute");
	settings.subtitles = !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles");
	settings.volume = MAX(ConfMan.getInt("music_volume"), MAX(ConfMan.getInt("sfx_volume"), ConfMan.getInt("speech_volume")));
	menu.open(screen, settings);
	menu.update(_eventMan->getMousePos());
	bool finished = false, changed = false;
	while (!shouldQuit() && !finished) {
		Common::Event event;
		while (!finished && _eventMan->pollEvent(event)) {
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
				case kPauseMenu:
				case kCancelAction:
				case kSkip:
					command = menu.cancel();
					break;
				case kScrollLeft:
					command = menu.adjustVolume(-8);
					break;
				case kScrollRight:
					command = menu.adjustVolume(8);
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat) {
				command = menu.key(event.kbd.ascii);
			} else if (event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN) {
				command = menu.adjustVolume(event.type == Common::EVENT_WHEELUP ? 8 : -8);
			}
			const RunawayMenu::Settings &current = menu.settings();
			if (settings.music != current.music || settings.subtitles != current.subtitles || settings.volume != current.volume) {
				ConfMan.setBool("music_mute", !current.music);
				ConfMan.setBool("subtitles", current.subtitles);
				// Preserve independently configured channel volumes unless the
				// player actually moves the original master-volume control.
				if (settings.volume != current.volume) {
					ConfMan.setInt("music_volume", current.volume);
					ConfMan.setInt("sfx_volume", current.volume);
					ConfMan.setInt("speech_volume", current.volume);
				}
				settings = current;
				syncSoundSettings();
				changed = true;
			}
			if (command == RunawayMenu::kResume || command == RunawayMenu::kQuit) {
				if (command == RunawayMenu::kQuit)
					quitGame();
				finished = true;
			}
		}
		if (finished || shouldQuit())
			break;
		menu.draw(screen);
		_system->copyRectToScreen(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
		_system->updateScreen();
		_system->delayMillis(16);
	}
	menu.close();
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
