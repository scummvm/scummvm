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

#include "common/config-manager.h"
#include "common/file.h"
#include "mads/madsv2/core/copy.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/popup.h"

namespace MADS {
namespace MADSV2 {

#define NUM_TEXT_LINES          10

extern char global_release_version[10];

void CopyProt::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(manual, page, line, word_number);
	src->read(say, 20);
}


static void copy_mangle(CopyProt *copy_prot) {
	int count;
	byte *dog;

	dog = (byte *)copy_prot;

	for (count = 0; count < sizeof(CopyProt); count++) {
		*dog ^= 0xff;
		dog++;
	}
}


static int copy_load(CopyProt *copy_prot) {
	int error_flag = true;
	int num_items;
	int item;
	int seek_dog;
	Common::SeekableReadStream *handle;

	handle = env_open("*HOGANUS.DAT", "rb");
	if (handle == NULL) goto done;

	num_items = handle->readUint16LE();
	item = imath_random(1, num_items);

	seek_dog = (item - 1) * CopyProt::SIZE;
	if (seek_dog)
		handle->seek(seek_dog, SEEK_CUR);

	copy_prot->load(handle);
	copy_mangle(copy_prot);

	error_flag = false;

done:
	delete handle;
	return error_flag;
}

static int copy_pop_and_ask() {
	int error_flag = COPY_FAIL;
	int result;
	int count;
	long mem_to_get;
	char work_buf[80];
	char page_buf[10];
	char line_buf[10];
	char word_buf[10];
	char entry_buf[80];
	char *read_buf[NUM_TEXT_LINES];
	char *load_buffer = NULL;
	CopyProt copy_prot;
	Common::SeekableReadStream *handle = NULL;

	box_param.erase_on_first = false;  // Allow keep of first letter

	if (copy_load(&copy_prot)) goto finish;

	mem_to_get = 80 * NUM_TEXT_LINES;
	load_buffer = (char *)mem_get(mem_to_get);
	if (load_buffer == NULL) goto finish;

	handle = env_open("*LANGUAGE.DAT", "rb");
	if (handle == NULL) goto finish;

	for (count = 0; count < NUM_TEXT_LINES; count++) {
		read_buf[count] = load_buffer + (count * 80);
		if (!fileio_fread_f(read_buf[count], 80, 1, handle)) goto finish;
	}

	delete handle;
	handle = NULL;

	mads_itoa(copy_prot.page, page_buf, 10);
	mads_itoa(copy_prot.line, line_buf, 10);
	mads_itoa(copy_prot.word_number, word_buf, 10);

	for (count = 0; (count < COPY_TRIES_ALLOWED); count++) {

		if (popup_create(popup_estimate_pieces(26), POPUP_CENTER, POPUP_CENTER)) goto finish;

		if (!count) {
			popup_center_string(read_buf[0], false);
			Common::strcpy_s(work_buf, read_buf[1]);
			Common::strcat_s(work_buf, " ");
			Common::strcat_s(work_buf, global_release_version);
			popup_center_string(work_buf, false);
			popup_next_line();
		} else {
			popup_center_string(read_buf[8], false);
			popup_next_line();
			popup_center_string(read_buf[9], false);
		}
		popup_next_line();

		popup_write_string(read_buf[2]);
		popup_write_string(read_buf[3]);

		Common::strcpy_s(work_buf, page_buf);
		Common::strcat_s(work_buf, read_buf[4]);
		popup_write_string(work_buf);

		Common::strcpy_s(work_buf, line_buf);
		Common::strcat_s(work_buf, read_buf[5]);
		popup_write_string(work_buf);

		Common::strcpy_s(work_buf, word_buf);
		popup_write_string(work_buf);

		popup_write_string(read_buf[6]);
		popup_write_string(read_buf[7]);

		popup_next_line();
		popup_next_line();
		popup_write_string("                      ");
		popup_set_ask();
		popup_next_line();

		entry_buf[0] = copy_prot.say[0];
		entry_buf[1] = 0;

		result = popup_ask_string(entry_buf, 12, true);
		if (result < 0) goto finish;
		if (result > 0) {
			error_flag = COPY_ESCAPE;
			goto done;
		}

		mads_strlwr(entry_buf);
		if (strcmp(entry_buf, copy_prot.say) == 0)
			goto finish;
	}

	goto done;

finish:
	error_flag = COPY_SUCCEED;

done:
	box_param.erase_on_first = true;  // Reset

	delete handle;
	if (load_buffer != NULL)
		mem_free(load_buffer);

	return error_flag;
}

int global_copy_verify() {
	int error_flag = COPY_FAIL;
	int loaded = false;

	if (!ConfMan.getBool("copy_protection"))
		return COPY_SUCCEED;

	if (box_param.series == NULL) {
		Common::strcpy_s(box_param.name, "*BOX");
		if (popup_box_load())
			goto done;
		loaded = true;
	}

	error_flag = copy_pop_and_ask();

	if (loaded) {
		sprite_free(&box_param.menu, true);
		sprite_free(&box_param.logo, true);
		sprite_free(&box_param.series, true);
	}

done:
	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
