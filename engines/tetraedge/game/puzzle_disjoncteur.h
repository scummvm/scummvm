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

#ifndef TETRAEDGE_GAME_PUZZLE_DISJONCTEUR_H
#define TETRAEDGE_GAME_PUZZLE_DISJONCTEUR_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_sprite_layout.h"
#include "tetraedge/te/te_xml_gui.h"

namespace Tetraedge {

class PuzzleDisjoncteur : public Te3DObject2 {
public:
	PuzzleDisjoncteur();

	void wakeUp();
	void sleep();

private:
	void addState(uint32 flags);
	void removeState(uint32 flags);
	void setDraggedSprite(TeSpriteLayout *sprite);
	void setDraggedSpriteBack();
	void setState(uint32 flags) { _state = flags; }

	bool onExitButton();
	bool onLevierAnimFinished();
	bool onMouseDown(const Common::Point &pt);
	bool onMouseMove(const Common::Point &pt);
	bool onMouseUp(const Common::Point &pt);
	bool onWinTimer();

	TeXmlGui _gui;
	uint32 _state;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_PUZZLE_DISJONCTEUR_H
