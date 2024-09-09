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
#include "twp/sqgame.h"
#include "twp/squtil.h"
#include "twp/squirrel/squirrel.h"
#include "twp/squirrel/sqvm.h"
#include "twp/squirrel/sqstring.h"
#include "twp/squirrel/sqstate.h"
#include "twp/squirrel/sqtable.h"
#include "twp/squirrel/sqstdstring.h"
#include "twp/squirrel/sqstdmath.h"
#include "twp/squirrel/sqstdaux.h"
#include "twp/squirrel/sqfuncproto.h"
#include "twp/squirrel/sqclosure.h"

namespace Twp {

static HSQUIRRELVM gVm = nullptr;

static void errorHandler(HSQUIRRELVM, const SQChar *desc, const SQChar *source, SQInteger line,
						 SQInteger column) {
	debugN("TWP: desc %s, source: %s (%lld,%lld)", desc, source, line, column);
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
	pf(v, _SC("\nAn error occurred in the script: %s\n"), error);
	sqstd_printcallstack(v);

	return 0;
}

static void printfunc(HSQUIRRELVM, const SQChar *s, ...) {
	char buf[1024 * 1024];
	va_list vl;
	va_start(vl, s);
	vsnprintf(buf, 1024 * 1024, s, vl);
	va_end(vl);

	debug("TWP: %s", buf);
}

Vm::Vm() {
	gVm = v = sq_open(1024 * 2);
	sq_setcompilererrorhandler(v, errorHandler);
	sq_newclosure(v, aux_printerror, 0);
	sq_seterrorhandler(v);
	sq_setprintfunc(v, printfunc, printfunc);

	sq_pushroottable(v);
	sqstd_register_stringlib(v);
	sqstd_register_mathlib(v);
	sq_pop(v, 1);

	sqgame_register_constants(v);
	sqgame_register_syslib(v);
	sqgame_register_genlib(v);
	sqgame_register_objlib(v);
	sqgame_register_actorlib(v);
	sqgame_register_roomlib(v);
	sqgame_register_soundlib(v);
}

Vm::~Vm() {
	sq_close(v);
}

void Vm::exec(const SQChar *code) {
	sqexec(v, code);
}
} // namespace Twp
