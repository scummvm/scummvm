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

#include "hpl1/engine/libraries/newton/core/dg.h"

#include "dgBodyMasterList.h"
#include "dgCollision.h"
#include "dgCollisionBVH.h"
#include "dgCollisionBox.h"
#include "dgCollisionCapsule.h"
#include "dgCollisionChamferCylinder.h"
#include "dgCollisionCompound.h"
#include "dgCollisionCompoundBreakable.h"
#include "dgCollisionCone.h"
#include "dgCollisionConvexHull.h"
#include "dgCollisionConvexModifier.h"
#include "dgCollisionCylinder.h"
#include "dgCollisionEllipse.h"
#include "dgCollisionHeightField.h"
#include "dgCollisionNull.h"
#include "dgCollisionScene.h"
#include "dgCollisionSphere.h"
#include "dgCollisionUserMesh.h"
#include "dgWorld.h"
#include "dgWorldDynamicUpdate.h"


#define DG_USE_CACHE_CONTACTS

#define DG_PRUNE_PADDING_BYTES 128

//#define DG_CONTACT_CACHE_TOLERANCE dgFloat32 (1.0e-6f)
//#define DG_REDUCE_CONTACT_TOLERANCE dgFloat32 (1.0e-2f)
//#define DG_PRUNE_CONTACT_TOLERANCE dgFloat32 (1.0e-2f)

#define DG_COMPOUND_MAX_SORT_ARRAY (1024 * 2)

dgCollision *dgWorld::CreateNull() {
	dgUnsigned32 crc;
	dgCollision *collision;
	dgBodyCollisionList::dgTreeNode *node;

	crc = dgCollision::dgCollisionNull_RTTI;

	node = dgBodyCollisionList::Find(crc);
	if (!node) {
		collision = new (m_allocator) dgCollisionNull(m_allocator, crc);
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateBox(dgFloat32 dx, dgFloat32 dy, dgFloat32 dz,
								dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	//	dgUnsigned32 crc;
	//	dgCollision* collision;
	//	dgBodyCollisionList::dgTreeNode* node;
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	dx = dgAbsf(dx);
	dy = dgAbsf(dy);
	dz = dgAbsf(dz);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_boxCollision;
	buffer[1] = dgCollision::Quantize(dx * dgFloat32(0.5f));
	buffer[2] = dgCollision::Quantize(dy * dgFloat32(0.5f));
	buffer[3] = dgCollision::Quantize(dz * dgFloat32(0.5f));
	buffer[4] = dgUnsigned32(shapeID);
	memcpy(&buffer[5], &offsetMatrix, sizeof(dgMatrix));
	dgUnsigned32 crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	dgBodyCollisionList::dgTreeNode *node = dgBodyCollisionList::Find(crc);
	if (!node) {
		dgCollision *const collision = new (m_allocator) dgCollisionBox(m_allocator,
																		crc, dx, dy, dz, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateSphere(dgFloat32 radii, dgInt32 shapeID,
								   const dgMatrix &offsetMatrix) {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];
	radii = dgAbsf(radii);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_sphereCollision;
	buffer[1] = dgCollision::Quantize(radii);
	buffer[2] = dgUnsigned32(shapeID);
	memcpy(&buffer[3], &offsetMatrix, sizeof(dgMatrix));
	dgUnsigned32 crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	dgBodyCollisionList::dgTreeNode *node = dgBodyCollisionList::Find(crc);
	if (!node) {
		dgCollision *const collision = new (m_allocator) dgCollisionSphere(
			m_allocator, crc, radii, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateEllipse(dgFloat32 rx, dgFloat32 ry, dgFloat32 rz,
									dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	rx = dgAbsf(rx);
	ry = dgAbsf(ry);
	rz = dgAbsf(rz);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_ellipseCollision;
	buffer[1] = dgCollision::Quantize(rx);
	buffer[2] = dgCollision::Quantize(ry);
	buffer[3] = dgCollision::Quantize(rz);
	buffer[4] = dgUnsigned32(shapeID);
	memcpy(&buffer[5], &offsetMatrix, sizeof(dgMatrix));
	dgUnsigned32 crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	dgBodyCollisionList::dgTreeNode *node = dgBodyCollisionList::Find(crc);
	if (!node) {
		dgCollision *const collision = new (m_allocator) dgCollisionEllipse(
			m_allocator, crc, rx, ry, rz, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateCapsule(dgFloat32 radius, dgFloat32 height,
									dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgUnsigned32 crc;
	dgCollision *collision;
	dgBodyCollisionList::dgTreeNode *node;
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	radius = dgAbsf(radius);
	height = dgAbsf(height);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_capsuleCollision;
	buffer[1] = dgCollision::Quantize(radius);
	buffer[2] = dgCollision::Quantize(height * dgFloat32(0.5f));
	buffer[3] = dgUnsigned32(shapeID);
	memcpy(&buffer[4], &offsetMatrix, sizeof(dgMatrix));
	crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	node = dgBodyCollisionList::Find(crc);
	if (!node) {
		collision = new (m_allocator) dgCollisionCapsule(m_allocator, crc, radius,
														 height, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateCone(dgFloat32 radius, dgFloat32 height,
								 dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgUnsigned32 crc;
	dgCollision *collision;
	dgBodyCollisionList::dgTreeNode *node;
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	radius = dgAbsf(radius);
	height = dgAbsf(height);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_coneCollision;
	buffer[1] = dgCollision::Quantize(radius);
	buffer[2] = dgCollision::Quantize(height * dgFloat32(0.5f));
	buffer[3] = dgUnsigned32(shapeID);
	memcpy(&buffer[4], &offsetMatrix, sizeof(dgMatrix));
	crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	node = dgBodyCollisionList::Find(crc);
	if (!node) {
		collision = new (m_allocator) dgCollisionCone(m_allocator, crc, radius,
													  height, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateCylinder(dgFloat32 radius, dgFloat32 height,
									 dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgUnsigned32 crc;
	dgCollision *collision;
	dgBodyCollisionList::dgTreeNode *node;
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	radius = dgAbsf(radius);
	height = dgAbsf(height);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_cylinderCollision;
	buffer[1] = dgCollision::Quantize(radius);
	buffer[2] = dgCollision::Quantize(height * dgFloat32(0.5f));
	buffer[3] = dgUnsigned32(shapeID);
	memcpy(&buffer[4], &offsetMatrix, sizeof(dgMatrix));
	crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	node = dgBodyCollisionList::Find(crc);
	if (!node) {
		collision = new (m_allocator) dgCollisionCylinder(m_allocator, crc, radius,
														  height, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateChamferCylinder(dgFloat32 radius, dgFloat32 height,
											dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgUnsigned32 crc;
	dgCollision *collision;
	dgBodyCollisionList::dgTreeNode *node;
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	radius = dgAbsf(radius);
	height = dgAbsf(height);

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_chamferCylinderCollision;
	buffer[1] = dgCollision::Quantize(radius);
	buffer[2] = dgCollision::Quantize(height * dgFloat32(0.5f));
	buffer[3] = dgUnsigned32(shapeID);
	memcpy(&buffer[4], &offsetMatrix, sizeof(dgMatrix));
	crc = dgCollision::MakeCRC(buffer, sizeof(buffer));

	node = dgBodyCollisionList::Find(crc);
	if (!node) {
		collision = new (m_allocator) dgCollisionChamferCylinder(m_allocator, crc,
																 radius, height, offsetMatrix);
		collision->SetUserDataID(dgUnsigned32(shapeID));
		node = dgBodyCollisionList::Insert(collision, crc);
	}

	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateConvexHull(dgInt32 count,
									   const dgFloat32 *vertexArray, dgInt32 strideInBytes, dgFloat32 tolerance,
									   dgInt32 shapeID, const dgMatrix &offsetMatrix) {
	dgStack<dgUnsigned32> buffer(
		2 + 3 * count + dgInt32(sizeof(dgMatrix) / sizeof(dgInt32)));

	// create crc signature for cache lookup
	memset(&buffer[0], 0, size_t(buffer.GetSizeInBytes()));
	buffer[0] = m_convexHullCollision;
	buffer[1] = dgUnsigned32(shapeID);
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));
	for (dgInt32 i = 0; i < count; i++) {
		buffer[2 + i * 3 + 0] = dgCollision::Quantize(vertexArray[i * stride + 0]);
		buffer[2 + i * 3 + 1] = dgCollision::Quantize(vertexArray[i * stride + 1]);
		buffer[2 + i * 3 + 2] = dgCollision::Quantize(vertexArray[i * stride + 2]);
	}
	memcpy(&buffer[2 + count * 3], &offsetMatrix, sizeof(dgMatrix));
	dgUnsigned32 crc = dgCollision::MakeCRC(&buffer[0], buffer.GetSizeInBytes());

	// find the shape in cache
	dgBodyCollisionList::dgTreeNode *node = dgBodyCollisionList::Find(crc);
	if (!node) {
		// chape not found crate a new one and add to teh cache
		dgCollisionConvexHull *const collision =
			new (m_allocator) dgCollisionConvexHull(m_allocator, crc, count,
													strideInBytes, tolerance, vertexArray, offsetMatrix);
		if (collision->GetVertexCount()) {
			collision->SetUserDataID(dgUnsigned32(shapeID));
			node = dgBodyCollisionList::Insert(collision, crc);
		} else {
			/*
			 // hack to save the data that make convex hull fail
			 char xxx[32];
			 static int aaa  ;
			 snprintf (xxx, 32, "file1__%d.txt", aaa);
			 aaa ++;
			 FILE * file = fopen (xxx, "wb");
			 fprintf (file, "float data[][3] = {\n");
			 for (dgInt32 i = 0; i < count; i ++) {
			 fprintf (file, "{%f, %f, %f},\n", vertexArray[i * stride + 0], vertexArray[i * stride + 1], vertexArray[i * stride + 2]);
			 }
			 fprintf (file, "};\n");
			 fclose (file);
			 */
			_ASSERTE(0);
			// could not make the shape destroy the shell and return NULL
			// note this is the only newton shape that can return NULL;
			collision->Release();
			return NULL;
		}
	}

	// add reference to teh shape and return the collision pointer
	node->GetInfo()->AddRef();
	return node->GetInfo();
}

dgCollision *dgWorld::CreateConvexModifier(dgCollision *convexCollision) {
	dgCollision *collision;

	collision = NULL;
	if (convexCollision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		//		if (!convexCollision->IsType (dgCollision::dgCollisionCompound_RTTI) && !convexCollision->IsType (m_nullType)) {
		if (!convexCollision->IsType(dgCollision::dgCollisionNull_RTTI)) {
			collision = new (m_allocator) dgCollisionConvexModifier(
				(dgCollisionConvex *)convexCollision, this);
		}
	}

	return collision;
}

dgCollision *dgWorld::CreateCollisionCompound(dgInt32 count,
											  dgCollision *const array[]) {
	// compound collision are not cached
	return new (m_allocator) dgCollisionCompound(count,
												 (dgCollisionConvex **)array, this);
}

// dgCollision* dgWorld::CreateCollisionCompoundBreakable (
//	dgInt32 count,
//	dgMeshEffect* const solidArray[],
//	dgMeshEffect* const splitePlanes[],
//  dgMatrix* const matrixArray,
//	dgInt32* const idArray,
//	dgFloat32* const densities,
//	dgInt32 debriID,
//	dgCollisionCompoundBreakableCallback callback,
//	void* buildUsedData)
dgCollision *dgWorld::CreateCollisionCompoundBreakable(dgInt32 count,
													   const dgMeshEffect *const solidArray[], const dgInt32 *const idArray,
													   const dgFloat32 *const densities, const dgInt32 *const internalFaceMaterial,
													   dgInt32 debriID, dgFloat32 gap) {
	//	return new dgCollisionCompoundBreakable (count, solidArray, splitePlanes, matrixArray, idArray, densities, debriID, callback, buildUsedData, this);
	return new (m_allocator) dgCollisionCompoundBreakable(count, solidArray,
														  idArray, densities, internalFaceMaterial, debriID, gap, this);
}

dgCollision *dgWorld::CreateBVH() {
	// collision tree are not cached
	return new (m_allocator) dgCollisionBVH(m_allocator);
}

dgCollision *dgWorld::CreateStaticUserMesh(const dgVector &boxP0,
										   const dgVector &boxP1, const dgUserMeshCreation &data) {
	return new (m_allocator) dgCollisionUserMesh(m_allocator, boxP0, boxP1, data);
}

dgCollision *dgWorld::CreateBVHFieldCollision(dgInt32 width, dgInt32 height,
											  dgInt32 contructionMode, const dgUnsigned16 *const elevationMap,
											  const dgInt8 *const atributeMap, dgFloat32 horizontalScale,
											  dgFloat32 vertcalScale) {
	return new (m_allocator) dgCollisionHeightField(this, width, height,
													contructionMode, elevationMap, atributeMap, horizontalScale, vertcalScale);
}

dgCollision *dgWorld::CreateScene() {
	return new (m_allocator) dgCollisionScene(this);
}

void dgWorld::Serialize(dgCollision *shape, dgSerialize serialization,
						void *const userData) const {
	dgInt32 signature[4];

	signature[0] = shape->GetCollisionPrimityType();
	signature[1] = dgInt32(shape->GetSignature());
	signature[2] = 0;
	signature[3] = 0;
	serialization(userData, signature, sizeof(signature));

	shape->Serialize(serialization, userData);

	dgInt32 end = SERIALIZE_END;
	serialization(userData, &end, sizeof(dgInt32));
}

// dgCollision* dgWorld::CreateFromSerialization (dgInt32 signature, dgCollisionID type, dgDeserialize deserialization, void* const userData)
dgCollision *dgWorld::CreateFromSerialization(dgDeserialize deserialization,
											  void *const userData) {
	dgInt32 signature[4];

	deserialization(userData, signature, sizeof(signature));

	dgBodyCollisionList::dgTreeNode *node = dgBodyCollisionList::Find(
		dgUnsigned32(signature[1]));
	dgCollision *collision = NULL;
	if (node) {
		collision = node->GetInfo();
		collision->AddRef();

	} else {
		switch (signature[0]) {
		case m_sceneCollision: {
			collision = new (m_allocator) dgCollisionScene(this, deserialization,
														   userData);
			break;
		}

		case m_heightField: {
			collision = new (m_allocator) dgCollisionHeightField(this,
																 deserialization, userData);
			break;
		}
		case m_boundingBoxHierachy: {
			collision = new (m_allocator) dgCollisionBVH(this, deserialization,
														 userData);
			break;
		}

			// if (!stricmp (type, "box"))
		case m_boxCollision: {
			collision = new (m_allocator) dgCollisionBox(this, deserialization,
														 userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "cone")) {
		case m_coneCollision: {
			collision = new (m_allocator) dgCollisionCone(this, deserialization,
														  userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "sphere_1")) {
		case m_ellipseCollision: {
			collision = new (m_allocator) dgCollisionEllipse(this, deserialization,
															 userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "sphere")) {
		case m_sphereCollision: {
			collision = new (m_allocator) dgCollisionSphere(this, deserialization,
															userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "cylinder")) {
		case m_cylinderCollision: {
			collision = new (m_allocator) dgCollisionCylinder(this, deserialization,
															  userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "capsule")) {
		case m_capsuleCollision: {
			collision = new (m_allocator) dgCollisionCapsule(this, deserialization,
															 userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

			//} else if (!stricmp (type, "compound")) {
		case m_compoundCollision: {
			collision = new (m_allocator) dgCollisionCompound(this, deserialization,
															  userData);
			break;
		}

		case m_compoundBreakable: {
			collision = new (m_allocator) dgCollisionCompoundBreakable(this,
																	   deserialization, userData);
			break;
		}

			//} else if (!stricmp (type, "convexHull")) {
		case m_convexHullCollision: {
			collision = new (m_allocator) dgCollisionConvexHull(this, deserialization,
																userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

		case m_convexConvexIntance: {
			collision =
				new (m_allocator) dgCollisionCompoundBreakable::dgCollisionConvexIntance(
					this, deserialization, userData);
			break;
		}

			//} else if (!stricmp (type, "chamferCylinder")) {
		case m_chamferCylinderCollision: {
			collision = new (m_allocator) dgCollisionChamferCylinder(this,
																	 deserialization, userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

		case m_nullCollision: {
			collision = new (m_allocator) dgCollisionNull(this, deserialization,
														  userData);
			node = dgBodyCollisionList::Insert(collision, collision->GetSignature());
			collision->AddRef();
			break;
		}

		case m_convexCollisionModifier: {
			collision = new (m_allocator) dgCollisionConvexModifier(this,
																	deserialization, userData);
			break;
		}

		default:
			_ASSERTE(0);
		}
	}

	dgInt32 endMarker;
	do {
		deserialization(userData, &endMarker, sizeof(dgInt32));
	} while (endMarker != SERIALIZE_END);
	return collision;
}

dgContactMaterial *dgWorld::GetMaterial(dgUnsigned32 bodyGroupId0,
										dgUnsigned32 bodyGroupId1) const {
	if (bodyGroupId0 > bodyGroupId1) {
		Swap(bodyGroupId0, bodyGroupId1);
	}

	dgUnsigned32 key = (bodyGroupId1 << 16) + bodyGroupId0;
	dgBodyMaterialList::dgTreeNode *const node = dgBodyMaterialList::Find(key);

	return node ? &node->GetInfo() : NULL;
}

dgContactMaterial *dgWorld::GetFirstMaterial() const {
	dgBodyMaterialList::dgTreeNode *const node = dgBodyMaterialList::Minimum();
	_ASSERTE(node);
	return &node->GetInfo();
}

dgContactMaterial *dgWorld::GetNextMaterial(dgContactMaterial *material) const {
	dgBodyMaterialList::dgTreeNode *const thisNode =
		dgBodyMaterialList::GetNodeFromInfo(*material);
	_ASSERTE(thisNode);
	dgBodyMaterialList::dgTreeNode *const node =
		(dgBodyMaterialList::dgTreeNode *)thisNode->Next();
	if (node) {
		return &node->GetInfo();
	}

	return NULL;
}

dgUnsigned32 dgWorld::GetDefualtBodyGroupID() const {
	return m_defualtBodyGroupID;
}

dgUnsigned32 dgWorld::CreateBodyGroupID() {
	dgContactMaterial pairMaterial;

	pairMaterial.m_aabbOverlap = NULL;
	pairMaterial.m_contactPoint = NULL;
	pairMaterial.m_compoundAABBOverlap = NULL;

	dgUnsigned32 newId = m_bodyGroupID;
	m_bodyGroupID += 1;
	for (dgUnsigned32 i = 0; i < m_bodyGroupID; i++) {
		dgUnsigned32 key = (newId << 16) + i;

		dgBodyMaterialList::Insert(pairMaterial, key);
	}

	return newId;
}

void dgWorld::RemoveFromCache(dgCollision *const collision) {
	dgBodyCollisionList::dgTreeNode *const node = dgBodyCollisionList::Find(
		collision->m_signature);
	if (node) {
		collision->m_signature = 0xffffffff;
		_ASSERTE(node->GetInfo() == collision);
		collision->Release();
		dgBodyCollisionList::Remove(node);
	}
}

void dgWorld::ReleaseCollision(dgCollision *const collision) {
	if (m_destroyCollision) {
		if (collision->GetRefCount() == 1) {
			m_destroyCollision(this, collision);
		}
	}

	dgInt32 ref = collision->Release();
	if (ref == 1) {
		dgBodyCollisionList::dgTreeNode *const node = dgBodyCollisionList::Find(
			collision->m_signature);
		if (node) {
			_ASSERTE(node->GetInfo() == collision);
			if (m_destroyCollision) {
				m_destroyCollision(this, collision);
			}
			collision->Release();
			dgBodyCollisionList::Remove(node);
		}
	}
}

// ********************************************************************************
//
// separate collision system
//
// ********************************************************************************
dgInt32 dgWorld::ClosestPoint(dgTriplex &point, dgCollision *const collision,
							  const dgMatrix &matrix, dgTriplex &contact, dgTriplex &normal,
							  dgInt32 threadIndex) const {
	dgTriplex contactA;
	dgMatrix pointMatrix(dgGetIdentityMatrix());

	contact = point;
	pointMatrix.m_posit.m_x = point.m_x;
	pointMatrix.m_posit.m_y = point.m_y;
	pointMatrix.m_posit.m_z = point.m_z;
	return ClosestPoint(collision, matrix, m_pointCollision, pointMatrix, contact,
						contactA, normal, threadIndex);
}

dgInt32 dgWorld::ClosestPoint(dgCollision *const collisionA,
							  const dgMatrix &matrixA, dgCollision *const collisionB,
							  const dgMatrix &matrixB, dgTriplex &contactA, dgTriplex &contactB,
							  dgTriplex &normalAB, dgInt32 threadIndex) const {
	dgBody collideBodyA;
	dgBody collideBodyB;

	collideBodyA.m_matrix = matrixA;
	collideBodyA.m_collision = collisionA;
	collideBodyA.m_collisionWorldMatrix = collisionA->m_offset * matrixA;

	collideBodyB.m_matrix = matrixB;
	collideBodyB.m_collision = collisionB;
	collideBodyB.m_collisionWorldMatrix = collisionB->m_offset * matrixB;

	if (collisionA->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		return ClosestCompoundPoint(&collideBodyA, &collideBodyB, contactA,
									contactB, normalAB, threadIndex);
	} else if (collisionB->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		dgInt32 flag;
		flag = ClosestCompoundPoint(&collideBodyB, &collideBodyA, contactB,
									contactA, normalAB, threadIndex);
		normalAB.m_x *= dgFloat32(-1.0f);
		normalAB.m_y *= dgFloat32(-1.0f);
		normalAB.m_z *= dgFloat32(-1.0f);
		return flag;

	} else if (collisionA->IsType(dgCollision::dgConvexCollision_RTTI) && collisionB->IsType(dgCollision::dgConvexCollision_RTTI)) {
		dgInt32 flag;
		dgCollisionParamProxy proxy(threadIndex);
		dgContactPoint contacts[16];

		proxy.m_referenceBody = &collideBodyA;
		proxy.m_referenceCollision = collideBodyA.m_collision;
		proxy.m_referenceMatrix = collideBodyA.m_collisionWorldMatrix;

		proxy.m_floatingBody = &collideBodyB;
		proxy.m_floatingCollision = collideBodyB.m_collision;
		proxy.m_floatingMatrix = collideBodyB.m_collisionWorldMatrix;

		proxy.m_timestep = dgFloat32(0.0f);
		proxy.m_penetrationPadding = dgFloat32(0.0f);
		proxy.m_unconditionalCast = 1;
		proxy.m_continueCollision = 0;
		proxy.m_maxContacts = 16;
		proxy.m_contacts = &contacts[0];
		flag = ClosestPoint(proxy);

		if (flag) {
			contactA.m_x = contacts[0].m_point.m_x;
			contactA.m_y = contacts[0].m_point.m_y;
			contactA.m_z = contacts[0].m_point.m_z;

			contactB.m_x = contacts[1].m_point.m_x;
			contactB.m_y = contacts[1].m_point.m_y;
			contactB.m_z = contacts[1].m_point.m_z;

			normalAB.m_x = contacts[0].m_normal.m_x;
			normalAB.m_y = contacts[0].m_normal.m_y;
			normalAB.m_z = contacts[0].m_normal.m_z;
		}
		return flag;
	}

	return 0;
}

dgInt32 dgWorld::ClosestCompoundPoint(dgBody *const compoundConvexA,
									  dgBody *const collisionB, dgTriplex &contactA, dgTriplex &contactB,
									  dgTriplex &normalAB, dgInt32 threadIndex) const {
	dgCollisionCompound *const collision =
		(dgCollisionCompound *)compoundConvexA->m_collision;
	_ASSERTE(collision->IsType(dgCollision::dgCollisionCompound_RTTI));
	return collision->ClosestDitance(compoundConvexA, contactA, collisionB,
									 contactB, normalAB);

	/*
	 dgInt32 retFlag;
	 dgInt32 count;
	 dgMatrix* collisionMatrixArray;
	 dgCollisionConvex** collisionArray;
	 dgCollisionCompound *compoundCollision;

	 dgContactPoint contact0;
	 dgContactPoint contact1;

	 compoundCollision = (dgCollisionCompound *) compoundConvexA->m_collision;
	 count = compoundCollision->m_count;
	 collisionArray = compoundCollision->m_array;
	 collisionMatrixArray = compoundCollision->m_collisionMatrix;

	 retFlag = 0;
	 if (collisionB->m_collision->IsType (dgCollision::dgConvexCollision_RTTI)) {
	 dgFloat32 minDist2;
	 dgCollisionParamProxy proxy(threadIndex);
	 dgContactPoint contacts[16];

	 proxy.m_referenceBody = compoundConvexA;
	 proxy.m_floatingBody = collisionB;
	 proxy.m_floatingCollision = collisionB->m_collision;
	 proxy.m_floatingMatrix = collisionB->m_collisionWorldMatrix ;

	 proxy.m_timestep = dgFloat32 (0.0f);
	 proxy.m_penetrationPadding = dgFloat32 (0.0f);
	 proxy.m_unconditionalCast = 1;
	 proxy.m_continueCollision = 0;
	 proxy.m_maxContacts = 16;
	 proxy.m_contacts = &contacts[0];

	 dgMatrix saveCollMatrix (compoundConvexA->m_collisionWorldMatrix);
	 minDist2 = dgFloat32 (1.0e10f);

	 for (dgInt32 i = 0; i < count; i ++) {
	 dgInt32 flag;

	 compoundConvexA->m_collision = collisionArray[i];
	 compoundConvexA->m_collisionWorldMatrix = collisionArray[i]->m_offset * saveCollMatrix;
	 proxy.m_referenceCollision = compoundConvexA->m_collision;
	 proxy.m_referenceMatrix = compoundConvexA->m_collisionWorldMatrix;
	 flag = ClosestPoint (proxy);
	 if (flag) {
	 dgFloat32 dist2;
	 retFlag = 1;
	 dgVector err (contacts[0].m_point - contacts[1].m_point);
	 dist2 = err % err;
	 if (dist2 < minDist2) {
	 minDist2 = dist2;
	 contact0 = contacts[0];
	 contact1 = contacts[1];
	 }
	 }
	 }

	 } else {
	 dgInt32 count1;
	 dgFloat32 minDist2;
	 dgCollisionParamProxy proxy(threadIndex);
	 dgContactPoint contacts[16];
	 dgMatrix* collisionMatrixArray1;
	 dgCollisionConvex** collisionArray1;
	 dgCollisionCompound *compoundCollision1;

	 _ASSERTE (collisionB->m_collision->IsType (dgCollision::dgCollisionCompound_RTTI));

	 dgMatrix saveCollMatrix (compoundConvexA->m_collisionWorldMatrix);
	 dgMatrix saveCollMatrix1 (collisionB->m_collisionWorldMatrix);

	 compoundCollision1 = (dgCollisionCompound *) collisionB->m_collision;
	 count1 = compoundCollision1->m_count;
	 collisionArray1 = compoundCollision1->m_array;
	 collisionMatrixArray1 = compoundCollision1->m_collisionMatrix;

	 proxy.m_referenceBody = compoundConvexA;
	 proxy.m_floatingBody = collisionB;
	 proxy.m_timestep = dgFloat32 (0.0f);
	 proxy.m_penetrationPadding = dgFloat32 (0.0f);
	 proxy.m_unconditionalCast = 1;
	 proxy.m_continueCollision = 0;
	 proxy.m_maxContacts = 16;
	 proxy.m_contacts = &contacts[0];

	 minDist2 = dgFloat32 (1.0e10f);
	 for (dgInt32 i = 0; i < count; i ++) {
	 compoundConvexA->m_collision = collisionArray[i];
	 compoundConvexA->m_collisionWorldMatrix = collisionArray[i]->m_offset * saveCollMatrix;

	 proxy.m_referenceCollision = compoundConvexA->m_collision;
	 proxy.m_referenceMatrix = compoundConvexA->m_collisionWorldMatrix;
	 for (dgInt32 j = 0; j < count1; j ++) {
	 dgInt32 flag;
	 collisionB->m_collision = collisionArray1[j];
	 collisionB->m_collisionWorldMatrix = collisionArray1[j]->m_offset * saveCollMatrix1;

	 proxy.m_floatingCollision = collisionB->m_collision;
	 proxy.m_floatingMatrix = collisionB->m_collisionWorldMatrix ;
	 flag = ClosestPoint (proxy);
	 if (flag) {
	 dgFloat32 dist2;
	 retFlag = 1;
	 dgVector err (contacts[0].m_point - contacts[1].m_point);
	 dist2 = err % err;
	 if (dist2 < minDist2) {
	 minDist2 = dist2;
	 contact0 = contacts[0];
	 contact1 = contacts[1];
	 }
	 }
	 }
	 }
	 }

	 if (retFlag) {
	 contactA.m_x = contact0.m_point.m_x;
	 contactA.m_y = contact0.m_point.m_y;
	 contactA.m_z = contact0.m_point.m_z;

	 contactB.m_x = contact1.m_point.m_x;
	 contactB.m_y = contact1.m_point.m_y;
	 contactB.m_z = contact1.m_point.m_z;

	 normalAB.m_x = contact0.m_normal.m_x;
	 normalAB.m_y = contact0.m_normal.m_y;
	 normalAB.m_z = contact0.m_normal.m_z;
	 }

	 return retFlag;
	 */
}

// **********************************************************************************
//
// dynamics collision system
//
// **********************************************************************************
static inline dgInt32 CompareContact(const dgContactPoint *const contactA,
									 const dgContactPoint *const contactB, void *dommy) {
	if (contactA->m_point[0] < contactB->m_point[0]) {
		return -1;
	} else if (contactA->m_point[0] > contactB->m_point[0]) {
		return 1;
	} else {
		return 0;
	}
}

void dgWorld::SortContacts(dgContactPoint *const contact, dgInt32 count) const {
	dgSort(contact, count, CompareContact, NULL);
}

dgInt32 dgWorld::ReduceContacts(dgInt32 count, dgContactPoint *const contact,
								dgInt32 maxCount, dgFloat32 tol, dgInt32 arrayIsSorted) const {
	if ((count > maxCount) && (maxCount > 1)) {
		//		dgInt32 index;
		//		dgInt32 countOver;
		//		dgFloat32 window;
		//		dgFloat32 window2;
		dgUnsigned8 mask[DG_MAX_CONTATCS];

		if (!arrayIsSorted) {
			SortContacts(contact, count);
		}

		dgInt32 index = 0;
		dgFloat32 window = tol;
		dgFloat32 window2 = window * window;
		dgInt32 countOver = count - maxCount;

		_ASSERTE(countOver >= 0);
		memset(mask, 0, size_t(count));
		do {
			for (dgInt32 i = 0; (i < count) && countOver; i++) {
				if (!mask[i]) {
					dgFloat32 val = contact[i].m_point[index] + window;
					for (dgInt32 j = i + 1;
						 (j < count) && countOver && (contact[j].m_point[index] < val);
						 j++) {
						if (!mask[j]) {
							dgVector dp(contact[j].m_point - contact[i].m_point);
							dgFloat32 dist2 = dp % dp;
							if (dist2 < window2) {
								mask[j] = 1;
								countOver--;
							}
						}
					}
				}
			}
			window *= dgFloat32(2.0f);
			window2 = window * window;

		} while (countOver);

		dgInt32 j = 0;
		for (dgInt32 i = 0; i < count; i++) {
			if (!mask[i]) {
				contact[j] = contact[i];
				j++;
			}
		}
		_ASSERTE(j == maxCount);

	} else {
		maxCount = count;
	}

	return maxCount;
}

dgInt32 dgWorld::PruneContacts(dgInt32 count, dgContactPoint *const contact,
							   dgInt32 maxCount) const {
	//	dgInt32 index;
	//	dgInt32 packContacts;
	//	dgFloat32 window;
	//	dgFloat32 window2;
	dgUnsigned8 mask[DG_MAX_CONTATCS];

	dgInt32 index = 0;
	dgInt32 packContacts = 0;
	dgFloat32 window = DG_PRUNE_CONTACT_TOLERANCE;
	dgFloat32 window2 = window * window;

	memset(mask, 0, size_t(count));
	SortContacts(contact, count);

	for (dgInt32 i = 0; i < count; i++) {
		if (!mask[i]) {
			dgFloat32 val = contact[i].m_point[index] + window;
			for (dgInt32 j = i + 1; (j < count) && (contact[j].m_point[index] < val);
				 j++) {
				if (!mask[j]) {
					dgVector dp(contact[j].m_point - contact[i].m_point);
					dgFloat32 dist2 = dp % dp;
					if (dist2 < window2) {
						mask[j] = 1;
						packContacts = 1;
					}
				}
			}
		}
	}

	if (packContacts) {
		dgInt32 j = 0;
		for (dgInt32 i = 0; i < count; i++) {
			if (!mask[i]) {
				contact[j] = contact[i];
				j++;
			}
		}
		count = j;
	}

	if (count > maxCount) {
		count = ReduceContacts(count, contact, maxCount, window * dgFloat32(2.0f),
							   1);
	}
	return count;
}

void dgWorld::ProcessCachedContacts(dgContact *const contact,
									const dgContactMaterial *const material, dgFloat32 timestep,
									dgInt32 threadIndex) const {
	_ASSERTE(contact);
	_ASSERTE(contact->m_body0);
	_ASSERTE(contact->m_body1);
	_ASSERTE(contact->m_myCacheMaterial);
	_ASSERTE(contact->m_myCacheMaterial == material);

	_ASSERTE(contact->m_body0 != contact->m_body1);
	dgList<dgContactMaterial> &list = *contact;
	contact->m_broadphaseLru = dgInt32(m_broadPhaseLru);
	contact->m_myCacheMaterial = material;

	dgList<dgContactMaterial>::dgListNode *nextContactNode;
	for (dgList<dgContactMaterial>::dgListNode *contactNode = list.GetFirst();
		 contactNode; contactNode = nextContactNode) {
		nextContactNode = contactNode->GetNext();
		dgContactMaterial &contactMaterial = contactNode->GetInfo();

		_ASSERTE(dgCheckFloat(contactMaterial.m_point.m_x));
		_ASSERTE(dgCheckFloat(contactMaterial.m_point.m_y));
		_ASSERTE(dgCheckFloat(contactMaterial.m_point.m_z));
		_ASSERTE(contactMaterial.m_body0);
		_ASSERTE(contactMaterial.m_body1);
		_ASSERTE(contactMaterial.m_collision0);
		_ASSERTE(contactMaterial.m_collision1);
		_ASSERTE(contactMaterial.m_body0 == contact->m_body0);
		_ASSERTE(contactMaterial.m_body1 == contact->m_body1);
		//		_ASSERTE (contactMaterial.m_userId != 0xffffffff);

		contactMaterial.m_softness = material->m_softness;
		contactMaterial.m_restitution = material->m_restitution;
		contactMaterial.m_staticFriction0 = material->m_staticFriction0;
		contactMaterial.m_staticFriction1 = material->m_staticFriction1;
		contactMaterial.m_dynamicFriction0 = material->m_dynamicFriction0;
		contactMaterial.m_dynamicFriction1 = material->m_dynamicFriction1;

		//		contactMaterial.m_collisionEnable = true;
		//		contactMaterial.m_friction0Enable = material->m_friction0Enable;
		//		contactMaterial.m_friction1Enable = material->m_friction1Enable;
		//		contactMaterial.m_override0Accel = false;
		//		contactMaterial.m_override1Accel = false;
		//		contactMaterial.m_overrideNormalAccel = false;
		contactMaterial.m_flags = dgContactMaterial::m_collisionEnable__ | (material->m_flags & (dgContactMaterial::m_friction0Enable__ | dgContactMaterial::m_friction1Enable__));
		contactMaterial.m_userData = material->m_userData;
	}

	if (material->m_contactPoint) {
		material->m_contactPoint(*contact, timestep, threadIndex);
	}

	contact->m_maxDOF = dgUnsigned32(3 * contact->GetCount());
}

void dgWorld::ProcessTriggers(dgCollidingPairCollector::dgPair *const pair,
							  dgFloat32 timestep, dgInt32 threadIndex) {
	dgBody *const body0 = pair->m_body0;
	dgBody *const body1 = pair->m_body1;
	dgContact *contact1 = pair->m_contact;
	const dgContactMaterial *const material = pair->m_material;
	_ASSERTE(body0 != body1);

	if (!contact1) {
		dgGetUserLock();
		contact1 = new (m_allocator) dgContact(this);
		pair->m_contact = contact1;
		AttachConstraint(contact1, body0, body1);
		dgReleasedUserLock();
	} else if (contact1->GetBody0() != body0) {
		_ASSERTE(0);
		_ASSERTE(contact1->GetBody1() == body0);
		_ASSERTE(contact1->GetBody0() == body1);
		Swap(contact1->m_body0, contact1->m_body1);
		Swap(contact1->m_link0, contact1->m_link1);
	}

	dgContact *const contact = contact1;
	contact->m_myCacheMaterial = material;
	contact->m_broadphaseLru = dgInt32(m_broadPhaseLru);

	_ASSERTE(body0);
	_ASSERTE(body1);
	_ASSERTE(contact->m_body0 == body0);
	_ASSERTE(contact->m_body1 == body1);

	if (material->m_contactPoint) {
		material->m_contactPoint(*contact, timestep, threadIndex);
	}
	contact->m_maxDOF = 0;
}

void dgWorld::ProcessContacts(dgCollidingPairCollector::dgPair *const pair,
							  dgFloat32 timestep, dgInt32 threadIndex) {
	dgBody *const body0 = pair->m_body0;
	dgBody *const body1 = pair->m_body1;
	dgContact *contact1 = pair->m_contact;
	const dgContactMaterial *const material = pair->m_material;
	dgContactPoint *const contactArray = pair->m_contactBuffer;
	_ASSERTE(body0 != body1);

	if (!contact1) {
		dgGetUserLock();
		contact1 = new (m_allocator) dgContact(this);
		pair->m_contact = contact1;
		AttachConstraint(contact1, body0, body1);
		dgReleasedUserLock();
	} else if (contact1->GetBody0() != body0) {
		_ASSERTE(0);
		_ASSERTE(contact1->GetBody1() == body0);
		_ASSERTE(contact1->GetBody0() == body1);
		Swap(contact1->m_body0, contact1->m_body1);
		Swap(contact1->m_link0, contact1->m_link1);
	}

	dgContact *const contact = contact1;
	dgInt32 contactCount = pair->m_contactCount;
	contact->m_myCacheMaterial = material;
	dgList<dgContactMaterial> &list = *contact;

	contact->m_broadphaseLru = dgInt32(m_broadPhaseLru);

	_ASSERTE(body0);
	_ASSERTE(body1);
	_ASSERTE(contact->m_body0 == body0);
	_ASSERTE(contact->m_body1 == body1);

	contact->m_prevPosit0 = body0->m_matrix.m_posit;
	contact->m_prevPosit1 = body1->m_matrix.m_posit;
	contact->m_prevRotation0 = body0->m_rotation;
	contact->m_prevRotation1 = body1->m_rotation;

	dgInt32 count = 0;
	dgVector cachePosition[DG_MAX_CONTATCS];
	dgList<dgContactMaterial>::dgListNode *nodes[DG_MAX_CONTATCS];
	for (dgList<dgContactMaterial>::dgListNode *contactNode = list.GetFirst();
		 contactNode; contactNode = contactNode->GetNext()) {
		nodes[count] = contactNode;
		cachePosition[count] = contactNode->GetInfo().m_point;
		count++;
	}

	const dgVector &v0 = body0->m_veloc;
	const dgVector &w0 = body0->m_omega;
	const dgMatrix &matrix0 = body0->m_matrix;

	const dgVector &v1 = body1->m_veloc;
	const dgVector &w1 = body1->m_omega;
	const dgMatrix &matrix1 = body1->m_matrix;

	dgVector controlDir0(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						 dgFloat32(0.0f));
	dgVector controlDir1(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						 dgFloat32(0.0f));
	dgVector controlNormal(contactArray[0].m_normal);
	dgVector vel0(v0 + w0 * (contactArray[0].m_point - matrix0.m_posit));
	dgVector vel1(v1 + w1 * (contactArray[0].m_point - matrix1.m_posit));
	dgVector vRel(vel1 - vel0);
	dgVector tangDir(vRel - controlNormal.Scale(vRel % controlNormal));
	dgFloat32 diff = tangDir % tangDir;

	dgInt32 staticMotion = 0;
	//	if (diff > dgFloat32 (1.0e-2f)) {
	//		staticMotion = 0;
	//	} else {
	if (diff <= dgFloat32(1.0e-2f)) {
		staticMotion = 1;
		if (dgAbsf(controlNormal.m_z) > dgFloat32(0.577f)) {
			tangDir = dgVector(-controlNormal.m_y, controlNormal.m_z, dgFloat32(0.0f),
							   dgFloat32(0.0f));
		} else {
			tangDir = dgVector(-controlNormal.m_y, controlNormal.m_x, dgFloat32(0.0f),
							   dgFloat32(0.0f));
		}
		controlDir0 = controlNormal * tangDir;
		_ASSERTE(controlDir0 % controlDir0 > dgFloat32(1.0e-8f));
		controlDir0 = controlDir0.Scale(dgRsqrt(controlDir0 % controlDir0));
		controlDir1 = controlNormal * controlDir0;
		_ASSERTE(
			dgAbsf((controlDir0 * controlDir1) % controlNormal - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
	}

	// dgTrace (("contact pair %d %d\n", body0->m_uniqueID, body1->m_uniqueID));

	dgFloat32 maxImpulse = dgFloat32(-1.0f);
	dgFloat32 breakImpulse0 = dgFloat32(0.0f);
	dgFloat32 breakImpulse1 = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < contactCount; i++) {
		dgList<dgContactMaterial>::dgListNode *contactNode;
		contactNode = NULL;
		dgFloat32 min = dgFloat32(1.0e20f);
		dgInt32 index = -1;
		for (dgInt32 j = 0; j < count; j++) {
			dgVector v(cachePosition[j] - contactArray[i].m_point);
			diff = v % v;
			if (diff < min) {
				min = diff;
				index = j;
				contactNode = nodes[j];
			}
		}

		if (contactNode) {
			count--;
			_ASSERTE(index != -1);
			nodes[index] = nodes[count];
			cachePosition[index] = cachePosition[count];
		} else {
			dgGetUserLock();
			contactNode = list.Append();
			dgReleasedUserLock();
		}

		dgContactMaterial *const contactMaterial = &contactNode->GetInfo();

		_ASSERTE(dgCheckFloat(contactArray[i].m_point.m_x));
		_ASSERTE(dgCheckFloat(contactArray[i].m_point.m_y));
		_ASSERTE(dgCheckFloat(contactArray[i].m_point.m_z));
		_ASSERTE(contactArray[i].m_body0);
		_ASSERTE(contactArray[i].m_body1);
		_ASSERTE(contactArray[i].m_collision0);
		_ASSERTE(contactArray[i].m_collision1);
		_ASSERTE(contactArray[i].m_body0 == body0);
		_ASSERTE(contactArray[i].m_body1 == body1);
		//		_ASSERTE (contactArray[i].m_userId != 0xffffffff);

		contactMaterial->m_point = contactArray[i].m_point;
		contactMaterial->m_normal = contactArray[i].m_normal;
		contactMaterial->m_userId = contactArray[i].m_userId;
		contactMaterial->m_penetration = contactArray[i].m_penetration;
		contactMaterial->m_body0 = contactArray[i].m_body0;
		contactMaterial->m_body1 = contactArray[i].m_body1;
		contactMaterial->m_collision0 = contactArray[i].m_collision0;
		contactMaterial->m_collision1 = contactArray[i].m_collision1;
		contactMaterial->m_softness = material->m_softness;
		contactMaterial->m_restitution = material->m_restitution;
		contactMaterial->m_staticFriction0 = material->m_staticFriction0;
		contactMaterial->m_staticFriction1 = material->m_staticFriction1;
		contactMaterial->m_dynamicFriction0 = material->m_dynamicFriction0;
		contactMaterial->m_dynamicFriction1 = material->m_dynamicFriction1;

		_ASSERTE(
			(dgAbsf(contactMaterial->m_normal % contactMaterial->m_normal) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f));

		// dgTrace (("%f\n", contactMaterial.m_penetration));
		// dgTrace (("p(%f %f %f) n(%f %f %f)\n", contactMaterial.m_point.m_x, contactMaterial.m_point.m_y, contactMaterial.m_point.m_z,
		//									   contactMaterial.m_normal.m_x, contactMaterial.m_normal.m_y, contactMaterial.m_normal.m_z));

		//		contactMaterial.m_collisionEnable = true;
		//		contactMaterial.m_friction0Enable = material->m_friction0Enable;
		//		contactMaterial.m_friction1Enable = material->m_friction1Enable;
		//		contactMaterial.m_override0Accel = false;
		//		contactMaterial.m_override1Accel = false;
		//		contactMaterial.m_overrideNormalAccel = false;
		contactMaterial->m_flags = dgContactMaterial::m_collisionEnable__ | (material->m_flags & (dgContactMaterial::m_friction0Enable__ | dgContactMaterial::m_friction1Enable__));
		contactMaterial->m_userData = material->m_userData;

		if (staticMotion) {
			if ((contactMaterial->m_normal % controlNormal) > dgFloat32(0.9995f)) {
				contactMaterial->m_dir0 = controlDir0;
				contactMaterial->m_dir1 = controlDir1;
			} else {
				if (dgAbsf(contactMaterial->m_normal.m_z) > dgFloat32(0.577f)) {
					tangDir = dgVector(-contactMaterial->m_normal.m_y,
									   contactMaterial->m_normal.m_z, dgFloat32(0.0f), dgFloat32(0.0f));
				} else {
					tangDir = dgVector(-contactMaterial->m_normal.m_y,
									   contactMaterial->m_normal.m_x, dgFloat32(0.0f), dgFloat32(0.0f));
				}
				contactMaterial->m_dir0 = contactMaterial->m_normal * tangDir;
				_ASSERTE(
					contactMaterial->m_dir0 % contactMaterial->m_dir0 > dgFloat32(1.0e-8f));
				contactMaterial->m_dir0 = contactMaterial->m_dir0.Scale(
					dgRsqrt(contactMaterial->m_dir0 % contactMaterial->m_dir0));
				contactMaterial->m_dir1 = contactMaterial->m_normal * contactMaterial->m_dir0;
				_ASSERTE(
					dgAbsf((contactMaterial->m_dir0 * contactMaterial->m_dir1) % contactMaterial->m_normal - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
			}

		} else {
			dgVector vel0(v0 + w0 * (contactMaterial->m_point - matrix0.m_posit));
			dgVector vel1(v1 + w1 * (contactMaterial->m_point - matrix1.m_posit));
			dgVector vRel(vel1 - vel0);

			dgFloat32 impulse = vRel % contactMaterial->m_normal;
			if (dgAbsf(impulse) > maxImpulse) {
				maxImpulse = dgAbsf(impulse);
				breakImpulse0 = contactMaterial->m_collision0->GetBreakImpulse();
				breakImpulse1 = contactMaterial->m_collision1->GetBreakImpulse();
			}

			dgVector tangDir(vRel - contactMaterial->m_normal.Scale(impulse));
			diff = tangDir % tangDir;

			if (diff > dgFloat32(1.0e-2f)) {
				contactMaterial->m_dir0 = tangDir.Scale(dgRsqrt(diff));
			} else {
				if (dgAbsf(contactMaterial->m_normal.m_z) > dgFloat32(0.577f)) {
					tangDir = dgVector(-contactMaterial->m_normal.m_y,
									   contactMaterial->m_normal.m_z, dgFloat32(0.0f), dgFloat32(0.0f));
				} else {
					tangDir = dgVector(-contactMaterial->m_normal.m_y,
									   contactMaterial->m_normal.m_x, dgFloat32(0.0f), dgFloat32(0.0f));
				}
				contactMaterial->m_dir0 = contactMaterial->m_normal * tangDir;
				_ASSERTE(
					contactMaterial->m_dir0 % contactMaterial->m_dir0 > dgFloat32(1.0e-8f));
				contactMaterial->m_dir0 = contactMaterial->m_dir0.Scale(
					dgRsqrt(contactMaterial->m_dir0 % contactMaterial->m_dir0));
			}
			contactMaterial->m_dir1 = contactMaterial->m_normal * contactMaterial->m_dir0;
			_ASSERTE(
				dgAbsf((contactMaterial->m_dir0 * contactMaterial->m_dir1) % contactMaterial->m_normal - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
		}
		contactMaterial->m_normal.m_w = dgFloat32(0.0f);
		contactMaterial->m_dir0.m_w = dgFloat32(0.0f);
		contactMaterial->m_dir1.m_w = dgFloat32(0.0f);
	}

	for (dgInt32 i = 0; i < count; i++) {
		dgGetUserLock();
		list.Remove(nodes[i]);
		dgReleasedUserLock();
	}

	if (material->m_contactPoint) {
		material->m_contactPoint(*contact, timestep, threadIndex);
	}

	if (maxImpulse > dgFloat32(1.0f)) {
		maxImpulse /= (body0->m_invMass.m_w + body1->m_invMass.m_w);
		if ((maxImpulse > breakImpulse0) || (maxImpulse > breakImpulse1)) {
			dgGetUserLock();
			if (maxImpulse > breakImpulse0) {
				AddToBreakQueue(contact, body0, maxImpulse);
			}
			if (maxImpulse > breakImpulse1) {
				AddToBreakQueue(contact, body1, maxImpulse);
			}
			dgReleasedUserLock();
		}
	}

	contact->m_maxDOF = dgUnsigned32(3 * contact->GetCount());
}

dgInt32 dgWorld::ValidateContactCache(dgBody *const convexBody,
									  dgBody *const otherBody, dgContact *const contact) const {
	_ASSERTE(contact && (contact->GetId() == dgContactConstraintId));
	_ASSERTE(
		(contact->GetBody0() == otherBody) || (contact->GetBody1() == otherBody));
	_ASSERTE(
		(contact->GetBody0() == convexBody) || (contact->GetBody1() == convexBody));

	dgInt32 contactCount = 0;

#ifdef DG_USE_CACHE_CONTACTS
#define DG_CACHE_DIST_TOL dgFloat32(1.0e-3f)

	dgBody *const body0 = contact->GetBody0();
	dgVector error0(contact->m_prevPosit0 - body0->m_matrix.m_posit);
	dgFloat32 err2 = error0 % error0;
	if (err2 < (DG_CACHE_DIST_TOL * DG_CACHE_DIST_TOL)) {
		dgBody *const body1 = contact->GetBody1();
		dgVector error1(contact->m_prevPosit1 - body1->m_matrix.m_posit);
		err2 = error1 % error1;
		if (err2 < (DG_CACHE_DIST_TOL * DG_CACHE_DIST_TOL)) {
			dgQuaternion errorRot0(contact->m_prevRotation0 - body0->m_rotation);
			err2 = errorRot0.DotProduct(errorRot0);
			if (err2 < (DG_CACHE_DIST_TOL * DG_CACHE_DIST_TOL)) {
				dgQuaternion errorRot1(contact->m_prevRotation1 - body1->m_rotation);
				err2 = errorRot1.DotProduct(errorRot1);
				if (err2 < (DG_CACHE_DIST_TOL * DG_CACHE_DIST_TOL)) {
					dgMatrix matrix0(
						dgMatrix(contact->m_prevRotation0, contact->m_prevPosit0).Inverse() * body0->m_matrix);
					dgMatrix matrix1(
						dgMatrix(contact->m_prevRotation1, contact->m_prevPosit1).Inverse() * body1->m_matrix);

					dgList<dgContactMaterial> &list = *contact;
					for (dgList<dgContactMaterial>::dgListNode *ptr = list.GetFirst();
						 ptr; ptr = ptr->GetNext()) {
						dgContactMaterial &contactMaterial = ptr->GetInfo();
						dgVector p0(matrix0.TransformVector(contactMaterial.m_point));
						dgVector p1(matrix1.TransformVector(contactMaterial.m_point));
						dgVector error(p1 - p0);

						err2 = error % error;
						if (err2 > (DG_CACHE_DIST_TOL * DG_CACHE_DIST_TOL)) {
							contactCount = 0;
							break;
						}
						contactCount++;
					}
				}
			}
		}
	}
#endif

	return contactCount;
}

void dgWorld::CompoundContactsSimd(dgCollidingPairCollector::dgPair *const pair,
								   dgCollisionParamProxy &proxy) const {
	dgInt32 contactCount;
	dgBody *otherBody;
	dgBody *compoundBody;
	dgContact *constraint;
	dgContactPoint *const contacts = pair->m_contactBuffer;

	constraint = pair->m_contact;
	compoundBody = pair->m_body0;
	otherBody = pair->m_body1;

	pair->m_contactCount = 0;
	proxy.m_contacts = contacts;

	pair->m_isTrigger = 0;
	proxy.m_isTriggerVolume = 0;
	proxy.m_inTriggerVolume = 0;

	if (constraint) {
		contactCount = ValidateContactCache(compoundBody, otherBody, constraint);
		if (contactCount) {
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	contactCount =
		((dgCollisionCompound *)compoundBody->m_collision)->CalculateContacts(pair, proxy, 1);

	if (contactCount) {
		// prune close contacts
		pair->m_contactCount = dgInt16(PruneContacts(contactCount, contacts));
	}
}

void dgWorld::CompoundContacts(dgCollidingPairCollector::dgPair *const pair,
							   dgCollisionParamProxy &proxy) const {
	dgInt32 contactCount;
	dgBody *otherBody;
	dgBody *compoundBody;
	dgContact *constraint;
	dgContactPoint *const contacts = pair->m_contactBuffer;

	constraint = pair->m_contact;
	compoundBody = pair->m_body0;
	otherBody = pair->m_body1;

	pair->m_contactCount = 0;
	proxy.m_contacts = contacts;

	pair->m_isTrigger = 0;
	proxy.m_isTriggerVolume = 0;
	proxy.m_inTriggerVolume = 0;

	if (constraint) {
		contactCount = ValidateContactCache(compoundBody, otherBody, constraint);
		if (contactCount) {
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	contactCount =
		((dgCollisionCompound *)compoundBody->m_collision)->CalculateContacts(pair, proxy, 0);

	if (contactCount) {
		// prune close contacts
		pair->m_contactCount = dgInt16(PruneContacts(contactCount, contacts));
	}
}

void dgWorld::ConvexContactsSimd(dgCollidingPairCollector::dgPair *const pair,
								 dgCollisionParamProxy &proxy) const {
	dgInt32 contactCount;
	dgBody *otherBody;
	dgBody *convexBody;
	dgContact *constraint;

	contactCount = 0;
	constraint = pair->m_contact;

	convexBody = pair->m_body0;
	otherBody = pair->m_body1;
	if (constraint) {
		contactCount = ValidateContactCache(convexBody, otherBody, constraint);
		if (contactCount) {
			pair->m_isTrigger = 0;
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	//	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_contacts = pair->m_contactBuffer;

	if (otherBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		if (convexBody->m_invMass.m_w <= dgFloat32(1.0e-6f)) {
			Swap(convexBody, otherBody);
			pair->m_body0 = convexBody;
			pair->m_body1 = otherBody;
		}

		_ASSERTE(
			convexBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			otherBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			pair->m_body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			pair->m_body1->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));

		proxy.m_referenceBody = convexBody;
		proxy.m_floatingBody = otherBody;
		proxy.m_referenceCollision = convexBody->m_collision;
		proxy.m_floatingCollision = otherBody->m_collision;
		proxy.m_referenceMatrix = convexBody->m_collisionWorldMatrix;
		proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;
		// contactCount = CalculateConvexToConvexContacts (proxy);
		pair->m_contactCount = dgInt16(CalculateConvexToConvexContactsSimd(proxy));
		pair->m_isTrigger = proxy.m_inTriggerVolume;

	} else {
		_ASSERTE(
			pair->m_body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			convexBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));

		proxy.m_referenceBody = convexBody;
		proxy.m_floatingBody = otherBody;
		proxy.m_referenceCollision = convexBody->m_collision;
		proxy.m_floatingCollision = otherBody->m_collision;
		proxy.m_referenceMatrix = convexBody->m_collisionWorldMatrix;
		proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;
		// contactCount = CalculateConvexToNonConvexContacts (proxy);
		pair->m_contactCount = dgInt16(
			CalculateConvexToNonConvexContactsSimd(proxy));
		pair->m_isTrigger = proxy.m_inTriggerVolume;
	}
}

void dgWorld::ConvexContacts(dgCollidingPairCollector::dgPair *const pair,
							 dgCollisionParamProxy &proxy) const {
	dgInt32 contactCount = 0;
	dgContact *const constraint = pair->m_contact;

	dgBody *convexBody = pair->m_body0;
	dgBody *otherBody = pair->m_body1;
	if (constraint) {
		contactCount = ValidateContactCache(convexBody, otherBody, constraint);
		if (contactCount) {
			pair->m_isTrigger = 0;
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	//	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_contacts = pair->m_contactBuffer;

	if (otherBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		if (convexBody->m_invMass.m_w <= dgFloat32(1.0e-6f)) {
			Swap(convexBody, otherBody);
			pair->m_body0 = convexBody;
			pair->m_body1 = otherBody;
		}

		_ASSERTE(
			convexBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			otherBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			pair->m_body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			pair->m_body1->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));

		proxy.m_referenceBody = convexBody;
		proxy.m_floatingBody = otherBody;
		proxy.m_referenceCollision = convexBody->m_collision;
		proxy.m_floatingCollision = otherBody->m_collision;
		proxy.m_referenceMatrix = convexBody->m_collisionWorldMatrix;
		proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;
		pair->m_contactCount = dgInt16(CalculateConvexToConvexContacts(proxy));
		pair->m_isTrigger = proxy.m_inTriggerVolume;

	} else {
		_ASSERTE(
			pair->m_body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));
		_ASSERTE(
			convexBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));

		proxy.m_referenceBody = convexBody;
		proxy.m_floatingBody = otherBody;
		proxy.m_referenceCollision = convexBody->m_collision;
		proxy.m_floatingCollision = otherBody->m_collision;
		proxy.m_referenceMatrix = convexBody->m_collisionWorldMatrix;
		proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;
		pair->m_contactCount = dgInt16(CalculateConvexToNonConvexContacts(proxy));
		pair->m_isTrigger = proxy.m_inTriggerVolume;
	}
}

void dgWorld::SceneContactsSimd(const dgCollisionScene::dgProxy &sceneProxy,
								dgCollidingPairCollector::dgPair *const pair,
								dgCollisionParamProxy &proxy) const {
	_ASSERTE(
		pair->m_body1->GetCollision()->IsType(dgCollision::dgCollisionScene_RTTI));
	if (sceneProxy.m_shape->IsType(dgCollision::dgConvexCollision_RTTI)) {
		proxy.m_floatingCollision = sceneProxy.m_shape;
		proxy.m_floatingMatrix = sceneProxy.m_matrix;
		proxy.m_maxContacts =
			((DG_MAX_CONTATCS - pair->m_contactCount) > 16) ? 16 : DG_MAX_CONTATCS - pair->m_contactCount;

		proxy.m_contacts = &pair->m_contactBuffer[pair->m_contactCount];
		pair->m_contactCount = pair->m_contactCount + dgInt16(CalculateConvexToConvexContactsSimd(proxy));
		if (pair->m_contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
			pair->m_contactCount = dgInt16(
				ReduceContacts(pair->m_contactCount, pair->m_contactBuffer,
							   DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE));
		}
		//		pair->m_isTrigger = proxy.m_inTriggerVolume;

	} else {
		proxy.m_floatingCollision = sceneProxy.m_shape;
		proxy.m_floatingMatrix = sceneProxy.m_matrix;
		proxy.m_maxContacts =
			((DG_MAX_CONTATCS - pair->m_contactCount) > 32) ? 32 : DG_MAX_CONTATCS - pair->m_contactCount;

		proxy.m_contacts = &pair->m_contactBuffer[pair->m_contactCount];
		pair->m_contactCount = pair->m_contactCount + dgInt16(CalculateConvexToNonConvexContactsSimd(proxy));
		if (pair->m_contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
			pair->m_contactCount = dgInt16(
				ReduceContacts(pair->m_contactCount, pair->m_contactBuffer,
							   DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE));
		}
		//		pair->m_isTrigger = proxy.m_inTriggerVolume;
	}
}

void dgWorld::SceneContacts(const dgCollisionScene::dgProxy &sceneProxy,
							dgCollidingPairCollector::dgPair *const pair,
							dgCollisionParamProxy &proxy) const {
	_ASSERTE(
		pair->m_body1->GetCollision()->IsType(dgCollision::dgCollisionScene_RTTI));
	if (sceneProxy.m_shape->IsType(dgCollision::dgConvexCollision_RTTI)) {
		proxy.m_floatingCollision = sceneProxy.m_shape;
		proxy.m_floatingMatrix = sceneProxy.m_matrix;
		proxy.m_maxContacts =
			((DG_MAX_CONTATCS - pair->m_contactCount) > 16) ? 16 : DG_MAX_CONTATCS - pair->m_contactCount;

		proxy.m_contacts = &pair->m_contactBuffer[pair->m_contactCount];
		pair->m_contactCount = pair->m_contactCount + dgInt16(CalculateConvexToConvexContacts(proxy));
		if (pair->m_contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
			pair->m_contactCount = dgInt16(
				ReduceContacts(pair->m_contactCount, pair->m_contactBuffer,
							   DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE));
		}
		//		pair->m_isTrigger = proxy.m_inTriggerVolume;

	} else {
		proxy.m_floatingCollision = sceneProxy.m_shape;
		proxy.m_floatingMatrix = sceneProxy.m_matrix;
		proxy.m_maxContacts =
			((DG_MAX_CONTATCS - pair->m_contactCount) > 32) ? 32 : DG_MAX_CONTATCS - pair->m_contactCount;

		proxy.m_contacts = &pair->m_contactBuffer[pair->m_contactCount];
		pair->m_contactCount = pair->m_contactCount + dgInt16(CalculateConvexToNonConvexContacts(proxy));
		if (pair->m_contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
			pair->m_contactCount = dgInt16(
				ReduceContacts(pair->m_contactCount, pair->m_contactBuffer,
							   DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE));
		}
		//		pair->m_isTrigger = proxy.m_inTriggerVolume;
	}
}

void dgWorld::SceneContacts(dgCollidingPairCollector::dgPair *const pair,
							dgCollisionParamProxy &proxy) const {
	dgContact *const constraint = pair->m_contact;

	pair->m_isTrigger = 0;
	pair->m_contactCount = 0;

	proxy.m_isTriggerVolume = 0;
	proxy.m_inTriggerVolume = 0;
	//	proxy.m_contacts = contacts;

	//	_ASSERTE (pair->m_body0->m_invMass.m_w != dgFloat32 (0.0f));
	//	_ASSERTE (pair->m_body1->m_invMass.m_w == dgFloat32 (0.0f));
	if (constraint) {
		dgInt32 contactCount = ValidateContactCache(pair->m_body0, pair->m_body1,
													constraint);
		if (contactCount) {
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	dgCollisionScene *const scene =
		(dgCollisionScene *)pair->m_body1->GetCollision();
	_ASSERTE(scene->IsType(dgCollision::dgCollisionScene_RTTI));
	if (pair->m_body0->GetCollision()->IsType(
			dgCollision::dgConvexCollision_RTTI)) {
		proxy.m_referenceBody = pair->m_body0;
		proxy.m_floatingBody = pair->m_body1;
		proxy.m_referenceCollision = pair->m_body0->m_collision;
		proxy.m_floatingCollision = NULL;
		proxy.m_referenceMatrix = pair->m_body0->m_collisionWorldMatrix;

		scene->CollidePair(pair, proxy);
		if (pair->m_contactCount) {
			// prune close contacts
			pair->m_contactCount = dgInt16(
				PruneContacts(pair->m_contactCount, pair->m_contactBuffer));
		}

	} else {
		_ASSERTE(0);
	}
}

void dgWorld::SceneContactsSimd(dgCollidingPairCollector::dgPair *const pair,
								dgCollisionParamProxy &proxy) const {
	dgContact *const constraint = pair->m_contact;

	pair->m_isTrigger = 0;
	pair->m_contactCount = 0;

	proxy.m_isTriggerVolume = 0;
	proxy.m_inTriggerVolume = 0;
	//	proxy.m_contacts = contacts;

	//	_ASSERTE (pair->m_body0->m_invMass.m_w != dgFloat32 (0.0f));
	//	_ASSERTE (pair->m_body1->m_invMass.m_w == dgFloat32 (0.0f));
	if (constraint) {
		dgInt32 contactCount = ValidateContactCache(pair->m_body0, pair->m_body1,
													constraint);
		if (contactCount) {
			pair->m_contactCount = 0;
			pair->m_contactBuffer = NULL;
			return;
		}
	}

	dgCollisionScene *const scene =
		(dgCollisionScene *)pair->m_body1->GetCollision();
	_ASSERTE(scene->IsType(dgCollision::dgCollisionScene_RTTI));
	if (pair->m_body0->GetCollision()->IsType(
			dgCollision::dgConvexCollision_RTTI)) {
		proxy.m_referenceBody = pair->m_body0;
		proxy.m_floatingBody = pair->m_body1;
		proxy.m_referenceCollision = pair->m_body0->m_collision;
		proxy.m_floatingCollision = NULL;
		proxy.m_referenceMatrix = pair->m_body0->m_collisionWorldMatrix;

		scene->CollidePairSimd(pair, proxy);

		if (pair->m_contactCount) {
			// prune close contacts
			pair->m_contactCount = dgInt16(
				PruneContacts(pair->m_contactCount, pair->m_contactBuffer));
		}

	} else {
		_ASSERTE(0);
	}
}

void dgWorld::CalculateContactsSimd(
	dgCollidingPairCollector::dgPair *const pair, dgFloat32 timestep,
	dgInt32 threadIndex) {
	dgBody *body0;
	dgBody *body1;
	const dgContactMaterial *material;

	dgCollisionParamProxy proxy(threadIndex);

	body0 = pair->m_body0;
	body1 = pair->m_body1;

	material = pair->m_material;
	proxy.m_timestep = timestep;
	proxy.m_unconditionalCast = 0;
	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_penetrationPadding = material->m_penetrationPadding;
	//	proxy.m_continueCollision = material->m_collisionContinueCollisionEnable & (body0->m_continueCollisionMode | body1->m_continueCollisionMode);
	proxy.m_continueCollision = dgInt32(
		((material->m_flags & dgContactMaterial::m_collisionContinueCollisionEnable__) ? 1 : 0) & (body0->m_continueCollisionMode | body1->m_continueCollisionMode));
	proxy.m_isTriggerVolume = body0->m_collision->IsTriggerVolume() | body1->m_collision->IsTriggerVolume();

	if (body0->m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		SceneContactsSimd(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		SceneContactsSimd(pair, proxy);
	} else if (body0->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		CompoundContactsSimd(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		CompoundContactsSimd(pair, proxy);
	} else if (body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		ConvexContactsSimd(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		ConvexContactsSimd(pair, proxy);
	}
}

void dgWorld::CalculateContacts(dgCollidingPairCollector::dgPair *const pair,
								dgFloat32 timestep, dgInt32 threadIndex) {
	dgCollisionParamProxy proxy(threadIndex);

	dgBody *const body0 = pair->m_body0;
	dgBody *const body1 = pair->m_body1;

	const dgContactMaterial *const material = pair->m_material;
	proxy.m_timestep = timestep;
	proxy.m_unconditionalCast = 0;
	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_penetrationPadding = material->m_penetrationPadding;
	proxy.m_isTriggerVolume = body0->m_collision->IsTriggerVolume() | body1->m_collision->IsTriggerVolume();
	//	proxy.m_continueCollision = material->m_collisionContinueCollisionEnable & (body0->m_continueCollisionMode | body1->m_continueCollisionMode);
	proxy.m_continueCollision = dgInt32(
		((material->m_flags & dgContactMaterial::m_collisionContinueCollisionEnable__) ? 1 : 0) & (body0->m_continueCollisionMode | body1->m_continueCollisionMode));

	if (body0->m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		SceneContacts(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		SceneContacts(pair, proxy);
	} else if (body0->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		CompoundContacts(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		CompoundContacts(pair, proxy);
	} else if (body0->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		ConvexContacts(pair, proxy);
	} else if (body1->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		Swap(pair->m_body0, pair->m_body1);
		ConvexContacts(pair, proxy);
	}
}

// ***************************************************************************
//
// ***************************************************************************
dgInt32 dgWorld::CollideContinueSimd(dgCollision *const collisionA,
									 const dgMatrix &matrixA, const dgVector &velocA, const dgVector &omegaA,
									 dgCollision *const collisionB, const dgMatrix &matrixB,
									 const dgVector &velocB, const dgVector &omegaB, dgFloat32 &retTimeStep,
									 dgTriplex *const points, dgTriplex *const normals,
									 dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex) {
	dgBody collideBodyA;
	dgBody collideBodyB;
	dgContactPoint contacts[DG_MAX_CONTATCS];

	dgInt32 count = 0;
	retTimeStep = dgFloat32(1.0e10f);
	maxSize = GetMin(DG_MAX_CONTATCS, maxSize);

	collideBodyA.m_world = this;
	collideBodyA.SetContinuesCollisionMode(true);
	collideBodyA.m_matrix = matrixA;
	collideBodyA.m_collision = collisionA;
	collideBodyA.m_masterNode = NULL;
	collideBodyA.m_collisionCell.m_cell = NULL;
	collideBodyA.m_collisionWorldMatrix = collisionA->m_offset * matrixA;
	collideBodyA.m_veloc = dgVector(velocA[0], velocA[1], velocA[2],
									dgFloat32(0.0f));
	collideBodyA.m_omega = dgVector(omegaA[0], omegaA[1], omegaA[2],
									dgFloat32(0.0f));
	collideBodyA.m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyA.m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyA.m_invMass = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
									  dgFloat32(1.0f), dgFloat32(1.0f));
	collideBodyA.UpdateCollisionMatrixSimd(dgFloat32(1.0f), 0);

	collideBodyB.m_world = this;
	collideBodyB.SetContinuesCollisionMode(true);
	collideBodyB.m_matrix = matrixB;
	collideBodyB.m_collision = collisionB;
	collideBodyB.m_masterNode = NULL;
	collideBodyB.m_collisionCell.m_cell = NULL;
	collideBodyB.m_collisionWorldMatrix = collisionB->m_offset * matrixB;
	collideBodyB.m_veloc = dgVector(velocB[0], velocB[1], velocB[2],
									dgFloat32(0.0f));
	collideBodyB.m_omega = dgVector(omegaB[0], omegaB[1], omegaB[2],
									dgFloat32(0.0f));
	collideBodyB.m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyB.m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyB.m_invMass = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
									  dgFloat32(1.0f), dgFloat32(1.0f));
	collideBodyB.UpdateCollisionMatrixSimd(dgFloat32(1.0f), 0);

	dgCollisionParamProxy proxy(threadIndex);
	proxy.m_timestep = dgFloat32(1.0f);
	proxy.m_unconditionalCast = 1;
	proxy.m_penetrationPadding = 0.0f;
	proxy.m_continueCollision = 1;
	//	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_maxContacts = maxSize;
	proxy.m_isTriggerVolume = 0;

	dgCollidingPairCollector::dgPair pair;
	pair.m_body0 = &collideBodyA;
	pair.m_body1 = &collideBodyB;
	pair.m_contact = NULL;
	pair.m_material = NULL;
	pair.m_contactCount = 0;
	pair.m_contactBuffer = contacts;

	dgFloat32 swapContactScale = dgFloat32(1.0f);
	if (collideBodyA.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		SceneContactsSimd(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		SceneContactsSimd(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		CompoundContactsSimd(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		CompoundContactsSimd(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		ConvexContactsSimd(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		ConvexContactsSimd(&pair, proxy);
	}
	count = pair.m_contactCount;

	if (proxy.m_timestep < dgFloat32(1.0f)) {
		retTimeStep = proxy.m_timestep;
	}

	if (count) {
		retTimeStep = proxy.m_timestep;
		if (count > maxSize) {
			count = PruneContacts(count, contacts, maxSize);
		}

		for (dgInt32 i = 0; i < count; i++) {
			points[i].m_x = contacts[i].m_point.m_x;
			points[i].m_y = contacts[i].m_point.m_y;
			points[i].m_z = contacts[i].m_point.m_z;
			normals[i].m_x = contacts[i].m_normal.m_x * swapContactScale;
			normals[i].m_y = contacts[i].m_normal.m_y * swapContactScale;
			normals[i].m_z = contacts[i].m_normal.m_z * swapContactScale;
			penetration[i] = contacts[i].m_penetration;
		}
	}
	return count;
}

dgInt32 dgWorld::CollideContinue(dgCollision *const collisionA,
								 const dgMatrix &matrixA, const dgVector &velocA, const dgVector &omegaA,
								 dgCollision *const collisionB, const dgMatrix &matrixB,
								 const dgVector &velocB, const dgVector &omegaB, dgFloat32 &retTimeStep,
								 dgTriplex *const points, dgTriplex *const normals,
								 dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex) {
	dgBody collideBodyA;
	dgBody collideBodyB;
	dgContactPoint contacts[DG_MAX_CONTATCS];

	// dgTrace (("%x -> p(%f, %f, %f) v(%f, %f, %f)\n", collisionB, matrixA.m_posit.m_x, matrixA.m_posit.m_y, matrixA.m_posit.m_z, velocA.m_x, velocA.m_y, velocA.m_z));

	dgInt32 count = 0;
	retTimeStep = dgFloat32(1.0e10f);
	maxSize = GetMin(DG_MAX_CONTATCS, maxSize);

	collideBodyA.m_world = this;
	collideBodyA.SetContinuesCollisionMode(true);
	collideBodyA.m_matrix = matrixA;
	collideBodyA.m_collision = collisionA;
	collideBodyA.m_masterNode = NULL;
	collideBodyA.m_collisionCell.m_cell = NULL;
	collideBodyA.m_collisionWorldMatrix = collisionA->m_offset * matrixA;
	collideBodyA.m_veloc = dgVector(velocA[0], velocA[1], velocA[2],
									dgFloat32(0.0f));
	collideBodyA.m_omega = dgVector(omegaA[0], omegaA[1], omegaA[2],
									dgFloat32(0.0f));
	collideBodyA.m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyA.m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyA.m_invMass = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
									  dgFloat32(1.0f), dgFloat32(1.0f));
	collideBodyA.UpdateCollisionMatrix(dgFloat32(1.0f), 0);

	collideBodyB.m_world = this;
	collideBodyB.SetContinuesCollisionMode(true);
	collideBodyB.m_matrix = matrixB;
	collideBodyB.m_collision = collisionB;
	collideBodyB.m_masterNode = NULL;
	collideBodyB.m_collisionCell.m_cell = NULL;
	collideBodyB.m_collisionWorldMatrix = collisionB->m_offset * matrixB;
	collideBodyB.m_veloc = dgVector(velocB[0], velocB[1], velocB[2],
									dgFloat32(0.0f));
	collideBodyB.m_omega = dgVector(omegaB[0], omegaB[1], omegaB[2],
									dgFloat32(0.0f));
	collideBodyB.m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyB.m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));
	collideBodyB.m_invMass = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
									  dgFloat32(1.0f), dgFloat32(1.0f));
	collideBodyB.UpdateCollisionMatrix(dgFloat32(1.0f), 0);

	dgCollisionParamProxy proxy(threadIndex);
	proxy.m_timestep = dgFloat32(1.0f);
	proxy.m_unconditionalCast = 1;
	proxy.m_penetrationPadding = 0.0f;
	proxy.m_continueCollision = 1;
	//	proxy.m_maxContacts = DG_MAX_CONTATCS;
	proxy.m_maxContacts = maxSize;
	proxy.m_isTriggerVolume = 0;

	dgCollidingPairCollector::dgPair pair;
	pair.m_body0 = &collideBodyA;
	pair.m_body1 = &collideBodyB;
	pair.m_contact = NULL;
	pair.m_material = NULL;
	pair.m_contactCount = 0;
	pair.m_contactBuffer = contacts;

	dgFloat32 swapContactScale = dgFloat32(1.0f);
	if (collideBodyA.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		SceneContactsSimd(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		SceneContactsSimd(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		CompoundContacts(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		CompoundContacts(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		ConvexContacts(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		ConvexContacts(&pair, proxy);
	}
	count = pair.m_contactCount;

	if (proxy.m_timestep < dgFloat32(1.0f)) {
		retTimeStep = proxy.m_timestep;
	}

	if (count) {

		if (count > maxSize) {
			count = PruneContacts(count, contacts, maxSize);
		}

		for (dgInt32 i = 0; i < count; i++) {
			points[i].m_x = contacts[i].m_point.m_x;
			points[i].m_y = contacts[i].m_point.m_y;
			points[i].m_z = contacts[i].m_point.m_z;
			normals[i].m_x = contacts[i].m_normal.m_x * swapContactScale;
			normals[i].m_y = contacts[i].m_normal.m_y * swapContactScale;
			normals[i].m_z = contacts[i].m_normal.m_z * swapContactScale;
			penetration[i] = contacts[i].m_penetration;
		}
	}

	return count;
}

dgInt32 dgWorld::Collide(dgCollision *const collisionA, const dgMatrix &matrixA,
						 dgCollision *const collisionB, const dgMatrix &matrixB,
						 dgTriplex *const points, dgTriplex *const normals,
						 dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex) {
	dgBody collideBodyA;
	dgBody collideBodyB;
	dgContactPoint contacts[DG_MAX_CONTATCS];

	dgInt32 count = 0;
	maxSize = GetMin(DG_MAX_CONTATCS, maxSize);

	collideBodyA.m_world = this;
	collideBodyA.m_masterNode = NULL;
	collideBodyA.m_collisionCell.m_cell = NULL;
	collideBodyA.SetContinuesCollisionMode(false);
	collideBodyA.m_matrix = matrixA;
	collideBodyA.m_collision = collisionA;
	//	collideBodyA.m_collisionWorldMatrix = collisionA->m_offset * matrixA;
	collideBodyA.UpdateCollisionMatrix(dgFloat32(0.0f), 0);

	collideBodyB.m_world = this;
	collideBodyB.m_masterNode = NULL;
	collideBodyB.m_collisionCell.m_cell = NULL;
	collideBodyB.SetContinuesCollisionMode(false);
	collideBodyB.m_matrix = matrixB;
	collideBodyB.m_collision = collisionB;
	//	collideBodyB.m_collisionWorldMatrix = collisionB->m_offset * matrixB;
	collideBodyB.UpdateCollisionMatrix(dgFloat32(0.0f), 0);

	//	isTriggerA = collisionA->IsTriggerVolume();
	//	isTriggerB = collisionB->IsTriggerVolume();

	dgCollisionParamProxy proxy(threadIndex);
	proxy.m_timestep = dgFloat32(0.0f);
	proxy.m_unconditionalCast = 1;
	proxy.m_penetrationPadding = 0.0f;
	proxy.m_continueCollision = 0;
	proxy.m_maxContacts = maxSize;
	proxy.m_isTriggerVolume = 0;

	dgCollidingPairCollector::dgPair pair;
	pair.m_body0 = &collideBodyA;
	pair.m_body1 = &collideBodyB;
	pair.m_contact = NULL;
	pair.m_material = NULL;
	pair.m_contactCount = 0;
	pair.m_contactBuffer = contacts;

	dgFloat32 swapContactScale = dgFloat32(1.0f);
	if (collideBodyA.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		SceneContacts(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		SceneContacts(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		CompoundContacts(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgCollisionCompound_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		CompoundContacts(&pair, proxy);

	} else if (collideBodyA.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		ConvexContacts(&pair, proxy);
	} else if (collideBodyB.m_collision->IsType(
				   dgCollision::dgConvexCollision_RTTI)) {
		swapContactScale = dgFloat32(-1.0f);
		Swap(pair.m_body0, pair.m_body1);
		ConvexContacts(&pair, proxy);
	}

	count = pair.m_contactCount;
	if (count > maxSize) {
		count = ReduceContacts(count, contacts, maxSize,
							   DG_REDUCE_CONTACT_TOLERANCE);
	}

	for (dgInt32 i = 0; i < count; i++) {
		points[i].m_x = contacts[i].m_point.m_x;
		points[i].m_y = contacts[i].m_point.m_y;
		points[i].m_z = contacts[i].m_point.m_z;
		normals[i].m_x = contacts[i].m_normal.m_x * swapContactScale;
		normals[i].m_y = contacts[i].m_normal.m_y * swapContactScale;
		normals[i].m_z = contacts[i].m_normal.m_z * swapContactScale;
		penetration[i] = contacts[i].m_penetration;
	}

	return count;
}
