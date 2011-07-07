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

struct KlaymanTableItem {
	int value;
	void (Klayman::*callback)();
};

class Klayman : public AnimatedSprite {
public:
	Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority, int objectPriority);

	void sub41FD30();
	void sub41FDA0();
	void sub41FDF0();
	void sub41FE60();
	void sub41FEB0();

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
	int _status;
	const KlaymanTableItem *_table;
	int _tableCount;
	int _tableMaxValue;
	/*
	00000114 field114		dd ?
	00000118 field118		dw ?
	*/
	bool _soundFlag;
	int _resourceHandle;
	virtual void xUpdate();
	virtual uint32 xHandleMessage(int messageNum, const MessageParam &param);
	void update();

	void setKlaymanTable(const KlaymanTableItem *table, int tableCount);
	void setKlaymanTable1();
	void setKlaymanTable2();
	void setKlaymanTable3();

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
	
	void sub41FC80();
	void update41D0F0();
	uint32 handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender);

	void sub41FF00();
	uint32 handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender);
	
	void sub41FCF0();
	
	uint32 handleMessage41F140(int messageNum, const MessageParam &param, Entity *sender);

	void sub41C930(int16 x, bool flag);
	
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_KLAYMAN_H */
