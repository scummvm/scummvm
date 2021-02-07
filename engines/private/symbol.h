#include "common/str.h"
#include "common/hash-str.h"
#include "common/hash-ptr.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

#ifndef PRIVATE_SYMBOL_H
#define PRIVATE_SYMBOL_H

namespace Private {

typedef struct Symbol {	/* symbol table entry */
    Common::String *name;
    short	type;	/* NAME, NUM, STRING or RECT  */
    union {
        int	val;	    /* NAME or NUM */
        char	*str;	    /* STRING */
        Common::Rect *rect; /* RECT */
    } u;
} Symbol;

// Symbols

extern void showSymbol(Symbol *);
extern void setSymbol(Symbol *, int);

typedef Common::HashMap<Common::String, Symbol*> SymbolMap;
typedef Common::List<Common::String> NameList;
typedef Common::List<Symbol*> ConstantList;

extern SymbolMap settings, variables, cursors, locations, rects;
extern ConstantList constants;
extern NameList variableList;
extern NameList locationList;

extern void     defineSymbol(char *, Common::Rect *);
extern Symbol  *install(Common::String *, int, int, char *, Common::Rect *, SymbolMap*);
extern Symbol  *lookupName(char *);
extern Symbol  *constant(int, int, char *);
extern void     installAll(char *);
extern Symbol  *lookup(Common::String, SymbolMap);

}

#endif