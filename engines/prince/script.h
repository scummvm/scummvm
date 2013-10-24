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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_SCRIPT_H
#define PRINCE_SCRIPT_H

#include "common/random.h"

namespace Common {
    class SeekableReadStream;
}

namespace Prince {

class PrinceEngine;

class Script
{
public:
    Script(PrinceEngine *vm);
    virtual ~Script();

    bool loadFromStream(Common::SeekableReadStream &stream);

    void step();

private:
    PrinceEngine *_vm;

    byte *_code;
    uint32 _codeSize;
    uint32 _currentInstruction;
    uint16 _lastOpcode;
    uint32 _lastInstruction;
    byte _result;
    int16 _flags[2000];
    bool _opcodeNF;

    // Stack
    uint16 _stack[500];
    uint8 _stacktop;
    uint8 _savedStacktop;

    // Helper functions
    void checkPC(uint32 address);
    uint8 getCodeByte(uint32 address);
    uint8 readScript8bits();
    uint16 readScript16bits();
    uint32 readScript32bits();
    uint16 readScript8or16bits();
    void debugScript(const char *s, ...);

    typedef void (Script::*OpcodeFunc)();
    static OpcodeFunc _opcodes[];

    void O_WAITFOREVER();
    void O_BLACKPALETTE();
    void O_SETUPPALETTE();
    void O_INITROOM();
    void O_SETSAMPLE();
    void O_FREESAMPLE();
    void O_PLAYSAMPLE();
    void O_PUTOBJECT();
    void O_REMOBJECT();
    void O_SHOWANIM();
    void O_CHECKANIMEND();
    void O_FREEANIM();
    void O_CHECKANIMFRAME();
    void O_PUTBACKANIM();
    void O_REMBACKANIM();
    void O_CHECKBACKANIMFRAME();
    void O_FREEALLSAMPLES();
    void O_SETMUSIC();
    void O_STOPMUSIC();
    void O__WAIT();
    void O_UPDATEOFF();
    void O_UPDATEON();
    void O_UPDATE ();
    void O_CLS();
    void O__CALL();
    void O_RETURN();
    void O_GO();
    void O_BACKANIMUPDATEOFF();
    void O_BACKANIMUPDATEON();
    void O_CHANGECURSOR();
    void O_CHANGEANIMTYPE();
    void O__SETFLAG();
    void O_COMPARE();
    void O_JUMPZ();
    void O_JUMPNZ();
    void O_EXIT();
    void O_ADDFLAG();
    void O_TALKANIM();
    void O_SUBFLAG();
    void O_SETSTRING();
    void O_ANDFLAG();
    void O_GETMOBDATA();
    void O_ORFLAG();
    void O_SETMOBDATA();
    void O_XORFLAG();
    void O_GETMOBTEXT();
    void O_MOVEHERO();
    void O_WALKHERO();
    void O_SETHERO();
    void O_HEROOFF();
    void O_HEROON();
    void O_CLSTEXT();
    void O_CALLTABLE();
    void O_CHANGEMOB();
    void O_ADDINV();
    void O_REMINV();
    void O_REPINV();
    void O_OBSOLETE_GETACTION();
    void O_ADDWALKAREA();
    void O_REMWALKAREA();
    void O_RESTOREWALKAREA();
    void O_WAITFRAME();
    void O_SETFRAME();
    void O_RUNACTION();
    void O_COMPAREHI();
    void O_COMPARELO();
    void O_PRELOADSET();
    void O_FREEPRELOAD();
    void O_CHECKINV();
    void O_TALKHERO();
    void O_WAITTEXT();
    void O_SETHEROANIM();
    void O_WAITHEROANIM();
    void O_GETHERODATA();
    void O_GETMOUSEBUTTON();
    void O_CHANGEFRAMES();
    void O_CHANGEBACKFRAMES();
    void O_GETBACKANIMDATA();
    void O_GETANIMDATA();
    void O_SETBGCODE();
    void O_SETBACKFRAME();
    void O_GETRND();
    void O_TALKBACKANIM();
    void O_LOADPATH();
    void O_GETCHAR();
    void O_SETDFLAG();
    void O_CALLDFLAG();
    void O_PRINTAT();
    void O_ZOOMIN();
    void O_ZOOMOUT();
    void O_SETSTRINGOFFSET();
    void O_GETOBJDATA();
    void O_SETOBJDATA();
    void O_SWAPOBJECTS();
    void O_CHANGEHEROSET();
    void O_ADDSTRING();
    void O_SUBSTRING();
    void O_INITDIALOG();
    void O_ENABLEDIALOGOPT();
    void O_DISABLEDIALOGOPT();
    void O_SHOWDIALOGBOX();
    void O_STOPSAMPLE();
    void O_BACKANIMRANGE();
    void O_CLEARPATH();
    void O_SETPATH();
    void O_GETHEROX();
    void O_GETHEROY();
    void O_GETHEROD();
    void O_PUSHSTRING();
    void O_POPSTRING();
    void O_SETFGCODE();
    void O_STOPHERO();
    void O_ANIMUPDATEOFF();
    void O_ANIMUPDATEON();
    void O_FREECURSOR();
    void O_ADDINVQUIET();
    void O_RUNHERO();
    void O_SETBACKANIMDATA();
    void O_VIEWFLC();
    void O_CHECKFLCFRAME();
    void O_CHECKFLCEND();
    void O_FREEFLC();
    void O_TALKHEROSTOP();
    void O_HEROCOLOR();
    void O_GRABMAPA();
    void O_ENABLENAK();
    void O_DISABLENAK();
    void O_GETMOBNAME();
    void O_SWAPINVENTORY();
    void O_CLEARINVENTORY();
    void O_SKIPTEXT();
    void O_SETVOICEH();
    void O_SETVOICEA();
    void O_SETVOICEB();
    void O_SETVOICEC();
    void O_VIEWFLCLOOP();
    void O_FLCSPEED();
    void O_OPENINVENTORY();
    void O_KRZYWA();
    void O_GETKRZYWA();
    void O_GETMOB();
    void O_INPUTLINE();
    void O_SETVOICED();
    void O_BREAK_POINT();
};

}

#endif
