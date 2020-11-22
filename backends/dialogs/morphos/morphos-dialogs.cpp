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
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_strdup
#include "common/scummsys.h"

#if defined(__MORPHOS__) && defined(USE_SYSDIALOGS)

#include "backends/dialogs/morphos/morphos-dialogs.h"

#include "common/config-manager.h"

#include <proto/exec.h>
#include <proto/dos.h>
#define __NOLIBBASE__
#include <proto/asl.h>
#include <proto/charsets.h>

char *MorphosDialogManager::utf8ToLocal(char *in) {

	if (!in) {
		return strdup("");
	}

	struct Library *CharsetsBase = OpenLibrary("charsets.library", 0);
	if (CharsetsBase) {

		LONG dstmib = GetSystemCharset(NULL, 0);
		if (dstmib != MIBENUM_INVALID) {
			LONG dstlen = GetByteSize((APTR)in, -1, MIBENUM_UTF_8, dstmib);
			char *out = (char *)malloc(dstlen + 1);
			if (out) {
				if (ConvertTagList((APTR)in, -1, (APTR)out, -1, MIBENUM_UTF_8, dstmib, NULL) != -1) {
					return out;
				}
				free(out);
			}
		}
		CloseLibrary(CharsetsBase);
	}

	return strdup(in);
}

Common::DialogManager::DialogResult MorphosDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {

	DialogResult result = kDialogCancel;
	char pathBuffer[PATH_MAX];
	Common::String utf8Title = title.encode();
	struct Library *AslBase = OpenLibrary(AslName, 39);

    if (AslBase) {

		struct FileRequester *fr = NULL;

		if (ConfMan.hasKey("browser_lastpath")) {
			strncpy(pathBuffer, ConfMan.get("browser_lastpath").c_str(), sizeof(pathBuffer) - 1);
		}

		fr = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest, TAG_DONE);

		if (!fr)
			return result;

		char *newTitle = utf8ToLocal((char *)utf8Title.c_str());

		if (AslRequestTags(fr, ASLFR_TitleText, (IPTR)newTitle, ASLFR_RejectIcons, TRUE, ASLFR_InitialDrawer, (IPTR)pathBuffer, ASLFR_DrawersOnly, (isDirBrowser ? TRUE : FALSE), TAG_DONE)) {

			if (strlen(fr->fr_Drawer) < sizeof(pathBuffer)) {
				strncpy(pathBuffer, fr->fr_Drawer, sizeof(pathBuffer));
				if (!isDirBrowser) {
					AddPart(pathBuffer, fr->fr_File, sizeof(pathBuffer));
				}
				choice = Common::FSNode(pathBuffer);
				ConfMan.set("browser_lastpath", pathBuffer);
				result = kDialogOk;
			}
		}

		free(newTitle);
		FreeAslRequest((APTR)fr);
		CloseLibrary(AslBase);
	}

	return result;
}

#endif
