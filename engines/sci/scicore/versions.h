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

/* Versions management */

#ifndef SCI_SCICORE_VERSIONS_H
#define SCI_SCICORE_VERSIONS_H

namespace Sci {

/* Version number guide:
** - Always use the version number of the first known version to have a special feature.
** - Don't assume that special feature changes are linked just because they appeared to change
**   simultaneously.
** - Put all the magic version numbers here, into THIS file.
** - "FTU" means "First To Use"
*/

#define SCI_VERSION_FTU_DOSOUND_VARIANT_2 SCI_VERSION(1,000,510)


} // End of namespace Sci

#endif // SCI_SCICORE_VERSIONS_H
