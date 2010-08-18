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

#include "sword25/kernel/window.h"

// Alle Implementationen von BS_Window müssen hier eingetragen werden
#include "sword25/kernel/scummvmwindow.h"

namespace Sword25 {

// Erstellt ein Fenster des GUI des aktuellen Betriebssystems
Window *Window::CreateBSWindow(int X, int Y, int Width, int Height, bool Visible) {
	// Fenster erstellen
	Window *pWindow = (Window *) new ScummVMWindow(X, Y, Width, Height, Visible);

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
bool Window::CloseWanted() {
	bool result = _CloseWanted;
	_CloseWanted = false;
	return result;
}

void Window::SetCloseWanted(bool Wanted) {
	_CloseWanted = Wanted;
}

} // End of namespace Sword25
