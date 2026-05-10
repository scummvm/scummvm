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

#ifndef MADS_CORE_QUAL_H
#define MADS_CORE_QUAL_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define qual_version "3.01"
#define qual_date    "28-Oct-91"


struct QualBuf {
	word id;
	word pointer;
};
typedef struct QualBuf Qual;


#define QU_MAXWORDS             800     /* Maximum words in vocabulary */

#define QU_MAXWORDLEN           20      /* Maximum word length */

#define QU_FUDGEFACTOR          50      /* Number of words slack space */

#define QU_MAINFILENAME         "*QUAL.DB"

#define QU_ERR_OPENMAINFILE     -1      /* Failed to open main file  */
#define QU_ERR_OPENHARDFILE     -2      /* Failed to open hard file  */
#define QU_ERR_READMAINFILE     -3      /* Read error on main file   */
#define QU_ERR_READHARDFILE     -4      /* Read error on hard file   */
#define QU_ERR_WORDTOOLONG      -5      /* Max. word length exceeded */
#define QU_ERR_SYNTAXHARDFILE   -6      /* Improper format hard file */
#define QU_ERR_NOMOREMEMORY     -7      /* Failed to allocate memory */
#define QU_ERR_MEMORYOVERFLOW   -8      /* No memory to add a word   */
#define QU_ERR_WORDALREADYEXISTS -9     /* Word alread in vocab list */
#define QU_ERR_WRITEMAINFILE    -10     /* Failed to write main file */
#define QU_ERR_RENAMEOUTFILE    -11     /* Failed to rename out file */
#define QU_ERR_TOOMANYWORDS     -12     /* QU_MAXWORDS exceeded      */
#define QU_ERR_NOSUCHWORD       -13     /* Word not in list          */

extern int qual_allocation;
extern int qual_words;
extern char *qual;

int        qual_destroy(void);

int        qual_load(int allocation_flag);
int        qual_get_code(char *inp);
char *qual_get_word(char *out, int inp);

int        qual_write_file(char *last_word);
int        qual_add_word(char *inp);

void       qual_report_error(int number);

char *qual_select_word(char *out,
	char *prompt,
	char *default_word);

void       qual_maint(void);

int        qual_build(void);

void       qual_unload_active(void);
void       qual_init_active(void);
int        qual_active_id(word id);
int        qual_make_active(word id);
int        qual_load_active(void);

extern char *qual_text;
extern word qual_active;

} // namespace MADSV2
} // namespace MADS

#endif
