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

#include "mads/madsv2/core/ems.h"

namespace MADS {
namespace MADSV2 {

word ems_page_frame = 0;
word ems_handle = 0;
word ems_pages = 0;
int ems_disabled = 0;
word ems_high_version = 0;
word ems_low_version = 0;
byte *ems_page[4] = {};
word ems_paging_active = 0;
word ems_pages_free = 0;
word ems_pages_reserved = 0;
byte *ems_page_flag = nullptr;
word ems_paging_reserve[EMS_PAGING_CLASSES] = {};

int ems_mapping_changed = 0;
int ems_page_mapped[4] = {};
int ems_page_stack[4] = {};

static byte *ems_physical_pool = nullptr;

bool ems_driver = false;
bool ems_exists = false;

bool ems_detect() {
	if (ems_disabled)
		return false;

	ems_physical_pool = new byte[4 * EMS_PAGE_SIZE]();  // zero-initialised
	if (!ems_physical_pool)
		return false;

	for (int i = 0; i < 4; i++) {
		ems_page[i] = ems_physical_pool + i * EMS_PAGE_SIZE;
		ems_page_mapped[i] = i;
	}

	ems_high_version = 4;
	ems_low_version = 0;
	ems_driver = true;
	ems_exists = true;
	return true;
}

void ems_shutdown() {
	delete[] ems_physical_pool;
	ems_physical_pool = nullptr;

	for (int i = 0; i < 4; i++)
		ems_page[i] = nullptr;

	ems_exists = false;
}

} // namespace MADSV2
} // namespace MADS
