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

#ifndef TETRAEDGE_TE_TE_FONT3_H
#define TETRAEDGE_TE_TE_FONT3_H

#include "common/file.h"
#include "common/str.h"
#include "common/path.h"
#include "common/types.h"
#include "common/hashmap.h"
#include "common/rect.h"

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_i_font.h"

namespace Graphics {
class Font;
}

namespace Tetraedge {

/**
 * TeFont3 is a minimal wrapper on Graphics::Font for TTF files, supporting
 * multiple sizes and matching the original TeFont api a bit closer.
 */
class TeFont3 : public TeIFont {
public:
	TeFont3();
	virtual ~TeFont3();

	bool load(const Common::Path &path);
	bool load(const TetraedgeFSNode &node);
	void unload();

private:

	TeIntrusivePtr<Te3DTexture> getFontSizeData(int size) const {
		return _fontSizeData[size];
	}

	Graphics::Font *getAtSize(uint size) override;
	Common::ScopedPtr<Common::SeekableReadStream> _fontFile;
	Common::HashMap<uint, Graphics::Font *> _fonts;
	Common::Path _loadedPath;
	Common::HashMap<uint, TeIntrusivePtr<Te3DTexture>> _fontSizeData;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_FONT3_H
