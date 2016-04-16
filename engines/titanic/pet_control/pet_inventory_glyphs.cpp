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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_inventory_glyphs.h"

namespace Titanic {

CPetInventoryGlyphs::CPetInventoryGlyphs() : _field10(0), _field14(7),
	_field18(-1), _field1C(-1), _field20(0), _field24(0) {
}

void CPetInventoryGlyphs::proc8() {
	error("TODO");
}

void CPetInventoryGlyphs::setup() {
	warning("TODO: CPetInventoryGlyphs::setup");
}

void CPetInventoryGlyphs::proc10() {
	error("TODO");
}

void CPetInventoryGlyphs::proc11() {
	error("TODO");
}

void CPetInventoryGlyphs::draw(CScreenManager *screenManager) {
	warning("TODO: CPetInventoryGlyphs::draw");
}

void CPetInventoryGlyphs::addItem(CPetCarry *item) {


	warning("TODO");
}

void CPetInventoryGlyphs::clear() {
	fn1(-1);
	destroyContents();
	_field10 = 0;
}

void CPetInventoryGlyphs::fn1(int val) {
	warning("TODO: CPetInventoryGlyphs::fn1");
}

} // End of namespace Titanic
