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

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

LogicHE::LogicHE(ScummEngine_v90he *vm) : _vm(vm) {
}

LogicHE::~LogicHE() {
}

LogicHErace::LogicHErace(ScummEngine_v90he *vm) : LogicHE(vm) {
	// Originally it used 0x930 and stored both floats and doubles inside
	_userData = (float *)calloc(550, sizeof(float));
	_userDataD = (double *)calloc(30, sizeof(double));

	// FIXME: of the 550 entries in _userData, only 516 till 532 are used
	// FIXME: similarly, in _userDataD only 9 till 17 are used for computations
	//       (some of the other entries are also set, but never read, hence useless).
}

LogicHErace::~LogicHErace() {
	free(_userData);
	free(_userDataD);
}

void LogicHE::writeScummVar(int var, int32 value) {
	_vm->writeVar(var, value);
}

static int32 scumm_round(double arg) {
	return (int32)(arg + 0.5);
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
	Common::String str;

	str = Common::String::format("LogicHE::dispatch(%d, %d, [", op, numArgs);
	if (numArgs > 0)
		str += Common::String::format("%d", args[0]);
	for (int i = 1; i < numArgs; i++) {
		str += Common::String::format(", %d", args[i]);
	}
	str += "])";

	debug(0, "%s", str.c_str());
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

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (M_PI / 180)

int32 LogicHErace::op_1003(int32 *args) {
	int value = args[2] ? args[2] : 1;

	writeScummVar(108, (int32)(atan2((double)args[0], (double)args[1]) * RAD2DEG * value));

	return 1;
}

int32 LogicHErace::op_1004(int32 *args) {
	int value = args[1] ? args[1] : 1;

	writeScummVar(108, (int32)(sqrt((float)args[0]) * value));

	return 1;
}

int32 LogicHErace::op_1100(int32 *args) {
	// _userData 516,517,518 describe a 3D translation?
	_userData[516] = (float)args[0] / args[10];
	_userData[517] = (float)args[1] / args[10];
	_userData[518] = (float)args[2] / args[10];

	// _userData 519,520,521 describe rotation angles around the x,y,z axes?
	_userData[519] = (float)args[3] / args[10];
	_userData[520] = (float)args[4] / args[10];
	_userData[521] = (float)args[5] / args[10];

	op_sub1(_userData[520]);
	op_sub2(_userData[521]);

	// _userData[532] seems to be some kind of global scale factor
	_userData[532] = (float)args[10];

	_userData[524] = (float)args[8];	// not used
	_userData[525] = (float)args[9];	// not used
	_userData[522] = (float)args[6] / args[10];	// not used
	_userData[523] = (float)args[7] / args[10];	// only used to compute 528 and 529

	// The following two are some kind of scale factors
	_userData[526] = (float)args[6] / args[8] / args[10];
	_userData[527] = (float)args[7] / args[9] / args[10];

	// Set var 108 and 109 -- the value set here corresponds to the values
	// set by op_1110!
	writeScummVar(108, (int32)((float)args[6] / args[8] * args[10]));
	writeScummVar(109, (int32)((float)args[7] / args[9] * args[10]));

	_userData[528] = (float)(_userData[519] - _userData[523] * 0.5);
	_userData[529] = (float)(_userData[519] + _userData[523] * 0.5);

	writeScummVar(110, (int32)(_userData[528] * args[10]));
	writeScummVar(111, (int32)(_userData[529] * args[10]));

	// 530 and 531 are only used to set vars 112 and 113, so no need
	// to store them permanently
	_userData[530] = (float)(_userData[517] / tan(_userData[529] * DEG2RAD));
	_userData[531] = (float)(_userData[517] / tan(_userData[528] * DEG2RAD));

	writeScummVar(112, (int32)(_userData[530] * args[10]));
	writeScummVar(113, (int32)(_userData[531] * args[10]));

	return 1;
}

int32 LogicHErace::op_1101(int32 *args) {
	// Update rotation params?
	int32 retval;
	float temp;

	temp = args[0] / _userData[532];
	if (_userData[519] != temp) {
		_userData[519] = temp;
		op_sub3(temp);
		retval = 1;
	} else {
		retval = (int32)temp;
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
	// Update translation params?
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
	double a0, a1, a2;
	double b0, b1, b2;
	double res1, res2;

	a0 = args[0] / _userData[532] - _userData[516];
	a1 = args[1] / _userData[532] - _userData[517];
	a2 = args[2] / _userData[532] - _userData[518];

	// Perform matrix multiplication (multiplying by a rotation matrix)
	b2 = a2 * _userDataD[17] + a1 * _userDataD[14] + a0 * _userDataD[11];
	b1 = a2 * _userDataD[16] + a1 * _userDataD[13] + a0 * _userDataD[10];
	b0 = a2 * _userDataD[15] + a1 * _userDataD[12] + a0 * _userDataD[9];

	res1 = (atan2(b0, b2) * RAD2DEG) / _userData[526];
	res2 = (atan2(b1, b2) * RAD2DEG - _userData[528]) / _userData[527];

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
	// This functions seems to perform some kind of projection: We project
	// the vector (arg2,arg3) onto the vector (arg0,arg1), but also apply
	// some kind of distortion factor ?!?
	double x = args[2], y = args[3];

	// We start by normalizing the vector described by arg2 and arg3.
	// So compute its length and divide the x and y coordinates
	const double sq = sqrt(x*x + y*y);
	x /= sq;
	y /= sq;

	// Compute the scalar product of the vectors (arg0,arg1) and (x,y)
	const double scalarProduct = x * args[0] + y * args[1];

	// Finally compute the projection of (arg2,arg3) onto (arg0,arg1)
	double projX = args[0] - 2 * scalarProduct * x;
	double projY = args[1] - 2 * scalarProduct * y;

	projX = projX * 20.0 / 23.0;	// FIXME: Why is this here?

	writeScummVar(108, (int32)projX);

	if (args[3] >= 0)	// FIXME: Why is this here?
		projY = projY * 5.0 / 6.0;

	writeScummVar(109, (int32)projY);

	return 1;
}

void LogicHErace::op_sub1(float arg) {
	// Setup a rotation matrix
	_userDataD[10] = _userDataD[12] = _userDataD[14] = _userDataD[16] = 0;
	_userDataD[13] = 1;

	_userDataD[9] = cos(arg * DEG2RAD);
	_userDataD[15] = sin(arg * DEG2RAD);
	_userDataD[11] = -_userDataD[15];
	_userDataD[17] = _userDataD[9];
}

void LogicHErace::op_sub2(float arg) {
	// Setup a rotation matrix -- but it is NEVER USED!
	_userDataD[20] = _userDataD[21] = _userDataD[24] = _userDataD[25] = 0;
	_userDataD[26] = 1;

	_userDataD[19] = sin(arg * DEG2RAD);
	_userDataD[18] = cos(arg * DEG2RAD);
	_userDataD[21] = -_userDataD[19];
	_userDataD[22] = _userDataD[18];
}

void LogicHErace::op_sub3(float arg) {
	// Setup a rotation matrix -- but it is NEVER USED!
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
	int i=0;

	for (i = 0; i <= 6; i += 2) {
		data[i] = getFromArray(args[0], 0, 519 + i);
		data[i + 1] = getFromArray(args[0], 0, 519 + i + 1);
	}
	int s = checkShape((int32)data[0], (int32)data[1], (int32)data[4], (int32)data[5],
		(int32)data[2], (int32)data[3], (int32)data[6], (int32)data[7], &x, &y);

	if (s != 1) {
		error("LogicHEfunshop::op_1004: Your shape has defied the laws of physics");
		return;
	}

	for (i = 0; i <= 6; i += 2) {
		data[i] -= (double)x;
		data[i + 1] -= (double)y;
	}

	double a1 = (double)args[1] * DEG2RAD;

	for (i = 0; i <= 6; i += 2) {
		at = atan2(data[i + 1], data[i]);
		sq = sqrt(data[i + 1] * data[i + 1] + data[i] * data[i]);

		if (at <= 0)
			at += 2 * M_PI;

		data[i] = cos(at + a1) * sq;
		data[i + 1] = sin(at + a1) * sq;
	}

	double minx = data[0];
	double miny = data[1];

	for (i = 0; i <= 6; i += 2) {
		if (data[i] < minx)
			minx = data[i];
		if (data[i + 1] < miny)
			miny = data[i + 1];
	}

	for (i = 0; i <= 6; i += 2) {
		data[i] -= minx;
		data[i + 1] -= miny;

		putInArray(args[0], 0, 519 + i, scumm_round(data[i]));
		putInArray(args[0], 0, 519 + i + 1, scumm_round(data[i + 1]));
	}
}

void LogicHEfunshop::op_1005(int32 *args) {
	double data[8];
	double args1, args2;
	int i;
	for (i = 520; i <= 526; i += 2) {
		data[i - 520] = getFromArray(args[0], 0, i - 1);
		data[i - 520 + 1] = getFromArray(args[0], 0, i);
	}

	args1 = (double)args[1] * 0.01 + 1;
	args2 = (double)args[2] * 0.01 + 1;

	for (i = 0; i < 4; i++) {
		data[2 * i] *= args1;
		data[2 * i + 1] *= args2;
	}

	for (i = 520; i <= 526; i += 2) {
		putInArray(args[0], 0, i - 1, scumm_round(data[i - 520]));
		putInArray(args[0], 0, i, scumm_round(data[i - 520 + 1]));
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
	sum2 = diff2_6 * data5 + diff2 + diff7_3 * data4;

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
 * Backyard Football 2002
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
		warning("Tell sev how to reproduce it (%d)", op);
	}

	return res;
}

int LogicHEfootball::op_1004(int32 *args) {
	// Identical to LogicHEsoccer::op_1004
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
	// This seems to be more or less the inverse of op_1010
	const double a1 = args[1];
	double res;

	// 2.9411764e-4 = 1/3400
	// 5.3050399e-2 = 1/18.85 = 20/377
	// 1.1764706e-2 = 1/85 = 40/3400
	// 1.2360656e-1 = 377/3050
	res = (1.0 - a1 * 2.9411764e-4 * 5.3050399e-2) * 1.2360656e-1 * args[0] +
		a1 * 1.1764706e-2 + 46;

	// Shortened / optimized version of that formula:
	// res = (377.0 - a1 / 170.0) / 3050.0 * args[0] + a1 / 85.0 + 46;

	writeScummVar(108, (int32)res);

	// 1.2360656e-1 = 377/3050
	// 1.1588235e-1 = 197/1700 = 394/3400
	res = 640.0 - args[2] * 1.2360656e-1 - a1 * 1.1588235e-1 - 26;

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
	// This seems to be more or less the inverse of op_1006
	double a1 = (640.0 - (double)args[1] - 26.0) / 1.1588235e-1;

	// 2.9411764e-4 = 1/3400
	// 5.3050399e-2 = 1/18.85 = 20/377
	// 1.1764706e-2 = 1/85 = 40/3400
	// 1.2360656e-1 = 377/3050
	double a0 = ((double)args[0] - 46 - a1 * 1.1764706e-2) /
		((1.0 - a1 * 2.9411764e-4 * 5.3050399e-2) * 1.2360656e-1);

	writeScummVar(108, (int32)a0);
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


/***********************
 * Backyard Soccer
 *
 */

int LogicHEsoccer::versionID() {
	return 1;
}

LogicHEsoccer::LogicHEsoccer(ScummEngine_v90he *vm) : LogicHE(vm) {
	_userDataD = (double *)calloc(1732, sizeof(double));
	_intArray1 = 0;
	_intArray2 = 0;
	_intArraysAllocated = false;
	_array1013 = 0;
	_array1013Allocated = false;
}

LogicHEsoccer::~LogicHEsoccer() {
	free(_userDataD);
	op_1020(); // clear int arrays
	delete[] _array1013;
}

int32 LogicHEsoccer::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1001:
		res = op_1001(args);
		break;

	case 1002:
		res = op_1002(args);
		break;

	case 1004:
		res = op_1004(args);
		break;

	case 1006:
		res = op_1006(args[0], args[1], args[2], args[3]);
		break;

	case 1011:
		// args[4] is ignored!
		// soccer passes the argument, but then ends up not using it
		// Makes sense that they removed it for soccermls
		res = op_1011(args[0], args[1], args[2], args[3], args[5]);
		break;

	case 1012:
		res = op_1012(args);
		break;

	case 1013:
		res = op_1013(args[0], args[1], args[2]);
		break;

	case 1019:
		res = op_1019(args);
		break;

	case 1020:
		res = op_1020();
		break;

	case 1021:
		res = op_1021(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		break;

	case 8221968:
		// Someone had a fun and used his birthday as opcode number
		res = getFromArray(args[0], args[1], args[2]);
		break;

	default:
		// original range is 1001 - 1021
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

void LogicHEsoccer::beforeBootScript() {
	if (_intArraysAllocated)
		op_1020();

	_userDataD[530] = 0;
}

void LogicHEsoccer::initOnce() {
	// The original sets some paths here that we don't need to worry about
	_array1013Allocated = false;
	_userDataD[530] = 0;
}

int LogicHEsoccer::startOfFrame() {
	// This variable is some sort of flag that activates this mode
	int res = (int)_userDataD[530];

	// _userDataD[535] is not used!
	// soccer passes the argument, but then ends up not using it
	// Makes sense that they removed it for soccermls
	if (res)
		res = op_1011((int)_userDataD[531], (int)_userDataD[532], (int)_userDataD[533], (int)_userDataD[534], (int)_userDataD[536]);

	return res;
}

int LogicHEsoccer::op_1001(int32 *args) {
	return (int)(args[0] * sin((float)args[1]));
}

int LogicHEsoccer::op_1002(int32 *args) {
	return _vm->VAR(2) * args[0];
}

int LogicHEsoccer::op_1003(int32 *args) {
	double data[6], out[3];
	int i;

	for (i = 0; i < 6; i++) {
		data[i] = getFromArray(args[0], 0, i);
	}

	out[0] = data[1] * data[5] - data[4] * data[2];
	out[1] = data[5] * data[0] - data[3] * data[2];
	out[2] = data[4] * data[0] - data[3] * data[1];

	for (i = 0; i < 3; i++) {
		putInArray(args[0], 0, i, scumm_round(out[i]));
	}

	return 1;
}

int LogicHEsoccer::op_1004(int32 *args) {
	// Identical to LogicHEfootball::op_1004
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

int LogicHEsoccer::op_1006(int32 a1, int32 a2, int32 a3, int32 a4) {
	double v1 = a1 * 0.01;
	double v2 = a2 * 0.01;
	double v3 = a3 * 0.01;
	double var108, var109;

	_userDataD[529] = a4;

	var108 = atan2(v1, v3) * _userDataD[523] - a4;
	var109 = _userDataD[526] - _userDataD[528] + (_userDataD[521] - atan2(_userDataD[524] - v2, v3)) * _userDataD[522];

	writeScummVar(108, (int32)var108);
	writeScummVar(109, (int32)var109);

	return 1;
}

int LogicHEsoccer::op_1007(int32 *args) {
	// Used when the HE logo is shown
	// This initializes the _userDataD fields that are used in op_1006/op_1011

	_intArraysAllocated = false;
	double v14 = (double)args[0] * 0.01;
	double v13 = (double)args[2] * 0.01;
	_userDataD[524] = v14;
	double v12 = atan2(v13, v14);
	_userDataD[520] = v12;
	double v15 = atan2(v13 - (double)args[4] * 0.01, (double)args[3] * 0.01);
	double v19 = v15 * 2.0;
	double v17 = atan2(v13 - (double)args[4] * 0.01, v14);
	_userDataD[519] = v19;
	_userDataD[521] = v17;
	_userDataD[525] = (v17 - v12) * 2.0;
	_userDataD[527] = (double)args[5];
	_userDataD[526] = (double)args[6];
	_userDataD[528] = (double)args[7];
	_userDataD[522] = _userDataD[526] / _userDataD[525];
	_userDataD[523] = _userDataD[527] / _userDataD[519];
	_userDataD[518] = v13;

	// Clear both byte arrays
	memset(_byteArray1, 0, 4096);
	memset(_byteArray2, 0, 585);

	if (_array1013Allocated == 0 )
		op_1013(4, args[8], args[9]);

	return 1;
}

// Returns the square root of the sum of the squares of the arguments
static inline double sqrtSquare(double a1, double a2, double a3) {
	return sqrt(a1 * a1 + a2 * a2 + a3 * a3);
}

int LogicHEsoccer::op_1008(int32 *args) {
	// TODO: Used during a match (kicking?)

	return 1;
}

int LogicHEsoccer::op_1011(int32 a1, int32 a2, int32 a3, int32 a4, int32 a5) {
	// This is called on each frame by startOfFrame() if activated by op_1012.
	// This seems to do player placement!

	float v28 = 0.0;

	for (int i = 0; i < 18; i++) {
		// These seem to be some sort of percent? of angles?
		int v32 = getFromArray(a1, i, 0);
		int v6 = getFromArray(a1, i, 1);
		int v30 = getFromArray(a1, i, 2);

		float v29 = (double)v32 / 100.0;
		v28 = (double)v6 / 100.0;
		float v31 = (double)v30 / 100.0;

		if (i < 13) {
			int v25 = ((v32 + 2760) / 500 >= 0) ? ((v32 + 2750) / 500) : 0;
			int v24 = 10;

			if (v25 <= 10) {
				int v23 = 0;
				if ((v32 + 2750) / 500 >= 0)
					v23 = (v32 + 2750) / 500;

				v24 = v23;
			}

			int v22 = 0;
			if ((9219 - v30) / 500 >= 0)
				v22 = (9219 - v30) / 500;

			int v21 = 6;
			if (v22 <= 6) {
				int v20 = 0;
				if ((9219 - v30) / 500 >= 0)
					v20 = (9219 - v30) / 500;
				v21 = v20;
			}

			if (a5)
				putInArray(a5, 0, i, v24 + 11 * v21);
		}

		float v7 = atan2(_userDataD[524] - v28, (double)v31);
		int v8 = (int)(_userDataD[526] - (_userDataD[521] - v7) * _userDataD[522] + 300.0);

		double v9 = atan2(_userDataD[523], (double)v31);
		// x/y position?
		putInArray(a2, i, 0, (int32)(v29 * v9 + 640.0));
		putInArray(a2, i, 1, v8);

		double v10 = atan2(_userDataD[524], (double)v31);
		int v12 = (int)(_userDataD[526] - (_userDataD[521] - (float)v10) * _userDataD[522] + 300.0);
		double v13 = _userDataD[523];

		v29 = atan2(v29, v31);
		// x/y position?
		putInArray(a2, i + 22, 0, (int32)(v29 * v13 + 640.0));
		putInArray(a2, i + 22, 1, v12);
	}

	// soccer only uses one array here
	// soccermls/soccer2004 use four
	int start = (_vm->_game.id == GID_SOCCER) ? 19 : 18;
	int end = (_vm->_game.id == GID_SOCCER) ? 19 : 21;

	for (int i = start; i <= end; i++) {
		int v14 = getFromArray(a2, i, 0);
		int v15 = getFromArray(a2, i, 1);

		// This retains v28 from (i == 17)?
		float v16 = _userDataD[524] - v28;
		float v17 = v16 / tan((_userDataD[528] + v15 - _userDataD[526]) / (_userDataD[522] + _userDataD[521]));
		double v18 = tan((double)(v14 - 640) / _userDataD[523]) * v17;
		putInArray(a1, i, 0, (int)(v18 * 100.0));
		putInArray(a1, i, 2, (int)(v17 * 100.0));
	}

	op_1011_sub(a1, a3, a4, a4);

	return 1;
}

static inline int distance(int a1, int a2, int a3, int a4) {
	return (int)sqrt((double)((a4 - a3) * (a4 - a3) + (a2 - a1) * (a2 - a1)));
}

void LogicHEsoccer::op_1011_sub(int32 a1, int32 a2, int32 a3, int32 a4) {
	// As you can guess, this is called from op_1011
	// This seems to be checking distances between the players and the ball
	// and which distance is the shortest.

	int v6[13];
	int v7[13];
	int v8[13];
	int v18[195];

	for (int i = 0; i < 13; i++) {
		v6[i] = 0;
		v7[i] = getFromArray(a1, i, 0);
		v8[i] = getFromArray(a1, i, 2);
	}

	// 12 here, 13 up there
	// Probably 12 for players, 13 for players+ball
	for (int i = 0; i < 12; i++) {
		int v22 = a4;
		for (int j = i + 1; j < 13; j++) {
			v18[i * 15 + j] = distance(v7[i], v7[j], v8[i], v8[j]);
			putInArray(a2, i, j, v18[i * 15 + j]);
			putInArray(a2, j, i, v18[i * 15 + j]);
			if (v18[i * 15 + j] < v22) {
				v22 = v18[i * 15 + j];
				v6[i] = j + 1;
				v6[j] = i + 1;
			}
		}
	}

	int v13 = getFromArray(a1, 18, 0);
	int v14 = getFromArray(a1, 18, 2);
	int v15 = getFromArray(a1, 19, 0);
	int v16 = getFromArray(a1, 19, 2);
	int v19[15];
	int v20[15];

	if (_vm->_game.id == GID_SOCCER) {
		// soccer gets to be different
		for (int i = 0; i < 13; i++)
			v20[i] = distance(v13, v7[i], v14, v8[i]);

		for (int i = 0; i < 13; i++)
			v19[i] = distance(v15, v7[i], v16, v8[i]);
	} else {
		// soccermls and soccer2004 use two other arrays here
		int v9 = getFromArray(a1, 20, 0);
		int v10 = getFromArray(a1, 20, 2);
		int v11 = getFromArray(a1, 21, 0);
		int v12 = getFromArray(a1, 21, 2);

		for (int i = 0; i < 6; i++) {
			v20[i] = distance(v9, v7[i], v10, v8[i]);
			v19[i] = distance(v13, v7[i], v14, v8[i]);
		}

		for (int i = 6; i < 13; i++) {
			v20[i] = distance(v11, v7[i], v12, v8[i]);
			v19[i] = distance(v15, v7[i], v16, v8[i]);
		}
	}

	for (int i = 0; i < 13; i++) {
		putInArray(a2, 14, i, v20[i]);
		putInArray(a2, i, 14, v20[i]);
		putInArray(a2, 13, i, v19[i]);
		putInArray(a2, i, 13, v19[i]);
		putInArray(a3, 0, i, v6[i]);
	}
}

int LogicHEsoccer::op_1012(int32 *args) {
	// Used after op_1019
	// This function activates startOfFrame() to call op_1011
	// (Possibly field parameters?)

	_userDataD[530] = (args[0] != 0) ? 1 : 0;
	_userDataD[531] = args[1];
	_userDataD[532] = args[2];
	_userDataD[533] = args[3];
	_userDataD[534] = args[4];
	_userDataD[535] = args[5]; // unused!!!
	_userDataD[536] = args[6];

	return 1;
}

// Some strange power operation, ignores negative exponents
static inline double u32Pow(float a1, int a2) {
	if (a2 < 0)
		return 0.0;

	float v4 = 1.0;

	for (int i = 1; i <= a2; i++)
		v4 *= a1;

	return v4;
}

int LogicHEsoccer::op_sub5(int a1, int a2, int a3) {
	byte *v9 = _array1013 + 44 * a2;

	*((uint32 *)v9 + 4) = a3;
	*((uint32 *)v9) = a2;

	if (a1 > 2) {
		// Casual observation: 585 is also the size of _byteArray2
		*((uint32 *)v9 + 40) = 8 * a2 - 585;
		for (int i = 0; i < 8; i++)
			*((uint32 *)v9 + 4 * i + 8) = 0xffffffff;
	} else {
		for (int i = 0; i < 8; i++)
			*((uint32 *)v9 + 4 * i + 8) = op_sub5(a1 + 1, i + 8 * a2 + 1, a2);
	}

	return a2;
}

int LogicHEsoccer::op_1013(int32 a1, int32 a2, int32 a3) {
	// Creates _array1013 for *some* purpose
	// _array1013Temp is used again op_1014 for some reason...
	// Seems to be used in op_1015

	int v4 = (int)((1.0 - u32Pow(8.0, 4)) / -7.0);

	_array1013 = new byte[v4 * 44];
	memset(_array1013, 0, v4 * 44);
	_array1013Allocated = true;
	memset(_array1013Temp, 0, 44);

	for (int i = 0; i < 8; i++)
		_array1013Temp[i + 2] = op_sub5(1, i + 1, 0);

	// Yes, this is not endian-safe, but should not matter since we're
	// not saving/loading the data anywhere
	memcpy(_array1013, _array1013Temp, 44);

	return 1;
}

int LogicHEsoccer::op_1014(int32 a1, int32 a2, int32 a3, int32 a4, int32 a5, int32 a6, int32 a7, int32 a8, int32 a9, int32 a10, int32 a11, int32 a12, int32 a13, int32 a14) {
	// TODO: Used many times during a match
	// And called from op_1008!

	return 1;
}

int LogicHEsoccer::op_1019(int32 *args) {
	// Used at the beginning of a match
	// Initializes some arrays. Player positions?

	// These two arrays are used in op_1014 and op_1015
	for (int i = 0; i < 4096; i++)
		_byteArray1[i] = getFromArray(args[1], 0, i);

	for (int i = 0; i < 585; i++)
		_byteArray2[i] = getFromArray(args[0], 0, i);

	// Deallocate the two integer arrays
	if (_intArraysAllocated)
		op_1020();

	// Reallocate them
	_intArray1 = new uint32[1008];
	_intArray2 = new uint32[168];
	_intArraysAllocated = true;

	memset(_intArray1, 0, 4 * 4);
	memset(_intArray2, 0, 24 * 4);

	// These two arrays are used in op_1015
	for (int i = 0; i < 42; i++) {
		for (int j = 0; j < 24; j++)
			_intArray1[j + 24 * i] = getFromArray(args[3], 0, j + 24 * i);

		for (int j = 0; j < 4; j++)
			_intArray2[j + 4 * i] = getFromArray(args[2], 0, j + 4 * i);
	}

	return 1;
}

int LogicHEsoccer::op_1020() {
	// Deallocate integer arrays
	// The arrays can be allocated in op_1015 or op_1019

	delete[] _intArray1; _intArray1 = 0;
	delete[] _intArray2; _intArray2 = 0;
	_intArraysAllocated = false;

	return 1;
}

int LogicHEsoccer::op_1021(int32 a1, int32 a2, int32 a3, int32 a4, int32 a5, int32 a6, int32 a7) {
	// Used during a match (ball movement?)
	// Also called from op_1008

	int v10;
	if (a4 && a5)
		v10 = (int)(((double)a2 - (double)a5 * (double)a1 / (double)a4) * -1.0 * (double)a4 / (double)a5);
	else
		v10 = a1;

	int v9;
	if (a6 && a5)
		v9 = (int)(((double)a2 - (double)a5 * (double)a3 / (double)a6) * -1.0 * (double)a6 / (double)a5);
	else
		v9 = a3;

	// The final argument chooses whether to store the results for op_1008 or
	// store them in SCUMM variables.
	if (a7) {
		_var1021[0] = v10;
		_var1021[1] = v9;
	} else {
		writeScummVar(108, v10);
		writeScummVar(109, v9);
	}

	return 1;
}

/***********************
 * Backyard Baseball 2001
 *
 */

int LogicHEbaseball2001::versionID() {
	return 1;
}

int32 LogicHEbaseball2001::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 3001:
		// Check network status
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

/***********************
 * Backyard Basketball
 *
 */

int LogicHEbasketball::versionID() {
	return 1;
}

int32 LogicHEbasketball::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1001:
		break;

	case 1006:
		break;

	case 1011:
		break;

	case 1012:
		break;

	case 1035:
		break;

	case 1050:
		break;

	case 1051:
		break;

	case 1052:
		break;

	case 1056:
		break;

	case 1057:
		break;

	case 1058:
		break;

	case 1060:
		break;

	case 1064:
		break;

	case 1067:
		break;

	case 1073:
		break;

	case 1075:
		break;

	case 1076:
		break;

	case 1080:
		break;

	case 1081:
		break;

	case 1090:
		break;

	case 1091:
		break;

	case 1513:
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

/***********************
 * Moonbase Commander
 *
 */

int LogicHEmoonbase::versionID() {
	if (_vm->_game.features & GF_DEMO)
		return -100;
	else
		return 100;
}

} // End of namespace Scumm

#endif // ENABLE_HE
