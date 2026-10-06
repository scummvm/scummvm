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
#include "mads/core/buffer.h"
#include "mads/core/env.h"
#include "mads/core/font.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/imath.h"
#include "mads/core/matte.h"
#include "mads/core/mouse.h"
#include "mads/core/pal.h"
#include "mads/core/video.h"
#include "mads/nebular/copy.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/popup.h"
#include "mads/mads.h"

namespace MADS {
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
	byte *dog;

	dog = (byte *)copy_prot;

	for (uint count = 0; count < CopyProt::SIZE; count++) {
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

	if (g_engine->getPlatform() == Common::kPlatformMacintosh) {
		RexNebularEngine *engine = static_cast<RexNebularEngine *>(g_engine);
		Common::String version = engine->getMacintoshApplicationVersion();
		const char versionPrefix[] = "Rex Nebular v";
		if (version.hasPrefix(versionPrefix))
			version = version.substr(sizeof(versionPrefix) - 1);

		for (count = 0; count < COPY_TRIES_ALLOWED; ++count) {
			const Common::String title = count == 0 ?
				Common::String("Macintosh REX NEBULAR version ") + version :
				Common::String("ANSWER INCORRECT!");
			const Common::String subtitle = count == 0 ?
				"(Copy Protection, for your convenience)" :
				"(But we'll give you another chance!)";
			const char *manual = copy_prot.manual == 'g' ?
				"the GAME MANUAL" : "REX'S LOGBOOK";
			const Common::String prompt = Common::String::format(
				"Take out your copy of %s.  See!  That was easy.  Next, "
				"just turn to page %d.  On line %d, find word number %d.",
				manual, copy_prot.page, copy_prot.line,
				copy_prot.word_number);

			entry_buf[0] = copy_prot.say[0];
			entry_buf[1] = '\0';
			result = engine->runMacintoshCopyProtectionDialog(title,
				subtitle, prompt, entry_buf, 12);
			if (result < 0) {
				warning("Could not open Macintosh copy-protection dialog");
				error_flag = COPY_ESCAPE;
				goto done;
			}
			if (result > 0) {
				error_flag = COPY_ESCAPE;
				goto done;
			}

			mads_strlwr(entry_buf);
			if (strcmp(entry_buf, copy_prot.say) == 0)
				goto finish;
		}
		goto done;
	}

	mads_itoa(copy_prot.page, page_buf, 10);
	mads_itoa(copy_prot.line, line_buf, 10);
	mads_itoa(copy_prot.word_number, word_buf, 10);

	for (count = 0; (count < COPY_TRIES_ALLOWED); count++) {
		if (popup_create(32, POPUP_CENTER, POPUP_CENTER))
			goto finish;

		if (!count) {
			if (g_engine->getPlatform() == Common::kPlatformMacintosh) {
				const char versionPrefix[] = "Rex Nebular v";
				Common::String version =
					static_cast<RexNebularEngine *>(g_engine)->
					getMacintoshApplicationVersion();
				if (version.hasPrefix(versionPrefix))
					version = version.substr(sizeof(versionPrefix) - 1);

				Common::strcpy_s(work_buf,
					"Macintosh REX NEBULAR version ");
				Common::strcat_s(work_buf, version.c_str());
			} else {
				Common::strcpy_s(work_buf, "REX NEBULAR version ");
				Common::strcat_s(work_buf, global_release_version);
			}
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

	int result = copy_pop_and_ask();

	if (result == COPY_FAIL) {
		new_room = 804;
		new_section = 8;
		global_init_code();
		global[kCopyProtectFailed] = true;
		game_restore_flag = false;

	} else if (result == COPY_ESCAPE) {
		game.going = false;
	}

	return result;
}

// Word-wraps text onto scr_main at the given left margin, starting at y, breaking
// lines so they stay within maxWidth pixels wide. Returns the y position following
// the last line drawn.
static int copy_fail_draw_paragraph(const char *text, int x, int y, int maxWidth) {
	char lineBuf[256];
	char wordBuf[64];
	char candidate[256];
	const char *p = text;

	lineBuf[0] = '\0';

	while (*p) {
		int len = 0;
		while (*p && *p != ' ' && len < (int)sizeof(wordBuf) - 1)
			wordBuf[len++] = *p++;
		wordBuf[len] = '\0';
		while (*p == ' ')
			++p;

		if (lineBuf[0]) {
			Common::strcpy_s(candidate, lineBuf);
			Common::strcat_s(candidate, " ");
			Common::strcat_s(candidate, wordBuf);
		} else {
			Common::strcpy_s(candidate, wordBuf);
		}

		if (lineBuf[0] && font_string_width(font_inter, candidate, 0) > maxWidth) {
			font_write(font_inter, &scr_main, lineBuf, x, y, 0);
			y += font_inter->max_y_size;
			Common::strcpy_s(lineBuf, wordBuf);
		} else {
			Common::strcpy_s(lineBuf, candidate);
		}
	}

	if (lineBuf[0]) {
		font_write(font_inter, &scr_main, lineBuf, x, y, 0);
		y += font_inter->max_y_size;
	}

	return y;
}

void copy_protection_fail_screen() {
	static const char *const TITLE = "COPY PROTECTION FAILURE";

	static const char *const BODY[] = {
		"You have failed to enter the correct word on the copy protection screen. "
		"If you are an honest, hard working person who purchased this game legitimately "
		"and simply failed to type in the correct word, then we bow down humbly before "
		"you and beg your forgiveness for this heinous inconvenience we have so callously "
		"inflicted upon you.  We beseech you to run the program again and check your "
		"manual or logbook very carefully for the correct word.",

		"HOWEVER, if you are a nasty rodent-like person who has copied this program "
		"illegally from a friend (Yes, we mean YOU, the guilty-looking one in the back "
		"of the room!  That's right, stand up!), then BE IT KNOWN that SOON your teeth "
		"will rot and your hair will turn green.  Your lawn will die, your car will break "
		"down, and warts will grow on the index finger of your firstborn child.  Small "
		"orange blotches will appear on your left leg, and your long lost Aunt Matilda "
		"will arrive unexpectedly for a year-long visit.  The only way you can relieve "
		"yourself of this immense karmic burden is to run down to your local software "
		"store IMMEDIATELY and purchase a legal copy of this COOL ADVENTURE GAME.  "
		"That way, none of these horrible things will happen and a whole generation of "
		"talented designers, programmers, artists, and playtesters can stop wandering "
		"aimlessly through the land at night."
	};

	const int LEFT_MARGIN = 10;
	const int TEXT_WIDTH = 300;

	mouse_hide();

	// Force plain black/white regardless of whatever palette the prior scene left
	// behind - this screen is shown right before the engine tears everything down.
	pal_change_color(0, 0, 0, 0);
	pal_change_color(15, 63, 63, 63);
	font_set_colors(-1, 15, 15, 15);

	buffer_fill(scr_main, 0);

	int width = font_string_width(font_inter, TITLE, 0);
	int y = 8;
	font_write(font_inter, &scr_main, TITLE, (320 - width) / 2, y, 0);
	y += font_inter->max_y_size + 2;
	buffer_hline(scr_main, LEFT_MARGIN, 320 - LEFT_MARGIN, y, 15);
	y += 4;

	for (uint i = 0; i < ARRAYSIZE(BODY); ++i) {
		y = copy_fail_draw_paragraph(BODY[i], LEFT_MARGIN, y, TEXT_WIDTH);
		y += font_inter->max_y_size;
	}

	video_update(&scr_main, 0, 0, 0, 0, 320, 200);

	g_engine->flushKeys();
	while (!g_engine->hasPendingKey() && !g_engine->shouldQuit())
		g_system->delayMillis(10);
	g_engine->flushKeys();
}

} // namespace RexNebular
} // namespace MADS
