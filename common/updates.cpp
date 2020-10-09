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

#include "common/system.h"
#include "common/updates.h"
#include "common/translation.h"

namespace Common {

static const int updateIntervals[] = {
	UpdateManager::kUpdateIntervalNotSupported,
	UpdateManager::kUpdateIntervalOneDay,
	UpdateManager::kUpdateIntervalOneWeek,
	UpdateManager::kUpdateIntervalOneMonth,
	-1
};

const int *UpdateManager::getUpdateIntervals() {
	return updateIntervals;
}

int UpdateManager::normalizeInterval(int interval) {
	const int *val = updateIntervals;

	while (*val != -1) {
		if (*val >= interval)
			return *val;
		val++;
	}

	return val[-1]; // Return maximal acceptable value
}

Common::U32String UpdateManager::updateIntervalToString(int interval) {
	switch (interval) {
	case kUpdateIntervalNotSupported:
		return _("Never");
	case kUpdateIntervalOneDay:
		return _("Daily");
	case kUpdateIntervalOneWeek:
		return _("Weekly");
	case kUpdateIntervalOneMonth:
		return _("Monthly");
	default:
		return _("<Bad value>");
	}
}

} // End of namespace Common
