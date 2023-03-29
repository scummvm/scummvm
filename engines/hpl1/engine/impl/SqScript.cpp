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

#include "hpl1/engine/impl/SqScript.h"
#include "common/file.h"
#include "hpl1/debug.h"
#include "hpl1/engine/libraries/angelscript/add-ons/scripthelper.h"
#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSqScript::cSqScript(const tString &asName, asIScriptEngine *apScriptEngine, int alHandle)
	: iScript(asName) {
	mpScriptEngine = apScriptEngine;
	mlHandle = alHandle;

	mpContext = mpScriptEngine->CreateContext();

	// Create a unique module name
	msModuleName = "Module_" + cString::ToString(cMath::RandRectl(0, 1000000)) +
				   "_" + cString::ToString(mlHandle);
}

cSqScript::~cSqScript() {
	_module->Discard();
	mpContext->Release();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSqScript::CreateFromFile(const tString &asFileName) {
	int lLength;
	char *pCharBuffer = LoadCharBuffer(asFileName, lLength);
	if (pCharBuffer == nullptr) {
		Error("Couldn't load script '%s'!\n", asFileName.c_str());
		return false;
	}

	_module = mpScriptEngine->GetModule(msModuleName.c_str(), asGM_ALWAYS_CREATE);
	if (_module->AddScriptSection(msModuleName.c_str(), pCharBuffer, lLength) < 0) {
		Error("Couldn't add script '%s'!\n", asFileName.c_str());
		hplDeleteArray(pCharBuffer);
		return false;
	}

	if (_module->Build() < 0) {
		Hpl1::logError(Hpl1::kDebugSaves, "Couldn't build script '%s'!\n", asFileName.c_str());
		hplDeleteArray(pCharBuffer);
		return false;
	}

	hplDeleteArray(pCharBuffer);
	return true;
}

//-----------------------------------------------------------------------

int cSqScript::GetFuncHandle(const tString &asFunc) {
	return _module->GetFunctionByName(asFunc.c_str())->GetId();
}

//-----------------------------------------------------------------------

void cSqScript::AddArg(const tString &asArg) {
}

//-----------------------------------------------------------------------

bool cSqScript::Run(const tString &asFuncLine) {
	ExecuteString(mpScriptEngine, asFuncLine.c_str(), _module);
	return true;
}

//-----------------------------------------------------------------------

bool cSqScript::Run(int alHandle) {
	error("call to unimplemented function cSqScript::run(int)");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

char *cSqScript::LoadCharBuffer(const tString &asFileName, int &alLength) {
	Common::File file;
	file.open(asFileName);
	if (!file.isOpen()) {
		debugCN(Hpl1::kDebugLevelError, Hpl1::kDebugFilePath,
				"script file at %s could not be opened", asFileName.c_str());
		return nullptr;
	}

	alLength = file.size();
	char *pBuffer = hplNewArray(char, alLength);
	file.read(pBuffer, alLength);
	if (file.err()) {
		debugCN(Hpl1::kDebugLevelError, Hpl1::kDebugResourceLoading,
				"error in reading script file %s", asFileName.c_str());
		return nullptr;
	}
	return pBuffer;
}

} // namespace hpl
