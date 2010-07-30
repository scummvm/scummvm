// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_WALKREGION_H
#define SWORD25_WALKREGION_H

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/math/region.h"

// -----------------------------------------------------------------------------
// Typdefinitionen
// -----------------------------------------------------------------------------

typedef std::vector<BS_Vertex> BS_Path;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

/**
	@brief Diese Klasse stellt die Region dar, in der sich der Hauptcharakter bewegen kann.
*/
class BS_WalkRegion : public BS_Region
{
	friend class BS_Region;

protected:
	BS_WalkRegion();
	BS_WalkRegion(BS_InputPersistenceBlock & Reader, unsigned int Handle);

public:
	virtual ~BS_WalkRegion();

	virtual bool Init(const BS_Polygon & Contour, const std::vector<BS_Polygon> * pHoles = 0);

	/**
		@brief Ermittelt den kürzesten Weg zwischen zwei Punkten in der Region.

		Diese Methode verlangt, dass der Startpunkt innerhalb der Region liegt. Der Endpunkt darf außerhalb der Region liegen. In diesem
		Fall wählt die Methode als Endpunkt den Punkt innerhalb der Region, der am dichtesten am Endpunkt liegt.

		@param X1 X-Koordinate des Startpunktes
		@param Y1 Y-Koordinate des Startpunktes
		@param X2 X-Koordinate des Zielpunktes
		@param Y2 Y-Koordinate des Zielpunktes
		@param Path ein leerer BS_Path, der den Ergebnispfad aufnehmen soll
		@return Gibt false zurück, fall die Eingaben ungültig waren, ansonsten wird true zurückgegeben.
	 */
	bool QueryPath(int X1, int Y1, int X2, int Y2, BS_Path & Path) { return QueryPath(BS_Vertex(X1, Y1), BS_Vertex(X2, Y2), Path); }

	/**
		@brief Ermittelt den kürzesten Weg zwischen zwei Punkten in der Region.

		Diese Methode verlangt, dass der Startpunkt innerhalb der Region liegt. Der Endpunkt darf außerhalb der Region liegen. In diesem
		Fall wählt die Methode als Endpunkt den Punkt innerhalb der Region, der am dichtesten am Endpunkt liegt.
		
		@param StartPoint der Startpunkt
		@param EndPoint der Endpunkt
		@param Path ein leerer BS_Path, der den Ergebnispfad aufnehmen soll
		@return Gibt false zurück, fall die Eingaben ungültig waren, ansonsten wird true zurückgegeben.
	*/
	bool QueryPath(BS_Vertex StartPoint, BS_Vertex EndPoint, BS_Path & Path);

	virtual void SetPos(int X, int Y);

	const std::vector<BS_Vertex> & GetNodes() const { return m_Nodes; }
	const std::vector< std::vector<int> > & GetVisibilityMatrix() const { return m_VisibilityMatrix; }

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	std::vector<BS_Vertex> m_Nodes;
	std::vector< std::vector<int> > m_VisibilityMatrix;

	void InitNodeVector();
	void ComputeVisibilityMatrix();
	bool CheckAndPrepareStartAndEnd(BS_Vertex & Start, BS_Vertex & End) const;
	bool FindPath(const BS_Vertex & Start, const BS_Vertex & End, BS_Path & Path) const;
};

#endif
