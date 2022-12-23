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
#include "dgNode.h"

dgInitRtti(dgBaseNode);

dgBaseNode::dgBaseNode(const dgBaseNode &clone) :
	dgRef(clone) {
	dgBaseNode *obj;
	dgBaseNode *newObj;

	Clear();

	for (obj = clone.child; obj; obj = obj->sibling) {
		newObj = (dgBaseNode *) obj->CreateClone();
		newObj->Attach(this);
		newObj->Release();
	}
}

dgBaseNode::~dgBaseNode() {
	dgBaseNode *ptr;
	dgBaseNode *tmp;

	if (child) {
		for (ptr = child; ptr && ptr->Release(); ptr = tmp) {
			ptr->Kill();
			tmp = ptr->sibling;
			ptr->parent = NULL;
			ptr->sibling = NULL;
		}
	}

	for (ptr = sibling; ptr && ptr->Release(); ptr = tmp) {
		ptr->Kill();
		tmp = ptr->sibling;
		ptr->parent = NULL;
		ptr->sibling = NULL;
	}
}

void dgBaseNode::CloneFixUp(const dgBaseNode &clone) {
	dgBaseNode *obj;
	dgBaseNode *cloneChild;

	NEWTON_ASSERT(GetNameID() == clone.GetNameID());

	cloneChild = clone.GetChild();
	for (obj = child; obj; obj = obj->sibling) {
		obj->CloneFixUp(*cloneChild);
		cloneChild = cloneChild->GetSibling();
	}
}

/*
 void dgBaseNode::Save (
 dgFile &file,
 dgSaveType saveType,
 void* context) const
 {
 dgBaseNode *obj;

 for (obj = child; obj; obj = obj->sibling) {
 obj->Save (file, saveType, context);
 }
 }
 */

void dgBaseNode::Attach(dgBaseNode *parentArg, bool addFirst) {
	dgBaseNode *obj;
	NEWTON_ASSERT(!parent);
	NEWTON_ASSERT(!sibling);
	NEWTON_ASSERT(parentArg);

	parent = parentArg;
	if (parent->child) {
		if (addFirst) {
			sibling = parent->child;
			parent->child = this;
		} else {
			for (obj = parent->child; obj->sibling; obj = obj->sibling) {

			}
			obj->sibling = this;
		}
	} else {
		parent->child = this;
	}

	AddRef();
}

void dgBaseNode::Detach() {
	if (parent) {
		if (parent->child == this) {
			parent->child = sibling;
		} else {
			dgBaseNode *ptr;
			for (ptr = parent->child; ptr->sibling != this; ptr = ptr->sibling) {

			}
			ptr->sibling = sibling;
		}
		parent = NULL;
		sibling = NULL;
		Release();
	}
}

const dgBaseNode *dgBaseNode::GetRoot() const {
	const dgBaseNode *root;
	for (root = this; root->parent; root = root->parent) {

	}
	return (const dgBaseNode *) root;
}

const dgBaseNode *dgBaseNode::GetFirst() const {
	const dgBaseNode *ptr;

	for (ptr = (const dgBaseNode *)this; ptr->child; ptr = ptr->child) {

	}
	return ptr;
}

const dgBaseNode *dgBaseNode::GetNext() const {
	const dgBaseNode *x;
	dgBaseNode *ptr;

	if (sibling) {
		return sibling->GetFirst();
	}

	x = (const dgBaseNode *)this;
	for (ptr = parent; ptr && (x == ptr->sibling); ptr = ptr->parent) {
		x = ptr;
	}
	return ptr;
}

const dgBaseNode *dgBaseNode::GetLast() const {
	const dgBaseNode *ptr;

	for (ptr = (const dgBaseNode *)this; ptr->sibling; ptr = ptr->sibling) {

	}
	return ptr;
}

const dgBaseNode *dgBaseNode::GetPrev() const {
	const dgBaseNode *x;
	dgBaseNode *ptr;

	if (child) {
		return child->GetNext();
	}

	x = (const dgBaseNode *)this;
	for (ptr = parent; ptr && (x == ptr->child); ptr = ptr->child) {
		x = ptr;
	}
	return ptr;
}

dgBaseNode *dgBaseNode::GetRoot() {
	dgBaseNode *root;
	for (root = this; root->parent; root = root->parent) {

	}
	return root;
}

dgBaseNode *dgBaseNode::GetFirst() {
	dgBaseNode *ptr;

	for (ptr = this; ptr->child; ptr = ptr->child) {

	}
	return ptr;
}

dgBaseNode *dgBaseNode::GetNext() {
	dgBaseNode *x;
	dgBaseNode *ptr;

	if (sibling) {
		return sibling->GetFirst();
	}

	x = this;
	for (ptr = parent; ptr && (x == ptr->sibling); ptr = ptr->parent) {
		x = ptr;
	}
	return ptr;
}

dgBaseNode *dgBaseNode::GetLast() {
	dgBaseNode *ptr;

	for (ptr = this; ptr->sibling; ptr = ptr->sibling) {

	}
	return ptr;
}

dgBaseNode *dgBaseNode::GetPrev() {
	dgBaseNode *x;
	dgBaseNode *ptr;

	if (child) {
		return child->GetNext();
	}

	x = this;
	for (ptr = parent; ptr && (x == ptr->child); ptr = ptr->child) {
		x = ptr;
	}
	return ptr;
}

const dgBaseNode *dgBaseNode::Find(dgUnsigned32 nameCRC) const {
	const dgBaseNode *ptr;

	for (ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
		if (nameCRC == ptr->GetNameID()) {
			break;
		}
	}
	return ptr;
}

dgBaseNode *dgBaseNode::Find(dgUnsigned32 nameCRC) {
	dgBaseNode *ptr;

	for (ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
		if (nameCRC == ptr->GetNameID()) {
			break;
		}
	}
	return ptr;
}

bool dgBaseNode::SanityCheck() {
	return true;
}

void dgBaseNode::PrintHierarchy(dgFile &file, char *indent) const {
	dgBaseNode *node;
	char newIndent[1024];

	snprintf(newIndent, 1024, "%s   ", indent);
	for (node = child; node; node = node->sibling) {
		node->PrintHierarchy(file, newIndent);
	}
}

void dgBaseNode::DebugPrint(const char *fileName) {
	/*
	 char indent[512];

	 indent[0] = '\0';
	 dgFile file (fileName, "w");
	 PrintHierarchy (file, indent);
	 */
}
