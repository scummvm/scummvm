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

#ifndef COMMON_DIALOG_MANAGER_H
#define COMMON_DIALOG_MANAGER_H

#include "common/scummsys.h"
#include "common/fs.h"

#if defined(USE_SYSDIALOGS)

namespace Common {

/**
 * The DialogManager allows GUI code to interact with native system dialogs.
 */
class DialogManager {
public:
	/**
	 * Values representing the user response to a dialog
	 */
	enum DialogResult {
		kDialogError = -1,	///< Dialog couldn't be displayed
		kDialogCancel = 0,	///< User cancelled the dialog (Cancel/No/Close buttons)
		kDialogOk = 1		///< User confirmed the dialog (OK/Yes buttons)
	};

	DialogManager() {}
	virtual ~DialogManager() {}

	/**
	 * Displays a dialog for selecting a file or folder in the filesystem.
	 *
	 * @param title The dialog title
	 * @param choice The path selected by the user
	 * @param isDirBrowser Restrict selection to directories
	 * @return The dialog result
	 */
	virtual DialogResult showFileBrowser(const char *title, FSNode &choice, bool isDirBrowser = false) { return kDialogError; }
};

} // End of namespace Common

#endif

#endif // COMMON_DIALOG_MANAGER_H
