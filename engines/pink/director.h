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

#ifndef PINK_DIRECTOR_H
#define PINK_DIRECTOR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/screen.h"

namespace Pink {

class Actor;
class ActionCEL;
class ActionSound;

class Director {
public:
	Director();

	void update();

	void setPallette(const byte *pallete) { g_system->getPaletteManager()->setPalette(pallete, 0, 256); }

	void addSprite(ActionCEL *sprite);
	void removeSprite(ActionCEL *sprite);

	void addSound(ActionSound* sound) { _sounds.push_back(sound); };
	void removeSound(ActionSound* sound);

	void clear();

	void pause(bool pause);

	void saveStage();
	void loadStage();

	Actor *getActorByPoint(const Common::Point point);

	Graphics::MacWindowManager &getWndManager() { return _wndManager; };

	uint32 count = 0;
	uint32 sum = 0;

private:
	void draw();
	void addDirtyRects(ActionCEL *sprite);
	void mergeDirtyRects();
	void drawRect(const Common::Rect &rect);

	Graphics::Screen _surface;
	Graphics::MacWindowManager _wndManager;
	Common::Array<Common::Rect> _dirtyRects;
	Common::Array<ActionCEL *> _sprites;
	Common::Array<ActionCEL *> _savedSprites;
	Common::Array<ActionSound *> _sounds;
};

} // End of namespace Pink


#endif
