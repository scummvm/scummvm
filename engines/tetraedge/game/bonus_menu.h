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

#ifndef TETRAEDGE_GAME_BONUS_MENU_H
#define TETRAEDGE_GAME_BONUS_MENU_H

#include "common/array.h"
#include "common/str.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class BonusMenu : public TeLuaGUI {
public:
	BonusMenu();

	class SaveButton : public TeLayout {
	public:
		SaveButton(TeButtonLayout *btn, const Common::String &name, BonusMenu *owner);
		bool onLoadSave();
		Common::String path() const;

		BonusMenu *_menu;
	};

	virtual void enter() override;
	virtual void enter(const Common::String &scriptName);
	void leave() override;

	void loadGame(Common::String &name) {
		_gameName = name;
	}

	bool onLeftButton();
	bool onMouseMove(const Common::Point &pt);
	bool onPictureButton();
	bool onQuitButton();
	bool onRightButton();
	bool onSlideButtonDown();

	// empty? bool onLoadGameConfirmed() { };

private:
	Common::Array<SaveButton *> _saveButtons;
	TeVector2s32 _slideBtnStartMousePos;
	Common::String _gameName;
	int _pageNo;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_BONUS_MENU_H
