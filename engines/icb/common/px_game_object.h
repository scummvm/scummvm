/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_GAME_OBJECT_H
#define ICB_GAME_OBJECT_H

#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

// object run-time status values
enum _object_status {       // possible values of object status field
	OB_STATUS_NOT_HELD, // 0
	OB_STATUS_HELD      // 1 (when an object is held it does not get processed or drawn - it is excluded from the game)
};

#define OB_INIT_SCRIPT 0
#define OB_LOGIC_CONTEXT 1
#define OB_ACTION_CONTEXT 2

typedef struct {
	uint32 m_size; // The size of the total data structure

	uint32 m_var_table_offset;

	uint32 ob_status; // low level internal stuff - see enum _object_status

	// The offsets to the blocks of data. All offsets
	// are from the start of the object

	uint32 m_script_name_hash_table_offset; // Offset to the script name table

	uint32 m_lvars_offset; // Offset to the local variable data
	uint32 m_name_offset;  // Offset to the object name

	// Variable and script count
	uint32 m_noLvars;   // How many lvars this object has
	uint32 m_noScripts; // The number of scripts associated with this object

	/*      This data is then followed by:

			Null terminated object name
			Object variable information block
			Script names information block
	*/
} CGame;

class CGameObject {
	// Only ob_status is made public. To access the other elements use
	// the access functions. This is so that further changes to the structure
	// can be catered for through the access functions, and not needed where
	// any element is specifically referenced

public:
	// Main access functions
	static const char *GetName(CGame *game);      // Get a pointer to the object name
	static uint32 GetNoLvars(CGame *game) ;   // Get the number of local variables
	static uint32 GetNoScripts(CGame *game); // Get the number of scripts
	static uint32 GetSize(CGame *game) { return FROM_LE_32(game->m_size); }

	// Using the hash system you cannot get names, only hashes
	static uint32 GetScriptNameFullHash(CGame *game, uint32);
	static uint32 GetScriptNamePartHash(CGame *game, uint32);

	static const char *GetScriptVariableName(CGame *game, uint32); // gets name
	static int32 GetVariable(CGame *game, const char *name);       // get's number of named variable

	static int32 IsVariableString(CGame *game, uint32); // is variable a string (1=string, 0=int)

	static void SetIntegerVariable(CGame *game, uint32, int32); // Sets the value of an integer variable

	static int32 GetIntegerVariable(CGame *game, uint32); // Get the value of an integer variable
	static int32 *GetIntegerVariablePtr(CGame *game, uint32); // Get the value of an integer variable
	static const char *GetStringVariable(CGame *game, uint32);    // Get the value of a string variable

	static const char *GetStringValueOrDefault(CGame *game, const char *varName, const char *defaultStr) {
		int32 var;
		var = GetVariable(game, varName);
		if (var == -1)
			return defaultStr;
		else
			return GetStringVariable(game, var);
	}

	static int32 GetIntegerValueOrDefault(CGame *game, const char *varName, int32 defaultInt) {
		int32 var;
		var = GetVariable(game, varName);
		if (var == -1)
			return defaultInt;
		else
			return GetIntegerVariable(game, var);
	}
};

inline const char *CGameObject::GetName(CGame *game) {
	// Get a pointer to the object name
	return ((const char *)(((const char *)game) + game->m_name_offset));
}

inline uint32 CGameObject::GetNoLvars(CGame *game) {
	// Get the number of local variables
	return FROM_LE_32(game->m_noLvars);
}

inline uint32 CGameObject::GetNoScripts(CGame *game) {
	// Get the number of scripts
	return FROM_LE_32(game->m_noScripts);
}

inline uint32 CGameObject::GetScriptNameFullHash(CGame *game, uint32 scriptNo) {
	assert(scriptNo < FROM_LE_32(game->m_noScripts));
	return FROM_LE_32(((const int32 *)(((const char *)game) + FROM_LE_32(game->m_script_name_hash_table_offset)))[scriptNo * 2]);
}

inline uint32 CGameObject::GetScriptNamePartHash(CGame *game, uint32 scriptNo) {
	assert(scriptNo < FROM_LE_32(game->m_noScripts));
	return FROM_LE_32(((const int32 *)(((const char *)game) + FROM_LE_32(game->m_script_name_hash_table_offset)))[scriptNo * 2 + 1]);
}

inline const char *CGameObject::GetScriptVariableName(CGame *game, uint32 varNo) {
	const char *currentPos;
	const uint32 *table;

	currentPos = (((const char *)game) + FROM_LE_32(game->m_var_table_offset));
	table = (const uint32 *)currentPos;
	return ((const char *)game) + FROM_LE_32(table[varNo * 2]);
}

inline int32 CGameObject::IsVariableString(CGame *game, uint32 varNo) {
	const char *currentPos;
	const uint32 *table;

	currentPos = (((const char *)game) + FROM_LE_32(game->m_var_table_offset));
	table = (const uint32 *)currentPos;
	return FROM_LE_32(table[varNo * 2 + 1]);
}

inline int32 CGameObject::GetVariable(CGame *game, const char *name) {
	const char *currentPos;
	const uint32 *table;
	int32 retValue;
	uint32 whichVar;

	currentPos = (((const char *)game) + FROM_LE_32(game->m_var_table_offset));
	table = (const uint32 *)currentPos;

	retValue = -1;

	for (whichVar = 0; whichVar < FROM_LE_32(game->m_noLvars); whichVar++) {
		if (strcmp(name, ((const char *)game) + FROM_LE_32(table[whichVar * 2])) == 0) {
			retValue = whichVar;
			whichVar = (int32)FROM_LE_32(game->m_noLvars);
		}
	}

	return retValue;
}

inline void CGameObject::SetIntegerVariable(CGame *game, uint32 lvar, int32 val) {
	assert(lvar < FROM_LE_32(game->m_noLvars));
	uint32 *lvars = (uint32 *)((byte *)game + FROM_LE_32(game->m_lvars_offset));
	WRITE_LE_UINT32(lvars + lvar, (uint32)val);
}

inline int32 CGameObject::GetIntegerVariable(CGame *game, uint32 lvar) {
	// Get an lvar value
	assert(lvar < FROM_LE_32(game->m_noLvars));
	uint32 *lvars = (uint32 *)((byte *)game + FROM_LE_32(game->m_lvars_offset));
	return (int32)READ_LE_UINT32(lvars + lvar);
}

inline int32 *CGameObject::GetIntegerVariablePtr(CGame *game, uint32 lvar) {
	// Get an lvar value
	assert(lvar < FROM_LE_32(game->m_noLvars));
	uint32 *lvars = (uint32 *)((byte *)game + FROM_LE_32(game->m_lvars_offset));
	return (int32 *)(lvars + lvar);
}

inline const char *CGameObject::GetStringVariable(CGame *game, uint32 lvar) {
	// Get an lvar value
	assert(lvar < FROM_LE_32(game->m_noLvars));
	uint32 *lvars = (uint32 *)((byte *)game + FROM_LE_32(game->m_lvars_offset));
	return (const char *)game + (int32)READ_LE_UINT32(lvars + lvar);
}

} // End of namespace ICB

#endif
