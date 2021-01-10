/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/gfx/gfxdriverfactory.h"

#include "ags/shared/core/platform.h"

#define AGS_HAS_DIRECT3D (AGS_PLATFORM_OS_WINDOWS)
#define AGS_HAS_OPENGL (AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_LINUX)

#include "ags/engine/gfx/ali3dscummvm.h"
#include "ags/engine/gfx/gfxfilter_allegro.h"
#include "ags/engine/main/main_allegro.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

void GetGfxDriverFactoryNames(StringV &ids) {
	ids.push_back("ScummVM");
}

IGfxDriverFactory *GetGfxDriverFactory(const String id) {
	if (id.CompareNoCase("ScummVM") == 0)
		return ALGfx::ALScummVMGraphicsFactory::GetFactory();

	set_allegro_error("No graphics factory with such id: %s", id.GetCStr());
	return nullptr;
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
