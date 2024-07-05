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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/stream.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_camera_mode.h"


namespace QDEngine {

qdCameraMode::qdCameraMode() : camera_mode_(MODE_UNASSIGNED),
	work_time_(0.0f),
	scrolling_speed_(100.0f),
	scrolling_distance_(100),
	smooth_switch_(false),
	center_offset_(0, 0) {
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
			smooth_switch_ = (buf.get_int()) ? true : false;
			break;
		case QDSCR_CAMERA_SCREEN_CENTER:
			buf > center_offset_.x > center_offset_.y;
			break;
		}
	}

	return true;
}

bool qdCameraMode::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<camera_mode type=\"%d\"", (int)camera_mode()));
	fh.writeString(Common::String::format(" scrolling_speed=\"%f\"", scrolling_speed_));
	fh.writeString(Common::String::format(" scrolling_dist=\"%d\"", scrolling_distance_));

	if (center_offset_.x || center_offset_.y) {
		fh.writeString(Common::String::format(" camera_screen_center=\"%d %d\"", center_offset_.x, center_offset_.y));
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
	int mode;
	mode = fh.readSint32LE();
	work_time_ = fh.readFloatLE();
	scrolling_speed_ = fh.readFloatLE();
	scrolling_distance_ = fh.readSint32LE();
	center_offset_.x = fh.readSint32LE();
	center_offset_.y = fh.readSint32LE();

	char switch_flag = fh.readByte();
	smooth_switch_ = (switch_flag) ? true : false;

	return true;
}

bool qdCameraMode::save_data(Common::SeekableWriteStream &fh) const {
	fh.writeSint32LE((int)camera_mode_);
	fh.writeFloatLE(work_time_);
	fh.writeFloatLE(scrolling_speed_);
	fh.writeSint32LE(scrolling_distance_);
	fh.writeSint32LE(center_offset_.x);
	fh.writeSint32LE(center_offset_.y);


	if (smooth_switch_) {
		fh.writeByte(1);
	} else {
		fh.writeByte(0);
	}

	return true;
}

} // namespace QDEngine
