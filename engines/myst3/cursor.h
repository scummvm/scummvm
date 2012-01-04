/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef CURSOR_H_
#define CURSOR_H_

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "common/rect.h"

namespace Myst3 {

class Cursor {
public:
	Cursor(Archive *archive);
	virtual ~Cursor();

	void changeCursor(uint32 index);
	void draw();
private:
	uint32 _currentCursorID;
	GLuint _textureId;
	static const uint _textureSize = 32;

	/** Position of the cursor */
	Common::Point _position;

	void loadAvailableCursors(Archive *archive);
	void generateTexture();
    void uploadTexture();
};

} /* namespace Myst3 */
#endif /* CURSOR_H_ */
