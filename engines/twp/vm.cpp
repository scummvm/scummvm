/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "twp/twp.h"
#include "twp/vm.h"
#include "twp/sqgame.h"
#include "twp/squtil.h"
#include "twp/thread.h"
#include "common/array.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/random.h"
#include "image/png.h"
#include "squirrel/squirrel.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqstring.h"
#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqstdstring.h"
#include "squirrel/sqstdmath.h"
#include "squirrel/sqstdio.h"
#include "squirrel/sqstdaux.h"
#include "squirrel/sqfuncproto.h"
#include "squirrel/sqclosure.h"

namespace Twp {

static HSQUIRRELVM gVm = nullptr;

static void sqExec(HSQUIRRELVM v, const char *code) {
	SQInteger top = sq_gettop(v);
	if (SQ_FAILED(sq_compilebuffer(v, code, strlen(code), "twp", SQTrue))) {
		sqstd_printcallstack(v);
		return;
	}
	sq_pushroottable(v);
	if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue))) {
		sqstd_printcallstack(v);
		sq_pop(v, 1); // removes the closure
		return;
	}
	sq_settop(v, top);
}

static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line,
						 SQInteger column) {
	debug("TWP: desc %s, source: %s (%lld,%lld)", desc, source, line, column);
}

static SQInteger aux_printerror(HSQUIRRELVM v) {
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if (!pf)
		return 0;

	if (sq_gettop(v) < 1)
		return 0;

	const SQChar *error = nullptr;
	if (SQ_FAILED(sq_getstring(v, 2, &error))) {
		error = "unknown";
	}
	pf(v, _SC("\nAn error occured in the script: %s\n"), error);
	sqstd_printcallstack(v);

	return 0;
}

static void printfunc(HSQUIRRELVM v, const SQChar *s, ...) {
	char buf[1024 * 1024];
	va_list vl;
	va_start(vl, s);
	vsnprintf(buf, 1024 * 1024, s, vl);
	va_end(vl);

	debug("TWP: %s", buf);
}

Vm::Vm() {
	gVm = v = sq_open(1024);
	sq_setcompilererrorhandler(v, errorHandler);
	sq_newclosure(v, aux_printerror, 0);
	sq_seterrorhandler(v);
	sq_setprintfunc(v, printfunc, printfunc);

	sq_pushroottable(v);
	sqstd_register_stringlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_iolib(v);
	sq_pop(v, 1);

	sqgame_register_syslib(v);
	sqgame_register_genlib(v);
	sqgame_register_objlib(v);

	// TODO: constants
	SQObject platform = sqtoobj(v, 666);
	_table(v->_roottable)->NewSlot(sqtoobj(v, _SC("PLATFORM")), SQObjectPtr(platform));
}

Vm::~Vm() {
	for (int i = 0; i < g_engine->_threads.size(); i++) {
		delete g_engine->_threads[i];
	}
	sq_close(v);
}

void Vm::exec(const SQChar *code) {
	sqExec(v, code);
}
} // namespace Twp
