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

#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"

#include "sword25/math/region.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/regionregistry.h"

#define BS_LOG_PREFIX "REGION"

namespace Sword25 {

// Constructor / Destructor
// ------------------------

BS_Region::BS_Region() : m_Valid(false), m_Type(RT_REGION) {
	BS_RegionRegistry::GetInstance().RegisterObject(this);
}

// -----------------------------------------------------------------------------

BS_Region::BS_Region(BS_InputPersistenceBlock &Reader, unsigned int Handle) : m_Valid(false), m_Type(RT_REGION) {
	BS_RegionRegistry::GetInstance().RegisterObject(this, Handle);
	Unpersist(Reader);
}

// -----------------------------------------------------------------------------

unsigned int BS_Region::Create(REGION_TYPE Type) {
	BS_Region * RegionPtr = NULL;
	switch (Type) {
		case RT_REGION:
			RegionPtr = new BS_Region();
			break;

		case RT_WALKREGION:
			RegionPtr = new BS_WalkRegion();
			break;

		default:
			BS_ASSERT(true);
	}

	return BS_RegionRegistry::GetInstance().ResolvePtr(RegionPtr);
}

// -----------------------------------------------------------------------------

unsigned int BS_Region::Create(BS_InputPersistenceBlock & Reader, unsigned int Handle) {
	// Read type
	unsigned int Type;
	Reader.Read(Type);

	// Depending on the type, create a new BS_Region or BS_WalkRegion object
	BS_Region *RegionPtr = NULL;
	if (Type == RT_REGION) {
		RegionPtr = new BS_Region(Reader, Handle);
	} else if (Type == RT_WALKREGION) {
		RegionPtr = new BS_WalkRegion(Reader, Handle);
	} else {
		BS_ASSERT(false);
	}

	return BS_RegionRegistry::GetInstance().ResolvePtr(RegionPtr);
}

// -----------------------------------------------------------------------------

BS_Region::~BS_Region() {
	BS_RegionRegistry::GetInstance().DeregisterObject(this);
}

// -----------------------------------------------------------------------------

bool BS_Region::Init(const BS_Polygon& Contour, const Common::Array<BS_Polygon> *pHoles) {
	// Reset object state
	m_Valid = false;
	m_Position = BS_Vertex(0, 0);
	m_Polygons.clear();

	// Reserve sufficient  space for countour and holes in the polygon list
	if (pHoles)
		m_Polygons.reserve(1 + pHoles->size());
	else
		m_Polygons.reserve(1);

	// The first polygon will be the contour
	m_Polygons.push_back(BS_Polygon());
	m_Polygons[0].Init(Contour.VertexCount, Contour.Vertecies);
	// Make sure that the Vertecies in the Contour are arranged in a clockwise direction
	m_Polygons[0].EnsureCWOrder();

	// Place the hole polygons in the following positions
	if (pHoles) {
		for (unsigned int i = 0; i< pHoles->size(); ++i) {
			m_Polygons.push_back(BS_Polygon());
			m_Polygons[i + 1].Init((*pHoles)[i].VertexCount, (*pHoles)[i].Vertecies);
			m_Polygons[i + 1].EnsureCWOrder();
		}
	}


	// Initialise bounding box
	UpdateBoundingBox();

	m_Valid = true;
	return true;
}

// -----------------------------------------------------------------------------

void BS_Region::UpdateBoundingBox() {
	if (m_Polygons[0].VertexCount) {
		int MinX = m_Polygons[0].Vertecies[0].X;
		int MaxX = m_Polygons[0].Vertecies[0].X;
		int MinY = m_Polygons[0].Vertecies[0].Y;
		int MaxY = m_Polygons[0].Vertecies[0].Y;

		for (int i = 1; i < m_Polygons[0].VertexCount; i++) {
			if (m_Polygons[0].Vertecies[i].X < MinX) MinX = m_Polygons[0].Vertecies[i].X;
			else if (m_Polygons[0].Vertecies[i].X > MaxX) MaxX = m_Polygons[0].Vertecies[i].X;
			if (m_Polygons[0].Vertecies[i].Y < MinY) MinY = m_Polygons[0].Vertecies[i].Y;
			else if (m_Polygons[0].Vertecies[i].Y > MaxY) MaxY = m_Polygons[0].Vertecies[i].Y;
		}

		m_BoundingBox = BS_Rect(MinX, MinY, MaxX + 1, MaxY + 1);
	}
}

// Position Changes
// ----------------

void BS_Region::SetPos(int X, int Y) {
	// Calculate the difference between the old and new position
	BS_Vertex Delta(X - m_Position.X, Y - m_Position.Y);

	// Save the new position
	m_Position = BS_Vertex(X, Y);

	// Move all the vertecies
	for (unsigned int i = 0; i < m_Polygons.size(); ++i) {
		m_Polygons[i] += Delta;
	}

	// Update the bounding box
	UpdateBoundingBox();
}

// -----------------------------------------------------------------------------

void BS_Region::SetPosX(int X) {
	SetPos(X, m_Position.Y);
}

// -----------------------------------------------------------------------------

void BS_Region::SetPosY(int Y) {
	SetPos(m_Position.X, Y);
}

// Point-Region Tests
// ------------------

bool BS_Region::IsPointInRegion(int X, int Y) const {
	// Test whether the point is in the bounding box
	if (m_BoundingBox.IsPointInRect(X, Y)) {
		// Test whether the point is in the contour
		if (m_Polygons[0].IsPointInPolygon(X, Y, true)) {
			// Test whether the point is in a hole
			for (unsigned int i = 1; i < m_Polygons.size(); i++) {
				if (m_Polygons[i].IsPointInPolygon(X,Y, false))
					return false;
			}

			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------------

bool BS_Region::IsPointInRegion(const BS_Vertex &Vertex) const {
	return IsPointInRegion(Vertex.X, Vertex.Y);
}

// -----------------------------------------------------------------------------

BS_Vertex BS_Region::FindClosestRegionPoint(const BS_Vertex &Point) const {
	// Determine whether the point is inside a hole. If that is the case, the closest
	// point on the edge of the hole is determined
	int PolygonIdx = 0;
	{
		for (unsigned int i = 1; i < m_Polygons.size(); ++i) {
			if (m_Polygons[i].IsPointInPolygon(Point)) {
				PolygonIdx = i;
				break;
			}
		}
	}

	const BS_Polygon & Polygon = m_Polygons[PolygonIdx];

	BS_ASSERT(Polygon.VertexCount > 1);

	// For each line of the polygon, calculate the point that is cloest to the given point
	// The point of this set with the smallest distance to the given point is the result.
	BS_Vertex ClosestVertex = FindClosestPointOnLine(Polygon.Vertecies[0], Polygon.Vertecies[1], Point);
	int ClosestVertexDistance2 = ClosestVertex.Distance(Point);
	for (int i = 1; i < Polygon.VertexCount; ++i) {
		int j = (i + 1) % Polygon.VertexCount;

		BS_Vertex CurVertex = FindClosestPointOnLine(Polygon.Vertecies[i], Polygon.Vertecies[j], Point);
		if (CurVertex.Distance(Point) < ClosestVertexDistance2) {
			ClosestVertex = CurVertex;
			ClosestVertexDistance2 = CurVertex.Distance(Point);
		}
	}

	// Determine whether the point is really within the region. This must not be so, as a result of rounding
	// errors can occur at the edge of polygons
	if (IsPointInRegion(ClosestVertex))
		return ClosestVertex;
	else {
		// Try to construct a point within the region - 8 points are tested in the immediate vacinity
		// of the point
		if (IsPointInRegion(ClosestVertex + BS_Vertex(-2, -2)))
			return ClosestVertex + BS_Vertex(-2, -2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(0, -2)))
			return ClosestVertex + BS_Vertex(0, -2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(2, -2)))
			return ClosestVertex + BS_Vertex(2, -2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(-2, 0)))
			return ClosestVertex + BS_Vertex(-2, 0);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(0, 2)))
			return ClosestVertex + BS_Vertex(0, 2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(-2, 2)))
			return ClosestVertex + BS_Vertex(-2, 2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(-2, 0)))
			return ClosestVertex + BS_Vertex(2, 2);
		else if (IsPointInRegion(ClosestVertex + BS_Vertex(2, 2)))
			return ClosestVertex + BS_Vertex(2, 2);

		// If no point could be found that way that lies within the region, find the next point
		ClosestVertex = Polygon.Vertecies[0];
		int ShortestVertexDistance2 = Polygon.Vertecies[0].Distance2(Point);
		{
			for (int i = 1; i < Polygon.VertexCount; i++) {
				int CurDistance2 = Polygon.Vertecies[i].Distance2(Point);
				if (CurDistance2 < ShortestVertexDistance2) {
					ClosestVertex = Polygon.Vertecies[i];
					ShortestVertexDistance2 = CurDistance2;
				}
			}
		}

		BS_LOG_WARNINGLN("Clostest vertex forced because edgepoint was outside region.");
		return ClosestVertex;
	}
}

// -----------------------------------------------------------------------------

BS_Vertex BS_Region::FindClosestPointOnLine(const BS_Vertex &LineStart, const BS_Vertex &LineEnd, const BS_Vertex Point) const {
	float Vector1X = static_cast<float>(Point.X - LineStart.X);
	float Vector1Y = static_cast<float>(Point.Y - LineStart.Y);
	float Vector2X = static_cast<float>(LineEnd.X - LineStart.X);
	float Vector2Y = static_cast<float>(LineEnd.Y - LineStart.Y);
	float Vector2Length = sqrtf(Vector2X * Vector2X + Vector2Y * Vector2Y);
	Vector2X /= Vector2Length;
	Vector2Y /= Vector2Length;
	float Distance = sqrtf(static_cast<float>((LineStart.X - LineEnd.X) * (LineStart.X - LineEnd.X) + 
		(LineStart.Y - LineEnd.Y) * (LineStart.Y - LineEnd.Y)));
	float Dot = Vector1X * Vector2X + Vector1Y * Vector2Y;

	if (Dot <= 0) return LineStart;
	if (Dot >= Distance) return LineEnd;

	BS_Vertex Vector3(static_cast<int>(Vector2X * Dot + 0.5f), static_cast<int>(Vector2Y * Dot + 0.5f));
	return LineStart + Vector3;
}

// -----------------------------------------------------------------------------
// Line of Sight
// -----------------------------------------------------------------------------

bool BS_Region::IsLineOfSight(const BS_Vertex &a, const BS_Vertex &b) const {
	BS_ASSERT(m_Polygons.size());

	// The line must be within the contour polygon, and outside of any hole polygons
	Common::Array<BS_Polygon>::const_iterator iter = m_Polygons.begin();
	if (!(*iter).IsLineInterior(a, b)) return false;
	for (iter++; iter != m_Polygons.end(); iter++)
		if (!(*iter).IsLineExterior(a, b)) return false;

	return true;
}

// -----------------------------------------------------------------------------
// Persistence
// -----------------------------------------------------------------------------

bool BS_Region::Persist(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	Writer.Write(static_cast<unsigned int>(m_Type));
	Writer.Write(m_Valid);
	Writer.Write(m_Position.X);
	Writer.Write(m_Position.Y);
	
	Writer.Write(m_Polygons.size());
	Common::Array<BS_Polygon>::iterator It = m_Polygons.begin();
	while (It != m_Polygons.end()) {
		Result &= It->Persist(Writer);
		++It;
	}

	Writer.Write(m_BoundingBox.left);
	Writer.Write(m_BoundingBox.top);
	Writer.Write(m_BoundingBox.right);
	Writer.Write(m_BoundingBox.bottom);

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_Region::Unpersist(BS_InputPersistenceBlock &Reader) {
	Reader.Read(m_Valid);
	Reader.Read(m_Position.X);
	Reader.Read(m_Position.Y);

	m_Polygons.clear();
	unsigned int PolygonCount;
	Reader.Read(PolygonCount);
	for (unsigned int i = 0; i < PolygonCount; ++i) {
		m_Polygons.push_back(BS_Polygon(Reader));
	}

	Reader.Read(m_BoundingBox.left);
	Reader.Read(m_BoundingBox.top);
	Reader.Read(m_BoundingBox.right);
	Reader.Read(m_BoundingBox.bottom);

	return Reader.IsGood();
}

// -----------------------------------------------------------------------------

BS_Vertex BS_Region::GetCentroid() const {
	if (m_Polygons.size() > 0)
		return m_Polygons[0].GetCentroid();
	return
		BS_Vertex();
}

} // End of namespace Sword25
