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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(__amigaos4__) && defined(USE_SYSDIALOGS)

#include "backends/dialogs/amigaos/amigaos-dialogs.h"

#include "common/config-manager.h"

#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/exec.h>

struct AslIFace *IAsl;
struct Library *AslBase;

Common::DialogManager::DialogResult AmigaOSDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {

	char pathBuffer[MAXPATHLEN];

	Common::String newTitle = title.encode(Common::kISO8859_1);

	DialogResult result = kDialogCancel;

	AslBase = IExec->OpenLibrary(AslName, 50);
	if (AslBase) {

		IAsl = (struct AslIFace*)IExec->GetInterface(AslBase, "main", 1, nullptr);

		struct FileRequester *fr = nullptr;

		if (ConfMan.hasKey("browser_lastpath")) {
			strncpy(pathBuffer, ConfMan.get("browser_lastpath").c_str(), sizeof(pathBuffer) - 1);
		}

		fr = (struct FileRequester *)IAsl->AllocAslRequestTags(ASL_FileRequest, TAG_DONE);

		if (!fr)
			return result;

		if (IAsl->AslRequestTags(fr, ASLFR_TitleText, newTitle.c_str(), ASLFR_RejectIcons, TRUE, ASLFR_InitialDrawer, pathBuffer, (isDirBrowser ? TRUE : FALSE), TAG_DONE)) {

			if (strlen(fr->fr_Drawer) < sizeof(pathBuffer)) {
				strncpy(pathBuffer, fr->fr_Drawer, sizeof(pathBuffer));
				if (!isDirBrowser) {
					IDOS->AddPart(pathBuffer, fr->fr_File, sizeof(pathBuffer));
				}
				choice = Common::FSNode(pathBuffer);
				ConfMan.set("browser_lastpath", pathBuffer);
				result = kDialogOk;
			}
			IAsl->FreeAslRequest((APTR)fr);
		}
		AslBase = nullptr;
		IAsl = nullptr;
		IExec->CloseLibrary(AslBase); 
		IExec->DropInterface((struct Interface*)IAsl);
	}

	return result;
}

#endif
