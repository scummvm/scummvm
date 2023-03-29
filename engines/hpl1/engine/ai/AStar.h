/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_A_STAR_H
#define HPL_A_STAR_H

#include "common/list.h"
#include "hpl1/engine/game/GameTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cAINodeContainer;
class cAINode;

//--------------------------------------

typedef Hpl1::Std::set<cAINode *> tAINodeSet;
typedef tAINodeSet::iterator tAINodeSetIt;

//--------------------------------------

typedef Common::List<cAINode *> tAINodeList;
typedef tAINodeList::iterator tAINodeListIt;

//--------------------------------------

class cAStarNode {
public:
	cAStarNode(cAINode *apAINode);

	float mfCost;
	float mfDistance;

	cAStarNode *mpParent;
	cAINode *mpAINode;
};

class cAStarNodeCompare {
public:
	bool operator()(cAStarNode *apNodeA, cAStarNode *apNodeB) const;
};

typedef Hpl1::Std::set<cAStarNode *, cAStarNodeCompare> tAStarNodeSet;
typedef tAStarNodeSet::iterator tAStarNodeSetIt;

//--------------------------------------
class cAStarHandler;

class iAStarCallback {
public:
	virtual ~iAStarCallback() {}

	virtual bool CanAddNode(cAINode *apParentNode, cAINode *apChildNode) = 0;
};

//--------------------------------------

class cAStarHandler {
public:
	cAStarHandler(cAINodeContainer *apContainer);
	~cAStarHandler();

	bool GetPath(const cVector3f &avStart, const cVector3f &avGoal, tAINodeList *apNodeList);

	/**
	 * Set max number of times the algorithm is iterated.
	 * \param alX -1 = until OpenList is empty
	 */
	void SetMaxIterations(int alX) { mlMaxIterations = alX; }

	void SetCallback(iAStarCallback *apCallback) { mpCallback = apCallback; }

private:
	void IterateAlgorithm();

	void AddOpenNode(cAINode *apAINode, cAStarNode *apParent, float afDistance);

	cAStarNode *GetBestNode();

	float Cost(float afDistance, cAINode *apAINode, cAStarNode *apParent);
	float Heuristic(const cVector3f &avStart, const cVector3f &avGoal);

	bool IsGoalNode(cAINode *apAINode);

	cVector3f mvGoal;

	cAStarNode *mpGoalNode;
	tAINodeSet m_setGoalNodes;

	cAINodeContainer *mpContainer;

	int mlMaxIterations;

	iAStarCallback *mpCallback;

	tAStarNodeSet m_setOpenList;
	tAStarNodeSet m_setClosedList;
};

} // namespace hpl

#endif // HPL_A_STAR_H
