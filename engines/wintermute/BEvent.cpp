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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "BGame.h"
#include "BFileManager.h"
#include "BEvent.h"
#include "BParser.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBEvent, false)

//////////////////////////////////////////////////////////////////////////
CBEvent::CBEvent(CBGame *inGame): CBBase(inGame) {
	m_Type = EVENT_NONE;
	m_Script = NULL;
	m_Name = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBEvent::CBEvent(CBGame *inGame, TEventType Type, char *Script): CBBase(inGame) {
	m_Type = Type;
	m_Script = new char [strlen(Script) + 1];
	if (m_Script) strcpy(m_Script, Script);
	m_Name = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBEvent::~CBEvent() {
	delete[] m_Script;
	m_Script = NULL;
	delete[] m_Name;
	m_Name = NULL;
}


//////////////////////////////////////////////////////////////////////////
const char *CBEvent::GetEventName(TEventType Type) {
	switch (Type) {
	case EVENT_INIT:
		return "INIT";
	case EVENT_SHUTDOWN:
		return "SHUTDOWN";
	case EVENT_LEFT_CLICK:
		return "LEFT_CLICK";
	case EVENT_RIGHT_CLICK:
		return "RIGHT_CLICK";
	case EVENT_MIDDLE_CLICK:
		return "MIDDLE_CLICK";
	case EVENT_LEFT_DBLCLICK:
		return "LEFT_DBLCLICK";
	case EVENT_PRESS:
		return "PRESS";
	case EVENT_IDLE:
		return "IDLE";
	case EVENT_MOUSE_OVER:
		return "MOUSE_OVER";
	case EVENT_LEFT_RELEASE:
		return "LEFT_RELEASE";
	case EVENT_RIGHT_RELEASE:
		return "RIGHT_RELEASE";
	case EVENT_MIDDLE_RELEASE:
		return "MIDDLE_RELEASE";

	default:
		return "NONE";
	}
}


//////////////////////////////////////////////////////////////////////////
void CBEvent::SetScript(char *Script) {
	if (m_Script) delete [] m_Script;

	m_Script = new char [strlen(Script) + 1];
	if (m_Script) strcpy(m_Script, Script);
}


//////////////////////////////////////////////////////////////////////////
void CBEvent::SetName(char *Name) {
	if (m_Name) delete [] m_Name;

	m_Name = new char [strlen(Name) + 1];
	if (m_Name) strcpy(m_Name, Name);
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBEvent::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBEvent::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing EVENT file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(EVENT)
TOKEN_DEF(NAME)
TOKEN_DEF(SCRIPT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CBEvent::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(EVENT)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_EVENT) {
			Game->LOG(0, "'EVENT' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_SCRIPT:
			SetScript((char *)params);
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in EVENT definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBEvent::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(m_Script));
	PersistMgr->Transfer(TMEMBER(m_Name));
	PersistMgr->Transfer(TMEMBER_INT(m_Type));

	return S_OK;
}

} // end of namespace WinterMute
