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

#ifndef SWORD25_FMODEXEXCEPTION_H
#define SWROD25_FMODEXEXCEPTION_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FMODExException
{
public:
	BS_FMODExException(const char * Function_, FMOD_RESULT Result_) :
		Function(Function_),
		Result(Result_) {}

	  const char *	Function;
	  FMOD_RESULT	Result;

	  void Log()
	  {
		  BS_LOG_ERROR("Call to %s failed.", Function);
		  BS_LOGLN(" FMOD error: %s(%d)", FMOD_ErrorString(Result), Result);
	  }
};

#endif
