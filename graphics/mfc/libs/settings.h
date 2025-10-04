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

#ifndef GRAPHICS_MFC_LIBS_SETTINGS_H
#define GRAPHICS_MFC_LIBS_SETTINGS_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/savefile.h"

namespace Graphics {
namespace MFC {
namespace Libs {

class Settings {
public:
	class Domain {
		friend class Settings;
	private:
		typedef Common::HashMap<Common::String, Common::String,
		        Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> Values;
		Settings *_settings = nullptr;
		Values _values;
		bool _modified = false;

	public:
		void load(Common::InSaveFile *src);
		void save(Common::OutSaveFile *dest);
		static Common::String getDomainName(Common::InSaveFile *src);

		bool isModified() const {
			return _modified;
		}
		bool empty() const {
			return _values.empty();
		}

		bool hasKey(const Common::String &key) const {
			return _values.contains(key);
		}
		int getInt(const Common::String &key, int defaultValue = 0) const {
			return hasKey(key) ? atoi(_values[key].c_str()) : defaultValue;
		}
		bool getBool(const Common::String &key, bool defaultValue = false) const {
			return !hasKey(key) || _values[key].empty() ? false :
			       tolower(_values[key][0]) == 't';
		}
		Common::String getString(const Common::String &key, const char *defaultValue = nullptr) const {
			return hasKey(key) ? _values[key] : defaultValue;
		}
		void setInt(const Common::String &key, int value) {
			_values[key] = Common::String::format("%d", value);
			_modified = true;
		}
		void setBool(const Common::String &key, bool value) {
			_values[key] = value ? "true" : "false";
			_modified = true;
		}
		void setString(const Common::String &key, const Common::String &value) {
			_values[key] = value;
			_modified = true;
		}

		void flushToDisk();
	};

	class Serializer {
	private:
		Domain &_domain;
		bool _isSaving;
	public:
		Serializer(Domain &domain, bool isSaving) :
			_domain(domain), _isSaving(isSaving) {
		}
		~Serializer() {
			_domain.flushToDisk();
		}

		void sync(const Common::String &key, int &field, int defaultValue = 0) {
			if (_isSaving)
				_domain.setInt(key, field);
			else
				field = _domain.getInt(key, defaultValue);
		}
		void sync(const Common::String &key, bool &field, bool defaultValue = false) {
			if (_isSaving)
				_domain.setBool(key, field);
			else
				field = _domain.getBool(key, defaultValue);
		}
		void sync(const Common::String &key, Common::String &field, const char *defaultValue = nullptr) {
			if (_isSaving)
				_domain.setString(key, field);
			else
				field = _domain.getString(key, defaultValue);
		}
	};

private:
	static Common::String getFilename();

public:
	~Settings() {
		save();
	}

	void load();
	void save();

	bool isModified() const;
	Domain &operator[](const Common::String &domain);

private:
	typedef Common::HashMap<Common::String, Domain,
	        Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> Domains;
	Domains _domains;
};

} // namespace Libs
} // namespace MFC
} // namespace Graphics

#endif
