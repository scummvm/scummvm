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
	surface.create(kBigThumbnailWidth, kBigThumbnailHeight, PixelFormat::createFormatRGBA32());
}

static void convertToGrayscale(ManagedSurface &surface) {
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

Menu::Menu()
	: _interactionSemaphore("menu")
	, _saveFileMgr(g_system->getSavefileManager()) {}

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
	g_engine->player().changeRoom("MENUPRINCIPAL", true);
	_savefiles = _saveFileMgr->listSavefiles(g_engine->getSaveStatePattern());
	sort(_savefiles.begin(), _savefiles.end()); // the pattern ensures that the last file has the greatest slot
	_selectedSavefileI = _savefiles.size();
	updateSelectedSavefile(false);

	g_engine->player().heldItem() = nullptr;
	g_engine->scheduler().backupContext();
	g_engine->camera().backup(1);
	g_engine->camera().setPosition(Math::Vector3d(
		g_system->getWidth() / 2.0f, g_system->getHeight() / 2.0f, 0.0f));
}

static int parseSavestateSlot(const String &filename) {
	if (filename.size() < 5) // minimal name would be "t.###"
		return 1;
	return atoi(filename.c_str() + filename.size() - 3);
}

void Menu::updateSelectedSavefile(bool hasJustSaved) {
	auto getButton = [] (const char *name) {
		MenuButton *button = dynamic_cast<MenuButton *>(g_engine->player().currentRoom()->getObjectByName(name));
		scumm_assert(button != nullptr);
		return button;
	};

	bool isOldSavefile = _selectedSavefileI < _savefiles.size();
	getButton("CARGAR")->isInteractable() = isOldSavefile;
	getButton("ANTERIOR")->toggle(_selectedSavefileI > 0);
	getButton("SIGUIENTE")->toggle(isOldSavefile);

	if (hasJustSaved) {
		// we just saved in-game so we also still have the correct thumbnail in memory
		_selectedThumbnail.copyFrom(_bigThumbnail);
	} else if (isOldSavefile) {
		if (!tryReadOldSavefile()) {
			_selectedSavefileDescription = String::format("Savestate %d",
				parseSavestateSlot(_savefiles[_selectedSavefileI]));
			createThumbnail(_selectedThumbnail);
		}
	} else {
		// the unsaved gamestate is shown as grayscale
		_selectedThumbnail.copyFrom(_bigThumbnail);
		convertToGrayscale(_selectedThumbnail);
	}

	ObjectBase *captureObject = g_engine->player().currentRoom()->getObjectByName("Capture");
	scumm_assert(captureObject);
	Graphic *captureGraphic = captureObject->graphic();
	scumm_assert(captureGraphic);
	captureGraphic->animation().overrideTexture(_selectedThumbnail);
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
			updateSelectedSavefile(false);
		}
		break;
	case MainMenuAction::PrevSave:
		if (_selectedSavefileI > 0) {
			_selectedSavefileI--;
			updateSelectedSavefile(false);
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
	if (_selectedSavefileI < _savefiles.size()) {
		fileName = _savefiles[_selectedSavefileI]; // overwrite a previous save
	} else {
		// for a new savefile we figure out the next slot index
		int nextSlot = _savefiles.empty()
			? 1 // start at one to keep autosave alone
			: parseSavestateSlot(_savefiles.back()) + 1;
		fileName = g_engine->getSaveStateName(nextSlot);
		_selectedSavefileDescription = String::format("Savestate %d", nextSlot);
	}

	Error error(kNoError);
	auto savefile = ScopedPtr<OutSaveFile>(_saveFileMgr->openForSaving(fileName));
	if (savefile == nullptr)
		error = Error(kReadingFailed);
	else
		error = g_engine->saveGameStream(savefile.get());
	if (error.getCode() == kNoError) {
		g_engine->getMetaEngine()->appendExtendedSave(savefile.get(), g_engine->getTotalPlayTime(), _selectedSavefileDescription, false);
		_savefiles.push_back(fileName);
		updateSelectedSavefile(true);
	} else {
		GUI::MessageDialog dialog(error.getTranslatedDesc());
		dialog.runModal();
	}
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

	updateSelectedSavefile(false);
}

const Graphics::Surface *Menu::getBigThumbnail() const {
	return _bigThumbnail.empty() ? nullptr : &_bigThumbnail.rawSurface();
}

}
