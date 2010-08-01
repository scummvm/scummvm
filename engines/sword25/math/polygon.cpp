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

#include <math.h>

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/math/polygon.h"
#include "sword25/math/line.h"

namespace Sword25 {

#define max(a,b) (((a) > (b)) ? (a) : (b))

// Constructor / Destructor
// --------------------------

BS_Polygon::BS_Polygon() : VertexCount(0), Vertecies(NULL) {
}

BS_Polygon::BS_Polygon(int VertexCount, const BS_Vertex *Vertecies) : VertexCount(0), Vertecies(NULL) { 
	Init(VertexCount, Vertecies);
}

BS_Polygon::BS_Polygon(const BS_Polygon &Other) : VertexCount(0), Vertecies(NULL) {
	Init(Other.VertexCount, Other.Vertecies);
}

BS_Polygon::BS_Polygon(BS_InputPersistenceBlock &Reader) : VertexCount(0), Vertecies(NULL) {
	Unpersist(Reader);
}

BS_Polygon::~BS_Polygon() {
	delete[] Vertecies;
}

// Initialisation
// ---------------

bool BS_Polygon::Init(int VertexCount, const BS_Vertex *Vertecies) {
	// Rember the old obstate to restore it if an error occurs whilst initialising it with the new data
	int OldVertexCount = this->VertexCount;
	BS_Vertex *OldVertecies = this->Vertecies;

	this->VertexCount = VertexCount;
	this->Vertecies = new BS_Vertex[VertexCount + 1];
	memcpy(this->Vertecies, Vertecies, sizeof(BS_Vertex) * VertexCount);
	// TODO:
	// Duplicate and remove redundant vertecies (Superflous = 3 co-linear verts)
	// _WeedRepeatedVertecies();
	// The first vertex is repeated at the end of the vertex array; this simplifies
	// some algorithms, running through the edges and thus can save the overflow control.
	this->Vertecies[VertexCount] = this->Vertecies[0];

	// If the polygon is self-intersecting, the object state is restore, and an error signalled
	if (CheckForSelfIntersection()) {
		delete[] this->Vertecies;
		this->Vertecies = OldVertecies;
		this->VertexCount = OldVertexCount;

		// BS_LOG_ERROR("POLYGON: Tried to create a self-intersecting polygon.\n");
		return false;
	}

	// Release old vertex list
	delete[] OldVertecies;

	// Calculate properties of the polygon
	m_IsCW = ComputeIsCW();
	m_IsConvex = ComputeIsConvex();
	m_Centroid = ComputeCentroid();

	return true;
}

// Review the order of the Vertecies
// ---------------------------------

bool BS_Polygon::IsCW() const {
	return m_IsCW;
}

bool BS_Polygon::IsCCW() const {
	return !IsCW();
}

bool BS_Polygon::ComputeIsCW() const {
	if (VertexCount) {
		// Find the vertex on extreme bottom right
		int V2Index = FindLRVertexIndex();

		// Find the vertex before and after it
		int V1Index = (V2Index + (VertexCount - 1)) % VertexCount;
		int V3Index = (V2Index + 1) % VertexCount;

		// Cross product form
		// If the cross product of the vertex lying fartherest bottom left is positive,
		// the vertecies arrranged in a clockwise order. Otherwise counter-clockwise
		if (CrossProduct(Vertecies[V1Index], Vertecies[V2Index], Vertecies[V3Index]) >= 0) return true;
	}

	return false;
}

int BS_Polygon::FindLRVertexIndex() const {
	if (VertexCount) {
		int CurIndex = 0;
		int MaxX = Vertecies[0].X;
		int MaxY = Vertecies[0].Y;

		for (int i = 1; i < VertexCount; i++) {
			if (Vertecies[i].Y > MaxY ||
			   (Vertecies[i].Y == MaxY && Vertecies[i].X > MaxX)) {
				MaxX = Vertecies[i].X;
				MaxY = Vertecies[i].Y;
				CurIndex = i;
			}
		}

		return CurIndex;
	}
	
	return -1;
}

// Testing for Convex / Concave
// ------------------------

bool BS_Polygon::IsConvex() const {
	return m_IsConvex;
}

bool BS_Polygon::IsConcave() const {
	return !IsConvex();
}

bool BS_Polygon::ComputeIsConvex() const {
	// Polygons with three or less Vertecies can only be convex
	if (VertexCount <= 3) return true;

	// All angles in the polygon computed will have the same direction sign if the polygon is convex
	int Flag = 0;
	for (int i = 0; i < VertexCount; i++) {
		// Determine the next two vertecies to check
		int j = (i + 1) % VertexCount;
		int k = (i + 2) % VertexCount;

		// Calculate the cross product of the three vertecies
		int Cross = CrossProduct(Vertecies[i], Vertecies[j], Vertecies[k]);

		// The lower two bits of the flag represent the following:
		// 0: negative angle occurred
		// 1: positive angle occurred

		// The sign of the current angle is recorded in Flag
		if (Cross < 0)
			Flag |= 1;
		else if (Cross > 0)
			Flag |= 2;

		// If flag is 3, there are both positive and negative angles; so the polygon is concave
		if (Flag == 3) return false;
	}

	// Polygon is convex
	return true;
}

// Make a determine vertex order
// -----------------------------

void BS_Polygon::EnsureCWOrder() {
	if (!IsCW())
		ReverseVertexOrder();
}

void BS_Polygon::EnsureCCWOrder() {
	if (!IsCCW())
		ReverseVertexOrder();
}

// Reverse the order of vertecies
// ------------------------------

void BS_Polygon::ReverseVertexOrder() {
	// Vertecies are exchanged in pairs, until the list has been completely reversed
	for (int i = 0; i < VertexCount / 2; i++) {
		BS_Vertex tempVertex = Vertecies[i];
		Vertecies[i] = Vertecies[VertexCount - i - 1];
		Vertecies[VertexCount - i - 1] = tempVertex;
	}

	// Vertexordnung neu berechnen.
	m_IsCW = ComputeIsCW();
}

// Cross Product
// -------------

int BS_Polygon::CrossProduct(const BS_Vertex &V1, const BS_Vertex &V2, const BS_Vertex &V3) const {
	return (V2.X - V1.X) * (V3.Y - V2.Y) -
		   (V2.Y - V1.Y) * (V3.X - V2.X);
}

// Scalar Product
// --------------

int BS_Polygon::DotProduct(const BS_Vertex &V1, const BS_Vertex &V2, const BS_Vertex &V3) const {
	return (V1.X - V2.X) * (V3.X - V2.X) +
		   (V1.Y - V2.Y) * (V3.X - V2.Y);
}

// Check for self-intersections
// ----------------------------

bool BS_Polygon::CheckForSelfIntersection() const {
	// TODO: Finish this
	/*
	float AngleSum = 0.0f;
	for (int i = 0; i < VertexCount; i++) {
		int j = (i + 1) % VertexCount;
		int k = (i + 2) % VertexCount;

		float Dot = DotProduct(Vertecies[i], Vertecies[j], Vertecies[k]);

		// Skalarproduct normalisieren
		float Length1 = sqrt((Vertecies[i].X - Vertecies[j].X) * (Vertecies[i].X - Vertecies[j].X) + 
							 (Vertecies[i].Y - Vertecies[j].Y) * (Vertecies[i].Y - Vertecies[j].Y));
		float Length2 =	sqrt((Vertecies[k].X - Vertecies[j].X) * (Vertecies[k].X - Vertecies[j].X) +
							 (Vertecies[k].Y - Vertecies[j].Y) * (Vertecies[k].Y - Vertecies[j].Y));
		float Norm = Length1 * Length2;

		if (Norm > 0.0f) {
			Dot /= Norm;
			AngleSum += acos(Dot);
		}
	}
	*/

	return false;
}

// Move
// ----

void BS_Polygon::operator+=(const BS_Vertex &Delta) {
	// Move all vertecies
	for (int i = 0; i < VertexCount; i++)
		Vertecies[i] += Delta;

	// Shift the focus
	m_Centroid += Delta;
}

// Line of Sight
// -------------

bool BS_Polygon::IsLineInterior(const BS_Vertex &a, const BS_Vertex &b) const {
	// Both points have to be in the polygon
	if (!IsPointInPolygon(a, true) || !IsPointInPolygon(b, true)) return false;

	// If the points are identical, the line is trivially within the polygon
	if (a == b) return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < VertexCount; i++) {
		int j = (i + 1) % VertexCount;
		const BS_Vertex &VS = Vertecies[i];
		const BS_Vertex &VE = Vertecies[j];
	
		// If the line intersects a line segment strictly (proper cross section) the line is not in the polygon
		if (BS_Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// then the line is not completely within the polygon
		if (BS_Line::IsOnLineStrict(VS, VE, a) && BS_Line::IsVertexRight(VS, VE, b)) return false;
		if (BS_Line::IsOnLineStrict(VS, VE, b) && BS_Line::IsVertexRight(VS, VE, a)) return false;

		// If one of the two line items is on a vertex, the line traces into the polygon
		if ((a == VS) && !IsLineInCone(i, b, true)) return false;
		if ((b == VS) && !IsLineInCone(i, a, true)) return false;
	}

	return true;
}

bool BS_Polygon::IsLineExterior(const BS_Vertex &a, const BS_Vertex &b) const {
	// Neither of the two points must be strictly in the polygon (on the edge is allowed)
	if (IsPointInPolygon(a, false) || IsPointInPolygon(b, false)) return false;

	// If the points are identical, the line is trivially outside of the polygon
	if (a == b) return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < VertexCount; i++) {
		int j = (i + 1) % VertexCount;
		const BS_Vertex &VS = Vertecies[i];
		const BS_Vertex &VE = Vertecies[j];

		// If the line intersects a line segment strictly (proper intersection), then
		// the line is partially inside the polygon
		if (BS_Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// the line is not completely outside the polygon
		if (BS_Line::IsOnLineStrict(VS, VE, a) && BS_Line::IsVertexLeft(VS, VE, b)) return false;
		if (BS_Line::IsOnLineStrict(VS, VE, b) && BS_Line::IsVertexLeft(VS, VE, a)) return false;

		// If one of the lwo line items is on a vertex, the line must not run into the polygon
		if ((a == VS) && IsLineInCone(i, b, false)) return false;
		if ((b == VS) && IsLineInCone(i, a, false)) return false;

		// If the vertex with start and end point is collinear, (a VS) and (b, VS) is not in the polygon
		if (BS_Line::IsOnLine(a, b, VS)) {
			if (IsLineInCone(i, a, false)) return false;
			if (IsLineInCone(i, b, false)) return false;
		}
	}

	return true;
}

bool BS_Polygon::IsLineInCone(int StartVertexIndex, const BS_Vertex &EndVertex, bool IncludeEdges) const {
	const BS_Vertex &StartVertex = Vertecies[StartVertexIndex];
	const BS_Vertex &NextVertex = Vertecies[(StartVertexIndex + 1) % VertexCount];
	const BS_Vertex &PrevVertex = Vertecies[(StartVertexIndex + VertexCount - 1) % VertexCount];

	if (BS_Line::IsVertexLeftOn(PrevVertex, StartVertex, NextVertex)) {
		if (IncludeEdges)
			return BS_Line::IsVertexLeftOn(EndVertex, StartVertex, NextVertex) &&
			BS_Line::IsVertexLeftOn(StartVertex, EndVertex, PrevVertex);
		else
			return BS_Line::IsVertexLeft(EndVertex, StartVertex, NextVertex) &&
			BS_Line::IsVertexLeft(StartVertex, EndVertex, PrevVertex);
	} else {
		if (IncludeEdges)
			return !(BS_Line::IsVertexLeft(EndVertex, StartVertex, PrevVertex) &&
			BS_Line::IsVertexLeft(StartVertex, EndVertex, NextVertex));
		else
			return !(BS_Line::IsVertexLeftOn(EndVertex, StartVertex, PrevVertex) &&
			BS_Line::IsVertexLeftOn(StartVertex, EndVertex, NextVertex));
	}
}

// Point-Polygon Tests
// -------------------

bool BS_Polygon::IsPointInPolygon(int X, int Y, bool BorderBelongsToPolygon) const {
	return IsPointInPolygon(BS_Vertex(X, Y), BorderBelongsToPolygon);
}

bool BS_Polygon::IsPointInPolygon(const BS_Vertex &Point, bool EdgesBelongToPolygon) const {
	int Rcross = 0; // Number of right-side overlaps
	int Lcross = 0; // Number of left-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	for (int i = 0; i < VertexCount; i++) {
		const BS_Vertex &EdgeStart = Vertecies[i];
		const BS_Vertex &EdgeEnd = Vertecies[(i + 1) % VertexCount];

		// A vertex is a point? Then it lies on one edge of the polygon
		if (Point == EdgeStart) return EdgesBelongToPolygon;

		if ((EdgeStart.Y > Point.Y) != (EdgeEnd.Y > Point.Y)) {
			int Term1 = (EdgeStart.X - Point.X) * (EdgeEnd.Y - Point.Y) - (EdgeEnd.X - Point.X) * (EdgeStart.Y - Point.Y);
			int Term2 = (EdgeEnd.Y - Point.Y) - (EdgeStart.Y - EdgeEnd.Y);
			if ((Term1 > 0) == (Term2 >= 0)) Rcross++;
		}

		if ((EdgeStart.Y < Point.Y) != (EdgeEnd.Y < Point.Y)) { 
			int Term1 = (EdgeStart.X - Point.X) * (EdgeEnd.Y - Point.Y) - (EdgeEnd.X - Point.X) * (EdgeStart.Y - Point.Y);
			int Term2 = (EdgeEnd.Y - Point.Y) - (EdgeStart.Y - EdgeEnd.Y);
			if ((Term1 < 0) == (Term2 <= 0)) Lcross++;
		}
	}	

	// The point is on an adge, if the number of left and right intersections have the same even numbers
	if ((Rcross % 2 ) != (Lcross % 2 )) return EdgesBelongToPolygon;

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	if ((Rcross % 2) == 1) return true;
	else return false;
}

bool BS_Polygon::Persist(BS_OutputPersistenceBlock &Writer) {
	Writer.Write(VertexCount);
	for (int i = 0; i < VertexCount; ++i) {
		Writer.Write(Vertecies[i].X);
		Writer.Write(Vertecies[i].Y);
	}

	return true;
}

bool BS_Polygon::Unpersist(BS_InputPersistenceBlock &Reader) {
	int StoredVertexCount;
	Reader.Read(StoredVertexCount);

	Common::Array<BS_Vertex> StoredVertecies;
	for (int i = 0; i < StoredVertexCount; ++i) {
		int x, y;
		Reader.Read(x);
		Reader.Read(y);
		StoredVertecies.push_back(BS_Vertex(x, y));
	}

	Init(StoredVertexCount, &StoredVertecies[0]);

	return Reader.IsGood();
}

// Main Focus
// ----------

BS_Vertex BS_Polygon::GetCentroid() const {
	return m_Centroid;
}

BS_Vertex BS_Polygon::ComputeCentroid() const {
	// Area of the polygon is calculated
	int DoubleArea = 0;
	for (int i = 0; i < VertexCount; ++i) {
		DoubleArea += Vertecies[i].X * Vertecies[i + 1].Y - Vertecies[i + 1].X * Vertecies[i].Y;
	}

	// Avoid division by zero in the next step
	if (DoubleArea == 0) return BS_Vertex();

	// Calculate centroid
	BS_Vertex Centroid;
	for (int i = 0; i < VertexCount; ++i) {
		int Area = Vertecies[i].X * Vertecies[i + 1].Y - Vertecies[i + 1].X * Vertecies[i].Y;
		Centroid.X += (Vertecies[i].X + Vertecies[i + 1].X) * Area;
		Centroid.Y += (Vertecies[i].Y + Vertecies[i + 1].Y) * Area;
	}
	Centroid.X /= 3 * DoubleArea;
	Centroid.Y /= 3 * DoubleArea;

	return Centroid;
}

} // End of namespace Sword25
