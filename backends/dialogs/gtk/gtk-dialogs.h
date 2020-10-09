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

#ifndef BACKEND_GTK_DIALOGS_H
#define BACKEND_GTK_DIALOGS_H

#if defined(POSIX) && defined(USE_SYSDIALOGS) && defined(USE_GTK)

#include "common/fs.h"
#include "common/dialogs.h"

class GtkDialogManager : public Common::DialogManager {
public:
	virtual DialogResult showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser);
};

#endif

#endif // BACKEND_GTK_DIALOGS_H
