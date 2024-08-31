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

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/system/sound/snd_dispatcher.h"


namespace QDEngine {

#ifdef __QD_DEBUG_ENABLE__
//#define __QD_SOUND_LOG__
#endif

qdSound::qdSound() : _volume(255) {
}

qdSound::~qdSound() {
	_sound.free_data();
}

bool qdSound::free_resource() {
	toggle_resource_status(false);
	_sound.free_data();

	return true;
}

bool qdSound::load_resource() {
	if (_file_name.empty()) return false;

	toggle_resource_status(true);

	return _sound.wav_file_load(_file_name);
}

void qdSound::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FILE:
			set_file_name(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_SOUND_VOLUME:
			xml::tag_buffer(*it) > _volume;
			break;
		}
	}
}

bool qdSound::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<sound name=");

	if (name()) {
		fh.writeString(Common::String::format("\"%s\"", qdscr_XML_string(name())));
	} else {
		fh.writeString("\" \"");
	}

	if (_volume != 255) {
		fh.writeString(Common::String::format(" volume=\"%d\"", _volume));
	}

	if (!_file_name.empty()) {
		fh.writeString(Common::String::format(" file=\"%s\"", qdscr_XML_string(_file_name.toString('\\'))));
	}

	fh.writeString("/>\r\n");
	return true;
}

bool qdSound::play(const qdSoundHandle *handle, bool loop) const {
	if (handle && handle->owner()) {
		debugCN(3, kDebugSound, "[%d] sound start %p owner: %s", g_system->getMillis(), (const void *)this, handle->owner()->toString().c_str());

		if (loop)
			debugCN(3, kDebugSound, " cycled");
		debugC(3, kDebugSound, "%s", "");
	}

	if (sndDispatcher *p = sndDispatcher::get_dispatcher()) {
		sndSound sound(&_sound, handle);
		return p->play_sound(&sound, loop, _volume);
	}

	return false;
}

bool qdSound::stop(const qdSoundHandle *handle) const {
	debugC(3, kDebugSound, "[%d] sound stop %p owner: %s", g_system->getMillis(), (const void *)this, handle ? handle->owner()->toString().c_str() : "<null>");
	if (sndDispatcher *p = sndDispatcher::get_dispatcher()) {
		if (!handle) {
			sndSound sound(&_sound);
			return p->stop_sound(&sound);
		} else {
			return p->stop_sound(handle);
		}
	}

	return false;
}

bool qdSound::is_stopped(const qdSoundHandle *handle) const {
	if (sndDispatcher *p = sndDispatcher::get_dispatcher()) {
		if (handle) {
			return (p->sound_status(handle) == sndSound::SOUND_STOPPED);
		} else {
			sndSound sound(&_sound);
			return (p->sound_status(&sound) == sndSound::SOUND_STOPPED);
		}
	}

	return true;
}

bool qdSound::set_frequency(const qdSoundHandle *handle, float frequency_coeff) const {
	if (sndDispatcher *p = sndDispatcher::get_dispatcher())
		return p->set_sound_frequency(handle, frequency_coeff);

	return false;
}
} // namespace QDEngine
