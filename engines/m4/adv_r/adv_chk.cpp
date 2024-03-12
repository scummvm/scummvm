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

#include "m4/adv_r/adv_chk.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/fileio/sys_file.h"
#include "m4/vars.h"

namespace M4 {

static HotSpotRec *read_hotspots(SysFile *fpdef, HotSpotRec *h, int32 num) {
	int32 str_len = 0;
	int32 a;
	int8 b;
	int16 c;
	bool d;
	char e;
	char s[MAX_FILENAME_SIZE];
	int32 x1, x2, y1, y2;
	HotSpotRec *head = nullptr;
	void *buffPtr;
	int32 i = 0;

	for (i = 0; i < num; i++) {
		buffPtr = &x1;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read ul_x");
		x1 = convert_intel32(x1);

		buffPtr = &y1;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read ul_y");
		y1 = convert_intel32(y1);

		buffPtr = &x2;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read lr_x");
		x2 = convert_intel32(x2);

		buffPtr = &y2;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read lr_y");
		y2 = convert_intel32(y2);

		h = hotspot_new(x1, y1, x2, y2);
		if (!head)
			head = h;
		else
			head = hotspot_add(head, h, false);

		buffPtr = &a;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read feet_x");
		a = convert_intel32(a);
		h->feet_x = a;

		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read feet_y");
		a = convert_intel32(a);
		h->feet_y = a;

		buffPtr = &b;
		if (!fpdef->read(&buffPtr, sizeof(int8)))
			error_show(FL, 0, "Could not read facing");
		h->facing = b;

		buffPtr = &d;
		if (!fpdef->read(&buffPtr, sizeof(bool)))
			error_show(FL, 0, "Could not read active");
		h->active = d;

		buffPtr = &e;
		if (!fpdef->read(&buffPtr, sizeof(char)))
			error_show(FL, 0, "Could not read cursor_number");
		h->cursor_number = e;

		if (!fpdef->read(&buffPtr, sizeof(char)))
			error_show(FL, 0, "Could not read syntax");
		h->syntax = e;

		buffPtr = &a;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read vocabID");
		a = convert_intel32(a);
		h->vocabID = a;

		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read verbID");
		a = convert_intel32(a);
		h->verbID = a;

		// -------

		buffPtr = &str_len;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read vocab length");
		str_len = convert_intel32(str_len);

		if (str_len) {
			buffPtr = &s[0];
			if (!fpdef->read(&buffPtr, str_len))
				error_show(FL, 0, "Could not read vocab");
			hotspot_newVocab(h, s);
		}

		buffPtr = &str_len;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read verb length");
		str_len = convert_intel32(str_len);

		if (str_len) {
			buffPtr = &s[0];
			if (!fpdef->read(&buffPtr, str_len))
				error_show(FL, 0, "Could not read verb");
			hotspot_newVerb(h, s);
		}

		buffPtr = &str_len;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read prep length");
		str_len = convert_intel32(str_len);

		if (str_len) {
			buffPtr = &s[0];
			if (!fpdef->read(&buffPtr, str_len))
				error_show(FL, 0, "Could not read prep");
			hotspot_newPrep(h, s);
		}

		buffPtr = &str_len;
		if (!fpdef->read(&buffPtr, sizeof(int32)))
			error_show(FL, 0, "Could not read sprite");
		str_len = convert_intel32(str_len);

		if (str_len) {
			buffPtr = &s[0];
			if (!fpdef->read(&buffPtr, str_len))
				error_show(FL, 0, "Could not read sprite");
			hotspot_new_sprite(h, s);
		}

		buffPtr = &c;
		if (!fpdef->read(&buffPtr, sizeof(int16)))
			error_show(FL, 0, "Could not read sprite");
		c = convert_intel16(c);
		h->hash = c;
	}

	return head;
}

static void load_def(SysFile *fpdef) {
	int32 i, a;
	int16 b;
	int32 c, x, y;
	char s[MAX_FILENAME_SIZE];
	void *buffPtr;

	buffPtr = &s[0];
	fpdef->read(&buffPtr, MAX_FILENAME_SIZE);
	Common::strlcpy(_G(myDef)->art_base, s, MAX_FILENAME_SIZE);

	fpdef->read(&buffPtr, MAX_FILENAME_SIZE);
	Common::strlcpy(_G(myDef)->picture_base, s, MAX_FILENAME_SIZE);

	buffPtr = &a;
	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->num_hotspots = a;

	buffPtr = &a;
	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->num_parallax = a;

	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->num_props = a;

	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->front_y = a;

	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->back_y = a;

	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->front_scale = a;

	fpdef->read(&buffPtr, sizeof(int32));
	a = convert_intel32(a);
	_G(myDef)->back_scale = a;

	buffPtr = &b;
	for (i = 0; i < 16; i++) {
		fpdef->read(&buffPtr, sizeof(int16));
		b = convert_intel16(b);
		_G(myDef)->depth_table[i] = b;
	}

	buffPtr = &c;
	fpdef->read(&buffPtr, sizeof(int32));
	c = convert_intel32(c);
	_G(myDef)->numRailNodes = c;
	for (i = 0; i < _G(myDef)->numRailNodes; i++) {
		fpdef->read(&buffPtr, sizeof(int32));
		c = convert_intel32(c);
		x = c;
		fpdef->read(&buffPtr, sizeof(int32));
		c = convert_intel32(c);
		y = c;

		if (AddRailNode(x, y, nullptr, true) < 0)
			error_show(FL, 0, "more than %d (defn. in intrrail.h) nodes", MAXRAILNODES);
	}

	_G(myDef)->hotspots = read_hotspots(fpdef, nullptr, _G(myDef)->num_hotspots);
	_G(myDef)->parallax = read_hotspots(fpdef, nullptr, _G(myDef)->num_parallax);
	_G(myDef)->props = read_hotspots(fpdef, nullptr, _G(myDef)->num_props);
}

int db_def_chk_read(int16 room_code, SceneDef *rdef) {
	_G(myDef) = rdef;

	_G(def_filename) = Common::String::format("%03d.chk", room_code);
	SysFile fpdef(_G(def_filename), BINARY);

	load_def(&fpdef);
	fpdef.close();

	return -1; // everything happy code
}

} // End of namespace M4
