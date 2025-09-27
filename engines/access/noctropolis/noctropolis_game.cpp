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

#include "access/noctropolis/noctropolis_game.h"

namespace Access {

namespace Noctropolis {


NoctropolisEngine::NoctropolisEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
AccessEngine(syst, gameDesc)
{
}

NoctropolisEngine::~NoctropolisEngine() {
}

void NoctropolisEngine::playGame() {
	doIntro();
}

void NoctropolisEngine::doIntro() {
	static const int lettersX[] = {106, 153, 197, 229, 271, 309, 357, 394, 443, 476, 504};
	static const int titlesSpriteX[] = {238, 237, 237, 200, 200, 216, 231, 207, 198, 201, 234, 200, 220, 235, 214};
	static const int titlesSpriteY[] = {108, 112, 125, 128, 128, 124, 128, 115, 131, 130, 102, 112, 112, 102, 112};

	int16 skylineSpriteOfsX1 = 740;
	int16 skylineSpriteOfsX2 = 1050;
	int16 skylineSpriteOfsX3 = 1250;
	int16 skylineSpriteOfsX4 = 1300;
	int16 skylineSpriteOfsX5 = 1500;
	int16 titlesSpriteIndex = 0;
	int lettersMax = 0;

	_timers[26]._initTm = 45;
	_timers[27]._initTm = 7;
	_timers[28]._initTm = 240;

	// TODO: Music 98, 1
	Resource *sceneRes = _files->loadSubFile("scene01.ap", 0);
	
	//_room->loadRoom();

	Resource *spriteRes = _files->loadSubFile("scene01.ap", 1);
	SpriteResource *sprites = new SpriteResource(this, spriteRes);
	delete spriteRes;

	// TODO: Check these fades
	_screen->fadeOut();
	_screen->setPalette();
	_screen->fadeIn();
	copyBF1BF2();
	copyBF2Vid();

	_timers[26].reset();
	_timers[27].reset();

	while (true) {

		if (!_timers[27].isActive()) {
			_timers[27].reset();
			if (_scrollX == _room->_playFieldWidth - _screen->w || _events->_leftButton || _events->_rightButton)
				break;

			_scrollX += 2;
			
			copyBF1BF2();
			
			for (int i = 0; i < lettersMax; i++) {
				_buffer2.plotImage(sprites, i + 8, Common::Point(lettersX[i], 40));
			}

			if (!_timers[26].isActive()) {
				if (lettersMax <= 10) {
					if (lettersMax == 9)
						_timers[28].reset();
					_timers[26].reset();
					lettersMax++;
				} else if (titlesSpriteIndex < 25) {
					_buffer2.plotImage(sprites, titlesSpriteIndex + 19, Common::Point(titlesSpriteX[titlesSpriteIndex], titlesSpriteY[titlesSpriteIndex]));
					if (!_timers[28].isActive()) {
						titlesSpriteIndex++;
						_timers[26].reset();
						_timers[28].reset();
					}
				}

			}
			
			if (skylineSpriteOfsX1 > -100) {
				_buffer2.plotImage(sprites, 0, Common::Point(skylineSpriteOfsX1, 60));
				skylineSpriteOfsX1 -= 5;
			}
			if (skylineSpriteOfsX2 > -100) {
				_buffer2.plotImage(sprites, 3, Common::Point(skylineSpriteOfsX2, 16));
				skylineSpriteOfsX2 -= 8;
			}
			if (skylineSpriteOfsX4 > -100) {
				_buffer2.plotImage(sprites, 1, Common::Point(skylineSpriteOfsX4, 145));
				skylineSpriteOfsX4 -= 7;
			}
			if (skylineSpriteOfsX3 > -100) {
				_buffer2.plotImage(sprites, 4, Common::Point(skylineSpriteOfsX3, 115));
				skylineSpriteOfsX3 -= 5;
			}
			if (skylineSpriteOfsX5 > -100) {
				_buffer2.plotImage(sprites, 2, Common::Point(skylineSpriteOfsX5, 126));
				skylineSpriteOfsX5 -= 7;
			}

			copyBF2Vid();

		}

		_events->pollEventsAndWait();
	}

	_screen->fadeOut();

	delete sprites;

}

} // end namespace Noctropolis

} // end namespace Access
