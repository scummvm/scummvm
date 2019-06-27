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

#include "glk/alan3/args.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/options.h"
#include "glk/alan3/sysdep.h"
#include "glk/alan3/utils.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
/* The files and filenames */
char *adventureName;        /* The name of the game */
char *adventureFileName;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#if 0
/*======================================================================*/
char *gameName(char *fullPathName) {
	char *foundGameName = "";

	if (fullPathName != NULL) {
		foundGameName = strdup(baseNameStart(fullPathName));
		foundGameName[strlen(foundGameName) - 4] = '\0'; /* Strip off .A3C */
	}

	if (foundGameName[0] == '.' && foundGameName[1] == '/')
		strcpy(foundGameName, &foundGameName[2]);

	return foundGameName;
}


/*----------------------------------------------------------------------*/
static char *removeQuotes(char *argument) {
	char *str = strdup(&argument[1]);
	str[strlen(str) - 1] = '\0';
	return str;
}


/*----------------------------------------------------------------------*/
static bool isQuoted(char *argument) {
	return argument[0] == '"' && strlen(argument) > 2;
}


/*----------------------------------------------------------------------*/
static char *addAcodeExtension(char *advFilename) {
	if (strlen(advFilename) < strlen(ACODEEXTENSION)
	        || compareStrings(&advFilename[strlen(advFilename) - 4], ACODEEXTENSION) != 0) {
		advFilename = (char *)realloc(advFilename, strlen(advFilename) + strlen(ACODEEXTENSION) + 1);
		strcat(advFilename, ACODEEXTENSION);
	}
	return advFilename;
}



/*----------------------------------------------------------------------*/
static void switches(int argc, char *argv[]) {
	int i;

	for (i = 1; i < argc; i++) {
		char *argument = argv[i];

		if (argument[0] == '-') {
			switch (toLower(argument[1])) {
			case 'i':
				ignoreErrorOption = TRUE;
				break;
			case 't':
				traceSectionOption = TRUE;
				switch (argument[2]) {
				case '9':
				case '8':
				case '7':
				case '6':
				case '5' :
					traceStackOption = TRUE;
				case '4' :
					tracePushOption = TRUE;
				case '3' :
					traceInstructionOption = TRUE;
				case '2' :
					traceSourceOption = TRUE;
				case '\0':
				case '1':
					traceSectionOption = TRUE;
				}
				break;
			case 'd':
				debugOption = TRUE;
				break;
			case 'l':
				transcriptOption = TRUE;
				logOption = FALSE;
				break;
			case 'v':
				verboseOption = TRUE;
				break;
			case 'n':
				statusLineOption = FALSE;
				break;
			case 'c':
				logOption = TRUE;
				transcriptOption = FALSE;
				break;
			case 'r':
				regressionTestOption = TRUE;
				break;
			default:
				printf("Unrecognized switch, -%c\n", argument[1]);
				usage(argv[0]);
				terminate(0);
			}
		} else {

			if (isQuoted(argument))
				adventureFileName = removeQuotes(argument);
			else
				adventureFileName = strdup(argument);

			adventureFileName = addAcodeExtension(adventureFileName);

			adventureName = gameName(adventureFileName);

		}
	}
}
#endif

/*----------------------------------------------------------------------*/
bool differentInterpreterName(char *string) {
	return strcasecmp(string, PROGNAME) != 0;
}

} // End of namespace Alan3
} // End of namespace Glk
