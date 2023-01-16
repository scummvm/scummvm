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

#ifndef TETRAEDGE_TE_TE_3D_TEXTURE_OPENGL_H
#define TETRAEDGE_TE_TE_3D_TEXTURE_OPENGL_H

#if defined(USE_OPENGL_GAME)

#include "tetraedge/te/te_3d_texture.h"

namespace Tetraedge {

class Te3DTextureOpenGL : public Te3DTexture {
public:
	Te3DTextureOpenGL();
	virtual ~Te3DTextureOpenGL();

	void bind() const override;
	void copyCurrentRender(uint xoffset, uint yoffset, uint x, uint y) override;
	void create() override;
	void destroy() override;
	void forceTexData(uint gltexture, uint xsize, uint ysize) override;

	bool load(const TeImage &img) override;

	static void unbind();
	bool unload() override;
	void update(const TeImage &img, uint xoff, uint yoff) override;

	void writeTo(Graphics::Surface &surf) override;

private:
	uint _glTexture;
	//uint _glPixelFormat;

};

} // end namespace Tetraedge

#endif // USE_OPENGL

#endif // TETRAEDGE_TE_TE_3D_TEXTURE_OPENGL_H
