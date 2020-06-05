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

#ifndef PINK_ACTION_PLAY_WITH_SFX_H
#define PINK_ACTION_PLAY_WITH_SFX_H

#include "pink/objects/actions/action_play.h"
#include "pink/sound.h"

namespace Pink {

class ActionSfx;

class ActionPlayWithSfx : public ActionPlay {
public:
	ActionPlayWithSfx()
		: _isLoop(false) {}
	~ActionPlayWithSfx() override;

	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void update() override;

	void end() override;

protected:
	void onStart() override;

private:
	Array<ActionSfx *> _sfxArray;
	bool _isLoop;
};

class Page;

class ActionSfx : public Object {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void play();
	void end();

	int32 getFrame() { return _frame; }

private:
	ActionPlayWithSfx *_sprite;
	Common::String _sfxName;
	Sound _sound;
	byte _volume;
	int32 _frame;
};

} // End of namespace Pink

#endif
