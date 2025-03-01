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

#ifndef HODJNPODJ_DIALOGS_VARS_H
#define HODJNPODJ_DIALOGS_VARS_H

#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/boflib/vector.h"
#include "bagel/hodjnpodj/fuge/defines.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

struct Vars {
	CVector _gvCenter;

	Vars();
	~Vars();
	void clear();
};

extern Vars *g_vars;

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
