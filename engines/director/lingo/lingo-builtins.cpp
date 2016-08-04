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
	// Math
	{ "abs",	Lingo::b_abs, 1 },
	{ "atan",	Lingo::b_atan, 1 },
	{ "cos",	Lingo::b_cos, 1 },
	{ "exp",	Lingo::b_exp, 1 },
	{ "float",	Lingo::b_float, 1 },
	{ "integer",Lingo::b_integer, 1 },
	{ "log",	Lingo::b_log, 1 },
	{ "pi",		Lingo::b_pi, 0 },
	{ "power",	Lingo::b_power, 2 },
	{ "random",	Lingo::b_random, 1 },
	{ "sin",	Lingo::b_sin, 1 },
	{ "sqrt",	Lingo::b_sqrt, 1 },
	{ "tan",	Lingo::b_tan, 1 },
	// String
	{ "chars",	Lingo::b_chars, 3 },
	{ "length",	Lingo::b_length, 1 },
	{ "string",	Lingo::b_string, 1 },
	// Misc
	{ "closeDA",	 	Lingo::b_closeDA, -1 },
	{ "continue",		Lingo::b_continue, -1 },
	{ "dontpassevent",	Lingo::b_dontpassevent, -1 },
	{ "updatestage",	Lingo::b_updatestage, -1 },
	{ "ilk",	 		Lingo::b_ilk, 1 },
	// point
	{ "point",	Lingo::b_point, 2 },
	{ 0, 0, 0 }
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		Symbol *sym = new Symbol;

		sym->name = (char *)calloc(strlen(blt->name) + 1, 1);
		Common::strlcpy(sym->name, blt->name, strlen(blt->name));
		sym->type = vBLTIN;
		sym->nargs = blt->nparams;
		sym->u.func = blt->func;

		_handlers[blt->name] = sym;
	}
}

///////////////////
// Math
///////////////////
void Lingo::b_abs() {
	Datum d = g_lingo->pop();

	if (d.type == vINT)
		d.u.i = ABS(d.u.i);
	else if (d.type == vFLOAT)
		d.u.f = ABS(d.u.f);

	g_lingo->push(d);
}

void Lingo::b_atan() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = atan(d.u.f);
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
	d.u.f = exp(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_float() {
	Datum d = g_lingo->pop();
	d.toFloat();
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
	d.u.f = log(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_pi() {
	Datum d;
	d.toFloat();
	d.u.f = M_PI;
	g_lingo->push(d);
}

void Lingo::b_power() {
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	d1.toFloat();
	d2.toFloat();
	d1.u.f = pow(d2.u.f, d1.u.f);
	g_lingo->push(d1);
}

void Lingo::b_random() {
	Datum max = g_lingo->pop();
	Datum res;

	max.toInt();

	res.u.i = g_lingo->_vm->_rnd.getRandomNumber(max.u.i);
	res.type = vINT;

	g_lingo->push(res);
}

void Lingo::b_sin() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sin(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_sqrt() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sqrt(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_tan() {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = tan(d.u.f);
	g_lingo->push(d);
}

///////////////////
// String
///////////////////
void Lingo::b_chars() {
	Datum to = g_lingo->pop();
	Datum from = g_lingo->pop();
	Datum s = g_lingo->pop();

	if (s.type != vSTRING)
		error("Incorrect type for 'chars' function: %s", s.type2str());

	to.toInt();
	from.toInt();

	int len = strlen(s.u.s->c_str());
	int f = MAX(0, MIN(len, from.u.i - 1));
	int t = MAX(0, MIN(len, to.u.i));

	Common::String *res = new Common::String(&(s.u.s->c_str()[f]), &(s.u.s->c_str()[t]));

	delete s.u.s;

	s.u.s = res;
	s.type = vSTRING;
	g_lingo->push(s);
}

void Lingo::b_length() {
	Datum d = g_lingo->pop();

	if (d.type != vSTRING)
		error("Incorrect type for 'length' function: %s", d.type2str());

	int len = strlen(d.u.s->c_str());
	delete d.u.s;

	d.u.i = len;
	d.type = vINT;
	g_lingo->push(d);
}

void Lingo::b_string() {
	Datum d = g_lingo->pop();
	d.toString();
	g_lingo->push(d);
}

///////////////////
// Misc
///////////////////
void Lingo::b_dontpassevent() {
	warning("STUB: b_dontpassevent");
}

void Lingo::b_updatestage() {
	warning("STUB: b_updatestage");
}

void Lingo::b_ilk() {
	Datum d = g_lingo->pop();
	d.u.i = d.type;
	d.type = vSYMBOL;
	g_lingo->push(d);
}

void Lingo::b_closeDA() {
	warning("STUB: b_closeDA");
}

void Lingo::b_continue() {
	warning("STUB: b_continue");
}

///////////////////
// Point
///////////////////
void Lingo::b_point() {
	Datum y = g_lingo->pop();
	Datum x = g_lingo->pop();
	Datum d;

	x.toFloat();
	y.toFloat();

	d.u.arr = new FloatArray;

	d.u.arr->push_back(x.u.f);
	d.u.arr->push_back(y.u.f);
	d.type = vPOINT;

	g_lingo->push(d);
}


} // End of namespace Director
