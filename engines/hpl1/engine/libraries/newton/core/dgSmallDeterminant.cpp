/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dgStdafx.h"
#include "dgGoogol.h"
#include "dgSmallDeterminant.h"

#define Absolute(a)  ((a) >= 0.0 ? (a) : -(a))

dgFloat64 Determinant2x2(const dgFloat64 matrix[2][2], dgFloat64 *const error) {
	dgFloat64 a00xa11 = matrix[0][0] * matrix[1][1];
	dgFloat64 a01xa10 = matrix[0][1] * matrix[1][0];
	*error = Absolute(a00xa11) + Absolute(a01xa10);
	return a00xa11 - a01xa10;
}

dgGoogol Determinant2x2(const dgGoogol matrix[2][2]) {
	dgGoogol a00xa11(matrix[0][0] * matrix[1][1]);
	dgGoogol a01xa10(matrix[0][1] * matrix[1][0]);
	return a00xa11 - a01xa10;
}

dgFloat64 Determinant3x3(const dgFloat64 matrix[3][3], dgFloat64 *const error) {
	dgFloat64 sign = dgFloat64(-1.0f);
	dgFloat64 det = dgFloat64(0.0f);
	dgFloat64 accError = dgFloat64(0.0f);
	for (dgInt32 i = 0; i < 3; i++) {
		dgFloat64 cofactor[2][2];
		for (dgInt32 j = 0; j < 2; j++) {
			dgInt32 k0 = 0;
			for (dgInt32 k = 0; k < 3; k++) {
				if (k != i) {
					cofactor[j][k0] = matrix[j][k];
					k0++;
				}
			}
		}

		dgFloat64 parcialError;
		dgFloat64 minorDet = Determinant2x2(cofactor, &parcialError);
		accError += parcialError * Absolute(matrix[2][i]);
		det += sign * minorDet * matrix[2][i];
		sign *= dgFloat64(-1.0f);
	}

	*error = accError;
	return det;
}

dgGoogol Determinant3x3(const dgGoogol matrix[3][3]) {
	dgGoogol negOne(dgFloat64(-1.0f));
	dgGoogol sign(dgFloat64(-1.0f));
	dgGoogol det = dgFloat64(0.0f);
	for (dgInt32 i = 0; i < 3; i++) {
		dgGoogol cofactor[2][2];

		for (dgInt32 j = 0; j < 2; j++) {
			dgInt32 k0 = 0;
			for (dgInt32 k = 0; k < 3; k++) {
				if (k != i) {
					cofactor[j][k0] = matrix[j][k];
					k0++;
				}
			}
		}

		dgGoogol minorDet(Determinant2x2(cofactor));
		det = det + sign * minorDet * matrix[2][i];
		sign = sign * negOne;
	}
	return det;
}

dgFloat64 Determinant4x4(const dgFloat64 matrix[4][4], dgFloat64 *const error) {
	dgFloat64 sign = dgFloat64(1.0f);
	dgFloat64 det = dgFloat64(0.0f);
	dgFloat64 accError = dgFloat64(0.0f);
	for (dgInt32 i = 0; i < 4; i++) {
		dgFloat64 cofactor[3][3];
		for (dgInt32 j = 0; j < 3; j++) {
			dgInt32 k0 = 0;
			for (dgInt32 k = 0; k < 4; k++) {
				if (k != i) {
					cofactor[j][k0] = matrix[j][k];
					k0++;
				}
			}
		}

		dgFloat64 parcialError;
		dgFloat64 minorDet = Determinant3x3(cofactor, &parcialError);
		accError += parcialError * Absolute(matrix[3][i]);
		det += sign * minorDet * matrix[3][i];
		sign *= dgFloat64(-1.0f);
	}

	*error = accError;
	return det;
}

dgGoogol Determinant4x4(const dgGoogol matrix[4][4]) {
	dgGoogol sign = dgFloat64(1.0f);
	dgGoogol det = dgFloat64(0.0f);
	dgGoogol negOne(dgFloat64(-1.0f));
	dgGoogol accError = dgFloat64(0.0f);
	for (dgInt32 i = 0; i < 4; i++) {
		dgGoogol cofactor[3][3];
		for (dgInt32 j = 0; j < 3; j++) {
			dgInt32 k0 = 0;
			for (dgInt32 k = 0; k < 4; k++) {
				if (k != i) {
					cofactor[j][k0] = matrix[j][k];
					k0++;
				}
			}
		}

		dgGoogol minorDet = Determinant3x3(cofactor);
		det = det + sign * minorDet * matrix[3][i];
		sign = sign * negOne;
	}
	return det;
}

