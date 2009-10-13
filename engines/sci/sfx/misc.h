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

#ifndef SCI_SFX_MISC_H
#define SCI_SFX_MISC_H

namespace Sci {

class SongIterator;



SongIterator *sfx_iterator_combine(SongIterator *it1, SongIterator *it2);
/* Combines two song iterators into one
** Parameters: (sfx_iterator_t *) it1: One of the two iterators, or NULL
**             (sfx_iterator_t *) it2: The other iterator, or NULL
** Returns   : (sfx_iterator_t *) A combined iterator
** If a combined iterator is returned, it will be flagged to be allowed to
** dispose of 'it1' and 'it2', where applicable. This means that this
** call should be used by song players, but not by the core sound system
*/


} // End of namespace Sci

#endif // SCI_SFX_MISC_H
