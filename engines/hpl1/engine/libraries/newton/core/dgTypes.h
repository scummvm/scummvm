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

#ifndef AFX_DGTYPES__42YH_HY78GT_YHJ63Y__INCLUDED_
#define AFX_DGTYPES__42YH_HY78GT_YHJ63Y__INCLUDED_

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include <ctype.h>
#include <math.h>
#include <new>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4725) // instruction may be inaccurate on some Pentium
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4820) //'4' bytes padding added after data member '_finddata32i64_t::name'
#pragma warning(disable : 4514) //'exp_2' : unreferenced inline function has been removed
#pragma warning(disable : 4987) // nonstandard extension used: 'throw (...)'
#pragma warning(disable : 4710) //'std::_Exception_ptr std::_Exception_ptr::_Current_exception(void)' : function not inlined
#pragma warning(disable : 4826) // Conversion from 'void *' to 'dgUnsigned64' is sign-extended. This may cause unexpected runtime behavior.
#pragma warning(disable : 4061) // enumerator 'm_convexConvexIntance' in switch of enum 'dgCollisionID' is not explicitly handled by a case label
#pragma warning(disable : 4191) //'type cast' : unsafe conversion from 'NewtonWorldRayFilterCallback' to 'OnRayCastAction'
#pragma warning(disable : 4711) // function 'float const & __thiscall dgTemplateVector<float>::operator[](int)const ' selected for automatic inline expansion
#pragma warning(disable : 4530) // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc

#if (_MSC_VER >= 1400)
#pragma warning(disable : 4996) // '_controlfp' was declared deprecated
#else
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4514) // function '$E1' selected for automatic inline expansion
#endif

#ifdef _DEBUG
#pragma warning(disable : 4127) // conditional expression is constant
#endif
#endif

#ifdef _MSC_VER
#pragma warning(push, 3)
#endif

//#include <crtdbg.h>

//	#include <mmsystem.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <direct.h>
#include <float.h>
#include <io.h>
#include <malloc.h>
#include <process.h>

#else
#include <assert.h>

#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#include <sys/sysctl.h>
#endif
#endif

#ifdef DG_BUILD_SIMD_CODE

#ifdef _WIN32
#if (_MSC_VER >= 1400) || defined(__MINGW32__)
#include <intrin.h>
#else
#if (_MSC_VER >= 1300)
#include <xmmintrin.h>
#endif
#endif
#else
#include <xmmintrin.h>
#endif

#ifdef __ppc__
#include <vecLib/veclib.h>
#endif

#endif

//************************************************************
#ifndef _WIN32
#ifdef _DEBUG
#define _ASSERTE(x) assert(x)
#else
#define _ASSERTE(x)
#endif
#endif

#define __USE_CPU_FOUND__

#define DG_MAXIMUN_THREADS 8

#ifdef _DEBUG
//	#define __ENABLE_SANITY_CHECK
#endif

#ifdef DLL_DECLSPEC
#undef DLL_DECLSPEC
#endif

#define DG_INLINE FORCEINLINE

#ifdef _MSC_VER
#define DG_MSC_VECTOR_ALIGMENT __declspec(align(16))
#define DG_GCC_VECTOR_ALIGMENT
#else
#define DG_MSC_VECTOR_ALIGMENT
#define DG_GCC_VECTOR_ALIGMENT __attribute__((aligned(16)))
#endif

typedef int8 dgInt8;
typedef uint8 dgUnsigned8;

typedef int16 dgInt16;
typedef uint16 dgUnsigned16;

typedef int32 dgInt32;
typedef uint32 dgUnsigned32;

typedef int64 dgInt64;
typedef uint64 dgUnsigned64;

typedef double dgFloat64;

#ifdef __USE_DOUBLE_PRECISION__
typedef double dgFloat32;
#else
typedef float dgFloat32;
#endif

class dgTriplex {
public:
	dgFloat32 m_x;
	dgFloat32 m_y;
	dgFloat32 m_z;
};

#define dgPI dgFloat32(3.14159f)
#define dgPI2 dgFloat32(dgPI * 2.0f)
#define dgEXP dgFloat32(2.71828f)
#define dgEPSILON dgFloat32(1.0e-5f)
#define dgGRAVITY dgFloat32(9.8f)
#define dgDEG2RAD dgFloat32(dgPI / 180.0f)
#define dgRAD2DEG dgFloat32(180.0f / dgPI)
#define dgKMH2MPSEC dgFloat32(0.278f)

class dgVector;
class dgBigVector;

#ifdef _WIN32
#define dgApi __cdecl

#ifdef _WIN64
#define dgNaked
#else
#define dgNaked __declspec(naked)
#endif
#else
#define dgApi
#define dgNaked
#endif

/*
#ifdef _WIN32
    #ifdef _DEBUG
        #define dgCheckFloat(x) _finite(x)
    #else
        #define dgCheckFloat(x) true
    #endif
#else
    #define dgCheckFloat(x) true
#endif
*/

#ifdef _DEBUG
#ifdef _WIN32
#define dgCheckFloat(x) (_finite(x) && !_isnan(x))
#else
#define dgCheckFloat(x) (isfinite(x) && !isnan(x))
#endif
#endif

#define NEWTON_ASSERT(x)

DG_INLINE dgInt32 exp_2(dgInt32 x) {
	dgInt32 exp;

	for (exp = -1; x; x >>= 1) {
		exp++;
	}

	return exp;
}

template<class T>
DG_INLINE T ClampValue(T val, T min, T max) {
	if (val < min) {
		return min;
	}

	if (val > max) {
		return max;
	}

	return val;
}

template<class T>
DG_INLINE T GetMin(T A, T B) {
	if (B < A) {
		A = B;
	}

	return A;
}

template<class T>
DG_INLINE T GetMax(T A, T B) {
	if (B > A) {
		A = B;
	}

	return A;
}

template<class T>
DG_INLINE T GetMin(T A, T B, T C) {
	return GetMin(GetMin(A, B), C);
}

template<class T>
DG_INLINE T GetMax(T A, T B, T C) {
	return GetMax(GetMax(A, B), C);
}

template<class T>
DG_INLINE void Swap(T &A, T &B) {
	T tmp(A);
	A = B;
	B = tmp;
}

template<class T>
DG_INLINE T GetSign(T A) {
	T sign(1.0f);

	if (A < T(0.0f)) {
		sign = T(-1.0f);
	}

	return sign;
}

template<class T>
dgInt32 dgBinarySearch(T const *array, dgInt32 elements, dgInt32 entry) {
	dgInt32 index0;
	dgInt32 index1;
	dgInt32 index2;
	dgInt32 entry1;

	index0 = 0;
	index2 = elements - 1;

	while ((index2 - index0) > 1) {
		index1 = (index0 + index2) >> 1;
		entry1 = array[index1].m_Key;

		if (entry1 == entry) {
			NEWTON_ASSERT(array[index1].m_Key <= entry);
			NEWTON_ASSERT(array[index1 + 1].m_Key >= entry);

			return index1;
		} else if (entry < entry1) {
			index2 = index1;
		} else {
			index0 = index1;
		}
	}

	if (array[index0].m_Key > index0) {
		index0--;
	}

	NEWTON_ASSERT(array[index0].m_Key <= entry);
	NEWTON_ASSERT(array[index0 + 1].m_Key >= entry);

	return index0;
}

template<class T>
void dgRadixSort(T *const array,
                 T *const tmpArray,
                 dgInt32 elements,
                 dgInt32 radixPass,
                 dgInt32(*getRadixKey)(const T *const A, void *const context),
                 void *const context = NULL) {
	dgInt32 scanCount[256];
	dgInt32 histogram[256][4];

	NEWTON_ASSERT(radixPass >= 1);
	NEWTON_ASSERT(radixPass <= 4);

	memset(histogram, 0, sizeof(histogram));

	for (dgInt32 i = 0; i < elements; i++) {
		dgInt32 key = getRadixKey(&array[i], context);

		for (dgInt32 j = 0; j < radixPass; j++) {
			dgInt32 radix = (key >> (j << 3)) & 0xff;
			histogram[radix][j] = histogram[radix][j] + 1;
		}
	}

	for (dgInt32 radix = 0; radix < radixPass; radix += 2) {
		scanCount[0] = 0;

		for (dgInt32 i = 1; i < 256; i++) {
			scanCount[i] = scanCount[i - 1] + histogram[i - 1][radix];
		}

		dgInt32 radixShift = radix << 3;

		for (dgInt32 i = 0; i < elements; i++) {
			dgInt32 key = (getRadixKey(&array[i], context) >> radixShift) & 0xff;
			dgInt32 index = scanCount[key];
			tmpArray[index] = array[i];
			scanCount[key] = index + 1;
		}

		if ((radix + 1) < radixPass) {
			scanCount[0] = 0;

			for (dgInt32 i = 1; i < 256; i++) {
				scanCount[i] = scanCount[i - 1] + histogram[i - 1][radix + 1];
			}

			dgInt32 radixShift2 = (radix + 1) << 3;

			for (dgInt32 i = 0; i < elements; i++) {
				dgInt32 key = (getRadixKey(&array[i], context) >> radixShift2) & 0xff;
				dgInt32 index = scanCount[key];
				array[index] = tmpArray[i];
				scanCount[key] = index + 1;
			}
		} else {
			memcpy(array, tmpArray, elements * sizeof(T));
		}
	}

#ifdef _DEBUG
	for (dgInt32 i = 0; i < (elements - 1); i++) {
		NEWTON_ASSERT(getRadixKey(&array[i], context) <= getRadixKey(&array[i + 1], context));
	}
#endif
}

template<class T>
void dgSort(T *const array,
            dgInt32 elements,
            dgInt32(*compare)(const T *const A, const T *const B, void *const context),
            void *const context = NULL) {
	dgInt32 stride = 8;
	dgInt32 stack[1024][2];

	stack[0][0] = 0;
	stack[0][1] = elements - 1;
	dgInt32 stackIndex = 1;

	while (stackIndex) {
		stackIndex--;
		dgInt32 lo = stack[stackIndex][0];
		dgInt32 hi = stack[stackIndex][1];

		if ((hi - lo) > stride) {
			dgInt32 i = lo;
			dgInt32 j = hi;
			T val(array[(lo + hi) >> 1]);

			do {
				while (compare(&array[i], &val, context) < 0)
					i++;
				while (compare(&array[j], &val, context) > 0)
					j--;

				if (i <= j) {
					T tmp(array[i]);
					array[i] = array[j];
					array[j] = tmp;
					i++;
					j--;
				}
			} while (i <= j);

			if (i < hi) {
				stack[stackIndex][0] = i;
				stack[stackIndex][1] = hi;
				stackIndex++;
			}

			if (lo < j) {
				stack[stackIndex][0] = lo;
				stack[stackIndex][1] = j;
				stackIndex++;
			}
			NEWTON_ASSERT(stackIndex < dgInt32(sizeof(stack) / (2 * sizeof(stack[0][0]))));
		}
	}

	stride = stride * 2;

	if (elements < stride) {
		stride = elements;
	}

	for (dgInt32 i = 1; i < stride; i++) {
		if (compare(&array[0], &array[i], context) > 0) {
			T tmp(array[0]);
			array[0] = array[i];
			array[i] = tmp;
		}
	}

	for (dgInt32 i = 1; i < elements; i++) {
		dgInt32 j = i;
		T tmp(array[i]);

		// for (; j && (compare (&array[j - 1], &tmp, context) > 0); j --) {
		for (; compare(&array[j - 1], &tmp, context) > 0; j--) {
			NEWTON_ASSERT(j > 0);
			array[j] = array[j - 1];
		}

		array[j] = tmp;
	}

#ifdef _DEBUG
	for (dgInt32 i = 0; i < (elements - 1); i++) {
		NEWTON_ASSERT(compare(&array[i], &array[i + 1], context) <= 0);
	}
#endif
}

template<class T>
void dgSortIndirect(T **const array,
                    dgInt32 elements,
                    dgInt32(*compare)(const T *const A, const T *const B, void *const context),
                    void *const context = NULL) {
	dgInt32 stride = 8;
	dgInt32 stack[1024][2];

	stack[0][0] = 0;
	stack[0][1] = elements - 1;
	dgInt32 stackIndex = 1;

	while (stackIndex) {
		stackIndex--;
		dgInt32 lo = stack[stackIndex][0];
		dgInt32 hi = stack[stackIndex][1];

		if ((hi - lo) > stride) {
			dgInt32 i = lo;
			dgInt32 j = hi;
			T *val(array[(lo + hi) >> 1]);

			do {
				while (compare(array[i], val, context) < 0)
					i++;
				while (compare(array[j], val, context) > 0)
					j--;

				if (i <= j) {
					T *tmp(array[i]);
					array[i] = array[j];
					array[j] = tmp;
					i++;
					j--;
				}
			} while (i <= j);

			if (i < hi) {
				stack[stackIndex][0] = i;
				stack[stackIndex][1] = hi;
				stackIndex++;
			}

			if (lo < j) {
				stack[stackIndex][0] = lo;
				stack[stackIndex][1] = j;
				stackIndex++;
			}
			NEWTON_ASSERT(stackIndex < dgInt32(sizeof(stack) / (2 * sizeof(stack[0][0]))));
		}
	}

	stride = stride * 2;

	if (elements < stride) {
		stride = elements;
	}

	for (dgInt32 i = 1; i < stride; i++) {
		if (compare(&array[0], &array[i], context) > 0) {
			T tmp(array[0]);
			array[0] = array[i];
			array[i] = tmp;
		}
	}

	for (dgInt32 i = 1; i < elements; i++) {
		dgInt32 j = i;
		T *tmp(array[i]);

		// for (; j && (compare (array[j - 1], tmp, context) > 0); j --) {
		for (; compare(array[j - 1], tmp, context) > 0; j--) {
			NEWTON_ASSERT(j > 0);
			array[j] = array[j - 1];
		}

		array[j] = tmp;
	}

#ifdef _DEBUG
	for (dgInt32 i = 0; i < (elements - 1); i++) {
		NEWTON_ASSERT(compare(array[i], array[i + 1], context) <= 0);
	}
#endif
}

#ifdef __USE_DOUBLE_PRECISION__
union dgFloatSign {
	struct {
		dgInt32 m_dommy;
		dgInt32 m_iVal;

	} m_integer;

	dgFloat64 m_fVal;
};
#else
union dgFloatSign {
	struct s {
		dgInt32 m_iVal;

	} m_integer;

	dgFloat32 m_fVal;
};
#endif

union dgDoubleInt {
	dgInt64 m_int;
	dgFloat64 m_float;
};

void GetMinMax(dgVector &Min,
               dgVector &Max,
               const dgFloat32 *const vArray,
               dgInt32 vCount,
               dgInt32 StrideInBytes);

void GetMinMax(dgBigVector &Min,
               dgBigVector &Max,
               const dgFloat64 *const vArray,
               dgInt32 vCount,
               dgInt32 strideInBytes);

dgInt32 dgVertexListToIndexList(dgFloat32 *const vertexList,
                                dgInt32 strideInBytes,
                                dgInt32 floatSizeInBytes,
                                dgInt32 unsignedSizeInBytes,
                                dgInt32 vertexCount,
                                dgInt32 *const indexListOut,
                                dgFloat32 tolerance = dgEPSILON);

dgInt32 dgVertexListToIndexList(dgFloat64 *const vertexList,
                                dgInt32 strideInBytes,
                                dgInt32 compareCount,
                                dgInt32 vertexCount,
                                dgInt32 *const indexListOut,
                                dgFloat64 tolerance = dgEPSILON);

enum dgCpuClass {
	dgNoSimdPresent = 0,
	dgSimdPresent
	//  dgSimdPresent_3,
};

#define PointerToInt(x) ((size_t)x)
#define IntToPointer(x) ((void *)(size_t(x)))

#ifndef _WIN64
#define dgControlFP(x, y) _controlfp(x, y)
#else
#define dgControlFP(x, y) x
#define stricmp(x, y) strcasecmp(x, y)
#endif

DG_INLINE dgFloat32 dgAbsf(dgFloat32 x) {
#if 0
	dgDoubleInt val;
	val.m_float = x;
	val.m_intH &= ~(dgUnsigned64(1) << 31);
	NEWTON_ASSERT(val.m_float == fabs(x));

	return dgFloat32(val.m_float);
#else
	// according to Intel this is better because is doe not read after write
	return (x >= dgFloat32(0.0f)) ? x : -x;
#endif
}

#ifndef __USE_DOUBLE_PRECISION__
DG_INLINE dgInt32 dgFastInt(dgFloat64 x) {
	dgInt32 i = dgInt32(x);

	if (dgFloat64(i) > x) {
		i--;
	}

	return i;
}
#endif

DG_INLINE dgInt32 dgFastInt(dgFloat32 x) {
#if 0
	volatile dgDoubleInt val;
	volatile dgDoubleInt round;
	const dgFloat64 conversionMagicConst = ((dgFloat64(dgInt64(1) << 52)) * dgFloat64(1.5f));
	val.m_float = dgFloat64(x) + conversionMagicConst;
	round.m_float = x - dgFloat64(val.m_intL);
	dgInt32 ret = val.m_intL + (round.m_intH >> 31);
	NEWTON_ASSERT(ret == dgInt32(floor(x)));

	return ret;
#else
	dgInt32 i = dgInt32(x);

	if (dgFloat32(i) > x) {
		i--;
	}

	return i;
#endif
}

DG_INLINE dgFloat32 dgFloor(dgFloat32 x) {
#ifdef _MSC_VER
	dgFloat32 ret = dgFloat32(dgFastInt(x));
	NEWTON_ASSERT(ret == floor(x));

	return ret;
#else
	return floor(x);
#endif
}

DG_INLINE dgFloat32 dgCeil(dgFloat32 x) {
#ifdef _MSC_VER
	dgFloat32 ret = dgFloor(x);

	if (ret < x) {
		ret += dgFloat32(1.0f);
	}

	NEWTON_ASSERT(ret == ceil(x));

	return ret;
#else
	return ceil(x);
#endif
}

#define dgSqrt(x) dgFloat32(sqrt(x))
#define dgRsqrt(x) (dgFloat32(1.0f) / dgSqrt(x))
#define dgSin(x) dgFloat32(sin(x))
#define dgCos(x) dgFloat32(cos(x))
#define dgAsin(x) dgFloat32(asin(x))
#define dgAcos(x) dgFloat32(acos(x))
#define dgAtan2(x, y) dgFloat32(atan2(x, y))
#define dgLog(x) dgFloat32(log(x))
#define dgPow(x, y) dgFloat32(pow(x, y))
#define dgFmod(x, y) dgFloat32(fmod(x, y))

typedef dgUnsigned32(dgApi *OnGetPerformanceCountCallback)();

dgCpuClass dgApi dgGetCpuType();

inline dgInt32 dgAtomicAdd(dgInt32 *const addend, dgInt32 amount) {
	return *addend += amount;
}

#endif
