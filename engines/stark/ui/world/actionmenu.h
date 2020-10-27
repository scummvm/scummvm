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

#ifndef STARK_UI_ACTIONMENU_H
#define STARK_UI_ACTIONMENU_H

#include "engines/stark/ui/window.h"

namespace Stark {

class Cursor;
class VisualImageXMG;
class VisualText;
class InventoryWindow;
class GameWindow;

namespace Resources {
class ItemVisual;
class Sound;
}

class ActionMenu : public Window {
public:
	ActionMenu(Gfx::Driver *gfx, Cursor *cursor);
	~ActionMenu() override;

	void setInventory(InventoryWindow *inventory);

	void open(Resources::ItemVisual *item, const Common::Point &itemRelativePos);
	void close();

	void onScreenChanged();

protected:
	Common::Rect computePosition(const Common::Point &mouse) const;

	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onGameLoop() override;
	void onRender() override;

private:
	void clearActions();
	void enableAction(uint32 action);
	void updateActionSound();

	enum ActionMenuType {
		kActionNone  = -1,
		kActionHand  =  0,
		kActionEye   =  1,
		kActionMouth =  2
	};

	struct ActionButton {
		bool enabled;
		uint32 action;
		Common::Rect rect;
	};

	bool _fromInventory;
	ActionButton _buttons[3];

	VisualImageXMG *_background;
	VisualText *_itemDescription;

	Common::Point _itemRelativePos;
	Resources::ItemVisual *_item;

	InventoryWindow *_inventory;

	int32 _autoCloseTimeRemaining;

	int32 _activeMenuType;

	Resources::Sound *_actionMouthHoverSound;
	Resources::Sound *_actionHoverSound;
};

} // End of namespace Stark

#endif
