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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GOB_MINIGAMES_GEISHA_DIVING_H
#define GOB_MINIGAMES_GEISHA_DIVING_H

#include "common/system.h"

#include "gob/sound/sounddesc.h"

namespace Gob {

class GobEngine;
class Surface;
class DECFile;
class ANIFile;
class ANIObject;

enum MouseButtons;

namespace Geisha {

class EvilFish;

/** Geisha's "Diving" minigame. */
class Diving {
public:
	Diving(GobEngine *vm);
	~Diving();

	bool play(uint16 playerCount, bool hasPearlLocation);

private:
	static const uint kEvilFishCount =  3;
	static const uint kMaxShotCount  = 10;

	GobEngine *_vm;

	DECFile *_background;
	ANIFile *_objects;
	ANIFile *_gui;
	ANIFile *_oko;

	ANIObject *_water;
	ANIObject *_lungs;
	ANIObject *_heart;

	EvilFish *_evilFish[kEvilFishCount];

	ANIObject *_shot[kMaxShotCount];

	Common::List<int> _activeShots;

	Common::List<ANIObject *> _anims;

	Surface *_blackPearl;

	uint8 _whitePearlCount;
	uint8 _blackPearlCount;

	uint8 _currentShot;

	SoundDesc _soundShoot;
	SoundDesc _soundBreathe;
	SoundDesc _soundWhitePearl;
	SoundDesc _soundBlackPearl;


	void init();
	void deinit();

	void initScreen();
	void initCursor();

	void evilFishEnter();

	void foundBlackPearl();
	void foundWhitePearl();

	void updateAnims();

	int16 checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);

	void shoot(int16 mouseX, int16 mouseY);
	void checkShots();
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_DIVING_H
