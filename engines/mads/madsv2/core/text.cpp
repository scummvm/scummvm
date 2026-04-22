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

#include "common/debug.h"
#include "common/memstream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/global.h"

namespace MADS {
namespace MADSV2 {

char text_filename[80] = TEXT_FILENAME;  /* Filename to load from  */
long text_last_id = 0;              /* Last ID used           */
int  text_last_number = 0;              /* Last number used       */
int  text_last_num_entries = 0;          /* Num entries last time  */

char text_build_filename[80] = TEXT_FILENAME;

int  text_force_width = 0;              /* Force width externally */
int  text_default_width = 17;             /* Default width          */
int  text_width = 17;             /* Last width used        */

int text_default_x = POPUP_CENTER;    /* Force position externally */
int text_default_y = POPUP_CENTER;
int text_force_location = false;           /* Forcing mandatory?        */

int text_saves_screen = true;            /* Text preserves screen     */

int text_index[TEXT_MAX_INDEX];
int text_capitalization = TEXT_UPPER_ONLY;

char text_command_title[6] = TEXT_COMMAND_TITLE;
char text_command_center[7] = TEXT_COMMAND_CENTER;
char text_command_cr[3] = TEXT_COMMAND_CR;
char text_command_ask[4] = TEXT_COMMAND_ASK;
char text_command_width[6] = TEXT_COMMAND_WIDTH;
char text_command_verb[5] = TEXT_COMMAND_VERB;
char text_command_noun1[6] = TEXT_COMMAND_NOUN1;
char text_command_noun2[6] = TEXT_COMMAND_NOUN2;
char text_command_prep[5] = TEXT_COMMAND_PREP;
char text_command_sentence[9] = TEXT_COMMAND_SENTENCE;
char text_command_bar[4] = TEXT_COMMAND_BAR;
char text_command_under[6] = TEXT_COMMAND_UNDER;
char text_command_down[5] = TEXT_COMMAND_DOWN;
char text_command_tab[4] = TEXT_COMMAND_TAB;
char text_command_index[6] = TEXT_COMMAND_INDEX;
char text_command_number[7] = TEXT_COMMAND_NUMBER;
char text_command_numberx[8] = TEXT_COMMAND_NUMBERX;
char text_command_xc[3] = TEXT_COMMAND_XC;
char text_command_x[2] = TEXT_COMMAND_X;
char text_command_yc[3] = TEXT_COMMAND_YC;
char text_command_y[2] = TEXT_COMMAND_Y;
char text_command_iconc[6] = TEXT_COMMAND_ICONC;
char text_command_icon[5] = TEXT_COMMAND_ICON;
char text_command_choice[7] = TEXT_COMMAND_CHOICE;

char text_command_object[7] = TEXT_COMMAND_OBJECT;
char text_command_subject[8] = TEXT_COMMAND_SUBJECT;
char text_command_custom[7] = TEXT_COMMAND_CUSTOM;
char text_command_demonstrative[14] = TEXT_COMMAND_DEMONSTRATIVE;

extern int16 global[GLOBAL_LIST_SIZE];


TextPtr text_load(long id) {
	TextPtr result = NULL;
	char temp_buf[80];
	word num_entries;
	long file_offset;
	long target_offset;
	int count;
	bool found = false;
	Common::SeekableReadStream *handle = NULL;
	TextDirectory dir;

	dir.id = dir.file_offset = dir.length = 0;
	mem_last_alloc_loader = MODULE_TEXT_LOADER;

	Common::strcpy_s(temp_buf, text_filename);
	Common::strcat_s(temp_buf, TEXT_COMPILED);

	handle = env_open(temp_buf, "rb");
	if (handle == NULL) goto done;

	file_offset = handle->pos();

	num_entries = handle->readUint16LE();

	for (count = 0; count < num_entries && !found; ++count) {
		// Iterate over reading table elements
		dir.load(handle);
		found = dir.id == id;
	}

	target_offset = file_offset + dir.file_offset;

	result = (TextPtr)mem_get_name(dir.length + sizeof(word), "$text$");
	if (result == NULL) goto done;

	result->length = dir.length;

	fileio_setpos(handle, target_offset);

	pack_strategy = PACK_PFAB;
	if (pack_data(PACK_EXPLODE, dir.length,
		FROM_DISK, handle,
		TO_MEMORY, &result->text[0]) != (int)dir.length) {
		mem_free(result);
		result = NULL;
	}

done:
	delete handle;
	return result;
}

void text_append(char **text_ptr, char *new_text) {
	int going = true;

	while (going) {
		*((*text_ptr)++) = *new_text;
		going = *new_text;
		new_text++;
	}
}

static int _fatoi(const char *string) {
	char temp_buf[40];
	Common::strcpy_s(temp_buf, string);

	return atoi(temp_buf);
}

static void text_copy_string(char **mark, char *string) {
	Common::strcpy_s(*mark, 65536, string);

	switch (text_capitalization) {
	case TEXT_AS_IS:
		break;
	case TEXT_UPPER_ONLY:
		mads_strupr(*mark);
		break;
	case TEXT_LOWER_ONLY:
		mads_strlwr(*mark);
		break;
	case TEXT_UPPER_AND_LOWER:
	default:
		mads_strupr(*mark);
		mads_strlwr((*mark) + 1);
		break;
	}

	while (**mark)
		(*mark)++;
}


static void text_copy_vocab(char **mark, int vocab_id, char *alternative) {
	char *my_string;

	if (vocab_id > 0) {
		my_string = vocab_string(vocab_id);
		if (!strlen(my_string)) my_string = alternative;
	} else {
		my_string = alternative;
	}

	text_copy_string(mark, my_string);
}

static int text_popup_check(int *popup_created, word width, int x, int y, SeriesPtr icon, int icon_id, int icon_center) {
	int error_flag = false;

	if (!*popup_created) {
		error_flag = popup_create(popup_estimate_pieces(width), x, y);
		*popup_created = !error_flag;

		if (!error_flag) {
			if (icon != NULL) {
				popup_add_icon(icon, icon_id, icon_center);
			}
		}
	}

	return error_flag;
}

static int text_compare(char *text1, char *text2, char **text3) {
	int len;
	int success;

	len = strlen(text2);

	if (text3 != NULL) {
		*text3 = text1 + len;
	}

	success = !scumm_strnicmp(text1, text2, len);

	if (success) {
		if (Common::isUpper(*text1) && Common::isUpper(*(text1 + 1))) {
			text_capitalization = TEXT_UPPER_ONLY;
		} else if (Common::isUpper(*text1)) {
			text_capitalization = TEXT_UPPER_AND_LOWER;
		} else {
			text_capitalization = TEXT_LOWER_ONLY;
		}
	}

	return success;
}

static void text_noun(char **source, char **dest, int noun_id) {
	int syntax;
	int vowel = false;
	char *finder;
	char *finder_2;
	char temp_buf[256];

	syntax = (noun_id != 2) ? player.main_syntax : player.second_syntax;

	// Check if a simple substitution of word
	if (**source != ':') {
		if (syntax >= SYNTAX_SINGULAR_MASC) text_capitalization = TEXT_AS_IS;
		text_copy_vocab(dest, player2.words[noun_id], (noun_id != 2) ? text_command_noun1 : text_command_noun2);
		goto done;
	}

	(*source)++;

	finder = vocab_string(noun_id);
	if (finder != NULL) {
		switch (toupper(*finder)) {
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
			vowel = true;
			break;
		}
	}

	// Check for subject pronoun
	if (!scumm_strnicmp(*source, text_command_subject, strlen(text_command_subject))) {
		text_copy_string(dest, istring_subject_pronoun[syntax]);
		goto done;
	}

	// Check for object pronoun
	if (!scumm_strnicmp(*source, text_command_object, strlen(text_command_object))) {
		text_copy_string(dest, istring_object_pronoun[syntax]);
		goto done;
	}

	// Check for demonstrative
	if (!scumm_strnicmp(*source, text_command_demonstrative, strlen(text_command_demonstrative))) {
		text_copy_string(dest, istring_demonstrative[syntax]);
		goto done;
	}

	// Try for a custom substitution
	if (scumm_strnicmp(*source, text_command_custom, strlen(text_command_custom))) {
		goto done;
	}

	text_capitalization = TEXT_AS_IS;

	do {
		finder = strchr(*source, ':');
		if (finder != NULL) {
			*source = finder + 1;
		}

		if (syntax >= 0) syntax--;
	} while (syntax >= 0);

	Common::strcpy_s(temp_buf, *source);

	finder = strchr(temp_buf, ':');
	if (finder != NULL) *finder = 0;

	finder_2 = strchr(temp_buf, TEXT_CLOSE_COMMAND);
	if (finder_2 != NULL) *finder_2 = 0;

	if (vowel) {
		if (!strcmp(temp_buf, "a")) {
			Common::strcpy_s(temp_buf, "an");
		}
		if (!strcmp(temp_buf, "a ")) {
			Common::strcpy_s(temp_buf, "an ");
		}
	}
	text_copy_string(dest, temp_buf);

done:
	;
}



static void text_choice(char **source, char **dest) {
	int index;
	int index_value;
	char *finder;
	char *finder_2;
	char temp_buf[256];

	text_capitalization = TEXT_AS_IS;

	Common::strcpy_s(temp_buf, *source);
	index = atoi(temp_buf);

	index_value = text_index[index];

	do {
		finder = strchr(*source, ':');
		if (finder != NULL) {
			*source = finder + 1;
		}

		if (index_value) index_value--;
	} while (index_value);

	Common::strcpy_s(temp_buf, *source);

	finder = strchr(temp_buf, ':');
	if (finder != NULL) *finder = 0;

	finder_2 = strchr(temp_buf, TEXT_CLOSE_COMMAND);
	if (finder_2 != NULL) *finder_2 = 0;

	text_copy_string(dest, temp_buf);
}



static void text_icon(char *my_text, SeriesPtr *icon, int *id) {
	char name[80];
	char *mark;
	int idd;

	while (*my_text == ' ') my_text++;
	if (*my_text != '=') goto done;
	my_text++;
	while (*my_text == ' ') my_text++;

	mark = strchr(my_text, ',');
	if (mark == NULL) {
		Common::strcpy_s(name, my_text);
		*id = 1;
	} else {
		*mark = 0;
		Common::strcpy_s(name, my_text);
		*mark = ',';
		mark++;
		*id = _fatoi(mark);
	}


	if (strcmp(name, "*logo.ss")) {

		idd = object_named(player_main_noun);

		if (idd == 9) {  // if == polystone
			if (global[128] != -1) {  // and it's imitating, make icon == what it's imitating
				Common::strcpy_s(name, "*OB0");
				env_catint(name, global[128], 2);
				Common::strcat_s(name, ".ss");
			}
		}
	}

	*icon = sprite_series_load(name, PAL_MAP_RESERVED);
	kernel_new_palette();


done:
	;
}

int text_show(long id) {
	int error_flag = true;
	int center = false;
	int cr;
	int tab;
	int brackets_on = false;
	int popup_created = false;
	int underline = false;
	int index;
	int count;
	int x, y;
	char temp_buf[256];
	char work1[40];
	char work2[40];
	char command_buf[80];
	char *more;
	char *mark;
	char *cmd;
	char *scan;
	TextPtr text = NULL;
	SeriesPtr icon = NULL;
	int icon_id = 0;
	int icon_center = false;

	x = text_default_x;
	y = text_default_y;

	text_capitalization = TEXT_UPPER_ONLY;

	text_width = (text_force_width > 0) ? text_force_width : text_default_width;

	text = text_load(id);
	if (text == NULL) goto done;

	scan = text->text;

	while ((scan - ((char *)text->text)) < (int)text->length) {
		mark = temp_buf;
		cmd = command_buf;
		center = false;
		cr = false;
		tab = false;
		underline = false;
		while (*scan) {
			if (*scan == TEXT_COMMAND) {
				brackets_on = true;
				cmd = command_buf;
			} else if (*scan == TEXT_CLOSE_COMMAND) {
				if (brackets_on) {
					*cmd = 0;
					// mads_strupr(command_buf);
					if (text_compare(command_buf, text_command_center, &more)) {
						center = true;
					} else if (text_compare(command_buf, text_command_title, &more)) {
						center = true;
						cr = true;
						underline = true;
						if (!text_force_width) {
							if (_fatoi(more) > 0) {
								text_width = _fatoi(more);
							}
						}
					} else if (text_compare(command_buf, text_command_cr, &more)) {
						if (center) {
							cr = true;
						} else {
							if (text_popup_check(&popup_created, text_width, x, y, icon, icon_id, icon_center)) goto done;
							*mark = 0;
							mark = temp_buf;
							popup_write_string(temp_buf);
							popup_next_line();
						}
					} else if (text_compare(command_buf, text_command_ask, &more)) {
						popup_set_ask();
					} else if (text_compare(command_buf, text_command_verb, &more)) {
						text_copy_vocab(&mark, player2.words[0], text_command_verb);
					} else if (text_compare(command_buf, text_command_index, &more)) {
						index = _fatoi(more);
						if (text_index[index] > 0) {
							mads_itoa(index, work1, 10);
							Common::strcpy_s(work2, "<");
							Common::strcat_s(work2, text_command_index);
							Common::strcat_s(work2, work1);
							Common::strcat_s(work2, ">");
							text_copy_vocab(&mark, text_index[index], work2);
						}
					} else if (text_compare(command_buf, text_command_numberx, &more)) {
						index = _fatoi(more);
						mads_itoa(text_index[index], work1, 10);
						work2[0] = 0;
						for (count = 0; count < (int)(4 - strlen(work1)); count++) {
							Common::strcat_s(work2, "0");
						}
						Common::strcat_s(work2, work1);
						Common::strcpy_s(mark, 65536, work2);
						while (*mark) mark++;
					} else if (text_compare(command_buf, text_command_number, &more)) {
						index = _fatoi(more);
						mads_itoa(text_index[index], work1, 10);
						Common::strcpy_s(mark, 65536, work1);
						while (*mark) mark++;
					} else if (text_compare(command_buf, text_command_noun1, &more)) {
						text_noun(&more, &mark, 1);
					} else if (text_compare(command_buf, text_command_noun2, &more)) {
						text_noun(&more, &mark, 2);
					} else if (text_compare(command_buf, text_command_prep, &more)) {
						Common::strcpy_s(mark, 65536, istring_prep_names[player.prep]);
						while (*mark) mark++;
					} else if (text_compare(command_buf, text_command_sentence, &more)) {
						Common::strcpy_s(mark, 65536, player.sentence);
						mads_strupr(mark);
						while (*mark) mark++;
					} else if (text_compare(command_buf, text_command_width, &more)) {
						if (!text_force_width) {
							text_width = _fatoi(more);
						}
					} else if (text_compare(command_buf, text_command_bar, &more)) {
						popup_bar();
					} else if (text_compare(command_buf, text_command_under, &more)) {
						underline = true;
					} else if (text_compare(command_buf, text_command_down, &more)) {
						popup_downpixel();
					} else if (text_compare(command_buf, text_command_tab, &more)) {
						popup_tab(_fatoi(more));
					} else if (text_compare(command_buf, text_command_xc, &more)) {
						x = _fatoi(more) | POPUP_CENTER;
					} else if (text_compare(command_buf, text_command_x, &more)) {
						x = _fatoi(more);
					} else if (text_compare(command_buf, text_command_yc, &more)) {
						y = _fatoi(more) | POPUP_CENTER;
					} else if (text_compare(command_buf, text_command_y, &more)) {
						y = _fatoi(more);
					} else if (text_compare(command_buf, text_command_iconc, &more)) {
						text_icon(more, &icon, &icon_id);
						icon_center = true;
					} else if (text_compare(command_buf, text_command_icon, &more)) {
						text_icon(more, &icon, &icon_id);
						icon_center = false;
					} else if (text_compare(command_buf, text_command_choice, &more)) {
						text_choice(&more, &mark);
					}
				}
				brackets_on = false;
			} else {
				if (brackets_on) {
					*(cmd++) = *scan;
				} else {
					*(mark++) = *scan;
				}
			}
			scan++;
		}
		*mark = 0;
		scan++;

		if (text_force_location) {
			x = text_default_x;
			y = text_default_y;
		}

		if (text_popup_check(&popup_created, text_width, x, y, icon, icon_id, icon_center)) goto done;

		if (center) {
			popup_center_string(temp_buf, underline);
			if (cr) popup_next_line();
		} else {
			popup_write_string(temp_buf);
		}
	}

	if (!center) popup_next_line();

	if (popup_and_wait(text_saves_screen)) goto done;

	error_flag = false;

done:
	if (icon != NULL) {
		sprite_free(&icon, (sprite_force_memory == NULL));
	}
	if (error_flag && popup_created) popup_destroy();
	if (text != NULL)  mem_free(text);

	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
