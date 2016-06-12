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

#include "backends/cloud/cloudconfighelper.h"
#include "common/config-manager.h"

namespace Common {

DECLARE_SINGLETON(Cloud::CloudConfigHelper);

}

namespace Cloud {

bool CloudConfigHelper::hasKey(const Common::String &key) const {
	return ConfMan.hasKey(key, ConfMan.kCloudDomain);
}

void CloudConfigHelper::removeKey(const Common::String &key) {
	ConfMan.removeKey(key, ConfMan.kCloudDomain);
}

const Common::String &CloudConfigHelper::get(const Common::String &key) const {
	return ConfMan.get(key, ConfMan.kCloudDomain);
}

int CloudConfigHelper::getInt(const Common::String &key) const {
	return ConfMan.getInt(key, ConfMan.kCloudDomain);
}

void CloudConfigHelper::set(const Common::String &key, const Common::String &value) {
	ConfMan.set(key, value, ConfMan.kCloudDomain);
}

void CloudConfigHelper::flushToDisk() {
	ConfMan.flushToDisk();
}

} // End of namespace Cloud
