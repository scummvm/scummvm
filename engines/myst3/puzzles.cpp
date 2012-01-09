/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/puzzles.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Puzzles::Puzzles(Myst3Engine *vm) :
	_vm(vm) {
}

Puzzles::~Puzzles() {
}

void Puzzles::run(uint16 id, uint16 arg0, uint16 arg1, uint16 arg3) {
	switch (id) {
	case 8:
		journalSaavedro(arg0);
		break;
	case 9:
		journalAtrus(arg0, arg1);
		break;
	default:
		warning("Puzzle %d is not implemented", id);
	}
}

void Puzzles::journalSaavedro(int16 move) {
	uint16 chapter = _vm->_vars->getJournalSaavedroChapter();
	int16 page = _vm->_vars->getJournalSaavedroPageInChapter();

	if (!_journalSaavedroHasChapter(chapter))
		chapter = _journalSaavedroNextChapter(chapter, true);

	if (move > 0) {
		// Go to the next available page
		int16 pageCount = _journalSaavedroPageCount(chapter);
		page++;

		if (page == pageCount) {
			chapter = _journalSaavedroNextChapter(chapter, true);
			page = 0;
		}

		_vm->_vars->setJournalSaavedroChapter(chapter);
		_vm->_vars->setJournalSaavedroPageInChapter(page);
	} else if (move < 0) {
		// Go to the previous available page
		page--;

		if (page < 0) {
			chapter = _journalSaavedroNextChapter(chapter, false);
			page = _journalSaavedroPageCount(chapter) - 1;
		}

		_vm->_vars->setJournalSaavedroChapter(chapter);
		_vm->_vars->setJournalSaavedroPageInChapter(page);
	} else {
		// Display current page
		int16 chapterStartNode = _journalSaavedroGetNode(chapter);
		int16 closed = 0;
		int16 opened = 0;
		int16 lastPage = 0;

		if (chapter > 0) {
			opened = 1;
			if (chapter == 21)
				lastPage = 2;
			else
				lastPage = 1;

		} else {
			closed = 1;
		}

		uint16 nodeRight;
		uint16 nodeLeft;
		if (page || !chapter) {
			nodeRight = chapterStartNode + page;
			nodeLeft = chapterStartNode + page;
		} else {
			nodeRight = chapterStartNode + page;
			uint16 chapterLeft = _journalSaavedroNextChapter(chapter, false);
			if (chapterLeft > 0)
				nodeLeft = _journalSaavedroGetNode(chapterLeft + 1);
			else
				nodeLeft = 2;
		}

		_vm->_vars->setJournalSaavedroClosed(closed);
		_vm->_vars->setJournalSaavedroOpen(opened);
		_vm->_vars->setJournalSaavedroLastPage(lastPage);

		// TODO: Draw nodeLeft on the left part of the screen
		_vm->loadNodeFrame(nodeRight);
	}
}

uint16 Puzzles::_journalSaavedroGetNode(uint16 chapter) {
	const DirectorySubEntry *desc = _vm->getFileDescription(0, 1200, 0, DirectorySubEntry::kMetadata);

	if (!desc)
		error("Node 1200 does not exist");

	return desc->getMiscData(chapter) + 199;
}

uint16 Puzzles::_journalSaavedroPageCount(uint16 chapter) {
	uint16 chapterStartNode = _journalSaavedroGetNode(chapter);
	if (chapter != 21)
		return _journalSaavedroGetNode(chapter + 1) - chapterStartNode;
	else
		return 1;
}

bool Puzzles::_journalSaavedroHasChapter(uint16 chapter) {
	return _vm->_vars->get(285 + chapter) != 0;
}

uint16 Puzzles::_journalSaavedroNextChapter(uint16 chapter, bool forward) {
	do {
		if (forward)
			chapter++;
		else
			chapter--;
	} while (!_journalSaavedroHasChapter(chapter));

	return chapter;
}

void Puzzles::journalAtrus(uint16 node, uint16 var) {
	uint numPages = 0;

	while (_vm->getFileDescription(0, node++, 0, DirectorySubEntry::kFrame))
		numPages++;

	_vm->_vars->set(var, numPages - 1);
}

} /* namespace Myst3 */
