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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_SNAIL_H
#define CGE_SNAIL_H

#include "cge/cge.h"

namespace CGE {

#define kSnailFrameRate 80
#define kSnailFrameDelay (1000 / kSnailFrameRate)
#define kDressed 3

enum SnCom {
	kSnLabel,  kSnPause,  kSnWait,        kSnLevel,       kSnHide,
	kSnSay,    kSnInf,    kSnTime,        kSnCave,        kSnKill,
	kSnRSeq,   kSnSeq,    kSnSend,        kSnSwap,        kSnKeep,
	kSnGive,   kSnIf,     kSnGame,        kSnSetX0,       kSnSetY0,
	kSnSlave,  kSnSetXY,  kSnRelX,        kSnRelY,        kSnRelZ,
	kSnSetX,   kSnSetY,   kSnSetZ,        kSnTrans,       kSnPort,
	kSnNext,   kSnNNext,  kSnTNext,       kSnRNNext,      kSnRTNext,
	kSnRMNear, kSnRmTake, kSnFlag,        kSnSetRef,      kSnBackPt,
	kSnFlash,  kSnLight,  kSnSetHBarrier, kSnSetVBarrier, kSnWalk,
	kSnReach,  kSnCover,  kSnUncover,     kSnClear,       kSnTalk,
	kSnMouse,  kSnSound,  kSnCount,       kSnExec,        kSnStep,
	kSnZTrim,  kSnGhost
};

class Snail {
public:
	struct Com {
		SnCom _com;
		int _ref;
		int _val;
		void *_ptr;
		CallbackType _cbType;
	} *_snList;
	uint8 _head;
	uint8 _tail;
	bool _turbo;
	bool _busy;
	bool _textDelay;
	uint32 _timerExpiry;
	static const char *_comText[];
	bool _talkEnable;
	Snail(CGEEngine *vm, bool turbo);
	~Snail();
	void runCom();
	void addCom(SnCom com, int ref, int val, void *ptr);
	void addCom2(SnCom com, int ref, int val, CallbackType cbType);
	void insCom(SnCom com, int ref, int val, void *ptr);
	bool idle();
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
