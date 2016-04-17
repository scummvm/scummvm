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

namespace Titanic {

void CPetGlyph::setOwner(CPetControl *petControl, CPetGlyphs *owner) {
	_element.setBounds(Rect(0, 0, 52, 50));
	_owner = owner;
}

void CPetGlyph::drawAt(CScreenManager *screenManager, int x, int y) {
	_element.translate(x, y);
	_element.draw(screenManager);
	_element.translate(-x, -y);
}

void CPetGlyph::proc14() {
	warning("TODO: CPetGlyph::proc14");
}

bool CPetGlyph::translateContains(const Point &delta, const Point &pt) {
	translate(delta);
	bool result = _element.contains2(pt);
	translateBack(delta);

	return result;
}

/*------------------------------------------------------------------------*/

CPetGlyphs::CPetGlyphs() : _firstVisibleIndex(0),  _numVisibleGlyphs(7),
		_highlightIndex(-1), _field1C(-1), _field20(0), _field24(0) {
}

void CPetGlyphs::clear() {
	changeHighlight(-1);
	destroyContents();
	_firstVisibleIndex = 0;
}

void CPetGlyphs::proc8() {
	error("TODO");
}

void CPetGlyphs::setup() {
	warning("TODO: CPetGlyphs::setup");
}

void CPetGlyphs::proc10() {
	error("TODO");
}

void CPetGlyphs::proc11() {
	error("TODO");
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

			if (glyph) {
				// TODO: Comparison with highlighted index, and a redundant push?
				glyph->drawAt(screenManager, pt.x, pt.y);
			}
		}
	}

	// Draw scrolling arrows if more than a screen's worth of items are showing
	if (listSize > _numVisibleGlyphs || _field20 != 16) {
		_scrollLeft.draw(screenManager);
		_scrollRight.draw(screenManager);
	}

	// Handle secondary highlight
	if (_highlightIndex != -1) {
		CPetGlyph *glyph = getGlyph(_highlightIndex);
		if (glyph)
			glyph->drawHighlight();
	}
}

Point CPetGlyphs::getPosition(int index) {
	Point tempPoint(37 + index * 58, 375);
	return tempPoint;
}

void CPetGlyphs::changeHighlight(int index) {
	warning("TODO: CPetGlyphs::changeHighlight");
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

} // End of namespace Titanic
