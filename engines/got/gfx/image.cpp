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

#include "got/gfx/image.h"
#include "common/memstream.h"
#include "got/events.h"
#include "got/got.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {

static void createSurface(Graphics::ManagedSurface &s, const byte *src) {
	s.create(16, 16);

	// Both 0 and 15 are transparent colors, so as we load the
	// surface, standard on a single color
	byte *dest = (byte *)s.getPixels();
	for (int i = 0; i < 16 * 16; ++i, ++src, ++dest)
		*dest = (*src == 15) ? 0 : *src;

	s.setTransparentColor(0);
}

void setupActor(Actor *actor, const char num, const char dir, const int x, const int y) {
	actor->_nextFrame = 0; // Next frame to be shown
	actor->_frameCount = actor->_frameSpeed;
	actor->_dir = dir;      // Direction of travel
	actor->_lastDir = dir; // Last direction of travel
	if (actor->_directions == 1)
		actor->_dir = 0;
	if (actor->_directions == 2)
		actor->_dir &= 1;
	if (actor->_directions == 4 && actor->_framesPerDirection == 1) {
		actor->_dir = 0;
		actor->_nextFrame = dir;
	}

	actor->_x = x;                         // Actual X coordinates
	actor->_y = y;                         // Actual Y coordinates
	actor->_width = 16;                    // Actual X coordinates
	actor->_height = 16;                   // Actual Y coordinates
	actor->_center = 0;                    // Center of object
	actor->_lastX[0] = x;                  // Last X coordinates on each page
	actor->_lastX[1] = x;
	actor->_lastY[0] = y;                  // Last Y coordinates on each page
	actor->_lastY[1] = y;
	actor->_active = true;                 // true=active, false=not active
	actor->_moveCountdown = 8;             // Count down to movement
	actor->_vulnerableCountdown = STAMINA; // Count down to vulnerability
	actor->_shotCountdown = 20;            // Count down to another shot
	actor->_currNumShots = 0;              // # of shots currently on screen
	actor->_creator = 0;                   // which actor # created this actor
	actor->_unpauseCountdown = 0;          // Pause must be 0 to move
	actor->_show = 0;
	actor->_actorNum = num;
	actor->_counter = 0;
	actor->_moveCounter = 0;
	actor->_edgeCounter = 20;
	actor->_hitThor = false;
	actor->_rand = g_engine->getRandomNumber(99);
	actor->_temp1 = 0;
	actor->_initHealth = actor->_health;
}

void make_actor_surface(Actor *actor) {
	assert(actor->_directions <= 4 && actor->_framesPerDirection <= 4);
	for (int d = 0; d < actor->_directions; d++) {
		for (int f = 0; f < actor->_framesPerDirection; f++) {
			Graphics::ManagedSurface &s = actor->pic[d][f];
			const byte *src = &_G(tmp_buff[256 * ((d * 4) + f)]);
			createSurface(s, src);
		}
	}
}

int loadStandardActors() {
	loadActor(0, 100 + _G(thor_info)._armor); // Load Thor
	_G(actor[0]).loadFixed(_G(tmp_buff) + 5120);
	setupActor(&_G(actor[0]), 0, 0, 100, 100);
	_G(thor) = &_G(actor[0]);

	make_actor_surface(&_G(actor[0]));

	_G(thor_x1) = _G(thor)->_x + 2;
	_G(thor_y1) = _G(thor)->_y + 2;
	_G(thor_x2) = _G(thor)->_x + 14;
	_G(thor_y2) = _G(thor)->_y + 14;

	loadActor(0, 103 + _G(thor_info)._armor); // Load hammer
	_G(actor[1]).loadFixed(_G(tmp_buff) + 5120);
	setupActor(&_G(actor[1]), 1, 0, 100, 100);
	_G(actor[1])._active = false;
	_G(hammer) = &_G(actor[1]);

	make_actor_surface(&_G(actor[1]));

	// Load sparkle
	loadActor(0, 106);
	_G(sparkle).loadFixed(_G(tmp_buff) + 5120);
	setupActor(&_G(sparkle), 20, 0, 100, 100);
	_G(sparkle)._active = false;
	make_actor_surface(&_G(sparkle));

	// Load explosion
	loadActor(0, 107);
	_G(explosion).loadFixed(_G(tmp_buff) + 5120);
	setupActor(&_G(explosion), 21, 0, 100, 100);
	_G(explosion)._active = false;
	make_actor_surface(&_G(explosion));

	// Load tornado
	loadActor(0, 108);
	_G(magic_item[0]).loadFixed((const byte *)_G(tmp_buff) + 5120);
	Common::copy(_G(tmp_buff), _G(tmp_buff) + 1024, _G(magic_pic[0]));

	setupActor(&_G(magic_item[0]), 20, 0, 0, 0);
	_G(magic_item[0])._active = false;

	// Load shield
	loadActor(0, 109);
	_G(magic_item[1]).loadFixed((const byte *)_G(tmp_buff) + 5120);
	Common::copy(_G(tmp_buff), _G(tmp_buff) + 1024, _G(magic_pic[1]));

	setupActor(&_G(magic_item[1]), 20, 0, 0, 0);
	_G(magic_item[1])._active = false;

	_G(actor[2])._active = false;

	make_actor_surface(&_G(magic_item[0]));

	return 1;
}

void showEnemies() {
	for (int i = 3; i < MAX_ACTORS; i++)
		_G(actor[i])._active = false;

	for (int i = 0; i < MAX_ENEMIES; i++)
		_G(enemy_type[i]) = 0;

	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn)._actorType[i] > 0) {
			const int id = loadEnemy(_G(scrn)._actorType[i]);
			if (id >= 0) {
				_G(actor[i + 3]) = _G(enemy[id]);

				const int d = _G(scrn)._actorDir[i];

				setupActor(&_G(actor[i + 3]), i + 3, d, (_G(scrn)._actorLoc[i] % 20) * 16,
							(_G(scrn)._actorLoc[i] / 20) * 16);
				_G(actor[i + 3])._initDir = _G(scrn)._actorDir[i];
				_G(actor[i + 3])._passValue = _G(scrn)._actorValue[i];

				if (_G(actor[i + 3])._moveType == 23) {
					// Spinball
					if (_G(actor[i + 3])._passValue & 1)
						_G(actor[i + 3])._moveType = 24;
				}

				if (_G(scrn)._actorInvis[i])
					_G(actor[i + 3])._active = false;
			}

			_G(etype[i]) = id;
		}
	}
}

int loadEnemy(const int type) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (_G(enemy_type[i]) == type)
			return i;
	}

	if (!loadActor(1, type)) {
		return -1;
	}

	int e = -1;
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (!_G(enemy_type[i])) {
			e = i;
			break;
		}
	}
	if (e == -1)
		return -1;

	Common::MemoryReadStream inf(_G(tmp_buff) + 5120, 40);
	_G(enemy[e]).loadFixed(&inf);

	make_actor_surface(&_G(enemy[e]));
	_G(enemy_type[e]) = type;
	_G(enemy[e])._shotType = 0;

	if (_G(enemy[e])._numShotsAllowed) {
		_G(enemy[e])._shotType = e + 1;

		// Set up shot info
		_G(shot[e]).loadFixed(_G(tmp_buff) + 5160);

		// Loop to set up graphics
		for (int d = 0; d < _G(shot[e])._directions; d++) {
			for (int f = 0; f < _G(shot[e])._framesPerDirection; f++) {
				if (_G(shot[e])._directions < _G(shot[e])._framesPerDirection) {
					Graphics::ManagedSurface &s = _G(shot[e]).pic[d][f];
					const byte *src = &_G(tmp_buff[4096 + (256 * ((d * 4) + f))]);
					createSurface(s, src);
				} else {
					Graphics::ManagedSurface &s = _G(shot[e]).pic[f][d];
					const byte *src = &_G(tmp_buff[4096 + (256 * ((f * 4) + d))]);
					createSurface(s, src);
				}
			}
		}
	}

	return e;
}

int actorVisible(const int invisNum) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn)._actorInvis[i] == invisNum) {
			const int etype = _G(etype[i]);
			if (etype >= 0 && !_G(actor[i + 3])._active) {
				_G(actor[i + 3]) = _G(enemy[etype]);

				int d = _G(scrn)._actorDir[i];
				setupActor(&_G(actor[i + 3]), i + 3, d, (_G(scrn)._actorLoc[i] % 20) * 16,
							(_G(scrn)._actorLoc[i] / 20) * 16);
				_G(actor[i + 3])._initDir = _G(scrn)._actorDir[i];
				_G(actor[i + 3])._passValue = _G(scrn)._actorValue[i];
				return i;
			}

			return -1;
		}
	}
	return -1;
}

void setupMagicItem(const int item) {
	for (int i = 0; i < 4; i++) {
		createSurface(_G(magic_item[item]).pic[i / 4][i % 4], &_G(magic_pic[item][256 * i]));
	}
}

void loadNewThor() {
	loadActor(0, 100 + _G(thor_info)._armor); // Load Thor

	make_actor_surface(&_G(actor[0]));
}

} // namespace Got
