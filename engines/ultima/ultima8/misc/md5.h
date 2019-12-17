/*
Copyright (C) 2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_MISC_MD5_H
#define ULTIMA8_MISC_MD5_H

namespace Ultima {
namespace Ultima8 {

class IDataSource;

namespace Pentagram {

bool md5_file(IDataSource *input, uint8 digest[16], uint32 length = 0);

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima

#endif
