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

#include "tinsel/noir/notebook.h"

#include "tinsel/background.h"
#include "tinsel/dialogs.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/noir/sysreel.h"
#include "tinsel/pdisplay.h"
#include "tinsel/polygons.h"
#include "tinsel/timers.h"

namespace Tinsel {

Notebook::Notebook() {
	_polygons = instantiateNoteBookPolygons();
}

Notebook::~Notebook() {
	delete _polygons;
}

void Notebook::addHyperlink(int32 id1, int32 id2) {
	auto *invObject = _vm->_dialogs->getInvObjectT3(id1);

	if (invObject->getTitle() != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	invObject = _vm->_dialogs->getInvObjectT3(id2);

	if (invObject->getTitle() != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	uint32 i;
	for (i = 0; i < MAX_HYPERS; ++i) {
		int32 curr_id1 = _hyperlinks[i].id1;
		if (curr_id1 == 0) {
			_hyperlinks[i].id1 = id1;
			_hyperlinks[i].id2 = id2;
			return;
		}

		if ((curr_id1 == id1) || (id1 == _hyperlinks[i].id2)) {
			if ((curr_id1 != id2) && (id2 != _hyperlinks[i].id2)) {
				error("A clue/title can only be hyperlinked to one other clue/title!");
			}
			return;
		}
	}

	// No free hyperlink entry was found
	error("Too many hyperlinks");
}

void Notebook::clearNotebookPage() {
	if (_prevPage != -1) {
		_pages[_prevPage].clear();
	}
	_prevPage = -1;
	_pages[_currentPage].clear();
}

void Notebook::refresh() {
	auto reel = (_currentPage == 0 ? SysReel::NOTEPAD_CLOSED : SysReel::NOTEPAD_OPEN);
	auto film = GetSystemReelFilm(reel);
	InitStepAnimScript(&_anim, _object, film->reels->script, ONE_SECOND / film->frate);
	clearNotebookPage();
	if (_currentPage != 0) {
		_pages[_currentPage].fillIn();
	}
}

int Notebook::addTitle(const InventoryObjectT3 &invObject) {
	int id = invObject.getId();
	assert(invObject.isNotebookTitle());
	for (uint32 i = 0; i < _numPages; i++) {
		if (_pages[i].getTitle() == id) {
			return i;
		}
	}
	int linkedFromPage = invObject.getUnknown();

	// 0 page is the closed notebook, has no entries.
	if (linkedFromPage != 0) {
		// Allocate a line on the linked from page.
		assert(_pages[linkedFromPage].getTitle() != 0);
		_pages[linkedFromPage].addLine(id);
	}
	int pageIndex = _numPages++;
	_pages[pageIndex].setTitle(id);
	return pageIndex;
}

void Notebook::addClue(const InventoryObjectT3 &invObject) {
	if (invObject.getUnknown() == 0) {
		// This affects two clues, that should get special treatment.
		warning("TODO: Handle clues with no parent page");
		return;
	}
	// Add title if missing, otherwise just get the page it's on.
	auto titleObject = _vm->_dialogs->getInvObjectT3(invObject.getUnknown());
	int pageIndex = addTitle(*titleObject);
	_pages[pageIndex].addLine(invObject.getId());
	if (invObject.getTitle() != 0) {
		auto secondTitleObject = _vm->_dialogs->getInvObjectT3(invObject.getTitle());
		pageIndex = addTitle(*secondTitleObject);
		_pages[pageIndex].addLine(invObject.getId());
	}
}

void Notebook::addClue(int id) {
	auto invObject = _vm->_dialogs->getInvObjectT3(id);
	if (invObject->isNotebookTitle()) {
		addTitle(*invObject);
	} else {
		addClue(*invObject);
	}
}

int Notebook::getPageWithTitle(int id) {
	for (uint32 i = 0; i < _numPages; i++) {
		if (_pages[i].getTitle() == id) {
			return i;
		}
	}
	return -1;
}

void Notebook::crossClue(int id) {
	auto invObject = _vm->_dialogs->getInvObjectT3(id);
	if (invObject->isNotebookTitle()) {
		return;
	}
	int titles[2] = {
		invObject->getUnknown(),
		invObject->getTitle()
	};
	for (int i = 0; i < 2; i++) {
		if (titles[i] == 0) {
			continue;
		}
		int page = getPageWithTitle(titles[i]);
		if (page != -1) {
			_pages[page].crossClue(id);
		}
	}
}

void InitNotebookAnim(OBJECT **obj, ANIM &anim, SysReel reel, int zPosition) {
	auto film = GetSystemReelFilm(reel);
	MultiDeleteObjectIfExists(FIELD_STATUS, obj);
	*obj = InsertReelObj(film->reels);
	MultiSetZPosition(*obj, zPosition);
	InitStepAnimScript(&anim, *obj, film->reels->script, ONE_SECOND / film->frate);
}

void Notebook::setNextPage(int pageIndex) {
	assert(_prevPage == -1 || _prevPage == (int32)_currentPage); // Check that we've cleaned any outstanding page.
	_prevPage = _currentPage;
	_currentPage = pageIndex;
}

void Notebook::pageFlip(bool up) {
	int nextPage = _currentPage + (up ? -1 : 1);
	if (nextPage <= 0) {
		setNextPage(0);
		refresh();
		return;
	} else if (nextPage == 1) {
		// TODO: Should possibly just call whatever function we use to open.
		InitNotebookAnim(&_object, _anim, SysReel::NOTEPAD_OPENING, Z_INV_RFRAME);
		_state = BOOKSTATE::OPEN_ANIMATING;
		setNextPage(nextPage);
		return;
	}
	setNextPage(nextPage);
	SysReel reel = (up ? SysReel::NOTEPAD_FLIPUP : SysReel::NOTEPAD_FLIPDOWN);
	InitNotebookAnim(&_pageObject, _pageAnim, reel, 19);
	_state = BOOKSTATE::PAGEFLIP;
}

void Notebook::show(bool isOpen) {
	auto reel = (isOpen ? SysReel::NOTEPAD_OPEN : SysReel::NOTEPAD_OPENING);
	InitNotebookAnim(&_object, _anim, reel, Z_INV_MFRAME);

	_state = (isOpen ? BOOKSTATE::OPENED : BOOKSTATE::OPEN_ANIMATING);
	setNextPage(1);
	refresh();
	DisableTags(); // Tags disabled in Notebook
	DisablePointing(); // Pointing disabled in Notebook
}

bool Notebook::isOpen() const {
	return _state != BOOKSTATE::CLOSED;
}

void Notebook::close() {
	clearNotebookPage();
	MultiDeleteObjectIfExists(FIELD_STATUS, &_object);
	MultiDeleteObjectIfExists(FIELD_STATUS, &_pageObject);
	_state = BOOKSTATE::CLOSED;
	if (_vm->_dialogs->inventoryOrNotebookActive()) {
		EnablePointing();
		EnableTags();
	}
}

void Notebook::stepAnimScripts() {
	if (_state == BOOKSTATE::OPEN_ANIMATING) {
		auto state = StepAnimScript(&_anim);
		if (state == ScriptFinished) {
			_state = BOOKSTATE::OPENED;
			refresh();
		}
	}
	if (_state == BOOKSTATE::PAGEFLIP) {
		auto state = StepAnimScript(&_pageAnim);
		if (state == ScriptFinished) {
			MultiDeleteObjectIfExists(FIELD_STATUS, &_pageObject);
			_state = BOOKSTATE::OPENED;
			refresh();
		}
	}
}

int32 Notebook::GetPointedClue(const Common::Point &point) const {
	if (_currentPage == 0 || _currentPage > _numPages) {
		return 0;
	}
	return _pages[_currentPage].getClueForLine(_polygons->lineHit(point));
}

bool Notebook::handlePointer(const Common::Point &point) {
	if (!isOpen()) {
		return 0;
	}
	auto inside  = _polygons->isInsideNotebook(point);
	if (inside) {
		auto hit = _polygons->lineHit(point);
		_pages[_currentPage].handlePointAtLine(hit);
		return true; // We handled the pointer
	}
	return false;
}

bool Notebook::handleEvent(PLR_EVENT pEvent, const Common::Point &coOrds) {
	if (!isOpen()) { // TODO: Clicking outside should close the notebook
		return false;
	}
	auto inside  = _polygons->isInsideNotebook(coOrds);
	switch(pEvent) {
	case PLR_ACTION:
		if (inside) {
			return true;
		}
		return false;
	case PLR_LOOK:
		if (inside) {
			return true;
		}
		return false;
	case PLR_WALKTO: {
		// Handle clue-clicks
		auto poly = _polygons->mostSpecificHit(coOrds);
		switch (poly) {
		case NoteBookPoly::NEXT:
			handleEvent(PLR_PGUP, coOrds);
			return true;
		case NoteBookPoly::PREV:
			handleEvent(PLR_PGDN, coOrds);
			return true;
		case NoteBookPoly::NONE:
			handleEvent(PLR_ESCAPE, coOrds);
			return true;
		default:
			return true;
		}
	}

	case PLR_ESCAPE:
		close();
		return true;
	case PLR_PGUP:
		pageFlip(true);
		return true;
	case PLR_PGDN:
		pageFlip(false);
		return true;
	case PLR_HOME:
	case PLR_END:
	default:
		return false;
	}
	return false;
}

} // End of namespace Tinsel
