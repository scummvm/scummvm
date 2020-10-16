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

#ifndef BACKEND_WIN32_DIALOGS_H
#define BACKEND_WIN32_DIALOGS_H

#if defined(WIN32) && defined(USE_SYSDIALOGS)

#include "common/fs.h"
#include "common/dialogs.h"
#include "common/ustr.h"

class SdlWindow_Win32;

class Win32DialogManager final : public Common::DialogManager {
public:
	Win32DialogManager(SdlWindow_Win32 *window);
	virtual ~Win32DialogManager();
	virtual DialogResult showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) override;

private:
	SdlWindow_Win32 *_window;
};

#endif

#endif // BACKEND_WIN32_DIALOGS_H
