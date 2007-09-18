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
 * $URL$
 * $Id$
 */

#include "backends/fs/abstract-fs-factory.h"

/*
 * All the following includes choose, at compile time, which specific backend will be used
 * during the execution of the ScummVM.
 * 
 * It has to be done this way because not all the necessary libraries will be available in
 * all build environments. Additionally, this results in smaller binaries.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.cpp"
#elif defined(__DC__)
	#include "backends/fs/dc/ronincd-fs-factory.cpp"
#elif defined(__DS__)
	#include "backends/fs/ds/ds-fs-factory.cpp"
#elif defined(__GP32__)
	#include "backends/fs/gp32/gp32-fs-factory.cpp"
#elif defined(__MORPHOS__)
	#include "backends/fs/morphos/abox-fs-factory.cpp"
#elif defined(PALMOS_MODE)
	#include "backends/fs/palmos/palmos-fs-factory.cpp"
#elif defined(__PLAYSTATION2__)
	#include "backends/fs/ps2/ps2-fs-factory.cpp"
#elif defined(__PSP__)
	#include "backends/fs/psp/psp-fs-factory.cpp"
#elif defined(__SYMBIAN32__)
	#include "backends/fs/symbian/symbian-fs-factory.cpp"
#elif defined(UNIX)
	#include "backends/fs/posix/posix-fs-factory.cpp"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.cpp"
#endif

/**
 * Creates concrete FilesystemFactory objects depending on the current architecture.
 * 
 * @return AbstractFilesystemFactory* The specific factory for the current architecture. 
 */
static AbstractFilesystemFactory *makeFSFactory() {
	#if defined(__amigaos4__)
		return &AmigaOSFilesystemFactory::instance();
	#elif defined(__DC__)
		return &RoninCDFilesystemFactory::instance();
	#elif defined(__DS__)
		return &DSFilesystemFactory::instance();
	#elif defined(__GP32__)
		return &GP32FilesystemFactory::instance();
	#elif defined(__MORPHOS__)
		return &ABoxFilesystemFactory::instance();
	#elif defined(PALMOS_MODE)
		return &PalmOSFilesystemFactory::instance();
	#elif defined(__PLAYSTATION2__)
		return &Ps2FilesystemFactory::instance();
	#elif defined(__PSP__)
		return &PSPFilesystemFactory::instance();
	#elif defined(__SYMBIAN32__)
		return &SymbianFilesystemFactory::instance();
	#elif defined(UNIX)
		return &POSIXFilesystemFactory::instance();
	#elif defined(WIN32)
		return &WindowsFilesystemFactory::instance();
	#endif
}
