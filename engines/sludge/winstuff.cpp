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
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

#include "allfiles.h"
#include "debug.h"

#include "winstuff.h"
#include "platform-dependent.h"
#include "language.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "fonttext.h"
#include "backdrop.h"
#include "sludger.h"
#include "cursors.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "direct.h"
#include "sound.h"
#include "colours.h"
#include "moreio.h"
#include "stringy.h"

#include <shellapi.h>
#include <shlobj.h> // For SHGetFolderPath

#include "..\..\images\resource.h"

namespace Sludge {

HINSTANCE hInst;                // Handle of the main instance
extern HWND hMainWindow;

extern variableStack *noStack;

// The platform-specific functions - Windows edition.

WCHAR *ConvertToUTF16(const char *input) {
	int s = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1, NULL, 0);
	WCHAR *ret = new WCHAR [s];
	checkNew(ret);
	/*int a = */MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1, ret, s);
	return ret;
}

char *ConvertFromUTF16(const WCHAR *input) {
	int s = WideCharToMultiByte(CP_UTF8, 0, input, -1, NULL, 0, NULL, NULL);
	char *ret = new char [s];
	checkNew(ret);
	/*int a = */WideCharToMultiByte(CP_UTF8, 0, input, -1, ret, s, NULL, NULL);
	return ret;
}


char *grabFileName() {
	OPENFILENAME ofn;
	WCHAR path[MAX_PATH];
	WCHAR file[MAX_PATH] = TEXT("");

	hInst = GetModuleHandle(NULL);

	memset(& ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = hInst;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrFilter = TEXT("SLUDGE games (*.SLG)\0*.slg\0\0");
	ofn.lpstrFile = file;

	if (GetOpenFileName(& ofn)) {
		return ConvertFromUTF16(file);
	} else {
		return NULL;
	}
}

extern char **languageName;
extern int *languageTable;

HBITMAP hLogo = NULL;
extern unsigned char *gameLogo;

BOOL CALLBACK setupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		if (gameLogo) {
			hLogo = CreateBitmap(310, 88, 1, 32, gameLogo);
			SendDlgItemMessage(hDlg, 1003, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hLogo);
		}

		if (gameSettings.userFullScreen)
			CheckDlgButton(hDlg, 1000, BST_CHECKED);
		else
			CheckDlgButton(hDlg, 1000, BST_UNCHECKED);

		SendDlgItemMessage(hDlg, 1002, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)TEXT("Default (best looking)")));
		SendDlgItemMessage(hDlg, 1002, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)TEXT("Linear (faster but blurry)")));
		SendDlgItemMessage(hDlg, 1002, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)TEXT("Off (blocky graphics)")));

		if (gameSettings.antiAlias < 0)
			SendDlgItemMessage(hDlg, 1002, CB_SETCURSEL, 1, 0);
		else if (gameSettings.antiAlias)
			SendDlgItemMessage(hDlg, 1002, CB_SETCURSEL, 0, 0);
		else
			SendDlgItemMessage(hDlg, 1002, CB_SETCURSEL, 2, 0);

		if (gameSettings.numLanguages) {
			WCHAR text[20];
			for (unsigned int i = 0; i <= gameSettings.numLanguages; i++) {
				if (languageName[i]) {
					WCHAR *w_lang = ConvertToUTF16(languageName[i]);
					SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)w_lang));
					delete w_lang;
				} else {
					swprintf(text, TEXT("Language %d"), i);
					SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)text));
				}
			}
			SendDlgItemMessage(hDlg, 1001, CB_SETCURSEL, getLanguageForFileB(), 0);
		} else {
			const WCHAR *text = TEXT("No translations available");
			SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)text));
			SendDlgItemMessage(hDlg, 1001, CB_SETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hDlg, 1001), false);
		}
		return true;

	case WM_COMMAND:
		if (hLogo) DeleteObject(hLogo);
		switch (LOWORD(wParam)) {
		case IDOK:

			gameSettings.userFullScreen = (IsDlgButtonChecked(hDlg, 1000) == BST_CHECKED);
			gameSettings.antiAlias = SendDlgItemMessage(hDlg, 1002, CB_GETCURSEL, 0, 0);
			if (gameSettings.antiAlias == 0) gameSettings.antiAlias = 1;
			else if (gameSettings.antiAlias == 1) gameSettings.antiAlias = -1;
			else if (gameSettings.antiAlias == 2) gameSettings.antiAlias = 0;

			if (gameSettings.numLanguages) {
				gameSettings.languageID = SendDlgItemMessage(hDlg, 1001, CB_GETCURSEL, 0, 0);
				if (gameSettings.languageID < 0) gameSettings.languageID = 0;
				gameSettings.languageID = languageTable[gameSettings.languageID];
			}
			EndDialog(hDlg, true);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, false);
			return TRUE;
		}
		break;
	}
	return false;
}

int showSetupWindow() {

	hInst = GetModuleHandle(NULL);

	if (! hInst) debugOut("ERROR: No hInst!\n");

	if (DialogBox(hInst, TEXT("SETUPWINDOW"), NULL, setupDlgProc)) return true;
	return false;

}

void msgBox(const char *head, const char *msg) {
	WCHAR *w_head = ConvertToUTF16(head);
	WCHAR *w_msg = ConvertToUTF16(msg);
	MessageBox(NULL, w_msg, w_head, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	delete w_head;
	delete w_msg;
}

int msgBoxQuestion(const char *head, const char *msg) {
	WCHAR *w_head = ConvertToUTF16(head);
	WCHAR *w_msg = ConvertToUTF16(msg);
	int val = MessageBox(NULL, w_msg, w_head, MB_YESNO | MB_SETFOREGROUND | MB_APPLMODAL | MB_ICONQUESTION) == IDNO;
	delete w_head;
	delete w_msg;
	if (val)
		return false;
	return true;
}

void changeToUserDir() {
	TCHAR szAppData[MAX_PATH];
	/*hr = */SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szAppData);
	_wchdir(szAppData);
}

uint32_t launch(char *f) {
	WCHAR *w_f = ConvertToUTF16(f);
	uint32_t r = (uint32_t) ShellExecute(hMainWindow, TEXT("open"), w_f, NULL, TEXT("C:\\"), SW_SHOWNORMAL);
	delete w_f;
	return r;
}

bool defaultUserFullScreen() {
	return true;
}

} // End of namespace Sludge

#endif
