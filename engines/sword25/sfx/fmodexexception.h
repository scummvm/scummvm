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
