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

#ifndef MENU_H_
#define MENU_H_

#include "engines/myst3/gfx.h"

#include "common/events.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/str-array.h"

#include "video/bink_decoder.h"

namespace Myst3 {

class Myst3Engine;
class SpotItemFace;
class GameState;

enum DialogType {
	kConfirmNewGame,
	kConfirmLoadGame,
	kConfirmOverwrite,
	kConfirmEraseSavedGame,
	kErrorEraseSavedGame,
	kConfirmQuit
};

class Menu : public Drawable {
public:
	Menu(Myst3Engine *vm);
	virtual ~Menu();

	/** Indicates if the player is currently in a menu node */
	bool isOpen() const;

	/**
	 * Handle an event for the menu
	 *
	 * @return true if the event was handled
	 */
	virtual bool handleInput(const Common::KeyState &e) = 0;

	void updateMainMenu(uint16 action);
	void goToNode(uint16 node);

	/**
	 * Capture a save thumbnail
	 */
	Graphics::Surface *captureThumbnail();

	/**
	 * Capture and save a thumbnail
	 * thumbnail can be obtain from borrowSaveThumbnail()
	 */
	void generateSaveThumbnail();

	/**
	 * Get the current save thumbnail
	 *
	 * Only valid while the menu is open
	 */
	Graphics::Surface *borrowSaveThumbnail();

	virtual void saveLoadAction(uint16 action, uint16 item) = 0;
	virtual void setSaveLoadSpotItem(uint16 id, SpotItemFace *spotItem);

protected:
	Myst3Engine *_vm;

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _saveThumbnail;

	SpotItemFace *_saveLoadSpotItem;
	Common::String _saveLoadAgeName;

	uint dialogIdFromType(DialogType type);
	uint16 dialogConfirmValue();
	uint16 dialogSaveValue();

	Graphics::Surface *createThumbnail(Graphics::Surface *big);

	Common::String getAgeLabel(GameState *gameState);
};

class PagingMenu : public Menu {
public:
	PagingMenu(Myst3Engine *vm);
	virtual ~PagingMenu();

	void draw() override;
	bool handleInput(const Common::KeyState &e) override;

	void saveLoadAction(uint16 action, uint16 item) override;

private:
	Common::StringArray _saveLoadFiles;
	Common::String _saveName;
	bool _saveDrawCaret;
	int32 _saveCaretCounter;

	static const uint kCaretSpeed = 25;

	void loadMenuOpen();
	void loadMenuSelect(uint16 item);
	void loadMenuLoad();
	void loadMenuChangePage();
	void saveMenuOpen();
	void saveMenuSelect(uint16 item);
	void saveMenuChangePage();
	void saveMenuSave();
	void saveLoadErase();

	void saveLoadUpdateVars();

	Common::String prepareSaveNameForDisplay(const Common::String &name);
};

class AlbumMenu : public Menu {
public:
	AlbumMenu(Myst3Engine *vm);
	virtual ~AlbumMenu();

	void draw() override;
	bool handleInput(const Common::KeyState &e) override;

	void saveLoadAction(uint16 action, uint16 item) override;
	void setSaveLoadSpotItem(uint16 id, SpotItemFace *spotItem) override;

private:
	static const uint16 kAlbumThumbnailWidth = 100;
	static const uint16 kAlbumThumbnailHeight = 56;

	// This map does not own its elements
	Common::HashMap<int, SpotItemFace *> _albumSpotItems;
	Common::String _saveLoadTime;

	void loadMenuOpen();
	void loadMenuSelect();
	void loadMenuLoad();
	void saveMenuOpen();
	void saveMenuSave();
	void setSavesAvailable();

	Common::String getSaveNameTemplate();
	Common::HashMap<int, Common::String> listSaveFiles();
	void loadSaves();
};

class Dialog : public Drawable {
public:
	Dialog(Myst3Engine *vm, uint id);
	virtual ~Dialog();
	virtual void draw() override;
	virtual int16 update() = 0;

protected:
	Common::Rect getPosition() const;

	Myst3Engine *_vm;
	Video::BinkDecoder _bink;
	Texture *_texture;

	uint _buttonCount;
};

class ButtonsDialog : public Dialog {
public:
	ButtonsDialog(Myst3Engine *vm, uint id);
	virtual ~ButtonsDialog();

	void draw() override;
	int16 update() override;

private:
	Common::Point getRelativeMousePosition() const;

	uint16 _previousframe;
	uint16 _frameToDisplay;

	Common::Rect _buttons[3];

	void loadButtons();
};

class GamepadDialog : public Dialog {
public:
	GamepadDialog(Myst3Engine *vm, uint id);
	virtual ~GamepadDialog();

	int16 update() override;
};

} // End of namespace Myst3

#endif // MENU_H_
