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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL:  $
 * $Id:  $
 */

#ifndef FS_FACTORY_MAKER_H
#define FS_FACTORY_MAKER_H

#include "backends/factories/abstract-fs-factory.h"

/*
 * All the following includes choose, at compile time, which specific backend will be used
 * during the execution of the ScummVM.
 * 
 * It has to be done this way because not all the necessary libraries will be available in
 * all build environments. Additionally, this results in smaller binaries.
 */
#if defined(__amigaos4__)
	#include "backends/factories/amigaos4/amigaos4-fs-factory.cpp"
#elif defined(__DC__)
	#include "backends/factories/dc/ronincd-fs-factory.cpp"
#elif defined(__DS__)
	#include "backends/factories/ds/ds-fs-factory.cpp"
#elif defined(__GP32__)
	#include "backends/factories/gp32/gp32-fs-factory.cpp"
#elif defined(__MORPHOS__)
	#include "backends/factories/morphos/abox-fs-factory.cpp"
#elif defined(PALMOS_MODE)
	#include "backends/factories/palmos/palmos-fs-factory.cpp"
#elif defined(__PLAYSTATION2__)
	#include "backends/factories/ps2/ps2-fs-factory.cpp"
#elif defined(__PSP__)
	#include "backends/factories/psp/psp-fs-factory.cpp"
#elif defined(__SYMBIAN32__)
	#include "backends/factories/symbian/symbian-fs-factory.cpp"
#elif defined(UNIX)
	#include "backends/factories/posix/posix-fs-factory.cpp"
#elif defined(WIN32)
	#include "backends/factories/windows/windows-fs-factory.cpp"
#endif

/**
 * Creates concrete FilesystemFactory and FileFactory objects depending on the current architecture.
 */
class FilesystemFactoryMaker {
public:

	/**
	 * Returns the correct concrete filesystem factory depending on the current build architecture.
	 */
	static AbstractFilesystemFactory *makeFactory();
	
protected:
	FilesystemFactoryMaker() {}; // avoid instances of this class
};

#endif	//FS_FACTORY_MAKER_H
