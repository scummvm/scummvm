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

#ifndef BLADERUNNER_DIALOGUE_MENU_H
#define BLADERUNNER_DIALOGUE_MENU_H

#include "bladerunner/shape.h"

#include "common/array.h"

#include "graphics/surface.h"

namespace BladeRunner {

class BladeRunnerEngine;
class TextResource;

struct DialogueItem {
	char text[50];
	int  answerValue;
	int  field_36;
	int  field_3A;
	int  field_3E;
	int  field_42;
	int  field_46;
};

class DialogueMenu {
	BladeRunnerEngine *_vm;

	TextResource         *_textResource;
	Common::Array<Shape>  _shapes;
	bool                  _isVisible;
	bool                  _waitingForInput;
	int                   _selectedItemIndex;
	int                   _listSize;

	// These track whether a dialogue option
	// has previously been selected
	int                   _neverRepeatListSize;
	int                   _neverRepeatValues[100];
	bool                  _neverRepeatWasSelected[100];

	int                   _centerX;
	int                   _centerY;
	int                   _screenX;
	int                   _screenY;
	int                   _maxItemWidth;
	DialogueItem          _items[10];

public:
	DialogueMenu(BladeRunnerEngine *vm);
	~DialogueMenu();

	bool loadText(const char *name);

	bool show();
	bool showAt(int x, int y);
	bool hide();
	bool clearList();
	bool addToList(int answer, int a3, int a4, int a5, int a6);
	bool addToListNeverRepeatOnceSelected(int answer, int a3, int a4, int a5);
	int  queryInput();
	int  listSize();
	bool isVisible();
	bool isOpen();
	void tick(int x, int y);
	void draw();
	int  getAnswerIndex(int answer);
	const char *getText(int id);
	void calculatePosition(int unusedX = 0, int unusedY = 0);

	void mouseUp();
	bool waitingForInput();

	void clear();
	void reset();

	static void darkenRect(Graphics::Surface &s, int x1, int y1, int x2, int y2);
};

} // End of namespace BladeRunner

#endif
