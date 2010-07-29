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

#include "sword25/kernel/memlog_off.h"
#include <utility>
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include <math.h>

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/math/polygon.h"
#include "sword25/math/line.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))

// Konstruktion / Destruktion
// --------------------------

BS_Polygon::BS_Polygon() : VertexCount(0), Vertecies(NULL)
{
}

BS_Polygon::BS_Polygon(int VertexCount, const BS_Vertex* Vertecies) : VertexCount(0), Vertecies(NULL)
{ 
	Init(VertexCount, Vertecies);
}

BS_Polygon::BS_Polygon(const BS_Polygon& Other) : VertexCount(0), Vertecies(NULL)
{
	Init(Other.VertexCount, Other.Vertecies);
}

BS_Polygon::BS_Polygon(BS_InputPersistenceBlock & Reader) : VertexCount(0), Vertecies(NULL)
{
	Unpersist(Reader);
}

BS_Polygon::~BS_Polygon()
{
	delete[] Vertecies;
}

// Initialisierung
// ---------------

bool BS_Polygon::Init(int VertexCount, const BS_Vertex* Vertecies)
{
	// Alten Objektzustand merken um ihn wieder herstellen zu können, falls beim Initialisieren mit den neuen Daten ein Fehler auftreten
	// sollte.
	int OldVertexCount = this->VertexCount;
	BS_Vertex* OldVertecies = this->Vertecies;

	this->VertexCount = VertexCount;
	this->Vertecies = new BS_Vertex[VertexCount + 1];
	memcpy(this->Vertecies, Vertecies, sizeof(BS_Vertex) * VertexCount);
	// TODO:
	// Doppelte und überflüssige Vertecies entfernen (überflüssig = 3 Verts kollinear)
	// _WeedRepeatedVertecies();
	// Das erste Vertex wird am Ende des Vertex-Arrays wiederholt, dieses vereinfacht einige Algorithmen, die alle Edges durchgehen und
	// sich so die Überlaufkontrolle sparen können.
	this->Vertecies[VertexCount] = this->Vertecies[0];

	// Falls das Polygon selbstüberschneidend ist, wird der alte Objektzustand wieder hergestellt und ein Fehler signalisiert.
	if (CheckForSelfIntersection())
	{
		delete[] this->Vertecies;
		this->Vertecies = OldVertecies;
		this->VertexCount = OldVertexCount;

		// BS_LOG_ERROR("POLYGON: Tried to create a self-intersecting polygon.\n");
		return false;
	}

	// Alte Vertexliste freigeben
	delete[] OldVertecies;

	// Eigenschaften des Polygons berechnen.
	m_IsCW = ComputeIsCW();
	m_IsConvex = ComputeIsConvex();
	m_Centroid = ComputeCentroid();

	return true;
}

// Überprüfung der Reihenfolge der Vertecies
// -----------------------------------------

bool BS_Polygon::IsCW() const
{
	return m_IsCW;
}

bool BS_Polygon::IsCCW() const
{
	return !IsCW();
}

bool BS_Polygon::ComputeIsCW() const
{
	if (VertexCount)
	{
		// Vertex finden, dass am weitesten rechts unten liegt.
		int V2Index = FindLRVertexIndex();

		// Vertex vorher und nachher finden.
		int V1Index = (V2Index + (VertexCount - 1)) % VertexCount;
		int V3Index = (V2Index + 1) % VertexCount;

		// Kreuzprodukt bilden.
		// Wenn das Kreuzprodukt des am weitesten unten links liegenden Vertex positiv ist, sind die Vertecies im Uhrzeigersinn angeordnet
		// ansonsten entgegen des Uhrzeigersinns.
		if (CrossProduct(Vertecies[V1Index], Vertecies[V2Index], Vertecies[V3Index]) >= 0) return true;
	}

	return false;
}

int BS_Polygon::FindLRVertexIndex() const
{
	if (VertexCount)
	{
		int CurIndex = 0;
		int MaxX = Vertecies[0].X;
		int MaxY = Vertecies[0].Y;

		for (int i = 1; i < VertexCount; i++)
		{
			if (Vertecies[i].Y > MaxY ||
			   (Vertecies[i].Y == MaxY && Vertecies[i].X > MaxX))
			{
				MaxX = Vertecies[i].X;
				MaxY = Vertecies[i].Y;
				CurIndex = i;
			}
		}

		return CurIndex;
	}
	
	return -1;
}

// Testen auf Konvex/Konkav
// ------------------------

bool BS_Polygon::IsConvex() const
{
	return m_IsConvex;
}

bool BS_Polygon::IsConcave() const
{
	return !IsConvex();
}

bool BS_Polygon::ComputeIsConvex() const
{
	// Polygone mit 3 oder weniger Vertecies können nur Konvex sein.
	if (VertexCount <= 3) return true;

	// Alle Winkel im Polygon berechnen, wenn das Polygon Konvex ist, müssen alle Winkel das selbe Vorzeichen haben.
	int Flag = 0;
	for (int i = 0; i < VertexCount; i++)
	{
		// Die Indizies der beiden nächsten Vertecies nach i bestimmen.
		int j = (i + 1) % VertexCount;
		int k = (i + 2) % VertexCount;

		// Kreuzprodukt der drei Vertecies berechnen.
		int Cross = CrossProduct(Vertecies[i], Vertecies[j], Vertecies[k]);

		// Die unteren beiden Bits von Flag haben folgende Bedeutung:
		// 0 : negativer Winkel ist aufgetreten
		// 1 : positiver Winkel ist aufgetreten

		// Vorzeichen des aktuellen Winkels in Flag vermerken.
		if (Cross < 0)
			Flag |= 1;
		else if (Cross > 0)
			Flag |= 2;

		// Falls Flag 3 ist, sind sowohl positive als auch negative Winkel vorhanden -> Polygon ist Konkav.
		if (Flag == 3) return false;
	}

	// Polygon ist Konvex.
	return true;
}

// Sicherstellen einer bestimmen Vertexordnung
// -------------------------------------------

void BS_Polygon::EnsureCWOrder()
{
	if (!IsCW())
		ReverseVertexOrder();
}

void BS_Polygon::EnsureCCWOrder()
{
	if (!IsCCW())
		ReverseVertexOrder();
}

// Umkehren der Reihenfolge der Vertecies
// --------------------------------------

void BS_Polygon::ReverseVertexOrder()
{
	// Vertecies paarweise vertauschen, bis die Liste komplett umgekehrt wurde.
	for (int i = 0; i < VertexCount / 2; i++)
		std::swap(Vertecies[i], Vertecies[VertexCount - i - 1]);

	// Vertexordnung neu berechnen.
	m_IsCW = ComputeIsCW();
}

// Kreuzprodukt
// ------------

int BS_Polygon::CrossProduct(const BS_Vertex& V1, const BS_Vertex& V2, const BS_Vertex& V3) const
{
	return (V2.X - V1.X) * (V3.Y - V2.Y) -
		   (V2.Y - V1.Y) * (V3.X - V2.X);
}

// Skalarproduct
// -------------

int BS_Polygon::DotProduct(const BS_Vertex& V1, const BS_Vertex& V2, const BS_Vertex& V3) const
{
	return (V1.X - V2.X) * (V3.X - V2.X) +
		   (V1.Y - V2.Y) * (V3.X - V2.Y);
}

// Überprüfen auf Selbstüberschneidung
// -----------------------------------

bool BS_Polygon::CheckForSelfIntersection() const
{
	// TODO: Fertigstellen
	/*
	float AngleSum = 0.0f;
	for (int i = 0; i < VertexCount; i++)
	{
		int j = (i + 1) % VertexCount;
		int k = (i + 2) % VertexCount;

		float Dot = DotProduct(Vertecies[i], Vertecies[j], Vertecies[k]);

		// Skalarproduct normalisieren
		float Length1 = sqrt((Vertecies[i].X - Vertecies[j].X) * (Vertecies[i].X - Vertecies[j].X) + 
							 (Vertecies[i].Y - Vertecies[j].Y) * (Vertecies[i].Y - Vertecies[j].Y));
		float Length2 =	sqrt((Vertecies[k].X - Vertecies[j].X) * (Vertecies[k].X - Vertecies[j].X) +
							 (Vertecies[k].Y - Vertecies[j].Y) * (Vertecies[k].Y - Vertecies[j].Y));
		float Norm = Length1 * Length2;

		if (Norm > 0.0f)
		{
			Dot /= Norm;
			AngleSum += acos(Dot);
		}
	}
	*/

	return false;
}

// Verschieben
// -----------

void BS_Polygon::operator+=(const BS_Vertex& Delta)
{
	// Alle Vetecies verschieben
	for (int i = 0; i < VertexCount; i++)
		Vertecies[i] += Delta;

	// Den Schwerpunkt verschieben.
	m_Centroid += Delta;
}

// Sichtlinie
// ----------

bool BS_Polygon::IsLineInterior(const BS_Vertex & a, const BS_Vertex & b) const
{
	// Beide Punkte müssen im Polygon sein
	if (!IsPointInPolygon(a, true) || !IsPointInPolygon(b, true)) return false;

	// Falls die Punkte identisch sind, ist die Linie trivialerweise innerhalb des Polygons
	if (a == b) return true;

	// Testen, ob die Linie ein Liniensegment strikt schneidet (proper intersection)
	for (int i = 0; i < VertexCount; i++)
	{
		int j = (i + 1) % VertexCount;
		const BS_Vertex & VS = Vertecies[i];
		const BS_Vertex & VE = Vertecies[j];
	
		// Falls die Linie ein Liniensegment strikt schneidet (proper intersection) ist die Linie nicht innerhalb des Polygons
		if (BS_Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// Falls einer der beiden Linienpunkte auf der Kante liegt und der andere rechts der Kante liegt, befindet sich die Linie nicht
		// vollständig innerhalb des Polygons.
		if (BS_Line::IsOnLineStrict(VS, VE, a) && BS_Line::IsVertexRight(VS, VE, b)) return false;
		if (BS_Line::IsOnLineStrict(VS, VE, b) && BS_Line::IsVertexRight(VS, VE, a)) return false;

		// Falls einer der beiden Linienpunkte auf einem Vertex liegt muss die Linie in das Polygon hinein verlaufen
		if ((a == VS) && !IsLineInCone(i, b, true)) return false;
		if ((b == VS) && !IsLineInCone(i, a, true)) return false;
	}

	return true;
}

bool BS_Polygon::IsLineExterior(const BS_Vertex & a, const BS_Vertex & b) const
{
	// Keiner der beiden Punkte darf strikt im Polygon sein (auf der Kante ist erlaubt)
	if (IsPointInPolygon(a, false) || IsPointInPolygon(b, false)) return false;

	// Falls die Punkte identisch sind, ist die Linie trivialerweise ausserhalb des Polygons
	if (a == b) return true;

	// Testen, ob die Linie ein Liniensegment strikt schneidet (proper intersection)
	for (int i = 0; i < VertexCount; i++)
	{
		int j = (i + 1) % VertexCount;
		const BS_Vertex & VS = Vertecies[i];
		const BS_Vertex & VE = Vertecies[j];

		// Falls die Linie ein Liniensegment strikt schneidet (proper intersection) ist die Linie teilweise innerhalb des Polygons
		if (BS_Line::DoesIntersectProperly(a, b, VS, VE)) return false;

		// Falls einer der beiden Linienpunkte auf der Kante liegt und der andere rechts der Kante liegt, befindet sich die Linie nicht vollständig
		// ausserhalb des Polygons.
		if (BS_Line::IsOnLineStrict(VS, VE, a) && BS_Line::IsVertexLeft(VS, VE, b)) return false;
		if (BS_Line::IsOnLineStrict(VS, VE, b) && BS_Line::IsVertexLeft(VS, VE, a)) return false;

		// Falls einer der beiden Linienpunkte auf einem Vertex liegt, darf die Linie nicht in das Polygon hinein verlaufen
		if ((a == VS) && IsLineInCone(i, b, false)) return false;
		if ((b == VS) && IsLineInCone(i, a, false)) return false;

		// Falls das Vertex mit Start- und Zielpunkt kollinear ist, dürfen die beiden Liniensegmente (a, VS) und (b, VS) nicht in das Polygon hinein
		// verlaufen
		if (BS_Line::IsOnLine(a, b, VS))
		{
			if (IsLineInCone(i, a, false)) return false;
			if (IsLineInCone(i, b, false)) return false;
		}
	}

	return true;
}

bool BS_Polygon::IsLineInCone(int StartVertexIndex, const BS_Vertex & EndVertex, bool IncludeEdges) const
{
	const BS_Vertex & StartVertex = Vertecies[StartVertexIndex];
	const BS_Vertex & NextVertex = Vertecies[(StartVertexIndex + 1) % VertexCount];
	const BS_Vertex & PrevVertex = Vertecies[(StartVertexIndex + VertexCount - 1) % VertexCount];

	if (BS_Line::IsVertexLeftOn(PrevVertex, StartVertex, NextVertex))
	{
		if (IncludeEdges)
			return BS_Line::IsVertexLeftOn(EndVertex, StartVertex, NextVertex) &&
			BS_Line::IsVertexLeftOn(StartVertex, EndVertex, PrevVertex);
		else
			return BS_Line::IsVertexLeft(EndVertex, StartVertex, NextVertex) &&
			BS_Line::IsVertexLeft(StartVertex, EndVertex, PrevVertex);
	}
	else
	{
		if (IncludeEdges)
			return !(BS_Line::IsVertexLeft(EndVertex, StartVertex, PrevVertex) &&
			BS_Line::IsVertexLeft(StartVertex, EndVertex, NextVertex));
		else
			return !(BS_Line::IsVertexLeftOn(EndVertex, StartVertex, PrevVertex) &&
			BS_Line::IsVertexLeftOn(StartVertex, EndVertex, NextVertex));
	}
}

// Punkt-Polygon Tests
// -------------------

bool BS_Polygon::IsPointInPolygon(int X, int Y, bool BorderBelongsToPolygon) const
{
	return IsPointInPolygon(BS_Vertex(X, Y), BorderBelongsToPolygon);
}

bool BS_Polygon::IsPointInPolygon(const BS_Vertex & Point, bool EdgesBelongToPolygon) const
{
	int Rcross = 0; // Anzahl der rechtsseitigen Überschneidungen
	int Lcross = 0; // Anzahl der linksseitigen Überschneidungen

	// Jede Kante wird überprüft ob sie den vom Punkt ausgehenden Strahl schneidet
	for (int i = 0; i < VertexCount; i++)
	{
		const BS_Vertex & EdgeStart = Vertecies[i];
		const BS_Vertex & EdgeEnd = Vertecies[(i + 1) % VertexCount];

		// Ist der Punkt ein Vertex? Dann liegt er auf einer Kante des Polygons
		if (Point == EdgeStart) return EdgesBelongToPolygon;

		if ((EdgeStart.Y > Point.Y) != (EdgeEnd.Y > Point.Y))
		{
			int Term1 = (EdgeStart.X - Point.X) * (EdgeEnd.Y - Point.Y) - (EdgeEnd.X - Point.X) * (EdgeStart.Y - Point.Y);
			int Term2 = (EdgeEnd.Y - Point.Y) - (EdgeStart.Y - EdgeEnd.Y);
			if ((Term1 > 0) == (Term2 >= 0)) Rcross++;
		}

		if ((EdgeStart.Y < Point.Y) != (EdgeEnd.Y < Point.Y))
		{ 
			int Term1 = (EdgeStart.X - Point.X) * (EdgeEnd.Y - Point.Y) - (EdgeEnd.X - Point.X) * (EdgeStart.Y - Point.Y);
			int Term2 = (EdgeEnd.Y - Point.Y) - (EdgeStart.Y - EdgeEnd.Y);
			if ((Term1 < 0) == (Term2 <= 0)) Lcross++;
		}
	}	

	// Der Punkt befindet sich auf einer Kante, wenn die Anzahl der linken und rechten Überschneidungen nicht die gleiche Geradzahligkeit haben
	if ((Rcross % 2 ) != (Lcross % 2 )) return EdgesBelongToPolygon;

	// Der Punkt befindet sich genau dann strikt innerhalb des Polygons, wenn die Anzahl der Überschneidungen ungerade ist
	if ((Rcross % 2) == 1) return true;
	else return false;
}

bool BS_Polygon::Persist(BS_OutputPersistenceBlock & Writer)
{
	Writer.Write(VertexCount);
	for (int i = 0; i < VertexCount; ++i)
	{
		Writer.Write(Vertecies[i].X);
		Writer.Write(Vertecies[i].Y);
	}

	return true;
}

bool BS_Polygon::Unpersist(BS_InputPersistenceBlock & Reader)
{
	int StoredVertexCount;
	Reader.Read(StoredVertexCount);

	std::vector<BS_Vertex> StoredVertecies(StoredVertexCount);
	for (int i = 0; i < StoredVertexCount; ++i)
	{
		Reader.Read(StoredVertecies[i].X);
		Reader.Read(StoredVertecies[i].Y);
	}

	Init(StoredVertexCount, &StoredVertecies[0]);

	return Reader.IsGood();
}

// Schwerpunkt
// -----------

BS_Vertex BS_Polygon::GetCentroid() const
{
	return m_Centroid;
}

BS_Vertex BS_Polygon::ComputeCentroid() const
{
	// Flächeninhalt des Polygons berechnen.
	int DoubleArea = 0;
	for (int i = 0; i < VertexCount; ++i)
	{
		DoubleArea += Vertecies[i].X * Vertecies[i + 1].Y - Vertecies[i + 1].X * Vertecies[i].Y;
	}

	// Division durch 0 beim nächsten Schritt vermeiden.
	if (DoubleArea == 0) return BS_Vertex();

	// Schwerpunkt berechnen.
	BS_Vertex Centroid;
	for (int i = 0; i < VertexCount; ++i)
	{
		int Area = Vertecies[i].X * Vertecies[i + 1].Y - Vertecies[i + 1].X * Vertecies[i].Y;
		Centroid.X += (Vertecies[i].X + Vertecies[i + 1].X) * Area;
		Centroid.Y += (Vertecies[i].Y + Vertecies[i + 1].Y) * Area;
	}
	Centroid.X /= 3 * DoubleArea;
	Centroid.Y /= 3 * DoubleArea;

	return Centroid;
}
