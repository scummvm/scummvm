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

LogicHE::LogicHE(ScummEngine_v90he *vm) : _vm(vm) {
	// Originally it used 0x930 and stored both floats and doubles inside
	_userData = (float *)calloc(550, sizeof(float));
	_userDataD = (double *)calloc(30, sizeof(double));
}

LogicHE::~LogicHE() {
	free(_userData);
	free(_userDataD);
}

double LogicHE::fpatan(double st0, double st1) {
	// TODO: Still incomplete

	if (st0 == 0)
		return 0;

	if (st1 == 0 && st0 < 0)
		return 3.14159265358;

	return atan(st1 / st0);
}

int LogicHE::versionID() {
	return 1;
}

int LogicHE::getFromArray(int arg0, int idx2, int idx1) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = arg0;
	return _vm->readArray(116, idx2, idx1);
}

void LogicHE::putInArray(int arg0, int idx2, int idx1, int val) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = arg0;
	_vm->writeArray(116, idx2, idx1, val);
}

int32 LogicHE::dispatch(int op, int numArgs, int32 *args) {
#if 1
	char tmp[32], str[256];

	if (numArgs > 0)
		snprintf(tmp, 32, "%d", args[0]);
	else
		*tmp = 0;

	snprintf(str, 256, "LogicHE::dispatch(%d, %d, [%s", op, numArgs, tmp);

	for (int i = 1; i < numArgs; i++) {
		snprintf(tmp, 32, ", %d", args[i]);
		strncat(str, tmp, 256);
	}
	strncat(str, "])", 256);

	debug(0, str);
#else
	// Used for parallel trace utility
	for (int i = 0; i < numArgs; i++)
		debug(0, "args[%d] = %d;", i, args[i]);

	debug(0, "dispatch(%d, %d, args);", op, numArgs);

#endif

	return 1;
}

/***********************
 * Putt-Putt Joins the Race
 *
 */

int LogicHErace::versionID() {
	return 1;
}

int32 LogicHErace::dispatch(int op, int numArgs, int32 *args) {
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

int32 LogicHErace::op_1003(int32 *args) {
	int value = args[2] ? args[2] : 1;

	writeScummVar(108, (int32)(fpatan(args[1], args[0]) * RAD2DEG * value));

	return 1;
}

int32 LogicHErace::op_1004(int32 *args) {
	int value = args[1] ? args[1] : 1;

	writeScummVar(108, (int32)(sqrt((float)args[0]) * value));

	return 1;
}

int32 LogicHErace::op_1100(int32 *args) {
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

	writeScummVar(108, (int32)((float)args[6] / args[8] * args[10]));

	writeScummVar(109, (int32)((float)args[7] / args[9] * args[10]));

	_userData[528] = (float)(_userData[519] - _userData[523] * 0.5);
	_userData[529] = (float)(_userData[519] + _userData[523] * 0.5);

	writeScummVar(110, (int32)(_userData[528] * args[10]));
	writeScummVar(111, (int32)(_userData[529] * args[10]));

	_userData[530] = (float)(_userData[517] / tan(_userData[529] * DEG2RAD));
	_userData[531] = (float)(_userData[517] / tan(_userData[528] * DEG2RAD));

	writeScummVar(112, (int32)(_userData[530] * args[10]));
	writeScummVar(113, (int32)(_userData[531] * args[10]));

	return 1;
}

int32 LogicHErace::op_1101(int32 *args) {
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

int32 LogicHErace::op_1102(int32 *args) {
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

int32 LogicHErace::op_1103(int32 *args) {
	double angle = args[0] / args[1] * DEG2RAD;
	
	writeScummVar(108, (int32)(sin(angle) * args[2]));
	writeScummVar(109, (int32)(cos(angle) * args[2]));

	return 1;
}

int32 LogicHErace::op_1110() {
	writeScummVar(108, (int32)(_userData[526] * _userData[532] * _userData[532]));
	writeScummVar(109, (int32)(_userData[527] * _userData[532] * _userData[532]));
	writeScummVar(110, (int32)(_userData[532]));

	return 1;
}

int32 LogicHErace::op_1120(int32 *args) {
	double a0, a1, a2, expr;
	double res1, res2;

	a0 = args[0] / _userData[532] - _userData[516];
	a1 = args[1] / _userData[532] - _userData[517];
	a2 = args[2] / _userData[532] - _userData[518];

	expr = a2 * _userDataD[17] + a1 * _userDataD[14] + a0 * _userDataD[11];

	res1 = (fpatan(expr, a2 * _userDataD[15] + a1 * _userDataD[12] + a0 * _userDataD[9]) * RAD2DEG) 
			/ _userData[526];
	res2 = (fpatan(expr, a2 * _userDataD[16] + a1 * _userDataD[13] + a0 * _userDataD[10]) * RAD2DEG
			- _userData[528]) / _userData[527];
	
	writeScummVar(108, (int32)res1);
	writeScummVar(109, (int32)res2);

	return 1;
}

int32 LogicHErace::op_1130(int32 *args) {
	double cs = cos(args[0] / _userData[532] * DEG2RAD);
	double sn = sin(args[0] / _userData[532] * DEG2RAD);

	writeScummVar(108, (int32)(cs * args[1] + sn * args[2]));

	writeScummVar(109, (int32)(cs * args[2] - sn * args[1]));

	return 1;
}

int32 LogicHErace::op_1140(int32 *args) {
	double arg2 = -args[2] * args[2];
	double arg3 = -args[3] * args[3];
	double sq = sqrt(arg2 + arg3);
	double res;

	arg2 = arg2 / sq;
	arg3 = arg3 / sq;

	res = (args[0] - 2 * (arg2 * args[0] + arg3 * args[1]) * arg2) * 0.86956525;

	writeScummVar(108, (int32)res);
	
	res = args[1] - 2 * (arg2 * args[0] + arg3 * args[1]) * arg3;
	
	if (-args[3] * args[3] >= 0)
		res *= 0.83333331f;

	writeScummVar(109, (int32)res);

	return 1;
}

void LogicHErace::op_sub1(float arg) {
	_userDataD[10] = _userDataD[12] = _userDataD[14] = _userDataD[16] = 0;
	_userDataD[13] = 1;

	_userDataD[9] = cos(arg * DEG2RAD);
	_userDataD[15] = sin(arg * DEG2RAD);
	_userDataD[11] = -_userDataD[15];
	_userDataD[17] = _userDataD[9];
}

void LogicHErace::op_sub2(float arg) {
	_userDataD[20] = _userDataD[21] = _userDataD[24] = _userDataD[25] = 0;
	_userDataD[26] = 1;

	_userDataD[19] = sin(arg * DEG2RAD);
	_userDataD[18] = cos(arg * DEG2RAD);
	_userDataD[21] = -_userDataD[19];
	_userDataD[22] = _userDataD[18];
}

void LogicHErace::op_sub3(float arg) {
	_userDataD[1] = _userDataD[2] = _userDataD[3] = _userDataD[6] = 0;
	_userDataD[0] = 1;

	_userDataD[4] = cos(arg * DEG2RAD);
	_userDataD[5] = sin(arg * DEG2RAD);
	_userDataD[7] = -_userDataD[5];
	_userDataD[8] = _userDataD[4];
}

/***********************
 * Freddi Fish's One-Stop Fun Shop
 * Pajama Sam's One-Stop Fun Shop
 * Putt-Putt's One-Stop Fun Shop
 *
 */

int LogicHEfunshop::versionID() {
	return 1;
}

int32 LogicHEfunshop::dispatch(int op, int numArgs, int32 *args) {
	switch (op) {
	case 1004:
		op_1004(args);
		break;

	case 1005:
		op_1005(args);
		break;

	default:
		break;

	}

	return 0;
}

void LogicHEfunshop::op_1004(int32 *args) {
	double data[8], at, sq;
	int32 x, y;

	for (int i = 0; i <= 6; i += 2) {
		data[i] = getFromArray(args[0], 0, 519 + i);
		data[i + 1] = getFromArray(args[0], 0, 519 + i + 1);

		printf("data[%d] = %lf\ndata[%d] = %lf\n", i, data[i], i + 1, data[i + 1]);
	}
	int s = checkShape((int32)data[0], (int32)data[1], (int32)data[4], (int32)data[5],
		(int32)data[2], (int32)data[3], (int32)data[6], (int32)data[7], &x, &y);

	if (s != 1) {
		printf("LogicHEfunshop::op_1004: Your shape has defied the laws of physics\n");
			return;
	}

	for (int i = 0; i <= 6; i += 2) {
		data[i] -= (double)x;
		data[i + 1] -= (double)y;
	}

	double a1 = (double)args[1] * 1.745328888888889e-2;

	for (int i = 0; i <= 6; i += 2) {
		at = fpatan(data[i], data[i + 1]);
		sq = sqrt(fabs(data[i]) * fabs(data[i]) + fabs(data[i + 1]) * fabs(data[i + 1]));

		if (at <= 0)
			at += 6.283184;

		data[i] = cos(at + a1) * sq;
		data[i + 1] = sin(at + a1) * sq;
	}
	
	int minx = 2;
	int miny = 3;

	for (int i = 0; i <= 6; i += 2) {
		if (data[i] < data[minx])
			minx = i;
		if (data[i + 1] < data[miny])
			miny = i + 1;
	}

	for (int i = 0; i <= 6; i += 2) {
		data[i] -= data[minx];
		data[i + 1] -= data[miny];

		if (floor(data[i]) + 0.5 > data[i])
			data[i] = floor(data[i]);
		else
			data[i] = ceil(data[i]);

		putInArray(args[0], 0, 519 + i, (int32)data[i]);

		if (floor(data[i + 1]) + 0.5 > data[i + 1])
			data[i + 1] = floor(data[i + 1]);
		else
			data[i + 1] = ceil(data[i + 1]);

		putInArray(args[0], 0, 519 + i + 1, (int32)data[i + 1]);
	}
}

void LogicHEfunshop::op_1005(int32 *args) {
	double data[8];
	double args1, args2;
	double temp;

	for (int i = 520; i <= 526; i += 2) {
		data[i - 520] = getFromArray(args[0], 0, i - 1);
		data[i - 520 + 1] = getFromArray(args[0], 0, i);
	}

	args1 = args[1] * 0.01 + 1;
	args2 = args[2] * 0.01 + 1;

	for (int i = 0; i < 4; i++) {
		data[2 * i] *= args1;
		data[2 * i + 1] *= args2;
	}
	
	for (int i = 520; i <= 526; i += 2) {
		if (floor(data[i - 520]) + 0.5 > data[i - 520]) {
			temp = ceil(data[i - 520]);
		} else {
			temp = floor(data[i - 520]);
		}
		putInArray(args[0], 0, i - 1, (int32)temp);

		if (floor(data[i - 520 + 1]) + 0.5 <= data[i - 520 + 1]) {
			temp = ceil(data[i - 520 + 1]);
		} else {
			temp = floor(data[i - 520 + 1]);
		}
		putInArray(args[0], 0, i, (int32)temp);
	}
}

int LogicHEfunshop::checkShape(int32 data0, int32 data1, int32 data4, int32 data5, int32 data2, int32 data3, int32 data6, int32 data7, int32 *x, int32 *y) {
	int32 diff5_1, diff0_4, diff7_3, diff2_6;
	int32 diff1, diff2;
	int32 delta, delta2;
	int32 sum1, sum2;

	diff0_4 = data0 - data4;
	diff5_1 = data5 - data1;
	diff1 = data1 * data4 - data0 * data5;
	sum1 = diff0_4 * data3 + diff1 + diff5_1 * data2;
	sum2 = diff0_4 * data7 + diff1 + diff5_1 * data6;

	if (sum1 != 0 && sum2 != 0) {
		sum2 ^= sum1;

		if (sum2 >= 0)
			return 0;
	}

	diff2_6 = data2 - data6;
	diff7_3 = data7 - data3;
	diff2 = data3 * data6 - data2 * data7;
	sum1 = diff2_6 * data1 + diff2 + diff7_3 * data0;
	sum2 = diff2_6 * data5 + diff2 + diff7_3 * data4;;

	if (sum1 != 0 && sum2 != 0) {
		sum2 ^= sum1;

		if (sum2 >= 0)
			return 0;
	}

	delta = diff2_6 * diff5_1 - diff0_4 * diff7_3;

	if (delta == 0) {
		return 2;
	}

	if (delta < 0) {
		data7 = -((delta + 1) >> 1);
	} else {
		data7 = delta >> 1;
	}

	delta2 = diff2 * diff0_4 - diff1 * diff2_6;
	
	if (delta2 < 0) {
		delta2 -= data7;
	} else {
		delta2 += data7;
	}

	*x = delta2 / delta;

	delta2 = diff1 * diff7_3 - diff2 * diff5_1;

	if (delta2 < 0) {
		delta2 -= data7;
	} else {
		delta2 += data7;
	}

	*y = delta2 / delta;

	return 1;
}

/***********************
 * Backyard Football
 * Backyard Football Demo
 *
 */

int LogicHEfootball::versionID() {
	return 1;
}

int32 LogicHEfootball::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1004:
		res = op_1004(args);
		break;

	case 1006:
		res = op_1006(args);
		break;

	case 1007:
		res = op_1007(args);
		break;

	case 1010:
		res = op_1010(args);
		break;

	case 1022:
		res = op_1022(args);
		break;

	case 1023:
		res = op_1023(args);
		break;

	case 1024:
		res = op_1024(args);
		break;

	case 8221968:
		// Someone had a fun and used his birthday as opcode number
		res = getFromArray(args[0], args[1], args[2]);
		break;

	case 1492: case 1493: case 1494: case 1495: case 1496:
	case 1497: case 1498: case 1499: case 1500: case 1501:
	case 1502: case 1503: case 1504: case 1505: case 1506:
	case 1507: case 1508: case 1509: case 1510: case 1511:
	case 1512: case 1513: case 1514: case 1555:
		// DirectPlay-related
		// 1513: initialize
		// 1555: set fake lag
		break;

	case 2200: case 2201: case 2202: case 2203: case 2204:
	case 2205: case 2206: case 2207: case 2208: case 2209:
	case 2210: case 2211: case 2212: case 2213: case 2214:
	case 2215: case 2216: case 2217: case 2218: case 2219:
	case 2220: case 2221: case 2222: case 2223: case 2224:
	case 2225: case 2226: case 2227: case 2228:
		// Boneyards-related
		break;

	case 3000: case 3001: case 3002: case 3003: case 3004:
		// Internet-related
		// 3000: check for updates
		// 3001: check network status
		// 3002: autoupdate
		// 3003: close connection
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
		warning("Tell me how to reproduce it");
		assert(0);
		break;

	}

	return res;
}

int LogicHEfootball::op_1004(int32 *args) {
	double res, a2, a4, a5;

	a5 = ((double)args[4] - (double)args[1]) / ((double)args[5] - (double)args[2]);
	a4 = ((double)args[3] - (double)args[0]) / ((double)args[5] - (double)args[2]);
	a2 = (double)args[2] - (double)args[0] * a4 - args[1] * a5;

	res = (double)args[6] * a4 + (double)args[7] * a5 + a2;
	writeScummVar(108, (int32)res);

	writeScummVar(109, (int32)a2);
	writeScummVar(110, (int32)a5);
	writeScummVar(111, (int32)a4);

	return 1;
}

int LogicHEfootball::op_1006(int32 *args) {
	double res;

	res = (1.0 - args[1] * 2.9411764e-4 * 5.3050399e-2) * args[0] * 1.2360656e-1 + 
		args[1] * 1.1764706e-2 + 46;
	writeScummVar(108, (int32)res);

	res = 640.0 - args[2] * 1.2360656e-1 - args[1] * 1.1588235e-1 - 26;
	writeScummVar(109, (int32)res);

	return 1;
}

int LogicHEfootball::op_1007(int32 *args) {
	double res, temp;

	temp = (double)args[1] * 0.32;

	if (temp > 304.0)
		res = -args[2] * 0.142;
	else
		res = args[2] * 0.142;

	res += temp;

	writeScummVar(108, (int32)res);

	res = (1000.0 - args[2]) * 0.48;

	writeScummVar(109, (int32)res);

	return 1;
}

int LogicHEfootball::op_1010(int32 *args) {
	double a1 = (640.0 - (double)args[1] - 26.0) * 8.6294413;
	double res;

	res = ((double)args[0] - 46 - a1 * 1.1764706e-2) / 
		((1.0 - a1 * 2.9411764e-4 * 5.3050399e-2) * 1.2360656e-1);
	writeScummVar(108, (int32)res);

	writeScummVar(109, (int32)a1);

	return 1;
}

int LogicHEfootball::op_1022(int32 *args) {
	double res;
	double var10 = args[4] - args[1];
	double var8 = args[5] - args[2];
	double var6 = args[3] - args[0];

	res = sqrt(var8 * var8 + var6 * var6 + var10 * var10);

	if (res >= (double)args[6]) {
		var8 = (double)args[6] * var8 / res;
		var10 = (double)args[6] * var10 / res;
		res = (double)args[6] * var6 / res;
	}

	writeScummVar(108, (int32)res);
	writeScummVar(109, (int32)var10);
	writeScummVar(110, (int32)var8);
	
	return 1;
}

int LogicHEfootball::op_1023(int32 *args) {
	double var10, var18, var20, var28, var30, var30_;
	double argf[7];

	for (int i = 0; i < 7; i++)
		argf[i] = args[i];

	var10 = (argf[3] - argf[1]) / (argf[2] - argf[0]);
	var28 = var10 * var10 + 1;
	var20 = argf[0] * var10;
	var18 = (argf[5] + argf[1] + var20) * argf[4] * var10 * 2 + 
		argf[6] * argf[6] * var28 + argf[4] * argf[4] - 
		argf[0] * argf[0] * var10 * var10 - 
		argf[5] * argf[0] * var10 * 2 - 
		argf[5] * argf[1] * 2 - 
		argf[1] * argf[1] - argf[5] * argf[5];
	
	if (var18 >= 0) {
		var18 = sqrt(var18);

		var30_ = argf[4] + argf[5] * var10 + argf[1] * var10 + argf[0] * var10 * var10;
		var30 = (var30_ - var18) / var28;
		var18 = (var30_ + var18) / var28;
		
		if ((argf[0] - var30 < 0) && (argf[0] - var18 < 0)) {
			var30_ = var30;
			var30 = var18;
			var18 = var30_;
		}
		var28 = var18 * var10 - var20 - argf[1];
		var20 = var30 * var10 - var20 - argf[1];
	} else {
		var18 = 0;
		var20 = 0;
		var28 = 0;
		var30 = 0;
	}

	writeScummVar(108, (int32)var18);
	writeScummVar(109, (int32)var28);
	writeScummVar(110, (int32)var30);
	writeScummVar(111, (int32)var20);
	
	return 1;
}
int LogicHEfootball::op_1024(int32 *args) {
	writeScummVar(108, 0);
	writeScummVar(109, 0);
	writeScummVar(110, 0);
	writeScummVar(111, 0);

	return 1;
}


} // End of namespace Scumm
