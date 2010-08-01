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

#ifndef SWORD25_LUASCRIPT_H
#define SWORD25_LUASCRIPT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/str.h"
#include "common/str-array.h"
#include "sword25/kernel/common.h"
#include "sword25/script/script.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

namespace {

struct lua_State;

}

namespace Sword25 {

class BS_Kernel;

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_LuaScriptEngine : public BS_ScriptEngine {
public:
	// -----------------------------------------------------------------------------
	// Constructor / Destructor
	// -----------------------------------------------------------------------------

	BS_LuaScriptEngine(BS_Kernel *KernelPtr);
	virtual ~BS_LuaScriptEngine();

	/**
	 * Initialises the scripting engine
	 * @return				Returns true if successful, otherwise false.
	*/
	virtual bool Init();

	/**
	 * Loads a script file and executes it
	 * @param FileName		The filename of the script
	 * @return				Returns true if successful, otherwise false.
	 */
	virtual bool ExecuteFile(const Common::String &FileName);

	/**
	 * Execute a string of script code
	 * @param Code			A string of script code
	 * @return				Returns true if successful, otherwise false.
	 */
	virtual bool ExecuteString(const Common::String &Code);

	/**
	 * Returns a pointer to the main object of the scripting language
	 * @remark				Using this method breaks the encapsulation of the language 
	 */
	virtual void *GetScriptObject() { return m_State; }

	/**
	 * Makes the command line parameters for the scripting environment available
	 * @param CommandLineParameters	An array containing all the command line parameters
	 * @remark				How the command line parameters will be used by scripts is 
	 * dependant on the particular implementation.
	 */
	virtual void SetCommandLine(const Common::StringArray &CommandLineParameters);

	/**
	 * @remark				The Lua stack is cleared by this method
	 */
	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	/**
	 * @remark				The Lua stack is cleared by this method
	 */
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

private:
	::lua_State *m_State;
	int m_PcallErrorhandlerRegistryIndex;

	bool RegisterStandardLibs();
	bool RegisterStandardLibExtensions();
	bool ExecuteBuffer(const char *Data, unsigned int Size, const Common::String &Name) const;
};

} // End of namespace Sword25

#endif
