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

#ifndef GPH_H
#define GPH_H

#if defined(GP2XWIZ) || defined(CAANOO)

#include "backends/base-backend.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/platform/sdl/posix/posix.h"
#include "backends/graphics/gph/gph-graphics.h"
#include "backends/events/gph/gph-events.h"

#define __GP2XWIZ__
#define MIXER_DOUBLE_BUFFERING 1

#ifndef PATH_MAX
	#define PATH_MAX 255
#endif

class OSystem_GPH : public OSystem_POSIX {
public:
	/* Platform Setup Stuff */
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);
	void initBackend();
	void quit();
};

#endif
#endif //GPH_H
