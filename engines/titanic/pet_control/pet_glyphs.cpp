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

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

bool CPetGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	_element.setBounds(Rect(0, 0, 52, 50));
	_owner = owner;
	return true;
}

void CPetGlyph::drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted) {
	_element.translate(pt.x, pt.y);
	_element.draw(screenManager);
	_element.translate(-pt.x, -pt.y);
}

void CPetGlyph::proc14() {
	warning("TODO: CPetGlyph::proc14");
}

bool CPetGlyph::contains(const Point &delta, const Point &pt) {
	translate(delta);
	bool result = _element.contains2(pt);
	translateBack(delta);

	return result;
}

CPetSection *CPetGlyph::getPetSection() const {
	return _owner ? _owner->getOwner() : nullptr;
}

CPetControl *CPetGlyph::getPetControl() const {
	return _owner ? _owner->getPetControl() : nullptr;
}

void CPetGlyph::setName(const CString &name, CPetControl *petControl) {
	Rect r(0, 0, 52, 52);
	_element.setBounds(r);
	_element.reset(name, petControl, MODE_UNSELECTED);
}

/*------------------------------------------------------------------------*/

CPetGlyphs::CPetGlyphs() : _firstVisibleIndex(0),  _numVisibleGlyphs(TOTAL_GLYPHS),
		_highlightIndex(-1), _field1C(-1), _flags(0),
		_dragGlyph(nullptr), _owner(nullptr) {
}

void CPetGlyphs::setNumVisible(int total) {
	if (total > 0)
		_numVisibleGlyphs = total;
}

void CPetGlyphs::clear() {
	changeHighlight(-1);
	destroyContents();
	_firstVisibleIndex = 0;
}

void CPetGlyphs::setup(int numVisible, CPetSection *owner) {
	setNumVisible(numVisible);
	_owner = owner;
	_selection.setBounds(Rect(0, 0, 76, 76));

	int buttonsLeft = numVisible * 70 + 21;

	_scrollLeft.setBounds(Rect(0, 0, 31, 15));
	_scrollLeft.translate(buttonsLeft, 373);
	_scrollRight.setBounds(Rect(0, 0, 31, 15));
	_scrollRight.translate(buttonsLeft, 413);
}

void CPetGlyphs::reset() {
	if (_owner && _owner->_petControl) {
		CPetControl *pet = _owner->_petControl;

		_scrollLeft.reset("PetScrollLeft", pet, MODE_UNSELECTED);
		_scrollRight.reset("PetScrollRight", pet, MODE_UNSELECTED);
		_selection.reset("PetSelection", pet, MODE_UNSELECTED);

		for (iterator i = begin(); i != end(); ++i) {
			(*i)->reset();
		}
	}
}

bool CPetGlyphs::enter() {
	if (_highlightIndex != -1) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph && glyph->enter())
			return true;
	}

	return false;
}

bool CPetGlyphs::leave() {
	if (_highlightIndex != -1) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph && glyph->leave())
			return true;
	}

	return false;
}

void CPetGlyphs::draw(CScreenManager *screenManager) {
	if (_highlightIndex != -1) {
		int index = getHighlightedIndex(_highlightIndex);
		if (index != -1) {
			Point tempPoint;
			Point pt = getPosition(index);
			pt -= Point(12, 13);
			_selection.translate(pt.x, pt.y);
			_selection.draw(screenManager);
			_selection.translate(-pt.x, -pt.y);
		}
	}

	// Iterate through displaying glyphs on the screen 
	int listSize = size();
	for (int index = 0; index < _numVisibleGlyphs; ++index) {
		int itemIndex = getItemIndex(index);

		if (itemIndex >= 0 && itemIndex < listSize) {
			Point pt = getPosition(itemIndex);
			CPetGlyph *glyph = getGlyph(itemIndex);

			if (glyph)
				glyph->drawAt(screenManager, pt, index == _highlightIndex);
		}
	}

	// Draw scrolling arrows if more than a screen's worth of items are showing
	if (listSize > _numVisibleGlyphs || (_flags & GFLAG_16)) {
		_scrollLeft.draw(screenManager);
		_scrollRight.draw(screenManager);
	}

	// Handle secondary highlight
	if (_highlightIndex != -1) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph)
			glyph->draw2(screenManager);
	}
}

Point CPetGlyphs::getPosition(int index) {
	Point tempPoint(37 + index * 70, 375);
	return tempPoint;
}

Rect CPetGlyphs::getRect(int index) {
	Point pt = getPosition(index);
	return Rect(pt.x, pt.y, pt.x + 52, pt.y + 52);
}

void CPetGlyphs::changeHighlight(int index) {
	if (index == _highlightIndex)
		return;

	if (_highlightIndex >= 0 && (_flags & GFLAG_4)) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph)
			glyph->unhighlightCurrent();
	}

	_highlightIndex = index;
	if (index >= 0) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);

		if (glyph) {
			if (_flags & GFLAG_4) {
				Point pt;
				int idx = getHighlightedIndex(_highlightIndex);
				if (idx >= 0)
					pt = getPosition(idx);

				glyph->highlightCurrent(pt);
			}

			glyph->proc14();
		}
	} else if (_owner) {
		_owner->proc28();
	}
}

void CPetGlyphs::highlight(int index) {
	warning("TODO: CPetGlyphs::highlight");
}

int CPetGlyphs::getHighlightedIndex(int index) {
	int idx = index - _firstVisibleIndex;
	return (idx >= 0 && idx < _numVisibleGlyphs) ? idx : -1;
}

int CPetGlyphs::getItemIndex(int index) {
	return _firstVisibleIndex + index;
}

CPetGlyph *CPetGlyphs::getGlyph(int index) {
	for (iterator i = begin(); i != end(); ++i) {
		if (index-- == 0)
			return *i;
	}

	return nullptr;
}

CPetControl *CPetGlyphs::getPetControl() const {
	return _owner ? _owner->getPetControl() : nullptr;
}

void CPetGlyphs::setFirstVisible(int index) {
	if (index != _firstVisibleIndex) {
		_firstVisibleIndex = index;

		if ((_flags & GFLAG_8) && _highlightIndex != -1) {
			CPetGlyph *glyph = getGlyph(_highlightIndex);
			
			if (glyph) {
				int idx = getHighlightedIndex(_highlightIndex);
				if (idx != -1) {
					Point tempPt = getPosition(idx);
					glyph->proc27(tempPt, true);
				}
			}
		}
	}
}

void CPetGlyphs::scrollLeft() {
	if (_firstVisibleIndex > 0) {
		setFirstVisible(_firstVisibleIndex - 1);
		if (_highlightIndex != -1) {
			int index = getHighlightedIndex(_highlightIndex);
			if (index == -1)
				changeHighlight(_highlightIndex - 1);
		}

		makePetDirty();
	}
}

void CPetGlyphs::scrollRight() {
	int count = size();
	int right = count - _numVisibleGlyphs;

	if (_firstVisibleIndex < right) {
		setFirstVisible(_firstVisibleIndex + 1);
		if (_highlightIndex != -1) {
			int index = getHighlightedIndex(_highlightIndex);
			if (index == -1)
				changeHighlight(_highlightIndex + 1);
		}

		makePetDirty();
	}
}

void CPetGlyphs::makePetDirty() {
	if (_owner && _owner->_petControl)
		_owner->_petControl->makeDirty();
}

bool CPetGlyphs::MouseButtonDownMsg(const Point &pt) {
	if (_scrollLeft.contains2(pt)) {
		scrollLeft();
		return true;
	}

	if (_scrollRight.contains2(pt)) {
		scrollRight();
		return true;
	}

	for (int idx = 0; idx < _numVisibleGlyphs; ++idx) {
		Rect glyphRect = getRect(idx);
		if (glyphRect.contains(pt)) {
			int index = getItemIndex(idx);
			CPetGlyph *glyph = getGlyph(index);
			if (glyph) {
				if (_highlightIndex == index) {
					glyph->MouseButtonDownMsg(glyphRect);
					glyph->proc14();
				} else {
					changeHighlight(index);
					makePetDirty();
				}

				return true;
			}
		}
	}

	if (_highlightIndex != -1) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);

		if (glyph) {
			if (glyph->MouseButtonDownMsg(pt))
				return true;

			if (!(_flags & GFLAG_2)) {
				changeHighlight(-1);
				makePetDirty();
			}
		}
	}

	return false;
}

bool CPetGlyphs::MouseButtonUpMsg(const Point &pt) {
	if (_highlightIndex >= 0) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph) {
			if (glyph->MouseButtonUpMsg(pt))
				return true;
		}
	}

	return false;
}

bool CPetGlyphs::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!(_flags & GFLAG_1) && _highlightIndex >= 0) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		int index = getHighlightedIndex(_highlightIndex);
		Rect glyphRect = getRect(index);

		if (glyphRect.contains(msg->_mousePos))
			return glyph->proc29(glyphRect);
		else
			return glyph->MouseDragStartMsg(msg);
	}

	return false;
}

bool CPetGlyphs::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	if (_dragGlyph) {
		return _dragGlyph->MouseDragMoveMsg(msg);
	} else {
		return false;
	}
}

bool CPetGlyphs::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	if (_dragGlyph) {
		return _dragGlyph->MouseDragEndMsg(msg);
	} else {
		return false;
	}
}

bool CPetGlyphs::KeyCharMsg(int key) {
	if (_highlightIndex >= 0) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);

		if (glyph && glyph->KeyCharMsg(key))
			return true;
	}

	return false;
}

bool CPetGlyphs::VirtualKeyCharMsg(int key) {
	bool handled = false;
	warning("TODO: CPetGlyphs::virtualKeyCharMsg");

	if (!handled && _highlightIndex >= 0) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph && glyph->VirtualKeyCharMsg(key))
			handled = true;
	}

	return handled;
}

bool CPetGlyphs::enterHighlighted() {
	if (_highlightIndex >= 0)
		return getGlyph(_highlightIndex)->enterHighlighted();
	else
		return false;
}

void CPetGlyphs::leaveHighlighted() {
	if (_highlightIndex >= 0)
		getGlyph(_highlightIndex)->leaveHighlighted();
}

void CPetGlyphs::startDragging(CPetGlyph *glyph, CMouseDragStartMsg *msg) {
	if (glyph) {
		_dragGlyph = glyph;
		msg->_dragItem = getPetControl();
	}
}

void CPetGlyphs::endDragging() {
	_dragGlyph = nullptr;
}

bool CPetGlyphs::highlighted14() {
	if (_highlightIndex != -1) {
		CPetGlyph *pet = getGlyph(_highlightIndex);
		if (pet) {
			pet->proc14();
			return true;
		}
	}

	return false;
}

} // End of namespace Titanic
