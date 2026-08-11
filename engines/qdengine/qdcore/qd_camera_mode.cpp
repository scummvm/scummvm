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
#include "common/stream.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_camera_mode.h"


namespace QDEngine {

qdCameraMode::qdCameraMode() : _camera_mode(MODE_UNASSIGNED),
	_work_time(0.0f),
	_scrolling_speed(100.0f),
	_scrolling_distance(100),
	_smooth_switch(false),
	_center_offset(0, 0) {
}

bool qdCameraMode::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);

		switch (it->ID()) {
		case QDSCR_TYPE:
			set_camera_mode(camera_mode_t(buf.get_int()));
			break;
		case QDSCR_TIME:
			set_work_time(buf.get_float());
			break;
		case QDSCR_CAMERA_SCROLLING_SPEED:
			set_scrolling_speed(buf.get_float());
			break;
		case QDSCR_CAMERA_SCROLLING_DIST:
			set_scrolling_distance(buf.get_int());
			break;
		case QDSCR_CAMERA_SMOOTH_SWITCH:
			_smooth_switch = (buf.get_int()) ? true : false;
			break;
		case QDSCR_CAMERA_SCREEN_CENTER:
			buf > _center_offset.x > _center_offset.y;
			break;
		}
	}

	return true;
}

bool qdCameraMode::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<camera_mode type=\"%d\"", (int)camera_mode()));
	fh.writeString(Common::String::format(" scrolling_speed=\"%f\"", _scrolling_speed));
	fh.writeString(Common::String::format(" scrolling_dist=\"%d\"", _scrolling_distance));

	if (_center_offset.x || _center_offset.y) {
		fh.writeString(Common::String::format(" camera_screen_center=\"%d %d\"", _center_offset.x, _center_offset.y));
	}

	if (has_work_time()) {
		fh.writeString(Common::String::format(" time=\"%f\"", work_time()));
	}

	if (smooth_switch()) {
		fh.writeString(" smooth_switch=\"1\"");
	}

	fh.writeString("/>\r\n");

	return true;
}

bool qdCameraMode::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(4, kDebugSave, "    qdCameraMode::load_data(): before: %d", (int)fh.pos());
	/*int mode = */fh.readSint32LE();
	_work_time = fh.readFloatLE();
	_scrolling_speed = fh.readFloatLE();
	_scrolling_distance = fh.readSint32LE();
	_center_offset.x = fh.readSint32LE();
	_center_offset.y = fh.readSint32LE();

	char switch_flag = fh.readByte();
	_smooth_switch = (switch_flag) ? true : false;
	debugC(4, kDebugSave, "    qdCameraMode::load_data(): after: %d", (int)fh.pos());

	return true;
}

bool qdCameraMode::save_data(Common::WriteStream &fh) const {
	debugC(4, kDebugSave, "    qdCameraMode::save_data(): before: %d", (int)fh.pos());

	fh.writeSint32LE((int)_camera_mode);
	fh.writeFloatLE(_work_time);
	fh.writeFloatLE(_scrolling_speed);
	fh.writeSint32LE(_scrolling_distance);
	fh.writeSint32LE(_center_offset.x);
	fh.writeSint32LE(_center_offset.y);


	if (_smooth_switch) {
		fh.writeByte(1);
	} else {
		fh.writeByte(0);
	}

	debugC(4, kDebugSave, "    qdCameraMode::save_data(): after: %d", (int)fh.pos());
	return true;
}

} // namespace QDEngine
