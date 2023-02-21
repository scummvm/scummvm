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

#ifndef __dgMeshEffect_H__
#define __dgMeshEffect_H__

#include "hpl1/engine/libraries/newton/core/dg.h"

class dgWorld;
class dgCollision;
class dgMeshEffect;
class dgMeshEffectSolidTree;
class dgMeshTreeCSGEdgePool;

#define DG_MESH_EFFECT_INITIAL_VERTEX_SIZE 8
#define DG_MESH_EFFECT_BOLLEAN_STACK 512
#define DG_MESH_EFFECT_POINT_SPLITED 512
#define DG_MESH_EFFECT_POLYGON_SPLITED 256
#define DG_MESH_EFFECT_FLAT_CUT_BORDER_EDGE 0x01
#define DG_VERTEXLIST_INDEXLIST_TOL (dgFloat64(0.0f))

#define DG_MESH_EFFECT_PRECISION_BITS 30
#define DG_MESH_EFFECT_PRECISION_SCALE dgFloat64(1 << DG_MESH_EFFECT_PRECISION_BITS)
#define DG_MESH_EFFECT_PRECISION_SCALE_INV (dgFloat64(1.0f) / DG_MESH_EFFECT_PRECISION_SCALE)

#define DG_MESG_EFFECT_BOOLEAN_INIT()     \
	dgMeshEffect *result = NULL;          \
	dgMeshEffect *sourceCoplanar = NULL;  \
	dgMeshEffect *leftMeshSource = NULL;  \
	dgMeshEffect *rightMeshSource = NULL; \
	dgMeshEffect *clipperCoplanar = NULL; \
	dgMeshEffect *leftMeshClipper = NULL; \
	dgMeshEffect *rightMeshClipper = NULL;

#define DG_MESG_EFFECT_BOOLEAN_FINISH()                 \
	if (sourceCoplanar) {                               \
		sourceCoplanar->Release();                      \
	}                                                   \
	if (clipperCoplanar) {                              \
		clipperCoplanar->Release();                     \
	}                                                   \
	if (leftMeshClipper) {                              \
		leftMeshClipper->Release();                     \
	}                                                   \
	if (rightMeshClipper) {                             \
		rightMeshClipper->Release();                    \
	}                                                   \
	if (leftMeshSource) {                               \
		leftMeshSource->Release();                      \
	}                                                   \
	if (rightMeshSource) {                              \
		rightMeshSource->Release();                     \
	}                                                   \
	if (result) {                                       \
		result->ConvertToPolygons();                    \
		dgStack<dgInt32> map(result->m_pointCount + 1); \
		result->RemoveUnusedVertices(&map[0]);          \
	}

class dgMeshEffect : public dgPolyhedra, public dgRefCounter {
public:
	class dgVertexAtribute {
	public:
		dgBigVector m_vertex;
		dgFloat64 m_normal_x;
		dgFloat64 m_normal_y;
		dgFloat64 m_normal_z;
		dgFloat64 m_u0;
		dgFloat64 m_v0;
		dgFloat64 m_u1;
		dgFloat64 m_v1;
		dgFloat64 m_material;

		void clear() {
			m_vertex = dgBigVector(0.0f, 0.0f, 0.0f, 0.0f);
			m_normal_x = 0.0f;
			m_normal_y = 0.0f;
			m_normal_z = 0.0f;
			m_u0 = 0.0f;
			m_v0 = 0.0f;
			m_u1 = 0.0f;
			m_v1 = 0.0f;
			m_material = 0.0f;
		}
	};

	class dgIndexArray {
	public:
		dgInt32 m_materialCount;
		dgInt32 m_indexCount;
		dgInt32 m_materials[256];
		dgInt32 m_materialsIndexCount[256];
		dgInt32 *m_indexList;
	};

	dgMeshEffect(dgMemoryAllocator *const allocator, bool preAllocaBuffers);
	dgMeshEffect(dgCollision *const collision);
	dgMeshEffect(const dgMeshEffect &source);
	dgMeshEffect(dgPolyhedra &mesh, const dgMeshEffect &source);

	// Create a convex hull Mesh form point cloud
	dgMeshEffect(dgMemoryAllocator *const allocator, const dgFloat64 *const vertexCloud, dgInt32 count, dgInt32 strideInByte, dgFloat64 distTol);

	// create a convex approximation
	dgMeshEffect(const dgMeshEffect &source, dgFloat32 maxConcavity, dgInt32 maxCount = 32);

	// create a planar Mesh
	dgMeshEffect(dgMemoryAllocator *const allocator, const dgMatrix &planeMatrix, dgFloat32 witdth, dgFloat32 breadth, dgInt32 material, const dgMatrix &textureMatrix0, const dgMatrix &textureMatrix1);
	virtual ~dgMeshEffect(void);

	void ApplyTransform(const dgMatrix &matrix);
	dgMatrix CalculateOOBB(dgBigVector &size) const;
	void CalculateAABB(dgBigVector &min, dgBigVector &max) const;

	void CalculateNormals(dgFloat64 angleInRadians);
	void SphericalMapping(dgInt32 material);
	void BoxMapping(dgInt32 front, dgInt32 side, dgInt32 top);
	void UniformBoxMapping(dgInt32 material, const dgMatrix &textruMatrix);
	void CylindricalMapping(dgInt32 cylinderMaterial, dgInt32 capMaterial);

	dgEdge *InsertEdgeVertex(dgEdge *const edge, dgFloat64 param);

	dgMeshEffect *Union(const dgMatrix &matrix, const dgMeshEffect *const clip) const;
	dgMeshEffect *Difference(const dgMatrix &matrix, const dgMeshEffect *const clip) const;
	dgMeshEffect *Intersection(const dgMatrix &matrix, const dgMeshEffect *const clip) const;
	void ClipMesh(const dgMatrix &matrix, const dgMeshEffect *const clip, dgMeshEffect **const top, dgMeshEffect **const bottom) const;

	bool CheckIntersection(const dgMeshEffectSolidTree *const solidTree, dgFloat64 scale) const;
	dgMeshEffectSolidTree *CreateSolidTree() const;
	static void DestroySolidTree(dgMeshEffectSolidTree *const tree);
	static bool CheckIntersection(const dgMeshEffect *const meshA, const dgMeshEffectSolidTree *const solidTreeA,
	                              const dgMeshEffect *const meshB, const dgMeshEffectSolidTree *const solidTreeB, dgFloat64 scale);

	dgMeshEffect *GetFirstLayer() const;
	dgMeshEffect *GetNextLayer(const dgMeshEffect *const layer) const;

	void Triangulate();
	void ConvertToPolygons();

	void RemoveUnusedVertices(dgInt32 *const vertexRemapTable);

	void BeginPolygon();
	void AddPolygon(dgInt32 count, const dgFloat32 *const vertexList, dgInt32 stride, dgInt32 material);
#ifndef __USE_DOUBLE_PRECISION__
	void AddPolygon(dgInt32 count, const dgFloat64 *const vertexList, dgInt32 stride, dgInt32 material);
#endif
	void EndPolygon(dgFloat64 tol);

	void PackVertexArrays();

	void BuildFromVertexListIndexList(dgInt32 faceCount, const dgInt32 *const faceIndexCount, const dgInt32 *const faceMaterialIndex,
	                                  const dgFloat32 *const vertex, dgInt32 vertexStrideInBytes, const dgInt32 *const vertexIndex,
	                                  const dgFloat32 *const normal, dgInt32 normalStrideInBytes, const dgInt32 *const normalIndex,
	                                  const dgFloat32 *const uv0, dgInt32 uv0StrideInBytes, const dgInt32 *const uv0Index,
	                                  const dgFloat32 *const uv1, dgInt32 uv1StrideInBytes, const dgInt32 *const uv1Index);

	dgInt32 GetVertexCount() const;
	dgInt32 GetVertexStrideInByte() const;
	dgFloat64 *GetVertexPool() const;

	dgInt32 GetPropertiesCount() const;
	dgInt32 GetPropertiesStrideInByte() const;
	dgFloat64 *GetAttributePool() const;
	dgFloat64 *GetNormalPool() const;
	dgFloat64 *GetUV0Pool() const;
	dgFloat64 *GetUV1Pool() const;

	dgEdge *ConectVertex(dgEdge *const e0, dgEdge *const e1);

	dgInt32 GetTotalFaceCount() const;
	dgInt32 GetTotalIndexCount() const;
	void GetFaces(dgInt32 *const faceCount, dgInt32 *const materials, void **const faceNodeList) const;

	void RepairTJoints(bool triangulate);
	bool SeparateDuplicateLoops(dgEdge *const edge);
	bool HasOpenEdges() const;
	dgFloat64 CalculateVolume() const;

	void GetVertexStreams(dgInt32 vetexStrideInByte, dgFloat32 *const vertex,
	                      dgInt32 normalStrideInByte, dgFloat32 *const normal,
	                      dgInt32 uvStrideInByte0, dgFloat32 *const uv0,
	                      dgInt32 uvStrideInByte1, dgFloat32 *const uv1) const;

	void GetIndirectVertexStreams(dgInt32 vetexStrideInByte, dgFloat64 *const vertex, dgInt32 *const vertexIndices, dgInt32 *const vertexCount,
	                              dgInt32 normalStrideInByte, dgFloat64 *const normal, dgInt32 *const normalIndices, dgInt32 *const normalCount,
	                              dgInt32 uvStrideInByte0, dgFloat64 *const uv0, dgInt32 *const uvIndices0, dgInt32 *const uvCount0,
	                              dgInt32 uvStrideInByte1, dgFloat64 *const uv1, dgInt32 *const uvIndices1, dgInt32 *const uvCount1);

	dgIndexArray *MaterialGeometryBegin() const;
	void MaterialGeomteryEnd(dgIndexArray *const handle) const;
	dgInt32 GetFirstMaterial(dgIndexArray *const handle) const;
	dgInt32 GetNextMaterial(dgIndexArray *const handle, dgInt32 materialHandle) const;
	dgInt32 GetMaterialID(dgIndexArray *const handle, dgInt32 materialHandle) const;
	dgInt32 GetMaterialIndexCount(dgIndexArray *const handle, dgInt32 materialHandle) const;
	void GetMaterialGetIndexStream(dgIndexArray *const handle, dgInt32 materialHandle, dgInt32 *const index) const;
	void GetMaterialGetIndexStreamShort(dgIndexArray *const handle, dgInt32 materialHandle, dgInt16 *const index) const;

	dgCollision *CreateCollisionTree(dgInt32 shapeID) const;
	dgCollision *CreateConvexCollision(dgFloat64 tolerance, dgInt32 shapeID, const dgMatrix &matrix = dgGetIdentityMatrix()) const;

	dgMeshEffect *CreateConvexApproximation(dgFloat32 maxConcavity, dgInt32 maxCount = 32) const;
	dgMeshEffect *CreateDelanayTretrahedralization(dgInt32 interionMaterial, dgMatrix &matrix) const;
	dgMeshEffect *CreateVoronoiPartition(dgInt32 pointsCount, dgInt32 pointStrideInBytes, const dgFloat32 *const pointCloud, dgInt32 interionMaterial, dgMatrix &matrix) const;

	void PlaneClipMesh(const dgMatrix &planeMatrix, const dgMatrix &planeTextMatrix, dgInt32 planeMaterial, dgMeshEffect **const leftMeshSource, dgMeshEffect **const rightMeshSource) const;

	dgVertexAtribute &GetAttribute(dgInt32 index) const;
	void TransformMesh(const dgMatrix &matrix);

	void *GetFirstVertex() const;
	void *GetNextVertex(void *const vertex) const;
	dgInt32 GetVertexIndex(void *const vertex) const;

	void *GetFirstPoint() const;
	void *GetNextPoint(void *const point) const;
	dgInt32 GetPointIndex(const void *const point) const;
	dgInt32 GetVertexIndexFromPoint(void *const point) const;

	void *GetFirstEdge() const;
	void *GetNextEdge(void *const edge) const;
	void GetEdgeIndex(const void *const edge, dgInt32 &v0, dgInt32 &v1) const;
	//  void GetEdgeAttributeIndex (const void* edge, dgInt32& v0, dgInt32& v1) const;

	void *GetFirstFace() const;
	void *GetNextFace(void *const face) const;
	dgInt32 IsFaceOpen(const void *const face) const;
	dgInt32 GetFaceMaterial(const void *const face) const;
	dgInt32 GetFaceIndexCount(const void *const face) const;
	void GetFaceIndex(const void *const face, int *const indices) const;
	void GetFaceAttributeIndex(const void *const face, int *const indices) const;

	bool Sanity() const;

protected:
	void Init(bool preAllocaBuffers);
	dgBigVector GetOrigin() const;
	dgInt32 CalculateMaxAttributes() const;
	dgFloat64 QuantizeCordinade(dgFloat64 val) const;
	dgInt32 EnumerateAttributeArray(dgVertexAtribute *const attib);
	void ApplyAttributeArray(dgVertexAtribute *const attib, dgInt32 maxCount);
	void AddVertex(const dgBigVector &vertex);
	void AddAtribute(const dgVertexAtribute &attib);
	void AddPoint(const dgFloat64 *vertexList, dgInt32 material);
	void FixCylindricalMapping(dgVertexAtribute *const attib) const;

	void MergeFaces(const dgMeshEffect *const source);
	void ReverseMergeFaces(dgMeshEffect *const source);
	dgVertexAtribute InterpolateEdge(dgEdge *const edge, dgFloat64 param) const;
	dgVertexAtribute InterpolateVertex(const dgBigVector &point, dgEdge *const face) const;

	dgMeshEffect *GetNextLayer(dgInt32 mark) const;

	void FilterCoplanarFaces(const dgMeshEffect *const otherCap, dgFloat32 sign);
	void ClipMesh(const dgMeshEffect *const clipMesh, dgMeshEffect **const back, dgMeshEffect **const front, dgMeshEffect **const coplanar) const;
	void ClipMesh(const dgMeshEffectSolidTree *const clipper, dgMeshEffect **const back, dgMeshEffect **const front, dgMeshEffect **const coplanar) const;

	dgInt32 PlaneApplyCap(const dgMeshEffect *planeMesh, const dgBigPlane &normal);
	void PlaneClipMesh(const dgMeshEffect *planeMesh, dgMeshEffect **leftMeshSource, dgMeshEffect **rightMeshSource) const;

	dgMeshEffect *MakeDelanayIntersection(dgMeshEffectSolidTree *const tree, dgBigVector *const points, dgInt32 count, dgInt32 materialId, const dgMatrix &textureProjectionMatrix, dgFloat32 normalAngleInRadians) const;

	bool CheckSingleMesh() const;

	dgInt32 m_pointCount;
	dgInt32 m_maxPointCount;

	dgInt32 m_atribCount;
	dgInt32 m_maxAtribCount;

	dgBigVector *m_points;
	dgVertexAtribute *m_attib;

	friend class dgConvexHull3d;
	friend class dgConvexHull4d;
	friend class dgMeshTreeCSGFace;
	friend class dgMeshEffectSolidTree;
};

inline dgInt32 dgMeshEffect::GetVertexCount() const {
	return m_pointCount;
}

inline dgInt32 dgMeshEffect::GetPropertiesCount() const {
	return m_atribCount;
}

inline dgInt32 dgMeshEffect::GetMaterialID(dgIndexArray *const handle, dgInt32 materialHandle) const {
	return handle->m_materials[materialHandle];
}

inline dgInt32 dgMeshEffect::GetMaterialIndexCount(dgIndexArray *const handle, dgInt32 materialHandle) const {
	return handle->m_materialsIndexCount[materialHandle];
}

inline dgMeshEffect::dgVertexAtribute &dgMeshEffect::GetAttribute(dgInt32 index) const {
	return m_attib[index];
}

inline dgInt32 dgMeshEffect::GetPropertiesStrideInByte() const {
	return sizeof(dgVertexAtribute);
}

inline dgFloat64 *dgMeshEffect::GetAttributePool() const {
	return &m_attib->m_vertex.m_x;
}

inline dgFloat64 *dgMeshEffect::GetNormalPool() const {
	return &m_attib->m_normal_x;
}

inline dgFloat64 *dgMeshEffect::GetUV0Pool() const {
	return &m_attib->m_u0;
}

inline dgFloat64 *dgMeshEffect::GetUV1Pool() const {
	return &m_attib->m_u1;
}

inline bool dgMeshEffect::CheckIntersection(const dgMeshEffect *const meshA, const dgMeshEffectSolidTree *const solidTreeA, const dgMeshEffect *const meshB, const dgMeshEffectSolidTree *const solidTreeB, dgFloat64 scale) {
	return (meshA->CheckIntersection(solidTreeB, scale) || meshB->CheckIntersection(solidTreeA, scale));
}

inline dgInt32 dgMeshEffect::GetVertexStrideInByte() const {
	return sizeof(dgBigVector);
}

inline dgFloat64 *dgMeshEffect::GetVertexPool() const {
	return &m_points[0].m_x;
}

inline dgMeshEffect *dgMeshEffect::GetFirstLayer() const {
	return GetNextLayer(IncLRU());
}

inline dgMeshEffect *dgMeshEffect::GetNextLayer(const dgMeshEffect *const layerSegment) const {
	if (!layerSegment) {
		return NULL;
	}
	return GetNextLayer(layerSegment->IncLRU() - 1);
}

inline dgFloat64 dgMeshEffect::QuantizeCordinade(dgFloat64 x) const {
	int exp;
	dgFloat64 mantissa = frexp(x, &exp);
	mantissa = DG_MESH_EFFECT_PRECISION_SCALE_INV * floor(mantissa * DG_MESH_EFFECT_PRECISION_SCALE);

	dgFloat64 x1 = ldexp(mantissa, exp);
	return x1;
}

#endif
