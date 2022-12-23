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
#include "dgTypes.h"
#include "dgDebug.h"
#include "dgVector.h"
#include "dgMemory.h"
#include "dgStack.h"

void GetMinMax(dgVector &minOut, dgVector &maxOut,
               const dgFloat32 *const vertexArray, dgInt32 vCount, dgInt32 strideInBytes) {
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));
	const dgFloat32 *vArray = vertexArray + stride;

	NEWTON_ASSERT(stride >= 3);
	minOut = dgVector(vertexArray[0], vertexArray[1], vertexArray[2],
	                  dgFloat32(0.0f));
	maxOut = dgVector(vertexArray[0], vertexArray[1], vertexArray[2],
	                  dgFloat32(0.0f));

	for (dgInt32 i = 1; i < vCount; i++) {
		minOut.m_x = GetMin(minOut.m_x, vArray[0]);
		minOut.m_y = GetMin(minOut.m_y, vArray[1]);
		minOut.m_z = GetMin(minOut.m_z, vArray[2]);

		maxOut.m_x = GetMax(maxOut.m_x, vArray[0]);
		maxOut.m_y = GetMax(maxOut.m_y, vArray[1]);
		maxOut.m_z = GetMax(maxOut.m_z, vArray[2]);

		vArray += stride;
	}
}

void GetMinMax(dgBigVector &minOut, dgBigVector &maxOut,
               const dgFloat64 *const vertexArray, dgInt32 vCount, dgInt32 strideInBytes) {
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));
	const dgFloat64 *vArray = vertexArray + stride;

	NEWTON_ASSERT(stride >= 3);
	minOut = dgBigVector(vertexArray[0], vertexArray[1], vertexArray[2],
	                     dgFloat64(0.0f));
	maxOut = dgBigVector(vertexArray[0], vertexArray[1], vertexArray[2],
	                     dgFloat64(0.0f));

	for (dgInt32 i = 1; i < vCount; i++) {
		minOut.m_x = GetMin(minOut.m_x, vArray[0]);
		minOut.m_y = GetMin(minOut.m_y, vArray[1]);
		minOut.m_z = GetMin(minOut.m_z, vArray[2]);

		maxOut.m_x = GetMax(maxOut.m_x, vArray[0]);
		maxOut.m_y = GetMax(maxOut.m_y, vArray[1]);
		maxOut.m_z = GetMax(maxOut.m_z, vArray[2]);

		vArray += stride;
	}
}

#ifdef _MSC_VER

#ifdef DG_BUILD_SIMD_CODE
#if (_MSC_VER >= 1400)
static bool DetectSSE_3() {
	__try {
		__m128 i;
		i = _mm_set_ps(dgFloat32(1.0f), dgFloat32(2.0f), dgFloat32(3.0f), dgFloat32(4.0f));
		i = _mm_hadd_ps(i, i);
		i = _mm_hadd_ps(i, i);
		i = i;
//					i = _mm_dp_ps(i, i, 0);
//					i = _mm_dp_ps(i, i, 0);
//					i = _mm_hadd_ps(i, i);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	return true;
}
#else
static bool DetectSSE_3() {
	return false;
}
#endif // _MSC_VER >= 1400
static bool DetectSSE() {
	__try {
		__m128 i;
		i = _mm_set_ps(dgFloat32(1.0f), dgFloat32(2.0f), dgFloat32(3.0f), dgFloat32(4.0f));
		i = _mm_add_ps(i, i);
		i = i;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	return true;
}
#else
static bool DetectSSE_3() {
	return false;
}

static bool DetectSSE() {
	return false;
}
#endif // DG_BUILD_SIMD_CODE

dgCpuClass dgApi dgGetCpuType() {
	if (DetectSSE_3()) {
		return dgSimdPresent;
	}

	if (DetectSSE()) {
		return dgSimdPresent;
	}

	return dgNoSimdPresent;
}

#elif defined(__APPLE__)

dgCpuClass dgApi dgGetCpuType() {
#ifdef __ppc__

	return dgNoSimdPresent;
	/*
 #ifdef   _SCALAR_AROTHMETIC_ONLY
	 return dgNoSimdPresent;
 #else
	 dgInt32 error;
	 dgInt32 hasSimd;
	 size_t length = sizeof( hasSimd );

	 hasSimd = 0;
	 error = sysctlbyname("hw.optional.altivec", & hasSimd, &length, NULL, 0);
	 if (error) {
	 return dgNoSimdPresent;
	 }
	 if (hasSimd) {
	 return dgSimdPresent;
	 }
	 return dgNoSimdPresent;
 #endif
	 */
#else
	dgInt32 error;
	dgInt32 hasSimd;
	size_t length = sizeof(hasSimd);

	hasSimd = 0;
	error = sysctlbyname("hw.optional.sse2", & hasSimd, &length, NULL, 0);
	if (error) {
		return dgNoSimdPresent;
	}
	if (hasSimd) {
		return dgSimdPresent;
	}
	return dgNoSimdPresent;
#endif

}

#elif defined(HAVE_X86) || defined(HAVE_AMD64)
/*  #define cpuid(func,ax,bx,cx,dx) __asm__ __volatile__ ("cpuid": "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func)); */

void cpuid(dgUnsigned32 op, dgUnsigned32 reg[4]) {
	asm volatile(
#ifdef __x86_64__
	    "pushq %%rbx      \n\t" /* save %rbx */
#else
	    "pushl %%ebx      \n\t" /* save %ebx */
#endif

	    "cpuid            \n\t"
	    "movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */

#ifdef __x86_64__
	    "popq %%rbx       \n\t" /* restore the old %rbx */
#else
	    "popl %%ebx       \n\t" /* restore the old %ebx */
#endif
	    : "=a"(reg[0]), "=r"(reg[1]), "=c"(reg[2]), "=d"(reg[3])
	    : "a"(op)
	    : "cc");
}

static dgInt32 cpuid(void) {
//		int a, b, c, d;
//		cpuid(1,a,b,c,d);
//		return d;

	dgUnsigned32 reg[4];
	cpuid(1, reg);
	return reg[3];
}

dgCpuClass dgApi dgGetCpuType() {
#define bit_MMX (1 << 23)
#define bit_SSE (1 << 25)
#define bit_SSE2 (1 << 26)

#ifndef __USE_DOUBLE_PRECISION__
	if (cpuid() & bit_SSE) {
		return dgSimdPresent;
	}
#endif

	return dgNoSimdPresent;
}

#else // defined(HAVE_X86) || defined(HAVE_AMD64)

dgCpuClass dgApi dgGetCpuType() {
	return dgNoSimdPresent;
}


#endif

static inline dgInt32 cmp_vertex(const dgFloat64 *const v1,
                                 const dgFloat64 *const v2, dgInt32 firstSortAxis) {
	if (v1[firstSortAxis] < v2[firstSortAxis]) {
		return -1;
	}

	if (v1[firstSortAxis] > v2[firstSortAxis]) {
		return 1;
	}

	return 0;
}

static dgInt32 SortVertices(dgFloat64 *const vertexList, dgInt32 stride,
                            dgInt32 compareCount, dgInt32 vertexCount, dgFloat64 tolerance) {
	dgFloat64 xc = 0;
	dgFloat64 yc = 0;
	dgFloat64 zc = 0;
	dgFloat64 x2c = 0;
	dgFloat64 y2c = 0;
	dgFloat64 z2c = 0;

	dgBigVector minP(1e10, 1e10, 1e10, 0);
	dgBigVector maxP(-1e10, -1e10, -1e10, 0);
	dgInt32 k = 0;
	for (dgInt32 i = 0; i < vertexCount; i++) {
		dgFloat64 x = vertexList[k + 2];
		dgFloat64 y = vertexList[k + 3];
		dgFloat64 z = vertexList[k + 4];
		k += stride;

		xc += x;
		yc += y;
		zc += z;
		x2c += x * x;
		y2c += y * y;
		z2c += z * z;

		if (x < minP.m_x) {
			minP.m_x = x;
		}
		if (y < minP.m_y) {
			minP.m_y = y;
		}

		if (z < minP.m_z) {
			minP.m_z = z;
		}

		if (x > maxP.m_x) {
			maxP.m_x = x;
		}
		if (y > maxP.m_y) {
			maxP.m_y = y;
		}

		if (z > maxP.m_z) {
			maxP.m_z = z;
		}
	}

	dgBigVector del(maxP - minP);
	dgFloat64 minDist = GetMin(del.m_x, del.m_y, del.m_z);
	if (minDist < 1.0e-3) {
		minDist = 1.0e-3;
	}

	dgFloat64 tol = tolerance * minDist + 1.0e-12f;
	dgFloat64 sweptWindow = 2.0 * tol;
	sweptWindow += 1.0e-4;

	x2c = vertexCount * x2c - xc * xc;
	y2c = vertexCount * y2c - yc * yc;
	z2c = vertexCount * z2c - zc * zc;

	dgInt32 firstSortAxis = 2;
	if ((y2c >= x2c) && (y2c >= z2c)) {
		firstSortAxis = 3;
	} else if ((z2c >= x2c) && (z2c >= y2c)) {
		firstSortAxis = 4;
	}

	dgInt32 stack[1024][2];
	stack[0][0] = 0;
	stack[0][1] = vertexCount - 1;
	dgInt32 stackIndex = 1;
	while (stackIndex) {
		stackIndex--;
		dgInt32 lo = stack[stackIndex][0];
		dgInt32 hi = stack[stackIndex][1];
		if ((hi - lo) > 8) {
			dgInt32 i = lo;
			dgInt32 j = hi;
			dgFloat64 val[64];
			memcpy(val, &vertexList[((lo + hi) >> 1) * stride],
			       stride * sizeof(dgFloat64));
			do {
				while (cmp_vertex(&vertexList[i * stride], val, firstSortAxis) < 0)
					i++;
				while (cmp_vertex(&vertexList[j * stride], val, firstSortAxis) > 0)
					j--;

				if (i <= j) {
					dgFloat64 tmp[64];
					memcpy(tmp, &vertexList[i * stride], stride * sizeof(dgFloat64));
					memcpy(&vertexList[i * stride], &vertexList[j * stride],
					       stride * sizeof(dgFloat64));
					memcpy(&vertexList[j * stride], tmp, stride * sizeof(dgFloat64));
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
		} else {
			for (dgInt32 i = lo + 1; i <= hi; i++) {
				dgFloat64 tmp[64];
				memcpy(tmp, &vertexList[i * stride], stride * sizeof(dgFloat64));

				dgInt32 j = i;
				for (;
				        j
				        && (cmp_vertex(&vertexList[(j - 1) * stride], tmp,
				                       firstSortAxis) > 0); j--) {
					memcpy(&vertexList[j * stride], &vertexList[(j - 1) * stride],
					       stride * sizeof(dgFloat64));
				}
				memcpy(&vertexList[j * stride], tmp, stride * sizeof(dgFloat64));
			}
		}
	}

#ifdef _DEBUG
	for (dgInt32 i = 0; i < (vertexCount - 1); i++) {
		NEWTON_ASSERT(
		    cmp_vertex(&vertexList[i * stride], &vertexList[(i + 1) * stride], firstSortAxis) <= 0);
	}
#endif

	dgInt32 count = 0;
	for (dgInt32 i = 0; i < vertexCount; i++) {
		dgInt32 m = i * stride;
		dgInt32 index = dgInt32(vertexList[m + 0]);
		if (index == dgInt32(0xffffffff)) {
			dgFloat64 swept = vertexList[m + firstSortAxis] + sweptWindow;
			dgInt32 q = i * stride + stride;
			for (dgInt32 i1 = i + 1; i1 < vertexCount; i1++) {

				index = dgInt32(vertexList[q + 0]);
				if (index == dgInt32(0xffffffff)) {
					dgFloat64 val = vertexList[q + firstSortAxis];
					if (val >= swept) {
						break;
					}
					bool test = true;
					for (dgInt32 t = 0; test && (t < compareCount); t++) {
						dgFloat64 v = fabs(vertexList[m + t + 2] - vertexList[q + t + 2]);
						test = test && (v <= tol);
					}
					if (test) {
						vertexList[q + 0] = dgFloat64(count);
					}
				}
				q += stride;
			}

			memcpy(&vertexList[count * stride + 2], &vertexList[m + 2],
			       (stride - 2) * sizeof(dgFloat64));
			vertexList[m + 0] = dgFloat64(count);
			count++;
		}
	}

	return count;
}

//static dgInt32 QuickSortVertices (dgFloat32* const vertList, dgInt32 stride, dgInt32 floatSize, dgInt32 unsignedSize, dgInt32 vertexCount, dgFloat32 tolerance)
static dgInt32 QuickSortVertices(dgFloat64 *const vertList, dgInt32 stride,
                                 dgInt32 compareCount, dgInt32 vertexCount, dgFloat64 tolerance) {
	dgInt32 count = 0;
	if (vertexCount > (3 * 1024 * 32)) {
		dgFloat64 x = dgFloat32(0.0f);
		dgFloat64 y = dgFloat32(0.0f);
		dgFloat64 z = dgFloat32(0.0f);
		dgFloat64 xd = dgFloat32(0.0f);
		dgFloat64 yd = dgFloat32(0.0f);
		dgFloat64 zd = dgFloat32(0.0f);

		for (dgInt32 i = 0; i < vertexCount; i++) {
			dgFloat64 x0 = vertList[i * stride + 2];
			dgFloat64 y0 = vertList[i * stride + 3];
			dgFloat64 z0 = vertList[i * stride + 4];
			x += x0;
			y += y0;
			z += z0;
			xd += x0 * x0;
			yd += y0 * y0;
			zd += z0 * z0;
		}

		xd = vertexCount * xd - x * x;
		yd = vertexCount * yd - y * y;
		zd = vertexCount * zd - z * z;

		dgInt32 axis = 2;
		dgFloat64 axisVal = x / vertexCount;
		if ((yd > xd) && (yd > zd)) {
			axis = 3;
			axisVal = y / vertexCount;
		}
		if ((zd > xd) && (zd > yd)) {
			axis = 4;
			axisVal = z / vertexCount;
		}

		dgInt32 i0 = 0;
		dgInt32 i1 = vertexCount - 1;
		do {
			for (; vertList[i0 * stride + axis] < axisVal; i0++)
				;
			for (; vertList[i1 * stride + axis] > axisVal; i1--)
				;
			if (i0 <= i1) {
				for (dgInt32 i = 0; i < stride; i++) {
					Swap(vertList[i0 * stride + i], vertList[i1 * stride + i]);
				}
				i0++;
				i1--;
			}
		} while (i0 <= i1);
		NEWTON_ASSERT(i0 < vertexCount);

		dgInt32 count0 = QuickSortVertices(&vertList[0 * stride], stride,
		                                   compareCount, i0, tolerance);
		dgInt32 count1 = QuickSortVertices(&vertList[i0 * stride], stride,
		                                   compareCount, vertexCount - i0, tolerance);

		count = count0 + count1;

		for (dgInt32 i = 0; i < count1; i++) {
			memcpy(&vertList[(count0 + i) * stride + 2],
			       &vertList[(i0 + i) * stride + 2], (stride - 2) * sizeof(dgFloat64));
		}

//		dgFloat64* const indexPtr = (dgInt64*)vertList;
		for (dgInt32 i = i0; i < vertexCount; i++) {
//			indexPtr[i * stride] += count0;
			vertList[i * stride] += dgFloat64(count0);
		}

	} else {
		count = SortVertices(vertList, stride, compareCount, vertexCount,
		                     tolerance);
	}

	return count;
}

dgInt32 dgVertexListToIndexList(dgFloat64 *const vertList,
                                dgInt32 strideInBytes, dgInt32 compareCount, dgInt32 vertexCount,
                                dgInt32 *const indexListOut, dgFloat64 tolerance) {
#ifdef _WIN32
	dgUnsigned32 controlWorld = dgControlFP(0xffffffff, 0);
	dgControlFP(_PC_53, _MCW_PC);
#endif

	if (strideInBytes < 3 * dgInt32(sizeof(dgFloat64))) {
		return 0;
	}
	if (compareCount < 3) {
		return 0;
	}
	NEWTON_ASSERT(compareCount <= dgInt32(strideInBytes / sizeof(dgFloat64)));
	NEWTON_ASSERT(strideInBytes == dgInt32(sizeof(dgFloat64) * (strideInBytes / sizeof(dgFloat64))));

	dgInt32 stride = strideInBytes / dgInt32(sizeof(dgFloat64));
	dgInt32 stride2 = stride + 2;

	dgStack<dgFloat64> pool(stride2 * vertexCount);
	dgFloat64 *const tmpVertexList = &pool[0];

//	dgInt64* const indexPtr = (dgInt64*)tmpVertexList;

	dgInt32 k = 0;
	dgInt32 m = 0;
	for (dgInt32 i = 0; i < vertexCount; i++) {
		memcpy(&tmpVertexList[m + 2], &vertList[k], stride * sizeof(dgFloat64));
		tmpVertexList[m + 0] = dgFloat64(-1.0f);
		tmpVertexList[m + 1] = dgFloat64(i);
		k += stride;
		m += stride2;
	}

	dgInt32 count = QuickSortVertices(tmpVertexList, stride2, compareCount,
	                                  vertexCount, tolerance);

	k = 0;
	m = 0;
	for (dgInt32 i = 0; i < count; i++) {
		k = i * stride;
		m = i * stride2;
		memcpy(&vertList[k], &tmpVertexList[m + 2], stride * sizeof(dgFloat64));
		k += stride;
		m += stride2;
	}

	m = 0;
	for (dgInt32 i = 0; i < vertexCount; i++) {
		dgInt32 i1 = dgInt32(tmpVertexList[m + 1]);
		dgInt32 index = dgInt32(tmpVertexList[m + 0]);
		indexListOut[i1] = index;
		m += stride2;
	}

#ifdef _WIN32
	dgControlFP(controlWorld, _MCW_PC);
#endif

	return count;
}

dgInt32 dgVertexListToIndexList(dgFloat32 *const vertList,
                                dgInt32 strideInBytes, dgInt32 floatSizeInBytes,
                                dgInt32 unsignedSizeInBytes, dgInt32 vertexCount, dgInt32 *const indexList,
                                dgFloat32 tolerance) {
	dgInt32 stride = strideInBytes / sizeof(dgFloat32);

	NEWTON_ASSERT(!unsignedSizeInBytes);
	dgStack<dgFloat64> pool(vertexCount * stride);

	dgInt32 floatCount = floatSizeInBytes / sizeof(dgFloat32);

	dgFloat64 *const data = &pool[0];
	for (dgInt32 i = 0; i < vertexCount; i++) {

		dgFloat64 *const dst = &data[i * stride];
		dgFloat32 *const src = &vertList[i * stride];
		for (dgInt32 j = 0; j < stride; j++) {
			dst[j] = src[j];
		}
	}

	dgInt32 count = dgVertexListToIndexList(data, stride * sizeof(dgFloat64),
	                                        floatCount, vertexCount, indexList, dgFloat64(tolerance));
	for (dgInt32 i = 0; i < count; i++) {
		dgFloat64 *const src = &data[i * stride];
		dgFloat32 *const dst = &vertList[i * stride];
		for (dgInt32 j = 0; j < stride; j++) {
			dst[j] = dgFloat32(src[j]);
		}
	}

	return count;
}
