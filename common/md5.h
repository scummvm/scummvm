/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifndef COMMON_MD5_H
#define COMMON_MD5_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/stream.h"

namespace Common {

bool md5_file(const char *name, uint8 digest[16], uint32 length = 0);
bool md5_file(const FilesystemNode &file, uint8 digest[16], uint32 length = 0);
bool md5_file(ReadStream &stream, uint8 digest[16], uint32 length = 0);

// The following two methods work similar to the above two, but 
// instead of computing the binary MD5 digest, they produce
// a human readable lowercase hexstring representing the digest.
bool md5_file_string(const char *name, char md5str[32+1], uint32 length = 0);
bool md5_file_string(const FilesystemNode &file, char md5str[32+1], uint32 length = 0);
bool md5_file_string(ReadStream &stream, char md5str[32+1], uint32 length = 0);


} // End of namespace Common

#endif
