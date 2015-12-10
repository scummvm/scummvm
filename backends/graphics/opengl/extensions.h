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

#ifndef BACKENDS_GRAPHICS_OPENGL_EXTENSIONS_H
#define BACKENDS_GRAPHICS_OPENGL_EXTENSIONS_H

namespace OpenGL {

/**
 * Description structure of all available extensions.
 *
 * This includes information whether extensions we are interested in is
 * available or not. If extensions we are interested in add additional
 * functions, we keep function pointers around in here too.
 */
struct ExtensionsDesc {
	/**
	 * Reset extension state.
	 *
	 * This marks all extensions as unavailable.
	 */
	void reset();

	/** Whether GL_ARB_texture_non_power_of_two is available or not. */
	bool NPOTSupported;
};

/**
 * Description of all available extensions.
 */
extern ExtensionsDesc g_extensions;

} // End of namespace OpenGL

#endif
