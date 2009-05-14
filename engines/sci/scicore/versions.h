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

#define SCI_VERSION(_major_, _minor_, _patchlevel_) (((_major_)<<20) | ((_minor_)<<10) | _patchlevel_)
/* This allows version numbers to be compared directly */

#define SCI_VERSION_MAJOR(_version_) ((_version_) >> 20)
#define SCI_VERSION_MINOR(_version_) (((_version_) >> 10) & 0x3ff)
#define SCI_VERSION_PATCHLEVEL(_version_) ((_version_) & 0x3ff)
#define SCI_VERSION_IGNORE_PATCHLEVEL(_version_) ((_version) & ~0x3ff)

/* Version number guide:
** - Always use the version number of the first known version to have a special feature.
** - Don't assume that special feature changes are linked just because they appeared to change
**   simultaneously.
** - Put all the magic version numbers here, into THIS file.
** - "FTU" means "First To Use"
*/

#define SCI_VERSION_FTU_NEW_SCRIPT_HEADER SCI_VERSION(0,000,395)
/* Last version known not to do this: 0.000.343
** Old SCI versions used two word header for script blocks (first word equal
** to 0x82, meaning of the second one unknown). New SCI versions used one
** word header.
*/

#define SCI_VERSION_FTU_2ND_ANGLES SCI_VERSION(0,000,395)
/* Last version known not to use this: ?
** Earlier versions assign 120 degrees to left & right , and 60 to up and down.
** Later versions use an even 90 degree distribution.
*/

#define SCI_VERSION_FTU_NEWER_DRAWPIC_PARAMETERS SCI_VERSION(0,000,502)
/* Last version known not to do this: 0.000.435
** Old SCI versions used to interpret the third DrawPic() parameter inversely,
** with the opposite default value (obviously)
*/

#define SCI_VERSION_FTU_PRIORITY_14_ZONES SCI_VERSION(0,000,502)
/* Last version known to do this: 0.000.490
 * Uses 14 zones from 42 to 190 instead of 15 zones from 42 to 200.
*/

#define SCI_VERSION_FTU_NEW_GETTIME SCI_VERSION(0,000,629)
/* These versions of SCI has a different set of subfunctions in GetTime() */

#define SCI_VERSION_FTU_DOSOUND_VARIANT_1 SCI_VERSION(1,000,000)

#define SCI_VERSION_FTU_LOFS_ABSOLUTE SCI_VERSION(1,000,200)
/* First version known to do this: ?
   In later versions (SCI1 and beyond), the argument of lofs[as]
   instructions is absolute rather than relative.
*/

#define SCI_VERSION_FTU_DOSOUND_VARIANT_2 SCI_VERSION(1,000,510)


typedef int sci_version_t;

struct EngineState;

} // End of namespace Sci

#endif // SCI_SCICORE_VERSIONS_H
