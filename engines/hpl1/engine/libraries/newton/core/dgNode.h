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

#ifndef __dgNode__
#define __dgNode__

#include "dgStdafx.h"
#include "dgCRC.h"
#include "dgRef.h"

#include "common/util.h"


class dgFile;
//enum dgSaveType;


class dgBaseNode: public dgRef {
public:
	dgBaseNode *GetChild() const;
	dgBaseNode *GetParent() const;
	dgBaseNode *GetSibling() const;

	void Detach();
	void Attach(dgBaseNode *parent, bool addFirst = false);

	const dgBaseNode *GetRoot() const;
	const dgBaseNode *GetFirst() const;
	const dgBaseNode *GetLast() const;
	const dgBaseNode *GetNext() const;
	const dgBaseNode *GetPrev() const;
	dgBaseNode *GetRoot();
	dgBaseNode *GetFirst();
	dgBaseNode *GetLast();
	dgBaseNode *GetNext();
	dgBaseNode *GetPrev();

	const dgBaseNode *Find(dgUnsigned32 nameCRC) const;
	dgBaseNode *Find(dgUnsigned32 nameCRC);
	const dgBaseNode *Find(const char *name) const;
	dgBaseNode *Find(const char *name);
	void DebugPrint(const char *fileName);
	bool SanityCheck();


protected:
	dgBaseNode();
	dgBaseNode(const char *name);
	dgBaseNode(const dgBaseNode &clone);
	~dgBaseNode();

//	virtual void Save (dgFile &file, dgSaveType saveType, void* const context) const;
	virtual void CloneFixUp(const dgBaseNode &clone);
	virtual void PrintHierarchy(dgFile &file, char *indentation) const;

private:
	inline void Clear();

	dgBaseNode *parent;
	dgBaseNode *child;
	dgBaseNode *sibling;

};

template<class T>
class dgNode: public dgBaseNode {
public:
	dgNode();
	dgNode(const char *name);
	void Attach(T *parentNode, bool addFirst = false);
	void Detach();
	T *GetChild() const;
	T *GetSibling() const;
	T *GetParent() const;
	T *GetRoot() const;
	T *GetRoot();
	T *GetFirst() const;
	T *GetLast() const;
	T *GetNext() const;
	T *GetPrev() const;
	T *Find(dgUnsigned32 nameCRC) const;
	T *Find(const char *name) const;

protected:
	dgNode(const T &clone);
	virtual ~dgNode();
	dgRef *CreateClone() const;

};





inline dgBaseNode::dgBaseNode()
	: dgRef() {
	Clear();
}

inline dgBaseNode::dgBaseNode(const char *name)
	: dgRef(name) {
	Clear();
}


inline void dgBaseNode::Clear() {
	child = NULL;
	parent = NULL;
	sibling = NULL;
}


inline dgBaseNode *dgBaseNode::GetChild() const {
	return child;
}

inline dgBaseNode *dgBaseNode::GetSibling() const {
	return sibling;
}

inline dgBaseNode *dgBaseNode::GetParent() const {
	return parent;
}


inline const dgBaseNode *dgBaseNode::Find(const char *name) const {
	return Find(dgCRC(name));
}

inline dgBaseNode *dgBaseNode::Find(const char *name) {
	return Find(dgCRC(name));
}



template<class T>
dgNode<T>::dgNode()
	: dgBaseNode() {
}

template<class T>
dgNode<T>::dgNode(const T &clone)
	: dgBaseNode(clone) {
}

template<class T>
dgNode<T>::dgNode(const char *name)
	: dgBaseNode(name) {
}

template<class T>
dgNode<T>::~dgNode() {
}


template<class T>
dgRef *dgNode<T>::CreateClone() const {
	return new T(*const_cast<T *>((const T *)this));
}

template<class T>
void dgNode<T>::Attach(T *parentNode, bool addFirst) {
	dgBaseNode::Attach(parentNode, addFirst);
}

template<class T>
void dgNode<T>::Detach() {
	dgBaseNode::Detach();
}

template<class T>
T *dgNode<T>::GetChild() const {
	return (T *) dgBaseNode::GetChild();
}

template<class T>
T *dgNode<T>::GetSibling() const {
	return (T *) dgBaseNode::GetSibling();
}

template<class T>
T *dgNode<T>::GetParent() const {
	return (T *) dgBaseNode::GetParent();
}


template<class T>
T *dgNode<T>::GetRoot() const {
	return (T *) dgBaseNode::GetRoot();
}


template<class T>
T *dgNode<T>::GetFirst() const {
	return (T *) dgBaseNode::GetFirst();
}

template<class T>
T *dgNode<T>::GetLast() const {
	return (T *) dgBaseNode::GetLast();
}


template<class T>
T *dgNode<T>::GetNext() const {
	return (T *) dgBaseNode::GetNext();
}

template<class T>
T *dgNode<T>::GetPrev() const {
	return (T *) dgBaseNode::GetPrev();
}


template<class T>
T *dgNode<T>::Find(dgUnsigned32 nameCRC) const {
	return (T *) dgBaseNode::Find(nameCRC);
}

template<class T>
T *dgNode<T>::Find(const char *name) const {
	return (T *) dgBaseNode::Find(name);
}




#endif
