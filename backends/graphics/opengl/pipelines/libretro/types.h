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

#ifndef BACKENDS_GRAPHICS_OPENGL_PIPELINES_LIBRETRO_TYPES_H
#define BACKENDS_GRAPHICS_OPENGL_PIPELINES_LIBRETRO_TYPES_H

#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#if !USE_FORCED_GLES
#include "common/str.h"
#include "common/array.h"
#include "common/fs.h"

namespace OpenGL {
namespace LibRetro {

typedef Common::HashMap<Common::String, float, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> UniformsMap;

enum FilteringMode {
	kFilteringModeUnspecified,
	kFilteringModeNearest,
	kFilteringModeLinear
};

struct ShaderTexture {
	ShaderTexture() : id(), fileName(), filteringMode(kFilteringModeUnspecified) {}
	ShaderTexture(const Common::String &i, const Common::String &fN, FilteringMode fM, WrapMode wM)
		: id(i), fileName(fN), filteringMode(fM), wrapMode(wM) {}

	Common::String id;
	Common::String fileName;
	FilteringMode filteringMode;
	WrapMode wrapMode;
};

enum ScaleType {
	kScaleTypeSource,
	kScaleTypeViewport,
	kScaleTypeAbsolute,
	kScaleTypeFull
};

inline void applyScale(const ScaleType type,
					   const float source, const float viewport,
					   const float scaleFloat, const uint scaleUint,
					   float *output) {
	switch (type) {
	case kScaleTypeSource:
		*output = source * scaleFloat;
		break;

	case kScaleTypeViewport:
		*output = viewport * scaleFloat;
		break;

	case kScaleTypeAbsolute:
		*output = scaleUint;
		break;

	case kScaleTypeFull:
		*output = viewport;
		break;
	}
}

struct ShaderPass {
	Common::String fileName;
	Common::String alias;

	FilteringMode filteringMode;
	WrapMode wrapMode;
	bool mipmapInput;

	bool floatFBO;
	bool srgbFBO;

	uint frameCountMod;

	ScaleType scaleTypeX;
	ScaleType scaleTypeY;

	float scaleXFloat;
	float scaleYFloat;

	uint scaleXUint;
	uint scaleYUint;

	void applyScale(const float sourceW, const float sourceH,
					const float viewportW, const float viewportH,
					float *outputW, float *outputH) const {
		OpenGL::LibRetro::applyScale(scaleTypeX, sourceW, viewportW, scaleXFloat, scaleXUint, outputW);
		OpenGL::LibRetro::applyScale(scaleTypeY, sourceH, viewportH, scaleYFloat, scaleYUint, outputH);
	}
};

struct ShaderPreset {
	Common::Archive *container;
	Common::Path basePath;

	typedef Common::Array<ShaderTexture> TextureArray;
	TextureArray textures;

	typedef Common::Array<ShaderPass> PassArray;
	PassArray passes;

	UniformsMap parameters;
};

} // End of namespace LibRetro
} // End of namespace OpenGL
#endif // !USE_FORCED_GLES

#endif
