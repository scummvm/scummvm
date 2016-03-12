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
#include "titanic/true_talk/tt_named_script.h"

namespace Titanic {

TTNamedScriptBase::TTNamedScriptBase(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTScriptBase(0, charClass, v2, charName, v3, v4, v5, v6, v7),
		_val1(val1), _field54(0), _val2(val2) {
}

/*------------------------------------------------------------------------*/

TTNamedScript::TTNamedScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTNamedScriptBase(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {

}

void TTNamedScript::proc4(int v) {
	warning("TODO");
}

int TTNamedScript::proc6() const {
	return 1;
}

void TTNamedScript::proc7(int v1, int v2) {
	warning("TODO");
}

int TTNamedScript::proc8() const {
	return 0;
}

int TTNamedScript::proc9() const {
	return 2;
}

int TTNamedScript::proc10() const {
	return 2;
}

int TTNamedScript::proc11() const {
	return 2;
}

int TTNamedScript::proc12() const {
	return 1;
}

void TTNamedScript::proc13() const {
	warning("TODO");
}

void TTNamedScript::proc14(int v) {
	warning("TODO");
}

int TTNamedScript::proc15() const {
	return 0;
}

int TTNamedScript::proc16() const {
	return 1;
}

int TTNamedScript::proc17() const {
	return 1;
}

int TTNamedScript::proc18() const {
	return 1;
}

void TTNamedScript::proc19(int v) {
	warning("TODO");
}

void TTNamedScript::proc20(int v) {
	warning("TODO");
}

int TTNamedScript::proc21(int v) {
	return v;
}

int TTNamedScript::proc22() const {
	return 0;
}

int TTNamedScript::proc23() const {
	return 0;
}

int TTNamedScript::proc25() const {
	return 0;
}

void TTNamedScript::proc26() {
}

void TTNamedScript::save1(SimpleFile *file) {
	error("TODO");
}

void TTNamedScript::proc28(int v) {
	warning("TODO");
}

void TTNamedScript::save2(SimpleFile *file) {
	error("TODO");
}

void TTNamedScript::proc30(int v) {
	warning("TODO");
}

void TTNamedScript::proc31() {
	warning("TODO");
}

void TTNamedScript::proc32() {
	warning("TODO");
}

void TTNamedScript::proc33(int v1, int v2) {
	warning("TODO");
}

int TTNamedScript::proc34() {
	warning("TODO");
	return 0;
}

int TTNamedScript::proc35(int v1, int v2) {
	warning("TODO");
	return 0;
}

int TTNamedScript::proc36() const {
	return 0;
}

int TTNamedScript::proc37() const {
	return 0;
}

} // End of namespace Titanic
