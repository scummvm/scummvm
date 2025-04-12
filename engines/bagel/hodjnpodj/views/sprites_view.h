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

#ifndef HODJNPODJ_VIEWS_SPRITES_VIEW_H
#define HODJNPODJ_VIEWS_SPRITES_VIEW_H

#include "common/list.h"
#include "bagel/hodjnpodj/views/resource_view.h"

namespace Bagel {
namespace HodjNPodj {

class SpritesView : public ResourceView {
	friend class Sprite;
private:
	Common::List<Sprite *> _linkedSprites;

protected:
	void drawSprites();

public:
	SpritesView(const Common::String &name, const Common::String &resFilename);
	virtual ~SpritesView() {}
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
