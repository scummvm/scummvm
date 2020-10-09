/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

namespace OpenGL {
namespace BuiltinShaders {

const char *boxVertex =
	"attribute vec2 position;\n"
	"attribute vec2 texcoord;\n"
	"uniform vec2 offsetXY;\n"
	"uniform vec2 sizeWH;\n"
	"uniform vec2 texcrop;\n"
	"uniform bool flipY;\n"
	"varying vec2 Texcoord;\n"
	"void main() {\n"
		"Texcoord = texcoord * texcrop;\n"
		"vec2 pos = offsetXY + position * sizeWH;\n"
		"pos.x = pos.x * 2.0 - 1.0;\n"
		"pos.y = pos.y * 2.0 - 1.0;\n"
		"if (flipY)\n"
			"pos.y *= -1.0;\n"
		"gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";

const char *boxFragment =
	"#ifdef GL_ES\n"
		"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
			"precision highp float;\n"
		"#else\n"
			"precision mediump float;\n"
		"#endif\n"
	"#endif\n"
	"varying vec2 Texcoord;\n"
	"uniform sampler2D tex;\n"
	"void main() {\n"
		"gl_FragColor = texture2D(tex, Texcoord);\n"
	"}\n";

}
} // End of namespace OpenGL

#endif
