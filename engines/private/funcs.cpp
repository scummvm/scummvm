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
    g_private->_mode = args[0].u.val;
    Common::String *s = new Common::String(args[1].u.str);
    g_private->_nextSetting = s;

    if (g_private->_mode == 0) {
        g_private->_origin->x = 0;
        g_private->_origin->y = 0;
	// TODO: should clear the screen?
    } 
    else if (g_private->_mode == 1) {
        g_private->_origin->x = 63;
        g_private->_origin->y = 48;
        g_private->drawScreenFrame();
    }
    else
        assert(0);
}

void VSPicture(ArgArray args) {
    // assert types
    debug("VSPicture(%s)", args[0].u.str);
    g_private->_nextVS = new Common::String(args[0].u.str);
}


void Goto(ArgArray args) { // should be goto, but this is a reserved word
    // assert types
    debug("goto(%s)", args[0].u.str);
    Common::String *s = new Common::String(args[0].u.str);
    g_private->_nextSetting = s;
}

void Quit(ArgArray args) {
    debug("quit()");
    g_private->quitGame();
}


void RestartGame(ArgArray args) {
    // assert types
    debug("WARNING: RestartGame is not implemented");
}

void DossierAdd(ArgArray args) {
    // assert types
    debug("WARNING: DossierAdd is not implemented");
}

void Inventory(ArgArray args) {
    // assert types
    debug("WARNING: Inventory is not implemented");
}

void SetFlag(ArgArray args) {
    // assert types
    debug("SetFlag(%s, %d)", args[0].u.sym->name->c_str(), args[1].u.val);
    args[0].u.sym->u.val = args[1].u.val;
}

void Exit(ArgArray args) {
    // assert types
    assert(args[2].type == RECT || args[2].type == NAME);
    debug("Exit(%d %d %d)", args[0].type, args[1].type, args[2].type); //, args[0].u.str, args[1].u.sym->name->c_str(), "RECT");
    ExitInfo *e = (ExitInfo*) malloc(sizeof(ExitInfo));

    if (args[0].type == NUM && args[0].u.val == 0)
        e->nextSetting = NULL;
    else
        e->nextSetting = new Common::String(args[0].u.str);

    if (args[1].type == NUM && args[1].u.val == 0)
        e->cursor = NULL;
    else
        e->cursor = args[1].u.sym->name;

    if (args[2].type == NAME) {
	assert(args[2].u.sym->type == RECT);
	args[2].u.rect = args[2].u.sym->u.rect;
    }

    e->rect = args[2].u.rect;
    debug("Rect %d %d %d %d", args[2].u.rect->top, args[2].u.rect->left, args[2].u.rect->bottom, args[2].u.rect->right);
    g_private->_exits.push_front(*e);
}

void SetModifiedFlag(ArgArray args) {
    // assert types
    debug("SetModifiedFlag(%d)", args[0].u.val);
    g_private->_modified = (bool) args[0].u.val;
}


void Sound(ArgArray args) {
    // assert types
    debug("Sound(%s)", args[0].u.str);
    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s);
        //assert(0);
    } else {
        g_private->stopSound();
    }
}

void LoopedSound(ArgArray args) {
    // assert types
    assert(args.size() == 1);
    debug("LoopedSound(%s)", args[0].u.str);
    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s);
        //assert(0);
    } else {
        g_private->stopSound();
    }
}



void Transition(ArgArray args) {
    // assert types
    debug("Transition(%s, %s)", args[0].u.str, args[1].u.str);
    g_private->_nextMovie = new Common::String(args[0].u.str);
    g_private->_nextSetting = new Common::String(args[1].u.str);
}

void Movie(ArgArray args) {
    // assert types
    debug("Movie(%s, %s)", args[0].u.str, args[1].u.str);
    if (strcmp(args[0].u.str, "\"\"") != 0)
        g_private->_nextMovie = new Common::String(args[0].u.str);
    g_private->_nextSetting = new Common::String(args[1].u.str);
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
    g_private->loadImage(*s, x, y);
}

void Mask(ArgArray args, bool drawn) {
    assert(args.size() == 3 || args.size() == 5);

    int x = 0;
    int y = 0;

    char *f = args[0].u.str;
    char *e = args[1].u.str;
    Common::String *c = args[2].u.sym->name;
    
    if (args.size() == 5) {
        x = args[3].u.val;
        y = args[4].u.val;
    }

    debug("Mask(%s, %s, %s, %d, %d)", f, e, c->c_str(), x, y);
    const Common::String *s = new Common::String(f);
    //if (drawed)
    //    g_private->loadImage(*s, x, y);

    MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, x, y, drawn);
    m->nextSetting = new Common::String(e);
    m->cursor = c;
    m->point = new Common::Point(x,y);
    g_private->_masks.push_front(*m);

}


void Timer(ArgArray args) {
    assert (args.size() == 2 || args.size() == 3);

    if (args.size() == 3)
        debug("Timer(%d, %s, %s)", args[0].u.val, args[1].u.str, args[2].u.str);
    else 
        debug("Timer(%d, %s)", args[0].u.val, args[1].u.str);
 
    g_system->delayMillis(100 * args[0].u.val);
    Common::String *s = new Common::String(args[1].u.str);
    g_private->_nextSetting = s;
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
    else if (strcmp(name, "SoundEffect") == 0) {
        Sound(args); // Unclear how this is different from Sound
    }
    else if (strcmp(name, "LoopedSound") == 0) {
        LoopedSound(args);
    }
    else if (strcmp(name, "Bitmap") == 0) {
        Bitmap(args);
    }
    else if (strcmp(name, "Mask") == 0) {
        Mask(args, false);
    }
    else if (strcmp(name, "MaskDrawn") == 0) {
        Mask(args, true);
    }
 
    else if (strcmp(name, "Timer") == 0) {
        Timer(args);
    }
    else if (strcmp(name, "Transition") == 0) {
        Transition(args);
    }
    else if (strcmp(name, "Movie") == 0) {
        Movie(args);
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
    else if (strcmp(name, "DossierAdd") == 0) {
        DossierAdd(args);
    }
    else if (strcmp(name, "Inventory") == 0) {
        Inventory(args);
    }
    else if (strcmp(name, "VSPicture") == 0) {
        VSPicture(args);
    }
    else if (strcmp(name, "CRect") == 0) {
        CRect(args);
    }
    else if (strcmp(name, "RestartGame") == 0) {
        RestartGame(args);
    }
    else {
        debug("I don't know how to exec %s", name);
        assert(0);
    }

}

}
