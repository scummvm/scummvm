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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_MINIGAMES_GEISHA_OKO_H
#define GOB_MINIGAMES_GEISHA_OKO_H

#include "gob/aniobject.h"

namespace Gob {

class Sound;
class SoundDesc;

namespace Geisha {

/** Oko, the person you control, in Geisha's "Diving" minigame. */
class Oko : public ANIObject {
public:
	enum State {
		kStateEnter,
		kStateSwim,
		kStateSink,
		kStateRaise,
		kStateBreathe,
		kStatePick,
		kStateHurt,
		kStateDead
	};

	Oko(const ANIFile &ani, Sound &sound, SoundDesc &breathe);
	~Oko() override;

	/** Advance the animation to the next frame. */
	void advance() override;

	/** Oko should sink a level. */
	void sink();
	/** Oko should raise a level. */
	void raise();

	/** Oko should get hurt. */
	void hurt();

	/** Oko should die. */
	void die();

	State getState() const;

	bool isBreathing() const;
	bool isMoving() const;

private:
	Sound *_sound;
	SoundDesc *_breathe;

	State _state;

	uint8 _level;
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_OKO_H
