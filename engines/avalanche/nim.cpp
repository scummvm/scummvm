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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"
#include "avalanche/nim.h"

namespace Avalanche {

const char * const Nim::names[2] = {"Avalot", "Dogfood"};

Nim::Nim(AvalancheEngine *vm) {
	_vm = vm;
}

void Nim::playNim() {
	warning("STUB: Nim::playNim()");
}

void Nim::chalk(int x,int y, Common::String z) {
	warning("STUB: Nim::chalk()");
}

void Nim::setup() {
	warning("STUB: Nim::setup()");
}

void Nim::plotStone(byte x,byte y) {
	warning("STUB: Nim::plotStone()");
}

void Nim::board() {
	warning("STUB: Nim::board()");
}

void Nim::startMove() {
	warning("STUB: Nim::startMove()");
}

void Nim::showChanges() {
	warning("STUB: Nim::showChanges()");
}

void Nim::blip() {
	warning("STUB: Nim::blip()");
}

void Nim::checkMouse() {
	warning("STUB: Nim::checkMouse()");
}

void Nim::less() {
	warning("STUB: Nim::less()");
}

void Nim::takeSome() {
	warning("STUB: Nim::takeSome()");
}

void Nim::endOfGame() {
	warning("STUB: Nim::endOfGame()");
}

void Nim::dogFood() {
	warning("STUB: Nim::dogFood()");
}

bool Nim::find(byte x) {
	warning("STUB: Nim::find()");
	return true;
}

void Nim::findAp(byte start,byte stepsize) {
	warning("STUB: Nim::findAp()");
}

} // End of namespace Avalanche
