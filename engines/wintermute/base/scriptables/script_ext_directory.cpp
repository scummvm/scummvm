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

#include "engines/wintermute/base/scriptables/script_ext_directory.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/persistent.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


IMPLEMENT_PERSISTENT(SXDirectory, true)

BaseScriptable *makeSXDirectory(BaseGame *inGame) {
	return new SXDirectory(inGame);
}

//////////////////////////////////////////////////////////////////////////
SXDirectory::SXDirectory(BaseGame *inGame) : BaseScriptable(inGame) {

}


//////////////////////////////////////////////////////////////////////////
SXDirectory::~SXDirectory() {

}


//////////////////////////////////////////////////////////////////////////
bool SXDirectory::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Create
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Create") == 0) {
		stack->correctParams(1);
		const char *dirName = stack->pop()->getString();

		if (strcmp(dirName, "saves") == 0) {
			// Known games that do this: alphapolaris, hamlet, papasdaughters1, papasdaughters2, polechudes
			// No need to actually create anything, files will be stored at SavefileManager
			stack->pushBool(true);
		} else {
			// No currently known games need this
			warning("Directory.Create is not implemented! Returning false...");
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delete") == 0) {
		stack->correctParams(1);
		stack->pop()->getString();

		// No currently known games need this
		warning("Directory.Delete is not implemented! Returning false...");

		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFiles / GetDirectories
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFiles") == 0 || strcmp(name, "GetDirectories") == 0) {
		stack->correctParams(2);
		const char *dirName = stack->pop()->getString();
		stack->pop()->getString();

		stack->pushInt(0);
		BaseScriptable *array = makeSXArray(_gameRef, stack);

		// used in secret scene of "Art of Murder 1: FBI Confidential"
		if (strcmp(dirName, "X:\\FBI\\data\\scenes\\17-magic\\") == 0 && strcmp(name, "GetDirectories") == 0) {
			//TODO: return list of "scenes\17-magic" subfolders from data.dcp
			warning("Directory.%s is not implemented! Returning empty array...", name);
		} else {
			// No currently known games need this
			warning("Directory.%s is not implemented! Returning empty array...", name);
		}

 		stack->pushNative(array, false);
 		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDrives
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDrives") == 0) {
		stack->correctParams(0);

		// No currently known games do this
		warning("Directory.GetDrives is not implemented! Returning empty array...");

		stack->pushInt(0);
 		stack->pushNative(makeSXArray(_gameRef, stack), false);
 		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXDirectory::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("directory");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PathSeparator
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PathSeparator") {
		_scValue->setString("\\");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentDirectory
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CurrentDirectory") {
		_scValue->setString("."); // See also: BaseGame::scGetProperty("SaveDirectory")
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TempDirectory
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TempDirectory") {
		_scValue->setString("temp"); // See also: BaseGame::scGetProperty("SaveDirectory")
		return _scValue;
	} else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXDirectory::persist(BasePersistenceManager *persistMgr) {

	BaseScriptable::persist(persistMgr);
	return STATUS_OK;
}

} // End of namespace Wintermute
