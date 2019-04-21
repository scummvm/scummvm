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

#include "titanic/game/gondolier/gondolier_base.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGondolierBase, CGameObject)
	ON_MESSAGE(PuzzleSolvedMsg)
END_MESSAGE_MAP()

bool CGondolierBase::_chestOpen;
bool CGondolierBase::_puzzleSolved;
int CGondolierBase::_volume1;
int CGondolierBase::_slider1;
int CGondolierBase::_volume2;
int CGondolierBase::_slider2;
bool CGondolierBase::_rightSliderHooked;
bool CGondolierBase::_leftSliderHooked;
bool CGondolierBase::_priorLeftSliderHooked;
bool CGondolierBase::_priorRightSliderHooked;

void CGondolierBase::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_chestOpen, indent);
	file->writeNumberLine(_puzzleSolved, indent);
	file->writeNumberLine(_volume1, indent);
	file->writeNumberLine(_slider1, indent);
	file->writeNumberLine(_rightSliderHooked, indent);
	file->writeNumberLine(_volume2, indent);
	file->writeNumberLine(_slider2, indent);
	file->writeNumberLine(_leftSliderHooked, indent);
	file->writeNumberLine(_priorLeftSliderHooked, indent);
	file->writeNumberLine(_priorRightSliderHooked, indent);

	CGameObject::save(file, indent);
}

void CGondolierBase::load(SimpleFile *file) {
	file->readNumber();
	_chestOpen = file->readNumber();
	_puzzleSolved = file->readNumber();
	_volume1 = file->readNumber();
	_slider1 = file->readNumber();
	_leftSliderHooked = file->readNumber();
	_volume2 = file->readNumber();
	_slider2 = file->readNumber();
	_rightSliderHooked = file->readNumber();
	_priorLeftSliderHooked = file->readNumber();
	_priorRightSliderHooked = file->readNumber();

	CGameObject::load(file);
}

bool CGondolierBase::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_puzzleSolved = true;
	return true;
}

} // End of namespace Titanic
