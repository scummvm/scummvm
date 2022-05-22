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

#include "common/events.h"

#include "hypno/hypno.h"

namespace Hypno {

void BoyzEngine::runMenu(Hotspots *hs, bool only_menu) {
	Hotspot *h = hs->begin();
	assert(h->type == MakeMenu);
	if (!h->background.empty()) {
		loadImage(h->background, 0, 0, false, true, 1);
		if (h->backgroundFrames.empty()) {
			h->backgroundFrames = decodeFrames(h->background);
		}
	}
	renderHighlights(hs);
}

void BoyzEngine::renderHighlights(Hotspots *hs) {
	Hotspot *menu = hs->begin();
	if (menu->type != MakeMenu || menu->background.empty())
		return;

	for (Hotspots::const_iterator it = hs->begin(); it != hs->end(); ++it) {
		if (it->type == MakeMenu)
			continue;

		Highlight *hl;
		for (Actions::const_iterator itt = it->actions.begin(); itt != it->actions.end(); ++itt) {
			Action *action = *itt;
			switch (action->type) {
			case HighlightAction:
				hl = (Highlight *)action;
				if (_sceneState[hl->condition]) {
					Graphics::Surface sub = menu->backgroundFrames[0]->getSubArea(it->rect);
					drawImage(sub, it->rect.left, it->rect.top, false);
				}
				break;

			default:
				break;
			}
		}
	}
}

bool BoyzEngine::hoverHotspot(Common::Point mousePos) {
	if (_rnd->getRandomBit())
		return false; // Dirty trick to avoid updating the screen too often
	Hotspots *hots = stack.back();
	Hotspot selected(MakeHotspot);
	bool found = false;
	int rs = 100000000;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		int cs = h.rect.width() * h.rect.height();
		if (h.rect.contains(mousePos)) {
			if (cs < rs) {
				selected = h;
				found = true;
				rs = cs;
			}
		}
	}
	if (found) {
		Hotspot *menu = hots->begin();
		if (menu->type == MakeMenu && !menu->background.empty()) { // Hihghlight
			Graphics::Surface sub = menu->backgroundFrames[2]->getSubArea(selected.rect);
			drawImage(*menu->backgroundFrames[1], 0, 0, false);
            drawImage(sub, selected.rect.left, selected.rect.top, false);
			renderHighlights(hots);
			drawScreen();
		}
		return true;
	}
	return false;
}

} // End of namespace Hypno