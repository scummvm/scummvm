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

#include "backends/graphics/opengl/extensions.h"
#include "backends/graphics/opengl/opengl-sys.h"

#include "common/tokenizer.h"

namespace OpenGL {

bool g_extNPOTSupported = false;

void initializeGLExtensions() {
	const char *extString = (const char *)glGetString(GL_EXTENSIONS);

	// Initialize default state.
	g_extNPOTSupported = false;

	Common::StringTokenizer tokenizer(extString, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two") {
			g_extNPOTSupported = true;
		}
	}
}

} // End of namespace OpenGL
