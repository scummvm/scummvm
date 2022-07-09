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

#include "dgBodyMasterList.h"
#include "dgBody.h"
#include "dgConstraint.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


dgBodyMasterListRow::dgBodyMasterListRow() : dgList<dgBodyMasterListCell>(NULL) {
	m_body = NULL;
}

dgBodyMasterListRow::~dgBodyMasterListRow() {
	_ASSERTE(GetCount() == 0);
}

dgBodyMasterListRow::dgListNode *dgBodyMasterListRow::AddJoint(
	dgConstraint *const joint, dgBody *const body) {
	dgListNode *const node = Addtop();
	node->GetInfo().m_joint = joint;
	node->GetInfo().m_bodyNode = body;
	return node;
}

void dgBodyMasterListRow::RemoveAllJoints() {
	dgWorld *const world = m_body->GetWorld();

	for (dgListNode *node = GetFirst(); node;) {
		dgConstraint *const constraint = node->GetInfo().m_joint;
		node = node->GetNext();
		world->DestroyConstraint(constraint);
	}
}

void dgBodyMasterListRow::SortList() {
	for (dgListNode *node = GetFirst(); node;) {

		dgListNode *const entry = node;
		node = node->GetNext();
		dgListNode *prev = entry->GetPrev();
		for (; prev; prev = prev->GetPrev()) {
			if (prev < entry) {
				break;
			}
		}

		if (!prev) {
			RotateToBegin(entry);
		} else {
			InsertAfter(prev, entry);
		}
	}
}

dgBodyMasterList::dgBodyMasterList(dgMemoryAllocator *const allocator) : dgList<dgBodyMasterListRow>(allocator) {
	m_constraintCount = 0;
}

dgBodyMasterList::~dgBodyMasterList(void) {
}

void dgBodyMasterList::AddBody(dgBody *const body) {

	dgListNode *const node = Append();
	body->m_masterNode = node;
	node->GetInfo().SetAllocator(body->GetWorld()->GetAllocator());
	node->GetInfo().m_body = body;

	if (GetFirst() != node) {
		InsertAfter(GetFirst(), node);
	}
}

void dgBodyMasterList::RemoveBody(dgBody *const body) {
	dgListNode *const node = body->m_masterNode;
	_ASSERTE(node);

	node->GetInfo().RemoveAllJoints();
	_ASSERTE(node->GetInfo().GetCount() == 0);

	Remove(node);
	body->m_masterNode = NULL;
}

dgBodyMasterListRow::dgListNode *dgBodyMasterList::FindConstraintLink(
	const dgBody *const body0, const dgBody *const body1) const {
	_ASSERTE(body0);
	_ASSERTE(body1);
	_ASSERTE(body0->m_masterNode);

	for (dgBodyMasterListRow::dgListNode *node =
			 body0->m_masterNode->GetInfo().GetFirst();
		 node; node = node->GetNext()) {
		if (node->GetInfo().m_bodyNode == body1) {
			return node;
		}
	}

	return NULL;
}

dgBodyMasterListRow::dgListNode *dgBodyMasterList::FindConstraintLinkNext(
	const dgBodyMasterListRow::dgListNode *const me,
	const dgBody *const body) const {
	_ASSERTE(me);
	_ASSERTE(body);
	for (dgBodyMasterListRow::dgListNode *node = me->GetNext(); node;
		 node = node->GetNext()) {
		if (node->GetInfo().m_bodyNode == body) {
			return node;
		}
	}

	return NULL;
}

void dgBodyMasterList::AttachConstraint(dgConstraint *const constraint,
										dgBody *const body0, dgBody *const srcbody1) {
	_ASSERTE(body0);
	dgBody *body1 = srcbody1;
	if (!body1) {
		body1 = body0->GetWorld()->GetSentinelBody();
		constraint->m_isUnilateral = true;
	}
	_ASSERTE(body1);

	constraint->m_body0 = body0;
	constraint->m_body1 = body1;
	constraint->m_link0 = body0->m_masterNode->GetInfo().AddJoint(constraint,
																  body1);
	constraint->m_link1 = body1->m_masterNode->GetInfo().AddJoint(constraint,
																  body0);

	// note this is in observation (to prevent bodies from not going to sleep  inside triggers
	body0->m_equilibrium = body0->m_invMass.m_w ? false : true;
	body1->m_equilibrium = body1->m_invMass.m_w ? false : true;

	//	body0->Unfreeze();
	//	body1->Unfreeze();

	m_constraintCount = m_constraintCount + 1;
}

void dgBodyMasterList::RemoveConstraint(dgConstraint *const constraint) {
	m_constraintCount = m_constraintCount - 1;
	_ASSERTE(((dgInt32)m_constraintCount) >= 0);

	dgBody *const body0 = constraint->m_body0;
	dgBody *const body1 = constraint->m_body1;
	_ASSERTE(body0);
	_ASSERTE(body1);
	_ASSERTE(body0 == constraint->m_link1->GetInfo().m_bodyNode);
	_ASSERTE(body1 == constraint->m_link0->GetInfo().m_bodyNode);

	body0->m_equilibrium = dgUnsigned32(body0->m_invMass.m_w ? false : true);
	body1->m_equilibrium = dgUnsigned32(body1->m_invMass.m_w ? false : true);

	body0->m_masterNode->GetInfo().Remove(constraint->m_link0);
	body1->m_masterNode->GetInfo().Remove(constraint->m_link1);

	body0->m_equilibrium = body0->m_invMass.m_w ? false : true;
	body1->m_equilibrium = body1->m_invMass.m_w ? false : true;

	//	body0->Unfreeze();
	//	body1->Unfreeze();
}

void dgBodyMasterList::SortMasterList() {
	GetFirst()->GetInfo().SortList();

	for (dgListNode *node = GetFirst()->GetNext(); node;) {
		node->GetInfo().SortList();
		dgBody *const body1 = node->GetInfo().GetBody();

		_ASSERTE(GetFirst() != node);

		body1->InvalidateCache();

		dgInt32 key1 = body1->m_uniqueID | ((body1->m_invMass.m_w > 0.0f) << 30);
		dgListNode *const entry = node;
		node = node->GetNext();
		dgListNode *prev = entry->GetPrev();
		for (; prev != GetFirst(); prev = prev->GetPrev()) {
			dgBody *const body0 = prev->GetInfo().GetBody();
			dgInt32 key0 = body0->m_uniqueID | ((body0->m_invMass.m_w > 0.0f) << 30);
			if (key0 < key1) {
				break;
			}
		}

		if (!prev) {
			_ASSERTE(entry == GetFirst());
			RotateToBegin(entry);
		} else {
			InsertAfter(prev, entry);
		}
	}
}
