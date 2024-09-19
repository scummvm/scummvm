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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_music_track.h"

namespace QDEngine {

qdMusicTrack::qdMusicTrack() : _volume(256) {
	toggle_cycle(true);
}

qdMusicTrack::qdMusicTrack(const qdMusicTrack &trk) : qdConditionalObject(trk),
	_file_name(trk._file_name),
	_volume(trk._volume) {
}

qdMusicTrack::~qdMusicTrack() {
}

qdMusicTrack &qdMusicTrack::operator = (const qdMusicTrack &trk) {
	if (this == &trk) return *this;

	*static_cast<qdConditionalObject *>(this) = trk;

	_file_name = trk._file_name;
	_volume = trk._volume;

	return *this;
}

bool qdMusicTrack::load_script(const xml::tag *p) {
	load_conditions_script(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			clear_flags();
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_MUSIC_TRACK_CYCLED:
			if (xml::tag_buffer(*it).get_int())
				toggle_cycle(true);
			else
				toggle_cycle(false);
			break;
		case QDSCR_FILE:
			set_file_name(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_SOUND_VOLUME:
			set_volume(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdMusicTrack::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<music_track");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	if (debugChannelSet(-1, kDebugLog))
		fh.writeString(Common::String::format(" flags=\"%s\"", flag2str(flags()).c_str()));
	else
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));

	if (!_file_name.empty()) {
		fh.writeString(Common::String::format(" file=\"%s\"", qdscr_XML_string(_file_name.toString('\\'))));
	}

	if (is_cycled()) {
		fh.writeString(" cycled=\"1\"");
	}

	if (_volume != 256) {
		fh.writeString(Common::String::format(" volume=\"%d\"", _volume));
	}

	if (has_conditions()) {
		fh.writeString(">\r\n");
		save_conditions_script(fh, indent);
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("</music_track>\r\n");
	} else {
		fh.writeString("/>\r\n");
	}

	return true;
}

#define defFlag(x) { x, #x }

struct FlagsList {
	int f;
	const char *s;
} static flagList[] = {
	defFlag(QD_MUSIC_TRACK_CYCLED),
	defFlag(QD_MUSIC_TRACK_DISABLE_RESTART),
	defFlag(QD_MUSIC_TRACK_DISABLE_SWITCH_OFF),
};

Common::String qdMusicTrack::flag2str(int fl) {
	Common::String res;

	for (int i = 0; i < ARRAYSIZE(flagList); i++) {
		if (fl & flagList[i].f) {
			if (!res.empty())
				res += " | ";

			res += flagList[i].s;

			fl &= ~flagList[i].f;
		}
	}

	if (fl)
		res += Common::String::format(" | %x", fl);

	return res;
}

qdConditionalObject::trigger_start_mode qdMusicTrack::trigger_start() {
	if (qdGameDispatcher * dp = qd_get_game_dispatcher()) {
		dp->play_music_track(this);
		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}
} // namespace QDEngine
