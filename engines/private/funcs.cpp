#include "common/str.h"
#include "common/debug.h"
#include "common/system.h"

#include "grammar.h"
#include "grammar.tab.h"
#include "private.h"

namespace Private {

void ChgMode(ArgArray args) {
    // assert types 
    debug("ChgMode(%d, %s)", args[0].u.val, args[1].u.str);  
    _mode = args[0].u.val;
    Common::String *s = new Common::String(args[1].u.str);
    _nextSetting = s;
}

void Goto(ArgArray args) { // should be goto, but this is a reserved word
    // assert types
    debug("goto(%s)", args[0].u.str);  
    Common::String *s = new Common::String(args[0].u.str);
    _nextSetting = s;
}

void Quit(ArgArray args) { 
    debug("quit()");
    _private->quitGame(); 
}



void SetFlag(ArgArray args) {
    // assert types
    debug("SetFlag(%s, %d)", args[0].u.sym->name->c_str(), args[1].u.val);
    args[0].u.sym->u.val = args[1].u.val;
}

void Exit(ArgArray args) {
    // assert types
    assert(args[2].type == RECT || args[2].type == NAME);
    debug("Exit(%s, %s, %s)", args[0].u.str, args[1].u.sym->name->c_str(), "RECT");
    ExitInfo *e = (ExitInfo*) malloc(sizeof(ExitInfo));
    e->nextSetting = new Common::String(args[0].u.str);
    e->cursor = args[1].u.sym->name;
    e->rect = args[2].u.rect;
    _exits.push_front(*e);
}



void SetModifiedFlag(ArgArray args) {
    // assert types
    debug("SetModifiedFlag(%d)", args[0].u.val);
    _modified = (bool) args[0].u.val;
}


void Sound(ArgArray args) {
    // assert types
    debug("Sound(%s)", args[0].u.str);
    if (strcmp("\"\"", args[0].u.str) != 0) {
            Common::String *s = new Common::String(args[0].u.str);
	    _private->playSound(*s);
	    //assert(0);
    } else {
	    _private->stopSound();
    }
}

void Transition(ArgArray args) {
    // assert types
    debug("Transition(%s, %s)", args[0].u.str, args[1].u.str);
    _nextMovie = new Common::String(args[0].u.str);
    _nextSetting = new Common::String(args[1].u.str);
}


void CRect(ArgArray args) {
    // assert types
    int x1, y1, x2, y2;

    debug("CRect(%d, %d, %d, %d)", args[0].u.val, args[1].u.val, args[0].u.val, args[1].u.val);
    //assert(0);
    x1 = args[0].u.val;
    y1 = args[1].u.val;

    x2 = args[2].u.val;
    y2 = args[3].u.val;

    Datum *d = new Datum();
    Common::Rect *rect = new Common::Rect(x1, y1, x2, y2);

    d->type = RECT;
    d->u.rect = rect;
    push(*d);
}

void Bitmap(ArgArray args) {
    assert(args.size() == 1 || args.size() == 3);

    int x = 0;
    int y = 0;

    char *f = args[0].u.str;
    if (args.size() == 3) {
	x = args[1].u.val;
	y = args[2].u.val;
    }

    debug("Bitmap(%s, %d, %d)", f, x, y);
    Common::String *s = new Common::String(args[0].u.str);
    _private->loadImage(*s, x, y);
}

void Timer(ArgArray args) {
    debug("Timer(%d, %s, %s)", args[0].u.val, args[1].u.str, args[2].u.str);
    g_system->delayMillis(100 * args[0].u.val);
    Common::String *s = new Common::String(args[1].u.str);
    _nextSetting = s; 
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
    else if (strcmp(name, "Bitmap") == 0) {
	Bitmap(args);
    }
    else if (strcmp(name, "Timer") == 0) {
        Timer(args);	    
    }
    else if (strcmp(name, "Transition") == 0) {
        Transition(args);	    
    }
    else if (strcmp(name, "SetModifiedFlag") == 0) {
        SetModifiedFlag(args);	    
    }
    else if (strcmp(name, "Exit") == 0) {
        Exit(args);	    
    }
    else if (strcmp(name, "Quit") == 0) {
        Quit(args);	    
    }
    else if (strcmp(name, "LoadGame") == 0) {
       ;	    
    }
    else if (strcmp(name, "CRect") == 0) {
        CRect(args);	    
    } 
    else {
        debug("I don't know how to exec %s", name);	    
        assert(0);
    }

}

}
