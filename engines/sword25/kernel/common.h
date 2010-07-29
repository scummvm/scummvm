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
	common.h
	-----------
	Diese Datei enthält Funktionen und Makros, die im gesamten Projekt bekannt sein müssen.
	Daher ist es äußerst wichtig, dass diese Headerdatei in jede andere Headerdatei des Projektes
	eingefügt wird.

	Autor: Malte Thiesen
*/

#ifndef BS_COMMON_H
#define BS_COMMON_H

// Globale Konstanten
#if _DEBUG && !DEBUG
	#define DEBUG
#endif

#define BS_ACTIVATE_LOGGING	// Wenn definiert, wird Logging aktiviert

// Engine Includes
#include "memleaks.h"
#include "log.h"

#ifdef DEBUG
#define BS_ASSERT(EXP) \
	if (!(EXP)) \
	{ \
		BS_Log::Log("!!ASSERTION FAILED!! - FILE: %s - LINE: %d.\n", __FILE__, __LINE__); \
		__asm { int 3 }; \
	}
#else
#define BS_ASSERT(EXP) do { (void)(EXP); } while(0) 
#endif

#endif
