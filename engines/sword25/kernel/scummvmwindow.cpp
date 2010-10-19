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

bool ScummVMWindow::_classRegistered = false;

// Constructor / Destructor
// ------------------------
ScummVMWindow::ScummVMWindow(int x, int y, int width, int height, bool visible) {
	// Presume that init will fail
	_initSuccess = false;

	// We don't support any window creation except at the origin 0,0
	assert(x == 0);
	assert(y == 0);

	if (!_classRegistered) {
		// Nothing here currently

		_classRegistered = true;
	}

	// Fenstersichtbarkeit setzen
	setVisible(visible);

	// Indicate success
	_initSuccess = true;
	_windowAlive = true;
	_closeWanted = false;
}

ScummVMWindow::~ScummVMWindow() {
}

// Get Methods
// ------------
int ScummVMWindow::getX() {
	return 0;
}

int ScummVMWindow::getY() {
	return 0;
}

int ScummVMWindow::getClientX() {
	return 0;
}

int ScummVMWindow::getClientY() {
	return 0;
}

int ScummVMWindow::getWidth() {
	return g_system->getWidth();
}

int ScummVMWindow::getHeight() {
	return g_system->getHeight();
}

Common::String ScummVMWindow::getTitle() {
	return Common::String("");
}

bool ScummVMWindow::isVisible() {
	return true;
}

bool ScummVMWindow::hasFocus() {
	// FIXME: Is there a way to tell if ScummVM has the focus in Windowed mode?
	return true;
}

uint ScummVMWindow::getWindowHandle() {
	return 0;
}

void ScummVMWindow::setWindowAlive(bool v) {
	_windowAlive = v;
}


// Set Methods
// ------------

void ScummVMWindow::setX(int X) {
	// No implementation
}

void ScummVMWindow::setY(int Y) {
	// No implementation
}

void ScummVMWindow::setWidth(int width) {
	// No implementation
}

void ScummVMWindow::setHeight(int height) {
	// No implementation
}

void ScummVMWindow::setVisible(bool visible) {
	// No implementation
}

void ScummVMWindow::setTitle(const Common::String &title) {
	// No implementation
}

bool ScummVMWindow::processMessages() {
	// All messages are handled separately in the input manager. The only thing we
	// need to do here is to keep returning whether the window/game is still alive
	return _windowAlive;
}

bool ScummVMWindow::waitForFocus() {
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
            byte theChar = static_cast<byte>(wParam & 0xff);

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
