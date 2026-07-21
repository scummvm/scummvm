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
#include "mads/core/env.h"
#include "mads/core/font.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/imath.h"
#include "mads/nebular/copy.h"
#include "mads/nebular/popup.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

#define COPY_TRIES_ALLOWED      2

#define COPY_LENGTH             4096
#define COPY_CHECK              256

struct CopyProt {
	char manual;
	int16 page;
	int16 line;
	int16 word_number;
	char say[20];

	static constexpr size_t SIZE = 2 + (3 * 2) + 20;
	void load(Common::SeekableReadStream *src) {
		manual = src->readByte();
		src->skip(1);
		src->readMultipleLE(page, line, word_number);
		src->read(say, 20);
	}
};


static void copy_mangle(byte copy_prot[CopyProt::SIZE]) {
	int count;
	byte *dog;

	dog = (byte *)copy_prot;

	for (count = 0; count < CopyProt::SIZE; count++) {
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
	if (handle == NULL)
		goto done;

	num_items = handle->readUint16LE();
	item = imath_random(1, num_items);

	seek_dog = (item - 1) * CopyProt::SIZE;
	handle->seek(seek_dog, SEEK_CUR);

	// Read in the entry and de-mangle it
	byte buffer[CopyProt::SIZE];
	handle->read(buffer, CopyProt::SIZE);
	copy_mangle(buffer);

	// Load the buffer into the copy protection record
	{
		Common::MemoryReadStream src(buffer, CopyProt::SIZE);
		copy_prot->load(&src);
	}

	error_flag = false;

done:
	delete handle;
	return error_flag;
}

static int copy_pop_and_ask() {
	int error_flag = COPY_FAIL;
	int result;
	int count;
	char work_buf[80];
	char page_buf[10];
	char line_buf[10];
	char word_buf[10];
	char entry_buf[80];
	CopyProt copy_prot;

	// Allow keep of first letter
	box_param.erase_on_first = false;

	if (copy_load(&copy_prot)) goto finish;

	mads_itoa(copy_prot.page, page_buf, 10);
	mads_itoa(copy_prot.line, line_buf, 10);
	mads_itoa(copy_prot.word_number, word_buf, 10);

	for (count = 0; (count < COPY_TRIES_ALLOWED); count++) {
		if (popup_create(32, POPUP_CENTER, POPUP_CENTER))
			goto finish;

		if (!count) {
			Common::strcpy_s(work_buf, "REX NEBULAR version ");
			Common::strcat_s(work_buf, global_release_version);
			popup_center_string(work_buf, true);
			popup_write_string("\n");
			popup_center_string("(Copy Protection, for your convenience)", false);
		} else {
			popup_center_string("ANSWER INCORRECT!", true);
			popup_write_string("\n");
			popup_center_string("(But we'll give you another chance!)", false);
		}
		popup_write_string("\n");

		Common::strcpy_s(work_buf, "Now comes the part that everybody hates.  But if we don't");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "do this, nasty rodent-like people will pirate this game,");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "and a whole generation of talented designers, programmers,");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "artists, and playtesters will go hungry, and will wander");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "aimlessly through the land at night searching for peace.");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "So let's grit our teeth and get it over with.  Just get");
		popup_write_string(work_buf);

		Common::strcpy_s(work_buf, "out your copy of ");
		if (copy_prot.manual == 'g') {
			Common::strcat_s(work_buf, "the GAME MANUAL");
		} else {
			Common::strcat_s(work_buf, "REX'S LOGBOOK");
		}
		Common::strcat_s(work_buf, ".  See!  That was easy.  ");
		popup_write_string(work_buf);

		Common::strcpy_s(work_buf, "Next, just turn to page ");
		Common::strcat_s(work_buf, page_buf);
		Common::strcat_s(work_buf, ".  On line ");
		Common::strcat_s(work_buf, line_buf);
		Common::strcat_s(work_buf, ", find word number ");
		Common::strcat_s(work_buf, word_buf);
		Common::strcat_s(work_buf, ", ");
		popup_write_string(work_buf);

		Common::strcpy_s(work_buf, "and type it on the line below (we've even given you");
		popup_write_string(work_buf);
		Common::strcpy_s(work_buf, "first letter as a hint).  As soon as you do that, we can get");
		popup_write_string(work_buf);
		popup_write_string("right into this really COOL adventure game!\n");

		popup_write_string("\n");
		popup_write_string("                    ");
		popup_set_ask();
		popup_write_string("\n");

		entry_buf[0] = copy_prot.say[0];
		entry_buf[1] = '\0';

		result = popup_ask_string(entry_buf, 12, true);
		if (result < 0) goto finish;
		if (result > 0) {
			error_flag = COPY_ESCAPE;
			goto done;
		}

		mads_strlwr(entry_buf);
		if (strcmp(entry_buf, copy_prot.say) == 0) goto finish;
	}

	goto done;

finish:
	error_flag = COPY_SUCCEED;

done:
	//popup_vomitation_flag = true; /* Reset */

	return error_flag;
}

int global_copy_verify() {
	if (!ConfMan.getBool("copy_protection"))
		return COPY_SUCCEED;

	// Note: the original did some extra work at this point to see whether the user's boot sector
	// remained the same, and if so, bypass the copy protection check. And even if there wasn't
	// an existing match, it would only sometimes prompt the user

	return copy_pop_and_ask();
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
