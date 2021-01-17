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
        g_private->_origin->x = 0; // use a constant
        g_private->_origin->y = 0;
    }
    else if (g_private->_mode == 1) { 
        g_private->_origin->x = 64;  // use a constant
        g_private->_origin->y = 48;
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


void SyncSound(ArgArray args) {
    // assert types
    debug("SyncSound(%s, %s)", args[0].u.str, args[1].u.str);
    Common::String *s = new Common::String(args[1].u.str);
    g_private->_nextSetting = s;

    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s, 1);
        //assert(0);
    } else {
        g_private->stopSound();
    }
}

void Quit(ArgArray args) {
    debug("Quit()");
    g_private->quitGame();
}

void LoadGame(ArgArray args) {
    // assert types
    debug("LoadGame(%s, %s)", args[0].u.str, args[2].u.sym->name->c_str());
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, 0, 0, true);
    m->cursor = args[2].u.sym->name;
    m->nextSetting = NULL;
    m->flag = NULL;
    g_private->_loadGameMask = m;
    g_private->_masks.push_front(*m);
}

void SaveGame(ArgArray args) {
    // assert types
    debug("SaveGame(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, 0, 0, true);
    m->cursor = args[1].u.sym->name;
    m->nextSetting = NULL;
    m->flag = NULL;
    g_private->_saveGameMask = m;
    g_private->_masks.push_front(*m);
}

void RestartGame(ArgArray args) {
    // assert types
    g_private->restartGame();
    //debug("WARNING: RestartGame is not implemented");
}

void PoliceBust(ArgArray args) {
    // assert types
    debug("WARNING: PoliceBust is not implemented");
}

void DossierAdd(ArgArray args) {
    // assert types
    debug("WARNING: DossierAdd is not implemented");
}

void NoStopSounds(ArgArray args) {
    // assert types
    debug("WARNING: NoStopSounds is not implemented");
}

void Inventory(ArgArray args) {

    // assert types
    Datum b1 = args[0];
    Datum v1 = args[1];
    Datum v2 = args[2];
    Datum e = args[3];

    Datum snd = args[8];

    assert(v1.type == STRING || v1.type == NAME);
    assert(b1.type == STRING);
    assert(e.type == STRING || e.type == NUM);
    assert(snd.type == STRING);


    if (v1.type == STRING)
        assert(strcmp(v1.u.str, "\"\"") == 0);

    debug("Inventory(...)");

    if (strcmp(b1.u.str, "\"\"") != 0) {
        Common::String *s = new Common::String(b1.u.str);
        MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);

        if (e.type == NUM)
            m->nextSetting = NULL;
        else
            m->nextSetting = new Common::String(e.u.str);

        m->cursor = new Common::String("kInventory");
        m->point = new Common::Point(0,0);
        if (v1.type == NAME)
            m->flag = v2.u.sym;
        else
            m->flag = NULL;

        g_private->_masks.push_front(*m);

    }

    if (v1.type == NAME)
        v1.u.sym->u.val = 1;

    if (strcmp(snd.u.str, "\"\"") != 0) {
        Common::String *s = new Common::String(snd.u.str);
        g_private->playSound(*s, 1);
    }

    // TODO: Keep track of inventory is missing
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
        g_private->playSound(*s, 1);
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
        g_private->playSound(*s, 0);
        //assert(0);
    } else {
        g_private->stopSound();
    }
}


void ViewScreen(ArgArray args) {
    // assert types
    debug("WARNING: ViewScreen not implemented!");
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

void _Mask(ArgArray args, bool drawn) {
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
    m->flag = NULL;
    m->point = new Common::Point(x,y);
    g_private->_masks.push_front(*m);

}

void Mask(ArgArray args) {
    _Mask(args, false);
}
void MaskDrawn(ArgArray args) {
    _Mask(args, true);
}

void AddSound(char *s, char *t, Symbol *flag = NULL, int val = 0) {
    if (strcmp(s, "\"\"") == 0)
        return;

    Common::String *sound = new Common::String(s);
    if (strcmp(t, "AMRadioClip") == 0)
        g_private->_AMRadio.push_back(*sound);
    else if (strcmp(t, "PoliceClip") == 0)
        g_private->_policeRadio.push_back(*sound);
    else if (strcmp(t, "PhoneClip") == 0) {
        PhoneInfo *p = (PhoneInfo*) malloc(sizeof(PhoneInfo));
        p->sound = sound;
        p->flag = flag;
        p->val = val;
        // This condition will avoid adding the same phone call twice,
        // it is unclear why this could be useful, but it looks like a bug
        // in the original script
        if (g_private->_phone.size() > 0 && 
             strcmp(g_private->_phone.back().sound->c_str(), s) == 0)
            return;

        g_private->_phone.push_back(*p);
    }
        
    else
        debug("error: invalid sound type %s", t);
}

void AMRadioClip(ArgArray args) {
    AddSound(args[0].u.str, "AMRadioClip");
}
void PoliceClip(ArgArray args) {
    AddSound(args[0].u.str, "PoliceClip");
}
void PhoneClip(ArgArray args) {
    if (args.size() == 2) {
        debug("Unimplemented PhoneClip special case");
        return;
    }
    AddSound(args[0].u.str, "PhoneClip", args[4].u.sym, args[5].u.val);
}

void SoundArea(ArgArray args) {
    // assert types
    char *n;

    if (args[1].type == NAME)
        n = (char *) args[1].u.sym->name->c_str();
    else if (args[1].type == STRING)
        n = args[1].u.str;
    else
        assert(0);

    debug("SoundArea(%s, %s)", args[0].u.str, n);
    if (strcmp(n, "kAMRadio") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag = NULL;
        g_private->_AMRadioArea = m;
        g_private->_masks.push_front(*m);
    } else if (strcmp(n, "kPoliceRadio") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag = NULL;
        g_private->_policeRadioArea = m;
        g_private->_masks.push_front(*m);
    } else if (strcmp(n, "kPhone") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo*) malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag = NULL;
        g_private->_phoneArea = m;
        g_private->_masks.push_front(*m);
    }
}

void AskSave(ArgArray args) {
    // This is not needed, since scummvm will take care of this
    debug("WARNING: AskSave is partially implemented");
    Common::String *s = new Common::String(args[0].u.str);
    g_private->_nextSetting = s;
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

static struct FuncTable {
    void (*func)(Private::ArgArray);
    const char *name;
} funcTable[] = {

    { ChgMode,         "ChgMode"},
    { Goto,            "goto"},
    { SetFlag,         "SetFlag"},
    { Timer,	       "Timer"},

    // Sounds
    { Sound,           "Sound"},
    { Sound,           "SoundEffect"},
    { LoopedSound,     "LoopedSound"},
    { NoStopSounds,    "NoStopSounds"},
    { SyncSound,       "SyncSound"},
    { AMRadioClip,     "AMRadioClip"},
    { PoliceClip,      "PoliceClip"},
    { PhoneClip,       "PhoneClip"},
    { SoundArea,       "SoundArea"},

    // Images
    { Bitmap,          "Bitmap"},
    { Mask,            "Mask"},
    { MaskDrawn,       "MaskDrawn"},
    { VSPicture,       "VSPicture"},
    { ViewScreen,      "ViewScreen"},

    // Video
    { Transition,      "Transition"},
    { Movie,           "Movie"},

    { SetModifiedFlag, "SetModifiedFlag"},
    { Exit,            "Exit"},
    { Quit,            "Quit"},
    { LoadGame,        "LoadGame"},
    { SaveGame,        "SaveGame"},
    { AskSave,         "AskSave"},
    
    { DossierAdd,      "DossierAdd"},
    { Inventory,       "Inventory"},
    { CRect,           "CRect"},
    { RestartGame,     "RestartGame"},
    { PoliceBust,      "PoliceBust"},
    { 0, 0}
};

NameToPtr _functions;

void initFuncs() {
    for (Private::FuncTable *fnc = funcTable; fnc->name; fnc++) {
        Common::String *name = new Common::String(fnc->name);
        _functions.setVal(*name, (void *)fnc->func);
    }
}

void call(char *name, ArgArray args) {
    Common::String n(name);
    if (!_functions.contains(n)) {
        debug("I don't know how to execute %s", name);
        assert(0);
    }

    void (*func)(ArgArray) = (void (*)(ArgArray)) _functions.getVal(n);
    func(args);

}

}
