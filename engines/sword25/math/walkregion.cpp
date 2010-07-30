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

#include <list>
#include <algorithm>
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/line.h"

#define BS_LOG_PREFIX "WALKREGION"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

static const int infinity = INT_MAX;

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_WalkRegion::BS_WalkRegion()
{
	m_Type = RT_WALKREGION;
}

// -----------------------------------------------------------------------------

BS_WalkRegion::BS_WalkRegion(BS_InputPersistenceBlock &Reader, unsigned int Handle) :
	BS_Region(Reader, Handle)
{
	m_Type = RT_WALKREGION;
	Unpersist(Reader);
}

// -----------------------------------------------------------------------------

BS_WalkRegion::~BS_WalkRegion()
{
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Init(const BS_Polygon & Contour, const std::vector<BS_Polygon> * pHoles)
{
	// Standard-Initialisierungen der Region vornehmen.
	if (!BS_Region::Init(Contour, pHoles)) return false;

	// Datenstrukturen fürs Pathfinding vorbereiten
	InitNodeVector();
	ComputeVisibilityMatrix();

	// Erfolg signalisieren.
	return true;
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::QueryPath(BS_Vertex StartPoint, BS_Vertex EndPoint, BS_Path & Path)
{
	BS_ASSERT(Path.empty());

	// Falls Start und Ziel identisch sind, muss trivialerweise kein Pfad gefunden werden.
	if (StartPoint == EndPoint) return true;

	// Sicherstellen, dass Start und Ziel gültig sind und neuen Start- und Zielpunkt finden, falls sie ausserhalb des Polygons liegen.
	if (!CheckAndPrepareStartAndEnd(StartPoint, EndPoint)) return false;

	// Wenn zwischen Start- und Endpunkt eine Sichtlinie besteht, muss kein Pathfindung durchgeführt werden und als Ergebnis wird die
	// direkte Verbindungslinie zwischen Start- und Endpunkt zurückgegeben.
	if (IsLineOfSight(StartPoint, EndPoint))
	{
		Path.push_back(StartPoint);
		Path.push_back(EndPoint);
		return true;
	}

	return FindPath(StartPoint, EndPoint, Path);
}

// -----------------------------------------------------------------------------

struct DijkstraNode
{
	typedef std::vector<DijkstraNode> Container;
	typedef Container::iterator Iter;
	typedef Container::const_iterator ConstIter;

	DijkstraNode() : Cost(infinity), Chosen(false) {};
	ConstIter	ParentIter;
	int			Cost;
	bool		Chosen;
};

static void InitDijkstraNodes(DijkstraNode::Container & DijkstraNodes, const BS_Region & Region, const BS_Vertex & Start, const std::vector<BS_Vertex> & Nodes)
{
	// Ausreichend Platz im Vector reservieren
	DijkstraNodes.resize(Nodes.size());

	// Alle Randknoten initialisieren, die vom Startknoten sichtbar sind
	DijkstraNode::Iter DijkstraIter = DijkstraNodes.begin();
	for (std::vector<BS_Vertex>::const_iterator NodesIter = Nodes.begin(); NodesIter != Nodes.end(); NodesIter++, DijkstraIter++)
	{
		(*DijkstraIter).ParentIter = DijkstraNodes.end();
		if (Region.IsLineOfSight(*NodesIter, Start)) (*DijkstraIter).Cost = (*NodesIter).Distance(Start);
	}
	BS_ASSERT(DijkstraIter == DijkstraNodes.end());
}

static DijkstraNode::Iter ChooseClosestNode(DijkstraNode::Container & Nodes)
{
	DijkstraNode::Iter ClosestNodeInter = Nodes.end();
	int MinCost = infinity;

	for (DijkstraNode::Iter iter = Nodes.begin(); iter != Nodes.end(); iter++)
	{
		if (!(*iter).Chosen && (*iter).Cost < MinCost)
		{
			MinCost = (*iter).Cost;
			ClosestNodeInter = iter;
		}
	}

	return ClosestNodeInter;
}

static void RelaxNodes(DijkstraNode::Container & Nodes,
					   const std::vector< std::vector<int> > & VisibilityMatrix, 
					   const DijkstraNode::ConstIter & CurNodeIter)
{
	// Alle Nachfolger vom aktuellen Knoten, die noch nicht gewählt wurden, werden in die Randknotenliste eingefügt und die Kosten werden
	// aktualisiert, wenn ein kürzerer Pfad zu ihnen gefunden wurde.

	int CurNodeIndex = CurNodeIter - Nodes.begin();
	for (unsigned int i = 0; i < Nodes.size(); i++)
	{
		int Cost = VisibilityMatrix[CurNodeIndex][i];
		if (!Nodes[i].Chosen && Cost != infinity)
		{
			int TotalCost = (*CurNodeIter).Cost + Cost;
			if (TotalCost < Nodes[i].Cost)
			{
				Nodes[i].ParentIter = CurNodeIter;
				Nodes[i].Cost = TotalCost;
			}
		}
	}
}

static void RelaxEndPoint(const BS_Vertex & CurNodePos,
						  const DijkstraNode::ConstIter & CurNodeIter,
						  const BS_Vertex & EndPointPos,
						  DijkstraNode & EndPoint,
						  const BS_Region & Region)
{
	if (Region.IsLineOfSight(CurNodePos, EndPointPos))
	{
		int TotalCost = (*CurNodeIter).Cost + CurNodePos.Distance(EndPointPos);
		if (TotalCost < EndPoint.Cost)
		{
			EndPoint.ParentIter = CurNodeIter;
			EndPoint.Cost = TotalCost;
		}
	}
}

bool BS_WalkRegion::FindPath(const BS_Vertex & Start, const BS_Vertex & End, BS_Path & Path) const
{
	// Dies ist eine Implementation des Dijkstra-Algorithmus

	// Randknotenliste initialisieren
	DijkstraNode::Container DijkstraNodes;
	InitDijkstraNodes(DijkstraNodes, *this, Start, m_Nodes);

	// Der Endpunkt wird gesondert behandelt, da er im Sichtbarkeitsgraphen nicht vorhanden ist
	DijkstraNode EndPoint;

	// Da in jedem Durchgang ein Knoten aus der Knotenliste gewählt wird, und danach nie wieder gewählt werden kann, ist die maximale Anzahl der
	// Schleifendurchläufe durch die Anzahl der Knoten begrenzt.
	for (unsigned int i = 0; i < m_Nodes.size(); i++)
	{
		// Bestimme nächstgelegenen Knoten in der Randknotenliste
		DijkstraNode::Iter NodeInter = ChooseClosestNode(DijkstraNodes);
		(*NodeInter).Chosen = true;

		// Falls kein freier Knoten mehr in der Randknotenliste vorhanden ist, gibt es keinen Weg vom Start- zum Endknoten.
		// Dieser Fall sollte nie auftreten, da die Anzahl der Schleifendurchgänge begrenzt ist, aber sicher ist sicher.
		if (NodeInter == DijkstraNodes.end()) return false;

		// Wenn der Zielpunkt noch näher liegt als der nächte Punkt, ist die Suche beendet
		if (EndPoint.Cost <= (*NodeInter).Cost)
		{
			// Ergebnispfad extrahieren

			// Den Endpunkt in den Ergebnispfad einfügen
			Path.push_back(End);

			// Die Wegknoten in umgekehrter Reihenfolge ablaufen und in den Ergebnispfad einfügen
			DijkstraNode::ConstIter CurNode = EndPoint.ParentIter;
			while (CurNode != DijkstraNodes.end())
			{
				BS_ASSERT((*CurNode).Chosen);
				Path.push_back(m_Nodes[CurNode - DijkstraNodes.begin()]);
				CurNode = (*CurNode).ParentIter;
			}

			// Den Startpunkt in den Ergebnispfad einfügen
			Path.push_back(Start);

			// Die Knoten des Pfades müssen ungedreht werden, da sie in umgekehrter Reihenfolge extrahiert wurden.
			// Diesen Schritt könnte man sich sparen, wenn man den Pfad vom Ende zum Anfang sucht.
			std::reverse(Path.begin(), Path.end());

			return true;
		}

		// Relaxation-Schritt für die Knoten des Graphen und für den Endknoten durchführen
		RelaxNodes(DijkstraNodes, m_VisibilityMatrix, NodeInter);
		RelaxEndPoint(m_Nodes[NodeInter - DijkstraNodes.begin()], NodeInter, End, EndPoint, *this);
	}

	// Falls die Schleife komplett durchlaufen wurde, wurden alle Knoten gewählt und es wurde trotzdem kein Pfad gefunden. Es existiert also keiner.
	return false;
}

// -----------------------------------------------------------------------------

void BS_WalkRegion::InitNodeVector()
{
	// Knoten-Vector leeren.
	m_Nodes.clear();

	// Anzahl der Knoten bestimmen.
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

void BS_WalkRegion::ComputeVisibilityMatrix()
{
	// Sichtbarkeitsmatrix initialisieren
	m_VisibilityMatrix = std::vector< std::vector <int> >(m_Nodes.size(), std::vector<int>(m_Nodes.size(), infinity));

	// Sichtbarkeiten zwischen Vertecies berechnen und in die Sichbarkeitsmatrix eintragen.
	for (unsigned int j = 0; j < m_Nodes.size(); ++j)
	{
		for (unsigned int i = j; i < m_Nodes.size(); ++i)
		{
			if (IsLineOfSight(m_Nodes[i], m_Nodes[j]))
			{
				// Wenn eine Sichtlinie besteht wird die Entfernung der Knoten eingetragen
				int Distance = m_Nodes[i].Distance(m_Nodes[j]);
				m_VisibilityMatrix[i][j] = Distance;
				m_VisibilityMatrix[j][i] = Distance;
			}
			else
			{
				// Wenn keine Sichtlinie besteht wird die Entfernung "unendlich" eingetragen
				m_VisibilityMatrix[i][j] = infinity;
				m_VisibilityMatrix[j][i] = infinity;
			}
		}
	}
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::CheckAndPrepareStartAndEnd(BS_Vertex & Start, BS_Vertex & End) const
{
	if (!IsPointInRegion(Start))
	{
		BS_Vertex NewStart = FindClosestRegionPoint(Start);

		// Sicherstellen, dass der ermittelte Punkt wirklich innerhalb der Region liegt und Notfalls abbrechen.
		if (!IsPointInRegion(NewStart))
		{
			BS_LOG_ERRORLN("Constructed startpoint ((%d,%d) from (%d,%d)) is not inside the region.",
				NewStart.X, NewStart.Y,
				Start.X, Start.Y);
			return false;
		}

		Start = NewStart;
	}

	// Falls der Zielpunkt außerhalb der Region liegt, wird der nächste Punkt innerhalb der Region bestimmt und als Endpunkt benutzt.
	if (!IsPointInRegion(End))
	{
		BS_Vertex NewEnd = FindClosestRegionPoint(End);

		// Sicherstellen, dass der ermittelte Punkt wirklich innerhalb der Region liegt und Notfalls abbrechen.
		if (!IsPointInRegion(NewEnd))
		{
			BS_LOG_ERRORLN("Constructed endpoint ((%d,%d) from (%d,%d)) is not inside the region.",
				NewEnd.X, NewEnd.Y,
				End.X, End.Y);
			return false;
		}

		End = NewEnd;
	}

	// Erfolg signalisieren
	return true;
}

// -----------------------------------------------------------------------------

void BS_WalkRegion::SetPos(int X, int Y)
{
	// Unterschied zwischen alter und neuer Position berechnen.
	BS_Vertex Delta(X - m_Position.X, Y - m_Position.Y);

	// Alle Nodes verschieben.
	for (unsigned int i = 0; i < m_Nodes.size(); i++) m_Nodes[i] += Delta;

	// Region verschieben
	BS_Region::SetPos(X, Y);
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	// Elternobjekt persistieren.
	Result &= BS_Region::Persist(Writer);

	// Knoten persistieren.
	Writer.Write(m_Nodes.size());
	std::vector<BS_Vertex>::const_iterator It = m_Nodes.begin();
	while (It != m_Nodes.end())
	{
		Writer.Write(It->X);
		Writer.Write(It->Y);
		++It;
	}

	// Sichtbarkeitsmatrix persistieren.
	Writer.Write(m_VisibilityMatrix.size());
	std::vector< std::vector<int> >::const_iterator RowIter = m_VisibilityMatrix.begin();
	while (RowIter != m_VisibilityMatrix.end())
	{
		Writer.Write(RowIter->size());
		std::vector<int>::const_iterator ColIter = RowIter->begin();
		while (ColIter != RowIter->end())
		{
			Writer.Write(*ColIter);
			++ColIter;
		}

		++RowIter;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_WalkRegion::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	// Das Elternobjekt wurde schon über den Konstruktor von BS_Region geladen, daher müssen an dieser Stelle nur noch die zusätzlichen Daten von
	// BS_WalkRegion geladen werden.

	// Knoten laden.
	unsigned int NodeCount;
	Reader.Read(NodeCount);
	m_Nodes.clear();
	m_Nodes.resize(NodeCount);
	std::vector<BS_Vertex>::iterator It = m_Nodes.begin();
	while (It != m_Nodes.end())
	{
		Reader.Read(It->X);
		Reader.Read(It->Y);
		++It;
	}

	// Sichtbarkeitsmatrix laden.
	unsigned int RowCount;
	Reader.Read(RowCount);
	m_VisibilityMatrix.clear();
	m_VisibilityMatrix.resize(RowCount);
	std::vector< std::vector<int> >::iterator RowIter = m_VisibilityMatrix.begin();
	while (RowIter != m_VisibilityMatrix.end())
	{
		unsigned int ColCount;
		Reader.Read(ColCount);
		RowIter->resize(ColCount);
		std::vector<int>::iterator ColIter = RowIter->begin();
		while (ColIter != RowIter->end())
		{
			Reader.Read(*ColIter);
			++ColIter;
		}

		++RowIter;
	}

	return Result && Reader.IsGood();
}
