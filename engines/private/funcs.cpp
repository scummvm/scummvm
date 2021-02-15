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

#include "common/str.h"
#include "common/timer.h"
#include "common/debug.h"
#include "common/system.h"

#include "private/grammar.h"
#include "private/tokens.h"
#include "private/private.h"

namespace Private {

void ChgMode(ArgArray args) {
    // assert types
    assert (args.size() == 2 || args.size() == 3);
    if (args.size() == 2)
        debugC(1, kPrivateDebugScript, "ChgMode(%d, %s)", args[0].u.val, args[1].u.str);
    else if (args.size() == 3)
        debugC(1, kPrivateDebugScript, "ChgMode(%d, %s, %s)", args[0].u.val, args[1].u.str, args[2].u.sym->name->c_str());
    else
        assert(0);

    g_private->_mode = args[0].u.val;
    Common::String *s = new Common::String(args[1].u.str);
    g_private->_nextSetting = s;

    if (g_private->_mode == 0) {
        g_private->_origin = &kPrivateOriginZero;
    } else if (g_private->_mode == 1) {
        g_private->_origin = &kPrivateOriginOne;
    } else
        assert(0);

    if (args.size() == 3)
        setSymbol(args[2].u.sym, true);

    // This is the only place where this should be used
    if (g_private->_noStopSounds) {
        g_private->_noStopSounds = false;
    } else {
        g_private->stopSound(true);
    }
}

void VSPicture(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "VSPicture(%s)", args[0].u.str);
    g_private->_nextVS = new Common::String(args[0].u.str);
}


void DiaryLocList(ArgArray args) {
    int x1, y1, x2, y2;

    debugC(1, kPrivateDebugScript, "DiaryLocList(%d, %d, %d, %d)", args[0].u.val, args[1].u.val, args[2].u.val, args[3].u.val);

    x2 = args[0].u.val;
    y2 = args[1].u.val;

    x1 = args[2].u.val;
    y1 = args[3].u.val;

    Common::Rect *rect = new Common::Rect(x1, y1, x2, y2);
    g_private->loadLocations(rect);

}

void DiaryGoLoc(ArgArray args) {
    debugC(1, kPrivateDebugScript, "WARNING: DiaryGoLoc not implemented");
}

void DiaryInvList(ArgArray args) {
    Common::Rect *r1, *r2;

    debugC(1, kPrivateDebugScript, "DiaryInvList(%d, ..)", args[0].u.val);

    r1 = args[1].u.rect;
    r2 = args[2].u.rect;

    g_private->loadInventory(args[0].u.val, r1, r2);
}

void Goto(ArgArray args) { // should be goto, but this is a reserved word
    // assert types
    debugC(1, kPrivateDebugScript, "goto(%s)", args[0].u.str);
    Common::String *s = new Common::String(args[0].u.str);
    g_private->_nextSetting = s;
}


void SyncSound(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "SyncSound(%s, %s)", args[0].u.str, args[1].u.str);
    Common::String *nextSetting = new Common::String(args[1].u.str);
    g_private->_nextSetting = nextSetting;

    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s, 1, true, false);
        //assert(0);
    }
}

void Quit(ArgArray args) {
    debugC(1, kPrivateDebugScript, "Quit()");
    g_private->quitGame();
}

void LoadGame(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "LoadGame(%s, %s)", args[0].u.str, args[2].u.sym->name->c_str());
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, 0, 0, true);
    m->cursor = args[2].u.sym->name;
    m->nextSetting = NULL;
    m->flag1 = NULL;
    m->flag2 = NULL;
    g_private->_loadGameMask = m;
    g_private->_masks.push_front(*m);
}

void SaveGame(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "SaveGame(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, 0, 0, true);
    m->cursor = args[1].u.sym->name;
    m->nextSetting = NULL;
    m->flag1 = NULL;
    m->flag2 = NULL;
    g_private->_saveGameMask = m;
    g_private->_masks.push_front(*m);
}

void RestartGame(ArgArray args) {
    assert(args.size() == 0);
    g_private->restartGame();
}

void PoliceBust(ArgArray args) {
    // assert types
    assert (args.size() == 1 || args.size() == 2);
    g_private->_policeBustEnabled = args[0].u.val;
    //debug("Number of clicks %d", g_private->computePoliceIndex());

    if (g_private->_policeBustEnabled)
        g_private->startPoliceBust();

    if (args.size() == 2) {
        if (args[1].u.val == 2) {
            // Unclear what it means
        } else if (args[1].u.val == 3) {
            g_private->_nextSetting = &kMainDesktop;
            g_private->_mode = 0;
            g_private->_origin = &kPrivateOriginZero;
        } else
            assert(0);
    }
    debugC(1, kPrivateDebugScript, "PoliceBust(%d, ..)", args[0].u.val);
    debugC(1, kPrivateDebugScript, "WARNING: PoliceBust partially implemented");
}

void BustMovie(ArgArray args) {
    // assert types
    assert (args.size() == 1);
    debugC(1, kPrivateDebugScript, "BustMovie(%s)", args[0].u.str);
    uint policeIndex = variables.getVal(kPoliceIndex)->u.val;
    int videoIndex = policeIndex/2 - 1;
    if (videoIndex < 0)
        videoIndex = 0;
    assert(videoIndex <= 5);

    char f[32];
    snprintf(f, 32, "po/animatio/spoc%02dxs.smk", kPoliceBustVideos[videoIndex]);

    if (kPoliceBustVideos[videoIndex] == 2) {
        Common::String *s = new Common::String("global/transiti/audio/spoc02VO.wav");
        g_private->playSound(*s, 1, false, false);
    }

    Common::String *pv = new Common::String(f);
    g_private->_nextMovie = pv;
    g_private->_nextSetting = new Common::String(args[0].u.str);
}

void DossierAdd(ArgArray args) {

    assert (args.size() == 2);
    Common::String *s1 = new Common::String(args[0].u.str);
    DossierInfo *m = (DossierInfo *)malloc(sizeof(DossierInfo));
    m->page1 = s1;

    if (strcmp(args[1].u.str, "\"\"") != 0) {
        Common::String *s2 = new Common::String(args[1].u.str);
        m->page2 = s2;

    } else {
        m->page2 = NULL;
    }

    g_private->_dossiers.push_back(*m);
}

void DossierBitmap(ArgArray args) {

    assert (args.size() == 2);

    int x = args[0].u.val;
    int y = args[1].u.val;

    assert(x == 40 && y == 30);

    g_private->loadDossier();

}

void DossierChgSheet(ArgArray args) {
    debugC(1, kPrivateDebugScript, "WARNING: DossierChgSheet is not implemented");
}

void DossierPrevSuspect(ArgArray args) {
    assert (args.size() == 3);
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));

    int x = args[1].u.val;
    int y = args[2].u.val;

    m->surf = g_private->loadMask(*s, x, y, true);
    m->cursor = new Common::String("kExit");
    m->nextSetting = NULL;
    m->flag1 = NULL;
    m->flag2 = NULL;
    g_private->_dossierPrevSuspectMask = m;
    g_private->_masks.push_front(*m);
}

void DossierNextSuspect(ArgArray args) {
    assert (args.size() == 3);
    Common::String *s = new Common::String(args[0].u.str);
    MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));

    int x = args[1].u.val;
    int y = args[2].u.val;

    m->surf = g_private->loadMask(*s, x, y, true);
    m->cursor = new Common::String("kExit");
    m->nextSetting = NULL;
    m->flag1 = NULL;
    m->flag2 = NULL;
    g_private->_dossierNextSuspectMask = m;
    g_private->_masks.push_front(*m);
}

void NoStopSounds(ArgArray args) {
    assert(args.size() == 0);
    debugC(1, kPrivateDebugScript, "NoStopSounds()");
    g_private->_noStopSounds = true;
}

void LoseInventory(ArgArray args) {
    assert(args.size() == 0);
    debugC(1, kPrivateDebugScript, "LoveInventory()");
    g_private->inventory.clear();
}

void Inventory(ArgArray args) {

    // assert types
    Datum b1 = args[0];
    Datum v1 = args[1];
    Datum v2 = args[2];
    Datum e = args[3];
    Datum i = args[4];
    Datum c = args[5];

    Datum snd = args[8];

    assert(v1.type == STRING || v1.type == NAME);
    assert(b1.type == STRING);
    assert(e.type == STRING || e.type == NUM);
    assert(snd.type == STRING);
    assert(i.type == STRING);

    Common::String *bmp = new Common::String(i.u.str);
    assert(g_private->isDemo() || strcmp(bmp->c_str(), "\"\"") != 0);


    if (v1.type == STRING)
        assert(strcmp(v1.u.str, "\"\"") == 0);

    debugC(1, kPrivateDebugScript, "Inventory(...)");

    if (strcmp(b1.u.str, "\"\"") != 0) {
        Common::String *s = new Common::String(b1.u.str);
        MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);

        if (e.type == NUM)
            m->nextSetting = NULL;
        else
            m->nextSetting = new Common::String(e.u.str);

        m->cursor = new Common::String("kInventory");
        m->point = new Common::Point(0,0);

        if (v1.type == NAME)
            m->flag1 = v1.u.sym;
        else
            m->flag1 = NULL;


        if (v2.type == NAME)
            m->flag2 = v2.u.sym;
        else
            m->flag2 = NULL;

        g_private->_masks.push_front(*m);
        g_private->_toTake = true;

        Common::String *f;

        if (strcmp(snd.u.str, "\"\"") != 0) {
            f = new Common::String(snd.u.str);
        } else {
            f = g_private->getTakeLeaveSound();
        }
        g_private->playSound(*f, 1, false, false);
        g_private->inventory.push_back(*bmp);
    } else {
        if (v1.type == NAME) {
            if (strcmp(c.u.str, "\"REMOVE\"") == 0) {
                v1.u.sym->u.val = 0;
                g_private->inventory.remove(*bmp);
            } else {
                v1.u.sym->u.val = 1;
                g_private->inventory.push_back(*bmp);
            }
        } else {
            g_private->inventory.push_back(*bmp);
        }

        if (v2.type == NAME)
            v2.u.sym->u.val = 1;
    }

}

void SetFlag(ArgArray args) {
    assert(args.size() == 2);
    assert(args[0].type == NAME && args[1].type == NUM);
    debugC(1, kPrivateDebugScript, "SetFlag(%s, %d)", args[0].u.sym->name->c_str(), args[1].u.val);
    args[0].u.sym->u.val = args[1].u.val;
}

void Exit(ArgArray args) {
    // assert types
    assert(args[2].type == RECT || args[2].type == NAME);
    debugC(1, kPrivateDebugScript, "Exit(%d %d %d)", args[0].type, args[1].type, args[2].type); //, args[0].u.str, args[1].u.sym->name->c_str(), "RECT");
    ExitInfo *e = (ExitInfo *)malloc(sizeof(ExitInfo));

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
    //debug("Rect %d %d %d %d", args[2].u.rect->top, args[2].u.rect->left, args[2].u.rect->bottom, args[2].u.rect->right);
    g_private->_exits.push_front(*e);
}

void SetModifiedFlag(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "SetModifiedFlag(%d)", args[0].u.val);
    g_private->_modified = args[0].u.val != 0;
}

void PaperShuffleSound(ArgArray args) {
    assert(args.size() == 0);
    debugC(1, kPrivateDebugScript, "PaperShuffleSound()");
    Common::String *s = g_private->getPaperShuffleSound();
    g_private->playSound(*s, 1, false, false);
}

void SoundEffect(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "SoundEffect(%s)", args[0].u.str);
    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s, 1, false, false);
        //assert(0);
    } else {
        g_private->stopSound(true);
    }
}

void Sound(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "Sound(%s)", args[0].u.str);
    if (args.size() == 4) {
        bool b1 = args[1].u.val != 0;
        bool b2 = args[2].u.val != 0;
        int c = args[3].u.val;

        if (!b1 && !b2 && c == 1) {
            g_private->stopSound(true);
        } else if (!b1 && !b2 && c == 2) {
            g_private->stopSound(false);
        } else
            assert(0);
    }

    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s, 1, false, false);
        //assert(0);
    } else {
        g_private->stopSound(true);
    }
}

void LoopedSound(ArgArray args) {
    // assert types
    assert(args.size() == 1);
    debugC(1, kPrivateDebugScript, "LoopedSound(%s)", args[0].u.str);
    if (strcmp("\"\"", args[0].u.str) != 0) {
        Common::String *s = new Common::String(args[0].u.str);
        g_private->playSound(*s, 0, true, true);
        //assert(0);
    } else {
        g_private->stopSound(true);
    }
}


void ViewScreen(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "WARNING: ViewScreen not implemented!");
}

void Transition(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "Transition(%s, %s)", args[0].u.str, args[1].u.str);
    g_private->_nextMovie = new Common::String(args[0].u.str);
    g_private->_nextSetting = new Common::String(args[1].u.str);
}

void Resume(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "Resume(%d)", args[0].u.val); // this value is always 1
    g_private->_nextSetting = g_private->_pausedSetting;
    g_private->_pausedSetting = NULL;
    g_private->_mode = 1;
    g_private->_origin = &kPrivateOriginOne;
}

void Movie(ArgArray args) {
    // assert types
    debugC(1, kPrivateDebugScript, "Movie(%s, %s)", args[0].u.str, args[1].u.str);
    Common::String *movie = new Common::String(args[0].u.str);
    Common::String *nextSetting = new Common::String(args[1].u.str);
    bool isEmptyString = strcmp(args[0].u.str, "\"\"") == 0;

    if (!g_private->_playedMovies.contains(*movie) && !isEmptyString) {
        g_private->_nextMovie = movie;
        g_private->_playedMovies.setVal(*movie, true);
        g_private->_nextSetting = nextSetting;

    } else if (isEmptyString) {
        g_private->_repeatedMovieExit = nextSetting;
        debugC(1, kPrivateDebugScript, "repeated movie exit is %s", nextSetting->c_str());
    } else {
        debugC(1, kPrivateDebugScript, "movie %s already played", movie->c_str());
        g_private->_nextSetting = g_private->_repeatedMovieExit;
    }
    //g_private->_nextSetting = new Common::String(args[1].u.str);
}

void CRect(ArgArray args) {
    // assert types
    int x1, y1, x2, y2;

    debugC(1, kPrivateDebugScript, "CRect(%d, %d, %d, %d)", args[0].u.val, args[1].u.val, args[2].u.val, args[3].u.val);

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

    debugC(1, kPrivateDebugScript, "Bitmap(%s, %d, %d)", f, x, y);
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

    debugC(1, kPrivateDebugScript, "Mask(%s, %s, %s, %d, %d)", f, e, c->c_str(), x, y);
    const Common::String *s = new Common::String(f);
    //if (drawed)
    //    g_private->loadImage(*s, x, y);

    MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
    m->surf = g_private->loadMask(*s, x, y, drawn);
    m->nextSetting = new Common::String(e);
    m->cursor = c;
    m->flag1 = NULL;
    m->flag2 = NULL;
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
        // This condition will avoid adding the same phone call twice,
        // it is unclear why this could be useful, but it looks like a bug
        // in the original scripts
        if (g_private->_playedPhoneClips.contains(*sound))
            return;

        g_private->_playedPhoneClips.setVal(*sound, true);
        PhoneInfo *p = (PhoneInfo *)malloc(sizeof(PhoneInfo));
        p->sound = sound;
        p->flag = flag;
        p->val = val;
        g_private->_phone.push_back(*p);
    }

    else
        error("error: invalid sound type %s", t);
}

void AMRadioClip(ArgArray args) {
    assert(args.size() <= 4);
    AddSound(args[0].u.str, "AMRadioClip");
}
void PoliceClip(ArgArray args) {
    assert(args.size() <= 4);
    AddSound(args[0].u.str, "PoliceClip");
}
void PhoneClip(ArgArray args) {
    if (args.size() == 2) {
        debugC(1, kPrivateDebugScript, "Unimplemented PhoneClip special case");
        return;
    }
    int i = args[2].u.val;
    int j = args[3].u.val;

    if (i == j)
        AddSound(args[0].u.str, "PhoneClip", args[4].u.sym, args[5].u.val);
    else {
        assert(i < j);
        char *clip = g_private->getRandomPhoneClip(args[0].u.str, i, j);
        AddSound(clip, "PhoneClip", args[4].u.sym, args[5].u.val);
    }

}

void SoundArea(ArgArray args) {
    // assert types
    char *n;

    if (args[1].type == NAME)
        n = (char *)args[1].u.sym->name->c_str();
    else if (args[1].type == STRING)
        n = args[1].u.str;
    else
        assert(0);

    debugC(1, kPrivateDebugScript, "SoundArea(%s, %s)", args[0].u.str, n);
    if (strcmp(n, "kAMRadio") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag1 = NULL;
        m->flag2 = NULL;
        g_private->_AMRadioArea = m;
        g_private->_masks.push_front(*m);
    } else if (strcmp(n, "kPoliceRadio") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag1 = NULL;
        m->flag2 = NULL;
        g_private->_policeRadioArea = m;
        g_private->_masks.push_front(*m);
    } else if (strcmp(n, "kPhone") == 0) {
        Common::String *s = new Common::String(args[0].u.str);
        MaskInfo *m = (MaskInfo *)malloc(sizeof(MaskInfo));
        m->surf = g_private->loadMask(*s, 0, 0, true);
        m->cursor = args[2].u.sym->name;
        m->nextSetting = NULL;
        m->flag1 = NULL;
        m->flag2 = NULL;
        g_private->_phoneArea = m;
        g_private->_masks.push_front(*m);
    }
}

void SafeDigit(ArgArray args) {
    // This is not needed, since scummvm will take care of this
    debugC(1, kPrivateDebugScript, "WARNING: SafeDigit is not implemented");
}

void AskSave(ArgArray args) {
    // This is not needed, since scummvm will take care of this
    debugC(1, kPrivateDebugScript, "WARNING: AskSave is partially implemented");
    Common::String *s = new Common::String(args[0].u.str);
    g_private->_nextSetting = s;
}

void Timer(ArgArray args) {
    assert (args.size() == 2 || args.size() == 3);

    if (args.size() == 3)
        debugC(1, kPrivateDebugScript, "Timer(%d, %s, %s)", args[0].u.val, args[1].u.str, args[2].u.str);
    else
        debugC(1, kPrivateDebugScript, "Timer(%d, %s)", args[0].u.val, args[1].u.str);

    //g_system->delayMillis(1000 * args[0].u.val);
    int32 delay = 1000000 * args[0].u.val;
    Common::String *s = new Common::String(args[1].u.str);
    if (delay > 0) {
        assert(g_private->installTimer(delay, s));
    } else if (delay == 0) {
        g_private->_nextSetting = s;
    } else {
        assert(0);
    }

}

static struct FuncTable {
    void (*func)(Private::ArgArray);
    const char *name;
} funcTable[] = {

    { ChgMode,         "ChgMode"},
    { Resume,          "Resume"},
    { Goto,            "goto"},
    { SetFlag,         "SetFlag"},
    { SetModifiedFlag, "SetModifiedFlag"},
    { Timer,           "Timer"},

    // Sounds
    { Sound,           "Sound"},
    { SoundEffect,     "SoundEffect"},
    { LoopedSound,     "LoopedSound"},
    { NoStopSounds,    "NoStopSounds"},
    { SyncSound,       "SyncSound"},
    { AMRadioClip,     "AMRadioClip"},
    { PoliceClip,      "PoliceClip"},
    { PhoneClip,       "PhoneClip"},
    { SoundArea,       "SoundArea"},
    { PaperShuffleSound, "PaperShuffleSound"},

    // Images
    { Bitmap,          "Bitmap"},
    { Mask,            "Mask"},
    { MaskDrawn,       "MaskDrawn"},
    { VSPicture,       "VSPicture"},
    { ViewScreen,      "ViewScreen"},

    // Video
    { Transition,      "Transition"},
    { Movie,           "Movie"},

    // Diary
    { DiaryLocList,    "DiaryLocList"},
    { DiaryInvList,    "DiaryInvList"},
    { DiaryGoLoc,      "DiaryGoLoc"},

    { Exit,            "Exit"},
    { Quit,            "Quit"},
    { LoadGame,        "LoadGame"},
    { SaveGame,        "SaveGame"},
    { AskSave,         "AskSave"},

    // Dossiers
    { DossierAdd,      "DossierAdd"},
    { DossierChgSheet, "DossierChgSheet"},
    { DossierBitmap,   "DossierBitmap"},
    { DossierPrevSuspect, "DossierPrevSuspect"},
    { DossierNextSuspect, "DossierNextSuspect"},

    { LoseInventory,   "LoseInventory"},
    { Inventory,       "Inventory"},
    { CRect,           "CRect"},
    { RestartGame,     "RestartGame"},

    // PoliceBust
    { PoliceBust,      "PoliceBust"},
    { BustMovie,       "BustMovie"},

    // Others

    { SafeDigit,       "SafeDigit"},

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
        error("I don't know how to execute %s", name);
    }

    void (*func)(ArgArray) = (void (*)(ArgArray)) _functions.getVal(n);
    func(args);

}

} // End of namespace Private
