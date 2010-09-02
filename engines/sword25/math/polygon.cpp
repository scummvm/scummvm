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

Polygon::Polygon() : VertexCount(0), Vertecies(NULL) {
}

Polygon::Polygon(int VertexCount_, const Vertex *Vertecies_) : VertexCount(0), Vertecies(NULL) {
	Init(VertexCount_, Vertecies_);
}

Polygon::Polygon(const Polygon &Other) : VertexCount(0), Vertecies(NULL) {
	Init(Other.VertexCount, Other.Vertecies);
}

Polygon::Polygon(InputPersistenceBlock &Reader) : VertexCount(0), Vertecies(NULL) {
	unpersist(Reader);
}

Polygon::~Polygon() {
	delete[] Vertecies;
}

// Initialisation
// ---------------

bool Polygon::Init(int VertexCount_, const Vertex *Vertecies_) {
	// Rember the old obstate to restore it if an error occurs whilst initialising it with the new data
	int OldVertexCount = this->VertexCount;
	Vertex *OldVertecies = this->Vertecies;

	this->VertexCount = VertexCount_;
	this->Vertecies = new Vertex[VertexCount_ + 1];
	memcpy(this->Vertecies, Vertecies_, sizeof(Vertex) * VertexCount_);
	// TODO:
	// Duplicate and remove redundant vertecies (Superflous = 3 co-linear verts)
	// _WeedRepeatedVertecies();
	// The first vertex is repeated at the end of the vertex array; this simplifies
	// some algorithms, running through the edges and thus can save the overflow control.
	this->Vertecies[VertexCount_] = this->Vertecies[0];

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

bool Polygon::IsCW() const {
	return m_IsCW;
}

bool Polygon::IsCCW() const {
	return !IsCW();
}

bool Polygon::ComputeIsCW() const {
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

int Polygon::FindLRVertexIndex() const {
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

bool Polygon::IsConvex() const {
	return m_IsConvex;
}

bool Polygon::IsConcave() const {
	return !IsConvex();
}

bool Polygon::ComputeIsConvex() const {
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

void Polygon::EnsureCWOrder() {
	if (!IsCW())
		ReverseVertexOrder();
}

void Polygon::EnsureCCWOrder() {
	if (!IsCCW())
		ReverseVertexOrder();
}

// Reverse the order of vertecies
// ------------------------------

void Polygon::ReverseVertexOrder() {
	// Vertecies are exchanged in pairs, until the list has been completely reversed
	for (int i = 0; i < VertexCount / 2; i++) {
		Vertex tempVertex = Vertecies[i];
		Vertecies[i] = Vertecies[VertexCount - i - 1];
		Vertecies[VertexCount - i - 1] = tempVertex;
	}

	// Vertexordnung neu berechnen.
	m_IsCW = ComputeIsCW();
}

// Cross Product
// -------------

int Polygon::CrossProduct(const Vertex &V1, const Vertex &V2, const Vertex &V3) const {
	return (V2.X - V1.X) * (V3.Y - V2.Y) -
	       (V2.Y - V1.Y) * (V3.X - V2.X);
}

// Scalar Product
// --------------

int Polygon::DotProduct(const Vertex &V1, const Vertex &V2, const Vertex &V3) const {
	return (V1.X - V2.X) * (V3.X - V2.X) +
	       (V1.Y - V2.Y) * (V3.X - V2.Y);
}

// Check for self-intersections
// ----------------------------

bool Polygon::CheckForSelfIntersection() const {
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
	    float Length2 = sqrt((Vertecies[k].X - Vertecies[j].X) * (Vertecies[k].X - Vertecies[j].X) +
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

void Polygon::operator+=(const Vertex &Delta) {
	// Move all vertecies
	for (int i = 0; i < VertexCount; i++)
		Vertecies[i] += Delta;

	// Shift the focus
	m_Centroid += Delta;
}

// Line of Sight
// -------------

bool Polygon::IsLineInterior(const Vertex &a, const Vertex &b) const {
	// Both points have to be in the polygon
	if (!IsPointInPolygon(a, true) || !IsPointInPolygon(b, true)) return false;

	// If the points are identical, the line is trivially within the polygon
	if (a == b) return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < VertexCount; i++) {
		int j = (i + 1) % VertexCount;
		const Vertex &VS = Vertecies[i];
		const Vertex &VE = Vertecies[j];

		// If the line intersects a line segment strictly (proper cross section) the line is not in the polygon
		if (Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// then the line is not completely within the polygon
		if (Line::IsOnLineStrict(VS, VE, a) && Line::IsVertexRight(VS, VE, b)) return false;
		if (Line::IsOnLineStrict(VS, VE, b) && Line::IsVertexRight(VS, VE, a)) return false;

		// If one of the two line items is on a vertex, the line traces into the polygon
		if ((a == VS) && !IsLineInCone(i, b, true)) return false;
		if ((b == VS) && !IsLineInCone(i, a, true)) return false;
	}

	return true;
}

bool Polygon::IsLineExterior(const Vertex &a, const Vertex &b) const {
	// Neither of the two points must be strictly in the polygon (on the edge is allowed)
	if (IsPointInPolygon(a, false) || IsPointInPolygon(b, false)) return false;

	// If the points are identical, the line is trivially outside of the polygon
	if (a == b) return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < VertexCount; i++) {
		int j = (i + 1) % VertexCount;
		const Vertex &VS = Vertecies[i];
		const Vertex &VE = Vertecies[j];

		// If the line intersects a line segment strictly (proper intersection), then
		// the line is partially inside the polygon
		if (Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// the line is not completely outside the polygon
		if (Line::IsOnLineStrict(VS, VE, a) && Line::IsVertexLeft(VS, VE, b)) return false;
		if (Line::IsOnLineStrict(VS, VE, b) && Line::IsVertexLeft(VS, VE, a)) return false;

		// If one of the lwo line items is on a vertex, the line must not run into the polygon
		if ((a == VS) && IsLineInCone(i, b, false)) return false;
		if ((b == VS) && IsLineInCone(i, a, false)) return false;

		// If the vertex with start and end point is collinear, (a VS) and (b, VS) is not in the polygon
		if (Line::IsOnLine(a, b, VS)) {
			if (IsLineInCone(i, a, false)) return false;
			if (IsLineInCone(i, b, false)) return false;
		}
	}

	return true;
}

bool Polygon::IsLineInCone(int StartVertexIndex, const Vertex &EndVertex, bool IncludeEdges) const {
	const Vertex &StartVertex = Vertecies[StartVertexIndex];
	const Vertex &NextVertex = Vertecies[(StartVertexIndex + 1) % VertexCount];
	const Vertex &PrevVertex = Vertecies[(StartVertexIndex + VertexCount - 1) % VertexCount];

	if (Line::IsVertexLeftOn(PrevVertex, StartVertex, NextVertex)) {
		if (IncludeEdges)
			return Line::IsVertexLeftOn(EndVertex, StartVertex, NextVertex) &&
			       Line::IsVertexLeftOn(StartVertex, EndVertex, PrevVertex);
		else
			return Line::IsVertexLeft(EndVertex, StartVertex, NextVertex) &&
			       Line::IsVertexLeft(StartVertex, EndVertex, PrevVertex);
	} else {
		if (IncludeEdges)
			return !(Line::IsVertexLeft(EndVertex, StartVertex, PrevVertex) &&
			         Line::IsVertexLeft(StartVertex, EndVertex, NextVertex));
		else
			return !(Line::IsVertexLeftOn(EndVertex, StartVertex, PrevVertex) &&
			         Line::IsVertexLeftOn(StartVertex, EndVertex, NextVertex));
	}
}

// Point-Polygon Tests
// -------------------

bool Polygon::IsPointInPolygon(int X, int Y, bool BorderBelongsToPolygon) const {
	return IsPointInPolygon(Vertex(X, Y), BorderBelongsToPolygon);
}

bool Polygon::IsPointInPolygon(const Vertex &Point, bool EdgesBelongToPolygon) const {
	int Rcross = 0; // Number of right-side overlaps
	int Lcross = 0; // Number of left-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	for (int i = 0; i < VertexCount; i++) {
		const Vertex &EdgeStart = Vertecies[i];
		const Vertex &EdgeEnd = Vertecies[(i + 1) % VertexCount];

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
	if ((Rcross % 2) != (Lcross % 2)) return EdgesBelongToPolygon;

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	if ((Rcross % 2) == 1) return true;
	else return false;
}

bool Polygon::persist(OutputPersistenceBlock &writer) {
	writer.write(VertexCount);
	for (int i = 0; i < VertexCount; ++i) {
		writer.write(Vertecies[i].X);
		writer.write(Vertecies[i].Y);
	}

	return true;
}

bool Polygon::unpersist(InputPersistenceBlock &reader) {
	int StoredVertexCount;
	reader.read(StoredVertexCount);

	Common::Array<Vertex> StoredVertecies;
	for (int i = 0; i < StoredVertexCount; ++i) {
		int x, y;
		reader.read(x);
		reader.read(y);
		StoredVertecies.push_back(Vertex(x, y));
	}

	Init(StoredVertexCount, &StoredVertecies[0]);

	return reader.isGood();
}

// Main Focus
// ----------

Vertex Polygon::GetCentroid() const {
	return m_Centroid;
}

Vertex Polygon::ComputeCentroid() const {
	// Area of the polygon is calculated
	int DoubleArea = 0;
	for (int i = 0; i < VertexCount; ++i) {
		DoubleArea += Vertecies[i].X * Vertecies[i + 1].Y - Vertecies[i + 1].X * Vertecies[i].Y;
	}

	// Avoid division by zero in the next step
	if (DoubleArea == 0) return Vertex();

	// Calculate centroid
	Vertex Centroid;
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
