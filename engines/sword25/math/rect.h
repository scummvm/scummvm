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

#ifndef BS_RECT_H
#define BS_RECT_H

// Includes
#include "kernel/common.h"
#include "vertex.h"

// Klassendefinition
/**
	@brief Diese Klasse beschreibt ein Rechteck und einige nützliche Operationen auf Rechtecken.
*/
class BS_Rect
{
public:
	/// Das linke Extrem des Rechteckes.
	int	left;
	/// Das obere Extrem des Rechteckes.
	int top;
	/// Das rechte Extrem des Rechteckes + 1.
	int right;
	/// Das untere Extrem des Rechteckes + 1.
	int bottom;

	/**
		@brief Konstruktor, der alle Werte des Rechteckes mit 0 initialisiert.
	*/
	BS_Rect()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}
	/**
		@brief Konstruktor, der das Rechteck mit den übergebenen Werten initialisiert.
		@param left das linke Extrem des Rechteckes
		@param top das obere Extrem des Rechteckes
		@param right das rechte Extrem des Rechteckes + 1
		@param bottom des untere Extrem des Rechteckes + 1
	*/
	BS_Rect(int left, int top, int right, int bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}
	/**
		@brief Verschiebt das Rechteck.
		@param DeltaX der Wert um den das Rechteck auf der X-Achse verschoben werden soll.
		@param DeltaY der Wert um den das Rechteck auf der Y-Achse verschoben werden soll.
	*/
	void Move(int DeltaX, int DeltaY)
	{
		left += DeltaX;
		right += DeltaX;
		top += DeltaY;
		bottom += DeltaY;
	}
	/**
		@brief Testet ob sich zwei Rechtecke schneiden.
		@return Gibt true zurück, wenn sich die Rechtecke schneiden.
	*/
	bool DoesIntersect(const BS_Rect& Rect) const
	{
		int Dist;
		
		// Intersektion auf der X-Achse
		Dist = left - Rect.left;
		if (Dist < 0)
		{
			Dist = abs(Dist);
			
			// Schneiden sich die Rechtecke nicht auf der X-Achse, so schneiden sie sich gar nicht
			if (Dist >= GetWidth())
				return false;
		}
		else
		{
			// Schneiden sich die Rechtecke nicht auf der X-Achse, so schneiden sie sich gar nicht
			if (Dist >= Rect.right - Rect.left)
				return false;
		}
		
		// Intersektion auf der Y-Achse
		Dist = top - Rect.top;
		if (Dist < 0)
		{
			Dist = abs(Dist);
			
			// Schneiden sich die Rechtecke nicht auf der Y-Achse, so schneiden sie sich gar nicht
			if (Dist >= GetHeight())
				return false;
		}
		else
		{
			// Schneiden sich die Rechtecke nicht auf der Y-Achse, so schneiden sie sich gar nicht
			if (Dist >= Rect.bottom - Rect.top)
				return false;
		}
		
		return true;
	}

	/**
		@brief Bildet den Durchschnitt zweier Rechtecke
		@param Rect das Rechteck, dass mit dem Objekt geschnitter werden soll.
		@param Result das Rechteck, dass das Ergebnisrechteck enthalten soll.
		@return Gibt false zurück, falls Result undefiniert ist.<br>
				Dies ist der Fall, wenn sich die Rechtecke nicht schneiden.
	*/
	bool Intersect(const BS_Rect& Rect, BS_Rect& Result) const
	{
		int Dist;
		
		// Intersektion auf der X-Achse
		Dist = left - Rect.left;
		if (Dist < 0)
		{
			Dist = abs(Dist);
			
			// Schneiden sich die Rechtecke nicht auf der X-Achse, so schneiden sie sich gar nicht
			if (Dist >= GetWidth())
			{
				Result = BS_Rect(0, 0, 0, 0);
				return false;
			}
			
			// Die Abmessungen des Rect auf der X-Achse berechnen
			Result.left = Rect.left;
		}
		else
		{
			// Schneiden sich die Rechtecke nicht auf der X-Achse, so schneiden sie sich gar nicht
			if (Dist >= Rect.right - Rect.left)
			{
				Result = BS_Rect(0, 0, 0, 0);
				return false;
			}
			
			// Die Abmessungen des Rect auf der X-Achse berechnen
			Result.left = left;
		}
		Result.right = right < Rect.right ? right : Rect.right;
		
		// Intersektion auf der Y-Achse
		Dist = top - Rect.top;
		if (Dist < 0)
		{
			Dist = abs(Dist);
			
			// Schneiden sich die Rechtecke nicht auf der Y-Achse, so schneiden sie sich gar nicht
			if (Dist >= GetHeight())
			{
				Result = BS_Rect(0, 0, 0, 0);
				return false;
			}
			
			// Die Abmessungen des Rect auf der Y-Achse berechnen
			Result.top = Rect.top;
		}
		else
		{
			// Schneiden sich die Rechtecke nicht auf der Y-Achse, so schneiden sie sich gar nicht
			if (Dist >= Rect.bottom - Rect.top)
			{
				Result = BS_Rect(0, 0, 0, 0);
				return false;
			}
			
			// Die Abmessungen des Rect auf der Y-Achse berechnen
			Result.top = top;
		}
		Result.bottom = bottom < Rect.bottom ? bottom : Rect.bottom;
		
		return true;
	}
	/**
		@brief Bildet die Bounding-Box zweier Rechtecke.
		@param Rect das Rechteck, dass mit dem Objekt verbunden werden soll.
		@remark Das Ergebnis ist nicht ein Join in eigentlichen Sinne, sondern vielmehr die Bounding-Box um die Beiden
				Rechtecke.
	*/
	void Join(const BS_Rect& Rect, BS_Rect& Result) const
	{
		Result.left = left < Rect.left ? left : Rect.left;
		Result.top = top < Rect.top ? top : Rect.top;
		Result.right = right > Rect.right ? right : Rect.right;
		Result.bottom = bottom > Rect.bottom ? bottom : Rect.bottom;
		return;
	}
	/**
		@brief Gibt die Breite des Rechteckes zurück.
		@return Die Breite des Rechteckes.
	*/
	int GetWidth() const
	{
		return right - left;
	}
	/**
		@brief Gibt die Höhe des Rechteckes zurück.
		@return Die Höhe des Rechteckes.
	*/
	int GetHeight() const
	{
		return bottom - top;
	}
	/** @brief Gibt den Flächeninhalt des Rechteckes zurück.
		@return Der Flächeninhalt des Rechteckes.
	*/
	int GetArea() const
	{
		return GetWidth() * GetHeight();
	}
	/**
		@brief Vergleichsoperator zum Überprüfen der Gleichheit zweier BS_Rect Objekte.
		@return Gibt true zurück, wenn die Objekte die gleichen Werte haben, ansonsten false.
	*/
	bool operator== (const BS_Rect& rhs)
	{
		return (left == rhs.left) &&
			   (top == rhs.top) &&
			   (right == rhs.right) &&
			   (bottom == rhs.bottom);
	}
	/**
		@brief Vergleichsoperator zum Überprüfen der Ungleichheit zweier BS_Rect Objekte.
		@return Gibt true zurück, wenn die Objekte ungleiche Werte haben, ansonsten false.
	*/
	bool operator!= (const BS_Rect& rhs)
	{
		return !(*this == rhs);
	}
	/**
		@brief Überprüft, ob das Objekt einen gültigen Zustand hat.
		@return Gibt false zurück, wenn das Objekt einen ungültigen Zustand hat.
	*/
	bool IsValid() const
	{
		if (left < right && top < bottom) return true;
		return false;
	}
	/**
		@brief Testet, ob sich ein Vertex innerhalb des Rechteckes befindet.
		@param Vertex das Vertex, dass mit dem Rechteckes getestet werden soll
		@return Gibt true zurück, wenn sich das Vertex innerhalb des Rechteckes befindet.<br>
				Gibt false zurück, wenn sich das Vertex außerhalb des Rechteckes befindet.
	*/
	bool IsPointInRect(const BS_Vertex& Vertex) const
	{
		if (Vertex.X >= left && Vertex.X < right &&
			Vertex.Y >= top && Vertex.Y < bottom)
			return true;
		return false;
	}
	/**
		@brief Testet, ob sich ein Punkt innerhalb des Rechteckes befindet.
		@param X die Position des Punktes auf der X-Achse.
		@param Y die Position des Punktes auf der Y-Achse.
		@return Gibt true zurück, wenn sich der Punkt innerhalb des Rechteckes befindet.<br>
				Gibt false zurück, wenn sich der Punkt außerhalb des Rechteckes befindet.
	*/
	bool IsPointInRect(int X, int Y) const
	{
		return IsPointInRect(BS_Vertex(X, Y));
	}
	/**
		@brief Testet, ob ein andere Rechteck komplett in den Rechteck enthalten ist.
		@param OtherRect das zu testende Rechteck
		@brief Gibt true zurück, wenn sich das andere Rechteck komplett im Rechteck enthalten ist, ansonsten false.
	*/
	bool ContainsRect(const BS_Rect & OtherRect) const
	{
		return	IsPointInRect(OtherRect.left, OtherRect.top) &&
				IsPointInRect(OtherRect.right - 1, OtherRect.bottom - 1);
	}
};

#endif