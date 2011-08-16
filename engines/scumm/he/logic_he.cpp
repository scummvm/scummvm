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
	_collisionTree = 0;
	_collisionTreeAllocated = false;
}

LogicHEsoccer::~LogicHEsoccer() {
	free(_userDataD);
	delete[] _collisionTree;
}

int32 LogicHEsoccer::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
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

	case 1001: case 1002: case 1003: case 1005:
	case 1009: case 8221968:
		// In the u32, but unused by any of the soccer scripts
		// 1005 is called from another opcode, however
		error("Unused soccer u32 opcode %d called", op);

	case 1004: case 1010: case 1015: case 1018:
	case 1020:
		// Used only by the unaccessible in-game editor (so, fall through)

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
	_collisionTreeAllocated = false;
	_userDataD[530] = 0;
}

int LogicHEsoccer::startOfFrame() {
	// This variable is some sort of flag that activates this mode
	int res = (int)_userDataD[530];

	if (res)
		res = op_1011((int)_userDataD[531], (int)_userDataD[532], (int)_userDataD[533], (int)_userDataD[534], (int)_userDataD[535], (int)_userDataD[536]);

	return res;
}

int LogicHEsoccer::op_1005(float x1, float y1, float z1, float x2, float y2, float z2, float *nextVelX, float *nextVelY, float *nextVelZ, float *a10) {
	// Called from op_1014

	double dot = x1 * x2 + y1 * y2 + z1 * z2;
	*nextVelX = x2 - 2 * dot * x1;
	*nextVelY = y2 - 2 * dot * y1;
	*nextVelZ = z2 - 2 * dot * z1;
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
	memset(_collisionObjIds, 0, 4096);
	memset(_collisionNodeEnabled, 0, 585);

	if (!_collisionTreeAllocated)
		op_1013(4, args[8], args[9]);

	return 1;
}

static inline double vectorLength(double x, double y, double z) {
	return sqrt(x * x + y * y + z * z);
}

int LogicHEsoccer::op_1008(int outArray, int srcX, int srcY, int srcZ, int vecX, int vecY, int vecZ, int airResX, int airResY, int airResZ, int vecNumerator, int vecDenom, int gravityMult, int requiredSegments, int a15, int a16, int a17, int a18, int fieldType) {
	// Calculate requiredSegments consecutive movement segments, and place
	// the associated data (positions, vectors, etc) into outArray.

	int loopsSoFar = 0;
	int segmentsSoFar = 1;
	int prevVecY = 500;
	int inX = srcX;
	int inZ = srcZ;
	int checkForCollisions = 0;

	while (segmentsSoFar <= requiredSegments) {
		if (fieldType == 1 && srcZ > 8819)
			checkForCollisions = 1;
		else if (fieldType == 2 && (srcX < -2350 || srcX > 2350))
			checkForCollisions = 1;
		else if (fieldType == 3 && (srcX < -2350 || srcX > 2350 || srcZ < 6119 || srcZ > 8819))
			checkForCollisions = 1;

		if (srcY > 0)
			vecY -= vecNumerator * gravityMult / vecDenom;

		int prevX = srcX;
		int prevY = srcY;
		int prevZ = srcZ;
		srcX += vecNumerator * vecX / vecDenom;
		srcY += vecNumerator * vecY / vecDenom;
		srcZ += vecNumerator * vecZ / vecDenom;

		if (srcY > 0) {
			if (checkForCollisions && op_1014(prevX, prevY, prevZ, vecX, vecY, vecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
				srcX = _internalCollisionOutData[6];
				srcY = _internalCollisionOutData[7];
				srcZ = _internalCollisionOutData[8];
				vecX = _internalCollisionOutData[3];
				vecY = _internalCollisionOutData[4];
				vecZ = _internalCollisionOutData[5];
				putInArray(outArray, segmentsSoFar, 0, loopsSoFar);
				putInArray(outArray, segmentsSoFar, 1, (int)vectorLength((double)(_internalCollisionOutData[6] - inX), 0.0, (double)(_internalCollisionOutData[8] - inZ)));
				putInArray(outArray, segmentsSoFar, 2, _internalCollisionOutData[6]);
				putInArray(outArray, segmentsSoFar, 3, _internalCollisionOutData[7]);
				putInArray(outArray, segmentsSoFar, 4, _internalCollisionOutData[8]);
				putInArray(outArray, segmentsSoFar, 5, vecX);
				putInArray(outArray, segmentsSoFar, 6, vecY);
				putInArray(outArray, segmentsSoFar++, 7, vecZ);
			} 
		} else {
			srcY = 0;
			int thisVecX = vecX;
			int thisVecZ = vecZ;
			vecX = vecX * airResX / 100;

			if (vecY) {
				int v18 = ABS(vecY);
				if (v18 > ABS(prevVecY))
					vecY = ABS(prevVecY);
				vecY = ABS(airResY * vecY) / 100;
			}

			vecZ = airResZ * vecZ / 100;

			if (prevVecY >= 0) {
				if (op_1014(prevX, prevY, prevZ, thisVecX, prevVecY, thisVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
					srcX = _internalCollisionOutData[6];
					srcY = _internalCollisionOutData[7];
					srcZ = _internalCollisionOutData[8];
					vecX = _internalCollisionOutData[3];
					vecY = _internalCollisionOutData[4];
					vecZ = _internalCollisionOutData[5];
				}
			} else {
				if (checkForCollisions) {
					op_1021(srcX, 0, srcZ, thisVecX, prevVecY, thisVecZ, 1);

					if (op_1014(prevX, prevY, prevZ, thisVecX, prevVecY, thisVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
						srcX = _internalCollisionOutData[6];
						srcY = _internalCollisionOutData[7];
						srcZ = _internalCollisionOutData[8];
						vecX = _internalCollisionOutData[3];
						vecY = _internalCollisionOutData[4];
						vecZ = _internalCollisionOutData[5];
					} else {
						// try it with the output of op_1021 instead
						int tmpVecZ = vecZ + prevZ - _var1021[1];
						int v20 = ABS(prevVecY);

						if (op_1014(_var1021[0], 0, _var1021[1], vecX + prevX - _var1021[0], v20 - prevY, tmpVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
							srcX = _internalCollisionOutData[6];
							srcY = _internalCollisionOutData[7];
							srcZ = _internalCollisionOutData[8];
							vecX = _internalCollisionOutData[3];
							vecY = _internalCollisionOutData[4];
							vecZ = _internalCollisionOutData[5];
						}
					}
				}
			}

			prevVecY = vecY;
			putInArray(outArray, segmentsSoFar, 0, loopsSoFar);
			putInArray(outArray, segmentsSoFar, 1, (int32)vectorLength(srcX - inX, 0.0, srcZ - inZ));
			putInArray(outArray, segmentsSoFar, 2, srcX);
			putInArray(outArray, segmentsSoFar, 3, srcY);
			putInArray(outArray, segmentsSoFar, 4, srcZ);
			putInArray(outArray, segmentsSoFar, 5, vecX);
			putInArray(outArray, segmentsSoFar, 6, vecY);
			putInArray(outArray, segmentsSoFar++, 7, vecZ);
		}

		loopsSoFar++;
	}

	return 1;
}

int LogicHEsoccer::op_1011(int32 worldPosArray, int32 screenPosArray, int32 a3, int32 closestActorArray, int32 maxDistance, int32 fieldAreaArray) {
	// This is called on each frame by startOfFrame() if activated by op_1012.

	float objY = 0.0;

	// First, iterate over the field objects and project them onto the screen.
	for (int i = 0; i < 18; i++) {
		int rawX = getFromArray(worldPosArray, i, 0);
		int rawY = getFromArray(worldPosArray, i, 1);
		int rawZ = getFromArray(worldPosArray, i, 2);

		float objX = (double)rawX / 100.0;
		objY = (double)rawY / 100.0;
		float objZ = (double)rawZ / 100.0;

		if (i < 13) {
			// For the players and the ball: work out the area of the field
			// this object is in, storing it in an array if provided.
			int areaX = (rawX + 2750) / 500;
			areaX = CLIP(areaX, 0, 10);

			int areaZ = (9219 - rawZ) / 500;
			areaZ = CLIP(areaZ, 0, 6);

			if (fieldAreaArray)
				putInArray(fieldAreaArray, 0, i, areaX + 11 * areaZ);
		}

		float v7 = atan2(_userDataD[524] - objY, (double)objZ);
		int screenY = (int)(_userDataD[526] - (_userDataD[521] - v7) * _userDataD[522] - 300.0);
		double v9 = _userDataD[523];

		// x/y position of objects
		putInArray(screenPosArray, i, 0, (int32)(atan2(objX, objZ) * v9 + 640.0));
		putInArray(screenPosArray, i, 1, screenY);

		double v10 = atan2(_userDataD[524], (double)objZ);
		int shadowScreenY = (int)(_userDataD[526] - (_userDataD[521] - (float)v10) * _userDataD[522] - 300.0);
		double v13 = _userDataD[523];

		// x/y position of shadows
		putInArray(screenPosArray, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 0, (int32)(atan2(objX, objZ) * v13 + 640.0));
		putInArray(screenPosArray, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 1, shadowScreenY);
	}

	// soccer only uses one array here
	// soccermls/soccer2004 use four
	int start = (_vm->_game.id == GID_SOCCER) ? 19 : 18;
	int end = (_vm->_game.id == GID_SOCCER) ? 19 : 21;

	// The following loop is doing cursor scaling
	// The further up on the screen, the smaller the cursor is
	for (int i = start; i <= end; i++) {
		int x = getFromArray(screenPosArray, i, 0);
		int y = getFromArray(screenPosArray, i, 1);

		// This retains objY from (i == 17)?
		float v16 = _userDataD[524] - objY;
		float scaledZ = v16 / tan((_userDataD[528] + y - _userDataD[526]) / _userDataD[522] + _userDataD[521]);
		double scaledX = tan((double)(x - ((_vm->_game.id == GID_SOCCER) ? 0 : 640)) / _userDataD[523]) * scaledZ;
		putInArray(worldPosArray, i, 0, (int)(scaledX * 100.0));
		putInArray(worldPosArray, i, 2, (int)(scaledZ * 100.0));
	}

	calculateDistances(worldPosArray, a3, closestActorArray, maxDistance);

	return 1;
}

static inline int distance(int a1, int a2, int a3, int a4) {
	return (int)sqrt((double)((a4 - a3) * (a4 - a3) + (a2 - a1) * (a2 - a1)));
}

void LogicHEsoccer::calculateDistances(int32 worldPosArray, int32 a2, int32 closestActorArray, int32 maxDistance) {
	// As you can guess, this is called from op_1011
	// This seems to be checking distances between the players and the ball
	// and which distance is the shortest.

	int closestActor[13];
	int objectX[13];
	int objectZ[13];
	int closestDistance[195];

	for (int i = 0; i < 13; i++) {
		closestActor[i] = 0;
		objectX[i] = getFromArray(worldPosArray, i, 0);
		objectZ[i] = getFromArray(worldPosArray, i, 2);
	}

	// 12 here, 13 up there
	// Probably 12 for players, 13 for players+ball
	for (int i = 0; i < 12; i++) {
		int bestDistance = maxDistance;
		for (int j = i + 1; j < 13; j++) {
			closestDistance[i * 15 + j] = distance(objectX[i], objectX[j], objectZ[i], objectZ[j]);
			putInArray(a2, i, j, closestDistance[i * 15 + j]);
			putInArray(a2, j, i, closestDistance[i * 15 + j]);
			if (closestDistance[i * 15 + j] < bestDistance) {
				bestDistance = closestDistance[i * 15 + j];
				closestActor[i] = j + 1;
				closestActor[j] = i + 1;
			}
		}
	}

	int v13 = getFromArray(worldPosArray, 18, 0);
	int v14 = getFromArray(worldPosArray, 18, 2);
	int v15 = getFromArray(worldPosArray, 19, 0);
	int v16 = getFromArray(worldPosArray, 19, 2);
	int v19[15];
	int v20[15];

	if (_vm->_game.id == GID_SOCCER) {
		// soccer gets to be different
		for (int i = 0; i < 13; i++)
			v20[i] = distance(v15, objectX[i], v16, objectZ[i]);

		for (int i = 0; i < 13; i++)
			v19[i] = distance(v13, objectX[i], v14, objectZ[i]);
	} else {
		// soccermls and soccer2004 use two other arrays here
		int v9 = getFromArray(worldPosArray, 20, 0);
		int v10 = getFromArray(worldPosArray, 20, 2);
		int v11 = getFromArray(worldPosArray, 21, 0);
		int v12 = getFromArray(worldPosArray, 21, 2);

		for (int i = 0; i < 6; i++) {
			v20[i] = distance(v9, objectX[i], v10, objectZ[i]);
			v19[i] = distance(v13, objectX[i], v14, objectZ[i]);
		}

		for (int i = 6; i < 13; i++) {
			v20[i] = distance(v11, objectX[i], v12, objectZ[i]);
			v19[i] = distance(v15, objectX[i], v16, objectZ[i]);
		}
	}

	for (int i = 0; i < 13; i++) {
		putInArray(a2, 14, i, v20[i]);
		putInArray(a2, i, 14, v20[i]);
		putInArray(a2, 13, i, v19[i]);
		putInArray(a2, i, 13, v19[i]);
		putInArray(closestActorArray, 0, i, closestActor[i]);
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

int LogicHEsoccer::addCollisionTreeChild(int depth, int index, int parent) {
	uint32 *dataPtr = _collisionTree + 11 * index;

	/*
	 * This sets up a node of the tree stored in _collisionTree. There are
	 * two sets of parents (at depth 1 and 2), then child nodes at depth
	 * 3 which represent a single collision object.
	 *
	 * 0 = this index, 1 = parent index,
	 * 2-9 = child indices (or all -1 if leaf),
	 * 10 = _collisionObjIds index (if leaf)
	 */
	dataPtr[0] = index;
	dataPtr[1] = parent;

	if (depth > 2) {
		// store the offset into _collisionObjIds (which holds collision object ids),
		// but subtract 585 first because there are already (8 + 8*8 + 8*8*8 = 584)
		// indexes at higher levels of the tree, and we want to start at 0
		dataPtr[10] = 8 * index - 585;
		for (int i = 0; i < 8; i++)
			dataPtr[i + 2] = 0xffffffff;
	} else {
		for (int i = 0; i < 8; i++)
			dataPtr[i + 2] = addCollisionTreeChild(depth + 1, i + 8 * index + 1, index);
	}

	return index;
}

int LogicHEsoccer::op_1013(int32 a1, int32 a2, int32 a3) {
	// Initialises _collisionTree, a tree used for collision detection.
	// It is used by op_1014 to work out which objects to check.

	_collisionTree = new uint32[585 * 11];
	_collisionTreeAllocated = true;
	for (int i = 0; i < 585 * 11; i++)
		_collisionTree[i] = 0;

	for (int i = 0; i < 8; i++)
		_collisionTree[i + 2] = addCollisionTreeChild(1, i + 1, 0);

	return 1;
}

int LogicHEsoccer::op_1014(int32 srcX, int32 srcY, int32 srcZ, int32 velX, int32 velY, int32 velZ, int32 outArray, int32 dataArrayId, int32 indexArrayId, int32 requestType, int32 vecNumerator, int32 vecDenom, int32 a13, int32 a14) {
	// Used many times during a match
	// And called from op_1008!
	// This seems to be doing collision handling

	double startX = (double)srcX;
	double startY = (double)srcY;
	double startZ = (double)srcZ;
	double adjustedVelZ = 0.0, adjustedVelY = 0.0, adjustedVelX = 0.0;

	writeScummVar(108, 0);
	writeScummVar(109, 0);

	switch (requestType) {
	case 1:
	case 3:
		adjustedVelX = (double)velX * (double)vecNumerator / (double)vecDenom / 100.0;
		adjustedVelY = (double)velY * (double)vecNumerator / (double)vecDenom / 100.0;
		adjustedVelZ = (double)velZ * (double)vecNumerator / (double)vecDenom / 100.0;
		break;
	case 2:
		// length of movement vector
		double v15 = vectorLength((double)velX * (double)vecNumerator / (double)vecDenom, (double)velY * (double)vecNumerator / (double)vecDenom, (double)velZ * (double)vecNumerator / (double)vecDenom);

		if (v15 != 0.0) {
			// add the (scaled) movement vector to the input
			double v26 = (double)ABS(velX) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcX = (int)((double)srcX + v26);
			double v25 = (double)ABS(velY) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcY = (int)((double)srcY + v25);
			double v24 = (double)ABS(velZ) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcZ = (int)((double)srcZ + v24);
		}

		// srcX = (newX / newZ) * 3869
		startX = (double)srcX / (double)srcZ * 3869.0;
		// srcY = (newY - (+524 * 100)) / (newZ * 3869 + (+524 * 100)
		startY = ((double)srcY - _userDataD[524] * 100.0) / (double)srcZ * 3869.0 + _userDataD[524] * 100.0;
		// srcZ = 3869
		startZ = 3869.0;
		// vectorX = (newX - srcX) / 100
		adjustedVelX = ((double)srcX - startX) / 100.0;
		// vectorY = (newY - srcY) / 100
		adjustedVelY = ((double)srcY - startY) / 100.0;
		// vectorZ = (newZ - 3869 = srcZ) / 100
		adjustedVelZ = ((double)srcZ - 3869.0) / 100.0;
		break;
	}
	
	int foundCollision = 0;

	// work out which collision objects we might collide with (if any)
	if (generateCollisionObjectList(startX, startY, startZ, adjustedVelX, adjustedVelY, adjustedVelZ)) {
		int collisionId = 0;
		float v46; // always 1.0 after a collision due to op_1005

		float collisionInfo[42 * 8];
		memset(collisionInfo, 0, 42 * 8 * sizeof(float));

		// check each potential collision object for an actual collision,
		// add it to collisionInfo if there is one
		for (Common::List<byte>::const_iterator it = _collisionObjs.begin(); it != _collisionObjs.end(); it++) {
			float collideZ, collideY, collideX;
			float nextVelX, nextVelY, nextVelZ;

			if (findCollisionWith(*it, startX, startY, startZ, adjustedVelX * 100.0, adjustedVelY * 100.0, adjustedVelZ * 100.0, collideX, collideY, collideZ, indexArrayId, dataArrayId, &nextVelX, &nextVelY, &nextVelZ, &v46)) {
				collisionInfo[collisionId * 8] = *it;
				collisionInfo[collisionId * 8 + 1] = vectorLength(collideX - startX, collideY - startY, collideZ - startZ);
				collisionInfo[collisionId * 8 + 2] = collideX;
				collisionInfo[collisionId * 8 + 3] = collideY;
				collisionInfo[collisionId * 8 + 4] = collideZ;
				collisionInfo[collisionId * 8 + 5] = vecDenom * nextVelX / vecNumerator;
				collisionInfo[collisionId * 8 + 6] = vecDenom * nextVelY / vecNumerator;
				collisionInfo[collisionId * 8 + 7] = vecDenom * nextVelZ / vecNumerator;
				foundCollision = 1;
				collisionId++;
			}
		}

		if (foundCollision) {
			// if we have more than one collision, sort them by distance
			// to find the closest one
			if (collisionId != 1)
				sortCollisionList(collisionInfo, 42, 8, 1);

			int v22, v39, v42;
			float tmpData[8];
			int outData[10];

			// output the collision we found
			switch (requestType) {
				case 1:
					for (int i = 0; i < 8; i++)
						tmpData[i] = collisionInfo[i];
					v22 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0));
					v42 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 2.0));
					setCollisionOutputData(tmpData, 8, dataArrayId, indexArrayId, (int)startX, (int)startY, (int)startZ, v46, v22, v42, v39, outData);
					for (int i = 0; i < 10; i++)
						putInArray(outArray, 0, i, outData[i]);
					break;
				case 2:
					// write the object id if collision happened (note that other case can't happen)
					if (collisionId)
						writeScummVar(109, (int)collisionInfo[(collisionId - 1) * 8]);
					else
						writeScummVar(109, 0);
					break;
				case 3:
					for (int i = 0; i < 8; i++)
						tmpData[i] = collisionInfo[i];
					v22 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0));
					v42 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 2.0));
					setCollisionOutputData(tmpData, 8, dataArrayId, indexArrayId, (int)startX, (int)startY, (int)startZ, v46, v22, v42, v39, outData);
					for (int i = 0; i < 10; i++)
						_internalCollisionOutData[i] = outData[i];
					break;
			}
		}
	}

	writeScummVar(108, foundCollision);

	_collisionObjs.clear();

	return foundCollision;
}

int LogicHEsoccer::generateCollisionObjectList(float srcX, float srcY, float srcZ, float velX, float velY, float velZ) {
	float v36 = srcX / 100.0;
	float v37 = v36 + 52.0;
	float destX = v37 + velX;

	int v33, v29;

	if (((int)destX / 52) ^ ((int)v37 / 52)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v37 / 52) {
		v29 = 0;
		v33 = 1;
	} else {
		v33 = 0;
		v29 = 1;
	}

	uint32 areaEnabled[8];
	for (int i = 0; i < 4; i++) {
		areaEnabled[i] = v29;
		areaEnabled[i + 4] = v33;
	}

	float v38 = srcY / 100.0;
	float destY = v38 + velY;

	if (((int)destY / 20) ^ ((int)v38 / 20)) {
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
		if (areaEnabled[i * 4 + 0])
			areaEnabled[i * 4 + 0] = v29;
		if (areaEnabled[i * 4 + 1])
			areaEnabled[i * 4 + 1] = v29;
		if (areaEnabled[i * 4 + 2])
			areaEnabled[i * 4 + 2] = v33;
		if (areaEnabled[i * 4 + 3])
			areaEnabled[i * 4 + 3] = v33;
	}

	float v39 = srcZ / 100.0;
	float v40 = v39 - 38.69;
	float destZ = v40 + velZ;

	if (((int)destZ / 36) ^ ((int)v40 / 36)) {
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
		if (areaEnabled[i])
			areaEnabled[i] = v29;
		if (areaEnabled[i + 1])
			areaEnabled[i + 1] = v33;
	}

	int objCount = 0;

	for (int i = 0; i < 8; i++) {
		if (areaEnabled[i]) {
			uint32 *ptr = _collisionTree +  _collisionTree[i + 2] * 11;
			objCount += addFromCollisionTreeNode(ptr[0], ptr[1], &ptr[2], ptr[10]);
		}
	}

	writeScummVar(109, objCount);
	return objCount;
}

int LogicHEsoccer::addFromCollisionTreeNode(int index, int parent, uint32 *indices, int objIndexBase) {
	int objCount = 0;

	if (indices[0] == 0xffffffff) {
		for (int i = 0; i < 8; i++) {
			if (_collisionObjIds[i + objIndexBase]) {
				addCollisionObj(_collisionObjIds[i + objIndexBase]);
				objCount = 1;
			}
		}
	} else {
		if (_collisionNodeEnabled[index]) {
			for (int i = 0; i < 8; i++) {
				uint32 *ptr = _collisionTree + indices[i] * 11;
				objCount += addFromCollisionTreeNode(ptr[0], ptr[1], &ptr[2], ptr[10]);
			}
		}
	}

	return objCount;
}

void LogicHEsoccer::addCollisionObj(byte objId) {
	// Add objId to the list if not found
	for (Common::List<byte>::const_iterator it = _collisionObjs.begin(); it != _collisionObjs.end(); it++)
		if (*it == objId)
			return;

	_collisionObjs.push_back(objId);
}

int LogicHEsoccer::findCollisionWith(int objId, float inX, float inY, float inZ, float inXVec, float inYVec, float inZVec, float &collideX, float &collideY, float &collideZ, int indexArrayId, int dataArrayId, float *nextVelX, float *nextVelY, float *nextVelZ, float *a15) {
	int foundCollision = 0;
	float inY_plus1 = inY + 1.0;
	float destX = inX + inXVec;
	float destY = inY_plus1 + inYVec;
	float destZ = inZ + inZVec;

	// don't go below the ground!
	if (inY_plus1 <= 1.0001 && destY < 0.0) {
		destY = 0.0;
		inYVec = ABS((int)inYVec);
	}

	// get the 8 points which define the 6 faces of this object
	int objIndex = getFromArray(indexArrayId, 0, 4 * objId - 1);
	int objPoints[24];
	for (int i = 0; i < 24; i++)
		objPoints[i] = getFromArray(dataArrayId, 0, objIndex + i);

	for (int faceId = 0; faceId < 6; faceId++) {
		// This assigns variables from objPoints based on faceId
		float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
		float faceCrossX, faceCrossY, faceCrossZ;
		getPointsForFace(faceId, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, objPoints);
		crossProduct(x1, y1, z1, x2, y2, z2, x1, y1, z1, x3, y3, z3, faceCrossX, faceCrossY, faceCrossZ);

		float faceArea = sqrt(faceCrossX * faceCrossX + faceCrossY * faceCrossY + faceCrossZ * faceCrossZ);

		// The original did not initialize these variables and would
		// use them uninitialized if faceArea == 0.0
		float xMult = 0.0, yMult = 0.0, zMult = 0.0;

		if (faceArea != 0.0) {
			// UnitCross = Cross/||Cross||
			xMult = faceCrossX / faceArea;
			yMult = faceCrossY / faceArea;
			zMult = faceCrossZ / faceArea;
		}
		double scalingMult = 5.0;

		float ZToFacePoint1 = z1 - inZ;
		float YToFacePoint1 = y1 - inY_plus1;
		float XToFacePoint1 = x1 - inX;
		// scalar component of UnitCross in direction of (start -> P1)
		double ToFacePoint1 = dotProduct(xMult, yMult, zMult, XToFacePoint1, YToFacePoint1, ZToFacePoint1);

		float ZToDest = destZ - inZ;
		float YToDest = destY - inY_plus1;
		float XToDest = destX - inX;
		// scalar component of UnitCross in direction of (start -> dest)
		double ToDest = dotProduct(xMult, yMult, zMult, XToDest, YToDest, ZToDest);

		if (fabs(ToDest) > 0.00000001)
			scalingMult = ToFacePoint1 / ToDest;

		if (scalingMult >= 0.0 && fabs(scalingMult) <= 1.0 && ToDest != 0.0) {
			// calculate where the collision would be, in the plane containing this face
			double collisionX = inX + (destX - inX) * scalingMult;
			double collisionY = inY_plus1 + (destY - inY_plus1) * scalingMult + 5.0;
			double collisionZ = inZ + (destZ - inZ) * scalingMult;

			// now we need to work out whether this point is actually inside the face
			double dot1 = dotProduct(x2 - x1, y2 - y1, z2 - z1, x3 - x1, y3 - y1, z3 - z1);
			double sqrt1 = vectorLength(x2 - x1, y2 - y1, z2 - z1);
			double num1 = dot1 / (vectorLength(x3 - x1, y3 - y1, z3 - z1) * sqrt1);
			num1 = CLIP(num1, -1.0, 1.0);
			double faceAngle = acos(num1);

			double dot2 = dotProduct(x2 - x1, y2 - y1, z2 - z1, collisionX - x1, collisionY - y1, collisionZ - z1);
			double sqrt2 = vectorLength(x2 - x1, y2 - y1, z2 - z1);
			double num2 = dot2 / (vectorLength(collisionX - x1, collisionY - y1, collisionZ - z1) * sqrt2);
			num2 = CLIP(num2, -1.0, 1.0);
			double angle1 = acos(num2);

			double dot3 = dotProduct(x3 - x1, y3 - y1, z3 - z1, collisionX - x1, collisionY - y1, collisionZ - z1);
			double sqrt3 = vectorLength(x3 - x1, y3 - y1, z3 - z1);
			double num3 = dot3 / (vectorLength(collisionX - x1, collisionY - y1, collisionZ - z1) * sqrt3);
			num3 = CLIP(num3, -1.0, 1.0);
			double angle2 = acos(num3);

			if (angle1 + angle2 - 0.001 <= faceAngle) {
				double dot4 = dotProduct(x2 - x4, y2 - y4, z2 - z4, x3 - x4, y3 - y4, z3 - z4);
				double sqrt4 = vectorLength(x2 - x4, y2 - y4, z2 - z4);
				double num4 = dot4 / (vectorLength(x3 - x4, y3 - y4, z3 - z4) * sqrt4);
				num4 = CLIP(num4, -1.0, 1.0);
				faceAngle = acos(num4);

				double dot5 = dotProduct(x2 - x4, y2 - y4, z2 - z4, collisionX - x4, collisionY - y4, collisionZ - z4);
				double sqrt5 = vectorLength(x2 - x4, y2 - y4, z2 - z4);
				double num5 = dot5 / (vectorLength(collisionX - x4, collisionY - y4, collisionZ - z4) * sqrt5);
				num5 = CLIP(num5, -1.0, 1.0);
				double angle3 = acos(num5);

				double dot6 = dotProduct(x3 - x4, y3 - y4, z3 - z4, collisionX - x4, collisionY - y4, collisionZ - z4);
				double sqrt6 = vectorLength(x3 - x4, y3 - y4, z3 - z4);
				double num6 = dot6 / (vectorLength(collisionX - x4, collisionY - y4, collisionZ - z4) * sqrt6);
				num6 = CLIP(num6, -1.0, 1.0);
				double angle4 = acos(num6);

				if (angle3 + angle4 - 0.001 <= faceAngle) {
					// found a collision with this face
					if (foundCollision) {
						// if we already found one, is the new one closer?
						// (except this don't adjust for the modification of collideX/Y/Z..)
						double ToCollide = vectorLength(inX - collisionX, inY_plus1 - collisionY, inZ - collisionZ);
						if (vectorLength(inX - collideX, inY_plus1 - collideY, inZ - collideZ) > ToCollide) {
							collideX = collisionX - xMult * 3.0;
							collideY = collisionY - yMult * 3.0;
							collideZ = collisionZ - zMult * 3.0;
							op_1005(xMult, yMult, zMult, inXVec, inYVec, inZVec, nextVelX, nextVelY, nextVelZ, a15);
						}
					} else {
						collideX = collisionX - xMult * 3.0;
						collideY = collisionY - yMult * 3.0;
						collideZ = collisionZ - zMult * 3.0;
						op_1005(xMult, yMult, zMult, inXVec, inYVec, inZVec, nextVelX, nextVelY, nextVelZ, a15);
					}

					foundCollision = 1;
				}
			}
		}
	}

	return foundCollision;
}

void LogicHEsoccer::getPointsForFace(int faceId, float &x1, float &y1, float &z1, float &x2, float &y2, float &z2, float &x3, float &y3, float &z3, float &x4, float &y4, float &z4, const int *objPoints) {
	// Note that this originally returned a value, but said value was never used
	// TODO: This can probably be shortened using a few tables...

	switch (faceId) {
	case 0:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[3];
		y2 = objPoints[4];
		z2 = objPoints[5];
		x3 = objPoints[6];
		y3 = objPoints[7];
		z3 = objPoints[8];
		x4 = objPoints[9];
		y4 = objPoints[10];
		z4 = objPoints[11];
		break;
	case 1:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[6];
		y2 = objPoints[7];
		z2 = objPoints[8];
		x3 = objPoints[12];
		y3 = objPoints[13];
		z3 = objPoints[14];
		x4 = objPoints[18];
		y4 = objPoints[19];
		z4 = objPoints[20];
		break;
	case 2:
		x1 = objPoints[3];
		y1 = objPoints[4];
		z1 = objPoints[5];
		x2 = objPoints[15];
		y2 = objPoints[16];
		z2 = objPoints[17];
		x3 = objPoints[9];
		y3 = objPoints[10];
		z3 = objPoints[11];
		x4 = objPoints[21];
		y4 = objPoints[22];
		z4 = objPoints[23];
		break;
	case 3:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[12];
		y2 = objPoints[13];
		z2 = objPoints[14];
		x3 = objPoints[3];
		y3 = objPoints[4];
		z3 = objPoints[5];
		x4 = objPoints[15];
		y4 = objPoints[16];
		z4 = objPoints[17];
		break;
	case 4:
		x1 = objPoints[6];
		y1 = objPoints[7];
		z1 = objPoints[8];
		x2 = objPoints[9];
		y2 = objPoints[10];
		z2 = objPoints[11];
		x3 = objPoints[18];
		y3 = objPoints[19];
		z3 = objPoints[20];
		x4 = objPoints[21];
		y4 = objPoints[22];
		z4 = objPoints[23];
		break;
	case 5:
		x1 = objPoints[15];
		y1 = objPoints[16];
		z1 = objPoints[17];
		x2 = objPoints[12];
		y2 = objPoints[13];
		z2 = objPoints[14];
		x3 = objPoints[21];
		y3 = objPoints[22];
		z3 = objPoints[23];
		x4 = objPoints[18];
		y4 = objPoints[19];
		z4 = objPoints[20];
		break;
	}
}

void LogicHEsoccer::crossProduct(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, float &outX, float &outY, float &outZ) {
	outX = (y2 - y1) * (z4 - z3) - (y4 - y3) * (z2 - z1);
	outY = ((x2 - x1) * (z4 - z3) - (x4 - x3) * (z2 - z1)) * -1.0;
	outZ = (x2 - x1) * (y4 - y3) - (x4 - x3) * (y2 - y1);
}

double LogicHEsoccer::dotProduct(float a1, float a2, float a3, float a4, float a5, float a6) {
	return a1 * a4 + a2 * a5 + a3 * a6;
}

void LogicHEsoccer::sortCollisionList(float *data, int numEntries, int entrySize, int compareOn) {
	// This takes an input array of collisions, and tries to sort it based on the distance
	// (index of compareOn, always 1), copying in groups of entrySize, which is always 8

	bool found = true;
	int entry = 0;

	while (found) {
		found = false;

		// while we still have entries, and there is an obj id set for the next entry
		while (entry <= numEntries - 2 && data[(entry + 1) * 8] != 0.0) {
			// if the current entry has distance 0, or the next entry is closer (distance is less)
			if (data[compareOn + entry * 8] == 0 || data[compareOn + entry * 8] > data[compareOn + (entry + 1) * 8]) {
				found = true;

				// swap all data with the next entry
				for (int i = 0; i < entrySize; i++) {
					float tmp = data[i + entry * 8];
					data[i + entry * 8] = data[i + (entry + 1) * 8];
					data[i + (entry + 1) * 8] = tmp;
				}
			}

			entry++;
		}
	}
}

int LogicHEsoccer::setCollisionOutputData(float *collisionData, int entrySize, int dataArrayId, int indexArrayId, int startX, int startY, int startZ, float a8, int a9, int a10, int a11, int *out) {
	// area-provided data
	out[0] = a9;
	out[1] = a10;
	out[2] = a11;
	// new velocity, slowed by area-provided value
	out[3] = (int)(collisionData[5] * (double)a10 / 100.0);
	out[4] = (int)(collisionData[6] * (double)a10 / 100.0 * a8); // Note: a8 should always be 1
	out[5] = (int)(collisionData[7] * (double)a10 / 100.0);
	// new position
	out[6] = (int)collisionData[2];
	out[7] = (int)collisionData[3];
	out[8] = (int)collisionData[4];
	// collision object id
	out[9] = (int)collisionData[0];
	return out[9];
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

	// _collisionObjIds provides object ids for leaf nodes
	// of the collision tree (_collisionTree).
	for (int i = 0; i < 4096; i++)
		_collisionObjIds[i] = getFromArray(args[1], 0, i);

	// _collisionNodeEnabled enables or disables non-leaf nodes
	// of the collision tree (_collisionTree).
	for (int i = 0; i < 585; i++)
		_collisionNodeEnabled[i] = getFromArray(args[0], 0, i);

	// The remaining code of this function was used for the
	// built-in editor. However, it is incomplete in the
	// final product, so we do not need to have it.

	return 1;
}

int LogicHEsoccer::op_1021(int32 inX, int32 inY, int32 inZ, int32 velX, int32 velY, int32 velZ, int32 internalUse) {
	// Used during a match (ball movement?)
	// Also called from op_1008

	int outX;
	if (velX && velY)
		outX = (int)(((double)inY - (double)velY * (double)inX / (double)velX) * -1.0 * (double)velX / (double)velY);
	else
		outX = inX;

	int outZ;
	if (velZ && velY)
		outZ = (int)(((double)inY - (double)velY * (double)inZ / (double)velZ) * -1.0 * (double)velZ / (double)velY);
	else
		outZ = inZ;

	// The final argument chooses whether to store the results for op_1008 or
	// store them in SCUMM variables.
	if (internalUse) {
		_var1021[0] = outX;
		_var1021[1] = outZ;
	} else {
		writeScummVar(108, outX);
		writeScummVar(109, outZ);
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
