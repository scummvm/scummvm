
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

#ifndef M4_ADV_DB_R_DB_CATALOG_H
#define M4_ADV_DB_R_DB_CATALOG_H

#include "m4/m4_types.h"

namespace M4 {

typedef char *StrngPtr;

struct Catalog {
	char *_data = nullptr;
	char **_catalog = nullptr;
	int32 _size = 0;
};

/**
 * Read in the catalog
 */
bool read_catalog();

/**
 * Return the catalog data
 */
char *db_get_catalog();

/**
 * Given an asset name, name, return "path" from the database in result.
 * if path starts or ends with a '\', the '\' characters at those places
 * are deleted.
 */
char *db_rmlst_get_asset_room_path(const char *s, char *result, int32 *sceneCode);

} // End of namespace M4

#endif
