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

#include "common/textconsole.h"
#include "ags/engine/gfx/gfxfilter_scummvm_renderer.h"
#include "ags/engine/gfx/gfx_driver_factory.h"
#include "ags/engine/gfx/ali_3d_scummvm.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

void GetGfxDriverFactoryNames(StringV &ids) {
	ids.push_back("ScummVM");
}

IGfxDriverFactory *GetGfxDriverFactory(const String id) {
	if (id.CompareNoCase("ScummVM") == 0)
		return ALSW::ScummVMRendererGraphicsFactory::GetFactory();

	error("No graphics factory with such id: %s", id.GetCStr());
	return nullptr;
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
