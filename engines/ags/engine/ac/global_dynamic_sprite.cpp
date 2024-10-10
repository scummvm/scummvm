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

#include "ags/engine/ac/global_dynamic_sprite.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

int LoadImageFile(const char *filename) {
	if (!_GP(spriteset).HasFreeSlots())
		return 0;
	ResolvedPath rp;
	if (!ResolveScriptPath(filename, true, rp))
		return 0;

	Bitmap *loadedFile;
	if (rp.AssetMgr) {
		PACKFILE *pf = PackfileFromAsset(AssetPath(rp.FullPath, "*"));
		if (!pf)
			return 0;
		loadedFile = BitmapHelper::LoadFromFile(pf);
	} else {
		loadedFile = BitmapHelper::LoadFromFile(rp.FullPath);
		if (!loadedFile && !rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
			loadedFile = BitmapHelper::LoadFromFile(rp.AltPath);
	}

	if (!loadedFile)
		return 0;

	std::unique_ptr<Bitmap> image(loadedFile);
	if (!image)
		return 0;
	return add_dynamic_sprite(std::unique_ptr<Bitmap>(PrepareSpriteForUse(image.release(), false)));
}

} // namespace AGS3
