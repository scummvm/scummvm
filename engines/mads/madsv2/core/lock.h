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

#ifndef MADS_CORE_LOCK_H
#define MADS_CORE_LOCK_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

extern char *lock_program_name;
extern char *lock_search_mark;
extern int  lock_search_length;
extern word *lock_hash_value;

word lock_get_disk_hash(void);

long lock_search_hash_offset(void);

void lock_write_new_hash(void);

int lock_get_copy_code(void);

void lock_short_protection_check(void);

int  lock_verification(void);
void lock_sabotage(void);

void lock_long_protection_check(void);

void lock_secret_protection_check(void);

void lock_preliminary_check(void);

} // namespace MADSV2
} // namespace MADS

#endif
