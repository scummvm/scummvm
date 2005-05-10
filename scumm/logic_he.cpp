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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "scumm/intern.h"
#include "scumm/logic_he.h"

namespace Scumm {

/***********************
 * Putt-Putt Joins the Race
 *
 */

LogicHE::LogicHE(ScummEngine *vm) : _vm(vm) {
	// Originally it used 0x930 and stored both floats and doubles inside
	_userData = (float *)calloc(550, sizeof(float));
	_userDataD = (double *)calloc(30, sizeof(double));
}

LogicHE::~LogicHE() {
	free(_userData);
	free(_userDataD);
}

void LogicHE::beforeBootScript() {
	// void implementation
}

void LogicHE::initOnce() {
	// void implementation
}

void LogicHE::startOfFrame() {
	// void implementation
}

void LogicHE::endOfFrame() {
	// void implementation
}

int LogicHE::versionID() {
	return 1;
}

int32 LogicHE::dispatch(int op, int numArgs, int32 *args) {
	int32 res;

	switch (op) {
	case 1003:
		res = op_1003(args);
		break;

	case 1004:
		res = op_1004(args);
		break;

	case 1100:
		res = op_1100(args);
		break;

	case 1101:
		res = op_1101(args);
		break;

	case 1102:
		res = op_1102(args);
		break;

	case 1103:
		res = op_1103(args);
		break;

	case 1110:
		res = op_1110();
		break;

	case 1120:
		res = op_1120(args);
		break;

	case 1130:
		res = op_1130(args);
		break;

	case 1140:
		res = op_1140(args);
		break;

	default:
		res = 0;
		break;

	}

	return res;
}

#define RAD2DEG 5.729577951308239e1
#define DEG2RAD 1.745329251994328e-2

int32 LogicHE::op_1003(int32 *args) {
	int value = args[2] ? args[2] : 1;

	_vm->writeVar(108, (int32)(atan((float)(args[0] / args[1])) * RAD2DEG * value));

	return 1;
}

int32 LogicHE::op_1004(int32 *args) {
	int value = args[1] ? args[1] : 1;

	_vm->writeVar(108, (int32)(sqrt((float)args[0]) * value));

	return 1;
}

int32 LogicHE::op_1100(int32 *args) {
	_userData[516] = (float)args[0] / args[10];
	_userData[517] = (float)args[1] / args[10];
	_userData[518] = (float)args[2] / args[10];
	_userData[519] = (float)args[3] / args[10];
	_userData[520] = (float)args[4] / args[10];

	op_sub1(_userData[520]);

	_userData[521] = (float)args[5] / args[10];
	
	op_sub2(_userData[521]);

	_userData[532] = (float)args[10];

	_userData[524] = (float)args[8];
	_userData[525] = (float)args[9];
	_userData[522] = (float)args[6] / args[10];
	_userData[523] = (float)args[7] / args[10];
	_userData[526] = (float)args[6] / args[8] / args[10];
	_userData[527] = (float)args[7] / args[9] / args[10];

	_vm->writeVar(108, (int32)((float)args[6] / args[8] * args[10]));

	_vm->writeVar(109, (int32)((float)args[7] / args[9] * args[10]));

	_userData[528] = (float)(_userData[519] - _userData[523] * 0.5);
	_userData[529] = (float)(_userData[519] + _userData[523] * 0.5);

	_vm->writeVar(110, (int32)(_userData[528] * args[10]));
	_vm->writeVar(111, (int32)(_userData[529] * args[10]));

	_userData[530] = (float)(_userData[517] / tan(_userData[529] * DEG2RAD));
	_userData[531] = (float)(_userData[517] / tan(_userData[528] * DEG2RAD));

	_vm->writeVar(112, (int32)(_userData[530] * args[10]));
	_vm->writeVar(113, (int32)(_userData[531] * args[10]));

	return 1;
}

int32 LogicHE::op_1101(int32 *args) {
	int32 retval;
	float temp;

    temp = args[0] / _userData[532];

	if (_userData[519] == temp) {
		retval = (int32)temp;
	} else {
		_userData[519] = temp;
		op_sub3(temp);
		retval = 1;
	}
	
	temp = args[1] / _userData[532];

	if (_userData[520] != temp) {
		_userData[520] = temp;
		op_sub1(temp);
		retval = 1;
	}

	temp = args[2] / _userData[532];

	if (_userData[521] != temp) {
		_userData[521] = temp;
		op_sub2(temp);
		retval = 1;
	}

	return retval;
}

int32 LogicHE::op_1102(int32 *args) {
	int32 retval;
	float temp;

	temp = args[0] / _userData[532];
	if (_userData[516] != temp) {
		_userData[516] = temp;
		retval = 1;
	} else {
		retval = (int32)_userData[532];
	}

	temp = args[1] / _userData[532];
	if (_userData[517] != temp) {
		_userData[517] = temp;
		retval = 1;
	}

	temp = args[2] / _userData[532];
	if (_userData[518] != temp) {
		_userData[518] = temp;
		retval = 1;
	}

	return retval;
}

int32 LogicHE::op_1103(int32 *args) {
	double angle = args[0] / args[1] * DEG2RAD;
	
	_vm->writeVar(108, (int32)(sin(angle) * args[2]));
	_vm->writeVar(109, (int32)(cos(angle) * args[2]));

	return 1;
}

int32 LogicHE::op_1110() {
	_vm->writeVar(108, (int32)(_userData[526] * _userData[532] * _userData[532]));
	_vm->writeVar(109, (int32)(_userData[527] * _userData[532] * _userData[532]));
	_vm->writeVar(110, (int32)(_userData[532]));

	return 1;
}

int32 LogicHE::op_1120(int32 *args) {
	double a0, a1, a2, expr;
	double res1, res2;

	a0 = args[0] / _userData[532] - _userData[516];
	a1 = args[1] / _userData[532] - _userData[517];
	a2 = args[2] / _userData[532] - _userData[518];

	expr = a2 * _userDataD[17] + a1 * _userDataD[14] + a0 * _userDataD[11];

	res1 = (atan((a2 * _userDataD[15] + a1 * _userDataD[12] + a0 * _userDataD[9]) / expr) * RAD2DEG) 
			/ _userData[526];
	res2 = (atan((a2 * _userDataD[16] + a1 * _userDataD[13] + a0 * _userDataD[10]) / expr) * RAD2DEG
			- _userData[528]) / _userData[527];
	
	_vm->writeVar(108, (int32)res1);
	_vm->writeVar(109, (int32)res2);

	return 1;
}

int32 LogicHE::op_1130(int32 *args) {
	double cs = cos(args[0] / _userData[532] * DEG2RAD);
	double sn = sin(args[0] / _userData[532] * DEG2RAD);

	_vm->writeVar(108, (int32)(cs * args[1] + sn * args[2]));

	_vm->writeVar(109, (int32)(cs * args[2] - sn * args[1]));

	return 1;
}

int32 LogicHE::op_1140(int32 *args) {
	double arg2 = -args[2] * args[2];
	double arg3 = -args[3] * args[3];
	double sq = sqrt(arg2 + arg3);
	double res;

	arg2 = arg2 / sq;
	arg3 = arg3 / sq;

	res = (args[0] - 2 * (arg2 * args[0] + arg3 * args[1]) * arg2) * 0.86956525;

	_vm->writeVar(108, (int32)res);
	
	res = args[1] - 2 * (arg2 * args[0] + arg3 * args[1]) * arg3;
	
	if (-args[3] * args[3] >= 0)
		res *= 0.83333331f;

	_vm->writeVar(109, (int32)res);

	return 1;
}

void LogicHE::op_sub1(float arg) {
	_userDataD[10] = _userDataD[12] = _userDataD[14] = _userDataD[16] = 0;
	_userDataD[13] = 1;

	_userDataD[9] = cos(arg * DEG2RAD);
	_userDataD[15] = sin(arg * DEG2RAD);
	_userDataD[11] = -_userDataD[15];
	_userDataD[17] = _userDataD[9];
}

void LogicHE::op_sub2(float arg) {
	_userDataD[20] = _userDataD[21] = _userDataD[24] = _userDataD[25] = 0;
	_userDataD[26] = 1;

	_userDataD[19] = sin(arg * DEG2RAD);
	_userDataD[18] = cos(arg * DEG2RAD);
	_userDataD[21] = -_userDataD[19];
	_userDataD[22] = _userDataD[18];
}

void LogicHE::op_sub3(float arg) {
	_userDataD[1] = _userDataD[2] = _userDataD[3] = _userDataD[6] = 0;
	_userDataD[0] = 1;

	_userDataD[4] = cos(arg * DEG2RAD);
	_userDataD[5] = sin(arg * DEG2RAD);
	_userDataD[7] = -_userDataD[5];
	_userDataD[8] = _userDataD[4];
}

} // End of namespace Scumm
