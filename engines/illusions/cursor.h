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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_CURSOR_H
#define ILLUSIONS_CURSOR_H

namespace Illusions {

class Control;
class IllusionsEngine;

class Cursor {
public:
	Cursor(IllusionsEngine *vm);
	void place(Control *control, uint32 sequenceId);
	void setActorIndex(int actorIndex, int a, int b);
	void setControl(Control *control);
	void show();
	void hide();
	void cursorControlRoutine(Control *control, uint32 deltaTime);
//protected:
public:
	IllusionsEngine *_vm;
	Control *_control;
	uint32 _sequenceId;
	int _status;
	int _cursorNum;
	int _field_10;
	int _visibleCtr;
	int16 _x, _y;
};

} // End of namespace Illusions

#endif // ILLUSIONS_CURSOR_H
