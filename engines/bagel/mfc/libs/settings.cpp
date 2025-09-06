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

#include "common/system.h"
#include "engines/engine.h"
#include "bagel/mfc/libs/settings.h"

namespace Bagel {
namespace MFC {
namespace Libs {

Common::String Settings::getFilename() {
	// Hack to get engine target name
	Common::String fname = g_engine->getSaveStateName(0);
	fname = Common::String(fname.c_str(), strchr(fname.c_str(), '.'));
	fname += "-settings.ini";

	return fname;
}

void Settings::load() {
	Common::InSaveFile *src = g_system->getSavefileManager()->openForLoading(getFilename());
	Common::String domainName;

	if (src) {
		while (!src->eos()) {
			domainName = Domain::getDomainName(src);
			if (domainName.empty())
				break;

			_domains[domainName].load(src);
		}
	}

	delete src;
}

void Settings::save() {
	if (_domains.empty() || !isModified())
		return;

	Common::OutSaveFile *dest = g_system->getSavefileManager()->openForSaving(getFilename(), false);
	if (!dest)
		return;
	Common::String domainName;

	for (Domains::iterator it = _domains.begin(); it != _domains.end(); ++it) {
		if (it->_value.empty())
			continue;

		domainName = it->_key;
		dest->writeByte('[');
		dest->writeString(it->_key);
		dest->writeByte(']');
		dest->writeByte('\n');

		it->_value.save(dest);
	}

	dest->finalize();
	delete dest;
}

bool Settings::isModified() const {
	for (Domains::iterator it = _domains.begin();
	        it != _domains.end(); ++it) {
		if (it->_value.isModified())
			return true;
	}

	return false;
}

Settings::Domain &Settings::operator[](const Common::String &domain) {
	Settings::Domain &result = _domains[domain];
	result._settings = this;
	return result;
}

Common::String Settings::Domain::getDomainName(Common::InSaveFile *src) {
	Common::String line = src->readLine();
	if (line.empty())
		return "";

	assert(line.hasPrefix("[") && line.hasSuffix("]"));
	return Common::String(line.c_str() + 1, line.size() - 2);
}

void Settings::Domain::load(Common::InSaveFile *src) {
	Common::String str;
	uint equals;

	for (;;) {
		str = src->readLine();
		if (str.empty())
			break;

		equals = str.findFirstOf('=');
		assert(equals != Common::String::npos);

		_values[Common::String(str.c_str(),
		                       str.c_str() + equals)] =
		                           Common::String(str.c_str() + equals + 1);
	}
}

void Settings::Domain::save(Common::OutSaveFile *dest) {
	Common::String str;

	for (Values::iterator it = _values.begin(); it != _values.end(); ++it) {
		str = Common::String::format("%s=%s",
		                             it->_key.c_str(), it->_value.c_str());
		dest->writeString(str);
		dest->writeByte('\n');
	}

	dest->writeByte('\n');
	_modified = false;
}

void Settings::Domain::flushToDisk() {
	assert(_settings);
	_settings->save();
}

} // namespace Libs
} // namespace MFC
} // namespace Bagel
