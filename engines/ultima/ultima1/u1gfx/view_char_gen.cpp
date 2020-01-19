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

#include "ultima/ultima1/u1gfx/view_char_gen.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewCharacterGeneration, Shared::Gfx::VisualContainer)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

ViewCharacterGeneration::ViewCharacterGeneration(Shared::TreeItem *parent) :
	Shared::Gfx::VisualContainer("CharGen", Rect(0, 0, 320, 200), parent), _flags(FLAG_INITIAL) {
}

void ViewCharacterGeneration::draw() {
	if (!_isDirty)
		return;

	// Handle drawing the frame
	Shared::Gfx::VisualSurface s = getSurface();
	s.clear();
	DrawingSupport ds(s);
	ds.drawFrame();

	VisualContainer::draw();
}

bool ViewCharacterGeneration::KeypressMsg(CKeypressMsg &msg) {

	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
