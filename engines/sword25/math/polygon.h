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

#ifndef SWORD25_POLYGON_H
#define SWORD25_POLYGON_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/vertex.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Vertex;

/**
    @brief Eine Polygonklasse.
*/
class BS_Polygon : public BS_Persistable {
public:
	/**
	 * Creates an object of type #BS_Polygon, containing 0 Vertecies.
	 *
	 * With the method Init(), Vertices can be added in later
	 */
	BS_Polygon();

	/**
	 * Copy constructor
	 */
	BS_Polygon(const BS_Polygon &Other);

	/**
	 * Creates a polygon using persisted data
	 */
	BS_Polygon(BS_InputPersistenceBlock &Reader);

	/**
	 * Creaes an object of type #BS_Polygon, and assigns Vertices to it
	 * @param VertexCount       The number of vertices being passed
	 * @param Vertecies         An array of BS_Vertex objects representing the vertices in the polygon.
	 * @remark                  The Vertecies that define a polygon must not have any self-intersections.
	 * If the polygon does have self-intersections, then an empty polygon object is created.
	 */
	BS_Polygon(int VertexCount, const BS_Vertex *Vertecies);

	/**
	 * Deletes the BS_Polygon object
	 */
	virtual ~BS_Polygon();

	/**
	 * Initialises the BS_Polygon with a list of Vertecies.
	 *
	 * The Vertices need to define a polygon must not have self-intersections.
	 * If a polygon already has verticies, this will re-initialise it with the new list.
	 *
	 * @param VertexCount       The number of vertices being passed
	 * @param Vertecies         An array of BS_Vertex objects representing the vertices in the polygon.
	 * @return                  Returns false if the Vertecies have self-intersections. In this case,
	 * the object is not initialised.
	 */
	bool Init(int VertexCount, const BS_Vertex *Vertecies);

	//
	// ** Exploratory methods **
	//

	/**
	 * Checks whether the Vertecies of the polygon are arranged in a clockwise direction.
	 * @return                  Returns true if the Vertecies of the polygon are arranged clockwise or co-planar.
	 * Returns false if the Vertecies of the polygon are arrange counter-clockwise.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool IsCW() const;

	/**
	 * Checks whether the Vertices of the polygon are arranged in a counter-clockwise direction.
	 * @return                  Returns true if the Vertecies of the polygon are arranged counter-clockwise.
	 * Returns false if the Vertecies of the polygon are arranged clockwise or co-planar.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool IsCCW() const;

	/**
	 * Checks whether the polygon is convex.
	 * @return                  Returns true if the polygon is convex. Returns false if the polygon is concave.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool IsConvex() const;

	/**
	 * Checks whether the polygon is concave.
	 * @return                  Returns true if the polygon is concave. Returns false if the polygon is convex.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool IsConcave() const;

	/**
	 * Checks whether a point is inside the polygon
	 * @param Vertex            A Vertex with the co-ordinates of the point to be tested.
	 * @param BorderBelongsToPolygon    Specifies whether the edge of the polygon should be considered
	 * @return                  Returns true if the point is inside the polygon, false if it is outside.
	 */
	bool IsPointInPolygon(const BS_Vertex &Vertex, bool BorderBelongsToPolygon = true) const;

	/**
	 * Checks whether a point is inside the polygon
	 * @param X                 The X position of the point
	 * @param Y                 The Y position of the point
	 * @param BorderBelongsToPolygon    Specifies whether the edge of the polygon should be considered
	 * @return                  Returns true if the point is inside the polygon, false if it is outside.
	 */
	bool IsPointInPolygon(int X, int Y, bool BorderBelongsToPolygon = true) const;

	/**
	 * Returns the focus/centroid of the polygon
	 */
	BS_Vertex GetCentroid() const;

	// Edge belongs to the polygon
	// Polygon must be CW
	bool IsLineInterior(const BS_Vertex &a, const BS_Vertex &b) const;
	// Edge does not belong to the polygon
	// Polygon must be CW
	bool IsLineExterior(const BS_Vertex &a, const BS_Vertex &b) const;

	//
	// Manipulation methods
	//

	/**
	 * Ensures that the Vertecies of the polygon are arranged in a clockwise direction
	 */
	void EnsureCWOrder();

	/**
	 * Ensures that the Vertecies of the polygon are arranged in a counter-clockwise direction
	 */
	void EnsureCCWOrder();

	/**
	 * Reverses the Vertecies order.
	 */
	void ReverseVertexOrder();

	/**
	 * Moves the polygon.
	 * @param Delta             The vertex around the polygon to be moved.
	 */
	void operator+=(const BS_Vertex &Delta);

	//
	//------------------
	//

	/// Specifies the number of Vertecies in the Vertecies array.
	int VertexCount;
	/// COntains the Vertecies of the polygon
	BS_Vertex *Vertecies;

	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

private:
	bool m_IsCW;
	bool m_IsConvex;
	BS_Vertex m_Centroid;

	/**
	 * Computes the centroid of the polygon.
	 */
	BS_Vertex ComputeCentroid() const;

	/**
	 * Determines how the Vertecies of the polygon are arranged.
	 * @return                  Returns true if the Vertecies are arranged in a clockwise
	 * direction, otherwise false.
	 */
	bool ComputeIsCW() const;

	/**
	 * Determines whether the polygon is convex or concave.
	 * @return                  Returns true if the polygon is convex, otherwise false.
	 */
	bool ComputeIsConvex() const;

	/**
	 * Calculates the cross product of three Vertecies
	 * @param V1                The first Vertex
	 * @param V2                The second Vertex
	 * @param V3                The third Vertex
	 * @return                  Returns the cross-product of the three vertecies
	 * @todo                    This method would be better as a method of the BS_Vertex class
	 */
	int CrossProduct(const BS_Vertex &V1, const BS_Vertex &V2, const BS_Vertex &V3) const;

	/**
	 * Computes the scalar product of two vectors spanning three vertecies
	 *
	 * The vectors are spanned by V2->V1 and V2->V3
	 *
	 * @param V1                The first Vertex
	 * @param V2                The second Vertex
	 * @param V3                The third Vertex
	 * @return                  Returns the dot product of the three Vertecies.
	 * @todo                    This method would be better as a method of the BS_Vertex class
	 */
	int DotProduct(const BS_Vertex &V1, const BS_Vertex &V2, const BS_Vertex &V3) const;

	/**
	 * Checks whether the polygon is self-intersecting
	 * @return                  Returns true if the polygon is self-intersecting.
	 * Returns false if the polygon is not self-intersecting.
	 */
	bool CheckForSelfIntersection() const;

	/**
	 * Find the vertex of the polygon that is located below the right-most point,
	 * and returns it's index in the vertex array.
	 * @return                  Returns the index of the vertex at the bottom-right of the polygon.
	 * Returns -1 if the vertex list is empty.
	 */
	int FindLRVertexIndex() const;

	bool IsLineInCone(int StartVertexIndex, const BS_Vertex &EndVertex, bool IncludeEdges) const;
};

} // End of namespace Sword25

#endif
