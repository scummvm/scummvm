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

bool EmulateHTTPConnectExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// Register
	// Used to register license key online at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// Specification: external "httpconnect.dll" cdecl long Register(string, long, string, long)
	// Known usage: Register(<productId>, 65535, <productKey>, 65535)
	// Known product ID values are: "357868", "353058" and "353006"
	// Known action: HTTP GET http://keygen.corbomitegames.com/keygen/validateKey.php?action=REGISTER&productId=productId&key=productKey
	// Returns 1   on success
	// Returns 0   on firewall error
	// Returns -1  on invalid product key
	// Returns -2  on invalid product ID
	// Returns -3  on expired product key
	// Returns -4  on invalid machine ID
	// Returns -5  on number of installations exceeded
	// Returns -6  on socket error
	// Returns -7  on no internet connection
	// Returns -8  on connection reset
	// Returns -11 on validation temporary unavaliable
	// Returns -12 on validation error
	// For some reason always returns -7 for me in a test game
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "Register") == 0) {
		stack->correctParams(4);
		const char *productId = stack->pop()->getString();
		int productIdMaxLen = stack->pop()->getInt();
		const char *productKey = stack->pop()->getString();
		int productKeyMaxLen = stack->pop()->getInt();

		warning("Register(\"%s\",%d,\"%s\",%d) is not implemented", productId , productIdMaxLen, productKey, productKeyMaxLen);

		stack->pushInt(-7); // "no internet connection" error
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Validate
	// Used to validate something at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// Specification: external "httpconnect.dll" cdecl long Validate()
	// Known usage: Validate()
	// Known action: HTTP GET http://keygen.corbomitegames.com/keygen/validateKey.php?action=VALIDATE&productId=Ar&key=Ar
	// Used only when Debug mode is active or game is started with "INVALID" cmdline parameter
	// For some reason always returns 1 for me in a test game
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "Validate") == 0) {
		stack->correctParams(0);

		// do nothing

		stack->pushInt(1);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SendHTTPAsync
	// Used to send game progress events to server at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// Specification: external "httpconnect.dll" cdecl long SendHTTPAsync(string, long, string, long, string, long)
	// Known usage: SendHTTPAsync("backend.pizzamorgana.com", 65535, <FullURL>, 65535, <Buffer?!>, 65535)
	// FullURL is formed as "http://backend.pizzamorgana.com/event.php?Event=<EventName>&player=<PlayerName>&extraParams=<ExtraParams>&SN=<ProductKey>&Episode=1&GameTime=<CurrentTime>&UniqueID=<UniqueId>"
	// Known EventName values are: "GameStart", "ChangeGoal", "EndGame" and "QuitGame"
	// Known ExtraParams values are: "ACT0", "ACT1", "ACT2", "ACT3", "ACT4", "Ep0FindFood", "Ep0FindCellMenu", "Ep0BroRoom", "Ep0FindKey", "Ep0FindCellMenuKey", "Ep0FindMenuKey", "Ep0FindCell", "Ep0FindMenu", "Ep0OrderPizza", "Ep0GetRidOfVamp", "Ep0GetVampAttention", "Ep0License"
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "SendHTTPAsync") == 0) {
		stack->correctParams(6);
		const char *server = stack->pop()->getString();
		int serverMaxLen = stack->pop()->getInt();
		const char *fullUrl = stack->pop()->getString();
		int fullUrlMaxLen = stack->pop()->getInt();
		const char *param5 = stack->pop()->getString();
		int param5MaxLen = stack->pop()->getInt();

		// TODO: Maybe parse URL and call some Achievements API using ExtraParams values in some late future
		warning("SendHTTPAsync(\"%s\",%d,\"%s\",%d,\"%s\",%d) is not implemented", server, serverMaxLen, fullUrl, fullUrlMaxLen, param5, param5MaxLen);

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SendRecvHTTP (6 params variant)
	// Declared at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// Seems to be unused, probably SendRecvHTTP was initially used instead of SendHTTPAsync
	// Specification: external "httpconnect.dll" cdecl long SendRecvHTTP(string, long, string, long, string, long)
	// Always returns -7 for me in a test game, probably returns the same network errors as Register()
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "SendRecvHTTP") == 0 && function->nu_params == 6) {
		stack->correctParams(6);
		const char *server = stack->pop()->getString();
		int serverMaxLen = stack->pop()->getInt();
		const char *fullUrl = stack->pop()->getString();
		int fullUrlMaxLen = stack->pop()->getInt();
		const char *param5 = stack->pop()->getString();
		int param5MaxLen = stack->pop()->getInt();

		warning("SendRecvHTTP(\"%s\",%d,\"%s\",%d,\"%s\",%d) is not implemented", server, serverMaxLen, fullUrl, fullUrlMaxLen, param5, param5MaxLen);

		stack->pushInt(-7); // "no internet connection" error
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SendRecvHTTP (4 params variant)
	// Used to call HTTP methods at Zbang! The Game
	// Specification: external "httpconnect.dll" cdecl long SendRecvHTTP(string, long, string, long)
	// Known usage: SendRecvHTTP("scoresshort.php?player=<PlayerName>", 65535, <Buffer>, 65535)
	// Known usage: SendRecvHTTP("/update.php?player=<PlayerName>&difficulty=<Difficulty>&items=<CommaSeparatedItemList>", 65535, <Buffer>, 65535)
	// My Zbang demo does not have this dll, so there is no way to actually test it with a test game
	// Return value is never used in Zbang scripts
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "SendRecvHTTP") == 0 && function->nu_params == 4) {
		stack->correctParams(4);
		const char *dirUrl = stack->pop()->getString();
		int dirUrlMaxLen = stack->pop()->getInt();
		/*ScValue *buf =*/ stack->pop();
		int bufMaxLen = stack->pop()->getInt();

		//TODO: Count items and give scores, persist those values
		warning("SendRecvHTTP(\"%s\",%d,buf,%d) is not implemented", dirUrl, dirUrlMaxLen, bufMaxLen);

		stack->pushInt(0);
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
