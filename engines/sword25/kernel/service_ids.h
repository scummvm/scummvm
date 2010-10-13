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

/*
 * service_ids.h
 * -------------
 * This file lists all the services.
 * EVERY new service needs to be entered here, otherwise it cannot be instantiated
 * by pKernel->NewService(..)
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_SERVICE_IDS
#define SWORD25_SERVICE_IDS

#include "sword25/kernel/common.h"

namespace Sword25 {

Service *GraphicEngine_CreateObject(Kernel *pKernel);
Service *PackageManager_CreateObject(Kernel *pKernel);
Service *InputEngine_CreateObject(Kernel *pKernel);
Service *SoundEngine_CreateObject(Kernel *pKernel);
Service *LuaScriptEngine_CreateObject(Kernel *pKernel);
Service *Geometry_CreateObject(Kernel *pKernel);
#ifdef USE_THEORADEC
Service *OggTheora_CreateObject(Kernel *pKernel);
#else
Service *OggTheora_CreateObject(Kernel *pKernel) { return NULL; }
#endif

// Services are recorded in this table
const BS_ServiceInfo BS_SERVICE_TABLE[] = {
	// The first two parameters are the name of the superclass and service
	// The third parameter is the static method of the class that creates an object
	// of the class and returns it
	// Example:
	// BS_ServiceInfo("Superclass", "Service", CreateMethod)
	BS_ServiceInfo("gfx", "opengl", GraphicEngine_CreateObject),
	BS_ServiceInfo("package", "archiveFS", PackageManager_CreateObject),
	BS_ServiceInfo("input", "winapi", InputEngine_CreateObject),
	BS_ServiceInfo("sfx", "fmodex", SoundEngine_CreateObject),
	BS_ServiceInfo("script", "lua", LuaScriptEngine_CreateObject),
	BS_ServiceInfo("geometry", "std", Geometry_CreateObject),
	BS_ServiceInfo("fmv", "oggtheora", OggTheora_CreateObject),
};

const uint BS_SERVICE_COUNT = sizeof(BS_SERVICE_TABLE) / sizeof(BS_ServiceInfo);

} // End of namespace Sword25

#endif
