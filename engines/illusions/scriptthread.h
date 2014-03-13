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

#ifndef ILLUSIONS_SCRIPTTHREAD_H
#define ILLUSIONS_SCRIPTTHREAD_H

#include "illusions/thread.h"

namespace Illusions {

class IllusionsEngine;

struct OpCall {
	byte _op;
	byte _opSize;
	uint32 _threadId;
	int16 _deltaOfs;
	byte *_scriptCode;
	int _result;
	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

class ScriptThread : public Thread {
public:
	ScriptThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10);
	virtual int onUpdate();
	virtual void onSuspend();
	virtual void onNotify();
	virtual void onPause();
	virtual void onResume();
	virtual void onTerminated();
public:
	int16 _sequenceStalled;
	byte *_scriptCodeIp;
	uint32 _value8;
	uint32 _valueC;
	uint32 _value10;
	void execOpcode(OpCall &opCall);
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTTHREAD_H
