/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_LOWLEVEL_PICTURE_H
#define HPL_LOWLEVEL_PICTURE_H

#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/system/String.h"

namespace hpl {

	class iLowLevelPicture
	{
	public:
		iLowLevelPicture(tString asType): msType(asType) {}
		virtual ~iLowLevelPicture(){}

		tString GetType(){return msType;}

		unsigned int GetHeight(){return  mlHeight;}
		unsigned int GetWidth(){return  mlWidth;}
		unsigned int GetDepth(){return  mlDepth;}

		unsigned int GetBpp(){return  mlBpp;}

		virtual bool HasAlpha()=0;

		void SetPath(tString asPath){msPath = asPath;}
		tString GetPath(){return msPath;}
		tString GetFileName(){return cString::GetFileName(msPath);}

	protected:
		unsigned int mlHeight;
		unsigned int mlWidth;
		unsigned int mlDepth;
		unsigned int mlBpp;

	private:
		tString msType;
		tString msPath;
	};

};
#endif // HPL_LOWLEVEL_PICTURE_H
