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

#ifndef AFX_DGCOLLISIONCOMPOUND_BREAKABLE_H__INCLUDED_
#define AFX_DGCOLLISIONCOMPOUND_BREAKABLE_H__INCLUDED_

#include "dgCollisionCompound.h"
#include "dgCollisionConvex.h"

class dgMeshEffect;

class dgCollisionCompoundBreakable : public dgCollisionCompound {
public:
	class dgFlatVertex {
	public:
		dgFloat32 m_point[10]; // 3 point, 3 normal. 2 uv0, 2 uv1
	};

	class dgFlatVertexArray : public dgArray<dgFlatVertex> {
	public:
		dgFlatVertexArray(dgMemoryAllocator *const allocator)
			: dgArray<dgFlatVertex>(1024 * 4, allocator) {
			m_count = 0;
		}

		dgInt32 m_count;
	};

	class dgVertexBuffer : public dgRefCounter {
	public:
		DG_CLASS_ALLOCATOR(allocator)
		dgVertexBuffer(dgInt32 count, dgMemoryAllocator *allocator);
		dgVertexBuffer(dgMemoryAllocator *const allocator, dgDeserialize deserialization, void *const userData);
		~dgVertexBuffer();

		void Serialize(dgSerialize callback, void *const userData) const;
		void GetVertexStreams(dgInt32 vertexStrideInByte, dgFloat32 *vertex, dgInt32 normalStrideInByte, dgFloat32 *normal,
		                      dgInt32 uvStrideInByte, dgFloat32 *uv) const;

		dgInt32 m_vertexCount;
		dgFloat32 *m_uv;
		dgFloat32 *m_vertex;
		dgFloat32 *m_normal;
		dgMemoryAllocator *m_allocator;
	};

	class dgSubMesh {
	public:
		dgSubMesh(dgMemoryAllocator *const allocator);
		~dgSubMesh();
		void Serialize(dgSerialize callback, void *const userData) const;

		dgInt32 m_faceOffset;
		dgInt32 m_visibleFaces;
		dgInt32 m_material;
		dgInt32 m_faceCount;
		dgInt32 *m_indexes;
		dgMemoryAllocator *m_allocator;
	};

	class dgMesh : public dgList<dgSubMesh>, public dgRefCounter {
	public:
		dgMesh(dgMemoryAllocator *const allocator);
		dgMesh(dgMemoryAllocator *const allocator, dgDeserialize deserialization, void *const userData);
		~dgMesh();
		void Serialize(dgSerialize callback, void *const userData) const;
		dgSubMesh *AddgSubMesh(dgInt32 indexCount, dgInt32 material);

		dgInt32 m_IsVisible;
	};

	class dgDebriNodeInfo {
	public:
		dgDebriNodeInfo();
		~dgDebriNodeInfo();

		struct PackedSaveData {
			union {
				dgInt32 m_lru;
				dgInt32 m_shapeID;
			};
			dgInt32 m_distanceToFixNode;
			dgInt32 m_islandIndex;
		} m_commonData;

		dgMesh *m_mesh;
		dgCollisionConvex *m_shape;
	};

	class dgSharedNodeMesh {
	public:
		dgSharedNodeMesh();
		~dgSharedNodeMesh();
	};

	class dgDebriGraph : public dgGraph<dgDebriNodeInfo, dgSharedNodeMesh> {
	public:
		dgDebriGraph(dgMemoryAllocator *const allocator);
		dgDebriGraph(const dgDebriGraph &source);
		dgDebriGraph(dgMemoryAllocator *const allocator, dgDeserialize deserialization, void *const userData);
		~dgDebriGraph();

		//      void AddToHeap (dgDownHeap<dgMeshEffect*, dgFloat32>& heap, dgMeshEffect* front);
		void AddNode(dgFlatVertexArray &vertexArray, dgMeshEffect *solid, dgInt32 clipperMaterial, dgInt32 id, dgFloat32 density, dgFloat32 padding);
		//      void SplitAndAddNodes (dgFlatVertexArray& vertexArray, dgMeshEffect* solid, dgMeshEffect* const clipper,
		//                             dgInt32 clipperMaterial, dgInt32 id, dgFloat32 density, dgCollisionCompoundBreakableCallback callback, void* buildUsedData);

		//      void AddMeshes (dgFlatVertexArray& vertexArray, dgInt32 count, dgMeshEffect* const solidArray[], dgMeshEffect* const clipperArray[],
		//                      dgMatrix* const matrixArray, dgInt32* const idArray, dgFloat32* const densities, dgCollisionCompoundBreakableCallback callback, void* buildUsedData);
		void AddMeshes(dgFlatVertexArray &vertexArray, dgInt32 count, dgMeshEffect *const solidArray[],
		               const dgInt32 *const idArray, const dgFloat32 *const densities, const dgInt32 *const internalFaceMaterial, dgFloat32 gaps);

		void Serialize(dgSerialize callback, void *const userData) const;
	};

	class dgIsland : public dgList<dgDebriGraph::dgListNode *> {
	public:
		dgIsland(dgMemoryAllocator *const allocator)
			: dgList<dgDebriGraph::dgListNode *>(allocator) {
		}
	};

	class dgCollisionConvexIntance : public dgCollisionConvex {
	public:
		dgCollisionConvexIntance(dgWorld *const world, dgDeserialize deserialization, void *const userData);
		dgCollisionConvexIntance(const dgCollisionConvexIntance &source, dgDebriGraph::dgListNode *node);
		dgCollisionConvexIntance(dgCollisionConvex *convexChild, dgDebriGraph::dgListNode *node, dgFloat32 density);
		virtual ~dgCollisionConvexIntance();

	protected:
		virtual dgFloat32 GetVolume() const;
		virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;
		virtual dgVector SupportVertex(const dgVector &dir) const;
		virtual dgVector SupportVertexSimd(const dgVector &dir) const;
		virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
		virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
		virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const;
		virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
		virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
		virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const;

	private:
		virtual dgInt32 CalculateSignature() const;
		virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);
		virtual dgFloat32 GetBoxMinRadius() const;
		virtual dgFloat32 GetBoxMaxRadius() const;
		virtual dgInt32 CalculatePlaneIntersection(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;
		virtual dgInt32 CalculatePlaneIntersectionSimd(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;
		virtual void GetCollisionInfo(dgCollisionInfo *info) const;
		virtual void Serialize(dgSerialize callback, void *const userData) const;

		virtual void SetBreakImpulse(dgFloat32 force);
		virtual dgFloat32 GetBreakImpulse() const;

		dgNodeBase *m_treeNode;
		dgCollisionConvex *m_myShape;
		dgDebriGraph::dgListNode *m_graphNode;
		dgVector m_inertia;
		dgFloat32 m_destructionImpulse;

#ifdef _DEBUG
		dgInt32 m_ordinal;
#endif
		friend class dgCollisionCompoundBreakable;
	};

	class dgCompoundBreakableFilterData {
	public:
		dgInt32 m_index;
		dgDebriGraph::dgListNode *m_node;
	};

	//  class dgSegmenList: public dgList<dgMeshEffect*>
	//  {
	//      public:
	//      dgSegmenList(dgMeshEffect* const source);
	//      ~dgSegmenList();
	//  };

	dgCollisionCompoundBreakable(const dgCollisionCompoundBreakable &source);
	//  dgCollisionCompoundBreakable (dgInt32 count, dgMeshEffect* const solidArray[], dgMeshEffect* const clipperArray[],
	//                                dgMatrix* const matrixArray, dgInt32* const idArray, dgFloat32* const densities, dgInt32 debriiId,
	//                                dgCollisionCompoundBreakableCallback callback, void* buildUsedData, dgWorld* world);

	dgCollisionCompoundBreakable(dgInt32 count, dgMeshEffect *const solidArray[], const dgInt32 *const idArray,
	                             const dgFloat32 *const densities, const dgInt32 *const internalFaceMaterial,
	                             dgInt32 debriiId, dgFloat32 gaps, dgWorld *world);

	dgCollisionCompoundBreakable(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionCompoundBreakable(void);

	void DeleteComponentBegin();
	dgBody *CreateComponentBody(dgDebriGraph::dgListNode *node) const;
	void DeleteComponent(dgDebriGraph::dgListNode *node);
	void DeleteComponentEnd();

	dgDebriGraph::dgListNode *GetMainMesh() const {
		return m_conectivity.GetLast();
	}
	dgDebriGraph::dgListNode *GetFirstComponentMesh() const {
		return (m_conectivity.GetCount() > 2) ? m_conectivity.GetFirst()->GetNext() : NULL;
	}
	dgInt32 GetSegmentsInRadius(const dgVector &origin, dgFloat32 radius, dgDebriGraph::dgListNode **segments, dgInt32 maxCount) const;

	void ResetAnchor();
	void SetAnchoredParts(dgInt32 count, const dgMatrix *const matrixArray, dgCollision * const *collisionArray);
	void EnumerateIslands();
	//  dgInt32 GetSegmentsInRadius (const dgVector& origin, dgFloat32 radius, dgDebriGraph::dgListNode** segments, dgInt32 maxCount);
	//  dgInt32 GetDetachedPieces (dgCollision** shapes, dgInt32 maxCount);

	dgInt32 GetSegmentIndexStream(dgDebriGraph::dgListNode *const node, dgMesh::dgListNode *segment, dgInt32 *const index) const;
	dgInt32 GetSegmentIndexStreamShort(const dgDebriGraph::dgListNode *const node, dgMesh::dgListNode *segment, dgInt16 *const index) const;

	dgInt32 GetVertecCount() const {
		return m_vertexBuffer->m_vertexCount;
	}
	void GetVertexStreams(dgInt32 vertexStrideInByte, dgFloat32 *vertex,
	                      dgInt32 normalStrideInByte, dgFloat32 *normal,
	                      dgInt32 uvStrideInByte, dgFloat32 *uv) const {
		m_vertexBuffer->GetVertexStreams(vertexStrideInByte, vertex, normalStrideInByte, normal, uvStrideInByte, uv);
	}

private:
	void LinkNodes();

	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;

	dgInt32 m_lru;
	dgInt32 m_lastIslandColor;
	dgInt32 m_visibilityMapIndexCount;
	dgInt8 *m_visibilityMap;
	dgInt32 *m_visibilityInderectMap;
	dgVertexBuffer *m_vertexBuffer;
	dgDebriGraph m_conectivity;
	dgIsland m_detachedIslands;
};

#endif
