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

#include "glk/archetype/error.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/token.h"

namespace Glk {
namespace Archetype {

void hit_eof(progfile &f, AclType expecting, int specific) {
	if (KeepLooking) {
		KeepLooking = false;
		g_vm->write("Found end of file; expected ");
		write_token(expecting, specific);
		g_vm->writeln();
	}
}

void expected(progfile &f, AclType expect_ttype, int expect_specific) {
	if (KeepLooking) {
		f.sourcePos();
		g_vm->write("Expected ");
		write_token(expect_ttype, expect_specific);
		g_vm->write("; found ");
		write_token(f.ttype, f.tnum);
		g_vm->writeln();
	}
}

void expect_general(progfile &f, const String &general_desc) {
	if (KeepLooking) {
		f.sourcePos();
		g_vm->write("Expected %s; found ", general_desc.c_str());
		write_token(f.ttype, f.tnum);
		g_vm->writeln();
	}
}

void error_message(progfile &f, const String &message) {
	if (KeepLooking) {
		f.sourcePos();
		g_vm->writeln(message);
	}
}

bool insist_on(progfile &f, AclType some_type, int some_number) {
	if (!get_token(f)) {
		hit_eof(f, some_type, some_number);
		return false;
	} else if (f.ttype != some_type && f.tnum != some_number) {
		expected(f, some_type, some_number);
		KeepLooking = false;
		return false;
	} else {
		return true;
	}
}

} // End of namespace Archetype
} // End of namespace Glk
