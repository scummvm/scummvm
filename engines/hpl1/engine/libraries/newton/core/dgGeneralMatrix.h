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

#ifndef __dgGeneralMatrix__
#define __dgGeneralMatrix__

#include "dgStdafx.h"
#include "dgDebug.h"
#include "dgGeneralVector.h"




template <class T> dgInt32 dgGeneralMatrixCalcBufferSizeInBytes (dgInt32 row, dgInt32 column)
{
	dgInt32 columnPad;
	columnPad = ((column * sizeof (T) + 0x0f) & -0x0f) / sizeof (T);
	return row * columnPad * sizeof (T) + row * sizeof (dgGeneralVector<T>);
}


template<class T>
class dgGeneralMatrix
{
	public:
	dgGeneralMatrix (dgInt32 row, dgInt32 column);
	dgGeneralMatrix (const dgGeneralMatrix<T>& src);
	dgGeneralMatrix (const dgGeneralMatrix<T>& src, T *elemBuffer);
	dgGeneralMatrix (dgInt32 row, dgInt32 column, T *elemBuffer);
	~dgGeneralMatrix ();

	dgGeneralVector<T>& operator[] (dgInt32 i);
	const dgGeneralVector<T>& operator[] (dgInt32 i) const;

	dgInt32 GetRowCount() const;
	dgInt32 GetColCount() const;

	void Clear (T val);
	void Identity ();

	void SwapRows (dgInt32 i, dgInt32 j); 
	void SwapColumns (dgInt32 i, dgInt32 j); 

//	dgGeneralMatrix Transpose ();
//	void Inverse (dgGeneralMatrix& inverseOut);
	void GaussianPivotStep (dgInt32 srcRow, dgInt32 pivotRow, dgInt32 pivotCol, T tol = T (1.0e-6f));


	// calculate out = V * A;
	void VectorTimeMatrix (const dgGeneralVector<T> &v, dgGeneralVector<T> &out);

	// calculate out = A * transpose (V);
	void MatrixTimeVectorTranspose (const dgGeneralVector<T> &v, dgGeneralVector<T> &out);


	// calculate M = A * B;
	void MatrixTimeMatrix (const dgGeneralMatrix<T>& A, const dgGeneralMatrix<T>& B);

	// calculate M = A * transpose (B);
	void MatrixTimeMatrixTranspose (const dgGeneralMatrix<T>& A, const dgGeneralMatrix<T>& Bt);

	bool Solve (dgGeneralVector<T> &b, T tol = T(0.0f));

	bool CholeskyDecomposition ();
	bool TestPSD () const;
	bool TestSymetry () const;


	void Trace () const;


	protected:
	bool m_ownMemory;
	dgInt32 m_rowCount;
	dgInt32 m_colCount;
	dgGeneralVector<T>* m_rows;

	private:
	T* m_buffer;
};


// ***********************************************************************************************
//
//	LinearSystem
//
// ***********************************************************************************************
template<class T>
dgGeneralMatrix<T>::dgGeneralMatrix (dgInt32 row, dgInt32 column)
{
	dgInt32 i;
	dgInt32 columnPad;
	_ASSERTE  (row > 0);
	_ASSERTE  (column > 0);

	m_rowCount = row;
	m_colCount = column;
	m_ownMemory = true;


	columnPad = ((column * sizeof (T) + 0x0f) & -0x0f) / sizeof (T);
	m_buffer = new T [row * columnPad];

	m_rows = new dgGeneralVector<T>[row];
	for (i = 0; i < row; i ++) {
		m_rows[i] = dgGeneralVector<T> (column, &m_buffer[i * columnPad]);
	}
}


template<class T>
dgGeneralMatrix<T>::dgGeneralMatrix (const dgGeneralMatrix<T>& src)
{
	dgInt32 i;
	dgInt32 columnPad;

	m_ownMemory = true;
	m_rowCount = src.m_rowCount;
	m_colCount = src.m_colCount;

	columnPad = ((m_colCount * sizeof (T) + 0x0f) & -0x0f) / sizeof (T);
	m_buffer = new T [m_rowCount * columnPad];
	m_rows = new dgGeneralVector<T>[m_rowCount];

	for (i = 0; i < m_rowCount; i ++) {
		m_rows[i] = dgGeneralVector<T> (src[i], &m_buffer[i * columnPad]);
	}
}

template<class T>
dgGeneralMatrix<T>::dgGeneralMatrix (
	dgInt32 row, 
	dgInt32 column,
	T *elemBuffer) 
{
	dgInt32 i;
	dgInt32 columnPad;

	m_ownMemory = false;
	m_rowCount = row;
	m_colCount = column;


	_ASSERTE ((((dgUnsigned32) elemBuffer) & 0x0f) == 0);

	m_buffer = elemBuffer;
	columnPad = ((m_colCount * sizeof (T) + 0x0f) & -0x0f) / sizeof (T);
	m_rows = (dgGeneralVector<T>*) &elemBuffer[m_rowCount * columnPad];
	for (i = 0; i < row; i ++) {
		m_rows[i] = dgGeneralVector<T> (column, &m_buffer[i * columnPad]);
	}
}


template<class T>
dgGeneralMatrix<T>::dgGeneralMatrix (
	const dgGeneralMatrix<T>& src, 
	T *elemBuffer) 
{
	dgInt32 i;
	dgInt32 columnPad;

	m_ownMemory = false;
	m_rowCount = src.m_rowCount;
	m_colCount = src.m_colCount;

	_ASSERTE ((((dgUnsigned32) elemBuffer) & 0x0f) == 0);
	m_buffer = elemBuffer;

	columnPad = ((m_colCount * sizeof (T) + 0x0f) & -0x0f) / sizeof (T);
	m_rows = (dgGeneralVector<T>*) &elemBuffer[m_rowCount * columnPad];
	for (i = 0; i < m_rowCount; i ++) {
		m_rows[i] = dgGeneralVector<T> (src[i], &m_buffer[i * columnPad]);
	}
}



template<class T>  
dgGeneralMatrix<T>::~dgGeneralMatrix ()
{
	if (m_ownMemory) {
		delete[] m_rows;
		delete[] m_buffer;
	}
}


template<class T>  
dgInt32 dgGeneralMatrix<T>::GetRowCount() const
{
	return m_rowCount;
}

template<class T>  
dgInt32 dgGeneralMatrix<T>::GetColCount() const
{
	return m_colCount;
}


template<class T>
void dgGeneralMatrix<T>::Trace () const
{
	dgInt32 i;

	for (i = 0; i < m_rowCount; i ++) {
		m_rows[i].Trace ();
	}
}



template<class T>
dgGeneralVector<T>& dgGeneralMatrix<T>::operator[] (dgInt32 i)
{
	_ASSERTE (i < m_rowCount);
	_ASSERTE (i >= 0);
	return m_rows[i];
}	

template<class T>
const dgGeneralVector<T>& dgGeneralMatrix<T>::operator[] (dgInt32 i) const
{
	_ASSERTE (i < m_rowCount);
	_ASSERTE (i >= 0);
	return m_rows[i];
}

template<class T>
void dgGeneralMatrix<T>::Clear (T val)
{
	dgInt32 i;
	for (i = 0; i < m_rowCount; i ++ ) {
		m_rows[i].Clear(val);
	}
}

template<class T>
void dgGeneralMatrix<T>::Identity ()
{
	dgInt32 i;

	for (i = 0; i < m_rowCount; i ++ ) {
		m_rows[i].Clear(T (0.0f));
		m_rows[i][i] = T (1.0f);
	}
}


//template<class T>
//void dgGeneralMatrix<T>::Transpose ()
//{
//	dgInt32 i;
//	dgInt32 j;
//
//	_ASSERTE (m_rowCount	== 
//	dgGeneralMatrix<T>& me = *this;
//	for (i = 0; i < m_rowCount; i ++) {
//		for (j = i + 1; j < m_rowCount; j ++) {
//			T tmp (me[i][j]);
//			me[i][j] = me[j][i];
//			me[j][i] = tmp;
//
//			#ifdef DG_COUNT_FLOAT_OPS
//			dgGeneralVector<T>::m_memoryWrite += 2;
//			#endif
//		}
//	}
//}


template<class T>
void dgGeneralMatrix<T>::GaussianPivotStep (
	dgInt32 srcRow, 
	dgInt32 pivotRow, 
	dgInt32 pivotCol,
	T tol)
{
	dgGeneralMatrix<T>& me = *this;

	T num (me[pivotRow][pivotCol]);
	if (T(dgAbsf (num)) > tol) {
		T den (me[srcRow][pivotCol]);
		_ASSERTE (T(dgAbsf (den)) > T (0.0f));

		#ifdef DG_COUNT_FLOAT_OPS
		dgGeneralVector<T>::m_floatsOp += 2;
		#endif
	
		den = - num / den;
		me[pivotRow].LinearCombine (den, me[srcRow], me[pivotRow]);
	}
}


//template<class T>
//void dgGeneralMatrix<T>::Inverse (dgGeneralMatrix& inverseOut)
//{
//	_ASSERTE (m_colCount == m_rowCount);
//}


template<class T>
void dgGeneralMatrix<T>::VectorTimeMatrix (const dgGeneralVector<T> &v, dgGeneralVector<T> &out)
{
	dgInt32 i;
	dgInt32 j;
	T acc;
	T* outMem;
	const T* inMem;

	_ASSERTE (&v != &out);
	_ASSERTE (m_rowCount	== v.m_colCount);
	_ASSERTE (m_colCount == out.m_colCount);

	inMem = &v[0];
	outMem = &out[0];
	const dgGeneralMatrix<T>& me = *this;
	for (i = 0; i < m_colCount; i ++) {
		acc = T (0.0f);
		for (j = 0; j < m_rowCount; j ++) {
			acc = acc + inMem[j] * me[j][i];

			#ifdef DG_COUNT_FLOAT_OPS
			dgGeneralVector<T>::m_floatsOp += 2;
			#endif
		}
		outMem[i] = acc;
		#ifdef DG_COUNT_FLOAT_OPS
		dgGeneralVector<T>::m_memoryWrite += 1;
		#endif
	}
}


template<class T>
void dgGeneralMatrix<T>::MatrixTimeVectorTranspose (const dgGeneralVector<T> &v, dgGeneralVector<T> &out)
{
	dgInt32 i;

	_ASSERTE (&v != &out);
	_ASSERTE (m_rowCount	== out.m_colCount);
	_ASSERTE (m_colCount	== v.m_colCount);

	for (i = 0; i < m_rowCount; i ++) {
		out[i] = v.DotProduct (m_rows[i]);
	}
}

template<class T>
void dgGeneralMatrix<T>::MatrixTimeMatrix (const dgGeneralMatrix<T>& A, const dgGeneralMatrix<T>& B)
{
	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	dgInt32 count;
 	T* out;
	T* rowA;

	_ASSERTE (m_rowCount	== A.m_rowCount);
	_ASSERTE (m_colCount	== B.m_colCount);
	_ASSERTE (A.m_colCount == B.m_rowCount);

	_ASSERTE (this != &A);

	count = A.m_colCount;
	for (i = 0; i < m_rowCount; i ++) {
		out = &m_rows[i][0];
		rowA = &A.m_rows[i][0];
		for (j = 0; j < m_colCount; j ++) {
			T acc (0.0f);
			for (k = 0; k < count; k ++) {
				acc = acc + rowA[k] * B.m_rows[k][j]; 

				#ifdef DG_COUNT_FLOAT_OPS
				dgGeneralVector<T>::m_floatsOp += 2;
				#endif
			}
			out[j] = acc;

			#ifdef DG_COUNT_FLOAT_OPS
			dgGeneralVector<T>::m_memoryWrite += 1;
			#endif
		}
	}
}


template<class T>
void dgGeneralMatrix<T>::MatrixTimeMatrixTranspose (const dgGeneralMatrix<T>& A, const dgGeneralMatrix<T>& Bt)
{
	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	dgInt32 count;
	T* out;
	T* rowA;
	T* rowB;

	_ASSERTE (m_rowCount	== A.m_rowCount);
	_ASSERTE (m_colCount	== Bt.m_rowCount);
	_ASSERTE (A.m_colCount == Bt.m_colCount);

	_ASSERTE (this != &A);
	_ASSERTE (this != &Bt);

	count = A.m_colCount;
	for (i = 0; i < m_rowCount; i ++) {
		out = &m_rows[i][0];
		rowA = &A.m_rows[i][0];
		for (j = 0; j < m_colCount; j ++) {
			T acc (0.0f);
			rowB = &Bt.m_rows[j][0];
			for (k = 0; k < count; k ++) {
				acc = acc + rowA[k] * rowB[k]; 

				#ifdef DG_COUNT_FLOAT_OPS
				dgGeneralVector<T>::m_floatsOp += 2;
				#endif
			}
			out[j] = acc;

			#ifdef DG_COUNT_FLOAT_OPS
			dgGeneralVector<T>::m_memoryWrite += 1;
			#endif
		}
	}
}




template<class T>
bool dgGeneralMatrix<T>::Solve (dgGeneralVector<T> &b, T tol)
{
	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	T* B;
	T* rowI;
	T* rowK;

	_ASSERTE (m_rowCount	== m_colCount);
	_ASSERTE (b.m_colCount == m_colCount);

	B = &b[0];
	// convert to upper triangular matrix by applying gauss pivoting
	for (i = 0; i < m_rowCount - 1; i ++) {
		rowI = &m_rows[i][0];
		T den (rowI[i]);

		if (T (dgAbsf (den)) < T (0.0f)) {
		 	return false;
		}

		for (k = i + 1; k < m_rowCount; k ++) {
			rowK = &m_rows[k][0];
			T num (rowK[i]);

			if (T (dgAbsf (num)) > tol) {
				num = num / den;
				for (j = i + 1; j < m_rowCount; j ++) {
					rowK[j] = rowK[j] - rowI[j] * num;

					#ifdef DG_COUNT_FLOAT_OPS
					dgGeneralVector<T>::m_floatsOp += 2;
					dgGeneralVector<T>::m_memoryWrite += 1;
					#endif
				}
				B[k] = B[k] - B[i] * num;

				#ifdef DG_COUNT_FLOAT_OPS
				dgGeneralVector<T>::m_floatsOp += 3;
				dgGeneralVector<T>::m_memoryWrite += 1;
				#endif
			}
		}
	}


	B[m_rowCount-1] = B[m_rowCount-1] / m_rows[m_rowCount-1][m_rowCount-1];
	for (i =	m_rowCount - 2; i >= 0; i --) {
		T acc (0);
		rowI = &m_rows[i][0];
		for (j = i + 1 ; j < m_rowCount; j ++) {
			acc = acc + rowI[j] * B[j];

			#ifdef DG_COUNT_FLOAT_OPS
			dgGeneralVector<T>::m_floatsOp += 2;
			#endif
		}
		B[i] = (B[i] - acc) / rowI[i];

		#ifdef DG_COUNT_FLOAT_OPS
		dgGeneralVector<T>::m_floatsOp += 2;
		dgGeneralVector<T>::m_memoryWrite += 1;
		#endif
	}

	#ifdef DG_COUNT_FLOAT_OPS
	dgGeneralVector<T>::m_floatsOp += 1;
	dgGeneralVector<T>::m_memoryWrite += 1;
	#endif
		
	return true;
}

template<class T>
void dgGeneralMatrix<T>::SwapRows (dgInt32 i, dgInt32 j)
{
	_ASSERTE (i	>= 0);
	_ASSERTE (j	>= 0);
	_ASSERTE (i	< m_rowCount);
	_ASSERTE (j	< m_rowCount);
	_ASSERTE (j	!= i);
	Swap(m_rows[i].m_columns, m_rows[j].m_columns);
}

template<class T>
void dgGeneralMatrix<T>::SwapColumns (dgInt32 i, dgInt32 j)
{
	dgInt32 k;
	_ASSERTE (i	>= 0);
	_ASSERTE (j	>= 0);
	_ASSERTE (i	< m_colCount);
	_ASSERTE (j	< m_colCount);
	for (k = 0; k < m_colCount; k ++) {
		Swap(m_rows[k][i], m_rows[k][j]);
	}
}



template<class T>
bool dgGeneralMatrix<T>::TestSymetry () const
{
	dgInt32 i;
	dgInt32 j;

	if (m_colCount	!= m_rowCount) {
		return false;
	}

	dgGeneralMatrix<T> mat = *this;
	for (i = 0; i < m_rowCount; i ++) {
		for (j = i + 1; j < m_rowCount; j ++) {
			if (dgAbsf (mat[i][j] - mat[j][i]) > 1.0e-12f) {
				return false;
			}
		}
	}
	return true;
}

template<class T>
bool dgGeneralMatrix<T>::TestPSD () const
{
	if (!TestSymetry ()) {
		return false;
	}

	dgGeneralMatrix<T> tmp (*this);
	return tmp.CholeskyDecomposition();
}



template<class T>
bool dgGeneralMatrix<T>::CholeskyDecomposition()
{
	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	T factor;
	T* rowK;
	T* rowJ;

	#ifdef DG_COUNT_FLOAT_OPS
	dgInt32 memCount;
	dgInt32 floatCount;

	memCount = dgGeneralVector<T>::GetMemWrites();
	floatCount = dgGeneralVector<T>::GetFloatOps();
	#endif

	for (j = 0; j < m_rowCount; j++) {
//Trace ();
//dgTrace (("\n"));
		rowJ = &m_rows[j].m_columns[0];

		for (k = 0; k < j; k ++ ) {
			rowK = &m_rows[k].m_columns[0];

			factor = rowK[j];
			if (dgAbsf (factor) > 1.0e-6f) {
				for (i = j; i < m_rowCount; i ++) {
					rowJ[i] -= rowK[i] * factor;
					#ifdef DG_COUNT_FLOAT_OPS
					memCount += 1;
					floatCount += 2;
					#endif
				}
			}
		}

		factor = rowJ[j];
		if (factor <= T (0.0f)) {
			if (factor <= T(-5.0e-4f)) {
	 			return false;
			}
			factor = T(1.0e-12f);
		}

		factor = T (dgSqrt (dgFloat32(factor)));
		rowJ[j] = factor;
		factor = T(1.0f / dgFloat32(factor));

		#ifdef DG_COUNT_FLOAT_OPS
		memCount += 1;
		floatCount += 1;
		#endif

		for (k = j + 1; k < m_rowCount; k ++) {
			rowJ[k] *= factor;
			#ifdef DG_COUNT_FLOAT_OPS
			memCount += 1;
			floatCount += 1;
			#endif
		}
	}

	#ifdef DG_COUNT_FLOAT_OPS
	dgGeneralVector<T>::SetMemWrites(memCount); 
	dgGeneralVector<T>::SetFloatOps(floatCount); 
	#endif

	return true;
}


#endif

