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

#ifndef BACKENDS_UPDATES_WIN32_H
#define BACKENDS_UPDATES_WIN32_H

#include "common/scummsys.h"

#if defined(WIN32) && defined(USE_SPARKLE)

#include "common/updates.h"

class SdlWindow_Win32;

class Win32UpdateManager final : public Common::UpdateManager {
public:
	Win32UpdateManager(SdlWindow_Win32 *window);
	virtual ~Win32UpdateManager();

	virtual void checkForUpdates() override;

	virtual void setAutomaticallyChecksForUpdates(UpdateState state) override;
	virtual UpdateState getAutomaticallyChecksForUpdates() override;

	virtual void setUpdateCheckInterval(int interval) override;
	virtual int getUpdateCheckInterval() override;

	virtual bool getLastUpdateCheckTimeAndDate(TimeDate &t) override;

private:
	static int canShutdownCallback();
	static void shutdownRequestCallback();
};

#endif

#endif // BACKENDS_UPDATES_WIN32_H
