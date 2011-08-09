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
	_array1013 = 0;
	_array1013Allocated = false;
}

LogicHEsoccer::~LogicHEsoccer() {
	free(_userDataD);
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

	case 1005: {
		// NOTE: This seems to be *completely* useless to call from dispatch
		// op_1005 is only useful from op_1014
		// However, the original does do this crazy setup to call it...
		float v11 = (float)args[6];
		float v12 = (float)args[7];
		float v13 = (float)args[8];
		float v14 = (float)args[9];
		res = op_1005(args[0], args[1], args[2], args[3], args[4], args[5], &v11, &v12, &v13, &v14);
		break;
	}

	case 1006:
		res = op_1006(args[0], args[1], args[2], args[3]);
		break;

	case 1007:
		res = op_1007(args);
		break;

	case 1008:
		res = op_1008(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18]);
		break;

	case 1011:
		res = op_1011(args[0], args[1], args[2], args[3], args[4], args[5]);
		break;

	case 1012:
		res = op_1012(args);
		break;

	case 1013:
		res = op_1013(args[0], args[1], args[2]);
		break;

	case 1014:
		res = op_1014(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13]);
		break;

	case 1016:
		res = op_1016(args);
		break;

	case 1017:
		res = op_1017(args);
		break;

	case 1019:
		res = op_1019(args);
		break;

	case 1021:
		res = op_1021(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		break;

	case 8221968:
		// Someone had a fun and used his birthday as opcode number
		res = getFromArray(args[0], args[1], args[2]);
		break;

	case 1010: case 1015: case 1018: case 1020:
		// Used only by the in-game editor (so, fall through)
	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

void LogicHEsoccer::beforeBootScript() {
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

	if (res)
		res = op_1011((int)_userDataD[531], (int)_userDataD[532], (int)_userDataD[533], (int)_userDataD[534], (int)_userDataD[535], (int)_userDataD[536]);

	return res;
}

int LogicHEsoccer::op_1001(int32 *args) {
	return (int)(args[0] * sin((float)args[1]));
}

int LogicHEsoccer::op_1002(int32 *args) {
	return _vm->VAR(2) * args[0];
}

int LogicHEsoccer::op_1003(int32 *args) {
	// NOTE: This function is never called, so it's here for reference only
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

int LogicHEsoccer::op_1005(float a1, float a2, float a3, float a4, float a5, float a6, float *a7, float *a8, float *a9, float *a10) {
	// Called from op_1014

	double v11 = a1 * a4 + a2 * a5 + a3 * a6;
	*a7 = a4 - (v11 + v11) * a1;
	*a8 = a5 - (v11 + v11) * a2;
	*a9 = a6 - (v11 + v11) * a3;
	*a10 = 1.0f; // It always does this. How curious!

	return 1;
}

int LogicHEsoccer::op_1006(int32 a1, int32 a2, int32 a3, int32 a4) {
	double v1 = a1 * 0.01;
	double v2 = a2 * 0.01;
	double v3 = a3 * 0.01;
	double var108, var109;

	_userDataD[529] = a4;

	var108 = atan2(v1, v3) * _userDataD[523] - a4;
	var109 = _userDataD[526] - _userDataD[528] - (_userDataD[521] - atan2(_userDataD[524] - v2, v3)) * _userDataD[522];

	writeScummVar(108, (int32)var108);
	writeScummVar(109, (int32)var109);

	return 1;
}

int LogicHEsoccer::op_1007(int32 *args) {
	// Used when the HE logo is shown
	// This initializes the _userDataD fields that are used in op_1006/op_1011

	float y1 = (double)args[0] / 100.0;
	float x1 = (double)args[1] / 100.0;
	float x2 = (double)args[2] / 100.0;
	float y2 = (double)args[3] / 100.0;
	float x3 = (double)args[4] / 100.0;

	_userDataD[518] = x2;
	_userDataD[519] = 2 * atan2(y2, x2 - x3);
	_userDataD[520] = atan2(y1, x2);
	_userDataD[521] = atan2(y1, x1);
	_userDataD[524] = y1;
	_userDataD[525] = 2 * (_userDataD[521] - _userDataD[520]);
	_userDataD[526] = args[6];
	_userDataD[527] = args[5];
	_userDataD[528] = args[7];
	_userDataD[522] = _userDataD[526] / _userDataD[525];
	_userDataD[523] = _userDataD[527] / _userDataD[519];

	// Clear both byte arrays
	memset(_byteArray1, 0, 4096);
	memset(_byteArray2, 0, 585);

	if (!_array1013Allocated)
		op_1013(4, args[8], args[9]);

	return 1;
}

// This looks like 3-dimensional distance where each argument the change in dimension?
static inline double sqrtSquare(double a1, double a2, double a3) {
	return sqrt(a1 * a1 + a2 * a2 + a3 * a3);
}

int LogicHEsoccer::op_1008(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19) {
	// Used during a match (kicking?)

	int v29 = 0;
	int v27 = 1;
	int v33 = 500;
	int v28 = a2;
	int v32 = a4;
	int v26 = 0;

	while (v27 <= a14) {
		if (a19 == 1 && a4 > 8819)
			v26 = 1;
		else if (a19 == 2 && (a2 < -2350 || a2 > 2350))
			v26 = 1;
		else if (a19 == 3 && (a2 < -2350 || a2 > 2350 || a4 < 6119 || a4 > 8819))
			v26 = 1;

		if (a3 > 0)
			a6 -= a11 * a13 / a12;

		int v30 = a2;
		int v31_1 = a3;
		int v31_2 = a4;
		a2 += a11 * a5 / a12;
		a3 += a11 * a6 / a12;
		a4 += a11 * a7 / a12;

		if (a3 > 0) {
			if (v26 && op_1014(v30, v31_1, v31_2, a5, a6, a7, 0, a17, a18, 3, a11, a12, a15, a16)) {
				a2 = _array1014[6];
				a3 = _array1014[7];
				a4 = _array1014[8];
				a5 = _array1014[3];
				a6 = _array1014[4];
				a7 = _array1014[5];
				putInArray(a1, v27, 0, v29);
				putInArray(a1, v27, 1, (int)sqrtSquare((double)(_array1014[6] - v28), 0.0, (double)(_array1014[8] - v32)));
				putInArray(a1, v27, 2, _array1014[6]);
				putInArray(a1, v27, 3, _array1014[7]);
				putInArray(a1, v27, 4, _array1014[8]);
				putInArray(a1, v27, 5, a5);
				putInArray(a1, v27, 6, a6);
				putInArray(a1, v27++, 7, a7);
			} 
		} else {
			a3 = 0;
			int v34 = a5;
			int v35 = a7;
			a5 = a5 * a8 / 100;

			if (a6) {
				int v18 = ABS(a6);
				if (v18 > ABS(v33))
					a6 = ABS(v33);
				a6 = ABS(a9 * a6) / 100;
			}

			a7 = a10 * a7 / 100;

			if (v33 >= 0) {
				if (op_1014(v30, v31_1, v31_2, v34, v33, v35, 0, a17, a18, 3, a11, a12, a15, a16)) {
					a2 = _array1014[6];
					a3 = _array1014[7];
					a4 = _array1014[8];
					a5 = _array1014[3];
					a6 = _array1014[4];
					a7 = _array1014[5];
				}
			} else {
				if (v26) {
					op_1021(a2, 0, a4, v34 ,v33, v35, 1);

					if (op_1014(v30, v31_1, v31_2, v34, v33, v35, 0, a17, a18, 3, a11, a12, a15, a16)) {
						a2 = _array1014[6];
						a3 = _array1014[7];
						a4 = _array1014[8];
						a5 = _array1014[3];
						a6 = _array1014[4];
						a7 = _array1014[5];
					} else {
						int v19 = a7 + v31_2 - _var1021[1];
						int v20 = ABS(v33);

						if (op_1014(_var1021[0], 0, _var1021[1], a5 + v30 - _var1021[0], v20 - v31_1, v19, 0, a17, a18, 3, a11, a12, a15, a16)) {
							a2 = _array1014[6];
							a3 = _array1014[7];
							a4 = _array1014[8];
							a5 = _array1014[3];
							a6 = _array1014[4];
							a7 = _array1014[5];
						}
					}
				}
			}

			v33 = a6;
			putInArray(a1, v27, 0, v29);
			putInArray(a1, v27, 1, (int32)sqrtSquare(a2 - v28, 0.0, a4 - v32));
			putInArray(a1, v27, 2, a2);
			putInArray(a1, v27, 3, a3);
			putInArray(a1, v27, 4, a4);
			putInArray(a1, v27, 5, a5);
			putInArray(a1, v27, 6, a6);
			putInArray(a1, v27++, 7, a7);
		}

		v29++;
	}

	return 1;
}

int LogicHEsoccer::op_1011(int32 a1, int32 a2, int32 a3, int32 a4, int32 a5, int32 a6) {
	// This is called on each frame by startOfFrame() if activated by op_1012.
	// This seems to do player placement!
	// It also seems to be doing camera panning

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
			int v25 = ((v32 + 2750) / 500 >= 0) ? ((v32 + 2750) / 500) : 0;
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

			if (a6)
				putInArray(a6, 0, i, v24 + 11 * v21);
		}

		float v7 = atan2(_userDataD[524] - v28, (double)v31);
		int v8 = (int)(_userDataD[526] - (_userDataD[521] - v7) * _userDataD[522] - 300.0);
		double v9 = _userDataD[523];

		// x/y position of objects
		putInArray(a2, i, 0, (int32)(atan2(v29, v31) * v9 + 640.0));
		putInArray(a2, i, 1, v8);

		double v10 = atan2(_userDataD[524], (double)v31);
		int v12 = (int)(_userDataD[526] - (_userDataD[521] - (float)v10) * _userDataD[522] - 300.0);
		double v13 = _userDataD[523];

		// x/y position of shadows
		putInArray(a2, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 0, (int32)(atan2(v29, v31) * v13 + 640.0));
		putInArray(a2, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 1, v12);
	}

	// soccer only uses one array here
	// soccermls/soccer2004 use four
	int start = (_vm->_game.id == GID_SOCCER) ? 19 : 18;
	int end = (_vm->_game.id == GID_SOCCER) ? 19 : 21;

	// The following loop is doing cursor scaling
	// The further up on the screen, the smaller the cursor is
	for (int i = start; i <= end; i++) {
		int v14 = getFromArray(a2, i, 0);
		int v15 = getFromArray(a2, i, 1);

		// This retains v28 from (i == 17)?
		float v16 = _userDataD[524] - v28;
		float v17 = v16 / tan((_userDataD[528] + v15 - _userDataD[526]) / _userDataD[522] + _userDataD[521]);
		double v18 = tan((double)(v14 - ((_vm->_game.id == GID_SOCCER) ? 0 : 640)) / _userDataD[523]) * v17;
		putInArray(a1, i, 0, (int)(v18 * 100.0));
		putInArray(a1, i, 2, (int)(v17 * 100.0));
	}

	op_1011_sub(a1, a3, a4, a5);

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
			v20[i] = distance(v15, v7[i], v16, v8[i]);

		for (int i = 0; i < 13; i++)
			v19[i] = distance(v13, v7[i], v14, v8[i]);
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
	_userDataD[535] = args[5];
	_userDataD[536] = args[6];

	return 1;
}

int LogicHEsoccer::op_sub5(int a1, int a2, int a3) {
	uint32 *v9 = _array1013 + 11 * a2;

	v9[0] = a2;
	v9[1] = a3;

	if (a1 > 2) {
		// Casual observation: 585 is also the size of _byteArray2
		v9[10] = 8 * a2 - 585;
		for (int i = 0; i < 8; i++)
			v9[i + 2] = 0xffffffff;
	} else {
		for (int i = 0; i < 8; i++)
			v9[i + 2] = op_sub5(a1 + 1, i + 8 * a2 + 1, a2);
	}

	return a2;
}

int LogicHEsoccer::op_1013(int32 a1, int32 a2, int32 a3) {
	// Creates _array1013 for *some* purpose
	// Seems to be used in op_1014 and op_1015

	_array1013 = new uint32[585 * 11];
	_array1013Allocated = true;
	for (int i = 0; i < 585 * 11; i++)
		_array1013[i] = 0;

	for (int i = 0; i < 8; i++)
		_array1013[i + 2] = op_sub5(1, i + 1, 0);

	return 1;
}

int LogicHEsoccer::op_1014(int32 a1, int32 a2, int32 a3, int32 a4, int32 a5, int32 a6, int32 a7, int32 a8, int32 a9, int32 a10, int32 a11, int32 a12, int32 a13, int32 a14) {
	// Used many times during a match
	// And called from op_1008!
	// This seems to be doing collision handling

	double v31 = (double)a1;
	double v29 = (double)a2;
	double v27 = (double)a3;
	double v15, v28 = 0.0, v30 = 0.0, v32 = 0.0;

	writeScummVar(108, 0);
	writeScummVar(109, 0);

	switch (a10) {
	case 1:
	case 3:
		v32 = (double)a4 * (double)a11 / (double)a12 / 100.0;
		v30 = (double)a5 * (double)a11 / (double)a12 / 100.0;
		v28 = (double)a6 * (double)a11 / (double)a12 / 100.0;
		break;
	case 2:
		v15 = sqrtSquare((double)a4 * (double)a11 / (double)a12, (double)a5 * (double)a11 / (double)a12, (double)a6 * (double)a11 / (double)a12);

		if (v15 != 0.0) {
			double v26 = (double)ABS(a4) * (double)a11 / (double)a12 * 50.0 / v15;
			a1 = (int)((double)a1 + v26);
			double v25 = (double)ABS(a5) * (double)a11 / (double)a12 * 50.0 / v15;
			a2 = (int)((double)a2 + v25);
			double v24 = (double)ABS(a6) * (double)a11 / (double)a12 * 50.0 / v15;
			a3 = (int)((double)a3 + v24);
		}

		v31 = (double)a1 / (double)a3 * 3869.0;
		v29 = ((double)a2 - _userDataD[524] * 100.0) / (double)a3 * 3869.0 + _userDataD[524] * 100.0;
		v27 = 3869.0;
		v32 = ((double)a1 - v31) / 100.0;
		v30 = ((double)a2 - v29) / 100.0;
		v28 = ((double)a3 - 3869.0) / 100.0;
		break;
	}
	
	int v41 = 0;
	float v55[336];
	memset(v55, 0, 336 * sizeof(float));

	if (op_1014_sub0(v31, v29, v27, v32, v30, v28)) {
		int v45 = 0;
		float v46;

		for (Common::List<byte>::const_iterator it = _list1014.begin(); it != _list1014.end(); it++) {
			float v18[2] = { v30 * 100.0, v28 * 100.0 };
			float v19 = v32 * 100.0;
			float v56, v58, v67;
			float v35, v33, v57;

			if (op_1014_sub1(*it, v31, v29, v27, v19, v18, v67, v58, v56, a9, a8, &v35, &v33, &v57, &v46)) {
				v55[v45 * 8] = *it;
				v55[v45 * 8 + 1] = sqrtSquare(v67 - v31, v58 - v29, v56 - v27);
				v55[v45 * 8 + 2] = v67;
				v55[v45 * 8 + 3] = v58;
				v55[v45 * 8 + 4] = v56;
				v55[v45 * 8 + 5] = a12 * v35 / a11;
				v55[v45 * 8 + 6] = a12 * v33 / a11;
				v55[v45 * 8 + 7] = a12 * v57 / a11;
				v41 = 1;
				v45++;
			}
		}

		if (v41) {
			if (v45 != 1)
				op_1014_sub2(v55, 42, 8, 1);

			int v22, v39, v42;
			float v59[8];
			int v47[10];

			switch (a10) {
				case 1:
					for (int i = 0; i < 8; i++)
						v59[i] = v55[i];
					v22 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0));
					v42 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0  + 2.0));
					op_1014_sub3(v59, 8, a8, a9, (int)v31, (int)v29, (int)v27, v46, v22, v42, v39, v47);
					for (int i = 0; i < 10; i++)
						putInArray(a7, 0, i, v47[i]);
					break;
				case 2:
					if (v45)
						writeScummVar(109, (int)v55[(v45 - 1) * 8]);
					else
						writeScummVar(109, 0);
					break;
				case 3:
					for (int i = 0; i < 8; i++)
						v59[i] = v55[i];
					v22 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0));
					v42 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(a9, 0, (int)((v59[0] - 1.0) * 4.0  + 2.0));
					op_1014_sub3(v59, 8, a8, a9, (int)v31, (int)v29, (int)v27, v46, v22, v42, v39, v47);
					for (int i = 0; i < 10; i++)
						_array1014[i] = v47[i];
					break;
			}
		}
	}

	writeScummVar(108, v41);

	_list1014.clear();

	return v41;
}

int LogicHEsoccer::op_1014_sub0(float a1, float a2, float a3, float a4, float a5, float a6) {
	float v36 = a1 / 100.0;
	float v37 = v36 + 52.0;
	float v28 = v37 + a4;

	int v33, v29;

	if (((int)v28 / 52) ^ ((int)v37 / 52)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v37 / 52) {
		v29 = 0;
		v33 = 1;
	} else {
		v33 = 0;
		v29 = 1;
	}

	uint32 v20[8];
	for (int i = 0; i < 4; i++) {
		v20[i] = v29;
		v20[i + 4] = v33;
	}

	float v38 = a2 / 100.0;
	float v17 = v38 + a5;

	if (((int)v17 / 20) ^ ((int)v38 / 20)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v38 / 20) {
		v33 = 1;
		v29 = 0;
	} else {
		v29 = 1;
		v33 = 0;
	}

	for (int i = 0; i < 2; i++) {
		if (v20[i * 4 + 0])
			v20[i * 4 + 0] = v29;
		if (v20[i * 4 + 1])
			v20[i * 4 + 1] = v29;
		if (v20[i * 4 + 2])
			v20[i * 4 + 2] = v33;
		if (v20[i * 4 + 3])
			v20[i * 4 + 3] = v33;
	}

	float v39 = a3 / 100.0;
	float v40 = v39 - 38.69;
	float v31 = v40 + a6;

	if (((int)v31 / 36) ^ ((int)v40 / 36)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v40 / 36) {
		v29 = 0;
		v33 = 1;
	} else {
		v33 = 0;
		v29 = 1;
	}

	for (int i = 0; i <= 6; i += 2) {
		if (v20[i])
			v20[i] = v29;
		if (v20[i + 1])
			v20[i + 1] = v33;
	}

	int v19 = 0;

	for (int i = 0; i < 8; i++) {
		if (v20[i]) {
			uint32 *ptr = _array1013 +  _array1013[i + 2] * 11;
			v19 += op_1014_sub0_0(ptr[0], ptr[1], &ptr[2], ptr[10]);
		}
	}

	writeScummVar(109, v19);
	return v19;
}

int LogicHEsoccer::op_1014_sub0_0(int a1, int a2, uint32 *a3, int a4) {
	int v20 = 0;

	if (a3[0] == 0xffffffff) {
		for (int i = 0; i < 8; i++) {
			if (_byteArray1[i + a4]) {
				op_1014_sub0_0_0(_byteArray1[i + a4]);
				v20 = 1;
			}
		}
	} else {
		if (_byteArray2[a1]) {
			for (int i = 0; i < 8; i++) {
				uint32 *ptr = _array1013 + a3[i] * 11;
				v20 += op_1014_sub0_0(ptr[0], ptr[1], &ptr[2], ptr[10]);
			}
		}
	}

	return v20;
}

void LogicHEsoccer::op_1014_sub0_0_0(byte a1) {
	// Add a1 to the list if not found
	for (Common::List<byte>::const_iterator it = _list1014.begin(); it != _list1014.end(); it++)
		if (*it == a1)
			return;

	_list1014.push_back(a1);
}

int LogicHEsoccer::op_1014_sub1(int a1, float a2, float a3, float a4, float a5, float *a6, float &a7, float &a8, float &a9, int a10, int a11, float *a12, float *a13, float *a14, float *a15) {
	// TODO: This can easily be optimized, but I'd rather hold off on that
	// until it's confirmed that the code actually works

	int v248 = 0;
	float v274 = a3 + 1.0;
	float v254 = a2 + a5;
	float v250 = v274 + a6[0];
	float v246 = a4 + a6[1];

	if (v274 <= 1.0001 && v250 < 0.0) {
		v250 = 0.0;
		a6[0] = ABS((int)a6[0]);
	}

	int v261 = getFromArray(a10, 0, 4 * a1 - 1);
	int v256[24];
	for (int i = 0; i < 24; i++)
		v256[i] = getFromArray(a11, 0, v261 + i);

	for (int i = 0; i < 6; i++) {
		// This assigns variables from v256 based on i
		// TODO: We probably can merge sub1_0 and sub1_1 at one point
		float v233, v267, v264, v238, v273, v259, v237, v271, v242, v269, v253, v240;
		float v265, v260, v255;
		op_1014_sub1_0(i, v233, v267, v264, v238, v273, v259, v237, v271, v242, v269, v253, v240, v256);
		op_1014_sub1_1(v233, v267, v264, v238, v273, v259, v237, v271, v242, v269, v253, v240, v265, v260, v255);

		double v14 = sqrt(v265 * v265 + v260 * v260 + v255 * v255);
		float v243 = v14;

		// The original did not initialize these variables and would
		// use them uninitialized if v14 == 0.0
		float v234 = 0.0, v247 = 0.0, v245 = 0.0;

		if (v14 != 0.0) {
			v234 = v265 / v243;
			v247 = v260 / v243;
			v245 = v255 / v243;
		}
		double v263 = 5.0;

		float v15 = v264 - a4;
		float v16 = v267 - v274;
		float v17 = v233 - a2;
		double v239 = op_1014_sub1_2(v234, v247, v245, v17, v16, v15);

		float v18 = v246 - a4;
		float v19 = v250 - v274;
		float v20 = v254 - a2;
		double v258 = op_1014_sub1_2(v234, v247, v245, v20, v19, v18);

		if (fabs(v258) > 0.00000001)
			v263 = v239 / v258;

		if (v263 >= 0.0 && fabs(v263) <= 1.0 && v258 != 0.0) {
			double v272 = a2 + (v254 - a2) * v263;
			double v270 = v274 + (v250 - v274) * v263 + 5.0;
			double v268 = a4 + (v246 - a4) * v263;

			float v21 = v242 - v264;
			float v22 = v271 - v267;
			float v23 = v237 - v233;
			float v24 = v259 - v264;
			float v25 = v273 - v267;
			float v26 = v238 - v233;
			double v27 = op_1014_sub1_2(v26, v25, v24, v23, v22, v21);
			double v28 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);

			double v232;
			if (v27 / (sqrtSquare(v237 - v233, v271 - v267, v242 - v264) * v28) >= -1.0) {
				float v29 = v242 - v264;
				float v30 = v271 - v267;
				float v31 = v237 - v233;
				float v32 = v259 - v264;
				float v33 = v273 - v267;
				float v34 = v238 - v233;
				double v35 = op_1014_sub1_2(v34, v33, v32, v31, v30, v29);
				double v36 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);
				v232 = v35 / (sqrtSquare(v237 - v233, v271 - v267, v242 - v264) * v36);
			} else {
				v232 = -1.0;
			}

			double v231;
			if (v232 <= 1.0) {
				float v37 = v242 - v264;
				float v38 = v271 - v267;
				float v39 = v237 - v233;
				float v40 = v259 - v264;
				float v41 = v273 - v267;
				float v42 = v238 - v233;
				double v43 = op_1014_sub1_2(v42, v41, v40, v39, v38, v37);
				double v44 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);

				double v230;
				if (v43 / (sqrtSquare(v237 - v233, v271 - v267, v242 - v264) * v44) >= -1.0) {
					float v45 = v242 - v264;
					float v46 = v271 - v267;
					float v47 = v237 - v233;
					float v48 = v259 - v264;
					float v49 = v273 - v267;
					float v50 = v238 - v233;
					double v51 = op_1014_sub1_2(v50, v49, v48, v47, v46, v45);
					double v52 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);
					v230 = v51 / (sqrtSquare(v237 - v233, v271 - v267, v242 - v264) * v52);
				} else {
					v230 = -1.0;
				}

				v231 = v230;
			} else {
				v231 = 1.0;
			}

			double v235 = acos(v231);

			float v53 = v268 - v264;
			float v54 = v270 - v267;
			float v55 = v272 - v233;
			float v56 = v259 - v264;
			float v57 = v273 - v267;
			float v58 = v238 - v233;
			double v59 = op_1014_sub1_2(v58, v57, v56, v55, v54, v53);
			double v60 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);

			double v229;
			if (v59 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v60) >= -1.0) {
				float v61 = v268 - v264;
				float v62 = v270 - v267;
				float v63 = v272 - v233;
				float v64 = v259 - v264;
				float v65 = v273 - v267;
				float v66 = v238 - v233;
				double v67 = op_1014_sub1_2(v66, v65, v64, v63, v62, v61);
				double v68 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);
				v229 = v67 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v68);
			} else {
				v229 = -1.0;
			}

			double v228;
			if (v229 <= 1.0) {
				float v69 = v268 - v264;
				float v70 = v270 - v267;
				float v71 = v272 - v233;
				float v72 = v259 - v264;
				float v73 = v273 - v267;
				float v74 = v238 - v233;
				double v75 = op_1014_sub1_2(v74, v73, v72, v71, v70, v69);
				double v76 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);

				double v227;
				if (v75 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v76) >= -1.0) {
					float v77 = v268 - v264;
					float v78 = v270 - v267;
					float v79 = v272 - v233;
					float v80 = v259 - v264;
					float v81 = v273 - v267;
					float v82 = v238 - v233;
					double v83 = op_1014_sub1_2(v82, v81, v80, v79, v78, v77);
					double v84 = sqrtSquare(v238 - v233, v273 - v267, v259 - v264);
					v227 = v83 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v84);
				} else {
					v227 = -1.0;
				}

				v228 = v227;
			} else {
				v228 = 1.0;
			}

			double v249 = acos(v228);

			float v85 = v268 - v264;
			float v86 = v270 - v267;
			float v87 = v272 - v233;
			float v88 = v242 - v264;
			float v89 = v271 - v267;
			float v90 = v237 - v233;
			double v91 = op_1014_sub1_2(v90, v89, v88, v87, v86, v85);
			double v92 = sqrtSquare(v237 - v233, v271 - v267, v242 - v264);

			double v226;
			if (v91 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v92) >= -1.0) {
				float v93 = v268 - v264;
				float v94 = v270 - v267;
				float v95 = v272 - v233;
				float v96 = v242 - v264;
				float v97 = v271 - v267;
				float v98 = v237 - v233;
				double v99 = op_1014_sub1_2(v98, v97, v96, v95, v94, v93);
				double v100 = sqrtSquare(v237 - v233, v271 - v267, v242 - v264);
				v226 = v99 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v100);
			} else {
				v226 = -1.0;
			}

			double v225;
			if (v226 <= 1.0) {
				float v101 = v268 - v264;
				float v102 = v270 - v267;
				float v103 = v272 - v233;
				float v104 = v242 - v264;
				float v105 = v271 - v267;
				float v106 = v237 - v233;
				double v107 = op_1014_sub1_2(v106, v105, v104, v103, v102, v101);
				double v108 = sqrtSquare(v237 - v233, v271 - v267, v242 - v264);

				double v224;
				if (v107 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v108) >= -1.0) {
					float v109 = v268 - v264;
					float v110 = v270 - v267;
					float v111 = v272 - v233;
					float v112 = v242 - v264;
					float v113 = v271 - v267;
					float v114 = v237 - v233;
					double v115 = op_1014_sub1_2(v114, v113, v112, v111, v110, v109);
					double v116 = sqrtSquare(v237 - v233, v271 - v267, v242 - v264);
					v224 = v115 / (sqrtSquare(v272 - v233, v270 - v267, v268 - v264) * v116);
				} else {
					v224 = -1.0;
				}

				v225 = v224;
			} else {
				v225 = 1.0;
			}

			double v252 = acos(v225);

			if (v249 + v252 - 0.001 <= v235) {
				float v117 = v242 - v240;
				float v118 = v271 - v253;
				float v119 = v237 - v269;
				float v120 = v259 - v240;
				float v121 = v273 - v253;
				float v122 = v238 - v269;
				double v123 = op_1014_sub1_2(v122, v121, v120, v119, v118, v117);
				double v124 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);

				double v223;
				if (v123 / (sqrtSquare(v237 - v269, v271 - v253, v242 - v240) * v124) >= -1.0) {
					float v125 = v242 - v240;
					float v126 = v271 - v253;
					float v127 = v237 - v269;
					float v128 = v259 - v240;
					float v129 = v273 - v253;
					float v130 = v238 - v269;
					double v131 = op_1014_sub1_2(v130, v129, v128, v127, v126, v125);
					double v132 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);
					v223 = v131 / (sqrtSquare(v237 - v269, v271 - v253, v242 - v240) * v132);
				} else {
					v223 = -1.0;
				}

				double v222;
				if (v223 <= 1.0) {
					float v133 = v242 - v240;
					float v134 = v271 - v253;
					float v135 = v237 - v269;
					float v136 = v259 - v240;
					float v137 = v273 - v253;
					float v138 = v238 - v269;
					double v139 = op_1014_sub1_2(v138, v137, v136, v135, v134, v133);
					double v140 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);

					double v221;
					if (v139 / (sqrtSquare(v237 - v269, v271 - v253, v242 - v240) * v140) >= -1.0) {
						float v141 = v242 - v240;
						float v142 = v271 - v253;
						float v143 = v237 - v269;
						float v144 = v259 - v240;
						float v145 = v273 - v253;
						float v146 = v238 - v269;
						double v147 = op_1014_sub1_2(v146, v145, v144, v143, v142, v141);
						double v148 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);
						v221 = v147 / (sqrtSquare(v237 - v269, v271 - v253, v242 - v240) * v148);
					} else {
						v221 = -1.0;
					}

					v222 = v221;
				} else {
					v222 = 1.0;
				}

				v235 = acos(v222);

				float v149 = v268 - v240;
				float v150 = v270 - v253;
				float v151 = v272 - v269;
				float v152 = v259 - v240;
				float v153 = v273 - v253;
				float v154 = v238 - v269;
				double v155 = op_1014_sub1_2(v154, v153, v152, v151, v150, v149);
				double v156 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);

				double v220;
				if (v155 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v156) >= -1.0) {
					float v157 = v268 - v240;
					float v158 = v270 - v253;
					float v159 = v272 - v269;
					float v160 = v259 - v240;
					float v161 = v273 - v253;
					float v162 = v238 - v269;
					double v163 = op_1014_sub1_2(v162, v161, v160, v159, v158, v157);
					double v164 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);
					v220 = v163 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v164);
				} else {
					v220 = -1.0;
				}

				double v219;
				if (v220 <= 1.0) {
					float v165 = v268 - v240;
					float v166 = v270 - v253;
					float v167 = v272 - v269;
					float v168 = v259 - v240;
					float v169 = v273 - v253;
					float v170 = v238 - v269;
					double v171 = op_1014_sub1_2(v170, v169, v168, v167, v166, v165);
					double v172 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);

					double v218;
					if (v171 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v172) >= -1.0) {
						float v173 = v268 - v240;
						float v174 = v270 - v253;
						float v175 = v272 - v269;
						float v176 = v259 - v240;
						float v177 = v273 - v253;
						float v178 = v238 - v269;
						double v179 = op_1014_sub1_2(v178, v177, v176, v175, v174, v173);
						double v180 = sqrtSquare(v238 - v269, v273 - v253, v259 - v240);
						v218 = v179 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v180);
					} else {
						v218 = -1.0;
					}

					v219 = v218;
				} else {
					v219 = 1.0;
				}

				double v257 = acos(v219);

				float v181 = v268 - v240;
				float v182 = v270 - v253;
				float v183 = v272 - v269;
				float v184 = v242 - v240;
				float v185 = v271 - v253;
				float v186 = v237 - v269;
				double v187 = op_1014_sub1_2(v186, v185, v184, v183, v182, v181);
				double v188 = sqrtSquare(v237 - v269, v271 - v253, v242 - v240);

				double v217;
				if (v187 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v188) >= -1.0) {
					float v189 = v268 - v240;
					float v190 = v270 - v253;
					float v191 = v272 - v269;
					float v192 = v242 - v240;
					float v193 = v271 - v253;
					float v194 = v237 - v269;
					double v195 = op_1014_sub1_2(v194, v193, v192, v191, v190, v189);
					double v196 = sqrtSquare(v237 - v269, v271 - v253, v242 - v240);
					v217 = v195 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v196);
				} else {
					v217 = -1.0;
				}

				double v216;
				if (v217 <= 1.0) {
					float v197 = v268 - v240;
					float v198 = v270 - v253;
					float v199 = v272 - v269;
					float v200 = v242 - v240;
					float v201 = v271 - v253;
					float v202 = v237 - v269;
					double v203 = op_1014_sub1_2(v202, v201, v200, v199, v198, v197);
					double v204 = sqrtSquare(v237 - v269, v271 - v253, v242 - v240);

					double v215;
					if (v203 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v204) >= -1.0) {
						float v205 = v268 - v240;
						float v206 = v270 - v253;
						float v207 = v272 - v269;
						float v208 = v242 - v240;
						float v209 = v271 - v253;
						float v210 = v237 - v269;
						double v211 = op_1014_sub1_2(v210, v209, v208, v207, v206, v205);
						double v212 = sqrtSquare(v237 - v269, v271 - v253, v242 - v240);
						v215 = v211 / (sqrtSquare(v272 - v269, v270 - v253, v268 - v240) * v212);
					} else {
						v215 = -1.0;
					}

					v216 = v215;
				} else {
					v216 = 1.0;
				}

				double v262 = acos(v216);

				if (v257 + v262 - 0.001 <= v235) {
					if (v248) {
						double v213 = sqrtSquare(a2 - v272, v274 - v270, a4 - v268);
						if (sqrtSquare(a2 - a7, v274 - a8, a4 - a9) > v213) {
							a7 = v272 - v234 * 3.0;
							a8 = v270 - v247 * 3.0;
							a9 = v268 - v245 * 3.0;
							op_1005(v234, v247, v245, a5, a6[0], a6[1], a12, a13, a14, a15);
						}
					} else {
						a7 = v272 - v234 * 3.0;
						a8 = v270 - v247 * 3.0;
						a9 = v268 - v245 * 3.0;
						op_1005(v234, v247, v245, a5, a6[0], a6[1], a12, a13, a14, a15);
					}

					v248 = 1;
				}
			}
		}
	}

	return v248;
}

void LogicHEsoccer::op_1014_sub1_0(int a1, float &a2, float &a3, float &a4, float &a5, float &a6, float &a7, float &a8, float &a9, float &a10, float &a11, float &a12, float &a13, int a14[24]) {
	// sub_1000176B
	// Note that this originally returned a value, but said value was never used
	// TODO: This can probably be shortened using a few tables...

	switch (a1) {
	case 0:
		a2 = a14[0];
		a3 = a14[1];
		a4 = a14[2];
		a5 = a14[3];
		a6 = a14[4];
		a7 = a14[5];
		a8 = a14[6];
		a9 = a14[7];
		a10 = a14[8];
		a11 = a14[9];
		a12 = a14[10];
		a13 = a14[11];
		break;
	case 1:
		a2 = a14[0];
		a3 = a14[1];
		a4 = a14[2];
		a5 = a14[6];
		a6 = a14[7];
		a7 = a14[8];
		a8 = a14[12];
		a9 = a14[13];
		a10 = a14[14];
		a11 = a14[18];
		a12 = a14[19];
		a13 = a14[20];
		break;
	case 2:
		a2 = a14[3];
		a3 = a14[4];
		a4 = a14[5];
		a5 = a14[15];
		a6 = a14[16];
		a7 = a14[17];
		a8 = a14[9];
		a9 = a14[10];
		a10 = a14[11];
		a11 = a14[21];
		a12 = a14[22];
		a13 = a14[23];
		break;
	case 3:
		a2 = a14[0];
		a3 = a14[1];
		a4 = a14[2];
		a5 = a14[12];
		a6 = a14[13];
		a7 = a14[14];
		a8 = a14[3];
		a9 = a14[4];
		a10 = a14[5];
		a11 = a14[15];
		a12 = a14[16];
		a13 = a14[17];
		break;
	case 4:
		a2 = a14[6];
		a3 = a14[7];
		a4 = a14[8];
		a5 = a14[9];
		a6 = a14[10];
		a7 = a14[11];
		a8 = a14[18];
		a9 = a14[19];
		a10 = a14[20];
		a11 = a14[21];
		a12 = a14[22];
		a13 = a14[23];
		break;
	case 5:
		a2 = a14[15];
		a3 = a14[16];
		a4 = a14[17];
		a5 = a14[12];
		a6 = a14[13];
		a7 = a14[14];
		a8 = a14[21];
		a9 = a14[22];
		a10 = a14[23];
		a11 = a14[18];
		a12 = a14[19];
		a13 = a14[20];
		break;
	}
}

void LogicHEsoccer::op_1014_sub1_1(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10, float a11, float a12, float &a13, float &a14, float &a15) {
	// sub_100016AD
	// Note that this originally returned a value, but said value was never used

	a13 = (a5 - a2) * (a12 - a9) - (a11 - a8) * (a6 - a3);
	a14 = ((a4 - a1) * (a12 - a9) - (a10 - a7) * (a6 - a3)) * -1.0;
	a15 = (a4 - a1) * (a11 - a8) - (a10 - a7) * (a5 - a2);
}

double LogicHEsoccer::op_1014_sub1_2(float a1, float a2, float a3, float a4, float a5, float a6) {
	return a1 * a4 + a2 * a5 + a3 * a6;
}

void LogicHEsoccer::op_1014_sub2(float *a1, int a2, int a3, int a4) {
	// This seems to do some kind of sorting...
	// And then copies in groups of a3 (which, btw, is always 8)

	int v6 = 1;
	int v8 = 0;

	while (v6) {
		v6 = 0;

		while (v8 <= a2 - 2 && a1[(v8 + 1) * 8] != 0.0) {
			if (a1[a4 + v8 * 8] == 0 || a1[a4 + v8 * 8] > a1[a4 + (v8 + 1) * 8]) {
				v6 = 1;

				for (int i = 0; i < a3; i++) {
					float v5 = a1[i + v8 * 8];
					a1[i + v8 * 8] = a1[i + (v8 + 1) * 8];
					a1[i + (v8 + 1) * 8] = v5;
				}
			}

			v8++;
		}
	}
}

int LogicHEsoccer::op_1014_sub3(float *a1, int a2, int a3, int a4, int a5, int a6, int a7, float a8, int a9, int a10, int a11, int *a12) {
	a12[0] = a9;
	a12[1] = a10;
	a12[2] = a11;
	a12[3] = (int)(a1[5] * (double)a10 / 100.0);
	a12[4] = (int)(a1[6] * (double)a10 / 100.0 * a8); // Note: a8 should always be 1
	a12[5] = (int)(a1[7] * (double)a10 / 100.0);
	a12[6] = (int)a1[2];
	a12[7] = (int)a1[3];
	a12[8] = (int)a1[4];
	a12[9] = (int)a1[0];
	return a12[9];
}

int LogicHEsoccer::op_1016(int32 *args) {
	// Called when a goal is scored

	int result = 0;

	double v9 = (double)args[1] / 100.0;
	double v13 = (double)args[2] / 100.0;
	double v12 = (double)args[3] / 100.0;
	double v18 = v13 * v13;
	double v10 = (double)args[0] / 100.0 * (double)args[0] / 100.0;
	double v11 = v9 * v9;
	double v19 = (v9 * v9 * v12 * v12 + 2.0 * v9 * v12 * v18 + v18 * v18) * v10 * v10 - (v10 + v11) * v12 * v12 * v10 * v10;

	if (v19 >= 0.0) {
		double v6 = sqrt(v19);
		double v17 = ((v9 * v12 + v18) * v10 + v6) / (v10 + v11 + v10 + v11);
		double v16 = ((v9 * v12 + v18) * v10 - v6) / (v10 + v11 + v10 + v11);
		double v7, v14;

		if (v17 <= 0.0 || (v7 = sqrt(v17), v14 = acos(v7 / v13), v14 > 0.7853981633974475)) {
			double v8, v15;
			if (v16 <= 0.0 || (v8 = sqrt(v16), v15 = acos(v8 / v13), v15 > 0.7853981633974475)) {
				writeScummVar(108, -1);
			} else {
				writeScummVar(108, (int)(v15 / 0.01745329251994328 * 100.0));
				result = 1;
			}
		} else {
			writeScummVar(108, (int)(v14 / 0.01745329251994328 * 100.0));
			result = 1;
		}
	} else {
		writeScummVar(108, -1);
	}

	return result;
}

int LogicHEsoccer::op_1017(int32 *args) {
	// Used sporadically during a match (out of bounds?)
	if (!args[1])
		args[1] = 1;

	double v3 = asin((double)args[0] / (double)args[1]);
	writeScummVar(108, (int32)(v3 / 0.01745329251994328 * (double)args[1]));

	return 1;
}

int LogicHEsoccer::op_1019(int32 *args) {
	// Used at the beginning of a match
	// Initializes some arrays with field collision data

	// These two arrays are used in op_1014 and op_1015
	for (int i = 0; i < 4096; i++)
		_byteArray1[i] = getFromArray(args[1], 0, i);

	for (int i = 0; i < 585; i++)
		_byteArray2[i] = getFromArray(args[0], 0, i);

	// The remaining code of this function was used for the
	// built-in editor. However, it is incomplete in the
	// final product, so we do not need to have it.

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
