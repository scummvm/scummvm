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

#if defined(WIN32) && defined(USE_SYSDIALOGS)

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <windows.h>
#include <shlobj.h>
#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif

#include "common/scummsys.h"

#include "backends/dialogs/win32/win32-dialogs.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"
#include "backends/platform/sdl/win32/win32-window.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"
#include "common/translation.h"

Win32DialogManager::Win32DialogManager(SdlWindow_Win32 *window) : _window(window) {
	CoInitialize(NULL);
}

Win32DialogManager::~Win32DialogManager() {
	CoUninitialize();
}

Common::DialogManager::DialogResult Win32DialogManager::showFileBrowser(const char *title, Common::FSNode &choice, bool isDirBrowser) {
	DialogResult result = kDialogError;

	// Do nothing if not running on Windows Vista or later
	if (!Win32::confirmWindowsVersion(6, 0))
		return result;

	IFileOpenDialog *dialog = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
	                              NULL,
	                              CLSCTX_INPROC_SERVER,
	                              IID_PPV_ARGS(&dialog));

	if (SUCCEEDED(hr)) {
		// If in fullscreen mode, switch to windowed mode
		bool wasFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		if (wasFullscreen) {
			g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, false);
			g_system->endGFXTransaction();
		}

		// Customize dialog
		bool showHidden = ConfMan.getBool("gui_browser_show_hidden", Common::ConfigManager::kApplicationDomain);

		DWORD dwOptions;
		hr = dialog->GetOptions(&dwOptions);
		if (SUCCEEDED(hr)) {
			if (isDirBrowser)
				dwOptions |= FOS_PICKFOLDERS;
			if (showHidden)
				dwOptions |= FOS_FORCESHOWHIDDEN;
			hr = dialog->SetOptions(dwOptions);
		}

		LPWSTR str = Win32::ansiToUnicode(title);
		hr = dialog->SetTitle(str);
		delete[] str;

		str = Win32::ansiToUnicode(_("Choose"));
		hr = dialog->SetOkButtonLabel(str);
		delete[] str;

		// Show dialog
		hr = dialog->Show(_window->getHwnd());

		if (SUCCEEDED(hr)) {
			IShellItem *selectedItem = NULL;
			LPWSTR path = NULL;

			// Get the selection from the user
			hr = dialog->GetResult(&selectedItem);

			if (SUCCEEDED(hr)) {
				hr = selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &path);

				if (SUCCEEDED(hr)) {
					char *str = Win32::unicodeToAnsi(path);
					choice = Common::FSNode(str);
					result = kDialogOk;
					CoTaskMemFree(path);
					delete[] str;
				}

				selectedItem->Release();
			}
		} else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
			result = kDialogCancel;
		}

		dialog->Release();

		// If we were in fullscreen mode, switch back
		if (wasFullscreen) {
			g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
			g_system->endGFXTransaction();
		}
	}

	return result;
}

#endif
