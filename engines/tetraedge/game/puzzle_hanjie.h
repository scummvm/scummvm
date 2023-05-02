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

#ifndef TETRAEDGE_GAME_PUZZLE_HANJIE_H
#define TETRAEDGE_GAME_PUZZLE_HANJIE_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_xml_gui.h"
#include "tetraedge/te/te_button_layout.h"
#include "tetraedge/te/te_sprite_layout.h"

namespace Tetraedge {

class PuzzleHanjie : public Te3DObject2 {
public:
	PuzzleHanjie();

	void wakeUp();
	void sleep();

private:
	bool isSolved();
	bool onExitButton();
	bool onWinTimer();
	bool onMouseUp(const Common::Point &pt);

	TeTimer _timer;
	TeXmlGui _gui;

	TeButtonLayout *_exitButton;
	TeSpriteLayout *_sprites[49];
	TeSpriteLayout *_foregroundSprite;
	int _backgroundNo;
	bool _entered;
	Common::String _bgImg;
	Common::String _soundBegin;
	TeSpriteLayout _bgSprite;
	bool _expectedVals[49];

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_PUZZLE_HANJIE_H
