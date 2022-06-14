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

#if 0
#ifndef HPL_P_BUFFER_H
#define HPL_P_BUFFER_H

#include <GL/GLee.h>
#include <vector>

#include "hpl1/engine/graphics/GraphicsTypes.h"
namespace hpl {
	class iLowLevelGraphics;

	class cPBuffer
	{
	public:
		cPBuffer(iLowLevelGraphics* apLowLevelGraphics,bool abShareObjects,bool abUseMipMaps=false,bool abUseDepth=true,bool abUseStencil=true);
		~cPBuffer();

		bool Init(unsigned int alWidth,unsigned int alHeight, cColor aCol);

		int MakeCurrentContext();

		void Bind();
		void UnBind();

	private:
#ifdef WIN32
		HDC         mDeviceContext;
		HGLRC       mGLContext;
		HPBUFFERARB mPBuffer;
#elif defined(__linux__)
		Display *gDpy;
		GLXContext glCtx;
		GLXPbuffer gPBuffer;
#endif

		int mlWidth;
		int mlHeight;

		bool mbShareObjects;
		iLowLevelGraphics* mpLowLevelGraphics;

		std::vector<int> mvAttribBuffer;
		std::vector<int> mvAttribFormat;
	};

};
#endif // HPL_P_BUFFER_H
#endif
