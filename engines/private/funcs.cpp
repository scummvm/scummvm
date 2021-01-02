#include "common/str.h"
#include "common/debug.h"

#include "grammar.h"
#include "private.h"

namespace Private {

void ChgMode(ArgArray args) {
    debug("ChgMode(%d, %s)", args[0].val, args[1].str);  
    _mode = args[0].val;
    Common::String *s = new Common::String(args[1].str);
    _nextSetting = s;
}

void Goto(ArgArray args) { // should be goto, but this is a reserved word
    debug("goto(%s)", args[0].str);  
    Common::String *s = new Common::String(args[0].str);
    _nextSetting = s;
}

void SetFlag(ArgArray args) {
    debug("SetFlag(%s, %d)", args[0].sym->name->c_str(), args[1].val);
    args[0].sym->u.val = args[1].val;
}

void Sound(ArgArray args) {
    debug("Sound(%s)", args[0].str);
    if (strcmp("\"\"", args[0].str) != 0) {
            Common::String *s = new Common::String(args[0].str);
	    _private->playSound(*s);
	    //assert(0);
    } else {
	    _private->stopSound();
    }
}


void execFunction(char *name, ArgArray args) {
    if (strcmp(name, "ChgMode") == 0) {
	ChgMode(args);
    }
    else if (strcmp(name, "goto") == 0) {
	Goto(args);
    }

    else if (strcmp(name, "SetFlag") == 0) {
	SetFlag(args);
    }
    else if (strcmp(name, "Sound") == 0) {
	Sound(args);
    }

    else 
        assert(0);

}

}
