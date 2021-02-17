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
        char    *str;
        Symbol  *sym;
        Common::Rect *rect;
    } u;
} Datum;

typedef struct Arg {
    int n;
    int (** inst)();
} Arg;

typedef int (* Inst)();  /* machine instruction */
#define STOP    (Inst) 0

typedef Common::HashMap<void *, Common::String *> PtrToName;
typedef Common::HashMap<Common::String, void *> NameToPtr;

void initInsts();
void initFuncs();

typedef struct Setting {

    Datum stack[NSTACK]; /* the stack */
    Datum *stackp;       /* next free spot on stack */

    Inst  prog[NPROG];   /* the machine */
    Inst  *progp;        /* next free spot for code generation */
    Inst  *pc;           /* program counter during execution */

} Setting;

// Settings


typedef Common::HashMap<Common::String, Setting *> SettingMap;

class SettingMaps {
    public:
    Setting *psetting;
    Setting *setting;
    SettingMap map;

    void init();
    void save(char *);
    void load(Common::String *);
};

// Funtions

typedef Common::Array<Datum> ArgArray;
extern void call(char *, ArgArray);

// Code Generation

Datum pop();
int push(Datum);
extern  Inst *progp;

extern  Inst *code(Inst);
extern  Inst *prog;
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

} // End of namespace Private

#endif
