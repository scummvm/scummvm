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

#ifndef SWORD25_STDWININPUT_H
#define SWORD25_STDWININPUT_H

/// Includes
#include "sword25/kernel/memlog_off.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <list>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/input/inputengine.h"

/// Klassendefinitionen
class BS_Kernel;

/// Klassendefinition
class BS_StdWinInput : public BS_InputEngine
{
public:
	BS_StdWinInput(BS_Kernel* pKernel);
	virtual ~BS_StdWinInput();
	
	virtual bool Init();
	virtual void Update();
	virtual bool IsLeftMouseDown();
	virtual bool IsRightMouseDown();
	virtual bool WasLeftMouseDown();
	virtual bool WasRightMouseDown();
	virtual bool IsLeftDoubleClick();
	virtual int GetMouseX();
	virtual int GetMouseY();
	virtual bool IsKeyDown(unsigned int KeyCode);
	virtual bool WasKeyDown(unsigned int KeyCode);
	virtual void SetMouseX(int PosX);
	virtual void SetMouseY(int PosY);
	virtual bool RegisterCharacterCallback(CharacterCallback Callback);
	virtual bool UnregisterCharacterCallback(CharacterCallback Callback);
	virtual bool RegisterCommandCallback(CommandCallback Callback);
	virtual bool UnregisterCommandCallback(CommandCallback Callback);
	virtual void ReportCharacter(unsigned char Character);
	virtual void ReportCommand(KEY_COMMANDS Command);

	bool Persist(BS_OutputPersistenceBlock & Writer);
	bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	void TestForLeftDoubleClick();

	BYTE							m_KeyboardState[2][256];
	bool							m_LeftMouseState[2];
	bool							m_RightMouseState[2];
	unsigned int					m_CurrentState;
	int								m_MouseX;
	int								m_MouseY;
	bool							m_LeftMouseDown;
	bool							m_RightMouseDown;
	bool							m_LeftDoubleClick;
	unsigned int					m_DoubleClickTime;
	int								m_DoubleClickRectWidth;
	int								m_DoubleClickRectHeight;
	unsigned int					m_LastLeftClickTime;
	int								m_LastLeftClickMouseX;
	int								m_LastLeftClickMouseY;
	std::list<CommandCallback>		m_CommandCallbacks;
	std::list<CharacterCallback>	m_CharacterCallbacks;
};

#endif
