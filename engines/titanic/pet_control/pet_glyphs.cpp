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

void CPetGlyph::translateDraw(CScreenManager *screenManager, int deltaX, int deltaY) {
	_element.translate(deltaX, deltaY);
	_element.draw(screenManager);
	_element.translate(-deltaX, -deltaY);
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

void CPetGlyphs::clear() {
	changeHighlight(-1);
	destroyContents();
	_field10 = 0;
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
	warning("TODO: CPetGlyphs::draw");
}

void CPetGlyphs::changeHighlight(int index) {
	warning("TODO: CPetGlyphs::changeHighlight");
}

void CPetGlyphs::highlight(int index) {
	warning("TODO: CPetGlyphs::highlight");
}

} // End of namespace Titanic
