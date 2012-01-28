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
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/anifile.h"

#include "gob/sound/sound.h"

#include "gob/minigames/geisha/evilfish.h"
#include "gob/minigames/geisha/oko.h"
#include "gob/minigames/geisha/meter.h"
#include "gob/minigames/geisha/diving.h"

namespace Gob {

namespace Geisha {

static const uint8 kAirDecreaseRate = 15;


const uint16 Diving::kEvilFishTypes[kEvilFishTypeCount][5] = {
	{ 0, 14,  8,  9, 3}, // Shark
	{15,  1, 12, 13, 3}, // Moray
	{16,  2, 10, 11, 3}  // Ray
};

const uint16 Diving::kPlantLevel1[] = { 18, 19, 20, 21 };
const uint16 Diving::kPlantLevel2[] = { 22, 23, 24, 25 };
const uint16 Diving::kPlantLevel3[] = { 26, 27, 28, 29, 30 };

const Diving::PlantLevel Diving::kPlantLevels[] = {
	{ 150, ARRAYSIZE(kPlantLevel1), kPlantLevel1 },
	{ 120, ARRAYSIZE(kPlantLevel2), kPlantLevel2 },
	{ 108, ARRAYSIZE(kPlantLevel3), kPlantLevel3 },
};


Diving::Diving(GobEngine *vm) : _vm(vm), _background(0),
	_objects(0), _gui(0), _okoAnim(0), _lungs(0), _heart(0),
	_blackPearl(0), _airMeter(0), _healthMeter(0),
	_whitePearlCount(0), _blackPearlCount(0) {

	_blackPearl = new Surface(11, 8, 1);

	_airMeter    = new Meter(4  , 195, 38, 2, 5, 7, 38, Meter::kFillToLeft);
	_healthMeter = new Meter(276, 195, 38, 2, 6, 7, 38, Meter::kFillToLeft);

	for (uint i = 0; i < kEvilFishCount; i++)
		_evilFish[i].evilFish = 0;

	for (uint i = 0; i < kDecorFishCount; i++)
		_decorFish[i].decorFish = 0;

	for (uint i = 0; i < kPlantCount; i++)
		_plant[i].plant = 0;

	for (uint i = 0; i < kMaxShotCount; i++)
		_shot[i] = 0;

	_pearl.pearl = 0;

	_oko = 0;
}

Diving::~Diving() {
	delete _airMeter;
	delete _healthMeter;

	delete _blackPearl;

	deinit();
}

bool Diving::play(uint16 playerCount, bool hasPearlLocation) {
	_hasPearlLocation = hasPearlLocation;

	init();
	initScreen();
	initCursor();
	initPlants();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit()) {
		checkShots();
		updateAirMeter();
		updateEvilFish();
		updateDecorFish();
		updatePlants();
		updatePearl();
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

		if (key == kKeyDown) {
			_oko->sink();
			if ((_oko->getState() == Oko::kStatePick) && (_oko->getFrame() == 0))
				getPearl();

		} else if (key == kKeyUp)
			_oko->raise();

		if ((_whitePearlCount >= 20) || (_blackPearlCount >= 2))
			break;
	}

	deinit();
	return _blackPearlCount >= 2;
}

void Diving::init() {
	_vm->_sound->sampleLoad(&_soundShoot     , SOUND_SND, "tirgim.snd");
	_vm->_sound->sampleLoad(&_soundBreathe   , SOUND_SND, "respir.snd");
	_vm->_sound->sampleLoad(&_soundWhitePearl, SOUND_SND, "virtou.snd");
	_vm->_sound->sampleLoad(&_soundBlackPearl, SOUND_SND, "trouve.snd");

	_background = new DECFile(_vm, "tperle.dec"  , 320, 200);
	_objects    = new ANIFile(_vm, "tperle.ani"  , 320);
	_gui        = new ANIFile(_vm, "tperlcpt.ani", 320);
	_okoAnim    = new ANIFile(_vm, "tplonge.ani" , 320);

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

	for (uint i = 0; i < kEvilFishCount; i++) {
		_evilFish[i].enterAt = 0;
		_evilFish[i].leaveAt = 0;

		_evilFish[i].evilFish = new EvilFish(*_objects, 320, 0, 0, 0, 0, 0);
	}

	for (uint i = 0; i < kDecorFishCount; i++) {
		_decorFish[i].enterAt = 0;

		_decorFish[i].decorFish = new ANIObject(*_objects);
	}

	for (uint i = 0; i < kPlantCount; i++) {
		_plant[i].level   = i / kPlantPerLevelCount;
		_plant[i].deltaX  = (kPlantLevelCount - _plant[i].level) * -2;

		_plant[i].x = -1;
		_plant[i].y = -1;

		_plant[i].plant = new ANIObject(*_objects);
	}

	_pearl.pearl = new ANIObject(*_objects);
	_pearl.black = false;

	_pearl.pearl->setAnimation(4);

	_decorFish[0].decorFish->setAnimation( 6); // Jellyfish
	_decorFish[0].deltaX = 0;

	_decorFish[1].decorFish->setAnimation(32); // Swarm of red/green fish
	_decorFish[1].deltaX = -5;

	_decorFish[2].decorFish->setAnimation(33); // Swarm of orange fish
	_decorFish[2].deltaX = -5;

	for (uint i = 0; i < kMaxShotCount; i++) {
		_shot[i] = new ANIObject(*_objects);

		_shot[i]->setAnimation(17);
		_shot[i]->setMode(ANIObject::kModeOnce);
	}

	_oko = new Oko(*_okoAnim, *_vm->_sound, _soundBreathe);

	Surface tmp(320, 103, 1);

	_vm->_video->drawPackedSprite("tperlobj.cmp", tmp);

	_blackPearl->blit(tmp, 282, 80, 292, 87, 0, 0);

	_blackPearlCount = 0;

	_currentShot = 0;

	_anims.push_back(_water);
	for (uint i = 0; i < kMaxShotCount; i++)
		_anims.push_back(_shot[i]);
	_anims.push_back(_pearl.pearl);
	for (uint i = 0; i < kDecorFishCount; i++)
		_anims.push_back(_decorFish[i].decorFish);
	for (uint i = 0; i < kEvilFishCount; i++)
		_anims.push_back(_evilFish[i].evilFish);
	for (int i = kPlantCount - 1; i >= 0; i--)
		_anims.push_back(_plant[i].plant);
	_anims.push_back(_oko);
	_anims.push_back(_lungs);
	_anims.push_back(_heart);

	_airMeter->setValue(38);
	_healthMeter->setValue(38);

	_airCycle = 0;
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
		delete _evilFish[i].evilFish;

		_evilFish[i].evilFish = 0;
	}

	for (uint i = 0; i < kDecorFishCount; i++) {
		delete _decorFish[i].decorFish;

		_decorFish[i].decorFish = 0;
	}

	for (uint i = 0; i < kPlantCount; i++) {
		delete _plant[i].plant;

		_plant[i].plant = 0;
	}

	delete _pearl.pearl;
	_pearl.pearl = 0;

	delete _oko;
	_oko = 0;

	delete _heart;
	delete _lungs;
	delete _water;

	delete _okoAnim;
	delete _gui;
	delete _objects;
	delete _background;

	_water = 0;
	_heart = 0;
	_lungs = 0;

	_okoAnim    = 0;
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


void Diving::initPlants() {
	for (uint i = 0; i < kPlantLevelCount; i++) {
		for (uint j = 0; j < kPlantPerLevelCount; j++) {
			int16 prevPlantX = -100;
			if (j > 0)
				prevPlantX = _plant[i * kPlantPerLevelCount + j - 1].x;

			enterPlant(_plant[i * kPlantPerLevelCount + j], prevPlantX);
		}
	}
}

void Diving::enterPlant(ManagedPlant &plant, int16 prevPlantX) {
	const PlantLevel &level = kPlantLevels[plant.level];
	const uint anim = level.plants[_vm->_util->getRandom(kPlantLevels[plant.level].plantCount)];

	plant.plant->setAnimation(anim);
	plant.plant->rewind();

	int16 width, height;
	plant.plant->getFrameSize(width, height);

	plant.x = prevPlantX + 150 - 10 + _vm->_util->getRandom(21);
	plant.y = kPlantLevels[plant.level].y - height;

	plant.plant->setPosition(plant.x, plant.y);
	plant.plant->setVisible(true);
	plant.plant->setPause(false);

	if (plant.x > 320)
		enterPearl(plant.x);
}

void Diving::enterPearl(int16 x) {
	// Only one pearl is ever visible
	if (_pearl.pearl->isVisible())
		return;

	// Only every 4th potential pearl position has a pearl
	if (_vm->_util->getRandom(4) != 0)
		return;

	// Every 5th pearl is a black one, but only if the location is correct
	_pearl.black = _hasPearlLocation && (_vm->_util->getRandom(5) == 0);

	_pearl.pearl->setPosition(x + 80, 130);

	_pearl.pearl->setVisible(true);
	_pearl.pearl->setPause(false);
}

void Diving::updateAirMeter() {
	if (_oko->isBreathing()) {
		_airCycle = 0;
		_airMeter->increase();
		return;
	}

	_airCycle = (_airCycle + 1) % kAirDecreaseRate;

	if (_airCycle == 0)
		_airMeter->decrease();

	if (_airMeter->getValue() == 0)
		_oko->die();
}

void Diving::updateEvilFish() {
	for (uint i = 0; i < kEvilFishCount; i++) {
		ManagedEvilFish &fish = _evilFish[i];

		if (fish.evilFish->isVisible()) {
			// Evil fishes leave on their own after 30s - 40s

			fish.enterAt = 0;

			if (fish.leaveAt == 0)
				fish.leaveAt = _vm->_util->getTimeKey() + 30000 + _vm->_util->getRandom(10000);

			if (_vm->_util->getTimeKey() >= fish.leaveAt)
				fish.evilFish->leave();

		} else {
			// Evil fishes enter the screen in 2s - 10s

			fish.leaveAt = 0;

			if (fish.enterAt == 0)
				fish.enterAt = _vm->_util->getTimeKey() + 2000 + _vm->_util->getRandom(8000);

			if (_vm->_util->getTimeKey() >= fish.enterAt) {
				int fishType = _vm->_util->getRandom(kEvilFishTypeCount);
				fish.evilFish->mutate(kEvilFishTypes[fishType][0], kEvilFishTypes[fishType][1],
				                      kEvilFishTypes[fishType][2], kEvilFishTypes[fishType][3],
				                      kEvilFishTypes[fishType][4]);

				fish.evilFish->enter((EvilFish::Direction)_vm->_util->getRandom(2),
				                     36 + _vm->_util->getRandom(3) * 40);
			}
		}
	}
}

void Diving::updateDecorFish() {
	for (uint i = 0; i < kDecorFishCount; i++) {
		ManagedDecorFish &fish = _decorFish[i];

		if (fish.decorFish->isVisible()) {
			// Move the fish
			int16 x, y;
			fish.decorFish->getPosition(x, y);
			fish.decorFish->setPosition(x + fish.deltaX, y);

			// Check if the fish has left the screen
			int16 width, height;
			fish.decorFish->getFramePosition(x, y);
			fish.decorFish->getFrameSize(width, height);

			if ((x + width) <= 0) {
				fish.decorFish->setVisible(false);
				fish.decorFish->setPause(true);

				fish.enterAt = 0;
			}

		} else {
			// Decor fishes enter the screen every 0s - 10s

			if (fish.enterAt == 0)
				fish.enterAt = _vm->_util->getTimeKey() + _vm->_util->getRandom(10000);

			if (_vm->_util->getTimeKey() >= fish.enterAt) {
				fish.decorFish->rewind();
				fish.decorFish->setPosition(320, 30 + _vm->_util->getRandom(100));
				fish.decorFish->setVisible(true);
				fish.decorFish->setPause(false);
			}
		}
	}
}

void Diving::updatePlants() {
	if (_oko->getState() == Oko::kStateBreathe)
		return;

	for (uint i = 0; i < kPlantCount; i++) {
		ManagedPlant &plant = _plant[i];

		if (plant.plant->isVisible()) {
			// Move the plant
			plant.plant->setPosition(plant.x += plant.deltaX, plant.y);

			// Check if the plant has left the screen
			int16 x, y, width, height;
			plant.plant->getFramePosition(x, y);
			plant.plant->getFrameSize(width, height);

			if ((x + width) <= 0) {
				plant.plant->setVisible(false);
				plant.plant->setPause(true);

				plant.x = 0;
			}

		} else {
			// Find the right-most plant in this level and enter the plant to the right of it

			int16 rightX = 320;
			for (uint j = 0; j < kPlantPerLevelCount; j++)
				rightX = MAX(rightX, _plant[plant.level * kPlantPerLevelCount + j].x);

			enterPlant(plant, rightX);
		}
	}
}

void Diving::updatePearl() {
	if (!_pearl.pearl->isVisible())
		return;

	if (_oko->getState() == Oko::kStateBreathe)
		return;

	// Move the pearl
	int16 x, y, width, height;
	_pearl.pearl->getPosition(x, y);
	_pearl.pearl->setPosition(x - 5, y);

	// Check if the pearl has left the screen
	_pearl.pearl->getFramePosition(x, y);
	_pearl.pearl->getFrameSize(width, height);

	if ((x + width) <= 0) {
		_pearl.pearl->setVisible(false);
		_pearl.pearl->setPause(true);
	}
}

void Diving::getPearl() {
	if (!_pearl.pearl->isVisible())
		return;

	// Make sure the pearl is within Oko's grasp

	int16 x, y, width, height;
	_pearl.pearl->getFramePosition(x, y);
	_pearl.pearl->getFrameSize(width, height);

	if ((x > 175) || ((x + width) < 168))
		return;

	// Remove the pearl
	_pearl.pearl->setVisible(false);
	_pearl.pearl->setPause(true);

	// Add the pearl to our found pearls repository
	if (_pearl.black)
		foundBlackPearl();
	else
		foundWhitePearl();
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

	_vm->_sound->blasterPlay(&_soundBlackPearl, 1, 0);
}

void Diving::foundWhitePearl() {
	_whitePearlCount++;

	int16 x = 54 + (_whitePearlCount - 1) * 8;
	if (_whitePearlCount > 10)
		x += 48;

	_background->drawLayer(*_vm->_draw->_backSurface, 0, 2, x, 177, 0);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, 177, x + 3, 180);

	_vm->_sound->blasterPlay(&_soundWhitePearl, 1, 0);
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

	// Draw the meters
	_airMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

	_healthMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
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
				EvilFish &evilFish = *_evilFish[i].evilFish;

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
