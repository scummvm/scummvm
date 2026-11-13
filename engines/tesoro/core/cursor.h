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

#ifndef TESORO_CORE_CURSOR_H
#define TESORO_CORE_CURSOR_H

#include "tesoro/core/hovertext.h"
#include "tesoro/core/pointerstate.h"
#include "tesoro/core/spriteobject.h"

namespace Tesoro {

class World;
class Level;

/**
 * The mouse pointer. The game has two: the main one (object kind 10) and a plain
 * companion in the level the player is in, which does the actual collision.
 */
class Cursor : public SpriteObject {
public:
	~Cursor() override { delete _companion; }

	/** Builds the plain companion: an object that follows the mouse in the level the player is in. */
	void init();

	/** Builds the main pointer from its level record. */
	void initMain(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now);

	void updateAnimation(int32 now) override;
	AnimPlayer *currentPlayer() override { return _owner ? _owner->currentPlayer() : SpriteObject::currentPlayer(); }

	void onAttach() override;

	void draw(const Camera &camera, Display &display) override;

	/** The mouse to follow, and the level the player is in (the companion lives there). */
	void setSurroundings(const PointerState *mouse, Level *playerLevel) { _mouse = mouse; _playerLevel = playerLevel; }

	void setHost(World *host) { _host = host; }

	int update() override;

	/** Second pass step: lets both pointers act on the objects they touched. */
	int update2() override;

	/** The pointer hits whatever is marked as a hitter. */
	bool isHitter(const Object *other) const override { return (other->flags() & kHitter) != 0; }

	/** Notes an object under the pointer this frame. */
	int onTouched(Object *other, int extra) override;

	void setActive(bool active) override;

	/**
	 * Walks through the objects touched this frame, checking for hover and click actions.
	 * Returns what the last object returned: 0 for none, 3 asks the frame to stop.
	 */
	int processCandidates(byte buttons, int verb);

	/** Forgets the objects touched this frame (done once all of them were processed). */
	void clearTouched() { _touched.clear(); }

	HoverText &label() { return _label; }

	/** The objects under the pointer after the last collision pass. The companion has its own list. */
	const ObjectList &touched() const { return _touched; }
	const Cursor *companion() const { return _companion; }

private:
	HoverText _label;
	ObjectList _hovered;	// objects that keep the label or the click
	ObjectList _touched;	// objects under the pointer this frame
	Cursor *_companion = nullptr;
	Cursor *_owner = nullptr;		// for the companion: the main pointer
	const PointerState *_mouse = nullptr;
	Level *_playerLevel = nullptr;
	World *_host = nullptr;
};

} // End of namespace Tesoro

#endif
