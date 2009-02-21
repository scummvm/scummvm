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

#define NEED_SCI_VERSIONS

#include "common/system.h"
#include "common/config-manager.h"

#include "sci/include/versions.h"
#include "sci/include/engine.h"
#include "sci/tools.h"

namespace Sci {

void version_require_earlier_than(EngineState *s, sci_version_t version) {
	if (s->version_lock_flag)
		return;

	if (version <= s->min_version) {
		sciprintf("Version autodetect conflict: Less than %d.%03d.%03d was requested, but %d.%03d.%03d is the current minimum\n",
		          SCI_VERSION_MAJOR(version), SCI_VERSION_MINOR(version), SCI_VERSION_PATCHLEVEL(version),
		          SCI_VERSION_MAJOR(s->min_version), SCI_VERSION_MINOR(s->min_version),
		          SCI_VERSION_PATCHLEVEL(s->min_version));
		return;
	} else if (version < s->max_version) {
		s->max_version = version - 1;
		if (s->max_version < s->version)
			s->version = s->max_version;
	}
}

void version_require_later_than(EngineState *s, sci_version_t version) {
	if (s->version_lock_flag)
		return;

	if (version > s->max_version) {
		sciprintf("Version autodetect conflict: More than %d.%03d.%03d was requested, but less than"
		          "%d.%03d.%03d is required ATM\n",
		          SCI_VERSION_MAJOR(version), SCI_VERSION_MINOR(version), SCI_VERSION_PATCHLEVEL(version),
		          SCI_VERSION_MAJOR(s->max_version), SCI_VERSION_MINOR(s->max_version),
		          SCI_VERSION_PATCHLEVEL(s->max_version));
		return;
	} else if (version > s->min_version) {
		s->min_version = version;
		if (s->min_version > s->version)
			s->version = s->min_version;
	}
}

int version_parse(const char *vn, sci_version_t *result) {
	char *endptr[3];
	int major = strtol(vn, &endptr[0], 10);
	int minor = strtol(vn + 2, &endptr[1], 10);
	int patchlevel = strtol(vn + 6, &endptr[2], 10);

	if (endptr[0] != vn + 1 || endptr[1] != vn + 5 || *endptr[2] != '\0') {
		sciprintf("Warning: Failed to parse version string '%s'\n", vn);
		return 1;
	}

	*result = SCI_VERSION(major, minor, patchlevel);

	return 0;
}

#undef VERSION_DETECT_BUF_SIZE

} // End of namespace Sci
