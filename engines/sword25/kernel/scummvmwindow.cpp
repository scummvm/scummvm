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

#include "common/system.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "sword25/kernel/scummvmwindow.h"
#include "sword25/kernel/kernel.h"
#include "sword25/input/inputengine.h"

#define BS_LOG_PREFIX "WIN32WINDOW"

namespace Sword25 {

bool BS_ScummVMWindow::_ClassRegistered = false;

// Constructor / Destructor 
// ------------------------
BS_ScummVMWindow::BS_ScummVMWindow(int X, int Y, int Width, int Height, bool Visible) {
	// Presume that init will fail
	_InitSuccess = false;

	// We don't support any window creation except at the origin 0,0
	assert(X == 0);
	assert(Y == 0);

	if (!_ClassRegistered) {
		// Nothing here currently

		_ClassRegistered = true;
	}

	// Fenstersichtbarkeit setzen
	SetVisible(Visible);

	// Indicate success
	_InitSuccess = true;
	_WindowAlive = true;
	_CloseWanted = false;
}

BS_ScummVMWindow::~BS_ScummVMWindow() {
}

// Get Methods
// ------------
int BS_ScummVMWindow::GetX() {
	return 0;
}

int BS_ScummVMWindow::GetY() {
	return 0;
}

int BS_ScummVMWindow::GetClientX() {
	return 0;
}

int BS_ScummVMWindow::GetClientY() {
	return 0;
}

int BS_ScummVMWindow::GetWidth() {
	return g_system->getWidth();
}

int BS_ScummVMWindow::GetHeight() {
	return g_system->getHeight();
}

Common::String BS_ScummVMWindow::GetTitle() {
	return Common::String("");
}

bool BS_ScummVMWindow::IsVisible() {
	return true;
}

bool BS_ScummVMWindow::HasFocus() {
	// FIXME: Is there a way to tell if ScummVM has the focus in Windowed mode?
	return true;
}

uint BS_ScummVMWindow::GetWindowHandle() {
	return 0;
}

void BS_ScummVMWindow::SetWindowAlive(bool v) {
	_WindowAlive = v;
}


// Set Methods
// ------------

void BS_ScummVMWindow::SetX(int X) {
	// No implementation
}

void BS_ScummVMWindow::SetY(int Y) {
	// No implementation
}

void BS_ScummVMWindow::SetWidth(int Width) {
	// No implementation
}

void BS_ScummVMWindow::SetHeight(int Height) {
	// No implementation
}

void BS_ScummVMWindow::SetVisible(bool Visible) {
	// No implementation
}

void BS_ScummVMWindow::SetTitle(const Common::String &Title) {
	// No implementation
}

bool BS_ScummVMWindow::ProcessMessages() {
	// No implementation
	return false;
}

bool BS_ScummVMWindow::WaitForFocus() {
	// No implementation
	return true;
}

// FIXME: Special keys detected here need to be moved into the Input Engine
/*
// Die WindowProc aller Fenster der Klasse
LRESULT CALLBACK BS_ScummVMWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		ValidateRect(hwnd, NULL);
		break;
		
	case WM_DESTROY:
		// Das Fenster wird zerstört
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		{
			BS_Window * WindowPtr = BS_Kernel::GetInstance()->GetWindow();
			if (WindowPtr) {
				WindowPtr->SetCloseWanted(true);
			}
			break;
		}

	case WM_KEYDOWN:
		{
			// Tastendrücke, die für das Inputmodul interessant sind, werden diesem gemeldet.
			BS_InputEngine * InputPtr = BS_Kernel::GetInstance()->GetInput();

			if (InputPtr)
			{
				switch (wParam)
				{
				case VK_RETURN:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_ENTER);
					break;

				case VK_LEFT:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_LEFT);
					break;

				case VK_RIGHT:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_RIGHT);
					break;

				case VK_HOME:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_HOME);
					break;

				case VK_END:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_END);
					break;

				case VK_BACK:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_BACKSPACE);
					break;

				case VK_TAB:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_TAB);
					break;

				case VK_INSERT:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_INSERT);
					break;

				case VK_DELETE:
					InputPtr->ReportCommand(BS_InputEngine::KEY_COMMAND_DELETE);
					break;
				}
			}
			break;
		}

	case WM_KEYUP:
	case WM_SYSKEYUP:
		// Alle Tastendrücke werden ignoriert, damit Windows per DefWindowProc() nicht darauf
		// reagieren kann und damit unerwartete Seiteneffekte auslöst.
		// Zum Beispiel würden ALT und F10 Tastendrücke das "Menü" aktivieren und somit den Message-Loop zum Stillstand bringen.
		break;

	case WM_SYSCOMMAND:
		// Verhindern, dass der Bildschirmschoner aktiviert wird, während das Spiel läuft
		if (wParam != SC_SCREENSAVE) return DefWindowProc(hwnd,uMsg,wParam,lParam);
		break;

	case WM_CHAR:
		{
			unsigned char theChar = static_cast<unsigned char>(wParam & 0xff);

			// Alle Zeichen, die keine Steuerzeichen sind, werden als Buchstaben dem Input-Service mitgeteilt.
			if (theChar >= 32)
			{
				BS_InputEngine * InputPtr = BS_Kernel::GetInstance()->GetInput();
				if (InputPtr) InputPtr->ReportCharacter(theChar);
			}
		}
		break;

	case WM_SETCURSOR:
		{
			// Der Systemcursor wird in der Client-Area des Fensters nicht angezeigt, jedoch in der nicht Client-Area, damit der Benutzer das Fenster wie gewohnt
			// schließen und verschieben kann.

			// Koordinaten des Cursors in der Client-Area berechnen.
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);

			// Feststellen, ob sich der Cursor in der Client-Area befindet.
			// Get client rect
			RECT rc;
			GetClientRect(hwnd, &rc);

			// See if cursor is in client area
			if(PtInRect(&rc, pt))
				// In der Client-Area keinen Cursor anzeigen.
				SetCursor(NULL);
			else
				// Ausserhalb der Client-Area den Cursor anzeigen.
				SetCursor(LoadCursor(NULL, IDC_ARROW));

			return TRUE;
		}
		break;

	default:
		// Um alle anderen Vorkommnisse kümmert sich Windows
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	return 0;
}
*/

} // End of namespace Sword25
