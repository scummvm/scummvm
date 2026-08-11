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

#include "bagel/boflib/error.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

/*
THE PICTURE OF DORIAN GRAY
4,13,19,6,5,22,20,11,12,21,15,18,14,7,9,8,16,17,1,2,10,3
THE PICTURE OF DORIAN GRAY
4,13,19,6,5,22,20,11,12,21,15,18,14,7,9,8,16,17,1,2,10,3
*/

#define MAX_PLENGTH    25
#define MAX_PLENGTH_S 30

typedef struct {
	CHAR   text[MAX_PLENGTH_S + 1];
	UBYTE  order[MAX_PLENGTH];
} PHRASES;

INT StrLenNoSpaces(const CHAR *);
ERROR_CODE ValidatePhrase(PHRASES *);

PHRASES curPhrase;


void main(int argc, char *argv[]) {
	char *p, buf[256];
	FILE *infile, *outfile;
	int i, n;

	if (argc > 1) {
		if ((infile = fopen(argv[1], "ra")) != NULL) {
			printf("\nConverting %s...", argv[1]);

			if ((outfile = fopen("OUT.DAT", "wb")) != NULL) {

				while (!feof(infile)) {
					fgets(buf, 255, infile);
					StrReplaceChar(buf, '\n', '\0');
					StrReplaceChar(buf, '\r', '\0');

					printf("\nFound: (%s)\n", buf);

					n = strlen(buf);
					if (n > MAX_PLENGTH_S || n <= 3)
						printf("\nPhrase: (%s) is not valid.  Skipping...\n", buf);
					else {

						memset(&curPhrase, 0, sizeof(PHRASES));

						strcpy(curPhrase.text, buf);

						fgets(buf, 255, infile);
						StrReplaceChar(buf, '\n', '\0');

						n = StrLenNoSpaces(curPhrase.text);
						p = buf;
						StrReplaceChar(p, ',', '\0');
						for (i = 0; i < n; i++) {
							curPhrase.order[i] = atoi(p);
							printf("%d ", curPhrase.order[i]);
							while (*p++ != '\0')
								;
						}
						ValidatePhrase(&curPhrase);
						fwrite(&curPhrase, sizeof(PHRASES), 1, outfile);
					}
				}
				fclose(outfile);
			} else {
				printf("\nCould not open OUT.DAT");
			}

			fclose(infile);
		} else {
			printf("\nCould not open %s", argv[1]);
		}
	}
}

INT StrLenNoSpaces(const CHAR *str) {
	INT len;

	len = 0;
	while (*str) {
		if (*str++ != ' ') {
			len++;
		}
	}
	return (len);
}

ERROR_CODE ValidatePhrase(PHRASES *phrase) {
	bool bList[MAX_PLENGTH];
	INT i, n, order;
	CHAR c;
	ERROR_CODE errCode;

	assert(phrase != NULL);

	ErrorLog("DEBUG.LOG", "Validating %s", phrase->text);

	/* set all entries to FALSE */
	memset(bList, 0, sizeof(bool)*MAX_PLENGTH);

	/* assume no error */
	errCode  = ERR_NONE;

	if ((n = strlen(phrase->text)) > MAX_PLENGTH_S) {
		ErrorLog("DEBUG.LOG", "Phrase too Long: strlen(%s)=%d > %d", phrase->text, n, MAX_PLENGTH_S);
		errCode = ERR_FTYPE;
	} else {

		for (i = 0; i < n; i++) {

			c = phrase->text[i];

			/*
			* verify that all characters in this phrase are valid.
			* valid chars are are '\0', ' ' or a letter
			*/
			if ((c != 0) && (c != 32) && !isalpha(c)) {
				ErrorLog("DEBUG.LOG", "Invalid Char in (%s) %c", phrase->text, c);
				errCode = ERR_FTYPE;
				break;
			}
		}

		/*
		* continues as long as there was no error
		*/
		if (errCode == ERR_NONE) {

			if ((n = StrLenNoSpaces(phrase->text)) > MAX_PLENGTH) {
				ErrorLog("DEBUG.LOG", "StrLenNoSpace(%s)=%d > %d", phrase->text, n, MAX_PLENGTH);
				errCode = ERR_FTYPE;
			} else {

				/*
				* check to make sure that the indexing order values are valid
				*/
				for (i = 0; i < n; i++) {
					order = (INT)phrase->order[i] - 1;

					if ((order >= n) || (order < 0) || bList[order]) {
						ErrorLog("DEBUG.LOG", "Invalid Indexing in %s: %d", phrase->text, phrase->order[i]);
						errCode = ERR_FTYPE;
						break;
					}
					bList[order] = TRUE;
				}
			}
		}
	}

	return errCode;
}

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel
