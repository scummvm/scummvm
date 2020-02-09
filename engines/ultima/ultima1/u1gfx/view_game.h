/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef ULTIMA_ULTIMA1_GFX_VIEW_GAME_H
#define ULTIMA_ULTIMA1_GFX_VIEW_GAME_H

#include "ultima/shared/gfx/visual_container.h"
#include "ultima/shared/gfx/bitmap.h"

namespace Ultima {

namespace Shared {
class Info;
class ViewportDungeon;

namespace Actions {
class Action;
} // End of namespace Actions
} // End of namespace Shared
	
namespace Ultima1 {
namespace U1Gfx {

class Status;
class ViewportMap;
using Shared::CShowMsg;
using Shared::CEndOfTurnMsg;
using Shared::CFrameMsg;
using Shared::CCharacterInputMsg;

/**
 * This class implements a standard view screen that shows a status and log area, as well as either
 * a map or dungeon view covering the bulk of the screen
 */
class ViewGame : public Shared::Gfx::VisualContainer {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool EndOfTurnMsg(CEndOfTurnMsg &msg);
	bool FrameMsg(CFrameMsg &msg);
	bool CharacterInputMsg(CCharacterInputMsg &msg);
private:
	Shared::Info *_info;
	Shared::ViewportDungeon *_viewportDungeon;
	ViewportMap *_viewportMap;
	Status *_status;
	Common::Array<Shared::Actions::Action *> _actions;
	int _frameCtr;
private:
	/**
	 * Draws level & direction indicators when in a dungeon
	 */
	void drawIndicators();

	/**
	 * Handle movement keys
	 */
	bool checkMovement(const Common::KeyState &keyState);
public:
	CLASSDEF;
	ViewGame(TreeItem *parent = nullptr);
	~ViewGame() override;

	/**
	 * Draw the game screen
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
