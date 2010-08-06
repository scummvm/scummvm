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

#ifndef SWORD25_WALKREGION_H
#define SWORD25_WALKREGION_H

#include "common/array.h"
#include "sword25/kernel/common.h"
#include "sword25/math/region.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Type definitions
// -----------------------------------------------------------------------------

typedef Common::Array<BS_Vertex> BS_Path;

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

/**
 * This class represents the region in which the main character can move
 */
class BS_WalkRegion : public BS_Region {
	friend class BS_Region;

protected:
	BS_WalkRegion();
	BS_WalkRegion(BS_InputPersistenceBlock &Reader, unsigned int Handle);

public:
	virtual ~BS_WalkRegion();

	virtual bool Init(const BS_Polygon &Contour, const Common::Array<BS_Polygon> *pHoles = 0);

	/**
	 * Get the shortest path between two points in the region
	 *
	 * This method requires that the starting point lies within the region. The end point
	 * may lie outside the region. Int his case, the end is chosen as the cloest point to it
	 * that lies within the region.
	 *
	 * @param X1            X Co-ordinate of the start point
	 * @param Y1            Y Co-ordinate of the start point
	 * @param X2            X Co-ordinate of the end point
	 * @param Y2            Y Co-ordinate of the end point
	 * @param Path          An empty BS_Path that will be set to the resulting path
	 * @return              Returns false if the result is invalid, otherwise returns true.
	 */
	bool QueryPath(int X1, int Y1, int X2, int Y2, BS_Path &Path) {
		return QueryPath(BS_Vertex(X1, Y1), BS_Vertex(X2, Y2), Path);
	}

	/**
	 * Get the shortest path between two points in the region.
	 *
	 * @param StartPoint    The start point
	 * @param EndPoint      The end point
	 * @param Path          An empty BS_Path that will be set to the resulting path
	 * @return              Returns false if the result is invalid, otherwise returns true.
	*/
	bool QueryPath(BS_Vertex StartPoint, BS_Vertex EndPoint, BS_Path &Path);

	virtual void SetPos(int X, int Y);

	const Common::Array<BS_Vertex> &GetNodes() const {
		return m_Nodes;
	}
	const Common::Array< Common::Array<int> > &GetVisibilityMatrix() const {
		return m_VisibilityMatrix;
	}

	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

private:
	Common::Array<BS_Vertex> m_Nodes;
	Common::Array< Common::Array<int> > m_VisibilityMatrix;

	void InitNodeVector();
	void ComputeVisibilityMatrix();
	bool CheckAndPrepareStartAndEnd(BS_Vertex &Start, BS_Vertex &End) const;
	bool FindPath(const BS_Vertex &Start, const BS_Vertex &End, BS_Path &Path) const;
};

} // End of namespace Sword25

#endif
