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
#define FORBIDDEN_SYMBOL_EXCEPTION_strdup
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(__amigaos4__) && defined(USE_SYSDIALOGS)

#include "backends/dialogs/amigaos/amigaos-dialogs.h"

#include "common/config-manager.h"

#include <proto/asl.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <cstdlib>
#include <cstring>

struct CodesetsIFace *ICodesets = nullptr;
struct Library *CodesetsBase = nullptr;

char *AmigaOSDialogManager::utf8ToLocal(const char *in) {

	if (!in) {
		return strdup("");
	}

	CodesetsBase = IExec->OpenLibrary("codesets.library", 6);

	if (CodesetsBase) {

		ICodesets = (CodesetsIFace *)IExec->GetInterface(CodesetsBase, "main", 1L, nullptr);

		struct codeset *dstmib = ICodesets->CodesetsFind("UTF-8", CSA_FallbackToDefault, FALSE, TAG_DONE);
		struct codeset *srcmib = ICodesets->CodesetsFind("ISO-8859-1", CSA_FallbackToDefault, FALSE, TAG_DONE);

		if (dstmib != nullptr) {
			ULONG dstlen = 0;
			STRPTR dst_str = ICodesets->CodesetsConvertStr(CSA_SourceCodeset, srcmib, CSA_DestCodeset, dstmib, CSA_Source, in, CSA_DestLenPtr, &dstlen, TAG_DONE);
			if (dst_str!= nullptr) {
				char *out = (char *)malloc(dstlen + 1);
				if (out) {
					strcpy(out, dst_str);
					CodesetsBase = nullptr;
					ICodesets->CodesetsFreeA(dst_str, nullptr);
					ICodesets = nullptr;
					IExec->CloseLibrary(CodesetsBase);
					IExec->DropInterface((struct Interface *)ICodesets);
					return out;
				}
				free(out);
			}
		}
	}
	return strdup(in);
}

struct AslIFace *IAsl;
struct Library *AslBase;

Common::DialogManager::DialogResult AmigaOSDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {

	char pathBuffer[MAXPATHLEN];

	Common::String utf8Title = title.encode();

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

		char *newTitle = utf8ToLocal(utf8Title.c_str());

		if (IAsl->AslRequestTags(fr, ASLFR_TitleText, newTitle, ASLFR_RejectIcons, TRUE, ASLFR_InitialDrawer, pathBuffer, (isDirBrowser ? TRUE : FALSE), TAG_DONE)) {

			if (strlen(fr->fr_Drawer) < sizeof(pathBuffer)) {
				strncpy(pathBuffer, fr->fr_Drawer, sizeof(pathBuffer));
				if (!isDirBrowser) {
					IDOS->AddPart(pathBuffer, fr->fr_File, sizeof(pathBuffer));
				}
				choice = Common::FSNode(pathBuffer);
				ConfMan.set("browser_lastpath", pathBuffer);
				result = kDialogOk;
			}
			free(newTitle);
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
