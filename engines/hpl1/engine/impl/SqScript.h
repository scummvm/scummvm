/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_SQ_SCRIPT_H
#define HPL_SQ_SCRIPT_H

#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/system/Script.h"

namespace hpl {

class cSqScript : public iScript {
public:
	cSqScript(const tString &asName, asIScriptEngine *apScriptEngine, int alHandle);
	~cSqScript();

	bool CreateFromFile(const tString &asFileName);

	int GetFuncHandle(const tString &asFunc);
	void AddArg(const tString &asArg);

	bool Run(const tString &asFuncLine);
	bool Run(int alHandle);

private:
	asIScriptEngine *mpScriptEngine;
	asIScriptModule *_module;

	asIScriptContext *mpContext;

	int mlHandle;
	tString msModuleName;

	char *LoadCharBuffer(const tString &asFileName, int &alLength);
};

} // namespace hpl

#endif // HPL_SCRIPT_H
