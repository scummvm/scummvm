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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHARED_CONF_CONF_SERIALIZER_H
#define SHARED_CONF_CONF_SERIALIZER_H

#include "common/config-manager.h"

namespace Ultima {
namespace Shared {

/**
 * A simplified serializer class for reading/writing stuff from ConfMan
 */
class ConfSerializer {
private:
	bool _isSaving;
public:
	/**
	 * Constructor
	 */
	ConfSerializer(bool saving) : _isSaving(saving) {}

	/**
	 * Destructor
	 */
	~ConfSerializer() {
		if (_isSaving)
			ConfMan.flushToDisk();
	}

	/**
	 * Returns true if saving
	 */
	bool isSaving() const {
		return _isSaving;
	}

	/**
	 * Returns true if loading
	 */
	bool isLoading() const {
		return !_isSaving;
	}

	/**
	 * Syncs a string
	 */
	void syncAsString(const Common::String &key, Common::String &value,
			const char *defaultValue = nullptr) {
		if (_isSaving)
			ConfMan.set(key, value);
		else
			value = ConfMan.hasKey(key) ? ConfMan.get(key) : Common::String(defaultValue);
	}

	/**
	 * Syncs a boolean
	 */
	void syncAsBool(const Common::String &key, bool &value,
			bool defaultValue = false) {
		if (_isSaving)
			ConfMan.setBool(key, value);
		else
			value = ConfMan.hasKey(key) ? ConfMan.getBool(key) : defaultValue;
	}

	/**
	 * Syncs an integer
	 */
	template<typename T>
	void syncAsInt(const Common::String &key, T &value, T defaultValue = 0) {
		if (_isSaving)
			ConfMan.setInt(key, value);
		else
			value = ConfMan.hasKey(key) ? ConfMan.getInt(key) : defaultValue;
	}
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
