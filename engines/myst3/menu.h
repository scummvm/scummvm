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
#include "common/memstream.h"
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

class Menu {
public:
	Menu(Myst3Engine *vm);
	virtual ~Menu();

	void draw();
	void handleInput(const Common::KeyState &e);

	void updateMainMenu(uint16 action);
	void goToNode(uint16 node);

	void loadMenuOpen();
	void loadMenuSelect(uint16 item);
	void loadMenuLoad();
	void loadMenuChangePage();
	void saveMenuOpen();
	void saveMenuSelect(uint16 item);
	void saveMenuChangePage();
	void saveMenuSave();
	void saveLoadErase();
	void setSaveLoadSpotItem(SpotItemFace *spotItem) { _saveLoadSpotItem = spotItem; }

private:
	Myst3Engine *_vm;

	Common::StringArray _saveLoadFiles;
	SpotItemFace *_saveLoadSpotItem;
	Common::String _saveLoadAgeName;
	Common::String _saveName;
	bool _saveDrawCaret;
	int32 _saveCaretCounter;

	static const uint kCaretSpeed = 25;

	uint dialogIdFromType(DialogType type);

	void saveLoadUpdateVars();

	Graphics::Surface *createThumbnail(Graphics::Surface *big);
	void saveGameReadThumbnail(Common::InSaveFile *save);
	void saveGameWriteThumbnail(Common::OutSaveFile *save);

	Common::String getAgeLabel(GameState *gameState);
	Common::String prepareSaveNameForDisplay(const Common::String &name);
};

class Dialog : public Drawable {
public:
	Dialog(Myst3Engine *vm, uint id);
	virtual ~Dialog();
	virtual void draw() override;
	virtual int16 update() = 0;

protected:
	Myst3Engine *_vm;
	Video::BinkDecoder _bink;
	Texture *_texture;

	uint _buttonCount;

	Common::Rect getPosition();
};

class ButtonsDialog : public Dialog {
public:
	ButtonsDialog(Myst3Engine *vm, uint id);
	virtual ~ButtonsDialog();

	void draw() override;
	int16 update() override;

private:
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
