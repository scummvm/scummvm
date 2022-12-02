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

/****************************************************************************
*
*  Visual C++ 6.0 created by: Julio Jerez
*
****************************************************************************/
#ifndef __dgPolygonSoupDatabaseBuilder0x23413452233__
#define __dgPolygonSoupDatabaseBuilder0x23413452233__


#include "dgStdafx.h"
#include "dgRef.h"
#include "dgArray.h"
#include "dgIntersections.h"


class AdjacentdFaces {
public:
	dgInt32 m_count;
	dgInt32 *m_index;
	dgPlane m_normal;
	dgInt64 m_edgeMap[256];
};

class dgPolygonSoupDatabaseBuilder {
public:
	dgPolygonSoupDatabaseBuilder(dgMemoryAllocator *const allocator);
	~dgPolygonSoupDatabaseBuilder();

	DG_CLASS_ALLOCATOR(allocator)

	void Begin();
	void End(bool optimize);
	void AddMesh(const dgFloat32 *const vertex, dgInt32 vertexCount, dgInt32 strideInBytes, dgInt32 faceCount,
	             const dgInt32 *const faceArray, const dgInt32 *const indexArray, const dgInt32 *const faceTagsData, const dgMatrix &worldMatrix);

	void SingleFaceFixup();

private:

	void Optimize(bool optimize);
	void EndAndOptimize(bool optimize);
	void OptimizeByGroupID();
	void OptimizeByIndividualFaces();
	dgInt32 FilterFace(dgInt32 count, dgInt32 *const indexArray);
	dgInt32 AddConvexFace(dgInt32 count, dgInt32 *const indexArray, dgInt32 *const  facesArray);
	void OptimizeByGroupID(dgPolygonSoupDatabaseBuilder &source, dgInt32 faceNumber, dgInt32 faceIndexNumber, dgPolygonSoupDatabaseBuilder &leftOver);

	void PackArray();

//	void WriteDebugOutput (const char* name);

public:
	struct VertexArray: public dgArray<dgBigVector> {
		VertexArray(dgMemoryAllocator *const allocator)
			: dgArray<dgBigVector>(1024 * 256, allocator) {
		}
	};

	struct IndexArray: public dgArray<dgInt32> {
		IndexArray(dgMemoryAllocator *const allocator)
			: dgArray<dgInt32>(1024 * 256, allocator) {
		}
	};

	dgInt32 m_run;
	dgInt32 m_faceCount;
	dgInt32 m_indexCount;
	dgInt32 m_vertexCount;
	dgInt32 m_normalCount;
	IndexArray m_faceVertexCount;
	IndexArray m_vertexIndex;
	IndexArray m_normalIndex;
	VertexArray m_vertexPoints;
	VertexArray m_normalPoints;
	dgMemoryAllocator *m_allocator;

};





#endif

