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

#include "dgBroadPhaseCollision.h"
#include "dgBody.h"
#include "dgCollisionConvex.h"
#include "dgContact.h"
#include "dgWorld.h"
#include "dgWorldDynamicUpdate.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


dgSortArray::dgSortArray() : dgList<dgSortArrayEntry>(NULL) {
	m_index = 0;
	m_isSorted = 0;
}

dgSortArray::~dgSortArray() {
	_ASSERTE(GetCount() == 0);
}

void dgSortArray::Add(dgBody *const body) {
	m_isSorted = 0;
	dgFloat32 val = body->m_minAABB[m_index];
	dgListNode *const node = Append();
	node->GetInfo().m_key = val;
	node->GetInfo().m_body = body;
	body->m_collisionCell.m_axisArrayNode[dgInt32(m_index)] = node;
}

void dgSortArray::Remove(dgBody *const body) {
	dgListNode *const node =
		(dgListNode *)body->m_collisionCell.m_axisArrayNode[dgInt32(m_index)];
	_ASSERTE(node);

	dgList<dgSortArrayEntry>::Remove(node);
	body->m_collisionCell.m_axisArrayNode[dgInt32(m_index)] = NULL;
}

dgFloat32 dgSortArray::Sort() {
	//	dgFloat32 sum;
	//	dgFloat32 sum2;

	m_isSorted = 1;

	dgFloat32 sum = GetFirst()->GetInfo().m_body->m_minAABB[m_index];
	dgFloat32 sum2 = sum * sum;
	GetFirst()->GetInfo().m_key = sum;
	for (dgListNode *node = GetFirst()->GetNext(); node;) {
		//		dgBody* body;
		//		dgFloat32 key;
		dgListNode *prev;
		//		dgListNode* entry;

		dgListNode *const entry = node;
		node = node->GetNext();
		dgBody *const body = entry->GetInfo().m_body;
		dgFloat32 key = body->m_minAABB[m_index];
		entry->GetInfo().m_key = key;

		sum += key;
		sum2 += key * key;
		for (prev = entry->GetPrev(); prev && (key < prev->GetInfo().m_key); prev =
																				 prev->GetPrev()) {
		}

		if (!prev) {
			RotateToBegin(entry);
		} else {
			InsertAfter(prev, entry);
		}
	}
	//	_ASSERTE ((GetCount() * sum2 - sum * sum) > dgFloat32 (-10.0f));
	return GetCount() * sum2 - sum * sum;
}

dgFloat32 dgSortArray::RayCast(dgFloat32 minT, const dgLineBox &line,
							   OnRayCastAction filter, OnRayPrecastAction prefilter,
							   void *const userData) const {
	if (m_isSorted) {
		//		dgFloat32 minVal = line.m_boxL0[m_index];
		dgFloat32 maxVal = line.m_boxL1[m_index];
		_ASSERTE(line.m_boxL0[m_index] <= maxVal);

		for (dgListNode *node = GetFirst();
			 node && (node->GetInfo().m_key < maxVal); node = node->GetNext()) {
			minT = node->GetInfo().m_body->RayCast(line, filter, prefilter, userData,
												   minT);
		}
	} else {
		for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			minT = node->GetInfo().m_body->RayCast(line, filter, prefilter, userData,
												   minT);
		}
	}

	return minT;
}

void dgSortArray::InvalidateCache() {
	for (dgListNode *node = GetFirst()->GetNext(); node;) {
		//		dgInt32 key;
		dgListNode *prev;
		//		dgListNode* entry;

		dgListNode *const entry = node;
		node = node->GetNext();
		dgInt32 key = entry->GetInfo().m_body->m_uniqueID;
		for (prev = entry->GetPrev();
			 prev && (prev->GetInfo().m_body->m_uniqueID > key); prev =
																	 prev->GetPrev()) {
		}

		if (!prev) {
			RotateToBegin(entry);
		} else {
			InsertAfter(prev, entry);
		}
	}
}

bool dgSortArray::SanityCheck() {
	dgFloat32 val;
	dgListNode *node;

	val = GetFirst()->GetInfo().m_key;
	for (node = GetFirst()->GetNext(); node; node = node->GetNext()) {
		if (val > node->GetInfo().m_key) {
			return false;
		}
		val = node->GetInfo().m_key;
	}
	return true;
}

dgBroadPhaseCell::dgBroadPhaseCell() {
}

dgBroadPhaseCell::~dgBroadPhaseCell() {
	_ASSERTE(m_count == 0);
}

void dgBroadPhaseCell::Init(dgInt32 layer, dgMemoryAllocator *allocator) {
	m_count = 0;
	m_active = 0;
	m_layerIndex = dgInt8(layer);
	m_sort[0].m_index = 0;
	m_sort[1].m_index = 1;
	m_sort[2].m_index = 2;
	m_sort[0].SetAllocator(allocator);
	m_sort[1].SetAllocator(allocator);
	m_sort[2].SetAllocator(allocator);

	m_lastSortArray = &m_sort[0];
}

void dgBroadPhaseCell::Add(dgBody *const body) {
	m_count++;
	m_active = 1;

	_ASSERTE(!body->m_collisionCell.m_cell);

	m_sort[0].Add(body);
	m_sort[1].Add(body);
	m_sort[2].Add(body);
	body->m_collisionCell.m_cell = this;
}

void dgBroadPhaseCell::Remove(dgBody *const body) {
	m_count--;
	m_active = 1;

	_ASSERTE(m_count >= 0);
	_ASSERTE(body->m_collisionCell.m_cell);

	m_sort[0].Remove(body);
	m_sort[1].Remove(body);
	m_sort[2].Remove(body);
	body->m_collisionCell.m_cell = NULL;
}

void dgBroadPhaseCell::Sort() {
	//	dgInt32 axis;
	//	dgFloat32 maxVariance;
	dgFloat32 variance[3];

	variance[0] = m_sort[0].Sort();
	variance[1] = m_sort[1].Sort();
	variance[2] = m_sort[2].Sort();

	dgInt32 axis = 0;
	dgFloat32 maxVariance = variance[0];
	for (dgInt32 i = 1; i < 3; i++) {
		if (variance[i] > maxVariance) {
			axis = i;
			maxVariance = variance[i];
		}
	}
	m_lastSortArray = &m_sort[axis];
}

void dgBroadPhaseCell::UpdateAutoPair(dgWorld *const world,
									  dgInt32 threadIndex) {
	//	dgInt32 indexX;
	//	dgFloat32 maxVal;
	//	dgBody *body0;
	//	dgBody *body1;
	//	dgSortArray* lastSort;
	//	dgSortArray::dgListNode* outerNode;
	//	dgSortArray::dgListNode* innerNode;

	dgSortArray *const lastSort = m_lastSortArray;
	dgInt32 indexX = lastSort->m_index;
	dgCollidingPairCollector &contactPair = *world;

	for (dgSortArray::dgListNode *outerNode = lastSort->GetFirst(); outerNode;
		 outerNode = outerNode->GetNext()) {
		dgBody *const body0 = outerNode->GetInfo().m_body;
		if (!body0->m_collision->IsType(dgCollision::dgCollisionNull_RTTI)) {
			dgFloat32 maxVal = body0->m_maxAABB[indexX];

			for (dgSortArray::dgListNode *innerNode = outerNode->GetNext();
				 innerNode && innerNode->GetInfo().m_key <= maxVal; innerNode =
																		innerNode->GetNext()) {
				dgBody *const body1 = innerNode->GetInfo().m_body;
				if (!body1->m_collision->IsType(dgCollision::dgCollisionNull_RTTI)) {
					_ASSERTE(body0 != body1);
					if (OverlapTest(body0, body1)) {
						contactPair.AddPair(body0, body1, threadIndex);
					}
				}
			}
		}
	}
}

dgBroadPhaseLayer::dgBroadPhaseLayer() : dgTree<dgBroadPhaseCell, dgUnsigned32>(NULL) {
	m_me = NULL;
	m_cellSize = dgFloat32(0.0f);
	m_invCellSize = dgFloat32(0.0f);
}

dgBroadPhaseLayer::~dgBroadPhaseLayer() {
	_ASSERTE(!GetCount());
}

void dgBroadPhaseLayer::Init(dgWorld *const world, dgFloat32 cellSize,
							 dgInt32 layerIndex) {
	_ASSERTE(GetCount() == 0);
	m_me = world;
	m_layerIndex = dgInt16(layerIndex);
	m_cellSize = cellSize;

	m_invCellSize = dgFloat32(1.0f) / m_cellSize;
}

dgBroadPhaseCell *dgBroadPhaseLayer::FindCreate(dgInt32 x, dgInt32 z) {
	dgUnsigned32 key = GetKey(x, z);
	dgTreeNode *node = dgTree<dgBroadPhaseCell, dgUnsigned32>::Find(key);
	if (!node) {
		dgBroadPhaseCell cell;
		cell.m_count = 0;
		node = Insert(cell, key);
		node->GetInfo().Init(m_layerIndex, m_me->GetAllocator());
	}

	return &node->GetInfo();
}

dgBroadPhaseCollision::dgBroadPhaseCollision(dgMemoryAllocator *allocator) : m_min(-dgFloat32(1000.0f), -dgFloat32(1000.0f), -dgFloat32(1000.0f),
																				   dgFloat32(0.0f)),
																			 m_max(dgFloat32(1000.0f), dgFloat32(1000.0f),
																				   dgFloat32(1000.0f), dgFloat32(0.0f)),
																			 m_appMinBox(-dgFloat32(1000.0f),
																						 -dgFloat32(1000.0f), -dgFloat32(1000.0f), dgFloat32(0.0f)),
																			 m_appMaxBox(
																				 dgFloat32(1000.0f), dgFloat32(1000.0f), dgFloat32(1000.0f),
																				 dgFloat32(0.0f)) {
	//	m_me = NULL;
	m_inactiveList.Init(0, allocator);

	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		m_layerMap[i].SetAllocator(allocator);
	}
}

dgBroadPhaseCollision::~dgBroadPhaseCollision() {
}

void dgBroadPhaseCollision::Init() {
	//	m_me = me;
	m_worlSize = dgFloat32(0.0f);

	dgVector p0(m_min);
	dgVector p1(m_max);
	SetWorldSize(p0, p1);
}

void dgBroadPhaseCollision::GetWorldSize(dgVector &p0, dgVector &p1) const {
	p0 = m_appMinBox;
	p1 = m_appMaxBox;
}

void dgBroadPhaseCollision::SetWorldSize(const dgVector &min,
										 const dgVector &max) {
	dgFloat32 cellSize;

	// remove all bodies for the map
	dgBodyMasterList &masterList(*((dgWorld *)this));
	for (dgBodyMasterList::dgListNode *node = masterList.GetFirst(); node; node =
																			   node->GetNext()) {
		dgBody *body;
		body = node->GetInfo().GetBody();
		Remove(body);
	}

	m_appMinBox = min;
	m_appMaxBox = max;

	// recalculate new map definitions
	m_min = min;
	m_max = max;

	m_min.m_x = dgFloor(m_min.m_x / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);
	m_min.m_y = dgFloor(m_min.m_y / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);
	m_min.m_z = dgFloor(m_min.m_z / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);

	m_max.m_x = dgCeil(m_max.m_x / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);
	m_max.m_y = dgCeil(m_max.m_y / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);
	m_max.m_z = dgCeil(m_max.m_z / dgFloat32(1 << DG_OCTREE_MAX_DEPTH)) * dgFloat32(1 << DG_OCTREE_MAX_DEPTH);

	dgVector size(m_max - m_min);
	cellSize = GetMax(GetMax(size[0], size[2]), size[1]);
	cellSize =
		dgPow(dgFloat32(2.0f), dgCeil(dgLog(cellSize) / dgLog(dgFloat32(2.0f))));

	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		cellSize *= dgFloat32(0.5f);
	}
	//	cellSize = dgCeil (cellSize * (1 << DG_OCTREE_MAX_DEPTH)) /  dgFloat32 (1 << DG_OCTREE_MAX_DEPTH);

	m_worlSize = cellSize;
	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		m_worlSize *= dgFloat32(2.0f);
	}

	dgWorld *const me = (dgWorld *)this;
	m_inactiveList.Init(0, me->GetAllocator());
	cellSize = m_worlSize;
	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		_ASSERTE(m_layerMap[i].GetCount() == 0);
		m_layerMap[i].Init(me, cellSize, i);
		cellSize *= dgFloat32(0.5f);
	}

	// first first cell to layer zero
	for (dgBodyMasterList::dgListNode *node = masterList.GetFirst(); node; node =
																			   node->GetNext()) {
		dgBody *body;
		body = node->GetInfo().GetBody();
		Add(body);
		body->SetMatrix(body->GetMatrix());
	}

	m_boxSize = m_max - m_min;
}

void dgBroadPhaseCollision::InvalidateCache() {
	/*
	 dgBodyMasterList& masterList (*m_me);
	 for (dgBodyMasterList::dgListNode* node = masterList.GetFirst(); node; node = node->GetNext()) {
	 dgBody* body;
	 body = node->GetInfo().GetBody();
	 Remove(body);

	 // invalidate AABB so next add can promote the body to the correct CELL.
	 body->m_minAABB = dgVector ( 1.0e10f,  1.0e10f,  1.0e10f, 0.0f);
	 body->m_maxAABB = dgVector (-1.0e10f, -1.0e10f, -1.0e10f, 0.0f);
	 }

	 for (dgBodyMasterList::dgListNode* node = masterList.GetFirst(); node; node = node->GetNext()) {
	 dgBody* body;
	 body = node->GetInfo().GetBody();
	 Add(body);
	 body->SetMatrix(body->GetMatrix());
	 }
	 */

	for (int layer = 0; layer < DG_OCTREE_MAX_DEPTH; layer++) {
		dgBroadPhaseLayer::Iterator iter(m_layerMap[layer]);
		for (iter.Begin(); iter; iter++) {
			dgBroadPhaseCell *cell;
			cell = &iter.GetNode()->GetInfo();

			cell->m_active = 1;
			cell->m_lastSortArray = &cell->m_sort[0];
			//			cell->m_sort[0].InvalidateCache();
			//			cell->m_sort[1].InvalidateCache();;
			//			cell->m_sort[2].InvalidateCache();
		}
	}
}

void dgBroadPhaseCollision::Add(dgBody *const body) {
	_ASSERTE(!body->m_collisionCell.m_cell);
	// new bodies are added to the root node, and the function set matrix relocate them
	m_layerMap[0].FindCreate(0, 0)->Add(body);
}

void dgBroadPhaseCollision::Remove(dgBody *const body) {
	//	dgBroadPhaseCell* obtreeCell;
	//	dgBroadPhaseLayer::dgTreeNode* node;

	_ASSERTE(body->m_collisionCell.m_cell);
	dgBroadPhaseCell *const obtreeCell = body->m_collisionCell.m_cell;
	obtreeCell->Remove(body);

	if (!obtreeCell->m_count) {
		if (obtreeCell != &m_inactiveList) {
			dgBroadPhaseLayer::dgTreeNode *const node = m_layerMap[dgInt32(
																	   obtreeCell->m_layerIndex)]
															.GetNodeFromInfo(*obtreeCell);
			_ASSERTE(node);
			m_layerMap[dgInt32(obtreeCell->m_layerIndex)].Remove(node);
		}
	}
}

void dgBroadPhaseCollision::UpdatePairs(dgBody *const body0,
										dgSortArray::dgListNode *const srcnode, dgInt32 axisX,
										dgInt32 threadIndex) const {
	//	dgFloat32 val;
	//	dgBody* body1;

	if (!body0->m_collision->IsType(dgCollision::dgCollisionNull_RTTI)) {
		dgFloat32 val = body0->m_maxAABB[axisX];
		dgCollidingPairCollector &contactPair = *((dgWorld *)this);
		for (dgSortArray::dgListNode *node = srcnode;
			 node && (node->GetInfo().m_key < val); node = node->GetNext()) {
			dgBody *const body1 = node->GetInfo().m_body;
			if (!body1->m_collision->IsType(dgCollision::dgCollisionNull_RTTI)) {
				_ASSERTE(body0 != body1);
				if (OverlapTest(body0, body1)) {
					contactPair.AddPair(body0, body1, threadIndex);
				}
			}
		}
	}
}

void dgBroadPhaseCollision::UpdatePairs(dgBroadPhaseCell &cellA,
										dgBroadPhaseCell &cellB, dgInt32 threadIndex) const {
	dgInt32 axisX = cellA.m_lastSortArray->m_index;
	dgSortArray *const listA = &cellA.m_sort[axisX];
	dgSortArray *const listB = &cellB.m_sort[axisX];

	dgSortArray::dgListNode *nodeA = listA->GetFirst();
	dgSortArray::dgListNode *nodeB = listB->GetFirst();
	while (nodeA && nodeB) {
		if (nodeA->GetInfo().m_key < nodeB->GetInfo().m_key) {
			UpdatePairs(nodeA->GetInfo().m_body, nodeB, axisX, threadIndex);
			nodeA = nodeA->GetNext();
		} else {
			UpdatePairs(nodeB->GetInfo().m_body, nodeA, axisX, threadIndex);
			nodeB = nodeB->GetNext();
		}
	}
}

void dgBroadPhaseCollision::ForEachBodyInAABB(const dgVector &p0,
											  const dgVector &p1, OnBodiesInAABB callback, void *const userdata) const {
	if (dgOverlapTest(p0, p1, m_appMinBox, m_appMaxBox)) {
		dgBody *const sentinel = ((dgWorld *)this)->GetSentinelBody();
		dgFloat32 x0 = GetMax(p0.m_x - m_min.m_x, dgFloat32(0.0f));
		//		dgFloat32 y0 = GetMax (p0.m_y - m_min.m_y, dgFloat32 (0.0f));
		dgFloat32 z0 = GetMax(p0.m_z - m_min.m_z, dgFloat32(0.0f));
		dgFloat32 x1 = GetMin(p1.m_x - m_min.m_x, m_worlSize * dgFloat32(0.999f));
		//		dgFloat32 y1 = GetMin (p1.m_y - m_min.m_y, m_worlSize * dgFloat32 (0.999f));
		dgFloat32 z1 = GetMin(p1.m_z - m_min.m_z, m_worlSize * dgFloat32(0.999f));
		for (dgInt32 layer = 0; layer < DG_OCTREE_MAX_DEPTH; layer++) {

			const dgBroadPhaseLayer &layerMap = m_layerMap[layer];
			if (layerMap.GetCount()) {
				dgFloat32 cellScale = layerMap.m_invCellSize;
				dgInt32 ix0 = dgFastInt(x0 * cellScale);
				dgInt32 ix1 = dgFastInt(x1 * cellScale);
				for (dgInt32 xIndex = ix0; xIndex <= ix1; xIndex++) {
					dgInt32 iz0 = dgFastInt(z0 * cellScale);
					dgInt32 iz1 = dgFastInt(z1 * cellScale);
					for (dgInt32 zIndex = iz0; zIndex <= iz1; zIndex++) {
						dgBroadPhaseCell *const cell = layerMap.Find(xIndex, zIndex);
						if (cell) {
							for (dgSortArray::dgListNode *node = cell->m_sort[0].GetFirst();
								 node; node = node->GetNext()) {
								dgBody *const body = node->GetInfo().m_body;
								if (dgOverlapTest(body->m_minAABB, body->m_maxAABB, p0, p1)) {
									if (body != sentinel) {
										callback(body, userdata);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

dgInt32 dgBroadPhaseCollision::ConvexCast(dgCollision *const shape,
										  const dgMatrix &matrixOrigin, const dgVector &target,
										  dgFloat32 &timeToImpact, OnRayPrecastAction prefilter, void *const userData,
										  dgConvexCastReturnInfo *const info, dgInt32 maxContacts,
										  dgInt32 threadIndex) const {
	dgVector p0;
	dgVector p1;
	dgVector q0;
	dgVector q1;
	dgMatrix matrixTarget(matrixOrigin);

	matrixTarget.m_posit = target;
	dgCollisionConvex *const collision = (dgCollisionConvex *)shape;

	collision->CalcAABB(matrixOrigin, p0, p1);
	collision->CalcAABB(matrixTarget, q0, q1);
	p0.m_x = GetMin(p0.m_x, q0.m_x);
	p0.m_y = GetMin(p0.m_y, q0.m_y);
	p0.m_z = GetMin(p0.m_z, q0.m_z);
	p1.m_x = GetMax(p1.m_x, q1.m_x);
	p1.m_y = GetMax(p1.m_y, q1.m_y);
	p1.m_z = GetMax(p1.m_z, q1.m_z);

	dgInt32 totalCount = 0;
	timeToImpact = dgFloat32(1.2f);
	if (dgOverlapTest(p0, p1, m_appMinBox, m_appMaxBox)) {

#define CONVEX_CAST_POOLSIZE 32
		dgTriplex points[CONVEX_CAST_POOLSIZE];
		dgTriplex normals[CONVEX_CAST_POOLSIZE];
		dgFloat32 penetration[CONVEX_CAST_POOLSIZE];

		if (maxContacts > CONVEX_CAST_POOLSIZE) {
			maxContacts = CONVEX_CAST_POOLSIZE;
		}

		dgWorld *const me = (dgWorld *)this;
		dgInt32 cpu = me->m_cpu;
		//		dgBody* const sentinel = me->GetSentinelBody();

		dgFloat32 timestep = 1.2f;
		dgMatrix alignedMatrix(matrixOrigin);
		dgVector velocA(target - matrixOrigin.m_posit);
		dgVector velocB(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						dgFloat32(0.0f));

		dgFloat32 x0 = GetMax(p0.m_x - m_min.m_x, dgFloat32(0.0f));
		//		dgFloat32 y0 = GetMax (p0.m_y - m_min.m_y, dgFloat32 (0.0f));
		dgFloat32 z0 = GetMax(p0.m_z - m_min.m_z, dgFloat32(0.0f));
		dgFloat32 x1 = GetMin(p1.m_x - m_min.m_x, m_worlSize * dgFloat32(0.999f));
		//		dgFloat32 y1 = GetMin (p1.m_y - m_min.m_y, m_worlSize * dgFloat32 (0.999f));
		dgFloat32 z1 = GetMin(p1.m_z - m_min.m_z, m_worlSize * dgFloat32(0.999f));
		for (dgInt32 layer = 0; layer < DG_OCTREE_MAX_DEPTH; layer++) {
			if (m_layerMap[layer].GetCount()) {
				dgFloat32 cellScale = m_layerMap[layer].m_invCellSize;
				dgInt32 ix0 = dgFastInt(x0 * cellScale);
				dgInt32 ix1 = dgFastInt(x1 * cellScale);
				for (dgInt32 xIndex = ix0; xIndex <= ix1; xIndex++) {
					dgInt32 iz0 = dgFastInt(z0 * cellScale);
					dgInt32 iz1 = dgFastInt(z1 * cellScale);
					for (dgInt32 zIndex = iz0; zIndex <= iz1; zIndex++) {
						dgBroadPhaseCell *const cell = m_layerMap[layer].Find(xIndex,
																			  zIndex);
						if (cell) {
							for (dgSortArray::dgListNode *node = cell->m_sort[0].GetFirst();
								 node; node = node->GetNext()) {
								const dgBody *const body = node->GetInfo().m_body;
								if (dgOverlapTest(body->m_minAABB, body->m_maxAABB, p0, p1)) {
									//									if (body != sentinel) {
									if (!body->m_collision->IsType(
											dgCollision::dgCollisionNull_RTTI)) {
										if (!PREFILTER_RAYCAST(prefilter, body, collision, userData)) {
											dgInt32 count;
											dgFloat32 time;

											if (cpu == dgSimdPresent) {
												count = me->CollideContinueSimd(collision,
																				alignedMatrix, velocA, velocB, body->m_collision,
																				body->m_matrix, velocB, velocB, time, points,
																				normals, penetration, CONVEX_CAST_POOLSIZE,
																				threadIndex);

											} else {
												count = me->CollideContinue(collision, alignedMatrix,
																			velocA, velocB, body->m_collision, body->m_matrix,
																			velocB, velocB, time, points, normals, penetration,
																			CONVEX_CAST_POOLSIZE, threadIndex);
											}

											timeToImpact = GetMin(time, timeToImpact);

											if (count) {
												if (time <= timestep) {
													if ((timestep - time) > dgFloat32(1.0e-3f)) {
														totalCount = 0;
														timestep = time;
													}
													if (count >= (maxContacts - totalCount)) {
														count = maxContacts - totalCount;
													}

													for (dgInt32 i = 0; i < count; i++) {
														info[totalCount].m_hitBody = body;
														info[totalCount].m_point[0] = points[i].m_x;
														info[totalCount].m_point[1] = points[i].m_y;
														info[totalCount].m_point[2] = points[i].m_z;
														info[totalCount].m_normal[0] = normals[i].m_x;
														info[totalCount].m_normal[1] = normals[i].m_y;
														info[totalCount].m_normal[2] = normals[i].m_z;
														info[totalCount].m_penetration = penetration[i];
														info[totalCount].m_contaID = 0;
														totalCount++;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (totalCount) {
#define DG_RAY_TEST_LENGTH dgFloat32(0.015625f)
			dgVector dir(
				velocA.Scale(DG_RAY_TEST_LENGTH * dgRsqrt(velocA % velocA)));

			for (dgInt32 i = 0; i < totalCount; i++) {
				//				dgFloat32 t;
				dgContactPoint contact;
				dgVector p0(info[i].m_point[0], info[i].m_point[1], info[i].m_point[2],
							dgFloat32(0.0f));
				dgVector p1(p0 + dir);
				p0 -= dir;

				const dgMatrix &matrix = info[i].m_hitBody->m_matrix;

				dgVector l0(matrix.UntransformVector(p0));
				dgVector l1(matrix.UntransformVector(p1));
				info[i].m_normalOnHitPoint[0] = info[i].m_normal[0];

				// bug fixed by thedmd
				// dgFloat32 t = info[i].m_hitBody->m_collision->RayCast (l0, l1, contact, NULL, NULL, NULL);
				dgFloat32 t = info[i].m_hitBody->m_collision->RayCast(l0, l1, contact,
																	  NULL, info[i].m_hitBody, NULL);
				if (t >= dgFloat32(0.0f) && t <= dgFloat32(dgFloat32(1.0f))) {
					contact.m_normal = matrix.RotateVector(contact.m_normal);
					info[i].m_normalOnHitPoint[0] = contact.m_normal[0];
					info[i].m_normalOnHitPoint[1] = contact.m_normal[1];
					info[i].m_normalOnHitPoint[2] = contact.m_normal[2];
					info[i].m_normalOnHitPoint[3] = dgFloat32(0.0f);
				} else {
					info[i].m_normalOnHitPoint[0] = info[i].m_normal[0];
					info[i].m_normalOnHitPoint[1] = info[i].m_normal[1];
					info[i].m_normalOnHitPoint[2] = info[i].m_normal[2];
					info[i].m_normalOnHitPoint[3] = dgFloat32(0.0f);
				}
			}
		}
	}
	return totalCount;
}

void dgBroadPhaseCalculateContactsWorkerThread::Realloc(dgInt32 jointsCount,
														dgInt32 contactCount, dgInt32 threadIndex) {
	m_world->dgGetUserLock();

	dgCollidingPairCollector::dgPair *const pairs = m_world->m_pairs;
	dgContactPoint *const contactBuffer =
		(dgContactPoint *)m_world->m_contactBuffers[threadIndex];

	dgInt32 size = m_world->m_contactBuffersSizeInBytes[threadIndex] * 2;
	dgContactPoint *const newBuffer =
		(dgContactPoint *)m_world->GetAllocator()->MallocLow(size);
	memcpy(newBuffer, contactBuffer, contactCount * sizeof(dgContactPoint));

	dgInt32 index = 0;
	for (dgInt32 j = 0; j < jointsCount; j += m_step) {
		dgCollidingPairCollector::dgPair &pair = pairs[j + threadIndex];
		if (pair.m_contactBuffer) {
			pair.m_contactBuffer = &newBuffer[index];
			index += pair.m_contactCount;
		}
	}

	_ASSERTE(index == contactCount);

	m_world->GetAllocator()->FreeLow(m_world->m_contactBuffers[threadIndex]);
	m_world->m_contactBuffersSizeInBytes[threadIndex] = size;
	m_world->m_contactBuffers[threadIndex] = newBuffer;

	m_world->dgReleasedUserLock();
}

void dgBroadPhaseCellPairsWorkerThread::ThreadExecute() {
	dgInt32 step = m_step;
	dgInt32 count = m_count;
	dgBroadPhaseCollision &broadPhase = *m_world;
	for (dgInt32 i = 0; i < count; i += step) {
		if (m_pairs[i].m_cell_B) {
			broadPhase.UpdatePairs(*m_pairs[i].m_cell_A, *m_pairs[i].m_cell_B,
								   m_threadIndex);
		} else {
			m_pairs[i].m_cell_A->UpdateAutoPair(m_world, m_threadIndex);
		}
	}
}

void dgBroadPhaseApplyExternalForce::ThreadExecute() {
	dgInt32 step = m_step;
	dgInt32 count = m_count;
	dgBody **const bodyArray = m_bodies;

	if (m_skipForceUpdate) {
		if (m_world->m_cpu == dgSimdPresent) {
			for (dgInt32 i = 0; i < count; i += step) {
				dgBody *const body = bodyArray[i];
				_ASSERTE(
					body->m_collision->IsType(dgCollision::dgConvexCollision_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionConvexModifier_RTTI));
				if (!body->IsInEquelibrium()) {
					body->UpdateCollisionMatrixSimd(m_timeStep, m_threadIndex);
				}
			}
		} else {
			for (dgInt32 i = 0; i < count; i += step) {
				dgBody *const body = bodyArray[i];
				_ASSERTE(
					body->m_collision->IsType(dgCollision::dgConvexCollision_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionConvexModifier_RTTI));
				if (!body->IsInEquelibrium()) {
					body->UpdateCollisionMatrixSimd(m_timeStep, m_threadIndex);
				}
			}
		}
	} else {
		if (m_world->m_cpu == dgSimdPresent) {
			for (dgInt32 i = 0; i < count; i += step) {
				dgBody *const body = bodyArray[i];

				body->m_solverInContinueCollision = false;
				_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));

				_ASSERTE(
					body->m_collision->IsType(dgCollision::dgConvexCollision_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionConvexModifier_RTTI));

				body->ApplyExtenalForces(m_timeStep, m_threadIndex);
				if (!body->IsInEquelibrium()) {
					body->m_sleeping = false;
					body->m_equilibrium = false;
					body->UpdateCollisionMatrixSimd(m_timeStep, m_threadIndex);
				}
				body->m_prevExternalForce = body->m_accel;
				body->m_prevExternalTorque = body->m_alpha;
			}
		} else {
			for (dgInt32 i = 0; i < count; i += step) {
				dgBody *const body = bodyArray[i];

				body->m_solverInContinueCollision = false;
				_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));

				_ASSERTE(
					body->m_collision->IsType(dgCollision::dgConvexCollision_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionCompound_RTTI) || body->m_collision->IsType(dgCollision::dgCollisionConvexModifier_RTTI));

				body->ApplyExtenalForces(m_timeStep, m_threadIndex);
				if (!body->IsInEquelibrium()) {
					body->m_sleeping = false;
					body->m_equilibrium = false;
					body->UpdateCollisionMatrix(m_timeStep, m_threadIndex);
				}
				body->m_prevExternalForce = body->m_accel;
				body->m_prevExternalTorque = body->m_alpha;
			}
		}
	}
}

void dgBroadPhaseCalculateContactsWorkerThread::ThreadExecute() {
	dgInt32 step = m_step;
	dgInt32 count = m_count;
	dgCollidingPairCollector::dgPair *const pairs = m_world->m_pairs;

	dgInt32 contactIndex = 0;
	dgInt32 contactSize = dgInt32(
		m_world->m_contactBuffersSizeInBytes[m_threadIndex] / sizeof(dgContactPoint));
	dgContactPoint *contactBuffer =
		(dgContactPoint *)m_world->m_contactBuffers[m_threadIndex];

	if (m_useSimd) {
		for (dgInt32 i = 0; i < count; i += step) {
			dgCollidingPairCollector::dgPair &pair = pairs[i + m_threadIndex];

			if ((contactIndex + DG_MAX_CONTATCS) > contactSize) {
				Realloc(i, contactIndex, m_threadIndex);
				contactSize = dgInt32(
					m_world->m_contactBuffersSizeInBytes[m_threadIndex] / sizeof(dgContactPoint));
				contactBuffer =
					(dgContactPoint *)m_world->m_contactBuffers[m_threadIndex];
			}

			pair.m_contactBuffer = &contactBuffer[contactIndex];
			m_world->CalculateContactsSimd(&pair, m_timestep, m_threadIndex);

			contactIndex += pair.m_contactCount;
			_ASSERTE(contactIndex < contactSize);
		}
	} else {
		for (dgInt32 i = 0; i < count; i += step) {
			dgCollidingPairCollector::dgPair &pair = pairs[i + m_threadIndex];

			if ((contactIndex + DG_MAX_CONTATCS) > contactSize) {
				Realloc(i, contactIndex, m_threadIndex);
				contactSize = dgInt32(
					m_world->m_contactBuffersSizeInBytes[m_threadIndex] / sizeof(dgContactPoint));
				contactBuffer =
					(dgContactPoint *)m_world->m_contactBuffers[m_threadIndex];
			}

			pair.m_contactBuffer = &contactBuffer[contactIndex];
			m_world->CalculateContacts(&pair, m_timestep, m_threadIndex);

			contactIndex += pair.m_contactCount;
			_ASSERTE(contactIndex < contactSize);
		}
	}
}

void dgBroadPhaseMaterialCallbackWorkerThread::ThreadExecute() {
	dgCollidingPairCollector::dgPair *const pairs = m_pairs;

	dgInt32 step = m_step;
	dgInt32 count = m_count;

	// now make all contact joints and perform callbacks and joint allocations allocation
	for (dgInt32 i = 0; i < count; i += step) {
		dgCollidingPairCollector::dgPair &pair = pairs[i];

		if (pair.m_contactCount || pair.m_isTrigger) {
			_ASSERTE(pair.m_contactCount <= (DG_CONSTRAINT_MAX_ROWS / 3));
			if (pair.m_isTrigger) {
				m_world->ProcessTriggers(&pair, m_timestep, m_threadIndex);
			} else {
				m_world->ProcessContacts(&pair, m_timestep, m_threadIndex);
			}

		} else if (pair.m_contact) {
			if (!pair.m_contactBuffer) {
				m_world->ProcessCachedContacts(pair.m_contact, pair.m_material,
											   m_timestep, m_threadIndex);
			} else {
				pair.m_contact->m_maxDOF = 0;
			}
		}
	}
}

void dgBroadPhaseCollision::RayCast(const dgVector &l0, const dgVector &l1,
									OnRayCastAction filter, OnRayPrecastAction prefilter,
									void *const userData) const {
	dgVector ll0(l0);
	dgVector ll1(l1);
	dgVector segment(l1 - l0);
	dgFloat32 dist2 = segment % segment;

	if (filter && (dist2 > dgFloat32(1.0e-8f)) && dgRayBoxClip(ll0, ll1, m_appMinBox, m_appMaxBox)) {
		dgLineBox line;

		line.m_l0 = l0;
		line.m_l1 = l1;
		if (line.m_l0.m_x <= line.m_l1.m_x) {
			line.m_boxL0.m_x = line.m_l0.m_x;
			line.m_boxL1.m_x = line.m_l1.m_x;
		} else {
			line.m_boxL0.m_x = line.m_l1.m_x;
			line.m_boxL1.m_x = line.m_l0.m_x;
		}

		if (line.m_l0.m_y <= line.m_l1.m_y) {
			line.m_boxL0.m_y = line.m_l0.m_y;
			line.m_boxL1.m_y = line.m_l1.m_y;
		} else {
			line.m_boxL0.m_y = line.m_l1.m_y;
			line.m_boxL1.m_y = line.m_l0.m_y;
		}

		if (line.m_l0.m_z <= line.m_l1.m_z) {
			line.m_boxL0.m_z = line.m_l0.m_z;
			line.m_boxL1.m_z = line.m_l1.m_z;
		} else {
			line.m_boxL0.m_z = line.m_l1.m_z;
			line.m_boxL1.m_z = line.m_l0.m_z;
		}

		dgFloat32 minT = dgFloat32(1.1f);

		dgVector rayP0(l0 - m_min);
		dgVector rayP1(l1 - m_min);

		dgFloat32 xx0 = GetMin(rayP0.m_x, rayP1.m_x) - dgFloat32(1.0e-3f);
		dgFloat32 zz0 = GetMin(rayP0.m_z, rayP1.m_z) - dgFloat32(1.0e-3f);
		dgFloat32 xx1 = GetMax(rayP0.m_x, rayP1.m_x) + dgFloat32(1.0e-3f);
		dgFloat32 zz1 = GetMax(rayP0.m_z, rayP1.m_z) + dgFloat32(1.0e-3f);
		dgFloat32 yy0 = GetMin(rayP0.m_y, rayP1.m_y) - dgFloat32(1.0e-3f);
		dgFloat32 yy1 = GetMax(rayP0.m_y, rayP1.m_y) + dgFloat32(1.0e-3f);

		dgWorld *const me = (dgWorld *)this;
		for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
			// the user data is the pointer to the collision geometry
			const dgBroadPhaseLayer &layer = m_layerMap[i];

			if (layer.GetCount()) {
				// calculate the ray bounding box
				dgFloat32 scale = layer.m_cellSize;
				dgFloat32 invScale = layer.m_invCellSize;
				dgFloat32 x0 = scale * dgFloor(xx0 * invScale);
				dgFloat32 y0 = scale * dgFloor(yy0 * invScale);
				dgFloat32 z0 = scale * dgFloor(zz0 * invScale);
				dgFloat32 x1 = scale * dgFloor(xx1 * invScale) + scale;
				dgFloat32 y1 = scale * dgFloor(yy1 * invScale) + scale;
				dgFloat32 z1 = scale * dgFloor(zz1 * invScale) + scale;

				dgVector boxP0(GetMax(x0, dgFloat32(0.0f)), GetMax(y0, dgFloat32(0.0f)),
							   GetMax(z0, dgFloat32(0.0f)), dgFloat32(0.0f));
				dgVector boxP1(GetMin(x1, m_boxSize.m_x), GetMin(y1, m_boxSize.m_y),
							   GetMin(z1, m_boxSize.m_z), dgFloat32(0.0f));

				dgVector dq(rayP1.m_x - rayP0.m_x, rayP1.m_y - rayP0.m_y,
							rayP1.m_z - rayP0.m_z, dgFloat32(0.0f));

				// make sure the line segment crosses the original segment box
				dgVector p0(rayP0);
				dgVector p1(rayP1);

				// clip the line against the bounding box
				if (dgRayBoxClip(p0, p1, boxP0, boxP1)) {
					dgVector dp(p1 - p0);
					dgInt32 ix0 = dgFastInt(p0.m_x * invScale);
					dgInt32 iz0 = dgFastInt(p0.m_z * invScale);

					// implement a 3ddda line algorithm

					dgInt32 xInc = 0;
					dgFloat32 stepX = dgFloat32(0.0f);
					dgFloat32 tx = dgFloat32(1.0e10f);

					if (dp.m_x > dgFloat32(0.0f)) {
						xInc = 1;
						dgFloat32 val = dgFloat32(1.0f) / dp.m_x;
						stepX = scale * val;
						tx = (scale * (ix0 + dgFloat32(1.0f)) - p0.m_x) * val;
					} else if (dp.m_x < dgFloat32(0.0f)) {
						xInc = -1;
						dgFloat32 val = dgFloat32(-1.0f) / dp.m_x;
						stepX = scale * val;
						tx = -(scale * ix0 - p0.m_x) * val;
					}

					dgInt32 zInc = 0;
					dgFloat32 stepZ = dgFloat32(0.0f);
					dgFloat32 tz = dgFloat32(1.0e10f);
					if (dp.m_z > dgFloat32(0.0f)) {
						zInc = 1;
						dgFloat32 val = dgFloat32(1.0f) / dp.m_z;
						stepZ = scale * val;
						tz = (scale * (iz0 + dgFloat32(1.0f)) - p0.m_z) * val;
					} else if (dp.m_z < dgFloat32(0.0f)) {
						zInc = -1;
						dgFloat32 val = dgFloat32(-1.0f) / dp.m_z;
						stepZ = scale * val;
						tz = -(scale * iz0 - p0.m_z) * val;
					}

					dgFloat32 txAcc = tx;
					dgFloat32 tzAcc = tz;
					dgInt32 xIndex0 = ix0;
					dgInt32 zIndex0 = iz0;

					// for each cell touched by the line
					do {
						dgBroadPhaseCell *const cell = layer.Find(xIndex0, zIndex0);
						if (cell) {
							//_ASSERTE (cell->m_count);
							_ASSERTE(cell->m_lastSortArray);
							if (!me->m_inUpdate) {
								if (!cell->m_lastSortArray->m_isSorted) {
									cell->m_lastSortArray->Sort();
								}
							}
							minT = cell->m_lastSortArray->RayCast(minT, line, filter,
																  prefilter, userData);
						}
						if (txAcc < tzAcc) {
							xIndex0 += xInc;
							tx = txAcc;
							txAcc += stepX;
						} else {
							zIndex0 += zInc;
							tz = tzAcc;
							tzAcc += stepZ;
						}
					} while ((tx <= dgFloat32(1.0f)) || (tz <= dgFloat32(1.0f)));
				}
			}
		}
	}
}

void dgBroadPhaseCollision::UpdateBodyBroadphase(dgBody *const body,
												 dgInt32 threadIndex) {
	if (!body->m_isInWorld) {
		if (dgOverlapTest(body->m_minAABB, body->m_maxAABB, m_appMinBox,
						  m_appMaxBox)) {
			//			dgBroadPhaseCell *cell;
			//			cell = body->m_collisionCell.m_cell;
			//			_ASSERTE (cell);
			Remove(body);
			Add(body);
			body->m_isInWorld = true;
			body->m_sleeping = false;
			body->m_equilibrium = false;
		}
	}

	if (body->m_isInWorld) {

		dgWorld *const me = (dgWorld *)this;
		if ((body->m_minAABB.m_x > m_appMinBox.m_x) && (body->m_minAABB.m_y > m_appMinBox.m_y) && (body->m_minAABB.m_z > m_appMinBox.m_z) && (body->m_maxAABB.m_x < m_appMaxBox.m_x) && (body->m_maxAABB.m_y < m_appMaxBox.m_y) && (body->m_maxAABB.m_z < m_appMaxBox.m_z)) {
			dgFloat32 x0 = body->m_minAABB.m_x - m_min.m_x;
			// dgFloat32 y0 = body->m_minAABB.m_y - m_min.m_y;
			dgFloat32 z0 = body->m_minAABB.m_z - m_min.m_z;

			dgFloat32 x1 = body->m_maxAABB.m_x - m_min.m_x;
			// dgFloat32 y1 = body->m_maxAABB.m_y - m_min.m_y;
			dgFloat32 z1 = body->m_maxAABB.m_z - m_min.m_z;

			for (dgInt32 layer = DG_OCTREE_MAX_DEPTH - 1; layer >= 0; layer--) {
				dgFloat32 cellScale = m_layerMap[layer].m_invCellSize;
				dgInt32 ix0 = dgFastInt(x0 * cellScale);
				dgInt32 ix1 = dgFastInt(x1 * cellScale);
				if (ix1 == ix0) {
					dgInt32 iz0 = dgFastInt(z0 * cellScale);
					dgInt32 iz1 = dgFastInt(z1 * cellScale);
					if (iz1 == iz0) {
						if (!body->m_spawnnedFromCallback) {
							me->dgGetUserLock();
						}

						dgBroadPhaseCell *const newCell = m_layerMap[layer].FindCreate(ix0,
																					   iz0);
						newCell->m_active = 1;
						dgBroadPhaseCell *const cell = body->m_collisionCell.m_cell;
						if (newCell != cell) {
							cell->Remove(body);
							if (!cell->m_count) {
								dgBroadPhaseLayer::dgTreeNode *const node = m_layerMap[dgInt32(
																						   cell->m_layerIndex)]
																				.GetNodeFromInfo(*cell);
								_ASSERTE(node);
								m_layerMap[int(cell->m_layerIndex)].Remove(node);
							}
							newCell->Add(body);
						}
						newCell->m_lastSortArray->m_isSorted = 0;
						body->m_isInWorld = true;
						if (!body->m_spawnnedFromCallback) {
							me->dgReleasedUserLock();
						}
						return;
					}
				}
			}
		}

		if (dgOverlapTest(body->m_minAABB, body->m_maxAABB, m_appMinBox,
						  m_appMaxBox)) {

			if (!body->m_spawnnedFromCallback) {
				me->dgGetUserLock();
			}
			dgBroadPhaseCell *const newCell = m_layerMap[0].FindCreate(0, 0);
			dgBroadPhaseCell *const cell = body->m_collisionCell.m_cell;
			if (newCell != cell) {
				cell->Remove(body);
				if (!cell->m_count) {
					dgBroadPhaseLayer::dgTreeNode *const node = m_layerMap[dgInt32(
																			   cell->m_layerIndex)]
																	.GetNodeFromInfo(*cell);
					_ASSERTE(node);
					m_layerMap[dgInt32(cell->m_layerIndex)].Remove(node);
				}
				newCell->Add(body);
			}
			newCell->m_lastSortArray->m_isSorted = 0;
			body->m_isInWorld = true;
			if (!body->m_spawnnedFromCallback) {
				me->dgReleasedUserLock();
			}
			return;
		}

		{

			body->m_sleeping = true;
			body->m_isInWorld = false;
			body->m_equilibrium = true;
			// bool inWorldFlag = body->m_isInWorld;

			if (!body->m_spawnnedFromCallback) {
				me->dgGetUserLock();
			}
			dgBroadPhaseCell *const cell = body->m_collisionCell.m_cell;
			cell->Remove(body);
			if (!cell->m_count) {
				dgBroadPhaseLayer::dgTreeNode *const node = m_layerMap[dgInt32(
																		   cell->m_layerIndex)]
																.GetNodeFromInfo(*cell);
				_ASSERTE(node);
				m_layerMap[dgInt32(cell->m_layerIndex)].Remove(node);
			}
			m_inactiveList.Add(body);
			if (!body->m_spawnnedFromCallback) {
				me->dgReleasedUserLock();
			}

			if (me->m_leavingWorldNotify) {
				me->m_leavingWorldNotify(body, threadIndex);
			}
		}
	}
}
/*
 void dgBroadPhaseCollision::ForceAndtorque (void** const userParamArray, dgInt32 threadID)
 {
 dgInt32 count  = dgInt32 (dgInt64 (userParamArray[1]));
 if (count > 8) {
 dgInt32 middle = count / 2;
 dgWorld* const me = (dgWorld*)userParamArray[4];
 userParamArray[1] = (void*)middle;
 me->QueueJob (ForceAndtorque, userParamArray, 5);

 dgBody* const bodies = ((dgBody*)userParamArray[0]) + middle;
 userParamArray[0] = bodies;
 userParamArray[1] = (void*) (count - middle);
 me->QueueJob (ForceAndtorque, userParamArray, 5);
 } else {
 for (dgInt32 i = 0; i < count; i ++) {

 }
 }

 //	m_userParamArray[3] = (void*)skipForceUpdate;
 //	m_userParamArray[4] = (void*)&timestep;
 }
 */

dgUnsigned32 dgBroadPhaseCollision::UpdateContactsBroadPhaseBegin(
	dgFloat32 timestep, bool collisioUpdateOnly, dgUnsigned32 ticksBase) {
	union {
		dgCellPair cellArray[1024];
		dgBody *bodyArray[1024];
	};
	dgInt32 chunkSizes[DG_MAXIMUN_THREADS];
	dgCollidingPairCollector::dgThreadPairCache pairCaches[DG_MAXIMUN_THREADS];

	dgWorld *const me = (dgWorld *)this;
	me->m_broadPhaseLru = me->m_broadPhaseLru + 1;
	const dgBodyMasterList &masterList = *me;
	dgInt32 threadCounts = dgInt32(me->m_numberOfTheads);
	dgInt32 skipForceUpdate = collisioUpdateOnly ? 1 : 0;

	dgInt32 cellsBodyCount = 0;
	_ASSERTE(masterList.GetFirst()->GetInfo().GetBody() == me->GetSentinelBody());
	for (dgBodyMasterList::dgListNode *node = masterList.GetFirst()->GetNext();
		 node; node = node->GetNext()) {
		dgBody *const body = node->GetInfo().GetBody();

		if (body->m_invMass.m_w == dgFloat32(0.0f)) {

			if (body->m_collision->GetCollisionPrimityType() == m_nullCollision) {
				if (body->m_collisionCell.m_cell != &m_inactiveList) {
					Remove(body);
					m_inactiveList.Add(body);
				}
			}
			body->m_sleeping = true;
			body->m_autoSleep = true;
			body->m_equilibrium = true;
			body->m_solverInContinueCollision = false;

		} else {

			bodyArray[cellsBodyCount] = body;
			cellsBodyCount++;
			if (cellsBodyCount >= dgInt32((sizeof(cellArray) / sizeof(dgCellPair)))) {

				if (threadCounts > 1) {
					_ASSERTE(0);
					me->m_threadsManager.CalculateChunkSizes(cellsBodyCount, chunkSizes);
					for (dgInt32 threadIndex = 0; threadIndex < threadCounts;
						 threadIndex++) {
						m_applyExtForces[threadIndex].m_step = threadCounts;
						m_applyExtForces[threadIndex].m_skipForceUpdate = skipForceUpdate;
						m_applyExtForces[threadIndex].m_count = chunkSizes[threadIndex] * threadCounts;
						m_applyExtForces[threadIndex].m_bodies = &bodyArray[threadIndex];
						m_applyExtForces[threadIndex].m_threadIndex = threadIndex;
						m_applyExtForces[threadIndex].m_timeStep = timestep;
						m_applyExtForces[threadIndex].m_world = me;
						me->m_threadsManager.SubmitJob(&m_applyExtForces[threadIndex]);
					}
					me->m_threadsManager.SynchronizationBarrier();
				} else {
					_ASSERTE(0);
					m_applyExtForces[0].m_step = 1;

					m_applyExtForces[0].m_count = cellsBodyCount;
					m_applyExtForces[0].m_timeStep = timestep;
					m_applyExtForces[0].m_bodies = &bodyArray[0];
					m_applyExtForces[0].m_threadIndex = 0;
					m_applyExtForces[0].m_skipForceUpdate = skipForceUpdate;
					m_applyExtForces[0].m_world = me;
					m_applyExtForces[0].ThreadExecute();
				}
				cellsBodyCount = 0;
			}
		}
	}

	if (threadCounts > 1) {
		me->m_threadsManager.CalculateChunkSizes(cellsBodyCount, chunkSizes);
		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_applyExtForces[threadIndex].m_step = threadCounts;
			m_applyExtForces[threadIndex].m_skipForceUpdate = skipForceUpdate;
			m_applyExtForces[threadIndex].m_count = chunkSizes[threadIndex] * threadCounts;
			m_applyExtForces[threadIndex].m_bodies = &bodyArray[threadIndex];
			m_applyExtForces[threadIndex].m_threadIndex = threadIndex;
			m_applyExtForces[threadIndex].m_timeStep = timestep;
			m_applyExtForces[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(&m_applyExtForces[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();
	} else {
		m_applyExtForces[0].m_step = 1;
		m_applyExtForces[0].m_count = cellsBodyCount;
		m_applyExtForces[0].m_bodies = &bodyArray[0];
		m_applyExtForces[0].m_timeStep = timestep;
		m_applyExtForces[0].m_threadIndex = 0;
		m_applyExtForces[0].m_skipForceUpdate = skipForceUpdate;
		m_applyExtForces[0].m_world = me;
		m_applyExtForces[0].ThreadExecute();
	}

	//	void* m_userParamArray[DG_MAX_THREADS_HIVE_PARAMETERS];
	//	m_userParamArray[0] = bodyArray;
	//	m_userParamArray[1] = (void*)cellsBodyCount;
	//	m_userParamArray[2] = (void*)skipForceUpdate;
	//	m_userParamArray[3] = (void*)&timestep;
	//	m_userParamArray[4] = me;
	//	me->QueueJob (ForceAndtorque, &m_userParamArray[0], 5);

	dgUnsigned32 ticks = me->m_getPerformanceCount();
	me->m_perfomanceCounters[m_forceCallback] = ticks - ticksBase;

	dgCollidingPairCollector &contactPair = *me;
	//	contactPair.m_count = 0;
	contactPair.Init();
	contactPair.SetCaches(pairCaches);

	dgInt32 cellsPairsCount = 0;
	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		dgBroadPhaseLayer::Iterator iter(m_layerMap[i]);
		for (iter.Begin(); iter; iter++) {
			dgBroadPhaseCell &cell = iter.GetNode()->GetInfo();
			_ASSERTE(cell.m_count);
			if (cell.m_active) {
				cell.Sort();
			}
		}
	}

	for (dgInt32 j = DG_OCTREE_MAX_DEPTH - 1; j > 0; j--) {
		dgBroadPhaseLayer::Iterator iter(m_layerMap[j]);
		for (iter.Begin(); iter; iter++) {
			dgInt32 ix0;
			dgInt32 iz0;
			dgBroadPhaseCell *const cell0 = &iter.GetNode()->GetInfo();
			m_layerMap[j].KeyToIndex(dgInt32(iter.GetNode()->GetKey()), ix0, iz0);

			for (dgInt32 i = j - 1; i >= 0; i--) {
				ix0 >>= 1;
				iz0 >>= 1;
				dgBroadPhaseCell *const cell1 = m_layerMap[i].Find(ix0, iz0);
				if (cell1) {
					if (cell0->m_active | cell1->m_active) {
						cellArray[cellsPairsCount].m_cell_A = cell1;
						cellArray[cellsPairsCount].m_cell_B = cell0;
						cellsPairsCount++;
						if (cellsPairsCount >= dgInt32(sizeof(cellArray) / sizeof(cellArray[0]))) {
							if (threadCounts > 1) {
								me->m_threadsManager.CalculateChunkSizes(cellsPairsCount,
																		 chunkSizes);
								for (dgInt32 threadIndex = 0; threadIndex < threadCounts;
									 threadIndex++) {
									m_cellPairsWorkerThreads[threadIndex].m_step = threadCounts;
									m_cellPairsWorkerThreads[threadIndex].m_count =
										chunkSizes[threadIndex] * threadCounts;
									m_cellPairsWorkerThreads[threadIndex].m_pairs =
										&cellArray[threadIndex];
									m_cellPairsWorkerThreads[threadIndex].m_threadIndex =
										threadIndex;
									m_cellPairsWorkerThreads[threadIndex].m_world = me;
									me->m_threadsManager.SubmitJob(
										&m_cellPairsWorkerThreads[threadIndex]);
								}
								me->m_threadsManager.SynchronizationBarrier();
							} else {
								m_cellPairsWorkerThreads[0].m_step = 1;
								m_cellPairsWorkerThreads[0].m_count = cellsPairsCount;
								m_cellPairsWorkerThreads[0].m_pairs = &cellArray[0];
								m_cellPairsWorkerThreads[0].m_threadIndex = 0;
								m_cellPairsWorkerThreads[0].m_world = me;
								m_cellPairsWorkerThreads[0].ThreadExecute();
							}
							cellsPairsCount = 0;
						}
					}
				}
			}
		}
	}

	for (dgInt32 i = 0; i < DG_OCTREE_MAX_DEPTH; i++) {
		dgBroadPhaseLayer::Iterator iter(m_layerMap[i]);
		for (iter.Begin(); iter; iter++) {
			dgBroadPhaseCell &cell = iter.GetNode()->GetInfo();
			_ASSERTE(cell.m_count);
			if (cell.m_active) {
				cellArray[cellsPairsCount].m_cell_A = &cell;
				cellArray[cellsPairsCount].m_cell_B = NULL;
				cellsPairsCount++;
				if (cellsPairsCount >= dgInt32(sizeof(cellArray) / sizeof(cellArray[0]))) {

					if (threadCounts > 1) {
						me->m_threadsManager.CalculateChunkSizes(cellsPairsCount,
																 chunkSizes);
						for (dgInt32 threadIndex = 0; threadIndex < threadCounts;
							 threadIndex++) {
							m_cellPairsWorkerThreads[threadIndex].m_step = threadCounts;
							m_cellPairsWorkerThreads[threadIndex].m_count =
								chunkSizes[threadIndex] * threadCounts;
							m_cellPairsWorkerThreads[threadIndex].m_pairs =
								&cellArray[threadIndex];
							m_cellPairsWorkerThreads[threadIndex].m_threadIndex = threadIndex;
							m_cellPairsWorkerThreads[threadIndex].m_world = me;
							me->m_threadsManager.SubmitJob(
								&m_cellPairsWorkerThreads[threadIndex]);
						}
						me->m_threadsManager.SynchronizationBarrier();
					} else {
						m_cellPairsWorkerThreads[0].m_step = 1;
						m_cellPairsWorkerThreads[0].m_count = cellsPairsCount;
						m_cellPairsWorkerThreads[0].m_pairs = &cellArray[0];
						m_cellPairsWorkerThreads[0].m_threadIndex = 0;
						m_cellPairsWorkerThreads[0].m_world = me;
						m_cellPairsWorkerThreads[0].ThreadExecute();
					}
					cellsPairsCount = 0;
				}
			}
			cell.m_active = 0;
		}
	}

	if (threadCounts > 1) {
		me->m_threadsManager.CalculateChunkSizes(cellsPairsCount, chunkSizes);
		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_cellPairsWorkerThreads[threadIndex].m_step = threadCounts;
			m_cellPairsWorkerThreads[threadIndex].m_count = chunkSizes[threadIndex] * threadCounts;
			m_cellPairsWorkerThreads[threadIndex].m_pairs = &cellArray[threadIndex];
			m_cellPairsWorkerThreads[threadIndex].m_threadIndex = threadIndex;
			m_cellPairsWorkerThreads[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(&m_cellPairsWorkerThreads[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();
	} else {
		m_cellPairsWorkerThreads[0].m_step = 1;
		m_cellPairsWorkerThreads[0].m_count = cellsPairsCount;
		m_cellPairsWorkerThreads[0].m_pairs = &cellArray[0];
		m_cellPairsWorkerThreads[0].m_threadIndex = 0;
		m_cellPairsWorkerThreads[0].m_world = me;
		m_cellPairsWorkerThreads[0].ThreadExecute();
	}

	for (dgInt32 i = 0; i < threadCounts; i++) {
		if (pairCaches[i].m_count) {
			contactPair.FlushChache(&pairCaches[i]);
		}
	}

	ticksBase = me->m_getPerformanceCount();
	me->m_perfomanceCounters[m_broadPhaceTicks] = ticksBase - ticks;
	return ticksBase;
}

void dgBroadPhaseCollision::UpdateContactsBroadPhaseEnd(dgFloat32 timestep) {
	// delete all non used contacts
	dgInt32 count = 0;
	dgWorld *const me = (dgWorld *)this;
	dgInt32 lru = dgInt32(me->m_broadPhaseLru);

	dgCollidingPairCollector &contactPair = *me;
	dgContact **const deadContacs = (dgContact **)contactPair.m_pairs;
	dgActiveContacts &contactList = *me;

	for (dgActiveContacts::dgListNode *contactNode = contactList.GetFirst();
		 contactNode; contactNode = contactNode->GetNext()) {
		dgContact *const contact = contactNode->GetInfo();
		if ((contact->m_broadphaseLru != lru) || (contact->GetCount() == 0)) {
			// note this is in observation (to prevent bodies from not going to sleep  inside triggers
			//			if (! (contact->m_body0->m_sleeping & contact->m_body1->m_sleeping) ) {
			const dgBody *const body0 = contact->m_body0;
			const dgBody *const body1 = contact->m_body1;
			if (!((body0->m_sleeping | body0->m_equilibrium) & (body1->m_sleeping | body1->m_equilibrium))) {
				deadContacs[count] = contact;
				count++;
			}
		}
	}

	for (dgInt32 i = 0; i < count; i++) {
		dgContact *const contact = deadContacs[i];
		me->DestroyConstraint(contact);
	}
}

void dgBroadPhaseCollision::UpdateContactsSimd(dgFloat32 timestep,
											   bool collisioUpdate) {
#ifdef DG_BUILD_SIMD_CODE
	dgWorld *const me = (dgWorld *)this;
	dgUnsigned32 ticks = me->m_getPerformanceCount();
	dgUnsigned32 narrowTicks = UpdateContactsBroadPhaseBegin(timestep,
															 collisioUpdate, ticks);

	// calculate or update new contacts
	dgCollidingPairCollector &contactPair = *me;

	dgInt32 count = contactPair.m_count;
	dgCollidingPairCollector::dgPair *const pairs = contactPair.m_pairs;
	dgInt32 threadCounts = dgInt32(me->m_numberOfTheads);

	if (threadCounts > 1) {
		dgInt32 chunkSizes[DG_MAXIMUN_THREADS];
		me->m_threadsManager.CalculateChunkSizes(count, chunkSizes);
		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_calculateContactsWorkerThreads[threadIndex].m_useSimd = 1;
			m_calculateContactsWorkerThreads[threadIndex].m_step = threadCounts;
			m_calculateContactsWorkerThreads[threadIndex].m_count =
				chunkSizes[threadIndex] * threadCounts;
			m_calculateContactsWorkerThreads[threadIndex].m_threadIndex = threadIndex;
			m_calculateContactsWorkerThreads[threadIndex].m_timestep = timestep;
			m_calculateContactsWorkerThreads[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(
				&m_calculateContactsWorkerThreads[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();

		// material callback and create contact joints
		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_materialCallbackWorkerThreads[threadIndex].m_step = threadCounts;
			m_materialCallbackWorkerThreads[threadIndex].m_useSimd = 0;
			m_materialCallbackWorkerThreads[threadIndex].m_count =
				chunkSizes[threadIndex] * threadCounts;
			;
			m_materialCallbackWorkerThreads[threadIndex].m_pairs =
				&pairs[threadIndex];
			m_materialCallbackWorkerThreads[threadIndex].m_threadIndex = threadIndex;
			m_materialCallbackWorkerThreads[threadIndex].m_timestep = timestep;
			m_materialCallbackWorkerThreads[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(
				&m_materialCallbackWorkerThreads[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();

	} else {
		m_calculateContactsWorkerThreads[0].m_step = 1;
		m_calculateContactsWorkerThreads[0].m_useSimd = 1;
		m_calculateContactsWorkerThreads[0].m_count = count;
		m_calculateContactsWorkerThreads[0].m_threadIndex = 0;
		m_calculateContactsWorkerThreads[0].m_timestep = timestep;
		m_calculateContactsWorkerThreads[0].m_world = me;
		m_calculateContactsWorkerThreads[0].ThreadExecute();

		// material callback and create contact joints
		m_materialCallbackWorkerThreads[0].m_step = 1;
		m_materialCallbackWorkerThreads[0].m_useSimd = 0;
		m_materialCallbackWorkerThreads[0].m_count = count;
		m_materialCallbackWorkerThreads[0].m_pairs = &pairs[0];
		m_materialCallbackWorkerThreads[0].m_threadIndex = 0;
		m_materialCallbackWorkerThreads[0].m_timestep = timestep;
		m_materialCallbackWorkerThreads[0].m_world = me;
		m_materialCallbackWorkerThreads[0].ThreadExecute();
	}

	UpdateContactsBroadPhaseEnd(timestep);

	dgUnsigned32 endTicks = me->m_getPerformanceCount();
	me->m_perfomanceCounters[m_narrowPhaseTicks] = endTicks - narrowTicks;
	me->m_perfomanceCounters[m_collisionTicks] = endTicks - ticks;

#endif
}

void dgBroadPhaseCollision::UpdateContacts(dgFloat32 timestep,
										   bool collisioUpdate) {
	//	dgUnsigned32 endTicks;

	dgWorld *const me = (dgWorld *)this;
	dgUnsigned32 ticks = me->m_getPerformanceCount();
	dgUnsigned32 narrowTicks = UpdateContactsBroadPhaseBegin(timestep,
															 collisioUpdate, ticks);

	// calculate or update new contacts
	dgCollidingPairCollector &contactPair = *me;

	dgInt32 count = contactPair.m_count;
	dgCollidingPairCollector::dgPair *const pairs = contactPair.m_pairs;
	dgInt32 threadCounts = dgInt32(me->m_numberOfTheads);
	if (threadCounts > 1) {
		dgInt32 chunkSizes[DG_MAXIMUN_THREADS];
		me->m_threadsManager.CalculateChunkSizes(count, chunkSizes);
		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_calculateContactsWorkerThreads[threadIndex].m_useSimd = 0;
			m_calculateContactsWorkerThreads[threadIndex].m_step = threadCounts;
			m_calculateContactsWorkerThreads[threadIndex].m_count =
				chunkSizes[threadIndex] * threadCounts;
			m_calculateContactsWorkerThreads[threadIndex].m_threadIndex = threadIndex;
			m_calculateContactsWorkerThreads[threadIndex].m_timestep = timestep;
			m_calculateContactsWorkerThreads[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(
				&m_calculateContactsWorkerThreads[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();

		for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
			m_materialCallbackWorkerThreads[threadIndex].m_step = threadCounts;
			m_materialCallbackWorkerThreads[threadIndex].m_useSimd = 0;
			m_materialCallbackWorkerThreads[threadIndex].m_count =
				chunkSizes[threadIndex] * threadCounts;
			;
			m_materialCallbackWorkerThreads[threadIndex].m_pairs =
				&pairs[threadIndex];
			m_materialCallbackWorkerThreads[threadIndex].m_threadIndex = threadIndex;
			m_materialCallbackWorkerThreads[threadIndex].m_timestep = timestep;
			m_materialCallbackWorkerThreads[threadIndex].m_world = me;
			me->m_threadsManager.SubmitJob(
				&m_materialCallbackWorkerThreads[threadIndex]);
		}
		me->m_threadsManager.SynchronizationBarrier();

	} else {

		// calculate contacts
		m_calculateContactsWorkerThreads[0].m_step = 1;
		m_calculateContactsWorkerThreads[0].m_useSimd = 0;
		m_calculateContactsWorkerThreads[0].m_count = count;
		m_calculateContactsWorkerThreads[0].m_threadIndex = 0;
		m_calculateContactsWorkerThreads[0].m_timestep = timestep;
		m_calculateContactsWorkerThreads[0].m_world = me;
		m_calculateContactsWorkerThreads[0].ThreadExecute();

		// material callback and create contact joints
		m_materialCallbackWorkerThreads[0].m_step = 1;
		m_materialCallbackWorkerThreads[0].m_useSimd = 0;
		m_materialCallbackWorkerThreads[0].m_count = count;
		m_materialCallbackWorkerThreads[0].m_pairs = &pairs[0];
		m_materialCallbackWorkerThreads[0].m_threadIndex = 0;
		m_materialCallbackWorkerThreads[0].m_timestep = timestep;
		m_materialCallbackWorkerThreads[0].m_world = me;
		m_materialCallbackWorkerThreads[0].ThreadExecute();
	}

	UpdateContactsBroadPhaseEnd(timestep);

	dgUnsigned32 endTicks = me->m_getPerformanceCount();
	me->m_perfomanceCounters[m_narrowPhaseTicks] = endTicks - narrowTicks;
	me->m_perfomanceCounters[m_collisionTicks] = endTicks - ticks;
}
