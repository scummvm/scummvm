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

#ifndef BS_REGION_H
#define BS_REGION_H

#include "kernel/memlog_off.h"
#include <vector>
#include "kernel/memlog_on.h"

#include "kernel/common.h"
#include "kernel/persistable.h"
#include "vertex.h"
#include "polygon.h"
#include "rect.h"

/**
	@brief Diese Klasse ist die Basisklasse aller Regionen.

	Mit der Methode IsValid() lässt sich abfragen, ob sich das Objekt in einem gültigen Zustand befindet.<br>
	Sollte dies nicht der Fall sein, ist die Methode Init() die einzige Methode die aufgerufen werden darf.
	Diese Klasse garantiert, dass die Vertecies der die Umriss- und die Lochpolygone im Uhrzeigersinn angeordnet sind, so dass auf den Polygonen
	arbeitende Algorithmen nur für diese Anordnung implementiert werden müssen.
*/
class BS_Region : public BS_Persistable
{
protected:
	/**
		@brief Erzeugt ein uninitialisiertes #BS_Region Objekt.

		Nach dem Erzeugen ist das Objekt noch ungültig (IsValid() gibt false zurück), allerdings kann das Objekt nachträglich über
		einen Aufruf von Init() in einen gültigen Zustand versetzt werden.
	*/
	BS_Region();

	BS_Region(BS_InputPersistenceBlock & Reader, unsigned int Handle);

public:
	enum REGION_TYPE
	{
		RT_REGION,
		RT_WALKREGION,
	};

	static unsigned int Create(REGION_TYPE Type);
	static unsigned int Create(BS_InputPersistenceBlock & Reader, unsigned int Handle = 0);

	virtual ~BS_Region();

	/**
		@brief Initialisiert ein BS_Region Objekt.
		@param Contour ein Polygon das den Umriss der Region angibt.
		@param pHoles ein Pointer auf einen Vector von Polygonen, die Löcher in der Region angeben.<br>
					  Falls die Region keine Löcher hat, muss NULL übergeben werden.<br>
					  Der Standardwert ist NULL.
		@return Gibt true zurück, wenn die Initialisierung erfolgreich war.<br>
				Gibt false zurück, wenn die Intialisierung fehlgeschlagen ist.
		@remark Falls die Region bereits initialisiert war, wird der alte Zustand gelöscht.
	*/
	virtual bool Init(const BS_Polygon& Contour, const std::vector<BS_Polygon>* pHoles = NULL);
	
	//@{
	/** @name Sondierende Methoden */

	/**
		@brief Gibt an, ob das Objekt in einem gültigen Zustand ist.
		@return Gibt true zurück, wenn sich das Objekt in einem gültigen Zustand befindet.
				Gibt false zurück, wenn sich das Objekt in einem ungültigen Zustand befindet.
		@remark Ungültige Objekte können durch einen Aufruf von Init() in einen gültigen Zustand versetzt werden.
	*/
	bool IsValid() const { return m_Valid; }

	/**
		@brief Gibt die Position der Region zurück.
	*/
	const BS_Vertex& GetPosition() const { return m_Position; }

	/**
		@brief Gibt die Position des Region auf der X-Achse zurück.
	*/
	int GetPosX() const { return m_Position.X; }

	/**
		@brief Gibt die Position des Region auf der Y-Achse zurück.
	*/
	int GetPosY() const { return m_Position.Y; }

	/**
		@brief Gibt an, ob sich ein Punkt innerhalb der Region befindet.
		@param Vertex ein Vertex, mit den Koordinaten des zu testenden Punktes.
		@return Gibt true zurück, wenn sich der Punkt innerhalb der Region befindet.<br>
				Gibt false zurück, wenn sich der Punkt außerhalb der Region befindet.
	*/
	bool IsPointInRegion(const BS_Vertex& Vertex) const;

	/**
		@brief Gibt an, ob sich ein Punkt innerhalb der Region befindet.
		@param X die Position des Punktes auf der X-Achse.
		@param Y die Position des Punktes auf der Y-Achse.
		@return Gibt true zurück, wenn sich der Punkt innerhalb der Region befindet.<br>
				Gibt false zurück, wenn sich der Punkt außerhalb der Region befindet.
	*/
	bool IsPointInRegion(int X, int Y) const;

	/**
		@brief Gibt das Umrisspolygon der Region zurück.
	*/
	const BS_Polygon& GetContour() const { return m_Polygons[0]; }

	/**
		@brief Gibt die Anzahl der Lochpolygone in der Region zurück.
	*/
	int GetHoleCount() const { return static_cast<int>(m_Polygons.size() - 1); }

	/**
		@brief Gibt ein bestimmtes Lochpolygon in der Region zurück.
		@param i die Nummer des zurückzugebenen Loches.<br>
				 Dieser Wert muss zwischen 0 und GetHoleCount() - 1 liegen.
		@return Gibt das gewünschte Lochpolygon zurück.
	*/
	inline const BS_Polygon& GetHole(unsigned int i) const;

	/**
		@brief Findet für einen Punkt ausserhalb der Region den nächsten Punkt, der sich innerhalb der Region befindet.
		@param Point der Punkt, der sich ausserhalb der Region befindet
		@return Gibt den Punkt innerhalb der Region zurück, der den geringsten Abstand zum übergebenen Punkt hat.
		@remark Diese Methode arbeitet nicht immer Pixelgenau. Man sollte sich also nicht darauf verlassen, dass es wirklich keine Punkt innerhalb der
				Region gibt, der dichter am übergebenen Punkt liegt.
	*/
	BS_Vertex FindClosestRegionPoint(const BS_Vertex& Point) const;

	/**
		@brief Gibt den Schwerpunkt des Umrisspolygons zurück.
	*/
	BS_Vertex GetCentroid() const;

	bool IsLineOfSight(const BS_Vertex & a, const BS_Vertex & b) const;

	//@}

	//@{
	/** @name Manipulierende Methoden */

	/**
		@brief Setzt die Position der Region.
		@param X die neue Position der Region auf der X-Achse.
		@param Y die neue Position der Region auf der Y-Achse.
	*/
	virtual void SetPos(int X, int Y);

	/**
		@brief Setzt die Position der Region auf der X-Achse.
		@param X die neue Position der Region auf der X-Achse.
	*/
	void SetPosX(int X);

	/**
		@brief Setzt die Position der Region auf der Y-Achse.
		@param Y die neue Position der Region auf der Y-Achse.
	*/
	void SetPosY(int Y);

	//@}

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	/// Diese Variable gibt den Typ des Objektes an.
	REGION_TYPE m_Type;
	/// Diese Variable gibt an, ob der aktuelle Objektzustand gültig ist.
	bool m_Valid;
	/// Dieses Vertex gibt die Position der Region an.
	BS_Vertex m_Position;
	/// Dieser Vector enthält alle Polygone die die Region definieren. Das erste Element des Vectors ist die Kontur, alle weiteren sind die Löcher.
	std::vector<BS_Polygon> m_Polygons;
	/// Die Bounding-Box der Region.
	BS_Rect m_BoundingBox;

	/**
		@brief Aktualisiert die Bounding-Box der Region.
	*/
	void UpdateBoundingBox();

	/**
		@brief Findet den Punkt auf einer Linie, der einem anderen Punkt am nächsten ist.
		@param LineStart der Startpunkt der Linie
		@param LineEnd der Endpunkt der Linie
		@param Point der Punkt, zu dem der nächste Punkt auf der Linie konstruiert werden soll.
		@return Gibt den Punkt auf der Linie zurück, der dem übergebenen Punkt am nächsten ist.
	*/
	BS_Vertex FindClosestPointOnLine(const BS_Vertex & LineStart, const BS_Vertex & LineEnd, const BS_Vertex Point) const;
};


// -----------------------------------------------------------------------------
// Inlines
// -----------------------------------------------------------------------------

inline const BS_Polygon& BS_Region::GetHole(unsigned int i) const
{
	BS_ASSERT(i < m_Polygons.size() - 1);
	return m_Polygons[i + 1];
}

#endif
