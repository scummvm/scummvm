/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_strdup
#include "common/scummsys.h"

#if defined(__MORPHOS__) && defined(USE_SYSDIALOGS)

#include "backends/dialogs/morphos/morphos-dialogs.h"

#include "common/config-manager.h"

#include <proto/exec.h>
#include <proto/dos.h>
#define __NOLIBBASE__
#include <proto/asl.h>

Common::DialogManager::DialogResult MorphosDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {

	DialogResult result = kDialogCancel;
	char pathBuffer[PATH_MAX];
	Common::String newTitle = title.encode(Common::kISO8859_1);
	struct Library *AslBase = OpenLibrary(AslName, 39);

	if (AslBase) {
		struct FileRequester *fr = NULL;

		if (ConfMan.hasKey("browser_lastpath")) {
			strncpy(pathBuffer, ConfMan.getPath("browser_lastpath").toString(Common::Path::kNativeSeparator).c_str(), sizeof(pathBuffer) - 1);
		}

		fr = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest, TAG_DONE);
		if (!fr)
			return result;

		if (AslRequestTags(fr, ASLFR_TitleText, (IPTR)newTitle.c_str(), ASLFR_RejectIcons, TRUE, ASLFR_InitialDrawer, (IPTR)pathBuffer, ASLFR_DrawersOnly, (isDirBrowser ? TRUE : FALSE), TAG_DONE)) {
			if (strlen(fr->fr_Drawer) < sizeof(pathBuffer)) {
				strncpy(pathBuffer, fr->fr_Drawer, sizeof(pathBuffer));
				ConfMan.setPath("browser_lastpath", pathBuffer); // only path
				if (!isDirBrowser) {
					AddPart(pathBuffer, fr->fr_File, sizeof(pathBuffer));
				}
				choice = Common::FSNode(pathBuffer);
				result = kDialogOk;
			}
		}
		FreeAslRequest((APTR)fr);
		CloseLibrary(AslBase);
	}
	return result;
}
#endif
