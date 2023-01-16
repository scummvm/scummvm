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

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_3d_texture_opengl.h"
#include "tetraedge/te/te_3d_texture_tinygl.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

Te3DTexture::Te3DTexture() : _createdTexture(false),
_numFrames(1), _frameRate(0), _format(TeImage::INVALID) {
}

Te3DTexture::~Te3DTexture() {
}

bool Te3DTexture::hasAlpha() const {
	TeImage::Format format = getFormat();
	return (format == TeImage::RGBA8 || format == 9
			|| format == 0xb || format == 1 || format == 0);
}

/*static*/
TeIntrusivePtr<Te3DTexture> Te3DTexture::load2(const Common::Path &path, uint size) {
	Common::Path fullPath = path.append(".3dtex");

	TeResourceManager *resMgr = g_engine->getResourceManager();
	if (!resMgr->exists(fullPath)) {
		TeIntrusivePtr<Te3DTexture> retval(makeInstance());
		retval->load(path);
		retval->setAccessName(fullPath);
		resMgr->addResource(retval.get());
		return retval;
	} else {
		return resMgr->getResourceOrMakeInstance<Te3DTexture>(fullPath);
	}
}

bool Te3DTexture::load(const Common::Path &path) {
	TeResourceManager *resmgr = g_engine->getResourceManager();
	Common::Path resPath = path;
	TeIntrusivePtr<TeImage> img = resmgr->getResource<TeImage>(resPath);
	load(*img);
	setAccessName(resPath.append(".3dtex"));
	return true;
}

/*static*/
TeVector2s32 Te3DTexture::optimisedSize(const TeVector2s32 &size) {
	//
	// Note: When we enabled optimized sizes it leaves artifacts around movies
	// etc unless the render size is exactly 800x600.
	//
	// This probably means there is a rounding error somewhere else, just leave
	// off for now.
	//
	if (g_engine->getDefaultScreenWidth() != 800)
		return size;

	// The maths here is a bit funky but it just picks the nearest power of 2 (up)
	int xsize = size._x - 1;
	int ysize = size._y - 1;

	xsize = (int)xsize >> 1 | xsize;
	xsize = (int)xsize >> 2 | xsize;
	xsize = (int)xsize >> 4 | xsize;
	xsize = (int)xsize >> 8 | xsize;
	int v1 = ((int)xsize >> 0x10 | xsize) + 1;
	if (v1 < 8) {
		v1 = 8;
	}

	ysize = (int)ysize >> 1 | ysize;
	ysize = (int)ysize >> 2 | ysize;
	ysize = (int)ysize >> 4 | ysize;
	ysize = (int)ysize >> 8 | ysize;
	int v2 = ((int)ysize >> 0x10 | ysize) + 1;
	if (v2 < 8) {
		v2 = 8;
	}
	return TeVector2s32(v1, v2);
}

/*static*/
Te3DTexture *Te3DTexture::makeInstance() {
	Graphics::RendererType r = g_engine->preferredRendererType();

#if defined(USE_OPENGL_GAME)
	if (r == Graphics::kRendererTypeOpenGL)
		return new Te3DTextureOpenGL();
#endif

#if defined(USE_TINYGL)
	if (r == Graphics::kRendererTypeTinyGL)
		return new Te3DTextureTinyGL();
#endif
	error("Couldn't create Te3DTexture for selected renderer");
}

} // end namespace Tetraedge
