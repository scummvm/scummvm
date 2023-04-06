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

#include <cstring>
#include <cstdio>

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/init/nl_parse.h"
#include "watchmaker/extraLS.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/types.h"

namespace Watchmaker {

char *TextBucket, cr = 0;                    //da rimpiazzare con la areatext allocata da fab
static char *CurText;
int SentenceNum = 1, SysSentNum = 1, TooltipSentNum = 1, ObjNameNum = 1, ExtraLSNum = 1;
int DlgItemNum = 1;

uint16 Credits_numRoles = 0;
uint16 Credits_numNames = 0;

/* -----------------24/08/00 9.31--------------------
 *                  LoadExternalText
 * --------------------------------------------------*/
int LoadExternalText(Init *init, char *et) {
	char line[1000];
	int len, num;

	if (!et) return false;
	if (et[0] == '\0') return true;

	auto stream = openFile(et);
	if (!stream)
		return false;

	CurText = TextBucket;
	memset(TextBucket, 0, TEXT_BUCKET_SIZE);
	SentenceNum = SysSentNum = TooltipSentNum = ObjNameNum = ExtraLSNum = 1;

	while (stream->readLine(line, 1000) != nullptr) {
		if ((line[0] == '/') && (line[1] == '/')) continue;

		if ((len = strlen(line)) > 260)
			return ParseError("ExternalText: line too long! curlen %d (MAX 250)\n%s", line - 10, line);

		if (len < 2) continue;

		if (sscanf(&line[5], "%d", &num) < 1)
			return ParseError("ExternalText: sentence number not found in line:\n%s", line);

		switch (line[4]) {
		case 's':
			Sentence[num] = CurText;
			SentenceNum ++;
			break;
		case 'y':
			SysSent[num] = CurText;
			SysSentNum ++;
			break;
		case 't':
			TooltipSent[num] = CurText;
			TooltipSentNum ++;
			break;
		case 'e':
			ExtraLS[num] = CurText;
			ExtraLSNum ++;
			break;
		case 'n':
			ObjName[num] = CurText;
			ObjNameNum ++;
			break;
		case 'd':
			break;
		default:
			return ParseError("ExternalText: unknown paramenters in line:\n%s", line);
		}

		if ((len - 10 - 1) > 0) {
			memcpy(CurText, &line[10], len - 10 - 1);

			switch (line[4]) {
			case 'd':
				strcpy(init->Room[num].desc, CurText);
				break;
			}

			CurText += (len - 10);
		} else {
			switch (line[4]) {
			case 'd':
				strcpy(init->Room[num].desc, "");
				break;
			}

			CurText ++;
		}
	}

	delete stream;

	return true;
}

} // End of namespace Watchmaker
