/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/cursor.h"
#include "engines/myst3/database.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/node.h"
#include "engines/myst3/scene.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "common/events.h"

#include "graphics/colormasks.h"

#include "gui/message.h"

namespace Myst3 {

Dialog::Dialog(Myst3Engine *vm, uint id):
	_vm(vm),
	_texture(0) {
	// Draw on the whole screen
	_isConstrainedToWindow = false;
	_scaled = !_vm->isWideScreenModEnabled();

	ResourceDescription countDesc = _vm->getFileDescription("DLGI", id, 0, Archive::kNumMetadata);
	ResourceDescription movieDesc = _vm->getFileDescription("DLOG", id, 0, Archive::kDialogMovie);
	if (!movieDesc.isValid()) {
		movieDesc = _vm->getFileDescription("DLOG", id, 0, Archive::kStillMovie);
	}

	if (!movieDesc.isValid() || !countDesc.isValid())
		error("Unable to load dialog %d", id);

	// Retrieve button count
	_buttonCount = countDesc.getMiscData(0);
	assert(_buttonCount <= 3);

	// Load the movie
	Common::SeekableReadStream *movieStream = movieDesc.getData();
	_bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	_bink.loadStream(movieStream);
	_bink.start();

	const Graphics::Surface *frame = _bink.decodeNextFrame();
	_texture = _vm->_gfx->createTexture(frame);

	_vm->_sound->playEffect(699, 10);
}

Dialog::~Dialog() {
	_vm->_gfx->freeTexture(_texture);
}

void Dialog::draw() {
	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);
	_vm->_gfx->drawTexturedRect2D(getPosition(), textureRect, _texture);
}

Common::Rect Dialog::getPosition() const {
	Common::Rect viewport;
	if (_scaled) {
		viewport = Common::Rect(Renderer::kOriginalWidth, Renderer::kOriginalHeight);
	} else {
		viewport = _vm->_gfx->viewport();
	}

	Common::Rect screenRect = Common::Rect(_texture->width, _texture->height);
	screenRect.translate((viewport.width() - _texture->width) / 2,
			(viewport.height() - _texture->height) / 2);
	return screenRect;
}

ButtonsDialog::ButtonsDialog(Myst3Engine *vm, uint id):
	Dialog(vm, id),
	_frameToDisplay(0),
	_previousframe(0) {

	loadButtons();
}

ButtonsDialog::~ButtonsDialog() {
}

void ButtonsDialog::loadButtons() {
	ResourceDescription buttonsDesc = _vm->getFileDescription("DLGB", 1000, 0, Archive::kNumMetadata);

	if (!buttonsDesc.isValid())
		error("Unable to load dialog buttons description");

	for (uint i = 0; i < 3; i++) {
		uint32 left = buttonsDesc.getMiscData(i * 4);
		uint32 top = buttonsDesc.getMiscData(i * 4 + 1);
		uint32 width = buttonsDesc.getMiscData(i * 4 + 2);
		uint32 height = buttonsDesc.getMiscData(i * 4 + 3);
		_buttons[i] = Common::Rect(width, height);
		_buttons[i].translate(left, top);
	}
}

void ButtonsDialog::draw() {
	if (_frameToDisplay != _previousframe) {
		_bink.seekToFrame(_frameToDisplay);

		const Graphics::Surface *frame = _bink.decodeNextFrame();
		_texture->update(frame);

		_previousframe = _frameToDisplay;
	}

	Dialog::draw();
}

int16 ButtonsDialog::update() {
	// Process events
	Common::Event event;
	while (_vm->getEventManager()->pollEvent(event)) {
		_vm->processEventForKeyboardState(event);

		if (event.type == Common::EVENT_MOUSEMOVE) {
			// Compute local mouse coordinates
			_vm->_cursor->updatePosition(event.mouse);
			Common::Point localMouse = getRelativeMousePosition();

			// No hovered button
			_frameToDisplay = 0;

			// Display the frame corresponding to the hovered button
			for (uint i = 0; i < _buttonCount; i++) {
				if (_buttons[i].contains(localMouse)) {
					_frameToDisplay = i + 1;
					break;
				}
			}
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			if (_frameToDisplay > 0) {
				return _frameToDisplay;
			} else {
				_vm->_sound->playEffect(697, 5);
			}
		} else if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				return _buttonCount;
			default:
				break;
			}
		}
	}

	return -2;
}

Common::Point ButtonsDialog::getRelativeMousePosition() const {
	Common::Rect position = getPosition();
	Common::Point localMouse =_vm->_cursor->getPosition(_scaled);
	localMouse.x -= position.left;
	localMouse.y -= position.top;
	return localMouse;
}

GamepadDialog::GamepadDialog(Myst3Engine *vm, uint id):
	Dialog(vm, id) {
}

GamepadDialog::~GamepadDialog() {
}

int16 GamepadDialog::update() {
	// Process events
	Common::Event event;
	while (_vm->getEventManager()->pollEvent(event)) {
		_vm->processEventForKeyboardState(event);
		_vm->processEventForGamepad(event);

		if (event.type == Common::EVENT_MOUSEMOVE) {
			// Compute local mouse coordinates
			_vm->_cursor->updatePosition(event.mouse);
		} else if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				return 1;
			case Common::KEYCODE_ESCAPE:
				if (_buttonCount == 2) {
					return 2;
				} else {
					return 1;
				}
			default:
				break;
			}
		}
	}

	return -2;
}

Menu::Menu(Myst3Engine *vm) :
		_vm(vm),
		_saveLoadSpotItem(0) {
}

Menu::~Menu() {
}

void Menu::updateMainMenu(uint16 action) {
	switch (action) {
	case 1: {
			// New game
			int16 choice = dialogConfirmValue();

			// If a game is playing, ask if wants to save
			if (_vm->_state->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(dialogIdFromType(kConfirmNewGame));
			}

			if (choice == dialogSaveValue()) {
				// Go to save screen
				_vm->_state->setMenuSaveBack(1);
				_vm->_state->setMenuSaveAction(6);
				goToNode(kNodeMenuSaveGame);
			} else if (choice == dialogConfirmValue()) {
				// New game
				goToNode(kNodeMenuNewGame);
			}
		}
		break;
	case 2: {
			// Load game
			int16 choice = dialogConfirmValue();

			// If a game is playing, ask if wants to save
			if (_vm->_state->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(dialogIdFromType(kConfirmLoadGame));
			}

			if (choice == dialogSaveValue()) {
				// Go to save screen
				_vm->_state->setMenuSaveBack(1);
				_vm->_state->setMenuSaveAction(3);
				goToNode(kNodeMenuSaveGame);
			} else if (choice == dialogConfirmValue()) {
				// Load game screen
				_vm->_state->setMenuLoadBack(1);
				goToNode(kNodeMenuLoadGame);
			}
		}
		break;
	case 3:
		// Go to save screen
		_vm->_state->setMenuSaveBack(1);
		_vm->_state->setMenuSaveAction(1);
		goToNode(kNodeMenuSaveGame);
		break;
	case 4:
		// Settings
		_vm->_state->setMenuOptionsBack(1);
		_vm->runScriptsFromNode(599, 0, 0);
		break;
	case 5: {
			// Asked to quit
			int16 choice = dialogConfirmValue();

			// If a game is playing, ask if wants to save
			if (_vm->_state->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(dialogIdFromType(kConfirmQuit));
			}

			if (choice == dialogSaveValue()) {
				// Go to save screen
				_vm->_state->setMenuSaveBack(1);
				_vm->_state->setMenuSaveAction(5);
				goToNode(kNodeMenuSaveGame);
			} else if (choice == dialogConfirmValue()) {
				// Quit
				_vm->quitGame();
			}
		}
		break;
	default:
		warning("Menu action %d is not implemented", action);
		break;
	}
}

Graphics::Surface *Menu::captureThumbnail() {
	Graphics::Surface *big = _vm->_gfx->getScreenshot();
	Graphics::Surface *thumbnail = createThumbnail(big);
	big->free();
	delete big;

	return thumbnail;
}

void Menu::goToNode(uint16 node) {
	if (_vm->_state->getMenuSavedAge() == 0 && _vm->_state->getLocationRoom() != kRoomMenu) {
		// Entering menu, save current location ...
		_vm->_state->setMenuSavedAge(_vm->_state->getLocationAge());
		_vm->_state->setMenuSavedRoom(_vm->_state->getLocationRoom());
		_vm->_state->setMenuSavedNode(_vm->_state->getLocationNode());

		// ... and capture the screen
		Graphics::Surface *thumbnail = captureThumbnail();
		_saveThumbnail.reset(thumbnail);

		// Reset some sound variables
		if (_vm->_state->getLocationAge() == 6 && _vm->_state->getSoundEdannaUnk587() == 1 && _vm->_state->getSoundEdannaUnk1031()) {
			_vm->_state->setSoundEdannaUnk587(0);
		}
		if (_vm->_state->getLocationAge() == 10 && _vm->_state->getSoundAmateriaUnk627() == 1 && _vm->_state->getSoundAmateriaUnk930()){
			_vm->_state->setSoundAmateriaUnk627(0);
		}
		if (_vm->_state->getLocationAge() == 7 && _vm->_state->getSoundVoltaicUnk540() == 1 && _vm->_state->getSoundVoltaicUnk1146()) {
			_vm->_state->setSoundVoltaicUnk540(0);
		}

		_vm->_sound->stopMusic(60);
		_vm->_state->setSoundScriptsSuspended(1);
	}

	if (_vm->_state->hasVarMenuEscapePressed()) {
		// This variable does not exist in the Xbox version
		_vm->_state->setMenuEscapePressed(0);
	}

	_vm->_state->setLocationNextAge(9);
	_vm->_state->setLocationNextRoom(kRoomMenu);
	_vm->goToNode(node, kTransitionNone);
}

uint Menu::dialogIdFromType(DialogType type) {
	struct {
		DialogType type;
		uint id;
		uint idXbox;
	} mapping[] = {
			{ kConfirmNewGame,        1080, 1010 },
			{ kConfirmLoadGame,       1060, 1003 },
			{ kConfirmOverwrite,      1040, 1004 },
			{ kConfirmEraseSavedGame, 1020, 0 },
			{ kErrorEraseSavedGame,   1050, 0 },
			{ kConfirmQuit,           1070, 0 }
	};

	uint id = 0;

	for (uint i = 0; i < ARRAYSIZE(mapping); i++) {
		if (mapping[i].type == type) {
			if (_vm->getPlatform() == Common::kPlatformXbox) {
				id = mapping[i].idXbox;
			} else {
				id = mapping[i].id;
			}
		}
	}

	if (id == 0) {
		error("No id for dialog %d", type);
	}

	return id;
}

uint16 Menu::dialogConfirmValue() {
	if (_vm->getPlatform() == Common::kPlatformXbox) {
		return 1;
	}

	return 2;
}

uint16 Menu::dialogSaveValue() {
	if (_vm->getPlatform() == Common::kPlatformXbox) {
		return 999; // No save value
	}

	return 1;
}

Common::String Menu::getAgeLabel(GameState *gameState) {
	uint32 age = 0;
	uint32 room = gameState->getLocationRoom();
	if (room == kRoomMenu)
		age = gameState->getMenuSavedAge();
	else
		age = gameState->getLocationAge();

	// Look for the age name
	ResourceDescription desc = _vm->getFileDescription("AGES", 1000, 0, Archive::kTextMetadata);

	if (!desc.isValid())
		error("Unable to load age descriptions.");

	Common::String label = desc.getTextData(_vm->_db->getAgeLabelId(age));
	label.toUppercase();

	return label;
}

Graphics::Surface *Menu::createThumbnail(Graphics::Surface *big) {
	assert(big->format == Texture::getRGBAPixelFormat());

	Graphics::Surface *small = new Graphics::Surface();
	small->create(GameState::kThumbnailWidth, GameState::kThumbnailHeight, Texture::getRGBAPixelFormat());

	// The portion of the screenshot to keep
	Common::Rect frame = _vm->_scene->getPosition();
	Graphics::Surface frameSurface = big->getSubArea(frame);

	uint32 *dst = (uint32 *)small->getPixels();
	for (uint i = 0; i < small->h; i++) {
		for (uint j = 0; j < small->w; j++) {
			uint32 srcX = frameSurface.w * j / small->w;
			uint32 srcY = frameSurface.h * i / small->h;
			uint32 *src = (uint32 *)frameSurface.getBasePtr(srcX, srcY);

			// Copy RGBA pixel
			*dst++ = *src;
		}
	}

	return small;
}

void Menu::setSaveLoadSpotItem(uint16 id, SpotItemFace *spotItem) {
	if (id == 1) {
		_saveLoadSpotItem = spotItem;
	}
}

bool Menu::isOpen() const {
	return _vm->_state->getLocationAge() == 9 && _vm->_state->getLocationRoom() == kRoomMenu;
}

void Menu::generateSaveThumbnail() {
	_saveThumbnail.reset(captureThumbnail());
}

Graphics::Surface *Menu::borrowSaveThumbnail() {
	return _saveThumbnail.get();
}

PagingMenu::PagingMenu(Myst3Engine *vm) :
		Menu(vm),
		_saveDrawCaret(false),
		_saveCaretCounter(0) {
}

PagingMenu::~PagingMenu() {
}

void PagingMenu::saveLoadAction(uint16 action, uint16 item) {
	switch (action) {
	case 0:
		loadMenuOpen();
		break;
	case 1:
		loadMenuSelect(item);
		break;
	case 2:
		loadMenuLoad();
		break;
	case 3:
		saveMenuOpen();
		break;
	case 4:
		saveMenuSelect(item);
		break;
	case 5:
		saveMenuSave();
		break;
	case 6:
		loadMenuChangePage();
		break;
	case 7:
		saveMenuChangePage();
		break;
	case 8:
		saveLoadErase();
		break;
	default:
		warning("Save load menu action %d for item %d is not implemented", action, item);
	}
}

void PagingMenu::loadMenuOpen() {
	_saveLoadFiles = Saves::list(_vm->getSaveFileManager(), _vm->getPlatform());

	_vm->_state->setMenuSaveLoadCurrentPage(0);
	saveLoadUpdateVars();
}

void PagingMenu::saveLoadUpdateVars() {
	int16 page = _vm->_state->getMenuSaveLoadCurrentPage();

	// Go back one page if the last element of the last page was removed
	if (page && (7 * page > (int)_saveLoadFiles.size() - 1))
		page--;
	_vm->_state->setMenuSaveLoadCurrentPage(page);

	// Set up pagination
	bool canGoLeft = (_saveLoadFiles.size() > 7) && page;
	bool canGoRight = (_saveLoadFiles.size() > 7) && (7 * (page + 1) < (int)_saveLoadFiles.size());

	_vm->_state->setMenuSaveLoadPageLeft(canGoLeft);
	_vm->_state->setMenuSaveLoadPageRight(canGoRight);
	_vm->_state->setMenuSaveLoadSelectedItem(-1);

	// Enable items
	uint16 itemsOnPage = _saveLoadFiles.size() % 7;

	if (itemsOnPage == 0 && _saveLoadFiles.size() != 0)
		itemsOnPage = 7;
	if (canGoRight)
		itemsOnPage = 7;

	for (uint i = 0; i < 7; i++)
		_vm->_state->setVar(1354 + i, i < itemsOnPage);
}

void PagingMenu::loadMenuSelect(uint16 item) {
	// Selecting twice the same item loads it
	if (item == _vm->_state->getMenuSaveLoadSelectedItem()) {
		loadMenuLoad();
		return;
	}

	_vm->_state->setMenuSaveLoadSelectedItem(item);
	int16 page = _vm->_state->getMenuSaveLoadCurrentPage();

	uint16 index = page * 7 + item;

	assert(index < _saveLoadFiles.size());
	Common::String filename = _saveLoadFiles[index];
	Common::InSaveFile *saveFile = _vm->getSaveFileManager()->openForLoading(filename);
	if (!saveFile) {
		warning("Unable to open save '%s'", filename.c_str());
		return;
	}

	// Extract the age to load from the savegame
	GameState gameState = GameState(_vm->getPlatform(), _vm->_db);
	gameState.load(saveFile);

	// Update the age name
	_saveLoadAgeName = getAgeLabel(&gameState);

	// Update the save thumbnail
	if (_saveLoadSpotItem) {
		Graphics::Surface *thumbnail = GameState::readThumbnail(saveFile);
		_saveLoadSpotItem->updateData(thumbnail);
		thumbnail->free();
		delete thumbnail;
	}

	delete saveFile;
}

void PagingMenu::loadMenuLoad() {
	uint16 item = _vm->_state->getMenuSaveLoadSelectedItem();
	int16 page = _vm->_state->getMenuSaveLoadCurrentPage();

	uint16 index = page * 7 + item;
	assert(index < _saveLoadFiles.size());

	Common::Error loadError = _vm->loadGameState(_saveLoadFiles[index], kTransitionFade);
	if (loadError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(loadError.getDesc());
		dialog.runModal();
	}
}

void PagingMenu::saveMenuOpen() {
	_saveLoadFiles = Saves::list(_vm->getSaveFileManager(), _vm->getPlatform());

	_saveLoadAgeName = getAgeLabel(_vm->_state);
	_saveCaretCounter = kCaretSpeed;

	_vm->_state->setMenuSaveLoadCurrentPage(0);
	saveLoadUpdateVars();

	// Update the thumbnail to display
	if (_saveLoadSpotItem && _saveThumbnail)
		_saveLoadSpotItem->updateData(_saveThumbnail.get());
}

void PagingMenu::saveMenuSelect(uint16 item) {
	_vm->_state->setMenuSaveLoadSelectedItem(item);

	if (item != 7) {
		int16 page = _vm->_state->getMenuSaveLoadCurrentPage();

		uint16 index = page * 7 + item;

		assert(index < _saveLoadFiles.size());
		_saveName = _saveLoadFiles[index];
	}
}

void PagingMenu::saveMenuChangePage() {
	saveLoadUpdateVars();
	_vm->_state->setMenuSaveLoadSelectedItem(7);
}

void PagingMenu::saveMenuSave() {
	if (_saveName.empty())
		return;

	Common::String fileName = _saveName;
	if (!fileName.hasSuffixIgnoreCase(".M3S"))
		fileName += ".M3S";

	// Check if file exists
	bool fileExists = false;
	for (uint i = 0; i < _saveLoadFiles.size(); i++) {
		if (_saveLoadFiles[i].equalsIgnoreCase(fileName)) {
			fileExists = true;
			break;
		}
	}

	// Ask the user if he wants to overwrite the existing save
	if (fileExists && _vm->openDialog(dialogIdFromType(kConfirmOverwrite)) != 1)
		return;

	Common::Error saveError = _vm->saveGameState(_saveName, _saveThumbnail.get(), false);
	if (saveError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(saveError.getDesc());
		dialog.runModal();
	}

	// Do next action
	_vm->_state->setMenuNextAction(_vm->_state->getMenuSaveAction());
	_vm->runScriptsFromNode(88);
}

void PagingMenu::saveLoadErase() {
	uint16 node = _vm->_state->getLocationNode();
	uint16 item = _vm->_state->getMenuSaveLoadSelectedItem();
	int16 page = _vm->_state->getMenuSaveLoadCurrentPage();

	uint16 index = page * 7 + item;
	assert(index < _saveLoadFiles.size());

	// Confirm dialog
	if (_vm->openDialog(dialogIdFromType(kConfirmEraseSavedGame)) != 1)
		return;

	// Delete the file
	if (!_vm->getSaveFileManager()->removeSavefile(_saveLoadFiles[index]))
		_vm->openDialog(dialogIdFromType(kErrorEraseSavedGame)); // Error dialog

	_saveLoadFiles = Saves::list(_vm->getSaveFileManager(), _vm->getPlatform());

	saveLoadUpdateVars();

	// Load menu specific
	if (node == kNodeMenuLoadGame && _saveLoadSpotItem) {
		_saveLoadSpotItem->clear();
		_saveLoadAgeName.clear();
	}

	// Save menu specific
	if (node == kNodeMenuSaveGame)
		_vm->_state->setMenuSaveLoadSelectedItem(7);
}

void PagingMenu::draw() {
	uint16 node = _vm->_state->getLocationNode();
	uint16 room = _vm->_state->getLocationRoom();
	uint16 age = _vm->_state->getLocationAge();

	if (room != kRoomMenu || !(node == kNodeMenuLoadGame || node == kNodeMenuSaveGame))
		return;

	int16 page = _vm->_state->getMenuSaveLoadCurrentPage();
	NodePtr nodeData = _vm->_db->getNodeData(node, room, age);

	for (uint i = 0; i < 7; i++) {
		uint itemToDisplay = page * 7 + i;

		if (itemToDisplay >= _saveLoadFiles.size())
			break;

		PolarRect rect = nodeData->hotspots[i + 1].rects[0];

		Common::String display = prepareSaveNameForDisplay(_saveLoadFiles[itemToDisplay]);
		_vm->_gfx->draw2DText(display, Common::Point(rect.centerPitch, rect.centerHeading));
	}

	if (!_saveLoadAgeName.empty()) {
		PolarRect rect = nodeData->hotspots[8].rects[0];
		_vm->_gfx->draw2DText(_saveLoadAgeName, Common::Point(rect.centerPitch, rect.centerHeading));
	}

	// Save screen specific
	if (node == kNodeMenuSaveGame) {
		uint16 item = _vm->_state->getMenuSaveLoadSelectedItem();
		Common::String display = prepareSaveNameForDisplay(_saveName);

		if (item == 7) {
			_saveCaretCounter--;
			if (_saveCaretCounter < 0) {
				_saveCaretCounter = kCaretSpeed;
				_saveDrawCaret = !_saveDrawCaret;
			}

			if (_saveDrawCaret) {
				display += '|';
			}
		}

		PolarRect rect = nodeData->hotspots[9].rects[0];
		_vm->_gfx->draw2DText(display, Common::Point(rect.centerPitch, rect.centerHeading));
	}
}

bool PagingMenu::handleInput(const Common::KeyState &e) {
	uint16 node = _vm->_state->getLocationNode();
	uint16 room = _vm->_state->getLocationRoom();
	uint16 item = _vm->_state->getMenuSaveLoadSelectedItem();

	if (room != kRoomMenu || node != kNodeMenuSaveGame || item != 7)
		return false;

	Common::String display = prepareSaveNameForDisplay(_saveName);

	if (e.keycode == Common::KEYCODE_BACKSPACE
			|| e.keycode == Common::KEYCODE_DELETE) {
		display.deleteLastChar();
		_saveName = display;
		return true;
	} else if (e.keycode == Common::KEYCODE_RETURN
			|| e.keycode == Common::KEYCODE_KP_ENTER) {
		saveMenuSave();
		return true;
	}

	if (((e.ascii >= 'a' && e.ascii <= 'z')
			|| (e.ascii >= 'A' && e.ascii <= 'Z')
			|| (e.ascii >= '0' && e.ascii <= '9')
			|| e.ascii == ' ')
			&& (display.size() < 17)) {
		display += e.ascii;
		display.toUppercase();
		_saveName = display;

		return true;
	}

	return false;
}

void PagingMenu::loadMenuChangePage() {
	saveLoadUpdateVars();
}

Common::String PagingMenu::prepareSaveNameForDisplay(const Common::String &name) {
	Common::String display = name;
	display.toUppercase();
	if (display.hasSuffixIgnoreCase(".M3S")) {
		display.deleteLastChar();
		display.deleteLastChar();
		display.deleteLastChar();
		display.deleteLastChar();
	}

	while (display.size() > 17)
		display.deleteLastChar();

	return display;
}

AlbumMenu::AlbumMenu(Myst3Engine *vm) :
		Menu(vm) {
}

AlbumMenu::~AlbumMenu() {
}

void AlbumMenu::draw() {
	uint16 node = _vm->_state->getLocationNode();
	uint16 room = _vm->_state->getLocationRoom();

	// Load and save menus only
	if (room != kRoomMenu || !(node == kNodeMenuLoadGame || node == kNodeMenuSaveGame))
		return;

	if (!_saveLoadAgeName.empty()) {
		Common::Point p(184 - (13 * _saveLoadAgeName.size()) / 2, 305);
		_vm->_gfx->draw2DText(_saveLoadAgeName, p);
	}

	if (!_saveLoadTime.empty()) {
		Common::Point p(184 - (13 * _saveLoadTime.size()) / 2, 323);
		_vm->_gfx->draw2DText(_saveLoadTime, p);
	}
}

bool AlbumMenu::handleInput(const Common::KeyState &e) {
	return false;
}

void AlbumMenu::saveLoadAction(uint16 action, uint16 item) {
	switch (action) {
	case 0:
		loadMenuOpen();
		break;
	case 1:
		loadMenuSelect();
		break;
	case 2:
		loadMenuLoad();
		break;
	case 3:
		saveMenuOpen();
		break;
	case 4:
		saveMenuSave();
		break;
	case 5:
		setSavesAvailable();
		break;
	default:
		warning("Save load menu action %d for item %d is not implemented", action, item);
	}
}

Common::String AlbumMenu::getSaveNameTemplate() {
	ResourceDescription saveNameDesc = _vm->getFileDescription("SAVE", 1000, 0, Archive::kTextMetadata);
	return saveNameDesc.getTextData(0); // "EXILE Saved Game %d"
}

Common::HashMap<int, Common::String> AlbumMenu::listSaveFiles() {
	Common::StringArray saveFiles = _vm->getSaveFileManager()->listSavefiles("*.m3x");
	Common::String fileNameTemplate = Common::String::format("%s.m3x", getSaveNameTemplate().c_str());

	Common::HashMap<int, Common::String> filteredSaveFiles;
	for (uint i = 0; i < 10; i++) {
		Common::String fileName = Common::String::format(fileNameTemplate.c_str(), i);

		for (uint j = 0; j < saveFiles.size(); j++) {
			if (saveFiles[j].equalsIgnoreCase(fileName)) {
				filteredSaveFiles.setVal(i, saveFiles[j]);
				break;
			}
		}
	}

	return filteredSaveFiles;
}

void AlbumMenu::loadSaves() {
	Common::HashMap<int, Common::String> saveFiles = listSaveFiles();
	for (uint i = 0; i < 10; i++) {
		// Skip empty slots
		if (!saveFiles.contains(i)) continue;

		// Open save
		Common::InSaveFile *saveFile = _vm->getSaveFileManager()->openForLoading(saveFiles[i]);
		if (!saveFile) {
			warning("Failed to open save %s for reading.", saveFiles[i].c_str());
			continue;
		}

		// Read state data
		Common::Serializer s = Common::Serializer(saveFile, 0);
		GameState::StateData data;
		data.syncWithSaveGame(s);

		if (_albumSpotItems.contains(i)) {
			// Read and resize the thumbnail
			Graphics::Surface *saveThumb = GameState::readThumbnail(saveFile);
			Graphics::Surface *miniThumb = GameState::resizeThumbnail(saveThumb, kAlbumThumbnailWidth, kAlbumThumbnailHeight);
			saveThumb->free();
			delete saveThumb;

			SpotItemFace *spotItem = _albumSpotItems.getVal(i);
			spotItem->updateData(miniThumb);

			miniThumb->free();
			delete miniThumb;
		}

		delete saveFile;
	}
}

void AlbumMenu::loadMenuOpen() {
	_saveLoadAgeName = "";
	_saveLoadTime = "";

	loadSaves();
}

void AlbumMenu::loadMenuSelect() {
	uint16 node = _vm->_state->getLocationNode();
	uint16 room = _vm->_state->getLocationRoom();

	// Details are only updated on the load menu
	if (room != kRoomMenu || node != kNodeMenuLoadGame)
		return;

	int32 selectedSave = _vm->_state->getMenuSelectedSave();
	Common::HashMap<int, Common::String> saveFiles = listSaveFiles();

	if (!saveFiles.contains(selectedSave)) {
		// No save in the selected slot
		_saveLoadAgeName = "";
		_saveLoadTime = "";
		_saveLoadSpotItem->initBlack(GameState::kThumbnailWidth, GameState::kThumbnailHeight);
		return;
	}

	Common::String filename = saveFiles[selectedSave];
	Common::InSaveFile *saveFile = _vm->getSaveFileManager()->openForLoading(filename);
	if (!saveFile) {
		warning("Unable to open save '%s'", filename.c_str());
		return;
	}

	// Extract the age to load from the savegame
	GameState *gameState = new GameState(_vm->getPlatform(), _vm->_db);
	gameState->load(saveFile);

	// Update the age name and save date
	_saveLoadAgeName = getAgeLabel(gameState);
	_saveLoadTime = gameState->formatSaveTime();

	// Update the save thumbnail
	if (_saveLoadSpotItem) {
		Graphics::Surface *thumbnail = GameState::readThumbnail(saveFile);
		_saveLoadSpotItem->updateData(thumbnail);
		thumbnail->free();
		delete thumbnail;
	}

	delete gameState;
}

void AlbumMenu::loadMenuLoad() {
	int32 selectedSave = _vm->_state->getMenuSelectedSave();
	Common::HashMap<int, Common::String> saveFiles = listSaveFiles();

	if (!saveFiles.contains(selectedSave)) {
		return; // No save to load, do nothing
	}

	Common::Error loadError = _vm->loadGameState(saveFiles[selectedSave], kTransitionFade);
	if (loadError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(loadError.getDesc());
		dialog.runModal();
	}
}

void AlbumMenu::saveMenuOpen() {
	loadSaves();

	_saveLoadAgeName = getAgeLabel(_vm->_state);
	_saveLoadTime = "";

	// Update the thumbnail to display
	if (_saveLoadSpotItem && _saveThumbnail)
		_saveLoadSpotItem->updateData(_saveThumbnail.get());
}

void AlbumMenu::saveMenuSave() {
	int32 selectedSave = _vm->_state->getMenuSelectedSave();

	Common::String saveNameTemplate = getSaveNameTemplate();
	Common::String saveName = Common::String::format(saveNameTemplate.c_str(), selectedSave);

	// Ask the user if he wants to overwrite the existing save
	Common::HashMap<int, Common::String> saveFiles = listSaveFiles();
	if (saveFiles.contains(selectedSave) && _vm->openDialog(dialogIdFromType(kConfirmOverwrite)) != 1)
		return;

	Common::Error saveError = _vm->saveGameState(saveName, _saveThumbnail.get(), false);
	if (saveError.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(saveError.getDesc());
		dialog.runModal();
	}

	// Do next action
	_vm->_state->setMenuNextAction(_vm->_state->getMenuSaveAction());
	_vm->runScriptsFromNode(88);
}

void AlbumMenu::setSavesAvailable() {
	Common::HashMap<int, Common::String> saveFiles = listSaveFiles();
	_vm->_state->setMenuSavesAvailable(!saveFiles.empty());
}

void AlbumMenu::setSaveLoadSpotItem(uint16 id, SpotItemFace *spotItem) {
	if (id % 100 == 2) {
		_albumSpotItems.setVal(id / 100, spotItem);
	} else {
		Menu::setSaveLoadSpotItem(id, spotItem);
	}
}

} // End of namespace Myst3
