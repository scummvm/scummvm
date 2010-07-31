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

#include "sword25/kernel/win32window.h"
#include "../../projects/resource.h"

#include "sword25/kernel/kernel.h"
#include "sword25/input/inputengine.h"

bool BS_Win32Window::_ClassRegistered = false;

#define BS_LOG_PREFIX "WIN32WINDOW"


// Konstanten
// ----------
static const UINT WINDOW_STYLE			= WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
static const UINT WINDOW_STYLE_EX		= 0;
static const UINT WINDOW_MAX_MESSAGES	= 50;

// Konstruktion/Destruktion
// ------------------------
BS_Win32Window::BS_Win32Window(int X, int Y, int Width, int Height, bool Visible)
{
	const char WINDOW_CLASS[] = "BSEngine-Class";

	// Von negativen Fall ausgehen
	_InitSuccess = false;

	// Fensterklasse registrieren falls nötig
	if (!_ClassRegistered)
	{
		//Fensterklasse
		WNDCLASSEX wndclass;
		
		//Werte der Fensterklasse festlegen
		ZeroMemory(&wndclass, sizeof(WNDCLASSEX));
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndclass.lpfnWndProc = BS_Win32Window::WindowProc;
		wndclass.hInstance = GetModuleHandle(NULL);
		wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
		wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndclass.lpszClassName = WINDOW_CLASS;
		
		//Fensterklasse registrieren
		if (!RegisterClassEx(&wndclass)) return;

		_ClassRegistered = true;
	}

	//Fenster erstellen
	if (!(_Window=CreateWindowEx(
		WINDOW_STYLE_EX,					// Erweiterte Darstellungsflags
		WINDOW_CLASS,						// Registrierter Fenstername
		"",									// Kein Fenstertitel
		WINDOW_STYLE,						// Darstellungsflags
		0,0,								// Default-Position
		0,0,								// Default-Grösse
		NULL,								// Kein Parent-Fenster
		NULL,								// Kein Menü
		GetModuleHandle(NULL),				// Instance-Handle
		NULL)))
		return;

	// Fensterposition und Fenstergröße setzen
	SetWidth(Width);
	SetHeight(Height);
	SetX(X);
	SetY(Y);

	// Fenstersichtbarkeit setzen
	SetVisible(Visible);

	// Icon setzen
	HICON hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1));
	if (hIcon)
	{
		SendMessage(_Window, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
		SendMessage(_Window, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
	}

	// Erfolg signalisieren
	_InitSuccess = true;
	_WindowAlive = true;
	_CloseWanted = false;
}

BS_Win32Window::~BS_Win32Window()
{
	// Fenster zerstören, falls dies nicht ohnehin schon passiert ist
	if (_WindowAlive) DestroyWindow(_Window);
}

// Get-Methoden
// ------------
int BS_Win32Window::GetX()
{
	RECT Rect;
	GetWindowRect(_Window, &Rect);
	return Rect.left;
}

int BS_Win32Window::GetY()
{
	RECT Rect;
	GetWindowRect(_Window, &Rect);
	return Rect.top;
}

int BS_Win32Window::GetClientX()
{
	POINT Point = {0, 0};
	ClientToScreen(_Window, &Point);
	return Point.x;
}

int BS_Win32Window::GetClientY()
{
	POINT Point = {0, 0};
	ClientToScreen(_Window, &Point);
	return Point.y;
}

int BS_Win32Window::GetWidth()
{
	RECT Rect;
	GetClientRect(_Window, &Rect);
	return Rect.right - Rect.left;
}

int BS_Win32Window::GetHeight()
{
	RECT Rect;
	GetClientRect(_Window, &Rect);
	return Rect.bottom - Rect.top;
}

std::string BS_Win32Window::GetTitle()
{
	char String[512];
	if (GetWindowText(_Window, String, sizeof(String)))
		return std::string(String);

	return std::string("");
}

bool BS_Win32Window::IsVisible()
{
	return IsWindowVisible(_Window) ? true : false;
}

bool BS_Win32Window::HasFocus()
{
	return GetForegroundWindow() == _Window ? true : false;
}

UINT BS_Win32Window::GetWindowHandle()
{
	return (UINT)_Window;
}

// Set Methoden
// ------------

void BS_Win32Window::SetX(int X)
{
	int RealX;
	if (X == -1)
	{
		RECT Rect;
		GetWindowRect(_Window, &Rect);
		RealX = (GetSystemMetrics(SM_CXSCREEN) - (Rect.right - Rect.left)) / 2;
	}
	else
		RealX = X;

	SetWindowPos(_Window, NULL, RealX, GetY(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void BS_Win32Window::SetY(int Y)
{
	int RealY;
	if (Y == -1)
	{
		RECT Rect;
		GetWindowRect(_Window, &Rect);
		RealY = (GetSystemMetrics(SM_CYSCREEN) - (Rect.bottom - Rect.top)) / 2;
	}
	else
		RealY = Y;
	
	SetWindowPos(_Window, NULL, GetX(), RealY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void BS_Win32Window::SetWidth(int Width)
{
	RECT Rect = {0, 0, Width, GetHeight()};
	AdjustWindowRectEx(&Rect, WINDOW_STYLE, false, WINDOW_STYLE_EX);
	SetWindowPos(_Window, NULL, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void BS_Win32Window::SetHeight(int Height)
{
	RECT Rect = {0, 0, GetWidth(), Height};
	AdjustWindowRectEx(&Rect, WINDOW_STYLE, false, WINDOW_STYLE_EX);
	SetWindowPos(_Window, NULL, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOMOVE | SWP_NOZORDER);	
}

void BS_Win32Window::SetVisible(bool Visible)
{
	ShowWindow(_Window, Visible ? SW_SHOW : SW_HIDE);
}

void BS_Win32Window::SetTitle(std::string Title)
{
	SetWindowText(_Window, Title.c_str());
}

// Asynchroner Message-Loop
bool BS_Win32Window::ProcessMessages()
{
	for (UINT i = 0; i < WINDOW_MAX_MESSAGES; i++)
	{
		MSG msg;
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				_WindowAlive = false;
				return false;
			}

			// Alle Nachrichten zur Verarbeitung durch WindowProc vorbereiten
			TranslateMessage(&msg);
			// Nachricht an WindowProc übergeben
			DispatchMessage(&msg);
		}
		else
			return true;
	}

	return true;
}

// Synchroner Message-Loop
bool BS_Win32Window::WaitForFocus()
{
	MSG msg;
	
	// Fenster minimieren
	ShowWindow(_Window, SW_MINIMIZE);

	for (;;)
	{
		// Auf Nachricht warten
		WaitMessage();
		// Nachricht einlesen
		GetMessage(&msg, NULL, 0, 0);
		// Nachricht zur Verarbeitung durch WindowProc vorbereiten
		TranslateMessage(&msg);
		// Nachricht an WindowProc übergeben
		DispatchMessage(&msg);

		// Überprüfen ob das Fenster geschlossen wurde
		if (msg.message == WM_QUIT)
		{
			_WindowAlive = false;
			return false;
		}

		// Überprüfen, ob das Fenster den Focus wiedererlangt hat
		if (HasFocus()) return true;
	}
}

// Die WindowProc aller Fenster der Klasse
LRESULT CALLBACK BS_Win32Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
