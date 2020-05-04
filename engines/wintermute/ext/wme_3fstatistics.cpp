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

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_3fstatistics.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SX3fStatistics, false)

BaseScriptable *makeSX3fStatistics(BaseGame *inGame, ScStack *stack) {
	return new SX3fStatistics(inGame,  stack);
}

//////////////////////////////////////////////////////////////////////////
SX3fStatistics::SX3fStatistics(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(4);

	ScValue * tmp;
	_baseUrl = stack->pop()->getString();
	tmp = stack->pop();
	_chapter = tmp->isNULL() ? "" : tmp->getString();
	tmp = stack->pop();
	_language = tmp->isNULL() ? "" : tmp->getString();
	tmp = stack->pop();
	_buildNum = tmp->isNULL() ? "" : tmp->getString();

	_repeat = 0;

	_gameRef->LOG(0, "new Statistics(\"%s\", \"%s\", \"%s\", \"%s\")", _baseUrl.c_str(), _chapter.c_str(), _language.c_str(), _buildNum.c_str());
}

//////////////////////////////////////////////////////////////////////////
SX3fStatistics::~SX3fStatistics() {
}

//////////////////////////////////////////////////////////////////////////
const char *SX3fStatistics::scToString() {
	return "[statistics object]";
}


//////////////////////////////////////////////////////////////////////////
bool SX3fStatistics::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Send()
	// Known stats to send are "Start a new game" and "--Juego Finalizado--"
	// Known action: send HTTP POST request to _baseUrl
	//
	// Headers:
	//   Accept: */*
	//   User-Agent: Mozilla/4.0
	//   Content-Length: <len>
	//   Accept-Language: en-us
	//   Host: www.soluciones3f.com.ar
	//
	// Body:
	// {
	//   "capitulo": "<_chapter value, e.g. '1'>",
	//   "dispositivo": "<OS family, e.g. 'windows'>",
	//   "hash": "<MD5 of MAC address, e.g. '58cb64ba781ca09f9e9cf8bd51ff0b05' for 44:8a:5b:00:00:00>",
	//   "idioma": "<_language value, e.g. 'ru'>",
	//   "idioma_os": "<OS language code, e.g. 'Russian_Russia.1251'>",
	//   "memoria": "<RAM size in bytes, e.g. '8504971264'>",
	//   "message": "<message value, e.g. 'Start a new game'>",
	//   "procesador": "<CPU model name, see /proc/cpuinfo for examples>",
	//   "resolucion": "<screen resolution, e.g. '1920x1080'>",
	//   "sistema": "<OS version, e.g. 'Microsoft  (build 9200), 64-bit'",
	//   "version": "<_buildNum value, e.g. '1.3.2369'>",
	//   "windowed": "<screen mode windowed flag, e.g. 'yes'>"
	// }
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Send") == 0) {
		stack->correctParams(1);
		const char *message = stack->pop()->getString();
		_gameRef->LOG(0, "Send(\"%s\")", message);

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetRepeat()
	// Known _repeat values are 0 and 60
	// Known action: set timer to send HTTP POST request every _repeat seconds
	// HTTP POST request is the same as with Send(), message is "Tick"
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetRepeat") == 0) {
		stack->correctParams(1);
		_repeat = stack->pop()->getInt();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SX3fStatistics::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("statistics");
		return _scValue;
	}

	else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SX3fStatistics::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SX3fStatistics::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	persistMgr->transferString(TMEMBER(_baseUrl));
	persistMgr->transferString(TMEMBER(_chapter));
	persistMgr->transferString(TMEMBER(_language));
	persistMgr->transferString(TMEMBER(_buildNum));
	persistMgr->transferSint32(TMEMBER(_repeat));
	
	return STATUS_OK;
}

} // End of namespace Wintermute
