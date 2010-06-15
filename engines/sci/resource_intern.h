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

#ifndef SCI_RESOURCE_INTERN_H
#define SCI_RESOURCE_INTERN_H

#include "common/macresman.h"

#include "sci/resource.h"

namespace Sci {

struct ResourceSource {
	ResSourceType source_type;
	bool scanned;
	Common::String location_name;	// FIXME: Replace by FSNode ?
	const Common::FSNode *resourceFile;
	int volume_number;
	ResourceSource *associated_map;
	uint32 audioCompressionType;
	int32 *audioCompressionOffsetMapping;
	Common::MacResManager macResMan;
	ResourceSource() {
		source_type = kSourceDirectory;
		scanned = false;
		resourceFile = 0;
		volume_number = 0;
		associated_map = NULL;
		audioCompressionType = 0;
		audioCompressionOffsetMapping = NULL;
	}
};


} // End of namespace Sci

#endif // SCI_RESOURCE_INTERN_H
