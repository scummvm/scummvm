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

#include "ags/engine/gfx/ali3dsw.h"
#include "ags/engine/gfx/gfxfilter_allegro.h"

#if AGS_HAS_OPENGL
#include "ags/engine/gfx/ali3dogl.h"
#include "ags/engine/gfx/gfxfilter_ogl.h"
#endif

#if AGS_HAS_DIRECT3D
#include "ags/engine/platform/windows/gfx/ali3dd3d.h"
#include "ags/engine/gfx/gfxfilter_d3d.h"
#endif

#include "ags/engine/main/main_allegro.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

void GetGfxDriverFactoryNames(StringV &ids) {
#if AGS_HAS_DIRECT3D
	ids.push_back("D3D9");
#endif
#if AGS_HAS_OPENGL
	ids.push_back("OGL");
#endif
	ids.push_back("Software");
}

IGfxDriverFactory *GetGfxDriverFactory(const String id) {
	if (id.CompareNoCase("Software") == 0)
		return ALSW::ALSWGraphicsFactory::GetFactory();

	set_allegro_error("No graphics factory with such id: %s", id.GetCStr());
	return nullptr;
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
