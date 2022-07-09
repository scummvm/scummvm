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

#ifndef __dgPathFinder__
#define __dgPathFinder__

#include "dgStdafx.h"
#include "dgTree.h"
#include "dgHeap.h"

template<class NODEID, class COST> class dgPathFinder;


#define DG_MAX_PATH_ENUMERATION_CHILDREN	128

template<class NODEID, class COST>
class dgPathNode
{
	friend class dgPathFinder<NODEID, COST>;
	NODEID m_Id;
	COST m_NodeCostToGoal;
	COST m_NodeCostFromSource;
	dgPathNode* m_Next;

	public:
	dgPathNode();
	NODEID GetId() const;
	const dgPathNode* GetNext() const;
	const dgPathNode* GetParent() const;
};

template<class NODEID, class COST>
class dgPathCloseList: public dgTree<dgPathNode<NODEID, COST>, NODEID>
{
	protected:
	dgPathCloseList(): dgTree<dgPathNode<NODEID, COST>, NODEID>() {}
};


template<class NODEID, class COST>
class dgPathOpenHeap: public dgUpHeap<typename dgPathCloseList<NODEID, COST>::dgTreeNode*, COST>
{
	protected:
	dgPathOpenHeap(dgInt32 maxElements)
		:dgUpHeap<typename dgPathCloseList<NODEID, COST>::dgTreeNode*, COST>(maxElements)
	{
	}

	friend class dgPathFinder<NODEID, COST>;
};


template<class NODEID, class COST>
class dgPathFinder: public dgPathCloseList<NODEID, COST>
{
	dgInt32 maxChildren;

	public:
	dgPathFinder(dgInt32 maxElementsInOpenList, dgInt32 nodeMaxChildren);
	virtual ~dgPathFinder();

	virtual const dgPathNode<NODEID, COST>* CalCulatePath (NODEID source, NODEID goal);

	// this funtions must be overloaded by the user
	virtual COST GetCostFromParent(const dgPathNode<NODEID, COST>& node) const;
	virtual COST GetEstimatedCostToGoal(NODEID id) const;
	virtual dgInt32 EnumerateChildren(NODEID parent, NODEID array[]) const;

	dgPathOpenHeap<NODEID, COST> m_openList;
};


template<class NODEID, class COST>
dgPathNode<NODEID, COST>::dgPathNode ()
{
}

template<class NODEID, class COST>
NODEID dgPathNode<NODEID, COST>::GetId () const
{
	return m_Id;
}


template<class NODEID, class COST>
const dgPathNode<NODEID, COST>* dgPathNode<NODEID, COST>::GetNext() const
{
	return m_Next;
}


template<class NODEID, class COST>
const dgPathNode<NODEID, COST>* dgPathNode<NODEID, COST>::GetParent() const
{
	return m_Next;
}


template<class NODEID, class COST>
dgPathFinder<NODEID, COST>::dgPathFinder(
	dgInt32 maxElementsInOpenList, 
	dgInt32 nodeMaxChildren)
	:dgPathCloseList<NODEID, COST>(),
	 m_openList(maxElementsInOpenList)
{
	maxChildren	= nodeMaxChildren;
}


template<class NODEID, class COST>
dgPathFinder<NODEID, COST>::~dgPathFinder()
{
}

template<class NODEID, class COST>
const dgPathNode<NODEID, COST>* dgPathFinder<NODEID, COST>::CalCulatePath (NODEID source, NODEID goal)
{
	dgInt32 count;
	dgInt32 heapMaxCount;
	dgPathNode<NODEID, COST> cell;
	dgPathNode<NODEID, COST>* next;
	dgPathNode<NODEID, COST>* prev;
	dgPathNode<NODEID, COST>* link;
	typename dgPathCloseList<NODEID, COST>::dgTreeNode* node;
	NODEID idArray[DG_MAX_PATH_ENUMERATION_CHILDREN];

	dgPathCloseList<NODEID, COST>& close = *this;

	m_openList.Flush();
	close.RemoveAll();

	cell.m_Id = source;
	cell.m_Next = NULL;
	cell.m_NodeCostFromSource = COST (0);
	cell.m_NodeCostToGoal = GetEstimatedCostToGoal(cell.m_Id);
		  
	node = close.Insert (cell, source);
	
	heapMaxCount = m_openList.GetMaxCount();
	m_openList.Push (node, cell.m_NodeCostFromSource + cell.m_NodeCostToGoal);
	while (m_openList.GetCount()) {
		node = m_openList[0];
		dgPathNode<NODEID, COST>& parent = node->GetInfo();
		if (parent.m_Id == goal) {
			link = &parent;
			next = NULL;
			do {
				prev = link->m_Next;
				link->m_Next = next;
				next = link;
				link = prev;
			} while (link);
			return next;
		}

		m_openList.Pop();
		count = EnumerateChildren(node->GetKey(), idArray);
		cell.m_Next = &parent;
		for (int i = 0; i < count; i ++) {
			bool state;
			cell.m_Id = idArray[i];

			COST newCostFromSource (GetCostFromParent(cell) + parent.m_NodeCostFromSource);
			node = close.Insert (cell, cell.m_Id, state);
			dgPathNode<NODEID, COST>& newCell = node->GetInfo();
			if (state) {
				if (newCell.m_NodeCostFromSource <= newCostFromSource) {
					continue;
				}
				//newCell.m_Next = cell.m_Next;
			}

			newCell.m_NodeCostFromSource = newCostFromSource;
			newCell.m_NodeCostToGoal = GetEstimatedCostToGoal(newCell.m_Id);
			if (m_openList.GetCount() >= heapMaxCount) {
				m_openList.Remove (heapMaxCount);
			}
			m_openList.Push (node, newCell.m_NodeCostFromSource + newCell.m_NodeCostToGoal);
		}
	}
	return NULL;
}

template<class NODEID, class COST>
COST dgPathFinder<NODEID, COST>::GetCostFromParent(const dgPathNode<NODEID, COST>& node) const
{
	return COST (1);
}

template<class NODEID, class COST>
COST dgPathFinder<NODEID, COST>::GetEstimatedCostToGoal(NODEID id) const
{
	return COST (1);
}

template<class NODEID, class COST>
dgInt32 dgPathFinder<NODEID, COST>::EnumerateChildren(NODEID parent, NODEID array[]) const
{
	return 0;
}




#endif



