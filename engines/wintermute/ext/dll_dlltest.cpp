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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

bool EmulateDLLTestExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// IRC_init
	// Used to connect to debug IRC server at games by Corbomite Games
	// Specification: external "dlltest.dll" cdecl long IRC_init(string)
	// Known usage: IRC_init(<PlayerName>)
	// Known actions:
	//  1. Connect to irc.starchat.net
	//  2. Send "NICK ZU_<PlayerName>/"
	//  3. Send "USER Blah ZbengHost ZbengServer ZbengRealname"
	//  4. Send "Join #Zbeng"
	// Returns 0 on success, other value on error
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "IRC_init") == 0) {
		stack->correctParams(1);
		/*const char *name =*/ stack->pop()->getString();

		// do nothing

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ChangeNick
	// Used to update nick at debug IRC server at games by Corbomite Games
	// Specification: external "dlltest.dll" cdecl long ChangeNick(string)
	// Known usage: ChangeNick(<PlayerName>)
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "ChangeNick") == 0) {
		stack->correctParams(1);
		/*const char *name =*/ stack->pop()->getString();

		// do nothing

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IRC_SendString
	// Used to send debug and chat lines to an IRC server at games by Corbomite Games
	// Specification: external "dlltest.dll" cdecl IRC_SendString(string, string)
	// Known usage: IRC_SendString(<Message>, <Channel>)
	// Known Channel values are: "#Zbeng" and "#ZbengDebug"
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "IRC_SendString") == 0) {
		stack->correctParams(2);
		const char *message = stack->pop()->getString();
		const char *channel = stack->pop()->getString();

		inGame->LOG(0, "IRC logging: [%s] %s", channel, message);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IRC_GetChatStrings
	// Used to get chat lines from an IRC server at games by Corbomite Games
	// Specification: external "dlltest.dll" cdecl IRC_GetChatStrings(string, long)
	// Known usage: IRC_GetChatStrings(<Buffer>, 65535)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "IRC_GetChatStrings") == 0) {
		stack->correctParams(2);
		/*const char *buffer =*/ stack->pop()->getString();
		/*int bufferMaxSize =*/ stack->pop()->getInt();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IRC_quit
	// Used to disconnect from debug IRC server at games by Corbomite Games
	// Specification: external "dlltest.dll" cdecl IRC_quit()
	// Known usage: IRC_quit()
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "IRC_quit") == 0) {
		stack->correctParams(0);

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
