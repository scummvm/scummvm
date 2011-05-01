/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/savestate.h"
#include "graphics/surface.h"
#include "common/textconsole.h"

void SaveStateDescriptor::setThumbnail(Graphics::Surface *t) {
	if (_thumbnail.get() == t)
		return;

	_thumbnail = Common::SharedPtr<Graphics::Surface>(t, Graphics::SharedPtrSurfaceDeleter());
}

bool SaveStateDescriptor::getBool(const Common::String &key) const {
	if (contains(key)) {
		const Common::String value = getVal(key);
		bool valueAsBool;
		if (Common::parseBool(value, valueAsBool))
			return valueAsBool;
		error("SaveStateDescriptor: %s '%s' has unknown value '%s' for boolean '%s'",
				save_slot().c_str(), description().c_str(), value.c_str(), key.c_str());
	}
	return false;
}

void SaveStateDescriptor::setDeletableFlag(bool state) {
	setVal("is_deletable", state ? "true" : "false");
}

void SaveStateDescriptor::setWriteProtectedFlag(bool state) {
	setVal("is_write_protected", state ? "true" : "false");
}

void SaveStateDescriptor::setSaveDate(int year, int month, int day) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d.%.2d.%.4d", day, month, year);
	setVal("save_date", buffer);
}

void SaveStateDescriptor::setSaveTime(int hour, int min) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d:%.2d", hour, min);
	setVal("save_time", buffer);
}

void SaveStateDescriptor::setPlayTime(int hours, int minutes) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d:%.2d", hours, minutes);
	setVal("play_time", buffer);
}

void SaveStateDescriptor::setPlayTime(uint32 msecs) {
	uint minutes = msecs / 60000;
	setPlayTime(minutes / 60, minutes % 60);
}

