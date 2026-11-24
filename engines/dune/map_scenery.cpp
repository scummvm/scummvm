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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dune/dune.h"
#include "dune/sprite_sheet.h"

namespace Dune {

uint DuneEngine::distanceFromPaul(uint16 longitude, int latitude) const {
	// 407E/7C8F: signed longitude wrap, divided by Paul's latitude scale.
	uint16 ours = READ_LE_UINT16(_state.begin() + 4);
	int row = (int8)_state[6];
	if (_state[6] == 0x80) {
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here < 0x100 || (uint)here + 6 > _state.size())
			return 0xffff;
		ours = READ_LE_UINT16(_state.begin() + here + 2);
		row = (int16)READ_LE_UINT16(_state.begin() + here + 4);
	}
	const uint across = ABS((int)(int16)(longitude - ours)) / _map.step(ABS(row));
	return MAX(across, (uint)ABS(row - latitude));
}

void DuneEngine::drawMapScenery() {
	const Common::Rect view(4, 4, 316, 148);
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	openSpriteSheet("ONMAP.HSQ", false);
	_map.drawVegetation(_worldLongitude, _worldLatitude,
			[](uint sprite, int x, int y, void *engine) {
				static_cast<DuneEngine *>(engine)->drawSpriteCentred(sprite, x, y);
			}, this);

	for (uint i = 0; i < _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		const Common::Point at = placeOnScreen(place);
		if (!place.isKnown() || !view.contains(at))
			continue;
		uint sprite = 122 + placeClass(place.kind); // 5E42: ONMAP, not ICONES.
		if (place.kind < 0x20 && distanceFromPaul(place.longitude, place.latitude) >
				READ_LE_UINT16(_state.begin() + 0x1176))
			sprite += 5;
		drawSpriteCentred(sprite, at.x, at.y);
	}

	const bool flying = _state[0x487e] == 2 && (_state[0x11c9] & 3);
	const Common::Point at = playerMapPosition();
	if (flying) {
		// The flight inset's ICONES 46/47 are the three-pixel aircraft and
		// gray route mark. ONMAP 152 is a palette-pattern tile, not an orni.
		openSpriteSheet("ICONES.HSQ", false);
		for (uint i = 0; i + 1 < _flightTrail.size(); i++) {
			const Common::Point p = _map.strategicPosition(_worldLongitude, _worldLatitude,
					(uint16)_flightTrail[i].x, _flightTrail[i].y);
			if (p.x > view.left && p.x < view.right - 1 &&
					p.y > view.top && p.y < view.bottom - 1)
				drawSprite(47, p.x - 1, p.y - 1);
		}
		if (at.x > view.left && at.x < view.right - 1 &&
				at.y > view.top && at.y < view.bottom - 1)
			drawSprite(46, at.x - 1, at.y - 1);
	} else if (view.contains(at)) {
		openSpriteSheet("ICONES.HSQ", false);
		int width, height;
		if (currentSpriteSheet->size(76, &width, &height))
			// 6330 subtracts in BL, without borrowing into BH.
			drawSprite(76, at.x - 13, (byte)(at.y - height));
	}
	for (int y = view.top; y < view.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + view.left,
				_framebuffer2 + y * kScreenWidth + view.left, view.width());
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("ONMAP.HSQ", false);
}

} // namespace Dune
