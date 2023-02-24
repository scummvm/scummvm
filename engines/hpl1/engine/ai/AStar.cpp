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

#include "hpl1/engine/ai/AStar.h"

#include "hpl1/engine/ai/AINodeContainer.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// NODE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAStarNode::cAStarNode(cAINode *apAINode) {
	mpParent = NULL;
	mpAINode = apAINode;
}

//-----------------------------------------------------------------------

bool cAStarNodeCompare::operator()(cAStarNode *apNodeA, cAStarNode *apNodeB) const {
	return apNodeA->mpAINode < apNodeB->mpAINode;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAStarHandler::cAStarHandler(cAINodeContainer *apContainer) {
	mlMaxIterations = -1;

	mpContainer = apContainer;

	mpCallback = NULL;
}

//-----------------------------------------------------------------------

cAStarHandler::~cAStarHandler() {
	STLDeleteAll(m_setClosedList);
	STLDeleteAll(m_setOpenList);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cAStarHandler::GetPath(const cVector3f &avStart, const cVector3f &avGoal, tAINodeList *apNodeList) {
	/////////////////////////////////////////////////
	// check if there is free path from start to goal
	if (mpContainer->FreePath(avStart, avGoal, 3)) {
		mpGoalNode = NULL;
		return true;
	}

	////////////////////////////////////////////////
	// Reset all variables
	// These should just be cleared and pools used instead.
	STLDeleteAll(m_setClosedList);
	STLDeleteAll(m_setOpenList);
	m_setGoalNodes.clear();
	mpGoalNode = NULL;

	// Set goal position
	mvGoal = avGoal;

	float fMaxHeight = mpContainer->GetMaxHeight() * 1.5f;

	////////////////////////////////////////////////
	// Find nodes reachable from the start and goal position (use double 2*2 distance)
	float fMaxDist = mpContainer->GetMaxEdgeDistance() * 2; // float fMaxDist = mpContainer->GetMaxEdgeDistance()*mpContainer->GetMaxEdgeDistance()*4;

	/////////////////////
	// Check with Start
	// Log(" Get Start\n");
	cAINodeIterator startNodeIt = mpContainer->GetNodeIterator(avStart, fMaxDist);
	while (startNodeIt.HasNext()) {
		cAINode *pAINode = startNodeIt.Next();
		// Log("Check node: %s\n",pAINode->GetName().c_str());

		float fHeight = fabs(avStart.y - pAINode->GetPosition().y);
		float fDist = cMath::Vector3Dist(avStart, pAINode->GetPosition()); // float fDist = cMath::Vector3DistSqr(avStart,pAINode->GetPosition());
		if (fDist < fMaxDist && fHeight <= fMaxHeight) {
			// Check if path is clear
			if (mpContainer->FreePath(avStart, pAINode->GetPosition(), -1,
									  eAIFreePathFlag_SkipDynamic)) {
				AddOpenNode(pAINode, NULL, fDist);
			}
		}
	}
	// Log(" Found start\n");

	////////////////////////////////
	// Check with Goal
	// Log(" Get Goal\n");
	cAINodeIterator goalNodeIt = mpContainer->GetNodeIterator(avGoal, fMaxDist);
	while (goalNodeIt.HasNext()) {
		cAINode *pAINode = goalNodeIt.Next();
		// Log("Check node: %s\n",pAINode->GetName().c_str());

		float fHeight = fabs(avGoal.y - pAINode->GetPosition().y);
		float fDist = cMath::Vector3Dist(avGoal, pAINode->GetPosition()); // fDist = cMath::Vector3DistSqr(avGoal,pAINode->GetPosition());
		if (fDist < fMaxDist && fHeight <= fMaxHeight) {
			// Check if path is clear
			if (mpContainer->FreePath(avGoal, pAINode->GetPosition(), 3)) {
				m_setGoalNodes.insert(pAINode);
			}
		}
	}
	// Log(" Found goal\n");

	/*for(int i=0; i<mpContainer->GetNodeNum(); ++i)
	{
		cAINode *pAINode = mpContainer->GetNode(i);

		////////////////////////////////
		//Check with Start
		float fHeight = fabs(avStart.y - pAINode->GetPosition().y);
		float fDist = cMath::Vector3Dist(avStart,pAINode->GetPosition());
		//float fDist = cMath::Vector3DistSqr(avStart,pAINode->GetPosition());
		if(fDist < fMaxDist && fHeight <= fMaxHeight)
		{
			//Check if path is clear
			if(mpContainer->FreePath(avStart,pAINode->GetPosition(),-1))
			{
				AddOpenNode(pAINode,NULL,fDist);
			}
		}

		////////////////////////////////
		//Check with Goal
		fHeight = fabs(avGoal.y - pAINode->GetPosition().y);
		fDist = cMath::Vector3Dist(avGoal,pAINode->GetPosition());
		//fDist = cMath::Vector3DistSqr(avGoal,pAINode->GetPosition());
		if(fDist < fMaxDist && fHeight <= fMaxHeight)
		{
			//Check if path is clear
			if(mpContainer->FreePath(avGoal,pAINode->GetPosition(),3))
			{
				m_setGoalNodes.insert(pAINode);
			}
		}
	}*/

	////////////////////////////////////////////////
	// Iterate the algorithm
	IterateAlgorithm();

	////////////////////////////////////////////////
	// Check if goal was found, if so build path.
	if (mpGoalNode) {
		if (apNodeList) {
			cAStarNode *pParentNode = mpGoalNode;
			while (pParentNode != NULL) {
				apNodeList->push_back(pParentNode->mpAINode);
				pParentNode = pParentNode->mpParent;
			}
		}

		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cAStarHandler::IterateAlgorithm() {
	int lIterationCount = 0;
	while (m_setOpenList.empty() == false && (mlMaxIterations < 0 || lIterationCount < mlMaxIterations)) {
		cAStarNode *pNode = GetBestNode();
		cAINode *pAINode = pNode->mpAINode;

		//////////////////////
		// Check if current node can reach goal
		if (IsGoalNode(pAINode)) {
			mpGoalNode = pNode;
			break;
		}

		/////////////////////
		// Add nodes connected to current
		int lEdgeCount = pAINode->GetEdgeNum();
		for (int i = 0; i < lEdgeCount; ++i) {
			cAINodeEdge *pEdge = pAINode->GetEdge(i);

			if (mpCallback == NULL || mpCallback->CanAddNode(pAINode, pEdge->mpNode)) {
				AddOpenNode(pEdge->mpNode, pNode, pNode->mfDistance + pEdge->mfDistance);
				// AddOpenNode(pEdge->mpNode, pNode, pNode->mfDistance + pEdge->mfSqrDistance);
			}
		}

		++lIterationCount;
	}
}

//-----------------------------------------------------------------------

void cAStarHandler::AddOpenNode(cAINode *apAINode, cAStarNode *apParent, float afDistance) {
	// TODO: free path check with dynamic objects here.

	// TODO: Some pooling here would be good.
	cAStarNode *pNode = hplNew(cAStarNode, (apAINode));

	// Check if it is in closed list.
	tAStarNodeSetIt it = m_setClosedList.find(pNode);
	if (it != m_setClosedList.end()) {
		hplDelete(pNode);
		return;
	}

	// Add it if it wasn't already inserted
	const auto test = m_setOpenList.find(pNode);
	if (test != m_setOpenList.end()) {
		hplDelete(pNode);
		return;
	}
	m_setOpenList.insert(pNode);

	pNode->mfDistance = afDistance;
	pNode->mfCost = Cost(afDistance, apAINode, apParent) + Heuristic(pNode->mpAINode->GetPosition(), mvGoal);
	pNode->mpParent = apParent;
}

//-----------------------------------------------------------------------

cAStarNode *cAStarHandler::GetBestNode() {
	tAStarNodeSetIt it = m_setOpenList.begin();
	tAStarNodeSetIt bestIt = it;
	cAStarNode *pBestNode = *it;
	++it;

	// Iterate open list and find the best node.
	for (; it != m_setOpenList.end(); ++it) {
		cAStarNode *pNode = *it;
		if (pBestNode->mfCost > pNode->mfCost) {
			pBestNode = pNode;
			bestIt = it;
		}
	}

	// Remove node from open
	m_setOpenList.erase(bestIt);

	// Add to closed list
	m_setClosedList.insert(pBestNode);

	return pBestNode;
}

//-----------------------------------------------------------------------

float cAStarHandler::Cost(float afDistance, cAINode *apAINode, cAStarNode *apParent) {
	if (apParent) {
		float fHeight = (1 + fabs(apAINode->GetPosition().y - apParent->mpAINode->GetPosition().y));
		return afDistance * fHeight;
	} else
		return afDistance;
}

//-----------------------------------------------------------------------

float cAStarHandler::Heuristic(const cVector3f &avStart, const cVector3f &avGoal) {
	// return cMath::Vector3DistSqr(avStart, avGoal);
	return cMath::Vector3Dist(avStart, avGoal);
}

//-----------------------------------------------------------------------

bool cAStarHandler::IsGoalNode(cAINode *apAINode) {
	tAINodeSetIt it = m_setGoalNodes.find(apAINode);
	if (it == m_setGoalNodes.end())
		return false;

	return true;
}

//-----------------------------------------------------------------------
} // namespace hpl
