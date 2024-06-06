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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/app_error_handler.h"

#ifndef __QD_SYSLIB__
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#endif

#include "qdengine/core/qdcore/qd_sound.h"
#include "qdengine/core/system/sound/snd_dispatcher.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#ifdef __QD_DEBUG_ENABLE__
//#define __QD_SOUND_LOG__
#endif

qdSound::qdSound() : volume_(255) {
}

qdSound::~qdSound() {
#ifndef __QD_SYSLIB__
	sound_.free_data();
#endif
}

bool qdSound::free_resource() {
	toggle_resource_status(false);

#ifndef __QD_SYSLIB__
	sound_.free_data();
#endif
	return true;
}

bool qdSound::load_resource() {
	if (file_name_.empty()) return false;

	toggle_resource_status(true);

#ifndef __QD_SYSLIB__
	return wav_file_load(file_name_.c_str(), &sound_);
#else
	return true;
#endif
}

void qdSound::load_script(const xml::tag *p) {
#ifndef __QD_SYSLIB__
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FILE:
			set_file_name(it -> data());
			break;
		case QDSCR_SOUND_VOLUME:
			xml::tag_buffer(*it) > volume_;
			break;
		}
	}
#endif
}

bool qdSound::save_script(XStream &fh, int indent) const {
#ifndef __QD_SYSLIB__
	for (int i = 0; i < indent; i ++) fh < "\t";

	fh < "<sound name=";

	if (name())
		fh < "\"" < qdscr_XML_string(name()) < "\"";
	else
		fh < "\" \"";

	if (volume_ != 255)
		fh < " volume=\"" <= volume_ < "\"";

	if (!file_name_.empty())
		fh < " file=\"" < qdscr_XML_string(file_name_.c_str()) < "\"";

	fh < "/>\r\n";
#endif
	return true;
}

bool qdSound::play(const qdSoundHandle *handle, bool loop, float start_position) const {
#ifdef __QD_SOUND_LOG__
	if (handle && handle -> owner()) {
		appLog::default_log() << appLog::default_log().time_string() << " sound start " << this << " owner: " << handle -> owner();

		if (loop)
			appLog::default_log() << " cycled";

		appLog::default_log() << "\r\n";
	}
#endif

#ifndef __QD_SYSLIB__
	if (sndDispatcher * p = sndDispatcher::get_dispatcher()) {
		sndSound sound(&sound_, handle);
		return p -> play_sound(&sound, loop, start_position, volume_);
	}
#endif

	return false;
}

bool qdSound::stop(const qdSoundHandle *handle) const {
#ifdef __QD_SOUND_LOG__
	if (handle && handle -> owner())
		appLog::default_log() << appLog::default_log().time_string() << " sound stop " << this << " owner: " << handle -> owner() << "\r\n";
#endif

#ifndef __QD_SYSLIB__
	if (sndDispatcher * p = sndDispatcher::get_dispatcher()) {
		if (!handle) {
			sndSound sound(&sound_);
			return p -> stop_sound(&sound);
		} else {
			return p -> stop_sound(handle);
		}
	}
#endif

	return false;
}

float qdSound::position(const qdSoundHandle *handle) const {
	if (!handle) return 0.0f;

#ifndef __QD_SYSLIB__
	if (sndDispatcher * p = sndDispatcher::get_dispatcher())
		return p -> sound_position(handle);
#endif

	return 0.0f;
}

bool qdSound::is_stopped(const qdSoundHandle *handle) const {
#ifndef __QD_SYSLIB__
	if (sndDispatcher * p = sndDispatcher::get_dispatcher()) {
		if (handle) {
			return (p -> sound_status(handle) == sndSound::SOUND_STOPPED);
		} else {
			sndSound sound(&sound_);
			return (p -> sound_status(&sound) == sndSound::SOUND_STOPPED);
		}
	}
#endif

	return true;
}

bool qdSound::set_frequency(const qdSoundHandle *handle, float frequency_coeff) const {
#ifndef __QD_SYSLIB__
	if (sndDispatcher * p = sndDispatcher::get_dispatcher())
		return p -> set_sound_frequency(handle, frequency_coeff);
#endif
	return false;
}
} // namespace QDEngine
