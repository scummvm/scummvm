/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

#if !defined(LOGIC_HE_H) && !defined(DISABLE_HE)
#define LOGIC_HE_H

#include "common/stdafx.h"

#include "scumm/intern.h"

namespace Scumm {

class LogicHE {
public:
	float *_userData;
	double *_userDataD;
	ScummEngine_v90he *_vm;

	LogicHE(ScummEngine_v90he *vm);
	virtual ~LogicHE();

	void writeScummVar(int var, int32 value) { _vm->writeVar(var, value); }
	int getFromArray(int arg0, int idx2, int idx1);
	void putInArray(int arg0, int idx2, int idx1, int val);

	void beforeBootScript(void) {};
	void initOnce() {};
	void startOfFrame() {};
	void endOfFrame() {};
	void processKeyStroke(int keyPressed) {};

	virtual int versionID();
	virtual int32 dispatch(int op, int numArgs, int32 *args);
};

class LogicHErace : public LogicHE {
public:
	LogicHErace(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int32 op_1003(int32 *args);
	int32 op_1004(int32 *args);
	int32 op_1100(int32 *args);
	int32 op_1101(int32 *args);
	int32 op_1102(int32 *args);
	int32 op_1103(int32 *args);
	int32 op_1110();
	int32 op_1120(int32 *args);
	int32 op_1130(int32 *args);
	int32 op_1140(int32 *args);

	void op_sub1(float arg);
	void op_sub2(float arg);
	void op_sub3(float arg);
};

class LogicHEfunshop : public LogicHE {
public:
	LogicHEfunshop(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	void op_1004(int32 *args);
	void op_1005(int32 *args);
	int checkShape(int32 data0, int32 data1, int32 data4, int32 data5, int32 data2, int32 data3, int32 data6, int32 data7, int32 *x, int32 *y);
};

class LogicHEfootball : public LogicHE {
public:
	LogicHEfootball(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int op_1004(int32 *args);
	int op_1006(int32 *args);
	int op_1007(int32 *args);
	int op_1010(int32 *args);
	int op_1022(int32 *args);
	int op_1023(int32 *args);
	int op_1024(int32 *args);
};

} // End of namespace Scumm

#endif
