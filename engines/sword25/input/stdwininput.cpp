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

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/input/stdwininput.h"

#include <algorithm>
using namespace std;

#define BS_LOG_PREFIX "WININPUT"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_StdWinInput::BS_StdWinInput(BS_Kernel* pKernel) :
	m_CurrentState(0),
	m_LeftMouseDown(false),
	m_RightMouseDown(false),
	m_MouseX(0),
	m_MouseY(0),
	m_LeftDoubleClick(false),
	m_DoubleClickTime(GetDoubleClickTime()),
	m_DoubleClickRectWidth(GetSystemMetrics(SM_CXDOUBLECLK)),
	m_DoubleClickRectHeight(GetSystemMetrics(SM_CYDOUBLECLK)),
	m_LastLeftClickTime(0),
	m_LastLeftClickMouseX(0),
	m_LastLeftClickMouseY(0),
	BS_InputEngine(pKernel)
{
	memset(m_KeyboardState[0], 0, sizeof(m_KeyboardState[0]));
	memset(m_KeyboardState[1], 0, sizeof(m_KeyboardState[1]));
	m_LeftMouseState[0] = false;
	m_LeftMouseState[1] = false;
	m_RightMouseState[0] = false;
	m_RightMouseState[1] = false;
}

BS_StdWinInput::~BS_StdWinInput()
{
}

// -----------------------------------------------------------------------------

BS_Service * BS_StdWinInput_CreateObject(BS_Kernel* pKernel) { return new BS_StdWinInput(pKernel); }

// -----------------------------------------------------------------------------

bool BS_StdWinInput::Init()
{
	// Keine Inialisierung notwendig
	return true;
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::Update()
{
	// Der Status wird nur aktualisiert, wenn das Applikationsfenster den Fokus hat, so wird verhindert, dass
	// Eingaben verarbeitet werden, die eigentlich für eine andere Applikation gedacht waren.
	if (BS_Kernel::GetInstance()->GetWindow()->HasFocus())
	{
		m_CurrentState ^= 1;

		// Der Status der Eingabegeräte wird nur einmal pro Frame ausgelesen, damit für
		// jeden Frame gleiche Anfragen die gleiche Antwort erhalten.

		POINT MousePos;
		if (GetCursorPos(&MousePos))
		{
			m_MouseX = MousePos.x - BS_Kernel::GetInstance()->GetWindow()->GetClientX();
			m_MouseY = MousePos.y - BS_Kernel::GetInstance()->GetWindow()->GetClientY();
		}
		else
		{
			BS_LOG_ERRORLN("Call to GetCursorPos() failed.");
			m_MouseX = 0;
			m_MouseY = 0;
		}

		GetKeyboardState(m_KeyboardState[m_CurrentState]);

		m_LeftMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		m_RightMouseDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		m_LeftMouseState[m_CurrentState] = m_LeftMouseDown;
		m_RightMouseState[m_CurrentState] = m_RightMouseDown;

		TestForLeftDoubleClick();
	}
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::IsLeftMouseDown()
{
	return m_LeftMouseDown;
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::IsRightMouseDown()
{
	return m_RightMouseDown;
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::TestForLeftDoubleClick()
{
	// Das Doppelklick-Flag wird gelöscht, für den Fall, dass im letzten Frame ein Doppelklick ausgetreten ist.
	m_LeftDoubleClick = false;

	// Die linke Maustaste wurde geklickt, also muss getestet werden, ob ein Doppelklick vorliegt.
	if (WasLeftMouseDown())
	{
		// Die Zeit auslesen.
		unsigned int Now = BS_Kernel::GetInstance()->GetMilliTicks();

		// Ein Doppelklick wird erkannt, wenn:
		// 1. Die zwei Klicks liegen nah genug zusammen.
		// 2. Der Mauscursor wurde zwischen den Klicks nicht zu viel bewegt.
		if (Now - m_LastLeftClickTime <= m_DoubleClickTime &&
			abs(m_MouseX - m_LastLeftClickMouseX) <= m_DoubleClickRectWidth / 2 &&
			abs(m_MouseY - m_LastLeftClickMouseY) <= m_DoubleClickRectHeight / 2)
		{
			m_LeftDoubleClick = true;

			// Die Zeit und Position des letzten Linksklicks zurücksetzen, damit dieser Klick nicht als erster Klick eines weiteren Doppelklicks
			// interpretiert wird.
			m_LastLeftClickTime = 0;
			m_LastLeftClickMouseX = 0;
			m_LastLeftClickMouseY = 0;
		}
		else
		{
			// Es liegt kein Doppelklick vor, die Zeit und die Position dieses Klicks merken, für den Fall das dies der erste Klick eines
			// zukünftigen Doppelklicks wird.
			m_LastLeftClickTime = Now;
			m_LastLeftClickMouseX = m_MouseX;
			m_LastLeftClickMouseY = m_MouseY;
		}
	}
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::IsLeftDoubleClick()
{
	return m_LeftDoubleClick;
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::WasLeftMouseDown()
{
	return (m_LeftMouseState[m_CurrentState] == false) && (m_LeftMouseState[m_CurrentState ^ 1] == true);
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::WasRightMouseDown()
{
	return (m_RightMouseState[m_CurrentState] == false) && (m_RightMouseState[m_CurrentState ^ 1] == true);
}

// -----------------------------------------------------------------------------

int BS_StdWinInput::GetMouseX()
{
	return m_MouseX;	
}

// -----------------------------------------------------------------------------

int BS_StdWinInput::GetMouseY()
{
	return m_MouseY;
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::IsKeyDown(unsigned int KeyCode)
{
	return (m_KeyboardState[m_CurrentState][KeyCode] & 0x80) != 0;
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::WasKeyDown(unsigned int KeyCode)
{
	return ((m_KeyboardState[m_CurrentState][KeyCode] & 0x80) == 0) && ((m_KeyboardState[m_CurrentState ^ 1][KeyCode] & 0x80) != 0);
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::SetMouseX(int PosX)
{
	m_MouseX = PosX;
	SetCursorPos(m_MouseX + BS_Kernel::GetInstance()->GetWindow()->GetClientX(), m_MouseY + BS_Kernel::GetInstance()->GetWindow()->GetClientY());
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::SetMouseY(int PosY)
{
	m_MouseY = PosY;
	SetCursorPos(m_MouseX + BS_Kernel::GetInstance()->GetWindow()->GetClientX(), m_MouseY + BS_Kernel::GetInstance()->GetWindow()->GetClientY());
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::RegisterCharacterCallback(CharacterCallback Callback)
{
	if (find(m_CharacterCallbacks.begin(), m_CharacterCallbacks.end(), Callback) == m_CharacterCallbacks.end())
	{
		m_CharacterCallbacks.push_back(Callback);
		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Tried to register an CharacterCallback that was already registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::UnregisterCharacterCallback(CharacterCallback Callback)
{
	list<CharacterCallback>::iterator CallbackIter = find(m_CharacterCallbacks.begin(), m_CharacterCallbacks.end(), Callback);
	if (CallbackIter != m_CharacterCallbacks.end())
	{
		m_CharacterCallbacks.erase(CallbackIter);
		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Tried to unregister an CharacterCallback that was not previously registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::RegisterCommandCallback(CommandCallback Callback)
{
	if (find(m_CommandCallbacks.begin(), m_CommandCallbacks.end(), Callback) == m_CommandCallbacks.end())
	{
		m_CommandCallbacks.push_back(Callback);
		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Tried to register an CommandCallback that was already registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::UnregisterCommandCallback(CommandCallback Callback)
{
	list<CommandCallback>::iterator CallbackIter = find(m_CommandCallbacks.begin(), m_CommandCallbacks.end(), Callback);
	if (CallbackIter != m_CommandCallbacks.end())
	{
		m_CommandCallbacks.erase(CallbackIter);
		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Tried to unregister an CommandCallback that was not previously registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::ReportCharacter(unsigned char Character)
{
	list<CharacterCallback>::const_iterator CallbackIter = m_CharacterCallbacks.begin();
	while (CallbackIter != m_CharacterCallbacks.end())
	{
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks 
		// invalidiert wird.
		list<CharacterCallback>::const_iterator CurCallbackIter = CallbackIter;
		++CallbackIter;

		(*CurCallbackIter)(Character);
	}
}

// -----------------------------------------------------------------------------

void BS_StdWinInput::ReportCommand(KEY_COMMANDS Command)
{
	list<CommandCallback>::const_iterator CallbackIter = m_CommandCallbacks.begin();
	while (CallbackIter != m_CommandCallbacks.end())
	{
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks 
		// invalidiert wird.
		list<CommandCallback>::const_iterator CurCallbackIter = CallbackIter;
		++CallbackIter;

		(*CurCallbackIter)(Command);
	}
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_StdWinInput::Persist(BS_OutputPersistenceBlock & Writer)
{
	// Anzahl an Command-Callbacks persistieren.
	Writer.Write(m_CommandCallbacks.size());

	// Alle Command-Callbacks einzeln persistieren.
	{
		list<CommandCallback>::const_iterator It = m_CommandCallbacks.begin();
		while (It != m_CommandCallbacks.end())
		{
			Writer.Write(BS_CallbackRegistry::GetInstance().ResolveCallbackPointer(*It));
			++It;
		}
	}

	// Anzahl an Character-Callbacks persistieren.
	Writer.Write(m_CharacterCallbacks.size());

	// Alle Character-Callbacks einzeln persistieren.
	{
		list<CharacterCallback>::const_iterator It = m_CharacterCallbacks.begin();
		while (It != m_CharacterCallbacks.end())
		{
			Writer.Write(BS_CallbackRegistry::GetInstance().ResolveCallbackPointer(*It));
			++It;
		}
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_StdWinInput::Unpersist(BS_InputPersistenceBlock & Reader)
{
	// Command-Callbackliste leeren.
	m_CommandCallbacks.clear();

	// Anzahl an Command-Callbacks lesen.
	unsigned int CommandCallbackCount;
	Reader.Read(CommandCallbackCount);

	// Alle Command-Callbacks wieder herstellen.
	for (unsigned int i = 0; i < CommandCallbackCount; ++i)
	{
		std::string CallbackFunctionName;
		Reader.Read(CallbackFunctionName);

		m_CommandCallbacks.push_back(reinterpret_cast<CommandCallback>(BS_CallbackRegistry::GetInstance().ResolveCallbackFunction(CallbackFunctionName)));
	}

	// Character-Callbackliste leeren.
	m_CharacterCallbacks.clear();

	// Anzahl an Character-Callbacks lesen.
	unsigned int CharacterCallbackCount;
	Reader.Read(CharacterCallbackCount);

	// Alle Character-Callbacks wieder herstellen.
	for (unsigned int i = 0; i < CharacterCallbackCount; ++i)
	{
		std::string CallbackFunctionName;
		Reader.Read(CallbackFunctionName);

		m_CharacterCallbacks.push_back(reinterpret_cast<CharacterCallback>(BS_CallbackRegistry::GetInstance().ResolveCallbackFunction(CallbackFunctionName)));
	}

	return Reader.IsGood();
}
