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

#ifndef BACKENDS_UPDATES_SPARKLE_UPDATE_H
#define BACKENDS_UPDATES_SPARKLE_UPDATE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/updates.h"

#if defined(USE_SPARKLE)

class SparkleUpdateManager : public Common::UpdateManager {
public:
	/**
	 * Gets the appcast url.
	 *
	 * Beta/RC versions and releases might have a different appcast url.
	 * This function takes care of checking which version of ScummVM is running
	 * and return the url to the proper appcast.
	 *
	 * @return the appcast url.
	 */
	Common::String getAppcastUrl();
};

#endif

#endif // BACKENDS_UPDATES_SPARKLE_UPDATE_H
