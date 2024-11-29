/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/graphics/opengl/pipelines/clut8.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"

namespace OpenGL {

#if !USE_FORCED_GLES
CLUT8LookUpPipeline::CLUT8LookUpPipeline()
	: ShaderPipeline(ShaderMan.query(ShaderManager::kCLUT8LookUp)), _paletteTexture(nullptr) {
}

void CLUT8LookUpPipeline::drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	assert(isActive());

	// Set the palette texture.
	GL_CALL(glActiveTexture(GL_TEXTURE1));
	if (_paletteTexture) {
		_paletteTexture->bind();
	}

	GL_CALL(glActiveTexture(GL_TEXTURE0));
	ShaderPipeline::drawTextureInternal(texture, coordinates, texcoords);
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
