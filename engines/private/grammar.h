#include "common/str.h"
#include "common/hash-str.h"
#include "common/hash-ptr.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

#ifndef PRIVATE_GRAMMAR_H
#define PRIVATE_GRAMMAR_H

#define	NSTACK	256
#define	NPROG	10000

typedef struct Arg {
  int n;
  int (**inst)();
} Arg;

typedef struct Symbol {	/* symbol table entry */
	Common::String *name;
	short	type;	/* NAME, NUM, STRING or RECT  */
	union {
		int	val;	    /* NAME or NUM */
		char	*str;	    /* STRING */
		Common::Rect *rect; /* RECT */
	} u;
} Symbol;

typedef struct Datum {	/* interpreter stack type */
        short type;
	union {	
	        int	 val;
	        char    *str;
	        Symbol	*sym;
		Common::Rect *rect;
	} u;
} Datum;

namespace Private {

typedef int (*Inst)();	/* machine instruction */
#define	STOP	(Inst) 0

typedef Common::HashMap<void *, Common::String *> FuncHash;

extern void initFuncs();

typedef struct Setting {

    Datum	stack[NSTACK];	/* the stack */
    Datum	*stackp;	/* next free spot on stack */

    Inst	prog[NPROG];	/* the machine */
    Inst	*progp;		/* next free spot for code generation */
    Inst	*pc;		/* program counter during execution */

} Setting;

// Settings

extern Setting *psetting;

typedef Common::HashMap<Common::String, Setting*> SettingMap;
typedef Common::Queue<Common::String> StringQueue;
typedef Common::Queue<Common::Rect*> RectQueue;


extern StringQueue todefine;
extern SettingMap settingcode;

// Symbols

extern void showSymbol(Symbol *);

typedef Common::HashMap<Common::String, Symbol*> SymbolMap;
typedef Common::List<Symbol*> ConstantList;

extern SymbolMap settings, variables, cursors, locations, rects;
extern ConstantList constants;

extern void define(char *, Common::Rect *); 
extern Symbol  *install(Common::String *, int, int, char *, Common::Rect *, SymbolMap*);
extern Symbol  *lookupName(char *);
extern Symbol *addconstant(int, int, char *);
extern void     installall(char *);
extern Symbol  *lookup(Common::String, SymbolMap);

// Funtions

typedef Common::Array<Datum> ArgArray;
extern void execFunction(char *, ArgArray);

// Code Generation

extern	Datum pop();
extern  int push(Datum);
extern  Inst	*progp;

extern  Inst *code(Inst);
extern	Inst *prog;
extern	int eval();
extern  int add();
extern  int negate(); 
extern  int power();
extern	int assign();
extern  int bltin();
extern  int varpush(); 
extern  int constpush();
extern  int strpush();
extern  int funcpush();
extern  int print();
extern  int ifcode();
extern  int fail();
extern  int lt();
extern  int gt();
extern  int le();
extern  int ge();
extern  int eq();
extern  int ne();

// Code Execution

extern void initSetting();
extern void saveSetting(char *);
extern void loadSetting(Common::String *);

extern void execute(Inst *);

}

#endif
