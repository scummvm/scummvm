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
 */

#include "backends/updates/sparkle-updates.h"

#if defined(USE_SPARKLE)

// TODO replace by proper URL
#if defined(MACOSX)
	#define APPCAST_BASE_URL "http://www.scummvm.org/appcasts/macosx/"
#elif defined(WIN32)
	#define APPCAST_BASE_URL "http://www.scummvm.org/appcasts/win32/"
#else
	#error "Please define APPCAST_BASE_URL for your platform!"
#endif


Common::String SparkleUpdateManager::getAppcastUrl() {
#ifdef RELEASE_BUILD
	return APPCAST_BASE_URL "release.xml";
#else
	return APPCAST_BASE_URL "beta.xml";
#endif
}

#endif
