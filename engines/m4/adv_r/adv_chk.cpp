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
	int32 str_len;
	char s[MAX_FILENAME_SIZE];
	int32 x1, x2, y1, y2;
	HotSpotRec *head = nullptr;

	for (int32 i = 0; i < num; i++) {
		x1 = fpdef->readSint32LE();
		y1 = fpdef->readSint32LE();
		x2 = fpdef->readSint32LE();
		y2 = fpdef->readSint32LE();

		h = hotspot_new(x1, y1, x2, y2);
		if (!head)
			head = h;
		else
			head = hotspot_add(head, h, false);

		h->feet_x = fpdef->readSint32LE();
		h->feet_y = fpdef->readSint32LE();
		h->facing = fpdef->readSByte();
		h->active = fpdef->readSByte();
		h->cursor_number = fpdef->readSByte();
		h->syntax = fpdef->readSByte();
		h->vocabID = fpdef->readSint32LE();
		h->verbID = fpdef->readSint32LE();

		// -------

		str_len = fpdef->readSint32LE();

		if (str_len) {
			if (!fpdef->read((byte *)s, str_len))
				error_show(FL, 0, "Could not read vocab");
			hotspot_newVocab(h, s);
		}

		str_len = fpdef->readSint32LE();

		if (str_len) {
			if (!fpdef->read((byte *)s, str_len))
				error_show(FL, 0, "Could not read verb");
			hotspot_newVerb(h, s);
		}

		str_len = fpdef->readSint32LE();

		if (str_len) {
			if (!fpdef->read((byte *)s, str_len))
				error_show(FL, 0, "Could not read prep");
			hotspot_newPrep(h, s);
		}

		str_len = fpdef->readSint32LE();

		if (str_len) {
			if (!fpdef->read((byte *)s, str_len))
				error_show(FL, 0, "Could not read sprite");
			hotspot_new_sprite(h, s);
		}

		h->hash = fpdef->readSint16LE();
	}

	return head;
}

static void load_def(SysFile *fpdef) {
	int32 x, y;
	char s[MAX_FILENAME_SIZE];

	fpdef->read((byte *)s, MAX_FILENAME_SIZE);
	Common::strlcpy(_G(myDef)->art_base, s, MAX_FILENAME_SIZE);

	fpdef->read((byte *)s, MAX_FILENAME_SIZE);
	Common::strlcpy(_G(myDef)->picture_base, s, MAX_FILENAME_SIZE);

	_G(myDef)->num_hotspots = fpdef->readSint32LE();
	_G(myDef)->num_parallax = fpdef->readSint32LE();
	_G(myDef)->num_props = fpdef->readSint32LE();
	_G(myDef)->front_y = fpdef->readSint32LE();
	_G(myDef)->back_y = fpdef->readSint32LE();
	_G(myDef)->front_scale = fpdef->readSint32LE();
	_G(myDef)->back_scale = fpdef->readSint32LE();

	for (int32 i = 0; i < 16; i++) {
		_G(myDef)->depth_table[i] = fpdef->readSint16LE();
	}

	_G(myDef)->numRailNodes = fpdef->readSint32LE();

	for (int32 i = 0; i < _G(myDef)->numRailNodes; i++) {
		x = fpdef->readSint32LE();
		y = fpdef->readSint32LE();

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
