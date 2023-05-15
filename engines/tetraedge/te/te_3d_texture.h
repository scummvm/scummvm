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

#ifndef TETRAEDGE_TE_TE_3D_TEXTURE_H
#define TETRAEDGE_TE_TE_3D_TEXTURE_H

#include "common/path.h"
#include "common/ptr.h"
#include "common/str.h"

#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class Te3DTexture : public TeResource {
public:
	Te3DTexture();
	virtual ~Te3DTexture();

	virtual void bind() const = 0;
	virtual void copyCurrentRender(uint xoffset, uint yoffset, uint x, uint y) = 0;
	virtual void create() = 0;
	virtual void destroy() = 0;
	virtual void forceTexData(uint gltexture, uint xsize, uint ysize) = 0;

	TeImage::Format getFormat() const { return _format; }
	bool hasAlpha() const;

	bool load(const TetraedgeFSNode &path);
	virtual bool load(const TeImage &img) = 0;
	// The original passes a GL enum param, but it's only ever GL_INVALID or GL_ALPHA.
	// Simplify to avoid leaking gl types.
	static TeIntrusivePtr<Te3DTexture> load2(const TetraedgeFSNode &node, bool alphaOnly);

	static TeVector2s32 optimisedSize(const TeVector2s32 &size);

	virtual bool unload() = 0;
	virtual void update(const TeImage &img, uint xoff, uint yoff) = 0;

	virtual void writeTo(Graphics::Surface &surf) = 0;

	uint width() const { return _width; }
	uint height() const { return _height; }
	void setLoadAlphaOnly() { _alphaOnly = true; }

	static Te3DTexture *makeInstance();


protected:
	uint _width;
	uint _height;
	TeImage::Format _format;
	bool _createdTexture;
	bool _loaded;
	TeMatrix4x4 _matrix;

	bool _alphaOnly;

	uint _texWidth;
	uint _texHeight;
	uint _leftBorder;
	uint _btmBorder;
	uint _rightBorder;
	uint _topBorder;
	bool _flipY;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_3D_TEXTURE_H
