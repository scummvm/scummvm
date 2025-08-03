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

#include "alcachofa.h"
#include "metaengine.h"
#include "menu.h"
#include "player.h"
#include "script.h"

using namespace Common;

namespace Alcachofa {

Menu::Menu()
	: _interactionSemaphore("menu")
	, _saveFileMgr(g_system->getSavefileManager()) {}

void Menu::resetAfterLoad() {
	_isOpen = false;
	_openAtNextFrame = false;
	_previousRoom = nullptr;
}

void Menu::updateOpeningMenu() {
	if (!_openAtNextFrame) {
		_openAtNextFrame =
			g_engine->input().wasMenuKeyPressed() &&
			g_engine->player().isAllowedToOpenMenu();
		return;
	}
	_openAtNextFrame = false;

	g_engine->sounds().pauseAll(true);
	_millisBeforeMenu = g_engine->getMillis();
	_previousRoom = g_engine->player().currentRoom();
	_isOpen = true;
	// TODO: Render thumbnail
	g_engine->player().changeRoom("MENUPRINCIPAL", true);
	_savefiles = _saveFileMgr->listSavefiles(g_engine->getSaveStatePattern());
	sort(_savefiles.begin(), _savefiles.end()); // the pattern ensures that the last file has the greatest slot
	_selectedSavefileI = _savefiles.size();
	updateSelectedSavefile();

	g_engine->player().heldItem() = nullptr;
	g_engine->scheduler().backupContext();
	g_engine->camera().backup(1);
	g_engine->camera().setPosition(Math::Vector3d(
		g_system->getWidth() / 2.0f, g_system->getHeight() / 2.0f, 0.0f));
}

void Menu::updateSelectedSavefile() {
	auto getButton = [] (const char *name) {
		MenuButton *button = dynamic_cast<MenuButton *>(g_engine->player().currentRoom()->getObjectByName(name));
		scumm_assert(button != nullptr);
		return button;
	};

	getButton("CARGAR")->isInteractable() = _selectedSavefileI < _savefiles.size();
	getButton("ANTERIOR")->toggle(_selectedSavefileI > 0);
	getButton("SIGUIENTE")->toggle(_selectedSavefileI < _savefiles.size());

	// TODO: Update thumbnail animation
}

void Menu::continueGame() {
	assert(_previousRoom != nullptr);
	_isOpen = false;
	g_engine->input().nextFrame(); // presumably to clear all was* flags
	g_engine->player().changeRoom(_previousRoom->name(), true);
	g_engine->sounds().pauseAll(false);
	g_engine->camera().restore(1);
	g_engine->scheduler().restoreContext();
	g_engine->setMillis(_millisBeforeMenu);
}

void Menu::triggerMainMenuAction(MainMenuAction action) {
	switch (action) {
	case MainMenuAction::ContinueGame:
		g_engine->menu().continueGame();
		break;
	case MainMenuAction::Save:
		warning("STUB: MainMenuAction Save");
		break;
	case MainMenuAction::Load: {
		// we are in some update loop, let's load next frame upon event handling
		// that should be safer
		Event ev;
		ev.type = EVENT_CUSTOM_ENGINE_ACTION_START;
		ev.customType = (CustomEventType)EventAction::LoadFromMenu;
		g_system->getEventManager()->pushEvent(ev);
	}break;
	case MainMenuAction::InternetMenu:
		g_system->messageBox(LogMessageType::kWarning, "Multiplayer is not implemented in this ScummVM version.");
		break;
	case MainMenuAction::OptionsMenu:
		g_engine->menu().openOptionsMenu();
		break;
	case MainMenuAction::Exit:
	case MainMenuAction::AlsoExit:
		// implemented in AlcachofaEngine as it has its own event loop
		g_engine->fadeExit();
		break;
	case MainMenuAction::NextSave:
		if (_selectedSavefileI < _savefiles.size()) {
			_selectedSavefileI++;
			updateSelectedSavefile();
		}
		break;
	case MainMenuAction::PrevSave:
		if (_selectedSavefileI > 0) {
			_selectedSavefileI--;
			updateSelectedSavefile();
		}
		break;
	case MainMenuAction::NewGame:
		// this action might be unused just like the only room it would appear: MENUPRINCIPALINICIO
		g_engine->player().isGameLoaded() = true;
		g_engine->script().createProcess(MainCharacterKind::None, g_engine->world().initScriptName());
		break;
	default:
		warning("Unknown main menu action: %d", (int32)action);
		break;
	}
}

void Menu::triggerLoad() {
	auto *savefile = _saveFileMgr->openForLoading(_savefiles[_selectedSavefileI]);
	g_engine->loadGameStream(savefile);
	delete savefile;
}

void Menu::openOptionsMenu() {
	setOptionsState();
	g_engine->player().changeRoom("MENUOPCIONES", true);
}

void Menu::setOptionsState() {
	Config &config = g_engine->config();
	Room *optionsMenu = g_engine->world().getRoomByName("MENUOPCIONES");
	scumm_assert(optionsMenu != nullptr);

	auto getSlideButton = [&] (const char *name) {
		SlideButton *slideButton = dynamic_cast<SlideButton *>(optionsMenu->getObjectByName(name));
		scumm_assert(slideButton != nullptr);
		return slideButton;
	};
	SlideButton
		*slideMusicVolume = getSlideButton("Slider Musica"),
		*slideSpeechVolume = getSlideButton("Slider Sonido");
	slideMusicVolume->value() = config.musicVolume() / 255.0f;
	slideSpeechVolume->value() = config.speechVolume() / 255.0f;

	if (!config.bits32())
		config.highQuality() = false;
	auto getCheckBox = [&] (const char *name) {
		CheckBox *checkBox = dynamic_cast<CheckBox *>(optionsMenu->getObjectByName(name));
		scumm_assert(checkBox != nullptr);
		return checkBox;
	};
	CheckBox
		*checkSubtitlesOn = getCheckBox("Boton ON"),
		*checkSubtitlesOff = getCheckBox("Boton OFF"),
		*check32Bits = getCheckBox("Boton 32 Bits"),
		*check16Bits = getCheckBox("Boton 16 Bits"),
		*checkHighQuality = getCheckBox("Boton Alta"),
		*checkLowQuality = getCheckBox("Boton Baja");
	checkSubtitlesOn->isChecked() = config.subtitles();
	checkSubtitlesOff->isChecked() = !config.subtitles();
	check32Bits->isChecked() = config.bits32();
	check16Bits->isChecked() = !config.bits32();
	checkHighQuality->isChecked() = config.highQuality();
	checkLowQuality->isChecked() = !config.highQuality();
	checkHighQuality->toggle(config.bits32());
}

void Menu::triggerOptionsAction(OptionsMenuAction action) {
	Config &config = g_engine->config();
	switch (action) {
	case OptionsMenuAction::SubtitlesOn:
		config.subtitles() = true;
		break;
	case OptionsMenuAction::SubtitlesOff:
		config.subtitles() = false;
		break;
	case OptionsMenuAction::HighQuality:
		config.highQuality() = true;
		break;
	case OptionsMenuAction::LowQuality:
		config.highQuality() = false;
		break;
	case OptionsMenuAction::Bits32:
		config.bits32() = true;
		config.highQuality() = true;
		break;
	case OptionsMenuAction::Bits16:
		config.bits32() = false;
		break;
	case OptionsMenuAction::MainMenu:
		continueMainMenu();
		break;
	default:
		warning("Unknown options menu action: %d", (int32)action);
		break;
	}
	setOptionsState();
}

void Menu::triggerOptionsValue(OptionsMenuValue valueId, float value) {
	Config &config = g_engine->config();
	switch (valueId) {
	case OptionsMenuValue::Music:
		config.musicVolume() = CLIP<uint8>((uint8)(value * 255), 0, 255);
		break;
	case OptionsMenuValue::Speech:
		config.speechVolume() = CLIP<uint8>((uint8)(value * 255), 0, 255);
		break;
	default:
		warning("Unknown options menu value: %d", (int32)valueId);
		break;
	}
	setOptionsState();
}

void Menu::continueMainMenu() {
	g_engine->config().saveToScummVM();
	g_engine->syncSoundSettings();
	g_engine->player().changeRoom(
		g_engine->player().isGameLoaded() ? "MENUPRINCIPAL" : "MENUPRINCIPALINICIO",
		true
	);
	// TODO: Update menu state and thumbanil
}

}
