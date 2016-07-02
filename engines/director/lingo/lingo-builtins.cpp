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

#include "engines/director/lingo/lingo.h"

namespace Director {

static struct BuiltinProto {
	const char *name;
	void (*func)(void);
	int nparams;
} builtins[] = {
	{ "abs",	Lingo::b_abs, 1},
	{ "atan",	Lingo::b_atan, 1},
	{ "cos",	Lingo::b_cos, 1},
	{ "exp",	Lingo::b_exp, 1},
	{ "integer",Lingo::b_integer, 1},
	{ "log",	Lingo::b_log, 1},
	{ "random",	Lingo::b_random, 1},
	{ 0, 0, 0 }
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		_builtins[blt->name] = new Builtin(blt->func, blt->nparams);
	}
}

void Lingo::b_abs() {
	Datum d = g_lingo->pop();

	if (d.type == INT)
		d.u.i = ABS(d.u.i);
	else if (d.type == FLOAT)
		d.u.f = ABS(d.u.f);

	g_lingo->push(d);
}

void Lingo::b_atan() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = atanf(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_cos() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = cos(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_exp() {
	Datum d = g_lingo->pop();
	d.toInt(); // Lingo uses int, so we're enforcing it
	d.toFloat();
	d.u.f = expf(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_integer() {
	Datum d = g_lingo->pop();
	d.toInt();
	g_lingo->push(d);
}

void Lingo::b_log() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = logf(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_random() {
	Datum max = g_lingo->pop();
	Datum res;

	max.toInt();

	res.u.i = g_lingo->_vm->_rnd.getRandomNumber(max.u.i);
	res.type = INT;

	g_lingo->push(res);
}

}
