/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/line.h"

#define BS_LOG_PREFIX "WALKREGION"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

static const int infinity = (~(-1));

// -----------------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------------

BS_WalkRegion::BS_WalkRegion() {
	m_Type = RT_WALKREGION;
}

// -----------------------------------------------------------------------------

BS_WalkRegion::BS_WalkRegion(BS_InputPersistenceBlock &Reader, unsigned int Handle) :
	BS_Region(Reader, Handle) {
	m_Type = RT_WALKREGION;
	Unpersist(Reader);
}

// -----------------------------------------------------------------------------

BS_WalkRegion::~BS_WalkRegion() {
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Init(const BS_Polygon &Contour, const Common::Array<BS_Polygon> *pHoles) {
	// Default initialisation of the region
	if (!BS_Region::Init(Contour, pHoles)) return false;

	// Prepare structures for pathfinding
	InitNodeVector();
	ComputeVisibilityMatrix();

	// Signal success
	return true;
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::QueryPath(BS_Vertex StartPoint, BS_Vertex EndPoint, BS_Path &Path) {
	BS_ASSERT(Path.empty());

	// If the start and finish are identical, no path can be found trivially
	if (StartPoint == EndPoint) return true;

	// Ensure that the start and finish are valid and find new start points if either
	// are outside the polygon
	if (!CheckAndPrepareStartAndEnd(StartPoint, EndPoint)) return false;

	// If between the start and point a line of sight exists, then it can be returned.
	if (IsLineOfSight(StartPoint, EndPoint)) {
		Path.push_back(StartPoint);
		Path.push_back(EndPoint);
		return true;
	}

	return FindPath(StartPoint, EndPoint, Path);
}

// -----------------------------------------------------------------------------

struct DijkstraNode {
	typedef Common::Array<DijkstraNode> Container;
	typedef Container::iterator Iter;
	typedef Container::const_iterator ConstIter;

	DijkstraNode() : Cost(infinity), Chosen(false) {};
	ConstIter   ParentIter;
	int         Cost;
	bool        Chosen;
};

static void InitDijkstraNodes(DijkstraNode::Container &DijkstraNodes, const BS_Region &Region,
                              const BS_Vertex &Start, const Common::Array<BS_Vertex> &Nodes) {
	// Allocate sufficient space in the array
	DijkstraNodes.resize(Nodes.size());

	// Initialise all the nodes which are visible from the starting node
	DijkstraNode::Iter DijkstraIter = DijkstraNodes.begin();
	for (Common::Array<BS_Vertex>::const_iterator NodesIter = Nodes.begin();
	        NodesIter != Nodes.end(); NodesIter++, DijkstraIter++) {
		(*DijkstraIter).ParentIter = DijkstraNodes.end();
		if (Region.IsLineOfSight(*NodesIter, Start))(*DijkstraIter).Cost = (*NodesIter).Distance(Start);
	}
	BS_ASSERT(DijkstraIter == DijkstraNodes.end());
}

static DijkstraNode::Iter ChooseClosestNode(DijkstraNode::Container &Nodes) {
	DijkstraNode::Iter ClosestNodeInter = Nodes.end();
	int MinCost = infinity;

	for (DijkstraNode::Iter iter = Nodes.begin(); iter != Nodes.end(); iter++) {
		if (!(*iter).Chosen && (*iter).Cost < MinCost) {
			MinCost = (*iter).Cost;
			ClosestNodeInter = iter;
		}
	}

	return ClosestNodeInter;
}

static void RelaxNodes(DijkstraNode::Container &Nodes,
                       const Common::Array< Common::Array<int> > &VisibilityMatrix,
                       const DijkstraNode::ConstIter &CurNodeIter) {
	// All the successors of the current node that have not been chosen will be
	// inserted into the boundary node list, and the cost will be updated if
	// a shorter path has been found to them.

	int CurNodeIndex = CurNodeIter - Nodes.begin();
	for (unsigned int i = 0; i < Nodes.size(); i++) {
		int Cost = VisibilityMatrix[CurNodeIndex][i];
		if (!Nodes[i].Chosen && Cost != infinity) {
			int TotalCost = (*CurNodeIter).Cost + Cost;
			if (TotalCost < Nodes[i].Cost) {
				Nodes[i].ParentIter = CurNodeIter;
				Nodes[i].Cost = TotalCost;
			}
		}
	}
}

static void RelaxEndPoint(const BS_Vertex &CurNodePos,
                          const DijkstraNode::ConstIter &CurNodeIter,
                          const BS_Vertex &EndPointPos,
                          DijkstraNode &EndPoint,
                          const BS_Region &Region) {
	if (Region.IsLineOfSight(CurNodePos, EndPointPos)) {
		int TotalCost = (*CurNodeIter).Cost + CurNodePos.Distance(EndPointPos);
		if (TotalCost < EndPoint.Cost) {
			EndPoint.ParentIter = CurNodeIter;
			EndPoint.Cost = TotalCost;
		}
	}
}

bool BS_WalkRegion::FindPath(const BS_Vertex &Start, const BS_Vertex &End, BS_Path &Path) const {
	// This is an implementation of Dijkstra's algorithm

	// Initialise edge node list
	DijkstraNode::Container DijkstraNodes;
	InitDijkstraNodes(DijkstraNodes, *this, Start, m_Nodes);

	// The end point is treated separately, since it does not exist in the visibility graph
	DijkstraNode EndPoint;

	// Since a node is selected each round from the node list, and can never be selected again
	// after that, the maximum number of loop iterations is limited by the number of nodes
	for (unsigned int i = 0; i < m_Nodes.size(); i++) {
		// Determine the nearest edge node in the node list
		DijkstraNode::Iter NodeInter = ChooseClosestNode(DijkstraNodes);
		(*NodeInter).Chosen = true;

		// If no free nodes are absent from the edge node list, there is no path from start
		// to end node. This case should never occur, since the number of loop passes is
		// limited, but etter safe than sorry
		if (NodeInter == DijkstraNodes.end()) return false;

		// If the destination point is closer than the point cost, scan can stop
		if (EndPoint.Cost <= (*NodeInter).Cost) {
			// Insert the end point in the list
			Path.push_back(End);

			// The list is done in reverse order and inserted into the path
			DijkstraNode::ConstIter CurNode = EndPoint.ParentIter;
			while (CurNode != DijkstraNodes.end()) {
				BS_ASSERT((*CurNode).Chosen);
				Path.push_back(m_Nodes[CurNode - DijkstraNodes.begin()]);
				CurNode = (*CurNode).ParentIter;
			}

			// The starting point is inserted into the path
			Path.push_back(Start);

			// The nodes of the path must be untwisted, as they were extracted in reverse order.
			// This step could be saved if the path from end to the beginning was desired
			ReverseArray<BS_Vertex>(Path);

			return true;
		}

		// Relaxation step for nodes of the graph, and perform the end nodes
		RelaxNodes(DijkstraNodes, m_VisibilityMatrix, NodeInter);
		RelaxEndPoint(m_Nodes[NodeInter - DijkstraNodes.begin()], NodeInter, End, EndPoint, *this);
	}

	// If the loop has been completely run through, all the nodes have been chosen, and still
	// no path was found. There is therefore no path available
	return false;
}

// -----------------------------------------------------------------------------

void BS_WalkRegion::InitNodeVector() {
	// Empty the Node list
	m_Nodes.clear();

	// Determine the number of nodes
	int NodeCount = 0;
	{
		for (unsigned int i = 0; i < m_Polygons.size(); i++)
			NodeCount += m_Polygons[i].VertexCount;
	}

	// Knoten-Vector füllen
	m_Nodes.reserve(NodeCount);
	{
		for (unsigned int j = 0; j < m_Polygons.size(); j++)
			for (int i = 0; i < m_Polygons[j].VertexCount; i++)
				m_Nodes.push_back(m_Polygons[j].Vertecies[i]);
	}
}

// -----------------------------------------------------------------------------

void BS_WalkRegion::ComputeVisibilityMatrix() {
	// Initialise visibility matrix
	m_VisibilityMatrix = Common::Array< Common::Array <int> >();
	for (uint idx = 0; idx < m_Nodes.size(); ++idx) {
		Common::Array<int> arr;
		for (uint idx2 = 0; idx2 < m_Nodes.size(); ++idx2)
			arr.push_back(infinity);

		m_VisibilityMatrix.push_back(arr);
	}

	// Calculate visibility been vertecies
	for (unsigned int j = 0; j < m_Nodes.size(); ++j) {
		for (unsigned int i = j; i < m_Nodes.size(); ++i)   {
			if (IsLineOfSight(m_Nodes[i], m_Nodes[j])) {
				// There is a line of sight, so save the distance between the two
				int Distance = m_Nodes[i].Distance(m_Nodes[j]);
				m_VisibilityMatrix[i][j] = Distance;
				m_VisibilityMatrix[j][i] = Distance;
			} else {
				// There is no line of sight, so save infinity as the distance
				m_VisibilityMatrix[i][j] = infinity;
				m_VisibilityMatrix[j][i] = infinity;
			}
		}
	}
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::CheckAndPrepareStartAndEnd(BS_Vertex &Start, BS_Vertex &End) const {
	if (!IsPointInRegion(Start)) {
		BS_Vertex NewStart = FindClosestRegionPoint(Start);

		// Check to make sure the point is really in the region. If not, stop with an error
		if (!IsPointInRegion(NewStart)) {
			BS_LOG_ERRORLN("Constructed startpoint ((%d,%d) from (%d,%d)) is not inside the region.",
			               NewStart.X, NewStart.Y,
			               Start.X, Start.Y);
			return false;
		}

		Start = NewStart;
	}

	// If the destination is outside the region, a point is determined that is within the region,
	// and that is used as an endpoint instead
	if (!IsPointInRegion(End)) {
		BS_Vertex NewEnd = FindClosestRegionPoint(End);

		// Make sure that the determined point is really within the region
		if (!IsPointInRegion(NewEnd)) {
			BS_LOG_ERRORLN("Constructed endpoint ((%d,%d) from (%d,%d)) is not inside the region.",
			               NewEnd.X, NewEnd.Y,
			               End.X, End.Y);
			return false;
		}

		End = NewEnd;
	}

	// Signal success
	return true;
}

// -----------------------------------------------------------------------------

void BS_WalkRegion::SetPos(int X, int Y) {
	// Calculate the difference between old and new position
	BS_Vertex Delta(X - m_Position.X, Y - m_Position.Y);

	// Move all the nodes
	for (unsigned int i = 0; i < m_Nodes.size(); i++) m_Nodes[i] += Delta;

	// Move regions
	BS_Region::SetPos(X, Y);
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Persist(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	// Persist the parent region
	Result &= BS_Region::Persist(Writer);

	// Persist the nodes
	Writer.Write(m_Nodes.size());
	Common::Array<BS_Vertex>::const_iterator It = m_Nodes.begin();
	while (It != m_Nodes.end()) {
		Writer.Write(It->X);
		Writer.Write(It->Y);
		++It;
	}

	// Persist the visibility matrix
	Writer.Write(m_VisibilityMatrix.size());
	Common::Array< Common::Array<int> >::const_iterator RowIter = m_VisibilityMatrix.begin();
	while (RowIter != m_VisibilityMatrix.end()) {
		Writer.Write(RowIter->size());
		Common::Array<int>::const_iterator ColIter = RowIter->begin();
		while (ColIter != RowIter->end()) {
			Writer.Write(*ColIter);
			++ColIter;
		}

		++RowIter;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Unpersist(BS_InputPersistenceBlock &Reader) {
	bool Result = true;

	// The parent object was already loaded in the constructor of BS_Region, so at
	// this point only the additional data from BS_WalkRegion needs to be loaded

	// Node load
	unsigned int NodeCount;
	Reader.Read(NodeCount);
	m_Nodes.clear();
	m_Nodes.resize(NodeCount);
	Common::Array<BS_Vertex>::iterator It = m_Nodes.begin();
	while (It != m_Nodes.end()) {
		Reader.Read(It->X);
		Reader.Read(It->Y);
		++It;
	}

	// Visibility matrix load
	unsigned int RowCount;
	Reader.Read(RowCount);
	m_VisibilityMatrix.clear();
	m_VisibilityMatrix.resize(RowCount);
	Common::Array< Common::Array<int> >::iterator RowIter = m_VisibilityMatrix.begin();
	while (RowIter != m_VisibilityMatrix.end()) {
		unsigned int ColCount;
		Reader.Read(ColCount);
		RowIter->resize(ColCount);
		Common::Array<int>::iterator ColIter = RowIter->begin();
		while (ColIter != RowIter->end()) {
			Reader.Read(*ColIter);
			++ColIter;
		}

		++RowIter;
	}

	return Result && Reader.IsGood();
}

} // End of namespace Sword25
