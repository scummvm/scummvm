// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

/*
	service_ids.h
	-------------
	In dieser Datei sind alle Services verzeichnet.
	JEDER neuer Service muss hier eingetragen werden, ansonsten kann er nicht mit
	pKernel->NewService(..) instanziiert werden.

	Autor: Malte Thiesen
*/

#include "sword25/kernel/common.h"

BS_Service * BS_OpenGLGfx_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_PhysfsPackageManager_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_StdWinInput_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_FMODExSound_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_LuaScriptEngine_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_Geometry_CreateObject(BS_Kernel* pKernel);
BS_Service * BS_OggTheora_CreateObject(BS_Kernel* pKernel);

// Services müssen in dieser Tabelle eingetragen werden
const BS_ServiceInfo BS_SERVICE_TABLE[] =
{
	// Die ersten beiden Parameter sind die Namen der Superclass und des Services.
	// Der dritte Parameter ist die statische Methode der Klasse, die ein Objekt der Klasse erzeugt und zurückgibt.
	// Beispiel:
	// BS_ServiceInfo("Superclass", "Service", CreateMethod)
	BS_ServiceInfo("gfx", "opengl", BS_OpenGLGfx_CreateObject),
	BS_ServiceInfo("package", "physfs", BS_PhysfsPackageManager_CreateObject),
	BS_ServiceInfo("input", "winapi", BS_StdWinInput_CreateObject),
	BS_ServiceInfo("sfx", "fmodex", BS_FMODExSound_CreateObject),
	BS_ServiceInfo("script", "lua", BS_LuaScriptEngine_CreateObject),
	BS_ServiceInfo("geometry", "std", BS_Geometry_CreateObject),
	BS_ServiceInfo("fmv", "oggtheora", BS_OggTheora_CreateObject),
};

const unsigned int BS_SERVICE_COUNT = sizeof(BS_SERVICE_TABLE) / sizeof(BS_ServiceInfo);
