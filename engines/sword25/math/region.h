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

#ifndef SWORD25_REGION_H
#define SWORD25_REGION_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/vertex.h"
#include "sword25/math/polygon.h"
#include "sword25/math/rect.h"

namespace Sword25 {

/**
 * This class is the base class of all regions.
 *
 * The IsValid() method can be queried to see whether the object is in a valid state.
 * If this is not the case, the method Init() is the only method that may be invoked.
 * This class guarantees that the Vertecies outline of the hole, and the polygons are
 * arranged in a clockwise direction, so that the polygon working algorithms will
 * work properly.
 */
class Region : public Persistable {
protected:
	/**
	 * Creates a new BS_Region object
	 *
	 * After creation the object is invaild (IsValid() return false), but a call can
	 * be made later on to Init() to set up the region into a valid state.
	 */
	Region();

	Region(InputPersistenceBlock &Reader, unsigned int Handle);

public:
	enum REGION_TYPE {
		RT_REGION,
		RT_WALKREGION
	};

	static unsigned int Create(REGION_TYPE Type);
	static unsigned int Create(InputPersistenceBlock &Reader, unsigned int Handle = 0);

	virtual ~Region();

	/**
	 * Initialises a BS_Region object
	 * @param Contour       A polygon indicating the outline of the region
	 * @param pHoles        A pointer to an array of polygons representing the hole state in the region.
	 * If the region has no holes, it must be passed as NULL. The default value is NULL.
	 * @return              Returns true if the initialisation was successful, otherwise false.
	 * @remark              If the region was already initialised, the old state will be deleted.
	 */
	virtual bool Init(const Polygon &Contour, const Common::Array<Polygon> *pHoles = NULL);

	//
	// Exploratory Methods
	//

	/**
	 * Specifies whether the object is in a valid state
	 * @return              Returns true if the object is in a valid state, otherwise false.
	 * @remark              Invalid objects can be made valid by calling Init with a valid state.
	 */
	bool IsValid() const {
		return m_Valid;
	}

	/**
	 * Returns the position of the region
	 */
	const Vertex &GetPosition() const {
		return m_Position;
	}

	/**
	 * Returns the X position of the region
	 */
	int GetPosX() const {
		return m_Position.X;
	}

	/**
	 * Returns the Y position of the region
	 */
	int GetPosY() const {
		return m_Position.Y;
	}

	/**
	 * Indicates whether a point is inside the region
	 * @param Vertex        A verex with the co-ordinates of the test point
	 * @return              Returns true if the point is within the region, otherwise false.
	 */
	bool IsPointInRegion(const Vertex &Vertex) const;

	/**
	 * Indicates whether a point is inside the region
	 * @param X             The X position
	 * @param Y             The Y position
	 * @return              Returns true if the point is within the region, otherwise false.
	 */
	bool IsPointInRegion(int X, int Y) const;

	/**
	 * Returns the countour of the region
	 */
	const Polygon &GetContour() const {
		return m_Polygons[0];
	}

	/**
	 * Returns the number of polygons in the hole region
	 */
	int GetHoleCount() const {
		return static_cast<int>(m_Polygons.size() - 1);
	}

	/**
	 * Returns a specific hole polygon in the region
	 * @param i             The number of the hole to return.
	 * The index must be between 0 and GetHoleCount() - 1.
	 * @return              Returns the desired hole polygon
	 */
	inline const Polygon &GetHole(unsigned int i) const;

	/**
	 * For a point outside the region, finds the closest point inside the region
	 * @param Point         The point that is outside the region
	 * @return              Returns the point within the region which is closest
	 * @remark              This method does not always work with pixel accuracy.
	 * One should not therefore rely on the fact that there is really no point in
	 * the region which is closer to the given point.
	 */
	Vertex FindClosestRegionPoint(const Vertex &Point) const;

	/**
	 * Returns the centroid for the region
	 */
	Vertex GetCentroid() const;

	bool IsLineOfSight(const Vertex &a, const Vertex &b) const;

	//
	// Manipulation Methods
	//

	/**
	 * Sets the position of the region
	 * @param X             The new X psoition of the region
	 * @param Y             The new Y psoition of the region
	 */
	virtual void SetPos(int X, int Y);

	/**
	 * Sets the X position of the region
	 * @param X             The new X position of the region
	 */
	void SetPosX(int X);

	/**
	 * Sets the Y position of the region
	 * @param Y             The new Y position of the region
	 */
	void SetPosY(int Y);

	//
	// Manipulation Methods
	//

	virtual bool Persist(OutputPersistenceBlock &Writer);
	virtual bool Unpersist(InputPersistenceBlock &Reader);

protected:
	/// This specifies the type of object
	REGION_TYPE m_Type;
	/// This variable indicates whether the current object state is valid
	bool m_Valid;
	/// This vertex is the position of the region
	Vertex m_Position;
	/// This array contains all the polygons that define the region. The first element of
	// the array is the contour, all others are the holes
	Common::Array<Polygon> m_Polygons;
	/// The bounding box for the region
	BS_Rect m_BoundingBox;

	/**
	 * Updates the bounding box of the region.
	 */
	void UpdateBoundingBox();

	/**
	 * Find the point on a line which is closest to another point
	 * @param LineStart     The start of the line
	 * @param LineEnd       The end of the line
	 * @param Point         The point to be compared against
	 * @return              Returns the point on the line which is cloest to the passed point.
	 */
	Vertex FindClosestPointOnLine(const Vertex &LineStart, const Vertex &LineEnd, const Vertex Point) const;
};


// -----------------------------------------------------------------------------
// Inlines
// -----------------------------------------------------------------------------

inline const Polygon &Region::GetHole(unsigned int i) const {
	BS_ASSERT(i < m_Polygons.size() - 1);
	return m_Polygons[i + 1];
}

} // End of namespace Sword25

#endif
