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
#include "common/str.h"

#include "graphics/surface.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;
class TextResource;

class DialogueMenu {
	static const int kMaxItems = 10;
	static const int kMaxRepeatHistory = 100;
	static const int kLineHeight = 9;
	static const int kBorderSize = 10;

	struct DialogueItem {
		Common::String text;
		int            answerValue;
		int            colorIntensity;
		int            priorityPolite;
		int            priorityNormal;
		int            prioritySurly;
		int            isDone;
	};

	BladeRunnerEngine *_vm;

	TextResource *_textResource;
	Shapes       *_shapes;
	bool          _isVisible;
	bool          _waitingForInput;
	int           _selectedItemIndex;
	int           _listSize;

	// These track whether a dialogue option
	// has previously been selected
	int           _neverRepeatListSize;
	int           _neverRepeatValues[kMaxRepeatHistory];
	bool          _neverRepeatWasSelected[kMaxRepeatHistory];

	int           _centerX;
	int           _centerY;
	int           _screenX;
	int           _screenY;
	int           _maxItemWidth;
	DialogueItem  _items[kMaxItems];

	int           _fadeInItemIndex;

public:
	DialogueMenu(BladeRunnerEngine *vm);
	~DialogueMenu();

	void clear();

	bool loadResources();

	bool show();
	bool hide();
	bool addToList(int answer, bool done, int priorityPolite, int priorityNormal, int prioritySurly);
	bool clearNeverRepeatWasSelectedFlag(int answer); // aux function - used in cut content mode to re-use some dialogue options for different characters
	bool addToListNeverRepeatOnceSelected(int answer, int priorityPolite, int priorityNormal, int prioritySurly);
	bool removeFromList(int answer);
	bool clearList();
	int  queryInput();
	int  listSize() const;
	bool isVisible() const;
	bool isOpen() const;
	void tick(int x, int y);
	void draw(Graphics::Surface &s);

	void mouseUp();
	bool waitingForInput() const;

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	bool showAt(int x, int y);
	int  getAnswerIndex(int answer) const;
	const char *getText(int id) const;
	void calculatePosition(int unusedX = 0, int unusedY = 0);
	void reset();

	static void darkenRect(Graphics::Surface &s, int x1, int y1, int x2, int y2);
};

} // End of namespace BladeRunner

#endif
