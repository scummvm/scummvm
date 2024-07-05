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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_file_owner.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void qdFileOwner::calc_files_size() {
	files_size_ = 0;

	qdFileNameList list0;
	qdFileNameList list1;

	if (get_files_list(list0, list1)) {
		for (qdFileNameList::const_iterator it = list0.begin(); it != list0.end(); ++it)
			files_size_ += app_io::file_size(it->c_str());
		for (qdFileNameList::const_iterator it = list1.begin(); it != list1.end(); ++it)
			files_size_ += app_io::file_size(it->c_str());
	}
}

} // namespace QDEngine
