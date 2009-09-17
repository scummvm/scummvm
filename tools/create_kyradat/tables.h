/* ScummVM Tools
 * Copyright (C) 2009 The ScummVM project
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

#ifndef CREATE_KYRADAT_TABLES_H
#define CREATE_KYRADAT_TABLES_H

#include "create_kyradat.h"

struct ExtractEntrySearchData {
	int lang;
	int platform;

	SearchData hint;
};

typedef std::list<ExtractEntrySearchData> ExtractEntryList;
ExtractEntryList getProvidersForId(int id);

#endif

