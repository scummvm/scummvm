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

#ifndef MUTATIONOFJB_GAMEWIDGET_H
#define MUTATIONOFJB_GAMEWIDGET_H

#include "mutationofjb/widgets/widget.h"

namespace MutationOfJB {

class GameWidget;
struct Door;
struct Static;

class GameWidgetCallback {
public:
	virtual ~GameWidgetCallback() {}
	virtual void onGameDoorClicked(GameWidget *, Door *door) = 0;
	virtual void onGameStaticClicked(GameWidget *, Static *stat) = 0;
	virtual void onGameEntityHovered(GameWidget *, const Common::String &entity) = 0;
};

class GameWidget : public Widget {
public:
	enum {
		GAME_NORMAL_AREA_WIDTH = 320,
		GAME_NORMAL_AREA_HEIGHT = 139,
		GAME_FULL_AREA_WIDTH = 320,
		GAME_FULL_AREA_HEIGHT = 200
	};

	enum DirtyFlags {
		DIRTY_AFTER_SCENE_CHANGE = 1 << 1,
		DIRTY_MAP_SELECTION = 1 << 2
	};

	GameWidget(GuiScreen &gui);
	void setCallback(GameWidgetCallback *callback) {
		_callback = callback;
	}

	void handleEvent(const Common::Event &) override;

	void clearState();
protected:
	void draw(Graphics::ManagedSurface &) override;

private:
	/**
	 * Handling for normal (non-map) scenes.
	 *
	 * Statics and doors define mouse clickable areas.
	 * Statics are used to start actions.
	 * Doors are used to transition between scenes.
	 *
	 * @param event ScummVM event.
	 */
	void handleNormalScene(const Common::Event &event);

	/**
	 * Special handling for map scenes.
	 *
	 * Bitmaps define mouse clickable areas.
	 * Statics are used to start actions.
	 * Objects are used for showing labels.
	 *
	 * @param event ScummVM event.
	 */
	void handleMapScene(const Common::Event &event);

	uint8 _currentMapObjectId;
	uint8 _nextMapObjectId;
	GameWidgetCallback *_callback;
};

}

#endif
