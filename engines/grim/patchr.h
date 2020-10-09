/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_PATCHR_H
#define GRIM_PATCHR_H

namespace Grim {

/**
 * Take an arbitrary SeekableReadStream and wrap it in a custom stream which
 * transparently patch it on-the-fly. It uses filename.patchr as patchfile,
 * but if it fails, it tries with filename_1.patchr and so on.
 * If no valid patchfile exists, the original stream is returned unmodified
 * (and in particular, not wrapped).
 * For more informations, see diffr and patchr manuals.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
Common::SeekableReadStream *wrapPatchedFile(Common::SeekableReadStream *rs, const Common::String &filename);

} // end of namespace Grim

#endif
