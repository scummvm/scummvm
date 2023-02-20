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

#include "dgCollisionCompound.h"
#include "dgCollisionBVH.h"
#include "dgCollisionConvex.h"
#include "dgCollisionConvexModifier.h"
#include "dgCollisionEllipse.h"
#include "dgCollisionHeightField.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgCollisionCompound::OOBBTestData::OOBBTestData(const dgMatrix &matrix) : m_matrix(matrix) {
	for (dgInt32 i = 0; i < 3; i++) {
		m_absMatrix[i][3] = dgFloat32(0.0f);
		dgVector dir(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		             dgFloat32(0.0f));
		dir[i] = dgFloat32(1.0f);
		for (dgInt32 j = 0; j < 3; j++) {
			m_absMatrix[i][j] = dgAbsf(m_matrix[i][j]);
			dgVector axis(dir * m_matrix[j]);
			m_crossAxis[i][j] = axis;
			m_crossAxisAbs[i][j] = dgVector(dgAbsf(axis.m_x), dgAbsf(axis.m_y),
			                                dgAbsf(axis.m_z), dgFloat32(0.0f));
			m_crossAxisDotAbs[i][j] = dgVector(dgAbsf(axis % matrix[0]),
			                                   dgAbsf(axis % matrix[1]), dgAbsf(axis % matrix[2]), dgFloat32(0.0f));
		}
	}
	m_absMatrix[3][3] = dgFloat32(1.0f);
}

dgCollisionCompound::OOBBTestData::OOBBTestData(const dgMatrix &matrix,
        const dgVector &p0, const dgVector &p1) : m_matrix(matrix), m_localP0(p0), m_localP1(p1) {
	m_size = (m_localP1 - m_localP0).Scale(dgFloat32(0.5f));
	m_origin = (m_localP1 + m_localP0).Scale(dgFloat32(0.5f));

	for (dgInt32 i = 0; i < 3; i++) {
		m_absMatrix[i][3] = dgFloat32(0.0f);
		dgVector dir(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		             dgFloat32(0.0f));
		dir[i] = dgFloat32(1.0f);
		for (dgInt32 j = 0; j < 3; j++) {
			m_absMatrix[i][j] = dgAbsf(m_matrix[i][j]);
			m_crossAxis[i][j] = dir * m_matrix[j];
		}
	}
	m_absMatrix[3][3] = dgFloat32(1.0f);

	dgVector size(m_absMatrix.RotateVector(m_size));
	dgVector origin(m_matrix.TransformVector(m_origin));
	m_aabbP0 = origin - size;
	m_aabbP1 = origin + size;

	for (dgInt32 i = 0; i < 3; i++) {
		for (dgInt32 j = 0; j < 3; j++) {
			dgFloat32 d;
			dgFloat32 c;
			dgVector &axis = m_crossAxis[i][j];
			d = m_size.m_x * dgAbsf(axis % m_matrix[0]) + m_size.m_y * dgAbsf(axis % m_matrix[1]) + m_size.m_z * dgAbsf(axis % m_matrix[2]) + dgFloat32(1.0e-3f);
			c = origin % axis;

			m_extends[i][j] = dgVector(c - d, c + d, dgFloat32(0.0f),
			                           dgFloat32(0.0f));

			NEWTON_ASSERT(m_extends[i][j].m_x <= m_extends[i][j].m_y);
			m_crossAxisAbs[i][j] = dgVector(dgAbsf(axis.m_x), dgAbsf(axis.m_y),
			                                dgAbsf(axis.m_z), dgFloat32(0.0f));
		}
	}
}

dgCollisionCompound::dgNodeBase::dgNodeBase() {
	m_id = -1;
	m_shape = NULL;
	m_left = NULL;
	m_right = NULL;
}

dgCollisionCompound::dgNodeBase::dgNodeBase(dgCollisionConvex *const shape,
        dgInt32 id) {
	m_id = id;
	m_left = NULL;
	m_right = NULL;
	m_parent = NULL;
	m_shape = shape;
	m_shape->AddRef();
	m_type = m_leaf;
	m_shape->CalcAABB(shape->GetOffsetMatrix(), m_p0, m_p1);
	m_p0.m_w = 0.0f;
	m_p1.m_w = 0.0f;
	m_size = (m_p1 - m_p0).Scale(dgFloat32(0.5f));
	m_origin = (m_p1 + m_p0).Scale(dgFloat32(0.5f));

	dgVector size1(m_size.m_y, m_size.m_z, m_size.m_x, dgFloat32(0.0f));
	m_area = m_size % size1;
}

dgCollisionCompound::dgNodeBase::dgNodeBase(dgNodeBase *const left,
        dgNodeBase *const right, dgInt32 id) {
	m_id = id;
	m_left = left;
	m_right = right;
	m_parent = NULL;
	m_type = m_node;
	m_shape = NULL;

	m_p0.m_x = GetMin(left->m_p0.m_x, right->m_p0.m_x);
	m_p0.m_y = GetMin(left->m_p0.m_y, right->m_p0.m_y);
	m_p0.m_z = GetMin(left->m_p0.m_z, right->m_p0.m_z);
	m_p1.m_x = GetMax(left->m_p1.m_x, right->m_p1.m_x);
	m_p1.m_y = GetMax(left->m_p1.m_y, right->m_p1.m_y);
	m_p1.m_z = GetMax(left->m_p1.m_z, right->m_p1.m_z);
	m_p0.m_w = 0.0f;
	m_p1.m_w = 0.0f;

	m_size = (m_p1 - m_p0).Scale(dgFloat32(0.5f));
	m_origin = (m_p1 + m_p0).Scale(dgFloat32(0.5f));

	dgVector size1(m_size.m_y, m_size.m_z, m_size.m_x, dgFloat32(0.0f));
	m_area = m_size % size1;
}

dgCollisionCompound::dgNodeBase::~dgNodeBase() {
	if (m_shape) {
		m_shape->Release();
	}
	if (m_left) {
		delete m_left;
	}
	if (m_right) {
		delete m_right;
	}
}

void dgCollisionCompound::dgNodeBase::reset() {
	m_id = 0; // FIXME: Maybe should reset to -1
	m_left = NULL;
	m_right = NULL;
	m_parent = NULL;
	m_type = 0;
	m_shape = NULL;

	m_p0 = dgVector(0.0f, 0.0f, 0.0f, 0.0f);
	m_p1 = dgVector(0.0f, 0.0f, 0.0f, 0.0f);;
	m_size = dgVector(0.0f, 0.0f, 0.0f, 0.0f);;
	m_origin = dgVector(0.0f, 0.0f, 0.0f, 0.0f);;

	m_area = 0.0f;
}

bool dgCollisionCompound::dgNodeBase::BoxTest(const OOBBTestData &data,
        const dgNodeBase *const otherNode) const {
	dgVector otherOrigin(data.m_matrix.TransformVector(otherNode->m_origin));
	dgVector otherSize(data.m_absMatrix.RotateVector(otherNode->m_size));
	dgVector otherP0(otherOrigin - otherSize);
	dgVector otherP1(otherOrigin + otherSize);
	if (dgOverlapTest(m_p0, m_p1, otherP0, otherP1)) {

		dgVector origin(data.m_matrix.UntransformVector(m_origin));
		dgVector size(data.m_absMatrix.UnrotateVector(m_size));
		dgVector p0(origin - size);
		dgVector p1(origin + size);
		if (dgOverlapTest(p0, p1, otherNode->m_p0, otherNode->m_p1)) {
			for (dgInt32 i = 0; i < 3; i++) {
				for (dgInt32 j = 0; j < 3; j++) {
					dgFloat32 x0;
					dgFloat32 x1;
					dgFloat32 z0;
					dgFloat32 z1;
					dgFloat32 d;
					dgFloat32 c;

					const dgVector &axis = data.m_crossAxis[i][j];

					const dgVector &axisAbs = data.m_crossAxisAbs[i][j];
					d = m_size.m_x * axisAbs.m_x + m_size.m_y * axisAbs.m_y + m_size.m_z * axisAbs.m_z + dgFloat32(1.0e-3f);
					c = m_origin % axis;
					x0 = c - d;
					x1 = c + d;
					NEWTON_ASSERT(x0 <= x1);

					const dgVector &axisDotAbs = data.m_crossAxisDotAbs[i][j];
					d = otherNode->m_size.m_x * axisDotAbs.m_x + otherNode->m_size.m_y * axisDotAbs.m_y + otherNode->m_size.m_z * axisDotAbs.m_z + dgFloat32(1.0e-3f);
					c = otherOrigin % axis;
					z0 = c - d;
					z1 = c + d;
					NEWTON_ASSERT(z0 <= z1);

					if ((x1 < z0) || (x0 > z1)) {
						return false;
					}
				}
			}

			return true;
		}
	}
	return false;
}

bool dgCollisionCompound::dgNodeBase::BoxTest(const OOBBTestData &data) const {
	if (dgOverlapTest(data.m_aabbP0, data.m_aabbP1, m_p0, m_p1)) {

		dgVector origin(data.m_matrix.UntransformVector(m_origin));
		dgVector size(data.m_absMatrix.UnrotateVector(m_size));
		dgVector p0(origin - size);
		dgVector p1(origin + size);

		if (dgOverlapTest(p0, p1, data.m_localP0, data.m_localP1)) {
			for (dgInt32 i = 0; i < 3; i++) {
				for (dgInt32 j = 0; j < 3; j++) {
					dgFloat32 x0;
					dgFloat32 x1;

					dgFloat32 d;
					dgFloat32 c;
					const dgVector &axis = data.m_crossAxisAbs[i][j];
					d = m_size.m_x * axis.m_x + m_size.m_y * axis.m_y + m_size.m_z * axis.m_z + dgFloat32(1.0e-3f);
					c = m_origin % data.m_crossAxis[i][j];

					x0 = c - d;
					x1 = c + d;
					NEWTON_ASSERT(x0 <= x1);
					const dgVector &extend = data.m_extends[i][j];
					if ((x1 < extend.m_x) || (x0 > extend.m_y)) {
						return false;
					}
				}
			}
			return true;
		}
	}

	return false;
}

dgFloat32 dgCollisionCompound::dgNodeBase::BoxClosestDistance(
    const dgVector *const points, dgInt32 count) const {
	dgVector box[8];
	box[0] = dgVector(m_p0.m_x, m_p0.m_y, m_p0.m_z, dgFloat32(0.0f));
	box[1] = dgVector(m_p0.m_x, m_p0.m_y, m_p1.m_z, dgFloat32(0.0f));
	box[2] = dgVector(m_p0.m_x, m_p1.m_y, m_p0.m_z, dgFloat32(0.0f));
	box[3] = dgVector(m_p0.m_x, m_p1.m_y, m_p1.m_z, dgFloat32(0.0f));
	box[4] = dgVector(m_p1.m_x, m_p0.m_y, m_p0.m_z, dgFloat32(0.0f));
	box[5] = dgVector(m_p1.m_x, m_p0.m_y, m_p1.m_z, dgFloat32(0.0f));
	box[6] = dgVector(m_p1.m_x, m_p1.m_y, m_p0.m_z, dgFloat32(0.0f));
	box[7] = dgVector(m_p1.m_x, m_p1.m_y, m_p1.m_z, dgFloat32(0.0f));

	dgFloat32 dist = dgFloat32(1.0e10f);
	for (dgInt32 i = 0; i < count; i++) {
		for (dgInt32 j = 0; j < 8; j++) {
			dgVector dp(points[i] - box[j]);
			dgFloat32 dist1 = dp % dp;
			if (dist1 < dist) {
				dist = dist1;
			}
		}
	}

	return dist;
}

dgCollisionCompound::dgCollisionCompound(dgWorld *world) : dgCollision(world->GetAllocator(), 0, dgGetIdentityMatrix(),
	        m_compoundCollision) {
	m_world = world;
	m_root = NULL;
	m_count = 0;
}

dgCollisionCompound::dgCollisionCompound(dgInt32 count,
        dgCollisionConvex *const shapeArray[], dgWorld *world) : dgCollision(world->GetAllocator(), 0, dgGetIdentityMatrix(),
	                m_compoundCollision) {
	m_world = world;
	m_root = NULL;
	if (count) {
		m_root = BuildTree(count, shapeArray);
	}

	Init(count, shapeArray);
}

dgCollisionCompound::dgCollisionCompound(const dgCollisionCompound &source) : dgCollision(source.GetAllocator(), 0, dgGetIdentityMatrix(),
	        m_compoundCollision) {
	int stack;
	dgNodeBase *pool[DG_COMPOUND_STACK_DEPTH];
	dgNodeBase **parent[DG_COMPOUND_STACK_DEPTH];

	m_root = NULL;
	m_world = source.m_world;

	parent[0] = &m_root;
	pool[0] = source.m_root;

	stack = 1;
	while (stack) {
		dgNodeBase *node;
		dgNodeBase **parentNode;

		stack--;
		node = pool[stack];
		parentNode = parent[stack];

		if (node->m_type == m_leaf) {
			*parentNode = new (m_allocator) dgNodeBase(node->m_shape, node->m_id);
		} else {
			dgNodeBase *newNode;
			newNode = new (m_allocator) dgNodeBase(*node);
			if (!m_root) {
				m_root = newNode;
			}
			*parentNode = newNode;

			pool[stack] = node->m_left;
			parent[stack] = &newNode->m_left;
			stack++;

			pool[stack] = node->m_right;
			parent[stack] = &newNode->m_right;
			stack++;
		}
	}

	Init(source.m_count, source.m_array);
	m_preCollisionFilter = source.m_preCollisionFilter;
}

dgCollisionCompound::dgCollisionCompound(dgWorld *const world,
        dgDeserialize deserialization, void *const userData) : dgCollision(world, deserialization, userData) {
	dgInt32 stack;
	dgInt32 count;
	dgInt32 data[4];
	dgNodeBase *pool[DG_COMPOUND_STACK_DEPTH];
	//  dgNodeBase* parentPool[DG_COMPOUND_STACK_DEPTH];

	deserialization(userData, data, sizeof(data));

	count = data[0];
	m_world = world;

	dgStack<dgCollisionConvex *> array(data[0]);
	for (dgInt32 i = 0; i < count; i++) {
		dgCollision *collision;
		collision = world->CreateFromSerialization(deserialization, userData);
		array[i] = (dgCollisionConvex *)collision;
	}

	struct Data : public dgNodeBase {
		Data() {
		}
		~Data() {
			this->reset();
		}

		dgInt8 m_padding[128];
	};

	m_root = NULL;
	Data nodeInfo;
	for (dgInt32 i = 0; i < 2 * count - 1; i++) {
		dgNodeBase *newNode;
		deserialization(userData, &nodeInfo.m_p0, sizeof(dgNodeBase));

		if (nodeInfo.m_type == m_leaf) {
			dgNodeBase *cell;
			cell = new (m_allocator) dgNodeBase(nodeInfo);
			cell->m_shape = array[nodeInfo.m_id];
			cell->m_shape->AddRef();
			newNode = cell;
		} else {
			dgNodeBase *node;
			node = new (m_allocator) dgNodeBase((dgNodeBase &)nodeInfo);
			node->m_left = NULL;
			node->m_right = NULL;
			newNode = node;
		}
		if (!m_root) {
			m_root = newNode;
		} else {
			stack = 1;
			pool[0] = m_root;
			while (stack) {
				dgNodeBase *node;

				stack--;
				node = pool[stack];

				//          if (node->m_id == dgInt32 (newNode->m_parent)) {
				if (((dgNodeBase *)dgInt64(node->m_id)) == newNode->m_parent) {
					if (node->m_left == NULL) {
						node->m_left = newNode;
					} else {
						NEWTON_ASSERT(!node->m_right);
						node->m_right = newNode;
					}
					break;
				}

				if (node->m_type == m_node) {
					if (node->m_left) {
						pool[stack] = node->m_left;
						stack++;
					}
					if (node->m_right) {
						pool[stack] = node->m_right;
						stack++;
					}
				}
			}
		}
	}

	Init(count, &array[0]);
	for (dgInt32 i = 0; i < count; i++) {
		world->ReleaseCollision(array[i]);
	}
}

dgCollisionCompound::~dgCollisionCompound() {
	if (m_root) {
		delete m_root;
	}

	for (dgInt32 i = 0; i < m_count; i++) {
		m_world->ReleaseCollision(m_array[i]);
	}
	m_allocator->Free(m_array);
}

void dgCollisionCompound::Init(dgInt32 count,
                               dgCollisionConvex *const shapeArray[]) {
	m_count = count;
	m_rtti |= dgCollisionCompound_RTTI;
	m_preCollisionFilter = NULL;

	m_array = (dgCollisionConvex **)m_allocator->Malloc(
	              m_count * dgInt32(sizeof(dgCollisionConvex *)));
	for (dgInt32 i = 0; i < m_count; i++) {
		m_array[i] = shapeArray[i];
		m_array[i]->AddRef();
	}

	m_boxMinRadius = GetMin(m_root->m_size.m_x, m_root->m_size.m_y,
	                        m_root->m_size.m_z);
	m_boxMaxRadius = dgSqrt(m_root->m_size % m_root->m_size);

	LinkParentNodes();
}

void dgCollisionCompound::LinkParentNodes() {
	dgInt32 stack;
	dgNodeBase *pool[DG_COMPOUND_STACK_DEPTH];
	dgNodeBase *parentPool[DG_COMPOUND_STACK_DEPTH];

	pool[0] = m_root;
	parentPool[0] = NULL;
	stack = 1;
	while (stack) {
		dgNodeBase *node;
		dgNodeBase *parent;

		stack--;
		node = pool[stack];
		parent = parentPool[stack];
		node->m_parent = parent;

		if (node->m_type == m_node) {
			parentPool[stack] = node;
			pool[stack] = node->m_right;
			stack++;

			parentPool[stack] = node;
			pool[stack] = node->m_left;
			stack++;
		}
	}
}

void dgCollisionCompound::SetCollisionBBox(const dgVector &p0__,
        const dgVector &p1__) {
	NEWTON_ASSERT(0);
}

dgInt32 dgCollisionCompound::CalculateSignature() const {
	NEWTON_ASSERT(0);
	return 0;
}

void dgCollisionCompound::CalcAABB(const dgMatrix &matrix, dgVector &p0,
                                   dgVector &p1) const {
	dgVector origin(matrix.TransformVector(m_root->m_origin));
	dgVector size(
	    m_root->m_size.m_x * dgAbsf(matrix[0][0]) + m_root->m_size.m_y * dgAbsf(matrix[1][0]) + m_root->m_size.m_z * dgAbsf(matrix[2][0]) + DG_MAX_COLLISION_PADDING,
	    m_root->m_size.m_x * dgAbsf(matrix[0][1]) + m_root->m_size.m_y * dgAbsf(matrix[1][1]) + m_root->m_size.m_z * dgAbsf(matrix[2][1]) + DG_MAX_COLLISION_PADDING,
	    m_root->m_size.m_x * dgAbsf(matrix[0][2]) + m_root->m_size.m_y * dgAbsf(matrix[1][2]) + m_root->m_size.m_z * dgAbsf(matrix[2][2]) + DG_MAX_COLLISION_PADDING,
	    dgFloat32(0.0f));

	p0 = origin - size;
	p1 = origin + size;
}

void dgCollisionCompound::CalcAABBSimd(const dgMatrix &matrix, dgVector &p0,
                                       dgVector &p1) const {

	dgVector origin(matrix.TransformVector(m_root->m_origin));
	dgVector size(
	    m_root->m_size.m_x * dgAbsf(matrix[0][0]) + m_root->m_size.m_y * dgAbsf(matrix[1][0]) + m_root->m_size.m_z * dgAbsf(matrix[2][0]) + DG_MAX_COLLISION_PADDING,
	    m_root->m_size.m_x * dgAbsf(matrix[0][1]) + m_root->m_size.m_y * dgAbsf(matrix[1][1]) + m_root->m_size.m_z * dgAbsf(matrix[2][1]) + DG_MAX_COLLISION_PADDING,
	    m_root->m_size.m_x * dgAbsf(matrix[0][2]) + m_root->m_size.m_y * dgAbsf(matrix[1][2]) + m_root->m_size.m_z * dgAbsf(matrix[2][2]) + DG_MAX_COLLISION_PADDING,
	    dgFloat32(0.0f));

	p0 = origin - size;
	p1 = origin + size;
}

// void dgCollisionCompound::dgMatrix& matrix, DebugCollisionMeshCallback callback) const
void dgCollisionCompound::DebugCollision(const dgMatrix &matrix,
        OnDebugCollisionMeshCallback callback, void *const userData) const {
	for (dgInt32 i = 0; i < m_count; i++) {
		m_array[i]->DebugCollision(matrix, callback, userData);
	}
}

dgFloat32 dgCollisionCompound::RayCast(const dgVector &localP0,
                                       const dgVector &localP1, dgContactPoint &contactOut,
                                       OnRayPrecastAction preFilter, const dgBody *const body,
                                       void *const userData) const {
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	if (!m_root) {
		return dgFloat32(1.2f);
	}

	dgInt32 stack = 1;
	stackPool[0] = m_root;
	dgFloat32 maxParam = dgFloat32(1.2f);

	dgFastRayTest ray(localP0, localP1);
	while (stack) {
		stack--;
		const dgNodeBase *const me = stackPool[stack];

		if (me && ray.BoxTest(me->m_p0, me->m_p1)) {
			if (me->m_type == m_leaf) {
				dgFloat32 param;
				dgContactPoint tmpContactOut;
				dgCollisionConvex *const shape = me->m_shape;

				dgVector p0(shape->m_offset.UntransformVector(localP0));
				dgVector p1(shape->m_offset.UntransformVector(localP1));
				// param = shape->RayCast (p0, p1, tmpContactOut, NULL, NULL, NULL);
				param = shape->RayCast(p0, p1, tmpContactOut, preFilter, body,
				                       userData);
				if (param < maxParam) {
					maxParam = param;
					contactOut.m_normal = shape->m_offset.RotateVector(
					                          tmpContactOut.m_normal);
					;
					contactOut.m_userId = tmpContactOut.m_userId;
					ray.Reset(maxParam);
				}

			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				stackPool[stack] = me->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack] = me->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
			}
		}
	}
	return maxParam;
}

dgFloat32 dgCollisionCompound::RayCastSimd(const dgVector &localP0,
        const dgVector &localP1, dgContactPoint &contactOut,
        OnRayPrecastAction preFilter, const dgBody *const body,
        void *const userData) const {
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	if (!m_root) {
		return dgFloat32(1.2f);
	}

	dgInt32 stack = 1;
	stackPool[0] = m_root;
	dgFloat32 maxParam = dgFloat32(1.2f);

	dgFastRayTest ray(localP0, localP1);
	while (stack) {
		stack--;
		const dgNodeBase *const me = stackPool[stack];

		if (me && ray.BoxTestSimd(me->m_p0, me->m_p1)) {
			if (me->m_type == m_leaf) {
				dgContactPoint tmpContactOut;
				dgCollisionConvex *const shape = me->m_shape;

				dgVector p0(shape->m_offset.UntransformVector(localP0));
				dgVector p1(shape->m_offset.UntransformVector(localP1));
				// param = shape->RayCastSimd (p0, p1, tmpContactOut, NULL, NULL, NULL);
				dgFloat32 param = shape->RayCastSimd(p0, p1, tmpContactOut, preFilter,
				                                     body, userData);
				if (param < maxParam) {
					maxParam = param;
					contactOut.m_normal = shape->m_offset.RotateVector(
					                          tmpContactOut.m_normal);
					contactOut.m_userId = tmpContactOut.m_userId;
					ray.Reset(maxParam);
				}

			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				stackPool[stack] = me->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack] = me->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
			}
		}
	}
	return maxParam;
}

dgFloat32 dgCollisionCompound::GetVolume() const {
	dgFloat32 volume = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < m_count; i++) {
		volume += m_array[i]->GetVolume();
	}
	return volume;
}

dgFloat32 dgCollisionCompound::GetBoxMinRadius() const {
	return m_boxMinRadius;
}

dgFloat32 dgCollisionCompound::GetBoxMaxRadius() const {
	return m_boxMaxRadius;
}

dgVector dgCollisionCompound::CalculateVolumeIntegral(
    const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane,
    void *const context) const {
	dgInt32 i;
	dgFloat32 scale;

	dgVector totalVolume(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                     dgFloat32(0.0f));
	for (i = 0; i < m_count; i++) {
		dgMatrix matrix(m_array[i]->m_offset * globalMatrix);
		//      dgVector vol (m_array[i]->CalculateVolumeIntegral (m_collisionMatrix[i], bouyancyPlane, context));
		dgVector vol(
		    m_array[i]->CalculateVolumeIntegral(matrix, bouyancyPlane, context));
		totalVolume.m_x += vol.m_x * vol.m_w;
		totalVolume.m_y += vol.m_y * vol.m_w;
		totalVolume.m_z += vol.m_z * vol.m_w;
		totalVolume.m_w += vol.m_w;
	}

	scale = dgFloat32(1.0f) / (totalVolume.m_w + dgFloat32(1.0e-6f));
	totalVolume.m_x *= scale;
	totalVolume.m_y *= scale;
	totalVolume.m_z *= scale;
	return totalVolume;
}

void dgCollisionCompound::CalculateInertia(dgVector &inertia,
        dgVector &origin) const {
	dgInt32 i;
	dgCollisionConvex *collision;

	dgFloat32 invVolume;
	dgFloat32 totalVolume;
	dgVector tmpOrigin;
	dgVector tmpInertia;
	dgVector tmpCrossInertia;
	dgVector totalOrigin(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                     dgFloat32(0.0f));
	dgVector totalInertia(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                      dgFloat32(0.0f));
	dgVector totalCrossInertia(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                           dgFloat32(0.0f));

#define DG_MIN_SIDE dgFloat32(1.0e-2f)
#define DG_MIN_VOLUME (DG_MIN_SIDE * DG_MIN_SIDE * DG_MIN_SIDE)

	totalVolume = dgFloat32(0.0f);
	for (i = 0; i < m_count; i++) {
		collision = m_array[i];
		totalVolume += collision->CalculateMassProperties(tmpInertia,
		               tmpCrossInertia, tmpOrigin);
		totalOrigin += tmpOrigin;
		totalInertia += tmpInertia;
		totalCrossInertia += tmpCrossInertia;
	}

	invVolume = dgFloat32(1.0f) / GetMax(totalVolume, DG_MIN_VOLUME);

	origin = totalOrigin.Scale(invVolume);
	totalInertia = totalInertia.Scale(invVolume);
	totalCrossInertia = totalCrossInertia.Scale(invVolume);

	inertia.m_x = totalInertia.m_x - (origin.m_y * origin.m_y + origin.m_z * origin.m_z);
	inertia.m_y = totalInertia.m_y - (origin.m_z * origin.m_z + origin.m_x * origin.m_x);
	inertia.m_z = totalInertia.m_z - (origin.m_x * origin.m_x + origin.m_y * origin.m_y);
	//  crossInertia.m_x += totalCrossInertia.m_x + origin.m_y * origin.m_z;
	//  crossInertia.m_y += totalCrossInertia.m_x + origin.m_z * origin.m_x;
	//  crossInertia.m_z += totalCrossInertia.m_x + origin.m_x * origin.m_y;

	NEWTON_ASSERT(inertia[0] > 0.0f);
	NEWTON_ASSERT(inertia[1] > 0.0f);
	NEWTON_ASSERT(inertia[2] > 0.0f);
}

void dgCollisionCompound::AddCollision(dgCollisionConvex *part) {
	NEWTON_ASSERT(0);
	/*
	 dgInt32 i;
	 dgInt8 *ptr;
	 dgCollisionConvex** array;
	 dgMatrix* collisionMatrix;
	 AABB* aabb;

	 NEWTON_ASSERT (0);
	 if (m_count >= m_maxCount) {
	 m_maxCount = m_maxCount * 2;

	 ptr = (dgInt8*) m_allocator->Malloc (m_maxCount * (sizeof (dgMatrix) + sizeof (dgCollisionConvex*) + sizeof(AABB)));

	 collisionMatrix = (dgMatrix*) ptr;
	 aabb = (AABB*) &ptr [m_maxCount * sizeof (dgMatrix)];
	 array = (dgCollisionConvex**) &ptr [m_maxCount * (sizeof (dgMatrix) + sizeof(AABB))];
	 for (i = 0; i < m_count; i ++) {
	 array[i] = m_array[i];
	 collisionMatrix[i] = m_collisionMatrix[i];
	 aabb[i] = m_aabb[i];
	 }
	 m_allocator->Free (m_collisionMatrix);

	 m_aabb = aabb;
	 m_array = array;
	 m_collisionMatrix = collisionMatrix;
	 }

	 m_array[m_count] = part;
	 m_array[m_count]->AddRef();
	 m_count ++;
	 */
}

void dgCollisionCompound::RemoveCollision(dgNodeBase *treeNode) {
	m_array[treeNode->m_id]->Release();
	m_count--;
	m_array[treeNode->m_id] = m_array[m_count];

	if (!treeNode->m_parent) {
		delete (m_root);
		m_root = NULL;
	} else if (!treeNode->m_parent->m_parent) {
		dgNodeBase *const root = m_root;
		if (treeNode->m_parent->m_left == treeNode) {
			m_root = treeNode->m_parent->m_right;
			treeNode->m_parent->m_right = NULL;
		} else {
			m_root = treeNode->m_parent->m_left;
			treeNode->m_parent->m_left = NULL;
		}
		m_root->m_parent = NULL;
		delete (root);

	} else {
		dgNodeBase *const root = treeNode->m_parent->m_parent;
		if (treeNode->m_parent == root->m_left) {
			if (treeNode->m_parent->m_right == treeNode) {
				root->m_left = treeNode->m_parent->m_left;
				treeNode->m_parent->m_left = NULL;
			} else {
				root->m_left = treeNode->m_parent->m_right;
				treeNode->m_parent->m_right = NULL;
			}
			root->m_left->m_parent = root;
		} else {
			if (treeNode->m_parent->m_right == treeNode) {
				root->m_right = treeNode->m_parent->m_left;
				treeNode->m_parent->m_left = NULL;
			} else {
				root->m_right = treeNode->m_parent->m_right;
				treeNode->m_parent->m_right = NULL;
			}
			root->m_right->m_parent = root;
		}
		delete (treeNode->m_parent);
	}
}

dgVector dgCollisionCompound::SupportVertex(const dgVector &dir) const {
	dgInt32 ix;
	dgInt32 iy;
	dgInt32 iz;
	dgInt32 stack;
	dgFloat32 maxProj;
	dgFloat32 aabbProjection[DG_COMPOUND_STACK_DEPTH];
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	stack = 1;
	stackPool[0] = m_root;
	aabbProjection[0] = dgFloat32(1.0e10f);

	maxProj = dgFloat32(-1.0e20f);
	dgVector searchDir(m_offset.UnrotateVector(dir));

	ix = (searchDir[0] > dgFloat32(0.0f)) ? 1 : 0;
	iy = (searchDir[1] > dgFloat32(0.0f)) ? 1 : 0;
	iz = (searchDir[2] > dgFloat32(0.0f)) ? 1 : 0;
	dgVector supportVertex(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                       dgFloat32(0.0f));

	while (stack) {
		dgFloat32 boxSupportValue;

		stack--;
		boxSupportValue = aabbProjection[stack];
		if (boxSupportValue > maxProj) {
			const dgNodeBase *const me = stackPool[stack];

			if (me->m_type == m_leaf) {

				dgFloat32 dist;
				dgCollision *const shape = me->m_shape;

				dgVector newDir(shape->m_offset.UnrotateVector(searchDir));
				dgVector vertex(
				    shape->m_offset.TransformVector(shape->SupportVertex(newDir)));
				dist = dir % vertex;
				if (dist > maxProj) {
					maxProj = dist;
					supportVertex = vertex;
				}

			} else {
				dgFloat32 dist0;
				dgFloat32 dist1;

				const dgNodeBase *const left = me->m_left;
				const dgNodeBase *const right = me->m_right;

				const dgVector *const box0 = &left->m_p0;
				dgVector p0(box0[ix].m_x, box0[iy].m_y, box0[iz].m_z, dgFloat32(0.0f));

				const dgVector *const box1 = &right->m_p0;
				dgVector p1(box1[ix].m_x, box1[iy].m_y, box1[iz].m_z, dgFloat32(0.0f));

				dist0 = p0 % dir;
				dist1 = p1 % dir;
				if (dist0 > dist1) {
					stackPool[stack] = right;
					aabbProjection[stack] = dist1;
					stack++;

					stackPool[stack] = left;
					aabbProjection[stack] = dist0;
					stack++;
				} else {
					stackPool[stack] = left;
					aabbProjection[stack] = dist0;
					stack++;

					stackPool[stack] = right;
					aabbProjection[stack] = dist1;
					stack++;
				}
			}
		}
	}

	return m_offset.TransformVector(supportVertex);
}

void dgCollisionCompound::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollision::GetCollisionInfo(info);

	info->m_offsetMatrix = GetOffsetMatrix();
	info->m_compoundCollision.m_chidrenCount = m_count;
	info->m_compoundCollision.m_chidren = (dgCollision **)m_array;
	info->m_collisionType = m_compoundCollision;
}

void dgCollisionCompound::Serialize(dgSerialize callback,
                                    void *const userData) const {
	dgInt32 stack;
	dgInt32 data[4];
	dgNodeBase *pool[DG_COMPOUND_STACK_DEPTH];

	data[0] = m_count;
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;

	SerializeLow(callback, userData);
	callback(userData, &data, sizeof(data));

	for (dgInt32 i = 0; i < m_count; i++) {
		dgCollision *collision;

		collision = m_array[i];

		m_world->Serialize(collision, callback, userData);
	}

	pool[0] = m_root;

	stack = 1;
	while (stack) {
		dgNodeBase *node;
		dgNodeBase *parent;

		stack--;
		node = pool[stack];

		parent = NULL;
		if (node->m_parent) {
			parent = node->m_parent;
			node->m_parent = (dgNodeBase *)(dgInt64(node->m_parent->m_id));
		}
		callback(userData, &node->m_p0, sizeof(dgNodeBase));
		node->m_parent = parent;

		if (node->m_type == m_node) {
			pool[stack] = node->m_right;
			stack++;
			pool[stack] = node->m_left;
			stack++;
		}
	}
}

bool dgCollisionCompound::OOBBTest(const dgMatrix &matrix,
                                   const dgCollisionConvex *const shape, void *const cacheOrder) const {
	NEWTON_ASSERT(0);
	return true;
}

/*
 dgInt32 dgCollisionCompound::GetAxis (dgNodeBase** const proxiArray, dgInt32 boxCount) const
 {
 dgInt32 axis;
 dgFloat32 maxVal;
 dgVector median (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
 dgVector varian (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
 for (dgInt32 i = 0; i < boxCount; i ++) {

 const dgNodeBase* const proxy = proxiArray[i];
 const dgVector& p0 = proxy->m_p0;
 const dgVector& p1 = proxy->m_p1;

 median += p0;
 median += p1;

 varian += p0.CompProduct(p0);
 varian += p1.CompProduct(p1);
 }

 boxCount *= 2;
 varian.m_x = boxCount * varian.m_x - median.m_x * median.m_x;
 varian.m_y = boxCount * varian.m_y - median.m_y * median.m_y;
 varian.m_z = boxCount * varian.m_z - median.m_z * median.m_z;

 axis = 0;
 maxVal = varian[0];
 for (dgInt32 i = 1; i < 3; i ++) {
 if (varian[i] > maxVal) {
 axis = i;
 maxVal = varian[i];
 }
 }
 return axis;
 }

 dgInt32 dgCollisionCompound::CompareBox (const dgNodeBase* const boxA, const dgNodeBase* const boxB, void* const context)
 {
 dgInt32 axis;

 axis = *((dgInt32*) context);

 if (boxA->m_p0[axis] < boxB->m_p0[axis]) {
 return -1;
 } else if (boxA->m_p0[axis] > boxB->m_p0[axis]) {
 return 1;
 }
 return 0;
 }


 dgCollisionCompound::dgNodeBase* dgCollisionCompound::BuildBottomUpTree(dgInt32 count, dgNodeBase** const proxiArray)
 {
 dgInt32 id;
 dgStack<dgHeapNodePair> pool (count / 4 + DG_COMPOUND_STACK_DEPTH);
 dgDownHeap<dgHeapNodePair, dgFloat32> heap (&pool[0], pool.GetSizeInBytes() - 64);

 id = count;
 while (count > 1){
 dgInt32 axis;
 dgInt32 newCount;

 axis = GetAxis (proxiArray, count);

 dgSortIndirect (proxiArray, count, CompareBox, &axis);
 heap.Flush();

 for (dgInt32 i = 0; i < count - 1; i ++) {

 dgInt32 bestProxi;
 dgFloat32 smallestVolume;
 dgFloat32 breakValue;
 dgNodeBase* nodeA;

 nodeA = proxiArray[i];
 bestProxi = -1;
 smallestVolume = dgFloat32 (1.0e20f);
 breakValue = ((count - i) < 32) ? dgFloat32 (1.0e20f) : nodeA->m_p1[axis] + dgFloat32 (2.0f);
 if (breakValue < proxiArray[i + 1]->m_p0[axis]) {
 breakValue = proxiArray[i + 1]->m_p0[axis] + dgFloat32 (2.0f);
 }

 for (dgInt32 j = i + 1; (j < count) && (proxiArray[j]->m_p0[axis] < breakValue); j ++) {

 dgFloat32 volume;
 dgVector p0;
 dgVector p1;
 dgNodeBase* nodeB;

 nodeB = proxiArray[j];
 p0.m_x = GetMin (nodeA->m_p0.m_x, nodeB->m_p0.m_x);
 p0.m_y = GetMin (nodeA->m_p0.m_y, nodeB->m_p0.m_y);
 p0.m_z = GetMin (nodeA->m_p0.m_z, nodeB->m_p0.m_z);
 p0.m_w = dgFloat32 (0.0f);
 p1.m_x = GetMax (nodeA->m_p1.m_x, nodeB->m_p1.m_x);
 p1.m_y = GetMax (nodeA->m_p1.m_y, nodeB->m_p1.m_y);
 p1.m_z = GetMax (nodeA->m_p1.m_z, nodeB->m_p1.m_z);
 p1.m_w = dgFloat32 (0.0f);
 dgVector dist (p1 - p0);
 volume = dist.m_x * dist.m_y * dist.m_z;
 if (volume < smallestVolume) {
 bestProxi = j;
 smallestVolume = volume;
 }
 }

 NEWTON_ASSERT (bestProxi != -1);

 dgHeapNodePair pair;
 pair.m_nodeA = i;
 pair.m_nodeB = bestProxi;

 if (heap.GetCount() < heap.GetMaxCount()) {
 heap.Push(pair, smallestVolume);
 } else {
 if (smallestVolume < heap.Value()) {
 heap.Pop();
 heap.Push(pair, smallestVolume);
 }
 }
 }

 heap.Sort ();

 for (dgInt32 j = heap.GetCount() - 1; j >= 0; j --) {
 dgHeapNodePair pair (heap[j]);

 if ((proxiArray[pair.m_nodeA]->m_p0.m_w == dgFloat32 (0.0f)) && (proxiArray[pair.m_nodeB]->m_p0.m_w == dgFloat32 (0.0f))) {
 proxiArray[pair.m_nodeA]->m_p0.m_w = dgFloat32 (1.0f);
 proxiArray[pair.m_nodeB]->m_p0.m_w = dgFloat32 (1.0f);

 proxiArray[count] = new (m_allocator) dgNodeBase (proxiArray[pair.m_nodeA], proxiArray[pair.m_nodeB], id);
 proxiArray[pair.m_nodeA]->m_parent = proxiArray[count];
 proxiArray[pair.m_nodeB]->m_parent = proxiArray[count];

 id ++;
 count ++;
 }
 }

 newCount = 0;
 for (dgInt32 i = 0; i < count; i ++) {
 if (proxiArray[i]->m_p0.m_w == dgFloat32 (0.0f)) {
 proxiArray[newCount] = proxiArray[i];
 newCount ++;
 }
 }

 NEWTON_ASSERT (newCount < count);
 count = newCount;
 }
 return proxiArray[0];
 }
 */

dgCollisionCompound::dgNodeBase *dgCollisionCompound::BuildTopDownTree(
    dgInt32 count, dgNodeBase **const proxiArray, dgInt32 &id) {
	dgNodeBase *tree = NULL;
	if (count == 1) {
		tree = proxiArray[0];
	} else {
		dgInt32 i0 = 1;
		if (count > 2) {
			dgVector median(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                dgFloat32(0.0f));
			dgVector varian(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                dgFloat32(0.0f));
			for (dgInt32 i = 0; i < count; i++) {
				const dgNodeBase *const proxy = proxiArray[i];
				const dgVector &p0 = proxy->m_p0;
				const dgVector &p1 = proxy->m_p1;

				median += p0;
				median += p1;

				varian += p0.CompProduct(p0);
				varian += p1.CompProduct(p1);
			}

			dgInt32 pointCount = count * 2;
			varian.m_x = pointCount * varian.m_x - median.m_x * median.m_x;
			varian.m_y = pointCount * varian.m_y - median.m_y * median.m_y;
			varian.m_z = pointCount * varian.m_z - median.m_z * median.m_z;

			dgInt32 axis = 0;
			dgFloat32 maxVal = varian[0];
			for (dgInt32 i = 1; i < 3; i++) {
				if (varian[i] > maxVal) {
					axis = i;
					maxVal = varian[i];
				}
			}

			dgVector center = median.Scale(dgFloat32(1.0f) / dgFloat32(pointCount));
			dgFloat32 test = center[axis];

			dgInt32 i1 = count - 1;
			do {
				for (; i0 <= i1; i0++) {
					const dgNodeBase *const proxy = proxiArray[i0];
					dgFloat32 val = (proxy->m_p0[axis] + proxy->m_p1[axis]) * dgFloat32(0.5f);
					if (val > test) {
						break;
					}
				}

				for (; i1 >= i0; i1--) {
					const dgNodeBase *const proxy = proxiArray[i1];
					dgFloat32 val = (proxy->m_p0[axis] + proxy->m_p1[axis]) * dgFloat32(0.5f);
					if (val < test) {
						break;
					}
				}

				if (i0 < i1) {
					Swap(proxiArray[i0], proxiArray[i1]);
					i0++;
					i1--;
				}
			} while (i0 <= i1);

			if (i0 == 0) {
				i0 = 1;
			}
			if (i0 >= count - 1) {
				i0 = count - 1;
			}
		}

		dgNodeBase *const left = BuildTopDownTree(i0, &proxiArray[0], id);
		dgNodeBase *const right = BuildTopDownTree(count - i0, &proxiArray[i0], id);
		tree = new (m_allocator) dgNodeBase(left, right, id);
		left->m_parent = tree;
		right->m_parent = tree;

		id++;
	}

	return tree;
}

void dgCollisionCompound::PushNodes(dgNodeBase *const root,
                                    dgNodeBase **const proxiArray, dgInt32 &index) const {
	if (root->m_left) {
		PushNodes(root->m_left, proxiArray, index);
	}
	if (root->m_right) {
		PushNodes(root->m_right, proxiArray, index);
	}
	if (!root->m_shape) {
		proxiArray[index] = root;
		index++;
	}
}

dgFloat32 dgCollisionCompound::CalculateSurfaceArea(dgNodeBase *const node0,
        dgNodeBase *const node1, dgVector &minBox, dgVector &maxBox) const {
	minBox = dgVector(GetMin(node0->m_p0.m_x, node1->m_p0.m_x),
	                  GetMin(node0->m_p0.m_y, node1->m_p0.m_y),
	                  GetMin(node0->m_p0.m_z, node1->m_p0.m_z), dgFloat32(0.0f));
	maxBox = dgVector(GetMax(node0->m_p1.m_x, node1->m_p1.m_x),
	                  GetMax(node0->m_p1.m_y, node1->m_p1.m_y),
	                  GetMax(node0->m_p1.m_z, node1->m_p1.m_z), dgFloat32(0.0f));
	dgVector side0((maxBox - minBox).Scale(dgFloat32(0.5f)));
	dgVector side1(side0.m_y, side0.m_z, side0.m_x, dgFloat32(0.0f));
	return side0 % side1;
}

void dgCollisionCompound::ImproveNodeFitness(dgNodeBase *const node) const {
	NEWTON_ASSERT(node->m_left);
	NEWTON_ASSERT(node->m_right);

	if (node->m_parent) {
		if (node->m_parent->m_left == node) {
			dgFloat32 cost0 = node->m_area;

			dgVector cost1P0;
			dgVector cost1P1;
			dgFloat32 cost1 = CalculateSurfaceArea(node->m_right,
			                                       node->m_parent->m_right, cost1P0, cost1P1);

			dgVector cost2P0;
			dgVector cost2P1;
			dgFloat32 cost2 = CalculateSurfaceArea(node->m_left,
			                                       node->m_parent->m_right, cost2P0, cost2P1);

			if ((cost1 <= cost0) && (cost1 <= cost2)) {
				dgNodeBase *const parent = node->m_parent;
				node->m_p0 = parent->m_p0;
				node->m_p1 = parent->m_p1;
				node->m_area = parent->m_area;
				node->m_size = parent->m_size;
				node->m_origin = parent->m_origin;

				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						NEWTON_ASSERT(parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_right->m_parent = parent;
				parent->m_left = node->m_right;
				node->m_right = parent;
				parent->m_p0 = cost1P0;
				parent->m_p1 = cost1P1;
				parent->m_area = cost1;
				parent->m_size = (parent->m_p1 - parent->m_p0).Scale(dgFloat32(0.5f));
				parent->m_origin = (parent->m_p1 + parent->m_p0).Scale(dgFloat32(0.5f));

			} else if ((cost2 <= cost0) && (cost2 <= cost1)) {
				dgNodeBase *const parent = node->m_parent;
				node->m_p0 = parent->m_p0;
				node->m_p1 = parent->m_p1;
				node->m_area = parent->m_area;
				node->m_size = parent->m_size;
				node->m_origin = parent->m_origin;

				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						NEWTON_ASSERT(parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_left->m_parent = parent;
				parent->m_left = node->m_left;
				node->m_left = parent;

				parent->m_p0 = cost2P0;
				parent->m_p1 = cost2P1;
				parent->m_area = cost2;
				parent->m_size = (parent->m_p1 - parent->m_p0).Scale(dgFloat32(0.5f));
				parent->m_origin = (parent->m_p1 + parent->m_p0).Scale(dgFloat32(0.5f));
			}
		} else {
			dgFloat32 cost0 = node->m_area;

			dgVector cost1P0;
			dgVector cost1P1;
			dgFloat32 cost1 = CalculateSurfaceArea(node->m_left,
			                                       node->m_parent->m_left, cost1P0, cost1P1);

			dgVector cost2P0;
			dgVector cost2P1;
			dgFloat32 cost2 = CalculateSurfaceArea(node->m_right,
			                                       node->m_parent->m_left, cost2P0, cost2P1);

			if ((cost1 <= cost0) && (cost1 <= cost2)) {

				dgNodeBase *const parent = node->m_parent;
				node->m_p0 = parent->m_p0;
				node->m_p1 = parent->m_p1;
				node->m_area = parent->m_area;
				node->m_size = parent->m_size;
				node->m_origin = parent->m_origin;

				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						NEWTON_ASSERT(parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_left->m_parent = parent;
				parent->m_right = node->m_left;
				node->m_left = parent;

				parent->m_p0 = cost1P0;
				parent->m_p1 = cost1P1;
				parent->m_area = cost1;
				parent->m_size = (parent->m_p1 - parent->m_p0).Scale(dgFloat32(0.5f));
				parent->m_origin = (parent->m_p1 + parent->m_p0).Scale(dgFloat32(0.5f));

			} else if ((cost2 <= cost0) && (cost2 <= cost1)) {
				dgNodeBase *const parent = node->m_parent;
				node->m_p0 = parent->m_p0;
				node->m_p1 = parent->m_p1;
				node->m_area = parent->m_area;
				node->m_size = parent->m_size;
				node->m_origin = parent->m_origin;

				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						NEWTON_ASSERT(parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_right->m_parent = parent;
				parent->m_right = node->m_right;
				node->m_right = parent;

				parent->m_p0 = cost2P0;
				parent->m_p1 = cost2P1;
				parent->m_area = cost2;
				parent->m_size = (parent->m_p1 - parent->m_p0).Scale(dgFloat32(0.5f));
				parent->m_origin = (parent->m_p1 + parent->m_p0).Scale(dgFloat32(0.5f));
			}
		}
	} else {
		// in the future I can handle this but it is too much work for little payoff
	}
}

dgCollisionCompound::dgNodeBase *dgCollisionCompound::BuildTree(dgInt32 count,
        dgCollisionConvex *const shapeArray[]) {

#if 0
	dgStack<dgNodeBase *> nodeList(count * 2);
	dgNodeBase **const proxiArray = &nodeList[0];
	for (dgInt32 i = 0; i < count; i ++) {
		proxiArray[i] = new (m_allocator) dgNodeBase(shapeArray[i], i);
	}

	dgNodeBase *tree = BuildBottomUpTree(count, proxiArray);
#else
	dgStack<dgNodeBase *> nodeList(count);
	dgNodeBase **const proxiArray = &nodeList[0];
	for (dgInt32 i = 0; i < count; i++) {
		proxiArray[i] = new (m_allocator) dgNodeBase(shapeArray[i], i);
	}

	dgInt32 id = count;
	dgNodeBase *tree = BuildTopDownTree(count, proxiArray, id);
#endif

	dgInt32 index = 0;
	PushNodes(tree, proxiArray, index);

	dgInt32 maxPasses = 2 * exp_2(index * 2) + 1;
	dgFloat64 newCost = dgFloat32(1.0e20f);
	dgFloat64 prevCost = newCost;
	do {
		prevCost = newCost;
		for (dgInt32 i = 0; i < index; i++) {
			dgNodeBase *const node = proxiArray[i];
			ImproveNodeFitness(node);
		}
		newCost = dgFloat32(0.0f);
		for (dgInt32 i = 0; i < index; i++) {
			dgNodeBase *const node = proxiArray[i];
			newCost += node->m_area;
		}

		maxPasses--;
	} while (maxPasses && (newCost < prevCost));

	if (tree->m_parent) {
		NEWTON_ASSERT(index);
		for (tree = proxiArray[index - 1]; tree->m_parent; tree = tree->m_parent)
			;
	}
	return tree;
}

dgInt32 dgCollisionCompound::CalculateContacts(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgInt32 contactCount = 0;
	if (m_root) {
		NEWTON_ASSERT(IsType(dgCollision::dgCollisionCompound_RTTI));
		if (pair->m_body1->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
			contactCount = CalculateContactsToSingle(pair, proxy, useSimd);
		} else if (pair->m_body1->m_collision->IsType(
		               dgCollision::dgCollisionCompound_RTTI)) {
			contactCount = CalculateContactsToCompound(pair, proxy, useSimd);
		} else if (pair->m_body1->m_collision->IsType(
		               dgCollision::dgCollisionBVH_RTTI)) {
			contactCount = CalculateContactsToCollisionTree(pair, proxy, useSimd);
		} else if (pair->m_body1->m_collision->IsType(
		               dgCollision::dgCollisionHeightField_RTTI)) {
			contactCount = CalculateContactsToHeightField(pair, proxy, useSimd);
		} else {
			NEWTON_ASSERT(
			    pair->m_body1->m_collision->IsType(dgCollision::dgCollisionUserMesh_RTTI));
			contactCount = CalculateContactsBruteForce(pair, proxy, useSimd);
		}
	}
	return contactCount;
}

dgInt32 dgCollisionCompound::CalculateContactsToSingle(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgVector p0;
	dgVector p1;
	dgContactPoint *const contacts = pair->m_contactBuffer;
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	dgBody *const otherBody = pair->m_body1;
	dgBody *const compoundBody = pair->m_body0;

	NEWTON_ASSERT(pair->m_body0->m_collision == this);
	NEWTON_ASSERT(
	    otherBody->m_collision->IsType(dgCollision::dgConvexCollision_RTTI));

	// dgInt32 lru = m_world->m_broadPhaseLru;
	proxy.m_referenceBody = compoundBody;

	proxy.m_floatingBody = otherBody;
	proxy.m_floatingCollision = otherBody->m_collision;
	proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;

	dgInt32 contactCount = 0;
	dgMatrix myMatrix(m_offset * compoundBody->m_matrix);
	dgMatrix matrix(otherBody->m_collisionWorldMatrix * myMatrix.Inverse());
	otherBody->m_collision->CalcAABB(dgGetIdentityMatrix(), p0, p1);
	if (proxy.m_unconditionalCast) {
		dgVector step(
		    (otherBody->m_veloc - compoundBody->m_veloc).Scale(proxy.m_timestep));
		step = otherBody->m_collisionWorldMatrix.UnrotateVector(step);
		for (dgInt32 j = 0; j < 3; j++) {
			if (step[j] > dgFloat32(0.0f)) {
				p1[j] += step[j];
			} else {
				p0[j] += step[j];
			}
		}
	}

	OOBBTestData data(matrix, p0, p1);

	dgInt32 stack = 1;
	stackPool[0] = m_root;
	while (stack) {

		stack--;
		const dgNodeBase *const me = stackPool[stack];
		NEWTON_ASSERT(me);

		if (me->BoxTest(data)) {
			if (me->m_type == m_leaf) {
				dgInt32 processContacts;

				processContacts = 1;
				if (pair->m_material && pair->m_material->m_compoundAABBOverlap) {
					processContacts = pair->m_material->m_compoundAABBOverlap(
					                      *pair->m_material, *compoundBody, *otherBody,
					                      proxy.m_threadIndex);
				}
				if (processContacts) {
					proxy.m_referenceCollision = me->m_shape;

					proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

					//                  proxy.m_floatingCollision = otherBody->m_collision;
					//                  proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;

					proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
					proxy.m_contacts = &contacts[contactCount];

					if (useSimd) {
						contactCount += m_world->CalculateConvexToConvexContactsSimd(proxy);
					} else {
						contactCount += m_world->CalculateConvexToConvexContacts(proxy);
					}

					if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
						contactCount = m_world->ReduceContacts(contactCount, contacts,
						                                       DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
					}
				}

			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				stackPool[stack] = me->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack] = me->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
			}
		}
	}

	return contactCount;
}

dgInt32 dgCollisionCompound::CalculateContactsToCompound(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgContactPoint *const contacts = pair->m_contactBuffer;
	const dgNodeBase *stackPool[4 * DG_COMPOUND_STACK_DEPTH][2];

	dgInt32 contactCount = 0;
	dgBody *const myBody = pair->m_body0;
	dgBody *const otherBody = pair->m_body1;

	NEWTON_ASSERT(pair->m_body0->m_collision == this);
	dgCollisionCompound *const otherCompound =
	    (dgCollisionCompound *)otherBody->m_collision;

	// dgInt32 lru = m_world->m_broadPhaseLru;
	proxy.m_referenceBody = myBody;
	proxy.m_floatingBody = otherBody;

	dgMatrix myMatrix(m_offset * myBody->m_matrix);
	dgMatrix otherMatrix(otherCompound->m_offset * otherBody->m_matrix);
	OOBBTestData data(otherMatrix * myMatrix.Inverse());

	dgInt32 stack = 1;
	stackPool[0][0] = m_root;
	stackPool[0][1] = otherCompound->m_root;
	while (stack) {

		stack--;
		const dgNodeBase *const me = stackPool[stack][0];
		const dgNodeBase *const other = stackPool[stack][1];

		NEWTON_ASSERT(me && other);

		if (me->BoxTest(data, other)) {
			if ((me->m_type == m_leaf) && (other->m_type == m_leaf)) {
				dgInt32 processContacts;

				processContacts = 1;
				if (pair->m_material && pair->m_material->m_compoundAABBOverlap) {
					processContacts = pair->m_material->m_compoundAABBOverlap(
					                      *pair->m_material, *myBody, *otherBody, proxy.m_threadIndex);
				}
				if (processContacts) {
					proxy.m_referenceCollision = me->m_shape;
					proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

					proxy.m_floatingCollision = other->m_shape;
					proxy.m_floatingMatrix = other->m_shape->m_offset * otherMatrix;

					proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
					proxy.m_contacts = &contacts[contactCount];

					if (useSimd) {
						contactCount += m_world->CalculateConvexToConvexContactsSimd(proxy);
					} else {
						contactCount += m_world->CalculateConvexToConvexContacts(proxy);
					}
					if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
						contactCount = m_world->ReduceContacts(contactCount, contacts,
						                                       DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
					}
				}

			} else if (me->m_type == m_leaf) {
				//              dgNode* const otherNode = (dgNode*)other;
				NEWTON_ASSERT(other->m_type == m_node);

				stackPool[stack][0] = me;
				stackPool[stack][1] = other->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack][0] = me;
				stackPool[stack][1] = other->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

			} else if (other->m_type == m_leaf) {
				//      dgNode* const myNode = (dgNode*)me;
				NEWTON_ASSERT(me->m_type == m_node);

				stackPool[stack][0] = me->m_left;
				stackPool[stack][1] = other;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack][0] = me->m_right;
				stackPool[stack][1] = other;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				NEWTON_ASSERT(other->m_type == m_node);

				stackPool[stack][0] = me->m_left;
				stackPool[stack][1] = other->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack][0] = me->m_left;
				stackPool[stack][1] = other->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack][0] = me->m_right;
				stackPool[stack][1] = other->m_left;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack][0] = me->m_right;
				stackPool[stack][1] = other->m_right;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
			}
		}
	}

	return contactCount;
}

dgInt32 dgCollisionCompound::CalculateContactsToCollisionTree(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgContactPoint *const contacts = pair->m_contactBuffer;
	dgTree<const dgNodeBase *, const dgNodeBase *> filter(m_allocator);

	struct NodePairs {
		dgNodeBase *m_myNode;
		dgInt32 m_treeNodeIsLeaf;
		const void *m_treeNode;
	};
	NodePairs stackPool[4 * DG_COMPOUND_STACK_DEPTH];

	dgInt32 contactCount = 0;

	dgBody *const myBody = pair->m_body0;
	dgBody *const treeBody = pair->m_body1;

	NEWTON_ASSERT(pair->m_body0->m_collision == this);
	dgCollisionBVH *const treeCollision = (dgCollisionBVH *)treeBody->m_collision;

	//  dgInt32 lru = m_world->m_broadPhaseLru;
	proxy.m_referenceBody = myBody;
	proxy.m_floatingBody = treeBody;

	proxy.m_floatingCollision = treeCollision;
	proxy.m_floatingMatrix = treeBody->m_collisionWorldMatrix;

	dgMatrix myMatrix(m_offset * myBody->m_matrix);
	OOBBTestData data(proxy.m_floatingMatrix * myMatrix.Inverse());

	dgInt32 stack = 1;
	stackPool[0].m_myNode = m_root;
	stackPool[0].m_treeNode = treeCollision->GetRootNode();
	stackPool[0].m_treeNodeIsLeaf = 0;

	dgNodeBase nodeProxi;
	nodeProxi.m_left = NULL;
	nodeProxi.m_right = NULL;

	while (stack) {
		dgInt32 treeNodeIsLeaf;

		stack--;
		dgNodeBase *const me = stackPool[stack].m_myNode;
		const void *const other = stackPool[stack].m_treeNode;
		treeNodeIsLeaf = stackPool[stack].m_treeNodeIsLeaf;

		NEWTON_ASSERT(me && other);

		treeCollision->GetNodeAABB(other, nodeProxi.m_p0, nodeProxi.m_p1);
		nodeProxi.m_size = (nodeProxi.m_p1 - nodeProxi.m_p0).Scale(dgFloat32(0.5f));
		nodeProxi.m_origin = (nodeProxi.m_p1 + nodeProxi.m_p0).Scale(dgFloat32(0.5f));
		dgVector size(nodeProxi.m_size.m_y, nodeProxi.m_size.m_z,
		              nodeProxi.m_size.m_x, dgFloat32(0.0f));
		nodeProxi.m_area = nodeProxi.m_size % size;
		NEWTON_ASSERT(nodeProxi.m_area > dgFloat32(0.0f));

		if (me->BoxTest(data, &nodeProxi)) {
			if ((me->m_type == m_leaf) && treeNodeIsLeaf) {
				if (!filter.Find(me)) {
					m_world->dgGetUserLock();
					filter.Insert(me, me);
					m_world->dgReleasedUserLock();

					//                  dgShapeCell* const myCell = (dgShapeCell*)me;
					//                  m_world->dgGetIndirectLock(&myCell->m_criticalSection);
					//                  if (myCell->m_lru != lru) {
					//                      myCell->m_lru = lru;
					//                      myCell->m_collisionMatrix = myCell->m_shape->m_offset * myMatrix;
					//                  }
					//                  m_world->dgReleaseIndirectLock(&myCell->m_criticalSection);

					proxy.m_referenceCollision = me->m_shape;
					proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

					proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
					proxy.m_contacts = &contacts[contactCount];

					if (useSimd) {
						contactCount += m_world->CalculateConvexToNonConvexContactsSimd(
						                    proxy);
					} else {
						contactCount += m_world->CalculateConvexToNonConvexContacts(proxy);
					}
					if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
						contactCount = m_world->ReduceContacts(contactCount, contacts,
						                                       DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
					}
					if (filter.GetCount() == m_count) {
						break;
					}
				}

			} else if (me->m_type == m_leaf) {

				const void *const frontNode = treeCollision->GetFrontNode(other);
				const void *const backNode = treeCollision->GetBackNode(other);
				if (backNode && frontNode) {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = backNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = frontNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

				} else if (backNode && !frontNode) {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = backNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

				} else if (!backNode && frontNode) {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = frontNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

				} else {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;
				}

			} else if (treeNodeIsLeaf) {
				stackPool[stack].m_myNode = me->m_left;
				stackPool[stack].m_treeNode = other;
				stackPool[stack].m_treeNodeIsLeaf = 1;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

				stackPool[stack].m_myNode = me->m_right;
				stackPool[stack].m_treeNode = other;
				stackPool[stack].m_treeNodeIsLeaf = 1;
				stack++;
				NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

			} else if (nodeProxi.m_area > me->m_area) {
				NEWTON_ASSERT(me->m_type == m_node);
				const void *const frontNode = treeCollision->GetFrontNode(other);
				const void *const backNode = treeCollision->GetBackNode(other);
				if (backNode && frontNode) {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = backNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = frontNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;
				} else if (backNode && !frontNode) {
					stackPool[stack].m_myNode = (dgNodeBase *)me;
					stackPool[stack].m_treeNode = backNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = me->m_left;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

					stackPool[stack].m_myNode = me->m_right;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

				} else if (!backNode && frontNode) {
					stackPool[stack].m_myNode = me;
					stackPool[stack].m_treeNode = frontNode;
					stackPool[stack].m_treeNodeIsLeaf = 0;
					stack++;

					stackPool[stack].m_myNode = me->m_left;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

					stackPool[stack].m_myNode = me->m_right;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

				} else {
					stackPool[stack].m_myNode = me->m_left;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;

					stackPool[stack].m_myNode = me->m_right;
					stackPool[stack].m_treeNode = other;
					stackPool[stack].m_treeNodeIsLeaf = 1;
					stack++;
				}

			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				stackPool[stack].m_myNode = me->m_left;
				stackPool[stack].m_treeNode = other;
				stackPool[stack].m_treeNodeIsLeaf = treeNodeIsLeaf;
				stack++;

				stackPool[stack].m_myNode = me->m_right;
				stackPool[stack].m_treeNode = other;
				stackPool[stack].m_treeNodeIsLeaf = treeNodeIsLeaf;
				stack++;
			}
		}
	}

	if (filter.GetCount()) {
		m_world->dgGetUserLock();
		filter.RemoveAll();
		m_world->dgReleasedUserLock();
	}

	return contactCount;
}

dgInt32 dgCollisionCompound::CalculateContactsToHeightField(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgNodeBase nodeProxi;
	dgContactPoint *const contacts = pair->m_contactBuffer;
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	dgInt32 contactCount = 0;
	dgBody *const myBody = pair->m_body0;
	dgBody *const terrainBody = pair->m_body1;

	NEWTON_ASSERT(pair->m_body0->m_collision == this);
	dgCollisionHeightField *const terrainCollision =
	    (dgCollisionHeightField *)terrainBody->m_collision;

	//  dgInt32 lru = m_world->m_broadPhaseLru;
	proxy.m_referenceBody = myBody;
	proxy.m_floatingBody = terrainBody;

	proxy.m_floatingCollision = terrainCollision;
	proxy.m_floatingMatrix = terrainBody->m_collisionWorldMatrix;

	dgMatrix myMatrix(m_offset * myBody->m_matrix);
	OOBBTestData data(proxy.m_floatingMatrix * myMatrix.Inverse());

	dgInt32 stack = 1;
	stackPool[0] = m_root;

	nodeProxi.m_left = NULL;
	nodeProxi.m_right = NULL;
	while (stack) {
		stack--;
		const dgNodeBase *const me = stackPool[stack];

		dgVector origin(data.m_matrix.UntransformVector(me->m_origin));
		dgVector size(data.m_absMatrix.UnrotateVector(me->m_size));
		dgVector p0(origin - size);
		dgVector p1(origin + size);

		terrainCollision->GetLocalAABB(p0, p1, nodeProxi.m_p0, nodeProxi.m_p1);
		nodeProxi.m_size = (nodeProxi.m_p1 - nodeProxi.m_p0).Scale(dgFloat32(0.5f));
		nodeProxi.m_origin = (nodeProxi.m_p1 + nodeProxi.m_p0).Scale(dgFloat32(0.5f));
		if (me->BoxTest(data, &nodeProxi)) {
			if (me->m_type == m_leaf) {
				//              dgShapeCell* const myCell = (dgShapeCell*)me;

				//              m_world->dgGetIndirectLock(&myCell->m_criticalSection);
				//              if (myCell->m_lru != lru) {
				//                  myCell->m_lru = lru;
				//                  myCell->m_collisionMatrix = myCell->m_shape->m_offset * myMatrix;
				//              }
				//              m_world->dgReleaseIndirectLock(&myCell->m_criticalSection);

				proxy.m_referenceCollision = me->m_shape;
				proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

				proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
				proxy.m_contacts = &contacts[contactCount];

				if (useSimd) {
					contactCount += m_world->CalculateConvexToNonConvexContactsSimd(
					                    proxy);
				} else {
					contactCount += m_world->CalculateConvexToNonConvexContacts(proxy);
				}
				if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
					contactCount = m_world->ReduceContacts(contactCount, contacts,
					                                       DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
				}

			} else {
				NEWTON_ASSERT(me->m_type == m_node);
				stackPool[stack] = me->m_left;
				stack++;

				stackPool[stack] = me->m_right;
				stack++;
			}
		}
	}

	return contactCount;
}

dgInt32 dgCollisionCompound::CalculateContactsBruteForce(
    dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy,
    dgInt32 useSimd) const {
	dgContactPoint *const contacts = pair->m_contactBuffer;
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	dgInt32 contactCount = 0;

	dgBody *const myBody = pair->m_body0;
	dgBody *const userBody = pair->m_body1;

	NEWTON_ASSERT(pair->m_body0->m_collision == this);
	dgCollisionMesh *const userCollision =
	    (dgCollisionMesh *)userBody->m_collision;

	//  dgInt32 lru = m_world->m_broadPhaseLru;
	proxy.m_referenceBody = myBody;
	proxy.m_floatingBody = userBody;

	proxy.m_floatingCollision = userCollision;
	proxy.m_floatingMatrix = userBody->m_collisionWorldMatrix;

	dgMatrix myMatrix(m_offset * myBody->m_matrix);

	dgInt32 stack = 1;
	stackPool[0] = m_root;
	dgNodeBase nodeProxi;
	nodeProxi.m_left = NULL;
	nodeProxi.m_right = NULL;

	while (stack) {
		stack--;
		const dgNodeBase *const me = stackPool[stack];

		if (me->m_type == m_leaf) {
			//          dgShapeCell* const myCell = (dgShapeCell*)me;

			//          m_world->dgGetIndirectLock(&myCell->m_criticalSection);
			//          if (myCell->m_lru != lru) {
			//              myCell->m_lru = lru;
			//              myCell->m_collisionMatrix = myCell->m_shape->m_offset * myMatrix;
			//          }
			//          m_world->dgReleaseIndirectLock(&myCell->m_criticalSection);

			proxy.m_referenceCollision = me->m_shape;
			proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

			proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
			proxy.m_contacts = &contacts[contactCount];

			if (useSimd) {
				contactCount += m_world->CalculateConvexToNonConvexContactsSimd(proxy);
			} else {
				contactCount += m_world->CalculateConvexToNonConvexContacts(proxy);
			}
			if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
				contactCount = m_world->ReduceContacts(contactCount, contacts,
				                                       DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
			}

		} else {
			NEWTON_ASSERT(me->m_type == m_node);
			stackPool[stack] = me->m_left;
			stack++;

			stackPool[stack] = me->m_right;
			stack++;
		}
	}

	return contactCount;
}

dgInt32 dgCollisionCompound::ClosestDitance(dgBody *const compoundBody,
        dgTriplex &contactA, dgBody *const bodyB, dgTriplex &contactB,
        dgTriplex &normalAB) const {
	if (!m_root) {
		return 0;
	}

#if 0
	NEWTON_ASSERT(compoundBody->m_collision == this);
	dgCollisionConvex *const collisionB = (dgCollisionConvex *) bodyB->m_collision;

	//    dgInt32 lru;
	//    dgInt32 stack;
	//    dgInt32 contactCount;
	//    dgBody* otherBody;
	//    dgBody* compoundBody;
	//    dgVector p0;
	//    dgVector p1;
	//    dgContactPoint* const contacts = pair->m_contactBuffer;
	//    const dgNodeBase* stackPool[DG_COMPOUND_STACK_DEPTH];
	//    otherBody = pair->m_body1;
	//    compoundBody = pair->m_body0;

	//    NEWTON_ASSERT (pair->m_body0->m_collision == this);
	//    NEWTON_ASSERT (otherBody->m_collision->IsType (dgCollision::dgConvexCollision_RTTI));

	//    lru = m_world->m_broadPhaseLru;
	//    proxy.m_referenceBody = compoundBody;
	//    proxy.m_floatingBody = otherBody;
	//    proxy.m_floatingCollision = otherBody->m_collision;
	//    proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;
	//    contactCount = 0;

	dgVector p0;
	dgVector p1;
	dgMatrix myMatrix(m_offset * compoundBody->m_matrix);
	dgMatrix matrix(bodyB->m_collisionWorldMatrix * myMatrix.Inverse());
	collisionB->CalcAABB(dgGetIdentityMatrix(), p0, p1);

	dgFloat32 distPool[DG_COMPOUND_STACK_DEPTH];
	const dgNodeBase *stackPool[DG_COMPOUND_STACK_DEPTH];

	dgVector points[8];
	points[0] = dgVector(p0.m_x, p0.m_y, p0.m_z, dgFloat32(0.0f));
	points[1] = dgVector(p0.m_x, p0.m_y, p1.m_z, dgFloat32(0.0f));
	points[2] = dgVector(p0.m_x, p1.m_y, p0.m_z, dgFloat32(0.0f));
	points[3] = dgVector(p0.m_x, p1.m_y, p1.m_z, dgFloat32(0.0f));
	points[4] = dgVector(p1.m_x, p0.m_y, p0.m_z, dgFloat32(0.0f));
	points[5] = dgVector(p1.m_x, p0.m_y, p1.m_z, dgFloat32(0.0f));
	points[6] = dgVector(p1.m_x, p1.m_y, p0.m_z, dgFloat32(0.0f));
	points[7] = dgVector(p1.m_x, p1.m_y, p1.m_z, dgFloat32(0.0f));
	matrix.TransformTriplex(points, sizeof(dgVector), points, sizeof(dgVector), 8);

	dgInt32 stack = 1;
	stackPool[0] = m_root;
	distPool[0] = m_root->BoxClosestDistance(points, 8);

	dgFloat32 baseDist = dgFloat32(1.0e10f);
	while (stack) {
		stack --;

		dgFloat32 dist = distPool[stack];
		const dgNodeBase *const me = stackPool[stack];
		NEWTON_ASSERT(me);

//		if (me->BoxTest (data)) {
		if (dist < baseDist) {

			if (me->m_type == m_leaf) {
				NEWTON_ASSERT(0);
				/*
				 dgInt32 processContacts;

				 processContacts = 1;
				 if (pair->m_material && pair->m_material->m_compoundAABBOverlap) {
				 processContacts = pair->m_material->m_compoundAABBOverlap (*pair->m_material, *compoundBody, *otherBody, proxy.m_threadIndex);
				 }
				 if (processContacts) {
				 proxy.m_referenceCollision = me->m_shape;

				 proxy.m_referenceMatrix = me->m_shape->m_offset * myMatrix;

				 //                 proxy.m_floatingCollision = otherBody->m_collision;
				 //                 proxy.m_floatingMatrix = otherBody->m_collisionWorldMatrix;

				 proxy.m_maxContacts = DG_MAX_CONTATCS - contactCount;
				 proxy.m_contacts = &contacts[contactCount];

				 if (useSimd) {
				 contactCount += m_world->CalculateConvexToConvexContactsSimd (proxy);
				 } else {
				 contactCount += m_world->CalculateConvexToConvexContacts (proxy);
				 }

				 if (contactCount > (DG_MAX_CONTATCS - 2 * (DG_CONSTRAINT_MAX_ROWS / 3))) {
				 contactCount = m_world->ReduceContacts (contactCount, contacts, DG_CONSTRAINT_MAX_ROWS / 3, DG_REDUCE_CONTACT_TOLERANCE);
				 }
				 }
				 */
			} else {
				const dgNodeBase *const left = me->m_left;
				dgFloat32 leftDist = dgFloat32(0.0f);
				if (left->m_type != m_leaf) {
					leftDist = left->BoxClosestDistance(points, 8);
				}

				const dgNodeBase *const right = me->m_right;;
				dgFloat32 rightDist = dgFloat32(0.0f);
				if (right->m_type != m_leaf) {
					rightDist = right->BoxClosestDistance(points, 8);
				}

				NEWTON_ASSERT(me->m_type == m_node);
				if (leftDist > rightDist) {
					distPool[stack] = leftDist;
					stackPool[stack] = me->m_left;
					stack++;
					NEWTON_ASSERT(stack <  dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

					distPool[stack] = rightDist;
					stackPool[stack] = me->m_right;
					stack++;
					NEWTON_ASSERT(stack <  dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
				} else {
					distPool[stack] = rightDist;
					stackPool[stack] = me->m_right;
					stack++;
					NEWTON_ASSERT(stack <  dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));

					distPool[stack] = leftDist;
					stackPool[stack] = me->m_left;
					stack++;
					NEWTON_ASSERT(stack <  dgInt32(sizeof(stackPool) / sizeof(dgNodeBase *)));
				}
			}
		}
	}
	return 0;
#endif

	// this is temporary until I implement the code above using the spacial organization
	dgInt32 retFlag = 1;
	dgContactPoint contact0;
	dgContactPoint contact1;
	contact0.m_point = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	contact1.m_point = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	contact0.m_normal = dgVector(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	contact1.m_normal = dgVector(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f));

	if (bodyB->m_collision->IsType(dgCollision::dgConvexCollision_RTTI)) {
		dgCollisionParamProxy proxy(0);
		dgContactPoint contacts[16];

		proxy.m_referenceBody = compoundBody;
		proxy.m_floatingBody = bodyB;
		proxy.m_floatingCollision = bodyB->m_collision;
		proxy.m_floatingMatrix = bodyB->m_collisionWorldMatrix;

		proxy.m_timestep = dgFloat32(0.0f);
		proxy.m_penetrationPadding = dgFloat32(0.0f);
		proxy.m_unconditionalCast = 1;
		proxy.m_continueCollision = 0;
		proxy.m_maxContacts = 16;
		proxy.m_contacts = &contacts[0];

		dgMatrix myMatrix(m_offset * compoundBody->m_matrix);
		dgFloat32 minDist2 = dgFloat32(1.0e10f);
		for (dgInt32 i = 0; (i < m_count) && retFlag; i++) {
			retFlag = 0;
			proxy.m_referenceCollision = m_array[i];
			proxy.m_referenceMatrix = m_array[i]->m_offset * myMatrix;
			dgInt32 flag = m_world->ClosestPoint(proxy);
			if (flag) {
				retFlag = 1;
				dgVector err(contacts[0].m_point - contacts[1].m_point);
				dgFloat32 dist2 = err % err;
				if (dist2 < minDist2) {
					minDist2 = dist2;
					contact0 = contacts[0];
					contact1 = contacts[1];
				}
			}
		}
	} else {

		dgCollisionParamProxy proxy(0);
		dgContactPoint contacts[16];

		NEWTON_ASSERT(
		    bodyB->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI));
		dgCollisionCompound *const compoundCollision1 =
		    (dgCollisionCompound *)bodyB->m_collision;
		dgInt32 count1 = compoundCollision1->m_count;
		dgCollisionConvex **collisionArray1 = compoundCollision1->m_array;

		proxy.m_referenceBody = compoundBody;
		proxy.m_floatingBody = bodyB;
		proxy.m_timestep = dgFloat32(0.0f);
		proxy.m_penetrationPadding = dgFloat32(0.0f);
		proxy.m_unconditionalCast = 1;
		proxy.m_continueCollision = 0;
		proxy.m_maxContacts = 16;
		proxy.m_contacts = &contacts[0];

		dgMatrix myMatrix(m_offset * compoundBody->m_matrix);
		dgMatrix otherMatrix(compoundCollision1->m_offset * bodyB->m_matrix);
		dgFloat32 minDist2 = dgFloat32(1.0e10f);
		for (dgInt32 i = 0; (i < m_count) && retFlag; i++) {
			proxy.m_referenceCollision = m_array[i];
			proxy.m_referenceMatrix = m_array[i]->m_offset * myMatrix;

			for (dgInt32 j = 0; (j < count1) && retFlag; j++) {
				retFlag = 0;
				proxy.m_floatingCollision = collisionArray1[j];
				proxy.m_floatingMatrix = collisionArray1[j]->m_offset * otherMatrix;

				dgInt32 flag = m_world->ClosestPoint(proxy);
				if (flag) {
					retFlag = 1;
					dgVector err(contacts[0].m_point - contacts[1].m_point);
					dgFloat32 dist2 = err % err;
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
}
