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

#ifndef BAGEL_HODJNPODJ_LIBS_SETTINGS_H
#define BAGEL_HODJNPODJ_LIBS_SETTINGS_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/savefile.h"

namespace Bagel {
namespace HodjNPodj {

class Settings {
public:
	class Domain {
	private:
		typedef Common::HashMap<Common::String, Common::String,
			Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> Values;
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
		void setInt(const Common::String &key, int value) {
			_values[key] = Common::String::format("%d", value);
			_modified = true;
		}
		void setBool(const Common::String &key, bool value) {
			_values[key] = value ? "true" : "false";
			_modified = true;
		}
	};

public:
	void load();
	void save();

	bool isModified() const;
	Domain &operator[](const Common::String &domain) {
		return _domains[domain];
	}

private:
	typedef Common::HashMap<Common::String, Domain,
		Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> Domains;
	Domains _domains;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
