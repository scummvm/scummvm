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

#ifndef TETRAEDGE_TE_TE_WARP_BLOC_H
#define TETRAEDGE_TE_TE_WARP_BLOC_H

#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_mesh.h"

#include "common/file.h"

namespace Tetraedge {

class TeWarpBloc {
public:
	// TODO; Better names than this.
	enum CubeFace {
		Face0 = 0,
		Face1 = 1,
		Face2 = 2,
		Face3 = 3,
		Face4 = 4,
		Face5 = 5,
		FaceInvalid = 6,
	};
	TeWarpBloc();
	~TeWarpBloc();

	void color(const TeColor &col);
	void color(uint num, const TeColor &col);
	void create(CubeFace face, uint i, uint j, const TeVector2s32 &offset);
	void create();
	void index(uint offset, uint val);
	bool isLoaded() const;
	void loadTexture(Common::SeekableReadStream &file, const Common::String &type);
	//void operator=(const TeWarpBloc &other); // unused
	//bool operator==(const TeWarpBloc &other); // unused
	void render();
	void texture(uint idx, float x, float y);
	void unloadTexture();
	void vertex(uint n, float x, float y, float z);
	TeVector3f32 vertex(uint n) const;

	const TeVector2s32 offset() const { return _offset; }
	CubeFace face() const { return _cubeFace; }
	void setCubeFace(CubeFace face) { _cubeFace = face; }
	void setTextureFileOffset(long offset) { _textureDataFileOffset = offset; }

private:
	TeVector2s32 _offset;
	CubeFace _cubeFace;
	// Tex UV / indexes / texture from orig now in here.
	Common::SharedPtr<TeMesh> _mesh;
	long _textureDataFileOffset;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_WARP_BLOC_H
