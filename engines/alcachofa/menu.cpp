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

#include "common/config-manager.h"
#include "gui/message.h"
#include "graphics/thumbnail.h"

#include "alcachofa/alcachofa.h"
#include "alcachofa/metaengine.h"
#include "alcachofa/menu.h"
#include "alcachofa/player.h"
#include "alcachofa/script.h"

using namespace Common;
using namespace Graphics;

namespace Alcachofa {

static void createThumbnail(ManagedSurface &surface) {
	const auto size = g_engine->game().getThumbnailResolution();
	surface.create(size.x, size.y, g_engine->renderer().getPixelFormat());
}

static void convertToGrayscale(ManagedSurface &surface) {
	// TODO: Support other pixel formats
	assert(!surface.empty());
	assert(surface.format == PixelFormat::createFormatRGBA32());
	uint32 rgbMask = ~(uint32(0xff) << surface.format.aShift);

	for (int y = 0; y < surface.h; y++) {
		union {
			uint32 *pixel;
			uint8 *components;
		};
		pixel = (uint32 *)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; x++, pixel++) {
			*pixel &= rgbMask;
			byte gray = (byte)CLIP(0.29f * components[0] + 0.58f * components[1] + 0.11f * components[2], 0.0f, 255.0f);
			*pixel =
				(uint32(gray) << surface.format.rShift) |
				(uint32(gray) << surface.format.gShift) |
				(uint32(gray) << surface.format.bShift) |
				(uint32(0xff) << surface.format.aShift);
		}
	}
}

Menu *Menu::create() {
	if (g_engine->isV1())
		return new MenuV1();
	else if (g_engine->isV2())
		return new MenuV2();
	else if (g_engine->isV3())
		return new MenuV3();
	else
		error("Menu is not implemented for this engine version");
}

Menu::Menu()
	: _interactionSemaphore("menu")
	, _saveFileMgr(g_system->getSavefileManager()) {}

Menu::~Menu() {}

void Menu::resetAfterLoad() {
	_isOpen = false;
	_openAtNextFrame = false;
	_previousRoom = nullptr;
	_bigThumbnail.free();
	_selectedThumbnail.free();
}

void Menu::updateOpeningMenu() {
	if (!_openAtNextFrame) {
		if (g_engine->input().wasMenuKeyPressed() && g_engine->player().isAllowedToOpenMenu()) {
			_openAtNextFrame = true;
			createThumbnail(_bigThumbnail);
			g_engine->renderer().setOutput(*_bigThumbnail.surfacePtr());
		}
		return;
	}
	_openAtNextFrame = false;

	g_engine->sounds().pauseAll(true);
	_millisBeforeMenu = g_engine->getMillis();
	_previousRoom = g_engine->player().currentRoom();
	_isOpen = true;
	g_engine->player().changeRoom(g_engine->game().getMenuRoom(), true);
	_savefiles = _saveFileMgr->listSavefiles(g_engine->getSaveStatePattern());
	sort(_savefiles.begin(), _savefiles.end()); // the pattern ensures that the last file has the greatest slot
	_selectedSavefileI = _savefiles.size();
	updateSelectedSavefile(false);

	g_engine->player().heldItem() = nullptr;
	g_engine->scheduler().backupContext();
	g_engine->camera().onOpenMenu();
}

void MenuV1::updateOpeningMenu() {
	bool willOpen = _openAtNextFrame;
	Menu::updateOpeningMenu();
	if (willOpen)
		switchToState(MainMenuAction::ConfirmSavestate);
}

void MenuV2::updateOpeningMenu() {
	bool willOpen = _openAtNextFrame;
	Menu::updateOpeningMenu();
	if (willOpen)
		toggleMessageBox(false);
}

static int parseSavestateSlot(const String &filename) {
	if (filename.size() < 5) // minimal name would be "t.###"
		return 1;
	return atoi(filename.c_str() + filename.size() - 3);
}

void Menu::updateSelectedSavefile(bool hasJustSaved) {
	if (hasJustSaved) {
		// we just saved in-game so we also still have the correct thumbnail in memory
		_selectedThumbnail.copyFrom(_bigThumbnail);
	} else if (!isOnNewSlot()) {
		if (!tryReadOldSavefile()) {
			// an old savefile we could not read, use blank thumbnail and default description
			_selectedSavefileDescription = String::format("Savestate %d",
				parseSavestateSlot(_savefiles[_selectedSavefileI]));
			createThumbnail(_selectedThumbnail);
		}
	} else {
		// the unsaved gamestate is shown as grayscale
		_selectedThumbnail.copyFrom(_bigThumbnail);
		if (g_engine->isV3())
			convertToGrayscale(_selectedThumbnail);
	}

	ObjectBase *captureObject = g_engine->player().currentRoom()->getObjectByName("Capture");
	scumm_assert(captureObject);
	Graphic *captureGraphic = captureObject->graphic();
	scumm_assert(captureGraphic);
	captureGraphic->animation().overrideTexture(_selectedThumbnail);
}

void MenuV1::updateSelectedSavefile(bool hasJustSaved) {
	Menu::updateSelectedSavefile(hasJustSaved);

	ObjectBase *captureObject = g_engine->player().currentRoom()->getObjectByName("Capture");
	scumm_assert(captureObject);
	bool isInCorrectState = _currentState == MainMenuAction::Load || _currentState == MainMenuAction::Save;
	captureObject->toggle(isInCorrectState && !isOnNewSlot());
}

void MenuV2::updateSelectedSavefile(bool hasJustSaved) {
	Menu::updateSelectedSavefile(hasJustSaved);

	auto getButton = [ ] (const char *name) -> MenuButton &{
		return g_engine->player().currentRoom()->getRequiredObjectByName<MenuButton>(name);
	};

	getButton("CARGAR").isInteractable() = !isOnNewSlot();
	getButton("ANTERIOR").toggle(_selectedSavefileI > 0);
	getButton("SIGUIENTE").toggle(!isOnNewSlot());
}

void MenuV3::updateSelectedSavefile(bool hasJustSaved) {
	Menu::updateSelectedSavefile(hasJustSaved);

	auto getButton = [ ] (const char *name) -> MenuButton& {
		return g_engine->player().currentRoom()->getRequiredObjectByName<MenuButton>(name);
	};

	getButton("CARGAR").isInteractable() = !isOnNewSlot();
	getButton("ANTERIOR").toggle(_selectedSavefileI > 0);
	getButton("SIGUIENTE").toggle(!isOnNewSlot());
}

bool Menu::tryReadOldSavefile() {
	auto savefile = ScopedPtr<InSaveFile>(
		_saveFileMgr->openForLoading(_savefiles[_selectedSavefileI]));
	if (savefile == nullptr)
		return false;

	ExtendedSavegameHeader header;
	if (!g_engine->getMetaEngine()->readSavegameHeader(savefile.get(), &header, true))
		return false;
	_selectedSavefileDescription = header.description;

	MySerializer serializer(savefile.get(), nullptr);
	if (!serializer.syncVersion((Serializer::Version)kCurrentSaveVersion) ||
		!g_engine->syncThumbnail(serializer, &_selectedThumbnail))
		return false;

	return true;
}

void Menu::continueGame() {
	assert(_previousRoom != nullptr);
	_isOpen = false;
	_bigThumbnail.free();
	_selectedThumbnail.free();
	g_engine->input().nextFrame(); // presumably to clear all was* flags
	g_engine->player().changeRoom(_previousRoom->name(), true);
	g_engine->sounds().pauseAll(false);
	g_engine->camera().onCloseMenu();
	g_engine->scheduler().restoreContext();
	g_engine->setMillis(_millisBeforeMenu);
}

void Menu::triggerMainMenuAction(MainMenuAction action) {
	switch (action) {
	case MainMenuAction::ContinueGame:
		continueGame();
		break;
	case MainMenuAction::Save:
		triggerSave();
		break;
	case MainMenuAction::Load: {
		// we are in some update loop, let's load next frame upon event handling
		// that should be safer
		Event ev;
		ev.type = EVENT_CUSTOM_ENGINE_ACTION_START;
		ev.customType = (CustomEventType)EventAction::LoadFromMenu;
		g_system->getEventManager()->pushEvent(ev);
	}break;
	case MainMenuAction::InternetMenu: {
		GUI::MessageDialog dialog("Multiplayer is not implemented in this ScummVM version.");
		dialog.runModal();
	}break;
	case MainMenuAction::OptionsMenu:
		openOptionsMenu();
		break;
	case MainMenuAction::Exit:
	case MainMenuAction::AlsoExit:
		// implemented in AlcachofaEngine as it has its own event loop
		g_engine->fadeExit();
		break;
	case MainMenuAction::NewGame:
		// this action is unused just like the only room it would appear: MENUPRINCIPALINICIO
		// it also breaks the engine in very funny ways so let's not do anything instead
		// g_engine->script().createProcess(MainCharacterKind::None, g_engine->world().initScriptName());
		warning("MainMenuAction::NewGame triggered!");
		break;
	default:
		g_engine->game().unknownMenuAction((int32)action);
		break;
	}
}

void MenuV1::triggerMainMenuAction(MainMenuAction action) {
	auto updateMusicVolume = [&] (int delta) {
		auto &config = g_engine->config();
		int volume = config.musicVolume();
		volume = volume + (delta * Audio::Mixer::kMaxChannelVolume / 100);
		volume = CLIP(volume, 0, (int)Audio::Mixer::kMaxChannelVolume);
		config.musicVolume() = volume;
		ConfMan.setInt("music_volume", config.musicVolume());
		g_engine->syncSoundSettings();
		// we do not use saveToScummVM here to avoid frequent disk flushes
	};

	const uint maxSavegameI = _currentState == MainMenuAction::Load
		? _savefiles.size()
		: _savefiles.size() + 1; // the "new savegame" slot
	switch (action) {
	case MainMenuAction::Load:
		if (!isOnNewSlot())
			Menu::triggerMainMenuAction(action);
		break;
	case MainMenuAction::PrevSave:
		if (_currentState == MainMenuAction::OptionsMenu)
			updateMusicVolume(-10);
		else {
			_selectedSavefileI = _selectedSavefileI == 0 ? maxSavegameI - 1 : _selectedSavefileI - 1;
			updateSelectedSavefile(false);
		}
		break;
	case MainMenuAction::NextSave:
		if (_currentState == MainMenuAction::OptionsMenu)
			updateMusicVolume(+10);
		else {
			_selectedSavefileI = maxSavegameI == 0 ? 0 : (_selectedSavefileI + 1) % maxSavegameI;
			updateSelectedSavefile(false);
		}
		break;
	default:
		Menu::triggerMainMenuAction(action);
		break;
	}
}

void MenuV2::triggerMainMenuAction(MainMenuAction action) {
	switch (action) {
	case MainMenuAction::NextSave:
		if (_selectedSavefileI < _savefiles.size()) {
			_selectedSavefileI++;
			updateSelectedSavefile(false);
		}
		break;
	case MainMenuAction::PrevSave:
		if (_selectedSavefileI > 0) {
			_selectedSavefileI--;
			updateSelectedSavefile(false);
		}
		break;
	case MainMenuAction::Exit:
		toggleMessageBox(true);
		break;
	case MainMenuAction::Cancel:
		toggleMessageBox(false);
		break;
	case MainMenuAction::Accept:
		Menu::triggerMainMenuAction(MainMenuAction::Exit);
		break;
	default:
		Menu::triggerMainMenuAction(action);
		break;
	}
}

void MenuV3::triggerMainMenuAction(MainMenuAction action) {
	switch (action) {
	case MainMenuAction::NextSave:
		if (_selectedSavefileI < _savefiles.size()) {
			_selectedSavefileI++;
			updateSelectedSavefile(false);
		}
		break;
	case MainMenuAction::PrevSave:
		if (_selectedSavefileI > 0) {
			_selectedSavefileI--;
			updateSelectedSavefile(false);
		}
		break;
	default:
		Menu::triggerMainMenuAction(action);
		break;
	}
}

void Menu::triggerLoad() {
	auto *savefile = _saveFileMgr->openForLoading(_savefiles[_selectedSavefileI]);
	auto result = g_engine->loadGameStream(savefile);
	delete savefile;
	if (result.getCode() != kNoError) {
		GUI::MessageDialog dialog(result.getTranslatedDesc());
		dialog.runModal();
		return;
	}
}

void Menu::triggerSave() {
	String fileName;
	if (isOnNewSlot()) {
		// for a new savefile we figure out the next slot index
		int nextSlot = _savefiles.empty()
			? 1 // start at one to keep autosave alone
			: parseSavestateSlot(_savefiles.back()) + 1;
		fileName = g_engine->getSaveStateName(nextSlot);
		_selectedSavefileDescription = String::format("Savestate %d", nextSlot);
	} else
		fileName = _savefiles[_selectedSavefileI]; // overwrite a previous save

	Error error(kNoError);
	auto savefile = ScopedPtr<OutSaveFile>(_saveFileMgr->openForSaving(fileName));
	if (savefile == nullptr)
		error = Error(kReadingFailed);
	else
		error = g_engine->saveGameStream(savefile.get());
	if (error.getCode() == kNoError) {
		g_engine->getMetaEngine()->appendExtendedSave(savefile.get(), g_engine->getTotalPlayTime(), _selectedSavefileDescription, false);
		if (isOnNewSlot())
			_savefiles.push_back(fileName);
		updateSelectedSavefile(true);
	} else {
		GUI::MessageDialog dialog(error.getTranslatedDesc());
		dialog.runModal();
	}
}

void Menu::openOptionsMenu() {
	_currentSlideButton = nullptr;
	setOptionsState();
	g_engine->player().changeRoom("MENUOPCIONES", true);
}

void MenuV1::setOptionsState() {}

void MenuV2::setOptionsState() {
	Config &config = g_engine->config();
	Room *optionsMenu = g_engine->world().getRoomByName("MENUOPCIONES");
	scumm_assert(optionsMenu != nullptr);
	auto getSlideButton = [&] (const char *name) -> SlideButton& {
		return optionsMenu->getRequiredObjectByName<SlideButton>(name);
	};
	auto getPushButton = [&] (const char *name) -> PushButton& {
		return optionsMenu->getRequiredObjectByName<PushButton>(name);
	};

	// there is a mouse sensitivity slider, this does not exist in ScummVM, so we ignore it
	getSlideButton("VOLUMENCD").value() = config.musicVolume() / 255.0f;
	getSlideButton("VOLUMENAUDIO").value() = config.speechVolume() / 255.0f;

	getPushButton("CURSOR0").isChecked() = config.cursor() == 0;
	getPushButton("CURSOR1").isChecked() = config.cursor() == 1;
	getPushButton("CURSOR2").isChecked() = config.cursor() == 2;
	getPushButton("CURSOR3").isChecked() = config.cursor() == 3;
	getPushButton("TEXTOSON").isChecked() = config.subtitles();
	getPushButton("TEXTOSOFF").isChecked() = !config.subtitles();
}

void MenuV3::setOptionsState() {
	Config &config = g_engine->config();
	Room *optionsMenu = g_engine->world().getRoomByName("MENUOPCIONES");
	scumm_assert(optionsMenu != nullptr);
	auto getSlideButton = [&] (const char *name) -> SlideButton& {
		return optionsMenu->getRequiredObjectByName<SlideButton>(name);
	};
	auto getCheckBox = [&] (const char *name) -> CheckBox& {
		return optionsMenu->getRequiredObjectByName<CheckBox>(name);
	};

	getSlideButton("Slider Musica").value() = config.musicVolume() / 255.0f;
	getSlideButton("Slider Sonido").value() = config.speechVolume() / 255.0f;

	if (!config.bits32())
		config.highQuality() = false;
	getCheckBox("Boton ON").isChecked() = config.subtitles();
	getCheckBox("Boton OFF").isChecked() = !config.subtitles();
	getCheckBox("Boton 32 Bits").isChecked() = config.bits32();
	getCheckBox("Boton 16 Bits").isChecked() = !config.bits32();
	getCheckBox("Boton Alta").isChecked() = config.highQuality();
	getCheckBox("Boton Baja").isChecked() = !config.highQuality();
	getCheckBox("Boton Alta").toggle(config.bits32());
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
	case OptionsMenuAction::Cursor0:
		config.cursor() = 0;
		break;
	case OptionsMenuAction::Cursor1:
		config.cursor() = 1;
		break;
	case OptionsMenuAction::Cursor2:
		config.cursor() = 2;
		break;
	case OptionsMenuAction::Cursor3:
		config.cursor() = 3;
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
	case OptionsMenuValue::Sensitivity:
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
	g_engine->player().changeRoom(g_engine->game().getMenuRoom(), true);

	updateSelectedSavefile(false);
}

const Graphics::Surface *Menu::getBigThumbnail() const {
	return _bigThumbnail.empty() ? nullptr : &_bigThumbnail.rawSurface();
}

void MenuV1::switchToState(MainMenuAction state) {
	_currentState = state;

	if (state == MainMenuAction::Load) {
		if (isOnNewSlot()) {
			_selectedSavefileI = _savefiles.empty() ? 0 : _savefiles.size() - 1;
			updateSelectedSavefile(false);
		}
	} else if (state == MainMenuAction::Save) {
		_selectedSavefileI = _savefiles.size();
		updateSelectedSavefile(false);
	}
}

void MenuV2::toggleMessageBox(bool show) {
	auto getButton = [&] (const char *name) -> MenuButton& {
		return g_engine->player().currentRoom()->getRequiredObjectByName<MenuButton>(name);
	};

	getButton("MBACEPTAR").toggle(show);
	getButton("MBCANCELAR").toggle(show);

	getButton("ANTERIOR").toggle(!show);
	getButton("SIGUIENTE").toggle(!show);
	getButton("CARGAR").toggle(!show);
	getButton("GRABAR").toggle(!show);
	getButton("INTERNET").toggle(!show);
	getButton("OPCIONES").toggle(!show);
	getButton("JUGAR").toggle(!show);
	getButton("SALIR").toggle(!show);
}

}
