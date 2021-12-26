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

#ifndef PRIVATE_GRAMMAR_H
#define PRIVATE_GRAMMAR_H

#include "common/str.h"
#include "common/hash-str.h"
#include "common/hash-ptr.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

#include "private/symbol.h"

#define NSTACK 256
#define NPROG  10000

namespace Private {

typedef struct Datum {  /* interpreter stack type */
	short type;
	union {
		int  val;
		const char	*str;
		Symbol  *sym;
		Common::Rect *rect;
	} u;
} Datum;

typedef struct Arg {
	int n;
	int (** inst)();
} Arg;

typedef int (* Inst)();  /* machine instruction */
#define STOP	(Inst) 0

typedef Common::HashMap<void *, Common::String *> PtrToName;

void initInsts();
void initFuncs();

namespace Settings {

typedef struct Setting {
	Datum stack[NSTACK]; /* the stack */
	Inst  prog[NPROG];   /* the machine */
} Setting;

typedef Common::HashMap<Common::String, Setting *> SettingMap;

class SettingMaps {
public:
	Setting *_setting;
	SettingMap _map;

	void init();
	void save(const char *);
	void load(const Common::String &);
};

extern SettingMaps *g_setts;

}

// Funtions

typedef Common::Array<Datum> ArgArray;
void call(const char *, const ArgArray &);

// Code Generation and Execution

namespace Gen {

class VM {
public:
	Datum *_stack;  /* the stack */
	Datum *_stackp; /* next free spot on stack */

	Inst  *_progp;  /* next free spot for code generation */
	Inst  *_prog;   /* the machine */
	Inst  *_pc;	 /* program counter during execution */
	void run();	 /* run the virtual machine */
};

extern VM *g_vm;

Datum pop();
int push(const Datum &);

Inst *code(const Inst &);
int eval();
int add();
int negate();
int power();
int assign();
int bltin();
int varpush();
int constpush();
int strpush();
int funcpush();
int print();
int ifcode();
int fail();
int lt();
int gt();
int le();
int ge();
int eq();
int ne();
int randbool();

// Code Execution

void execute(Inst *);

}

} // End of namespace Private

#endif
