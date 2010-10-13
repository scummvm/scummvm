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

#define BS_LOG_PREFIX "INPUTENGINE"

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/input/inputengine.h"

namespace Sword25 {

#define DOUBLE_CLICK_TIME 500
#define DOUBLE_CLICK_RECT_SIZE 4

InputEngine::InputEngine(Kernel *pKernel) :
	Service(pKernel),
	m_CurrentState(0),
	m_LeftMouseDown(false),
	m_RightMouseDown(false),
	m_MouseX(0),
	m_MouseY(0),
	m_LeftDoubleClick(false),
	m_DoubleClickTime(DOUBLE_CLICK_TIME),
	m_DoubleClickRectWidth(DOUBLE_CLICK_RECT_SIZE),
	m_DoubleClickRectHeight(DOUBLE_CLICK_RECT_SIZE),
	m_LastLeftClickTime(0),
	m_LastLeftClickMouseX(0),
	m_LastLeftClickMouseY(0) {
	memset(m_KeyboardState[0], 0, sizeof(m_KeyboardState[0]));
	memset(m_KeyboardState[1], 0, sizeof(m_KeyboardState[1]));
	m_LeftMouseState[0] = false;
	m_LeftMouseState[1] = false;
	m_RightMouseState[0] = false;
	m_RightMouseState[1] = false;

	if (!registerScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");
}

Service *InputEngine_CreateObject(Kernel *pKernel) {
	return new InputEngine(pKernel);
}

// -----------------------------------------------------------------------------

bool InputEngine::Init() {
	// No initialisation needed
	return true;
}

// -----------------------------------------------------------------------------

void InputEngine::Update() {
	Common::Event event;
	m_CurrentState ^= 1;

	// Loop through processing any pending events
	bool handleEvents = true;
	while (handleEvents && g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
			m_LeftMouseDown = event.type == Common::EVENT_LBUTTONDOWN;
			m_MouseX = event.mouse.x;
			m_MouseY = event.mouse.y;
			handleEvents = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			m_RightMouseDown = event.type == Common::EVENT_RBUTTONDOWN;
			m_MouseX = event.mouse.x;
			m_MouseY = event.mouse.y;
			handleEvents = false;
			break;

		case Common::EVENT_MOUSEMOVE:
			m_MouseX = event.mouse.x;
			m_MouseY = event.mouse.y;
			break;

		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			AlterKeyboardState(event.kbd.keycode, (event.type == Common::EVENT_KEYDOWN) ? 0x80 : 0);
			break;

		case Common::EVENT_QUIT:
			Kernel::GetInstance()->GetWindow()->SetWindowAlive(false);
			break;

		default:
			break;
		}
	}

	m_LeftMouseState[m_CurrentState] = m_LeftMouseDown;
	m_RightMouseState[m_CurrentState] = m_RightMouseDown;

	TestForLeftDoubleClick();
}

// -----------------------------------------------------------------------------

bool InputEngine::IsLeftMouseDown() {
	return m_LeftMouseDown;
}

// -----------------------------------------------------------------------------

bool InputEngine::IsRightMouseDown() {
	return m_RightMouseDown;
}

// -----------------------------------------------------------------------------

void InputEngine::TestForLeftDoubleClick() {
	m_LeftDoubleClick = false;

	// Only bother checking for a double click if the left mouse button was clicked
	if (WasLeftMouseDown()) {
		// Get the time now
		uint Now = Kernel::GetInstance()->GetMilliTicks();

		// A double click is signalled if
		// 1. The two clicks are close enough together
		// 2. The mouse cursor hasn't moved much
		if (Now - m_LastLeftClickTime <= m_DoubleClickTime &&
		        ABS(m_MouseX - m_LastLeftClickMouseX) <= m_DoubleClickRectWidth / 2 &&
		        ABS(m_MouseY - m_LastLeftClickMouseY) <= m_DoubleClickRectHeight / 2) {
			m_LeftDoubleClick = true;

			// Reset the time and position of the last click, so that clicking is not
			// interpreted as the first click of a further double-click
			m_LastLeftClickTime = 0;
			m_LastLeftClickMouseX = 0;
			m_LastLeftClickMouseY = 0;
		} else {
			// There is no double click. Remember the position and time of the click,
			// in case it's the first click of a double-click sequence
			m_LastLeftClickTime = Now;
			m_LastLeftClickMouseX = m_MouseX;
			m_LastLeftClickMouseY = m_MouseY;
		}
	}
}

// -----------------------------------------------------------------------------

void InputEngine::AlterKeyboardState(int keycode, byte newState) {
	m_KeyboardState[m_CurrentState][keycode] = newState;
}

// -----------------------------------------------------------------------------

bool InputEngine::IsLeftDoubleClick() {
	return m_LeftDoubleClick;
}

// -----------------------------------------------------------------------------

bool InputEngine::WasLeftMouseDown() {
	return (m_LeftMouseState[m_CurrentState] == false) && (m_LeftMouseState[m_CurrentState ^ 1] == true);
}

// -----------------------------------------------------------------------------

bool InputEngine::WasRightMouseDown() {
	return (m_RightMouseState[m_CurrentState] == false) && (m_RightMouseState[m_CurrentState ^ 1] == true);
}

// -----------------------------------------------------------------------------

int InputEngine::GetMouseX() {
	return m_MouseX;
}

// -----------------------------------------------------------------------------

int InputEngine::GetMouseY() {
	return m_MouseY;
}

// -----------------------------------------------------------------------------

bool InputEngine::IsKeyDown(uint KeyCode) {
	return (m_KeyboardState[m_CurrentState][KeyCode] & 0x80) != 0;
}

// -----------------------------------------------------------------------------

bool InputEngine::WasKeyDown(uint KeyCode) {
	return ((m_KeyboardState[m_CurrentState][KeyCode] & 0x80) == 0) &&
	       ((m_KeyboardState[m_CurrentState ^ 1][KeyCode] & 0x80) != 0);
}

// -----------------------------------------------------------------------------

void InputEngine::SetMouseX(int PosX) {
	m_MouseX = PosX;
	g_system->warpMouse(m_MouseX, m_MouseY);
}

// -----------------------------------------------------------------------------

void InputEngine::SetMouseY(int PosY) {
	m_MouseY = PosY;
	g_system->warpMouse(m_MouseX, m_MouseY);
}

// -----------------------------------------------------------------------------

bool InputEngine::RegisterCharacterCallback(CharacterCallback Callback) {
	if (Common::find(m_CharacterCallbacks.begin(), m_CharacterCallbacks.end(), Callback) == m_CharacterCallbacks.end()) {
		m_CharacterCallbacks.push_back(Callback);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to register an CharacterCallback that was already registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool InputEngine::UnregisterCharacterCallback(CharacterCallback Callback) {
	Common::List<CharacterCallback>::iterator CallbackIter = Common::find(m_CharacterCallbacks.begin(),
	        m_CharacterCallbacks.end(), Callback);
	if (CallbackIter != m_CharacterCallbacks.end()) {
		m_CharacterCallbacks.erase(CallbackIter);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to unregister an CharacterCallback that was not previously registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool InputEngine::RegisterCommandCallback(CommandCallback Callback) {
	if (Common::find(m_CommandCallbacks.begin(), m_CommandCallbacks.end(), Callback) == m_CommandCallbacks.end()) {
		m_CommandCallbacks.push_back(Callback);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to register an CommandCallback that was already registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool InputEngine::UnregisterCommandCallback(CommandCallback Callback) {
	Common::List<CommandCallback>::iterator CallbackIter =
	    Common::find(m_CommandCallbacks.begin(), m_CommandCallbacks.end(), Callback);
	if (CallbackIter != m_CommandCallbacks.end()) {
		m_CommandCallbacks.erase(CallbackIter);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to unregister an CommandCallback that was not previously registered.");
		return false;
	}
}

// -----------------------------------------------------------------------------

void InputEngine::ReportCharacter(byte Character) {
	Common::List<CharacterCallback>::const_iterator CallbackIter = m_CharacterCallbacks.begin();
	while (CallbackIter != m_CharacterCallbacks.end()) {
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks
		// invalidiert wird.
		Common::List<CharacterCallback>::const_iterator CurCallbackIter = CallbackIter;
		++CallbackIter;

		(*CurCallbackIter)(Character);
	}
}

// -----------------------------------------------------------------------------

void InputEngine::ReportCommand(KEY_COMMANDS Command) {
	Common::List<CommandCallback>::const_iterator CallbackIter = m_CommandCallbacks.begin();
	while (CallbackIter != m_CommandCallbacks.end()) {
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks
		// invalidiert wird.
		Common::List<CommandCallback>::const_iterator CurCallbackIter = CallbackIter;
		++CallbackIter;

		(*CurCallbackIter)(Command);
	}
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool InputEngine::persist(OutputPersistenceBlock &writer) {
	// Anzahl an Command-Callbacks persistieren.
	writer.write(m_CommandCallbacks.size());

	// Alle Command-Callbacks einzeln persistieren.
	{
		Common::List<CommandCallback>::const_iterator It = m_CommandCallbacks.begin();
		while (It != m_CommandCallbacks.end()) {
			writer.write(CallbackRegistry::instance().resolveCallbackPointer(*It));
			++It;
		}
	}

	// Anzahl an Character-Callbacks persistieren.
	writer.write(m_CharacterCallbacks.size());

	// Alle Character-Callbacks einzeln persistieren.
	{
		Common::List<CharacterCallback>::const_iterator It = m_CharacterCallbacks.begin();
		while (It != m_CharacterCallbacks.end()) {
			writer.write(CallbackRegistry::instance().resolveCallbackPointer(*It));
			++It;
		}
	}

	return true;
}

// -----------------------------------------------------------------------------

bool InputEngine::unpersist(InputPersistenceBlock &reader) {
	// Command-Callbackliste leeren.
	m_CommandCallbacks.clear();

	// Anzahl an Command-Callbacks lesen.
	uint CommandCallbackCount;
	reader.read(CommandCallbackCount);

	// Alle Command-Callbacks wieder herstellen.
	for (uint i = 0; i < CommandCallbackCount; ++i) {
		Common::String CallbackFunctionName;
		reader.read(CallbackFunctionName);

		m_CommandCallbacks.push_back(reinterpret_cast<CommandCallback>(
		                                 CallbackRegistry::instance().resolveCallbackFunction(CallbackFunctionName)));
	}

	// Character-Callbackliste leeren.
	m_CharacterCallbacks.clear();

	// Anzahl an Character-Callbacks lesen.
	uint CharacterCallbackCount;
	reader.read(CharacterCallbackCount);

	// Alle Character-Callbacks wieder herstellen.
	for (uint i = 0; i < CharacterCallbackCount; ++i) {
		Common::String CallbackFunctionName;
		reader.read(CallbackFunctionName);

		m_CharacterCallbacks.push_back(reinterpret_cast<CharacterCallback>(CallbackRegistry::instance().resolveCallbackFunction(CallbackFunctionName)));
	}

	return reader.isGood();
}

} // End of namespace Sword25
