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
#include "dgMemory.h"
#include "dgSPDMatrix.h"

//static bool CholeskyDecomposition (dgFloat32 **rowPointers, dgInt32 size);
//static void BackAndForwardSustitition (dgFloat32 **rowPointers, dgInt32 size, dgFloat32 *rightsideVector);

/*
 bool _CholeskyDecomposition (
 void *rowPointers, 
 dgInt32 rowStrideInBytes,
 dgInt32 typeSizeInBytes,
 dgInt32 size)
 {
 dgUnsigned8 *rowArray;

 rowArray	= (dgUnsigned8*)rowPointers;
 if (typeSizeInBytes == sizeof (dgFloat32)) {
 dgInt32 i;
 dgFloat32 **rows;
 rows = (dgFloat32**) dgStackAlloc (size * sizeof (dgFloat32*));
 for (i = 0; i < size; i ++) {
 rows[i] = *((dgFloat32 **)rowArray);
 rowArray += rowStrideInBytes;
 }
 return CholeskyDecomposition (rows, size);

 } else {
 _ASSERTE (0);
 _ASSERTE (typeSizeInBytes == sizeof (dgFloat64));
 }

 return true;
 }
 */

/*
 void _BackAndForwardSustitition (
 void *rightsideVector,
 void *rowPointers, 
 dgInt32 rowStrideInBytes, 
 dgInt32 typeSizeInBytes, 
 dgInt32 size)
 {
 dgUnsigned8 *rowArray;

 rowArray	= (dgUnsigned8*)rowPointers;
 if (typeSizeInBytes == sizeof (dgFloat32)) {
 dgInt32 i;
 dgFloat32 **rows;
 rows = (dgFloat32**) dgStackAlloc (size * sizeof (dgFloat32*));
 for (i = 0; i < size; i ++) {
 rows[i] = *((dgFloat32 **)rowArray);
 rowArray += rowStrideInBytes;
 }
 BackAndForwardSustitition (rows, size, (dgFloat32*)rightsideVector);

 } else {
 _ASSERTE (0);
 _ASSERTE (typeSizeInBytes == sizeof (dgFloat64));
 }
 }
 */

/*
 bool _SolveByCholeskyDecomposition (
 void *rightsideVector,
 void *rowPointers, 
 dgInt32 rowStrideInBytes, 
 dgInt32 typeSizeInBytes, 
 dgInt32 size)
 {
 dgUnsigned8 *rowArray;

 rowArray	= (dgUnsigned8*)rowPointers;
 if (typeSizeInBytes == sizeof (dgFloat32)) {
 dgInt32 i;
 dgFloat32 **rows;
 rows = (dgFloat32**) dgStackAlloc (size * sizeof (dgFloat32*));
 for (i = 0; i < size; i ++) {
 rows[i] = *((dgFloat32 **)rowArray);
 rowArray += rowStrideInBytes;
 }


 if (CholeskyDecomposition (rows, size)) {
 BackAndForwardSustitition (rows, size, (dgFloat32*)rightsideVector);
 return true;
 }
 } else {
 _ASSERTE (0);
 _ASSERTE (typeSizeInBytes == sizeof (dgFloat64));
 }

 return false;
 }
 */

/*
 void BackAndForwardSustitition (
 dgFloat32 **rows, 
 dgInt32 size, 
 dgFloat32 *B)
 {
 dgInt32 i;
 dgInt32 j;
 dgFloat32 acc;

 //dgSPDMatrix<dgFloat32> M (8);
 //M.CholeskyDecomposition();


 #ifdef DG_COUNT_FLOAT_OPS
 dgInt32 memCount;
 dgInt32 floatCount;

 memCount = dgGeneralVector<dgFloat32>::GetMemWrites();
 floatCount = dgGeneralVector<dgFloat32>::GetFloatOps();
 #endif

 B[0] = B[0] / rows[0][0];
 for (i =	1; i < size; i ++) {
 acc = 0.0f;
 for (j =	0; j < i; j ++) {
 acc = acc + rows[j][i] * B[j];
 #ifdef DG_COUNT_FLOAT_OPS
 floatCount += 2;
 #endif
 }
 B[i] = (B[i] - acc) / rows[i][i];
 #ifdef DG_COUNT_FLOAT_OPS
 floatCount += 2;
 memCount  += 1;
 #endif
 }

 B[size-1] = B[size-1] / rows[size-1][size-1];
 for (i =	size - 2; i >= 0; i --) {
 acc = 0.0f;
 dgFloat32 *row; 

 row = rows[i];
 for (j = i + 1; j < size; j ++) {
 acc = acc + row[j] * B[j];

 #ifdef DG_COUNT_FLOAT_OPS
 floatCount += 2;
 #endif
 }
 B[i] = (B[i] - acc) / rows[i][i];

 #ifdef DG_COUNT_FLOAT_OPS
 floatCount += 2;
 memCount  += 1;
 #endif
 }

 #ifdef DG_COUNT_FLOAT_OPS
 dgGeneralVector<dgFloat32>::SetMemWrites(memCount); 
 dgGeneralVector<dgFloat32>::SetFloatOps(floatCount); 
 #endif
 }

 */
/*
 bool CholeskyDecomposition (dgFloat32 **rows, dgInt32 size)
 {
 dgInt32 i;
 dgInt32 j;
 dgInt32 k;
 dgFloat32 factor;

 #ifdef DG_COUNT_FLOAT_OPS
 dgInt32 memCount;
 dgInt32 floatCount;

 memCount = dgGeneralVector<dgFloat32>::GetMemWrites();
 floatCount = dgGeneralVector<dgFloat32>::GetFloatOps();
 #endif

 for (j = 0; j < size; j++) {
 for (k = 0; k < j; k ++ ) {
 factor = rows[k][j];
 if (dgAbsf (factor) > 1.0e-6f) {
 for (i = j; i < size; i ++) {
 rows[j][i] -= rows[k][i] * factor;
 #ifdef DG_COUNT_FLOAT_OPS
 memCount += 1;
 floatCount += 2;
 #endif
 }
 }
 }

 factor = rows[j][j];
 if (factor <= 0.0f) {
 if (factor <= -5.0e-4f) {
 return false;
 }
 factor = 1.0e-12f;
 }

 factor = dgSqrt (factor);
 rows[j][j] = factor;
 factor = 1.0f / factor;

 #ifdef DG_COUNT_FLOAT_OPS
 memCount += 1;
 floatCount += 1;
 #endif

 for (k = j + 1; k < size; k ++) {
 rows[j][k] *= factor;
 #ifdef DG_COUNT_FLOAT_OPS
 memCount += 1;
 floatCount += 1;
 #endif
 }
 }

 #ifdef DG_COUNT_FLOAT_OPS
 dgGeneralVector<dgFloat32>::SetMemWrites(memCount); 
 dgGeneralVector<dgFloat32>::SetFloatOps(floatCount); 
 #endif

 return true;
 }
 */

