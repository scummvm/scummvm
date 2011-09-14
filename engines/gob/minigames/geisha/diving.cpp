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

#include "common/list.h"

#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/anifile.h"

#include "gob/sound/sound.h"

#include "gob/minigames/geisha/evilfish.h"
#include "gob/minigames/geisha/diving.h"

namespace Gob {

namespace Geisha {

static const int kEvilFishTypeCount = 3;

static const int kEvilFishTypes[kEvilFishTypeCount][5] = {
	{ 0, 14,  8,  9, 3}, // Shark
	{15,  1, 12, 13, 3}, // Moray
	{16,  2, 10, 11, 3}  // Ray
};


Diving::Diving(GobEngine *vm) : _vm(vm), _background(0),
	_objects(0), _gui(0), _oko(0), _lungs(0), _heart(0),
	_blackPearl(0), _whitePearlCount(0), _blackPearlCount(0) {

	_blackPearl = new Surface(11, 8, 1);
}

Diving::~Diving() {
	delete _blackPearl;

	deinit();
}

bool Diving::play(uint16 playerCount, bool hasPearlLocation) {
	init();
	initScreen();
	initCursor();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit()) {
		evilFishEnter();

		checkShots();

		updateAnims();

		_vm->_draw->animateCursor(1);

		_vm->_draw->blitInvalidated();

		_vm->_util->waitEndFrame();
		_vm->_util->processInput();

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if (key == kKeyEscape)
			break;

		if (mouseButtons == kMouseButtonsLeft)
			shoot(mouseX, mouseY);

		if ((_whitePearlCount >= 20) || (_blackPearlCount >= 2))
			break;
	}

	deinit();
	return _blackPearlCount >= 2;
}

void Diving::init() {
	_background = new DECFile(_vm, "tperle.dec"  , 320, 200);
	_objects    = new ANIFile(_vm, "tperle.ani"  , 320);
	_gui        = new ANIFile(_vm, "tperlcpt.ani", 320);
	_oko        = new ANIFile(_vm, "tplonge.ani" , 320);

	_water = new ANIObject(*_objects);
	_lungs = new ANIObject(*_gui);
	_heart = new ANIObject(*_gui);

	_water->setAnimation(7);
	_water->setPosition();
	_water->setVisible(true);

	_lungs->setAnimation(0);
	_lungs->setPosition();
	_lungs->setVisible(true);
	_lungs->setPause(true);

	_heart->setAnimation(1);
	_heart->setPosition();
	_heart->setVisible(true);
	_heart->setPause(true);

	for (uint i = 0; i < kEvilFishCount; i++)
		_evilFish[i] = new EvilFish(*_objects, 320, 0, 0, 0, 0, 0);

	for (uint i = 0; i < kMaxShotCount; i++) {
		_shot[i] = new ANIObject(*_objects);

		_shot[i]->setAnimation(17);
		_shot[i]->setMode(ANIObject::kModeOnce);
	}

	Surface tmp(320, 103, 1);

	_vm->_video->drawPackedSprite("tperlobj.cmp", tmp);

	_blackPearl->blit(tmp, 282, 80, 292, 87, 0, 0);

	_blackPearlCount = 0;

	_currentShot = 0;

	_anims.push_back(_water);
	for (uint i = 0; i < kMaxShotCount; i++)
		_anims.push_back(_shot[i]);
	for (uint i = 0; i < kEvilFishCount; i++)
		_anims.push_back(_evilFish[i]);
	_anims.push_back(_lungs);
	_anims.push_back(_heart);

	_vm->_sound->sampleLoad(&_soundShoot     , SOUND_SND, "tirgim.snd");
	_vm->_sound->sampleLoad(&_soundBreathe   , SOUND_SND, "respir.snd");
	_vm->_sound->sampleLoad(&_soundWhitePearl, SOUND_SND, "virtou.snd");
	_vm->_sound->sampleLoad(&_soundBlackPearl, SOUND_SND, "trouve.snd");
}

void Diving::deinit() {
	_vm->_draw->_cursorHotspotX = -1;
	_vm->_draw->_cursorHotspotY = -1;

	_soundShoot.free();
	_soundBreathe.free();
	_soundWhitePearl.free();
	_soundBlackPearl.free();

	_anims.clear();

	_activeShots.clear();

	for (uint i = 0; i < kMaxShotCount; i++) {
		delete _shot[i];

		_shot[i] = 0;
	}

	for (uint i = 0; i < kEvilFishCount; i++) {
		delete _evilFish[i];

		_evilFish[i] = 0;
	}

	delete _heart;
	delete _lungs;
	delete _water;

	delete _oko;
	delete _gui;
	delete _objects;
	delete _background;

	_water = 0;
	_heart = 0;
	_lungs = 0;

	_oko        = 0;
	_gui        = 0;
	_objects    = 0;
	_background = 0;
}

void Diving::initScreen() {
	_vm->_util->setFrameRate(15);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface->clear();
	_background->draw(*_vm->_draw->_backSurface);

	int16 left, top, right, bottom;
	_lungs->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_heart->draw(*_vm->_draw->_backSurface, left, top, right, bottom);

	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

void Diving::initCursor() {
	const int index = _vm->_draw->_cursorIndex;

	const int16 left   = index * _vm->_draw->_cursorWidth;
	const int16 top    = 0;
	const int16 right  = left + _vm->_draw->_cursorWidth - 1;
	const int16 bottom = _vm->_draw->_cursorHeight - 1;

	_vm->_draw->_cursorSprites->fillRect(left, top, right, bottom, 0);

	_objects->draw(*_vm->_draw->_cursorSprites, 31, 0, left, top);
	_vm->_draw->_cursorAnimLow[index] = 0;

	_vm->_draw->_cursorHotspotX = 8;
	_vm->_draw->_cursorHotspotY = 8;
}

void Diving::evilFishEnter() {
	for (uint i = 0; i < kEvilFishCount; i++) {
		EvilFish &fish = *_evilFish[i];

		if (fish.isVisible())
			continue;

		int fishType = _vm->_util->getRandom(kEvilFishTypeCount);
		fish.mutate(kEvilFishTypes[fishType][0], kEvilFishTypes[fishType][1],
		            kEvilFishTypes[fishType][2], kEvilFishTypes[fishType][3],
		            kEvilFishTypes[fishType][4]);

		fish.enter((EvilFish::Direction)_vm->_util->getRandom(2), 90 + i * 20);
	}
}

void Diving::foundBlackPearl() {
	_blackPearlCount++;

	if        (_blackPearlCount == 1) {
		_vm->_draw->_backSurface->blit(*_blackPearl, 0, 0, 10, 7, 147, 179, 0);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 147, 179, 157, 186);
	} else if (_blackPearlCount == 2) {
		_vm->_draw->_backSurface->blit(*_blackPearl, 0, 0, 10, 7, 160, 179, 0);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 147, 179, 160, 186);
	}
}

void Diving::foundWhitePearl() {
	_whitePearlCount++;

	int16 x = 54 + (_whitePearlCount - 1) * 8;
	if (_whitePearlCount > 10)
		x += 48;

	_background->drawLayer(*_vm->_draw->_backSurface, 0, 2, x, 177, 0);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, 177, x + 3, 180);
}

void Diving::updateAnims() {
	int16 left, top, right, bottom;

	// Clear the previous animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.reverse_begin();
			 a != _anims.end(); --a) {

		(*a)->clear(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}

	// Draw the current animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.begin();
			 a != _anims.end(); ++a) {

		(*a)->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		(*a)->advance();
	}
}

int16 Diving::checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);

	return _vm->_util->checkKey();
}

void Diving::shoot(int16 mouseX, int16 mouseY) {
	// Outside the playable area?
	if (mouseY > 157)
		return;

	// Too many shots still active?
	if (_activeShots.size() >= kMaxShotCount)
		return;

	ANIObject &shot = *_shot[_currentShot];

	shot.rewind();
	shot.setVisible(true);
	shot.setPause(false);
	shot.setPosition(mouseX - 8, mouseY - 8);

	_activeShots.push_back(_currentShot);

	_currentShot = (_currentShot + 1) % kMaxShotCount;

	_vm->_sound->blasterPlay(&_soundShoot, 1, 0);
}

void Diving::checkShots() {
	Common::List<int>::iterator activeShot = _activeShots.begin();

	while (activeShot != _activeShots.end()) {
		ANIObject &shot = *_shot[*activeShot];

		if (shot.lastFrame()) {
			int16 x, y;

			shot.getPosition(x, y);

			for (uint i = 0; i < kEvilFishCount; i++) {
				EvilFish &evilFish = *_evilFish[i];

				if (evilFish.isIn(x + 8, y + 8)) {
					evilFish.die();

					break;
				}
			}

			activeShot = _activeShots.erase(activeShot);
		} else
			++activeShot;
	}
}

} // End of namespace Geisha

} // End of namespace Gob
