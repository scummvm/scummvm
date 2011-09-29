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

#ifndef NEVERHOOD_KLAYMAN_H
#define NEVERHOOD_KLAYMAN_H

#include "neverhood/neverhood.h"
#include "neverhood/sprite.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

// TODO: This code is horrible and weird and a lot of stuff needs renaming once a better name is found

class Klayman;

const uint32 kKlaymanSpeedUpHash = 0x004A2148;

struct KlaymanTableItem {
	int value;
	void (Klayman::*callback)();
};

class Klayman : public AnimatedSprite {
public:
	Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority = 1000, int objectPriority = 1000, NRectArray *clipRects = NULL);

	void update();

	void sub41FD30();
	void sub41FDA0();
	void sub41FDF0();
	void sub41FE60();
	void sub41FEB0();
	void sub41FC80();
	void sub4211F0();
	void sub4211B0();
	void sub41FF80();
	void sub420120();
	void sub420170();
	void sub4200D0();
	void sub41FBC0();
	void sub420870();
	void sub4208B0();
	void sub4208F0();
	void sub420930();
	void sub420830();
	void sub41FC40();
	void sub420210();
	void sub4201C0();
	void sub420340();
	void sub420250();
	void sub420290();
	void sub420380();
	void sub4203C0();
	void sub420300();
	void sub420970();
	void sub4209D0();
	void sub420BC0();
	void sub420AD0();
	void sub421030();
	void sub420FE0();
	void sub4210C0();
	void sub421070();
	void sub420420();
	void sub420ED0();
	void sub420750();
	void sub4207A0();
	void sub4207F0();
	void sub420F20();
	void sub421350();
	void sub4213F0();
	void sub4213B0();
	void sub420060();
	void sub41FFF0();
	void sub4214D0();
	void sub421510();
	void sub421160();
	void sub4212C0();
	void sub421310();
	void sub420600();
	void sub420660();
	void sub4205C0();
	void sub420550();
	void sub420C50();
	void sub420CD0();
	void sub420D10();
	void sub420D50();
	void sub420DA0();
	void sub420DE0();
	void sub420E20();
	void sub420E90();
	void sub420EB0();
	void sub420680();
	void sub41F9E0();
	void sub41FA40();
	void sub41FB30();

	void sub41CE70();
	
	void spriteUpdate41F250();
	void spriteUpdate41F5F0();
	void spriteUpdate41F780();
	void spriteUpdate41F230();
	void spriteUpdate41F5A0();
	void spriteUpdate41F300();
	void spriteUpdate41F320();

	uint32 handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41EB70(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E210(int messageNum, const MessageParam &param, Entity *sender);

	void setKlaymanTable(const KlaymanTableItem *table, int tableCount);
	void setKlaymanTable1();
	void setKlaymanTable2();
	void setKlaymanTable3();
	
	void setSoundFlag(bool value) { _soundFlag = value; }

protected:
	Entity *_parentScene;
	Entity *_attachedSprite;
	int _statusE0;
	bool _flagE1;
	bool _flagE2;
	bool _flagE3;
	bool _flagE4;
	bool _flagE5;
	int16 _x4, _y4;
	int16 _counter, _counterMax;
	int16 _counter3, _counter3Max;
	int16 _counter1;
	int16 _counter2;
	bool _flagF6;
	bool _flagF7;
	bool _flagF8;
	int _status2;
	bool _flagFA;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	int _status3;
	const KlaymanTableItem *_table;
	int _tableCount;
	int _tableMaxValue;
	uint32 _field114;
	/*
	00000118 field118		dw ?
	*/
	bool _soundFlag;
	int _resourceHandle;
	virtual void xUpdate();
	virtual uint32 xHandleMessage(int messageNum, const MessageParam &param);

	void sub41FD40();
	void sub41FD90();
	uint32 handleMessage41EB10(int messageNum, const MessageParam &param, Entity *sender);

	void sub41FDB0();
	uint32 handleMessage41E980(int messageNum, const MessageParam &param, Entity *sender);

	void sub41FE00();
	void sub41FE50();
	uint32 handleMessage41E9E0(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41FE70();
	uint32 handleMessage41EF80(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41FEC0();
	uint32 handleMessage41EFE0(int messageNum, const MessageParam &param, Entity *sender);

	void sub41D320(uint32 fileHash, AnimationCb callback);
	void update41D2B0();

	bool sub41CF10(AnimationCb callback);
	void sub41C7B0();
	void sub41C770();
	void sub41C790();
	
	void update41D0F0();

	void sub41FF00();
	
	void sub41FCF0();
	
	uint32 handleMessage41F140(int messageNum, const MessageParam &param, Entity *sender);

	void sub41C930(int16 x, bool flag);

	uint32 handleMessage41E920(int messageNum, const MessageParam &param, Entity *sender);
	
	bool sub41CEB0(AnimationCb callback3);
	
	void sub41FB40();
	void sub41FBB0();
	uint32 handleMessage41DD80(int messageNum, const MessageParam &param, Entity *sender);
	void sub41CD70(int16 x);
	void sub41F950();
	uint32 handleMessage41EC70(int messageNum, const MessageParam &param, Entity *sender);

	uint32 handleMessage41D4C0(int messageNum, const MessageParam &param, Entity *sender);
	
	uint32 handleMessage41DAD0(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41CD00(int16 x);
	void sub41CC40(int16 x1, int16 x2);
	void sub41CAC0(int16 x);
	void sub41CCE0(int16 x);
	void sub41FC30();
	
	uint32 handleMessage41DF10(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41EEF0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E3C0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D790(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D880(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41DAA0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41DFD0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E0D0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E490(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E290(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41E2F0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D640(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41EAB0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41D970(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage41DD20(int messageNum, const MessageParam &param, Entity *sender);
	
	void update41D1C0();
	
	uint32 handleMessage41DB90(int messageNum, const MessageParam &param, Entity *sender);
	
};

class KmScene1001 : public Klayman {
public:
	KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:	
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub44FA50();
	uint32 handleMessage44FA00(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1002 : public Klayman {
public:
	KmScene1002(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Sprite *class599, Sprite *ssLadderArch);
protected:
	Sprite *_class599;
	Sprite *_ssLadderArch;
	Sprite *_otherSprite;
	int _status;
	void xUpdate();	
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void update4497D0();
	uint32 handleMessage449800(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage4498E0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449990(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449A30(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449BA0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449C90(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449D60(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate449DC0();
	void sub449E20();
	void sub449E90();
	void sub449EF0();
	void sub449F70();
	void sub44A050();
	void sub44A0D0();
	void sub44A150();
	void sub44A230();
	void sub44A250();
	void sub44A2C0();
	void sub44A330();
	void sub44A370();
	void sub44A3C0();
	void sub44A3E0();
	void sub44A460();
	void sub44A4B0();
};

class KmScene1004 : public Klayman {
public:
	KmScene1004(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage478110(int messageNum, const MessageParam &param, Entity *sender);
	void sub478170();
};

class KmScene1109 : public Klayman {
public:
	KmScene1109(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage461EA0(int messageNum, const MessageParam &param, Entity *sender);
	void sub461F30();
	void sub461F70();
};

class KmScene1201 : public Klayman {
public:
	KmScene1201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Entity *class464);
protected:
	Entity *_class464;
	int _countdown;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void update40DBE0();
	uint32 handleMessage40DC00(int messageNum, const MessageParam &param, Entity *sender);
	void sub40DD20();
	void sub40DD90();
	uint32 handleMessage40DDF0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40DEA0(int messageNum, const MessageParam &param, Entity *sender);
	void sub40DF00();
	void sub40DF60();
	void sub40DFA0();
	void sub40E040();
};

class KmScene1303 : public Klayman {
public:
	KmScene1303(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage4160A0(int messageNum, const MessageParam &param, Entity *sender);
	void update4161A0();
	void sub4161D0();
	void sub416210();
	void sub416250();
	void sub4162B0();
};

class KmScene1304 : public Klayman {
public:
	KmScene1304(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1305 : public Klayman {
public:
	KmScene1305(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub46BEF0();
	void sub46BF60();
};

class KmScene1306 : public Klayman {
public:
	KmScene1306(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage417CB0(int messageNum, const MessageParam &param, Entity *sender);
	void sub417D40();
	void sub417D80();
};

class KmScene1308 : public Klayman {
public:
	KmScene1308(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub456150();
};

class KmScene1401 : public Klayman {
public:
	KmScene1401(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1402 : public Klayman {
public:
	KmScene1402(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1403 : public Klayman {
public:
	KmScene1403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1404 : public Klayman {
public:
	KmScene1404(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1608 : public Klayman {
public:
	KmScene1608(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene1705 : public Klayman {
public:
	KmScene1705(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage4689A0(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate468A30();
	void sub468A80();
	void sub468AD0();
	void sub468B10();
};

class KmScene1901 : public Klayman {
public:
	KmScene1901(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2001 : public Klayman {
public:
	KmScene2001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage4401A0(int messageNum, const MessageParam &param, Entity *sender);
	void sub440230();
	void sub440270();
};

class KmScene2101 : public Klayman {
public:
	KmScene2101(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	bool _flag1;
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	uint32 handleMessage486160(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage486230(int messageNum, const MessageParam &param, Entity *sender);
	void sub4862C0();
	void sub486320();
	void sub486360();
};

class KmScene2201 : public Klayman {
public:
	KmScene2201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2203 : public Klayman {
public:
	KmScene2203(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2205 : public Klayman {
public:
	KmScene2205(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
	void sub423980();
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
};

class KmScene2206 : public Klayman {
public:
	KmScene2206(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
	~KmScene2206();
protected:
	int16 _yDelta;
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void spriteUpdate482450();
	void sub482490();
	void sub482530();
};

class KmScene2207 : public Klayman {
public:
	KmScene2207(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void spriteUpdate442430();
	void sub442460();
	void sub4424B0();
	void sub442520();
	void sub442560();
	void sub4425A0();
};

class KmScene2242 : public Klayman {
public:
	KmScene2242(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub444D20();
};

class KmHallOfRecords : public Klayman {
public:
	KmHallOfRecords(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub43B130();
};

class KmScene2247 : public Klayman {
public:
	KmScene2247(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y);
protected:
	void xUpdate();
	uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void sub453520();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_KLAYMAN_H */
