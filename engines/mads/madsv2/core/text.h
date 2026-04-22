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

#ifndef MADS_CORE_TEXT_H
#define MADS_CORE_TEXT_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {


#define TEXT_FILENAME           "*MESSAGES"
#define TEXT_SOURCE             ".TXT"
#define TEXT_COMPILED           ".DAT"
#define TEXT_HEADER             ".H"

#define TEXT_MARKER             "text_"
#define TEXT_COMMENT            ';'
#define TEXT_COMMAND            '['
#define TEXT_CLOSE_COMMAND      ']'

/* Allowable commands in file */
#define TEXT_COMMAND_TITLE      "TITLE"
#define TEXT_COMMAND_CENTER     "CENTER"
#define TEXT_COMMAND_CR         "CR"
#define TEXT_COMMAND_ASK        "ASK"
#define TEXT_COMMAND_WIDTH      "WIDTH"
#define TEXT_COMMAND_VERB       "VERB"
#define TEXT_COMMAND_NOUN1      "NOUN1"
#define TEXT_COMMAND_NOUN2      "NOUN2"
#define TEXT_COMMAND_PREP       "PREP"
#define TEXT_COMMAND_SENTENCE   "SENTENCE"
#define TEXT_COMMAND_BAR        "BAR"
#define TEXT_COMMAND_UNDER      "UNDER"
#define TEXT_COMMAND_DOWN       "DOWN"
#define TEXT_COMMAND_TAB        "TAB"
#define TEXT_COMMAND_INDEX      "INDEX"
#define TEXT_COMMAND_NUMBER     "NUMBER"
#define TEXT_COMMAND_NUMBERX    "NUMBERX"
#define TEXT_COMMAND_CHOICE     "CHOICE"
#define TEXT_COMMAND_XC         "XC"
#define TEXT_COMMAND_X          "X"
#define TEXT_COMMAND_YC         "YC"
#define TEXT_COMMAND_Y          "Y"
#define TEXT_COMMAND_ICON       "ICON"
#define TEXT_COMMAND_ICONC      "ICONC"

#define TEXT_COMMAND_OBJECT             "OBJECT"
#define TEXT_COMMAND_SUBJECT            "SUBJECT"
#define TEXT_COMMAND_DEMONSTRATIVE      "DEMONSTRATIVE"
#define TEXT_COMMAND_CUSTOM             "CUSTOM"

#define TEXT_MAX_INDEX          10

#define TEXT_UPPER_ONLY         0
#define TEXT_LOWER_ONLY         1
#define TEXT_UPPER_AND_LOWER    2
#define TEXT_AS_IS              3

typedef struct {
	word length;
	char text[1];
} Text;


struct TextDirectory {
	int32 id;
	uint32 file_offset;
	word length;

	static constexpr int SIZE = 4 + 4 + 2;
	void load(Common::SeekableReadStream *src) {
		src->readMultipleLE(id, file_offset, length);
	}
};

typedef Text *TextPtr;


extern int text_force_width;                /* Force width externally */
extern int text_default_width;              /* Default width          */
extern int text_width;                      /* Last width used        */

extern int text_default_x;                  /* Force position externally */
extern int text_default_y;
extern int text_force_location;             /* Forcing or optional?      */

extern int text_saves_screen;               /* Text preserves screen     */

extern int text_index[TEXT_MAX_INDEX];      /* Indexed vocab words.      */

extern char text_filename[80];              /* Filename for text_load    */
extern char text_build_filename[80];        /* Filename for text_build   */

extern long text_last_id;                    /* Last text id loaded       */
extern int  text_last_number;                /* Last text number used     */
extern int  text_last_num_entries;           /* Number of entries         */


extern TextPtr text_load(long id);

extern int text_build(int build_header);
extern void text_copy_vocab(char **mark, int vocab_id);
extern int text_show(long id);

} // namespace MADSV2
} // namespace MADS

#endif
