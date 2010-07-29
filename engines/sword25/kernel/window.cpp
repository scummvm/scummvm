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

#include "window.h"

// Alle Implementationen von BS_Window müssen hier eingetragen werden
#include "win32window.h"

// Erstellt ein Fenster des GUI des aktuellen Betriebssystems
BS_Window* BS_Window::CreateBSWindow(int X, int Y, int Width, int Height, bool Visible)
{
	// Fenster erstellen
	BS_Window* pWindow = (BS_Window*) new BS_Win32Window(X, Y, Width, Height, Visible);

	// Falls das Fenster erfolgreich initialisiert wurde, wird ein Pointer auf das Fensterobjekt
	// zurückgegeben
	if (pWindow->_InitSuccess)
		return pWindow;

	// Ansonsten wird das Fensterobjekt zerstört und NULL zurückgegeben
	delete pWindow;
	return NULL;
}

// Gibt True zurück wenn das Fenster WM_CLOSE empfangen hat - 
// solange, bis RejectClose() aufgerufen wurde.
bool BS_Window::CloseWanted()
{
	bool result = _CloseWanted;
	_CloseWanted = false;
	return result;
}

void BS_Window::SetCloseWanted(bool Wanted)
{
	_CloseWanted = Wanted;
}
