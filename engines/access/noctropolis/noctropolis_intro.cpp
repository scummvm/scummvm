#include "access/access.h"
#include "access/access_noctropolis.h"
#include "access/timer.h"

namespace Access {

void AccessEngine_Noctropolis::runIntro() {

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

	SpriteResource *sprites = new SpriteResource();

	_timer->set(26, 45);
	_timer->set(27, 7);
	_timer->set(28, 240);

	// TODO: Music 98, 1
	_res->load(_scene, "scene01.ap", 0);
	_res->load(sprites, "scene01.ap", 1);
	// TODO: Fade to black and fade to scene palette
	setPalette(_scene->getPalette());
	copySceneBackgroundToWorkScreen();
	copyWorkScreenToVgaScreen();

	_timer->reset(26);
	_timer->reset(27);

	while (1) {

		if (!_timer->isActive(27)) {
			_timer->reset(27);
			if (_cameraX == _scene->getWidth() - _scene->getDisplayWidth() || _leftMouseButton || _rightMouseButton)
				break;

			scrollCameraRight(2);
			
			copySceneBackgroundToWorkScreen();
			
			for (int i = 0; i < lettersMax; i++) {
				_workScreen->drawSprite(sprites, i + 8, lettersX[i], 40);
			}

			if (!_timer->isActive(26)) {
				if (lettersMax <= 10) {
					if (lettersMax == 9)
						_timer->reset(28);
					_timer->reset(26);
					lettersMax++;
				} else if (titlesSpriteIndex < 25) {
					_workScreen->drawSprite(sprites, titlesSpriteIndex + 19, titlesSpriteX[titlesSpriteIndex], titlesSpriteY[titlesSpriteIndex]);
					if (!_timer->isActive(28)) {
						titlesSpriteIndex++;
						_timer->reset(26);
						_timer->reset(28);
					}
				}

			}
			
			if (skylineSpriteOfsX1 > -100) {
				_workScreen->drawSprite(sprites, 0, skylineSpriteOfsX1, 60);
				skylineSpriteOfsX1 -= 5;
			}
			if (skylineSpriteOfsX2 > -100) {
				_workScreen->drawSprite(sprites, 3, skylineSpriteOfsX2, 16);
				skylineSpriteOfsX2 -= 8;
			}
			if (skylineSpriteOfsX4 > -100) {
				_workScreen->drawSprite(sprites, 1, skylineSpriteOfsX4, 145);
				skylineSpriteOfsX4 -= 7;
			}
			if (skylineSpriteOfsX3 > -100) {
				_workScreen->drawSprite(sprites, 4, skylineSpriteOfsX3, 115);
				skylineSpriteOfsX3 -= 5;
			}
			if (skylineSpriteOfsX5 > -100) {
				_workScreen->drawSprite(sprites, 2, skylineSpriteOfsX5, 126);
				skylineSpriteOfsX5 -= 7;
			}

			copyWorkScreenToVgaScreen();

		}

		_timer->update();
		updateEvents();
		_system->updateScreen();
	}

	// TODO: Fade to black

	delete sprites;

}

}
