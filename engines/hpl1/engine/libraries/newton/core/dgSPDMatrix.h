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

#ifndef __dgSPDMatrix__
#define __dgSPDMatrix__

#include "dgStdafx.h"
#include "dgDebug.h"
#include "dgGeneralMatrix.h"

/*
void _BackAndForwardSustitition (void *rightsideVector,
                                           void *rowPointers,
                                           dgInt32 rowStrideInBytes,
                                           dgInt32 typeSizeInBytes,
                                           dgInt32 size);

bool _SolveByCholeskyDecomposition (void *rightsideVector,
                                                void *rowPointers,
                                                dgInt32 rowStrideInBytes,
                                                dgInt32 typeSizeInBytes,
                                                dgInt32 size);



template<class T>
class dgSPDMatrix: public dgGeneralMatrix<T>
{
    public:
    dgSPDMatrix (dgInt32 size);
    dgSPDMatrix (const dgSPDMatrix<T>& src);
    dgSPDMatrix (dgInt32 size, T *elemBuffer, dgGeneralVector<T>* m_rowBuffer);
    dgSPDMatrix (const dgSPDMatrix<T>& src, T *elemBuffer, dgGeneralVector<T>* m_rowBuffer);

    ~dgSPDMatrix ();

    bool TestPSD () const;
    bool CholeskyDecomposition();
    void DownDateCholeskyDecomposition (dgInt32 column);


    bool Solve (dgGeneralVector<T> &b);
    void BackAndForwardSustitition(dgGeneralVector<T> &b);
};


// ***********************************************************************************************
//
//  LinearSystem
//
// ***********************************************************************************************
template<class T>
dgSPDMatrix<T>::dgSPDMatrix (dgInt32 size)
    :dgGeneralMatrix<T>(size, size)
{
}

template<class T>
dgSPDMatrix<T>::dgSPDMatrix (const dgSPDMatrix<T>& src)
    :dgGeneralMatrix<T>(src)
{
}


template<class T>
dgSPDMatrix<T>::dgSPDMatrix (
    dgInt32 size,
    T *elemBuffer,
    dgGeneralVector<T>* m_rowBuffer)
    :dgGeneralMatrix<T>(size, size, elemBuffer, m_rowBuffer)
{
}

template<class T>
dgSPDMatrix<T>::dgSPDMatrix (
    const dgSPDMatrix<T>& src,
    T *elemBuffer,
    dgGeneralVector<T>* m_rowBuffer)
    :dgGeneralMatrix<T>(src, elemBuffer, m_rowBuffer)
{
}


template<class T>
dgSPDMatrix<T>::~dgSPDMatrix ()
{
}

template<class T>
bool dgSPDMatrix<T>::TestPSD () const
{
    if (!TestSymetry ()) {
        return false;
    }

    dgSPDMatrix<T> tmp (*this);
    return tmp.CholeskyDecomposition();
}


template<class T>
void dgSPDMatrix<T>::BackAndForwardSustitition(dgGeneralVector<T> &b)
{
    _BackAndForwardSustitition (b.m_columns, &m_rows[0].m_columns, sizeof (dgGeneralVector<T>), sizeof (T), m_rowCount);
}


template<class T>
bool dgSPDMatrix<T>::Solve (dgGeneralVector<T> &b)
{
    return _SolveByCholeskyDecomposition (b.m_columns, &m_rows[0].m_columns, sizeof (dgGeneralVector<T>), sizeof (T), m_rowCount);
}



template<class T>
bool dgSPDMatrix<T>::CholeskyDecomposition()
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


template<class T>
void dgSPDMatrix<T>::DownDateCholeskyDecomposition (dgInt32 column)
{
    dgInt32 i;
    dgInt32 j;

    T *buffer;
    dgGeneralVector<T>* rowsBuffer;

    rowsBuffer = (dgGeneralVector<dgFloat32>*) dgStackAlloc (m_rowCount * sizeof (dgGeneralVector<T>));
    buffer = (T *) dgStackAlloc ((m_rowCount + 4) * (m_rowCount + 4) * sizeof (T));

    dgGeneralMatrix<T> tmp (m_rowCount, m_rowCount, buffer, rowsBuffer);
    dgGeneralMatrix<T>& me = *this;
    for (i = 0; i < m_rowCount; i ++) {
        tmp[i][i] =  me[i][i];
        for (j = i + 1; j < m_rowCount; j ++) {
            tmp[i][j] = T (0.0f);
            tmp[j][i] =  me[i][j];
        }
    }

    me.MatrixTimeMatrixTranspose (tmp, tmp);
    for (i = 0; i < m_rowCount; i ++) {
        me[i][column] = T (0.0f);
        me[column][i] = T (0.0f);
    }
    me[column][column] = T (1.0f);

    CholeskyDecomposition();
}
*/

#endif

