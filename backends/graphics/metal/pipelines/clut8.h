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

#ifndef BACKENDS_GRAPHICS_METAL_PIPELINES_CLUT8_H
#define BACKENDS_GRAPHICS_METAL_PIPELINES_CLUT8_H

#include "backends/graphics/metal/pipelines/shader.h"

namespace Metal {

class CLUT8LookUpPipeline : public ShaderPipeline {
public:
	CLUT8LookUpPipeline(Renderer *renderer);

	void setPaletteTexture(const MetalTexture *paletteTexture) { _paletteTexture = paletteTexture; }

protected:
	void drawTextureInternal(const MetalTexture &texture, const float *coordinates, const float *texcoords) override;
	
private:
	const MetalTexture *_paletteTexture;

};

} // End of namespace Metal

#endif
