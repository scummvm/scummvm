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
	int minArgs;
	int maxArgs;
	bool parens;
} builtins[] = {
	// Math
	{ "abs",	Lingo::b_abs,		1, 1, true },
	{ "atan",	Lingo::b_atan,		1, 1, true },
	{ "cos",	Lingo::b_cos,		1, 1, true },
	{ "exp",	Lingo::b_exp,		1, 1, true },
	{ "float",	Lingo::b_float,		1, 1, true },
	{ "integer",Lingo::b_integer,	1, 1, true },
	{ "log",	Lingo::b_log,		1, 1, true },
	{ "pi",		Lingo::b_pi,		0, 0, true },
	{ "power",	Lingo::b_power,		2, 2, true },
	{ "random",	Lingo::b_random,	1, 1, true },
	{ "sin",	Lingo::b_sin,		1, 1, true },
	{ "sqrt",	Lingo::b_sqrt,		1, 1, true },
	{ "tan",	Lingo::b_tan,		1, 1, true },
	// String
	{ "chars",	Lingo::b_chars,		3, 3, true },
	{ "length",	Lingo::b_length,	1, 1, true },
	{ "string",	Lingo::b_string,	1, 1, true },
	// Files
	{ "closeDA",	 	Lingo::b_closeDA, 		0, 0, false },
	{ "closeResFile",	Lingo::b_closeResFile,	0, 1, false },
	{ "closeXlib",		Lingo::b_closeXlib,		0, 1, false },
	{ "openDA",	 		Lingo::b_openDA, 		1, 1, false },
	{ "openResFile",	Lingo::b_openResFile,	1, 1, false },
	{ "openXlib",		Lingo::b_openXlib,		1, 1, false },
	// Control
	{ "continue",		Lingo::b_continue,		0, 0, false },
	{ "dontPassEvent",	Lingo::b_dontPassEvent,	0, 0, false },
	{ "delay",	 		Lingo::b_delay,			1, 1, false },
	{ "do",		 		Lingo::b_do,			1, 1, false },
	{ "nothing",		Lingo::b_nothing,		0, 0, false },
	{ "pause",			Lingo::b_pause,			0, 0, false },
	{ "playAccel",		Lingo::b_playAccel,		-1,0, false }, // variable number of parameters
	// Misc
	{ "alert",	 		Lingo::b_alert,			1, 1, false },
	{ "cursor",	 		Lingo::b_cursor,		1, 1, false },
	{ "editableText",	Lingo::b_editableText,	0, 0, false },
	{ "installMenu",	Lingo::b_installMenu,	1, 1, false },
	{ "moveableSprite",	Lingo::b_moveableSprite,0, 0, false },
	{ "printFrom",	 	Lingo::b_printFrom,		-1,0, false },
	{ "updatestage",	Lingo::b_updatestage,	0, 0, false },
	{ "ilk",	 		Lingo::b_ilk,			1, 2, true },
	// Point
	{ "point",	Lingo::b_point, 2, 2, true },
	// Sound
	{ "beep",	 		Lingo::b_beep,			0, 1, false },
	{ 0, 0, 0, 0, false }
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		Symbol *sym = new Symbol;

		sym->name = (char *)calloc(strlen(blt->name) + 1, 1);
		Common::strlcpy(sym->name, blt->name, strlen(blt->name));
		sym->type = BLTIN;
		sym->nargs = blt->minArgs;
		sym->maxArgs = blt->maxArgs;
		sym->parens = blt->parens;
		sym->u.func = blt->func;

		_handlers[blt->name] = sym;
	}
}

///////////////////
// Math
///////////////////
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
	res.type = INT;

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

	if (s.type != STRING)
		error("Incorrect type for 'chars' function: %s", s.type2str());

	to.toInt();
	from.toInt();

	int len = strlen(s.u.s->c_str());
	int f = MAX(0, MIN(len, from.u.i - 1));
	int t = MAX(0, MIN(len, to.u.i));

	Common::String *res = new Common::String(&(s.u.s->c_str()[f]), &(s.u.s->c_str()[t]));

	delete s.u.s;

	s.u.s = res;
	s.type = STRING;
	g_lingo->push(s);
}

void Lingo::b_length() {
	Datum d = g_lingo->pop();

	if (d.type != STRING)
		error("Incorrect type for 'length' function: %s", d.type2str());

	int len = strlen(d.u.s->c_str());
	delete d.u.s;

	d.u.i = len;
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::b_string() {
	Datum d = g_lingo->pop();
	d.toString();
	g_lingo->push(d);
}

///////////////////
// Files
///////////////////
void Lingo::b_closeDA() {
	warning("STUB: b_closeDA");
}

void Lingo::b_closeResFile() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_closeXlib() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_openDA() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openDA(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_openResFile() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_openXlib() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

///////////////////
// Control
///////////////////
void Lingo::b_dontPassEvent() {
	warning("STUB: b_dontPassEvent");
}

void Lingo::b_continue() {
	warning("STUB: b_continue");
}

void Lingo::b_nothing() {
	warning("STUB: b_nothing");
}

void Lingo::b_delay() {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_delay(%d)", d.u.i);
}

void Lingo::b_do() {
	Datum d = g_lingo->pop();
	d.toString();
	warning("STUB: b_do(%s)", d.u.s->c_str());
}

void Lingo::b_pause() {
	warning("STUB: b_pause");
}

void Lingo::b_playAccel() {
	warning("STUB: b_playAccel");
}

void Lingo::b_printFrom() {
	warning("STUB: b_printFrom");
}

///////////////////
// Misc
///////////////////
void Lingo::b_updatestage() {
	warning("STUB: b_updatestage");
}

void Lingo::b_ilk() {
	Datum d = g_lingo->pop();
	d.u.i = d.type;
	d.type = SYMBOL;
	g_lingo->push(d);
}

void Lingo::b_alert() {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_alert");

	delete d.u.s;
}

void Lingo::b_cursor() {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_cursor(%d)", d.u.i);
}

void Lingo::b_editableText() {
	warning("STUB: b_editableText");
}

void Lingo::b_installMenu() {
	Datum d = g_lingo->pop();
	warning("STUB: b_installMenu(%d)", d.u.i);
}

void Lingo::b_moveableSprite() {
	Datum d = g_lingo->pop();
	warning("STUB: b_moveableSprite(%d)", d.u.i);
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
	d.type = POINT;

	g_lingo->push(d);
}


///////////////////
// Sound
///////////////////
void Lingo::b_beep() {
	Datum d = g_lingo->pop();
	warning("STUB: b_beep(%d)", d.u.i);
}



} // End of namespace Director
