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

#ifndef BS_B25SLOADER_H
#define BS_B25SLOADER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "imageloader.h"

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_B25SLoader : public BS_ImageLoader
{
public:
	static BS_ImageLoader * CreateInstance() 
	{ 
		#include "kernel/memlog_off.h"
		return static_cast<BS_ImageLoader *>(new BS_B25SLoader());
		#include "kernel/memlog_on.h"
	}

protected:
	virtual bool IsCorrectImageFormat(const char * FileDataPtr, unsigned int FileSize);
	virtual bool DecodeImage(const char * FileDataPtr, unsigned int FileSize,  BS_GraphicEngine::COLOR_FORMATS ColorFormat, char * & UncompressedDataPtr,
							 int & Width, int & Height, int & Pitch);
	virtual bool ImageProperties(const char * FileDataPtr, unsigned int FileSize, BS_GraphicEngine::COLOR_FORMATS & ColorFormat, int & Width, int & Height);

};

#endif
