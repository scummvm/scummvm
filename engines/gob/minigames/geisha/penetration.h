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

#ifndef GOB_MINIGAMES_GEISHA_PENETRATION_H
#define GOB_MINIGAMES_GEISHA_PENETRATION_H

#include "common/system.h"
#include "common/list.h"

namespace Gob {

class GobEngine;
class Surface;
class CMPFile;
class ANIFile;

namespace Geisha {

class Meter;
class Mouth;

/** Geisha's "Penetration" minigame. */
class Penetration {
public:
	Penetration(GobEngine *vm);
	~Penetration();

	bool play(bool hasAccessPass, bool hasMaxEnergy, bool testMode);

private:
	static const int kModeCount  = 2;
	static const int kFloorCount = 3;

	static const int kMapWidth  = 17;
	static const int kMapHeight = 13;

	static const byte kMaps[kModeCount][kFloorCount][kMapWidth * kMapHeight];

	struct Position {
		uint16 x;
		uint16 y;

		Position(uint16 pX, uint16 pY);
	};

	enum MouthType {
		kMouthTypeBite,
		kMouthTypeKiss
	};

	struct ManagedMouth : public Position {
		Mouth *mouth;
		MouthType type;

		ManagedMouth(uint16 pX, uint16 pY, MouthType t);
		~ManagedMouth();
	};

	GobEngine *_vm;

	bool _hasAccessPass;
	bool _hasMaxEnergy;
	bool _testMode;

	Surface *_background;
	CMPFile *_sprites;
	ANIFile *_objects;

	ANIObject *_sub;

	Common::List<ANIObject *> _anims;
	Common::List<ANIObject *> _mapAnims;

	Meter *_shieldMeter;
	Meter *_healthMeter;

	uint8 _floor;

	Surface *_map;
	byte _mapTiles[kMapWidth * kMapHeight];

	uint16 _mapX;
	uint16 _mapY;

	uint8 _subTileX;
	uint8 _subTileY;

	Common::List<Position>     _shields;
	Common::List<ManagedMouth> _mouths;


	void init();
	void deinit();

	void createMap();

	void initScreen();

	void updateAnims();

	int16 checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);

	void handleSub(int16 key);
	void moveSub(int x, int y, uint16 animation);

	bool isWalkable(byte tile) const;

	void checkShields();
	void checkMouths();
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_PENETRATION_H
