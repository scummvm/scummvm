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

#ifndef HPL_LOWLEVELSYSTEM_SDL_H
#define HPL_LOWLEVELSYSTEM_SDL_H

#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//------------------------------------------------------

class cScriptOutput // : public  asIOutputStream
{
public:
	cScriptOutput() : msMessage("") {}
	~cScriptOutput() {}

	void AddMessage(const asSMessageInfo *msg);
	void Display();
	void Clear();

private:
	tString msMessage;
};

//------------------------------------------------------

class cLowLevelSystemSDL : public iLowLevelSystem {
public:
	cLowLevelSystemSDL();
	~cLowLevelSystemSDL();

	void SetWindowCaption(const tString &asName);

	unsigned long GetTime();
	cDate GetDate();

	iScript *CreateScript(const tString &asName);

	bool AddScriptFunc(const tString &asFuncDecl, void *pFunc, int callConv);
	bool AddScriptVar(const tString &asVarDecl, void *pVar);

	void Sleep(const unsigned int alMillisecs);

private:
	asIScriptEngine *mpScriptEngine;
	cScriptOutput *mpScriptOutput;
	int mlHandleCount;
};

//------------------------------------------------------

};     // namespace hpl
#endif // HPL_LOWLEVELSYSTEM_SDL_H
