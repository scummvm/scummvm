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

#ifndef TETRAEDGE_TE_TE_TILED_TEXTURE_H
#define TETRAEDGE_TE_TE_TILED_TEXTURE_H

#include "common/path.h"
#include "common/ptr.h"

#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_palette.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeTiledTexture : public TeResource {
public:
	TeTiledTexture();

	typedef struct {
		TeVector3f32 _vec1;
		TeVector3f32 _vec2;
		TeIntrusivePtr<Te3DTexture> _texture;
	} Tile;

	uint imageFormat();
	bool isLoaded();
	bool load(const Common::String &path);
	bool load(const TeImage &image);
	bool load(const TeIntrusivePtr<Te3DTexture> &texture);
	uint32 numberOfColumns() const;
	uint32 numberOfRow() const;

	TeImage *optimisedTileImage(Common::Array<TeImage> &images, const TeVector2s32 &size,
								const Common::SharedPtr<TePalette> &pal, enum TeImage::Format format);

	void release();
	void save() {};
	Tile *tile(const TeVector2s32 &loc);
	void update(const TeImage &image);

	TeVector2s32 totalSize() const { return _totalSize; }

private:
	TeVector2s32 _totalSize;
	Common::Array<Tile> _tileArray;
	TeVector2s32 _tileSize;
	TeVector2s32 _somethingSize;
	bool _skipBlank;  // note: not clear if this can ever get set?

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TILED_TEXTURE_H
