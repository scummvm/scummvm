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

#ifndef __dgGoogol__
#define __dgGoogol__


#include "dgStdafx.h"
#include "dgMemory.h"
#include "dgArray.h"
#include "dgVector.h"



//#define DG_GOOGOL_SIZE    16
#define DG_GOOGOL_SIZE      4

class dgGoogol {
public:
	dgGoogol(void);
	dgGoogol(dgFloat64 value);
	~dgGoogol(void);

	dgFloat64 GetAproximateValue() const;
	void InitFloatFloat(dgFloat64 value);

	dgGoogol operator+ (const dgGoogol &A) const;
	dgGoogol operator- (const dgGoogol &A) const;
	dgGoogol operator* (const dgGoogol &A) const;
	dgGoogol operator/ (const dgGoogol &A) const;

	dgGoogol operator+= (const dgGoogol &A);
	dgGoogol operator-= (const dgGoogol &A);

	dgGoogol Floor() const;

#ifdef _DEBUG
	void Trace() const;
	void ToString(char *const string) const;
#endif

private:
	void NegateMantissa(dgUnsigned64 *const mantissa) const;
	void CopySignedMantissa(dgUnsigned64 *const mantissa) const;
	dgInt32 NormalizeMantissa(dgUnsigned64 *const mantissa) const;
	dgUnsigned64 CheckCarrier(dgUnsigned64 a, dgUnsigned64 b) const;
	void ShiftRightMantissa(dgUnsigned64 *const mantissa, dgInt32 bits) const;

	dgInt32 LeadinZeros(dgUnsigned64 a) const;
	void ExtendeMultiply(dgUnsigned64 a, dgUnsigned64 b, dgUnsigned64 &high, dgUnsigned64 &low) const;
	void ScaleMantissa(dgUnsigned64 *const out, dgUnsigned64 scale) const;

	dgInt8 m_sign;
	dgInt16 m_exponent;
	dgUnsigned64 m_mantissa[DG_GOOGOL_SIZE];
};


class dgHugeVector: public dgTemplateVector<dgGoogol> {
public:
	dgHugeVector()
		: dgTemplateVector<dgGoogol>() {
	}

	dgHugeVector(const dgBigVector &a)
		: dgTemplateVector<dgGoogol>(dgGoogol(a.m_x), dgGoogol(a.m_y), dgGoogol(a.m_z), dgGoogol(a.m_w)) {
	}

	dgHugeVector(const dgTemplateVector<dgGoogol> &a)
		: dgTemplateVector<dgGoogol>(a) {
	}

	dgHugeVector(dgFloat64 x, dgFloat64 y, dgFloat64 z, dgFloat64 w)
		: dgTemplateVector<dgGoogol>(x, y, z, w) {
	}

	dgGoogol EvaluePlane(const dgHugeVector &point) const {
		return (point % (*this)) + m_w;
	}




#ifdef _DEBUG
	void Trace() const {
		m_x.Trace();
		m_y.Trace();
		m_z.Trace();
		m_w.Trace();
		dgTrace(("\n"));
	}
#endif
};


#endif
