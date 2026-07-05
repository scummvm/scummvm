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

#include "common/textconsole.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/dialog.h"
#include "mads/madsv2/core/mouse.h"

namespace MADS {
namespace MADSV2 {

#define background      colorbyte(white, blue)

char *vocab;
int vocab_words = 0;
int vocab_allocation = 0;
int vocab_first_soft = 0;
int vocab_longest = 0;

char *vocab_text = NULL;
word vocab_size;

int vocab_destroy() {
	int result;

	if (vocab_allocation > 0) {
		mem_free(vocab);
		vocab_allocation = 0;
		vocab_words = 0;
		vocab_first_soft = 0;
		vocab = NULL;
		result = true;
	} else {
		result = false;
	}

	return result;
}

static int vocab_count(Common::SeekableReadStream *handle) {
	int  read_error;
	int  read_count;

	read_error = 0;
	read_count = 0;

	while (!(handle->eos() || read_error)) {
		Common::String line = handle->readLine();
		const char *temp_buf = line.c_str();

		if ((temp_buf[0] != '/') && (temp_buf[0] != '*') && (temp_buf[0] != ';')) {
			read_count++;
		} else {
			if (!handle->eos())
				read_error = true;
		}
	}

	if (read_error) read_count = -1;

	return read_count;
}

int vocab_load(int allocation_flag) {
	Common::SeekableReadStream *handle;
	char temp_buf[80];
	char *comma_find;
	int result;
	int mylen;
	int more_words;
	int vocab_space;
	int vocab_address;

	result = 0;

	if (vocab_allocation == 0) {

		vocab = NULL;

		handle = env_open(VC_HARDFILENAME, "rt");

		if (handle != NULL) {

			vocab_words = vocab_count(handle);
			vocab_longest = 0;
			delete handle;

			if (vocab_words >= 0) {
				handle = env_open(VC_MAINFILENAME, "rt");

				if (handle != NULL) {

					more_words = vocab_count(handle);
					delete handle;

					if (more_words >= 0) {
						vocab_words += more_words;

					} else {
						result = VC_ERR_READMAINFILE;
					}
				}
			} else {
				result = VC_ERR_READHARDFILE;
			}
		} else {
			result = VC_ERR_OPENHARDFILE;
		}
	} else {
		result = vocab_words;
	}

	if (result == 0) {
		if (allocation_flag) {
			vocab_allocation = VC_MAXWORDS;
		} else {
			vocab_allocation = vocab_words + VC_FUDGEFACTOR;
		}
		vocab_space = ((VC_MAXWORDLEN + 1) * vocab_allocation);
		vocab = (char *)mem_get(vocab_space);

		if (vocab != NULL) {
			vocab_words = 0;

			handle = env_open(VC_HARDFILENAME, "rt");

			while (!handle->eos() && (result == 0)) {
				Common::String line = handle->readLine();
				Common::strcpy_s(temp_buf, line.c_str());

				if ((temp_buf[0] != '/') && (temp_buf[0] != '*') && (temp_buf[0] != ';')) {
					fileio_fix_lf_input(temp_buf);
					fileio_purge_trailing_spaces(temp_buf);
					comma_find = strchr(temp_buf, ',');
					if (comma_find != NULL) {
						comma_find++;
						if (strlen(comma_find) <= VC_MAXWORDLEN) {
							vocab_address = ((VC_MAXWORDLEN + 1) * vocab_words);
							Common::strcpy_s((vocab + vocab_address), 65536, comma_find);
							vocab_words++;
						} else {
							result = VC_ERR_WORDTOOLONG;
						}
					} else {
						result = VC_ERR_SYNTAXHARDFILE;
					}
				}
			}
			delete handle;

			if (result == 0) {

				vocab_first_soft = vocab_words + 1;

				handle = env_open(VC_MAINFILENAME, "rt");

				while (!handle->eos() && (result == 0)) {
					Common::String line = handle->readLine();
					Common::strcpy_s(temp_buf, line.c_str());

					if ((temp_buf[0] != '*') && (temp_buf[0] != '/') && (temp_buf[0] != ';')) {
						fileio_fix_lf_input(temp_buf);
						fileio_purge_trailing_spaces(temp_buf);
						mylen = strlen(temp_buf);
						if (mylen <= VC_MAXWORDLEN) {
							if (mylen > vocab_longest) vocab_longest = mylen;
							vocab_address = ((VC_MAXWORDLEN + 1) * vocab_words);
							Common::strcpy_s((vocab + vocab_address), 65536, temp_buf);
							vocab_words++;
						} else {
							result = VC_ERR_WORDTOOLONG;
						}
					}
				}

				delete handle;
				if (result == 0) result = vocab_words;
			}
		} else {
			result = VC_ERR_NOMOREMEMORY;
		}
	}

	if ((result < 0) && (vocab != NULL)) mem_free(vocab);

	return result;
}

int vocab_get_code(char *my_word) {
	int result;
	int vocab_count, vocab_address;
	char temp_buf[40];

	Common::strcpy_s(temp_buf, my_word);
	fileio_purge_trailing_spaces(temp_buf);

	if (vocab_allocation == 0) {
		result = vocab_load(false);
		if (result >= 0) result = 0;
	} else {
		result = 0;
	}

	if (result == 0) {
		for (vocab_count = vocab_address = 0;
			(vocab_count < vocab_words) && (!result); vocab_count++) {
			if (strcmp((vocab + vocab_address), temp_buf) == 0) {
				result = vocab_count + 1;
			} else {
				vocab_address += (VC_MAXWORDLEN + 1);
			}
		}
	}
	return result;
}

char *vocab_get_word(char *word_buf, int word_code) {
	char *result;
	int load_check;
	int vocab_address;

	if (vocab_allocation == 0) {
		load_check = vocab_load(false);
	} else {
		load_check = 0;
	}

	if (load_check >= 0) {
		if (word_code <= vocab_words) {
			vocab_address = ((word_code - 1) * (VC_MAXWORDLEN + 1));
			Common::strcpy_s(word_buf, 65536, (vocab + vocab_address));
			result = word_buf;
		} else {
			result = NULL;
		}
	} else {
		result = NULL;
	}

	return result;
}

void vocab_report_error(int number) {
	char temp_buf_1[80], temp_buf_2[80];
	int dos_flag;

	dos_flag = false;

	switch (number) {

	case VC_ERR_OPENMAINFILE:
		Common::strcpy_s(temp_buf_1, "Failed to open main vocabulary file:");
		env_get_path(temp_buf_2, VC_MAINFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_OPENHARDFILE:
		Common::strcpy_s(temp_buf_1, "Failed to open hardcode vocab file:");
		env_get_path(temp_buf_2, VC_HARDFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_READMAINFILE:
		Common::strcpy_s(temp_buf_1, "Read error on main vocabulary file:");
		env_get_path(temp_buf_2, VC_MAINFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_READHARDFILE:
		Common::strcpy_s(temp_buf_1, "Read error on hardcode vocab file:");
		env_get_path(temp_buf_2, VC_HARDFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_WRITEMAINFILE:
		Common::strcpy_s(temp_buf_1, "Write error on main vocabulary file:");
		env_get_path(temp_buf_2, VC_MAINFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_WORDTOOLONG:
		Common::sprintf_s(temp_buf_1, "Word too long.  Max length is %d.", VC_MAXWORDLEN);
		break;

	case VC_ERR_SYNTAXHARDFILE:
		Common::strcpy_s(temp_buf_1, "Syntax error in hardcode vocab file.");
		break;

	case VC_ERR_NOMOREMEMORY:
		Common::strcpy_s(temp_buf_1, "Insufficient memory to load vocab list.");
		break;

	case VC_ERR_MEMORYOVERFLOW:
		Common::strcpy_s(temp_buf_1, "No more room for flying vocab list insert.");
		break;

	case VC_ERR_WORDALREADYEXISTS:
		Common::strcpy_s(temp_buf_1, "Word already in vocabulary list.");
		break;

	case VC_ERR_RENAMEOUTFILE:
		Common::strcpy_s(temp_buf_1, "Failed to rename outfile:");
		env_get_path(temp_buf_2, VC_MAINFILENAME);
		dos_flag = true;
		break;

	case VC_ERR_TOOMANYWORDS:
		Common::sprintf_s(temp_buf_1, "Max # of vocab words (%d) exceeded.", VC_MAXWORDS);
		break;

	case VC_ERR_NOSUCHWORD:
		Common::strcpy_s(temp_buf_1, "Word not in list.");
		break;

	}

	if (dos_flag) {
		dialog_alert_ok(temp_buf_1, temp_buf_2, "Error", NULL);
	} else {
		dialog_alert_ok(temp_buf_1, NULL, NULL, NULL);
	}
}

char *vocab_select_word(char *out, const char *prompt, const char *default_word) {
	dialog_declare(dialog);
	char *result;
	int vocab_address, vocab_howmany;
	int vocab_rows, vocab_columns;
	int load_flag;
	ItemPtr word_item;

	if (vocab_allocation == 0) {
		load_flag = vocab_load(false);
		if (load_flag < 0) {
			vocab_report_error(load_flag);
		}
	} else {
		load_flag = 0;
	}

	if (vocab_longest < 8) vocab_longest = 8;
	vocab_columns = 64 / (vocab_longest + 1);
	vocab_rows = screen_max_y - 18;

	vocab_address = (vocab_first_soft - 1) * (VC_MAXWORDLEN + 1);
	vocab_howmany = (vocab_words - vocab_first_soft) + 1;

	if (load_flag == 0) {

		dialog = dialog_create_default();

		if (dialog != NULL) {

			dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, prompt);
			dialog_add_blank(dialog);

			word_item = dialog_add_listbased(dialog, DD_IX_LEFT, DD_IY_AUTOFILL,
				"~Search: ", default_word, VC_MAXWORDLEN,
				"~Vocabulary List:", vocab + vocab_address,
				vocab_howmany, VC_MAXWORDLEN + 1,
				vocab_longest, vocab_rows, vocab_columns);

			dialog_buttons(dialog);
			dialog_exec(dialog, word_item);

			if (result_item == cancel_item) {
				result = NULL;
			} else {
				Common::strcpy_s(out, 65536, dialog_read_list(dialog, word_item));
				result = out;
			}

			dialog_destroy(dialog);

		} else {
			vocab_report_error(VC_ERR_NOMOREMEMORY);
			result = NULL;
		}
	} else {
		result = NULL;
	}

	return result;
}

void vocab_sort() {
	char temp_buf[VC_MAXWORDLEN + 2];
	int any_changes;
	int vocab_count, vocab_howmany;
	int vocab_address_1, vocab_address_2;

	vocab_howmany = (vocab_words - vocab_first_soft) + 1;

	any_changes = (vocab_howmany > 1);

	while (any_changes) {
		any_changes = false;

		for (vocab_count = vocab_first_soft; (vocab_count < vocab_words) && (!any_changes); vocab_count++) {
			vocab_address_1 = (vocab_count - 1) * (VC_MAXWORDLEN + 1);
			vocab_address_2 = vocab_address_1 + VC_MAXWORDLEN + 1;
			if (strcmp(vocab + vocab_address_1, vocab + vocab_address_2) > 0) {
				Common::strcpy_s(temp_buf, vocab + vocab_address_1);
				Common::strcpy_s(vocab + vocab_address_1, 65536, vocab + vocab_address_2);
				Common::strcpy_s(vocab + vocab_address_2, 65536, temp_buf);
				any_changes = true;
			}
		}
	}
}

static bool check_for_caps(const char *string) {
	bool any_caps = false;
	const char *mark;

	for (mark = string; *mark; mark++) {
		any_caps |= Common::isUpper(*mark);
	}

	return any_caps;
}

void vocab_unload_active() {
	vocab_size = 0;
	if (vocab_text != NULL) {
		mem_free(vocab_text);
		vocab_text = NULL;
	}
}

int vocab_load_active() {
	int error_flag = true;
	int vocab_error = 0;
	Common::SeekableReadStream *handle = NULL;

	mem_last_alloc_loader = MODULE_VOCAB_LOADER;

	vocab_unload_active();

	handle = env_open("*VOCAB.DAT", "rb");
	if (handle == NULL) {
		vocab_error = 1;
		goto done;
	}

	vocab_size = (word)env_get_file_size(handle);

	vocab_text = (char *)mem_get_name(vocab_size, "$vocab$");
	if (vocab_text == NULL) {
		vocab_error = 2;
		goto done;
	}

	if (!fileio_fread_f(vocab_text, vocab_size, 1, handle)) {
		vocab_error = 3;
		goto done;
	}

	error_flag = false;

done:
	delete handle;

	if (error_flag) {
		vocab_unload_active();
		error_report(ERROR_KERNEL_NO_VOCAB, SEVERE, MODULE_KERNEL, vocab_error, 0);
	}

	return error_flag;
}

int vocab_make_active(int id) {
	return id;
}

char *vocab_string(int vocab_id) {
	char *p = vocab_text;
	int bx = vocab_id - 1;

	if (bx != 0) {
		// Search for the bx-th null terminator to find the bx-th string.
		// repne scasb scans forward decrementing cx; if cx hits zero before
		// finding the null, the id is out of range and we fall back to the
		// first string.
		int cx = vocab_size;
		while (bx > 0) {
			// Find next null terminator
			while (cx > 0 && *p != '\0') {
				p++;
				cx--;
			}
			if (cx == 0)
				return vocab_text;  // aint_got_it: out of range, return first
			p++;  // step past the null
			cx--;
			bx--;
		}
	}

	return p;  // got_it
}

void init_vocab() {
	vocab = NULL;
	vocab_words = 0;
	vocab_allocation = 0;
	vocab_first_soft = 0;
	vocab_longest = 0;
	vocab_text = NULL;
	vocab_size = 0;
}

} // namespace MADSV2
} // namespace MADS
