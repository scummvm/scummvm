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
 */

#include "common/scummsys.h"

#ifdef __MORPHOS__

#include "backends/platform/sdl/morphos/morphos.h"
#include "backends/fs/morphos/morphos-fs-factory.h"
#include "backends/dialogs/morphos/morphos-dialogs.h"
#include <proto/openurl.h>

void OSystem_MorphOS::init() {
	// Initialze File System Factory
	_fsFactory = new MorphOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new MorphosDialogManager();
#endif
}

bool OSystem_MorphOS::hasFeature(Feature f) {
	if (f == kFeatureOpenUrl)
		return true;

#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

bool OSystem_MorphOS::openUrl(const Common::String &url) {

	static struct TagItem URLTags[] = {{TAG_DONE, (ULONG) NULL}};

	if (OpenURLBase){
		URL_OpenA((STRPTR)url.c_str(), (struct TagItem*) URLTags);
		return true;
	}

	return false;

}
#endif
