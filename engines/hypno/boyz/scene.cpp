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

const char *sceneVariablesBoyz[] = {
	"GS_NONE",
	"GS_SCTEXT",
	"GS_AMBIENT",
	"GS_MUSIC",
	"GS_VOLUME",
	"GS_LEVELCOMPLETE",
	"GS_LEVELWON",
	"GS_HOLDMOUSE",
	"GS_DIFFICULTY",
	"GS_TERRITORY",
	"GS_SECTOR",
	"GS_HITPOINTS",
	"GS_TERRITORY1_RAND",
	"GS_C5MAP",
	"GS_WONSHELLGAME",
	"GS_C36_READY",
	"GS_MINEMAP",
	"GS_MINEMAP_VIEWED",
	"GS_HOTELDONE",
	"GS_SEQ_11",
	"GS_SEQ_12",
	"GS_SEQ_13",
	"GS_SEQ_14",
	"GS_SEQ_15",
	"GS_SEQ_16",
	"GS_SEQ_17",
	"GS_SEQ_18",
	"GS_SEQ_19",
	"GS_SEQ_21",
	"GS_SEQ_22",
	"GS_SEQ_31",
	"GS_SEQ_32",
	"GS_SEQ_33",
	"GS_SEQ_34",
	"GS_SEQ_35",
	"GS_SEQ_351",
	"GS_SEQ_352",
	"GS_SEQ_353",
	"GS_SEQ_354",
	"GS_SEQ_355",
	"GS_SEQ_36",
	"GS_SEQ_41",
	"GS_SEQ_42",
	"GS_SEQ_51",
	"GS_SEQ_52",
	"GS_SEQ_53",
	"GS_SEQ_54",
	"GS_SEQ_55",
	"GS_SEQ_56",
	"GS_SEQ_57",
	"GS_SEQ_58",
	"GS_SEQ_59",
	nullptr
};

void BoyzEngine::resetSceneState() {
	uint32 i = 0;
	while (sceneVariablesBoyz[i]) {
		_sceneState[sceneVariablesBoyz[i]] = 0;
		i++;
	}
	_intros.clear();
}

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
				assert(_sceneState.contains(hl->condition));
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