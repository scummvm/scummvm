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

#if defined(USE_SYSDIALOGS)

#include "common/fs.h"
#include "common/system.h"
#include "common/events.h"

namespace Common {

/**
 * @defgroup common_dialogs Dialog Manager
 * @ingroup common
 *
 * @brief  The Dialog Manager allows GUI code to interact with native system dialogs.
 *
 * @{
 */

/**
 * The DialogManager allows GUI code to interact with native system dialogs.
 */
class DialogManager {
public:
	/**
	 * Values representing the user response to a dialog.
	 */
	enum DialogResult {
		kDialogError = -1,	///< Dialog could not be displayed.
		kDialogCancel = 0,	///< User cancelled the dialog (Cancel/No/Close buttons).
		kDialogOk = 1		///< User confirmed the dialog (OK/Yes buttons).
	};

	DialogManager() : _wasFullscreen(false) {}
	virtual ~DialogManager() {}

	/**
	 * Display a dialog for selecting a file or folder in the file system.
	 *
	 * @param title        Dialog title.
	 * @param choice       Path selected by the user.
	 * @param isDirBrowser Restrict selection to directories.
	 * @return The dialog result.
	 */
	virtual DialogResult showFileBrowser(const Common::U32String &title, FSNode &choice, bool isDirBrowser = false) { return kDialogError; }

protected:
	bool _wasFullscreen;

	/**
	 * Call before opening a dialog.
	 */
	void beginDialog() {
		// If we were in fullscreen mode, switch back
		_wasFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		if (_wasFullscreen) {
			g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, false);
			g_system->endGFXTransaction();
		}
	}

	/**
	 * Call after closing a dialog.
	 */
	void endDialog() {
		// While the native file browser is open, any input events (e.g. keypresses) are
		// still received by the application. With SDL backend for example this results in the
		// events beeing queued and processed after we return, thus dispatching events that were
		// intended for the native file browser. For example: pressing Esc to cancel the native
		// file browser would cause the application to quit in addition to closing the
		// file browser. To avoid this happening clear all pending events.
		g_system->getEventManager()->getEventDispatcher()->clearEvents();

		// If we were in fullscreen mode, switch back
		if (_wasFullscreen) {
			g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
			g_system->endGFXTransaction();
		}
	}
};

/** @} */

} // End of namespace Common

#endif

#endif // COMMON_DIALOG_MANAGER_H
