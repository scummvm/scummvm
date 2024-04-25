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

#ifndef ULTIMA8_GUMPS_DESKTOPGUMP_H
#define ULTIMA8_GUMPS_DESKTOPGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

/**
 * A virtual gump which contains all the regular game components.
 */
class DesktopGump : public Gump {
	static bool _fadedModal;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	DesktopGump();
	DesktopGump(int32 x, int32 y, int32 width, int32 height);
	~DesktopGump() override;

	void RenderSurfaceChanged() override;

	void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	void PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	static void SetFadedModal(bool set) {
		_fadedModal = set;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
