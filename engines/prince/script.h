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

    Common::RandomSource _random;

    byte *_code;
    uint16 _codeSize;
    uint32 _currentInstruction;
    uint16 _lastOpcode;
    uint32 _lastInstruction;

    // Stack
    uint16 _stack[500];
    uint8 _stacktop;
    uint8 _savedStacktop;

    // Helper functions
    uint8 getCodeByte(uint16 address);
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


#if 0
    O_WAITFOREVER       ;00
    O_BLACKPALETTE      ;01
    O_SETUPPALETTE      ;02
    O_INITROOM      ;03
    O_SETSAMPLE     ;04
    O_FREESAMPLE        ;05
    O_PLAYSAMPLE        ;06
    O_PUTOBJECT     ;07
    O_REMOBJECT     ;08
    O_SHOWANIM      ;09
    O_CHECKANIMEND      ;10
    O_FREEANIM      ;11
    O_CHECKANIMFRAME    ;12
    O_PUTBACKANIM       ;13
    O_REMBACKANIM       ;14
    O_CHECKBACKANIMFRAME    ;15
    O_FREEALLSAMPLES    ;16
    O_SETMUSIC      ;17
    O_STOPMUSIC     ;18
    O__WAIT         ;19
    O_UPDATEOFF     ;20
    O_UPDATEON      ;21
    O_UPDATE        ;22
    O_CLS           ;23
    O__CALL         ;24
    O_RETURN        ;25
    O_GO            ;26
    O_BACKANIMUPDATEOFF ;27
    O_BACKANIMUPDATEON  ;28
    O_CHANGECURSOR      ;29
    O_CHANGEANIMTYPE    ;30
    O__SETFLAG      ;31
    O_COMPARE       ;32
    O_JUMPZ         ;33
    O_JUMPNZ        ;34
    O_EXIT          ;35
    O_ADDFLAG       ;36
    O_TALKANIM      ;37
    O_SUBFLAG       ;38
    O_SETSTRING     ;39
    O_ANDFLAG       ;40
    O_GETMOBDATA        ;41
    O_ORFLAG        ;42
    O_SETMOBDATA        ;43
    O_XORFLAG       ;44
    O_GETMOBTEXT        ;45
    O_MOVEHERO      ;46
    O_WALKHERO      ;47
    O_SETHERO       ;48
    O_HEROOFF       ;49
    O_HEROON        ;50
    O_CLSTEXT       ;51
    O_CALLTABLE     ;52
    O_CHANGEMOB     ;53
    O_ADDINV        ;54
    O_REMINV        ;55
    O_REPINV        ;56
    O_OBSOLETE_GETACTION    ;57
    O_ADDWALKAREA       ;58
    O_REMWALKAREA       ;59
    O_RESTOREWALKAREA   ;60
    O_WAITFRAME     ;61
    O_SETFRAME      ;62
    O_RUNACTION     ;63
    O_COMPAREHI     ;64
    O_COMPARELO     ;65
    O_PRELOADSET        ;66
    O_FREEPRELOAD       ;67
    O_CHECKINV      ;68
    O_TALKHERO      ;69
    O_WAITTEXT      ;70
    O_SETHEROANIM       ;71
    O_WAITHEROANIM      ;72
    O_GETHERODATA       ;73
    O_GETMOUSEBUTTON    ;74
    O_CHANGEFRAMES      ;75
    O_CHANGEBACKFRAMES  ;76
    O_GETBACKANIMDATA   ;77
    O_GETANIMDATA       ;78
    O_SETBGCODE     ;79
    O_SETBACKFRAME      ;80
    O_GETRND        ;81
    O_TALKBACKANIM      ;82
    O_LOADPATH      ;83
    O_GETCHAR       ;84
    O_SETDFLAG      ;85
    O_CALLDFLAG     ;86
    O_PRINTAT       ;87
    O_ZOOMIN        ;88
    O_ZOOMOUT       ;89
    O_SETSTRINGOFFSET   ;90
    O_GETOBJDATA        ;91
    O_SETOBJDATA        ;92
    O_SWAPOBJECTS       ;93
    O_CHANGEHEROSET     ;94
    O_ADDSTRING     ;95
    O_SUBSTRING     ;96
    O_INITDIALOG        ;97
    O_ENABLEDIALOGOPT   ;98
    O_DISABLEDIALOGOPT  ;99
    O_SHOWDIALOGBOX     ;100
    O_STOPSAMPLE        ;101
    O_BACKANIMRANGE     ;102
    O_CLEARPATH     ;103
    O_SETPATH       ;104
    O_GETHEROX      ;105
    O_GETHEROY      ;106
    O_GETHEROD      ;107
    O_PUSHSTRING        ;108
    O_POPSTRING     ;109
    O_SETFGCODE     ;110
    O_STOPHERO      ;111
    O_ANIMUPDATEOFF     ;112
    O_ANIMUPDATEON      ;113
    O_FREECURSOR        ;114
    O_ADDINVQUIET       ;115
    O_RUNHERO       ;116
    O_SETBACKANIMDATA   ;117
    O_VIEWFLC       ;118
    O_CHECKFLCFRAME     ;119
    O_CHECKFLCEND       ;120
    O_FREEFLC       ;121
    O_TALKHEROSTOP      ;122
    O_HEROCOLOR     ;123
    O_GRABMAPA      ;124
    O_ENABLENAK     ;125
    O_DISABLENAK        ;126
    O_GETMOBNAME        ;127
    O_SWAPINVENTORY     ;128
    O_CLEARINVENTORY    ;129
    O_SKIPTEXT      ;130
    O_SETVOICEH     ;131
    O_SETVOICEA     ;132
    O_SETVOICEB     ;133
    O_SETVOICEC     ;134
    O_VIEWFLCLOOP       ;135
    O_FLCSPEED      ;136
    O_OPENINVENTORY     ;137
    O_KRZYWA        ;138
    O_GETKRZYWA     ;139
    O_GETMOB        ;140
    O_INPUTLINE     ;141
    O_SETVOICED     ;142
    O_BREAK_POINT       ;143
#endif
};

}

#endif
