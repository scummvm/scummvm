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
	_userData = (float *)calloc(0x930, 1);
}

LogicHE::~LogicHE() {
	free(_userData);
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

	_vm->writeVar(108, (int32)(atan(args[0] / args[1]) * RAD2DEG * value));

	return 1;
}

int32 LogicHE::op_1004(int32 *args) {
	int value = args[1] ? args[1] : 1;

	_vm->writeVar(108, (int32)(sqrt(args[0]) * value));

	return 1;
}

int32 LogicHE::op_1100(int32 *args) {
	_userData[516] = args[0] / args[10];
	_userData[517] = args[1] / args[10];
	_userData[518] = args[2] / args[10];
	_userData[519] = args[3] / args[10];
	_userData[520] = (float)args[4] / args[10];
	args[4] = args[10];

	op_sub1(_userData[520]);

	_userData[521] = (float)args[5] / args[4];
	
	op_sub2(_userData[521]);

	_userData[532] = args[10];

	args[1] = args[8];
	args[10] = args[9];

	_userData[524] = args[1];
	_userData[525] = args[10];
	_userData[522] = args[6] / args[4];
	_userData[523] = args[7] / args[4];
	_userData[526] = args[6] / args[1] / args[4];
	_userData[527] = args[7] / args[10] / args[4];

	args[0] = args[7] / args[10];

	_vm->writeVar(108, (int32)(args[6] / args[1] * args[4]));

	_vm->writeVar(109, (int32)(args[0] * args[4]));

	_userData[528] = _userData[519] - _userData[523] * 0.5;
	_userData[529] = _userData[519] + _userData[523] * 0.5;

	_vm->writeVar(110, (int32)(_userData[528] * args[4]));

	_vm->writeVar(111, (int32)(_userData[529] * args[4]));

	_userData[530] = _userData[517] * tan(_userData[529] * DEG2RAD);
	_userData[531] = _userData[517] * tan(_userData[528] * DEG2RAD);

	_vm->writeVar(112, (int32)(_userData[517] * tan(_userData[529] * DEG2RAD) * args[4]));

	_vm->writeVar(113, (int32)(_userData[531] * args[4]));

	return 1;
}

int32 LogicHE::op_1101(int32 *args) {
	int32 retval = 1;

	// TODO

	return retval;
}

int32 LogicHE::op_1102(int32 *args) {
	int32 retval = 1;

	// TODO

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
	// TODO

	return 1;
}

int32 LogicHE::op_1130(int32 *args) {
	float cs = cos(args[0] / _userData[532] * DEG2RAD);
	float sn = sin(args[0] / _userData[532] * DEG2RAD);

	_vm->writeVar(108, (int32)(cs * args[1] + sn * args[2]));

	_vm->writeVar(109, (int32)(cs * args[2] - sn * args[1]));

	return 1;
}

int32 LogicHE::op_1140(int32 *args) {
	// TODO

	return 1;
}

void LogicHE::op_sub1(float arg) {
}

void LogicHE::op_sub2(float arg) {
}

void LogicHE::op_sub3(float arg) {
}

} // End of namespace Scumm
