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

#ifndef ULTIMA_ULTIMA1_GFX_GAME_VIEW_H
#define ULTIMA_ULTIMA1_GFX_GAME_VIEW_H

#include "ultima/shared/gfx/visual_container.h"

namespace Ultima {

namespace Shared {
	class Info;
	class Status;
	class ViewportDungeon;
	class ViewportMap;
}
	
namespace Ultima1 {

/**
 * This class implements a standard view screen that shows a status and log area, as well as either
 * a map or dungeon view covering the bulk of the screen
 */
class GameView : public Shared::Gfx::VisualContainer {
private:
	Shared::Info *_info;
	Shared::Status *_status;
	Shared::ViewportDungeon *_viewportDungeon;
	Shared::ViewportMap *_viewportMap;
private:
	/**
	 * Draw the frame for the screen
	 */
	void drawFrame();
public:
	GameView();
	virtual ~GameView();

	/**
	 * Draw the game screen
	 */
	virtual void draw();
};

} // End of namespace Shared
} // End of namespace Xeen

#endif
