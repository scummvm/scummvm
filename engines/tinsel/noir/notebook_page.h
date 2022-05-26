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

#ifndef TINSEL_NOTEBOOK_PAGE_H // prevent multiple includes
#define TINSEL_NOTEBOOK_PAGE_H

#include "common/scummsys.h"
#include "tinsel/anim.h"
#include "tinsel/tinsel.h"
#include "tinsel/object.h"

namespace Tinsel {

class NotebookLine {
public:
	int _id = 0;
	void CrossOut();
	void Clear();
	void FillIn(int pageLine);
private:
	bool _crossedOut = false;
	ANIM _anim = {};
	OBJECT *_obj = nullptr;
	OBJECT *_scribbles = nullptr;
	ANIM _scribbleAnim = {};
};

class NotebookPage {
public:
	bool ContainsClue(int id);
	void CrossClue(int id);
	void AddLine(int id);
	void SetTitle(int id);
	int32 GetTitle() const;
	void FillIn();
	void Clear();
private:
	int IndexOfClue(int id) const;
	const static uint32 MAX_ENTRIES_PER_PAGE = 8;
	NotebookLine _lines[MAX_ENTRIES_PER_PAGE] = {};
	uint32 _numLines = 0;
};

} // End of namespace Tinsel

#endif // SCUMMVM_NOTEBOOK_PAGE_H
