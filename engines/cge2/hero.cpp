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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/hero.h"

namespace CGE2 {

Hero::Hero(CGE2Engine *vm) : Sprite(vm) {
	warning("STUB: Hero::Hero()");
}

Sprite *Hero::expand(void) {
	warning("STUB: Hero::expand()");
	return this;
}

void Hero::setCurrent(void) {
	warning("STUB: Hero::setCurrent()");
}

void Hero::hStep(void) {
	warning("STUB: Hero::hStep()");
}

Sprite *Hero::setContact(void) {
	warning("STUB: Hero::setContact()");
	return this;
}

void Hero::tick(void) {
	warning("STUB: Hero::tick()");
}

int Hero::distance(V3D pos) {
	warning("STUB: Hero::distance()");
	return 0;
}

int Hero::distance(Sprite *spr) {
	warning("STUB: Hero::distance()");
	return 0;
}

void Hero::turn(Dir d) {
	warning("STUB: Hero::turn()");
}

void Hero::park(void) {
	warning("STUB: Hero::park()");
}

bool Hero::lower(Sprite * spr) {
		warning("STUB: Hero::lower()");
		return false;
}

void Hero::reach(int mode) {
	warning("STUB: Hero::reach()");
}

void Hero::fun(void) {
	warning("STUB: Hero::fun()");
}

void Hero::operator ++ (void) {
	warning("STUB: Hero::operator ++()");
}

void Hero::operator -- (void) {
	warning("STUB: Hero::operator --()");
}

uint32 Hero::len(V2D v) {
	warning("STUB: Hero::works()");
	return 0;
}

bool Hero::findWay(void){
	warning("STUB: Hero::findWay()");
	return false;
}

int Hero::snap(int p, int q, int grid) {
	warning("STUB: Hero::findWay()");
	return 0;
}

void Hero::walkTo(V3D pos) {
	warning("STUB: Hero::walkTo()");
}

void Hero::walkTo(Sprite *spr) {
	warning("STUB: Hero::walkTo()");
}

int Hero::cross(const V2D &a, const V2D &b) {
	warning("STUB: Hero::cross()");
	return 0;
}

int Hero::mapCross(const V2D &a, const V2D &b) {
	warning("STUB: Hero::mapCross()");
	return 0;
}

int Hero::mapCross(const V3D &a, const V3D &b) {
	warning("STUB: Hero::mapCross()");
	return 0;
}

void Hero::setCave(int c) {
	warning("STUB: Hero::mapCross()");
}

} // End of namespace CGE2
