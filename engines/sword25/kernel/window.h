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

/*
	BS_Window
	---------
	Simples Fensterklasseninterface.
	Ist nur aus Gründen der Portabilität in einer Klasse gekapselt.
	TODO: Für andere Betriebssysteme implementieren

	Autor: Malte Thiesen
*/

#ifndef SWORD25_WINDOW_H
#define SWORD25_WINDOW_H

// Includes
#include "sword25/kernel/common.h"

// Klassendefinition
/**
	@brief Ein Simples Fensterklasseninterface.
	
	Fenster werden ausschließlich mit BS_Window::CreateBSWindow() erstellt. BS_Window wählt
	dann selbständig die richtige Klasse für das Betriebssystem aus.
*/
class BS_Window
{
protected:
	bool _InitSuccess;
	bool _CloseWanted;

public:
	virtual ~BS_Window(){};

	/**
		@brief Gibt den Sichtbarkeitsstatus des Fensters zurück
		@return Gibt true zurück wenn das Fenster sichtbar ist, andernfalls false
	*/
	virtual bool IsVisible() = 0;
	/**
		@brief Setzt den Sichtbarkeitsstatus des Fensters
		@param Visible gibt an, ob das Fenster sichtbar oder versteckt sein soll
	*/
	virtual void SetVisible(bool Visible) = 0;
	/**
		@brief Gibt die Position des Fensters auf der X-Achse zurück
		@return Gibt die Position des Fensters auf der X-Achse zurück
	*/
	virtual int GetX() = 0;
	/**
		@brief Setzt die Position des Fensters auf der X-Achse
		@param X die X-Position des Fensters oder -1 für zentrierte Ausrichtung auf der X-Achse
	*/
	virtual void SetX(int X) = 0;
	/**
		@brief Gibt die Position des Fensters auf der Y-Achse zurück
		@return Gibt die Position des Fensters auf der Y-Achse zurück
	*/
	virtual int GetY() = 0;
	/**
		@brief Setzt die Position des Fensters auf der Y-Achse
		@param Y die Y-Position des Fensters oder -1 für zentrierte Ausrichtung auf der Y-Achse
	*/
	virtual void SetY(int X) = 0;
	/**
		@brief Gibt die Position des Fensterinhaltes auf der X-Achse zurück
		@return Gibt die Position des Fensterinhaltes auf der X-Achse zurück
	*/
	virtual int GetClientX() = 0;
		/**
		@brief Gibt die Position des Fensterinhaltes auf der Y-Achse zurück
		@return Gibt die Position des Fensterinhaltes auf der Y-Achse zurück
	*/
	virtual int GetClientY() = 0;
	/**
		@brief Gibt die Breite des Fensters ohne Rahmen zurück
		@return Gibt die Breite des Fensters ohne Rahmen zurück
	*/
	virtual int GetWidth() = 0;
	/**
		@brief Setzt die Breite des Fensters ohne Rahmen
		@param Width die Breite des Fensters ohne Rahmen
	*/
	virtual void SetWidth(int Width) = 0;
	/**
		@brief Gibt die Höhe des Fensters ohne Rahmen und Kopfzeile zurück
		@return Gibt die Höhe des Fensters ohne Rahmen und Kopfzeile zurück
	*/
	virtual int GetHeight() = 0;
	/**
		@brief Setzt die Höhe des Fensters ohne Rahmen und Kopfzeile
		@param Height die Höhe des Fensters ohne Rahmen und Kopfzeile
	*/
	virtual void SetHeight(int Height) = 0;
	/**
		@brief Gibt den Titel der Fensters zurück
		@return Gibt den Titel des Fenster zurück
	*/
	virtual std::string GetTitle() = 0;
	/**
		@brief Setzt den Titel des Fensters
		@param Title der neue Titel des Fensters
	*/
	virtual void SetTitle(std::string Title) = 0;
	/**
		@brief Arbeitet die Windowmessages des Fensters ab.
			   Diese Methode sollte während des Main-Loops aufgerufen werden.
		@return Gibt false zurück, falls das Fenster geschlossen wurde.
	*/
	virtual bool ProcessMessages() = 0;
	/**
		@brief Pausiert die Applikation bis das Fenster wieder den Focus hat oder geschlossen wurde.
		@return Gibt false zurück, falls das Fenster geschlossen wurde.
	*/
	virtual bool WaitForFocus() = 0;	
	/**
		@brief Gibt zurück, ob das Fenster den Focus hat.
		@return Gibt true zurück, wenn das Fenster den Focus hat, ansonsten false
	*/
	virtual bool HasFocus() = 0;
	/**
		@brief Gibt das Windowhandle, des Systems zurück.
		@return Das Windowhandle des Fensters
		@remark Wenn das Windowshandle benutzt wird, sind die entsprechenden Codeteile 
				natürlich nicht mehr portable.
	*/
	virtual unsigned int GetWindowHandle() = 0;


	/**
		@brief Setzt den Rückgabewert für den nächsten Aufruf von CloseWanted. Sollte vom
		       Fenster selbst verwendet werden, wenn es geschlossen werden möchte. Dieser 
		       Mechanismus erlaubt den Scripten abzufragen, wann das Hauptfenster geschlossen
			   werden soll, und sich entsprechend zu beenden, bzw. beim Nutzer nachzufragen.
	**/
	void SetCloseWanted(bool Wanted);	
	/**
		@brief Gibt einmal den Wert des letztes Aufrufs von SetCloseWanted zurück, 
			   und danach sofort wieder false, solange bis mit SetCloseWanted wieder
			   ein neuer Wert gesetzt wird.
	**/
	bool CloseWanted();


	/**
		@brief Erstellt eine neue Fensterinstanz.
		@param X die X-Position des Fensters oder -1 für zentrierte Ausrichtung auf der X-Achse
		@param Y des Y-Position des Fensters oder -1 für zentrierte Ausrichtung auf der Y-Achsen
		@param Width die Breite des Fensters ohne Rahmen
		@param Height die Höhe des Fensters ohne Rahmen und Kopfzeile
		@param Visible gibt an, ob das Fenster dargestellt werden soll
		@return Gibt einen Pointer auf ein Fensterobjekt zurück, oder NULL wenn das Erstellen
				fehlgeschlagen ist.
		@remark Das Fenster muss nach Benutzung mit delete freigegeben werden!
	*/
	static BS_Window* CreateBSWindow(int X, int Y, int Width, int Height, bool Visible);
};

#endif
