#include "common/array.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/func.h"
#include "common/util.h"

#include "access/graphics.h"
#include "access/script_noctropolis.h"
#include "access/timer.h"

namespace Access {

void AccessEngine_Noctropolis::runMap() {

	const int kMapLocationCount = 14;

	static const struct { int x, y; } kPlayerMapEntryPositions[kMapLocationCount] = {
		{280, 390},
		{372, 338},
		{414, 201},
		{253, 387},
		{72, 371},
		{471, 340},
		{70, 380},
		{0, 0},
		{85, 378},
		{194, 347},
		{73, 370},
		{323, 386},
		{43, 362},
		{0, 0}
	};

	static const int kMapLocationRoomNumbers[kMapLocationCount] = {
		12, 28, 11, 41, 17, 51, 43, 15, 27, 85, 32, 37, 26, 33
	};

	static const struct { int x, y, spriteIndex; } kDrawMapLocationsInfos[kMapLocationCount] = {
		{259, 98, 0},
		{487, 297, 1},
		{272, 491, 2},
		{323, 658, 3},
		{466, 115, 4},
		{333, 150, 5},
		{42, 106, 6},
		{28, 449, 7},
		{49, 727, 8},
		{390, 380, 9},
		{152, 282, 10},
		{179, 25, 11},
		{114, 193, 12},
		{201, 242, 13}
	};

	SpriteResource *locationIcons;
	StringResource *locationNames;

	int selectedLocationNum = -1, locationNum = -1;
	bool needRedraw = true, needFadeIn = true;

	setMouseCursor(0);
	_player.disabled = true;
	_stiletto.disabled = true;

	_res->load(_scene, "map.ap", 0);
	
	_sceneCenterX = 0;
	_sceneCenterY = 0;
	// TODO: Restore current map position
	_cameraX = 0;
	_cameraY = 0;

	locationIcons = new SpriteResource();
	_res->load(locationIcons, "map.ap", 1);

	locationNames = new StringResource();
	_res->load(locationNames, GID_NOCTROPOLIS, kResStringTable, 2);

	fadeToBlack();
	clearVgaScreen();

	startPaletteCycle(181, 190, 5, 6);

	while (selectedLocationNum == -1) {
	
		if (needRedraw) {
			copySceneBackgroundToWorkScreen();
			for (int i = 0; i < kMapLocationCount; i++) {
				_workScreen->drawSprite(locationIcons, kDrawMapLocationsInfos[i].spriteIndex,
					kDrawMapLocationsInfos[i].x - _cameraX, kDrawMapLocationsInfos[i].y - _cameraY);
			}
			copyWorkScreenToVgaScreen();
			if (needFadeIn) {
				memcpy(_mainPalette, _scene->getPalette(), 768);
				copySystemPalette();
				fadeToPalette();
				needFadeIn = false;
			}
		}

		updatePaletteCycle();

		int hoveredLocationNum = _scene->findHotspotAt(_cameraX + _mouseX, _cameraY + _mouseY);
		//debug("hoveredLocationNum = %d", hoveredLocationNum);

		// TODO: location available?
		
		if (locationNum != hoveredLocationNum) {
			locationNum = hoveredLocationNum;
			_vgaScreen->lock();
			_vgaScreen->fillRect(220, 380, 220 + 300, 380 + 16, 246);
			if (locationNum != -1)
				_vgaScreen->drawText(_fonts[3], locationNames->getString(locationNum), 220, 380, 181, 0, kFontBackground);
			_vgaScreen->unlock();
		}

		if (leftMouseButton() && locationNum != -1) {
			selectedLocationNum = locationNum;
		}
		
		if ((_mouseWheelDelta < 0 || _mouseY < 32) && _cameraY > 0) {
			scrollCameraUp(8);
			needRedraw = true;
		} else if ((_mouseWheelDelta > 0 || _mouseY > 368) && _cameraY < _scene->getHeight() - _scene->getDisplayHeight()) {
			scrollCameraDown(8);
			needRedraw = true;
		}

		_timer->update();	
		updateEvents();
		updateScreen();

		// TODO: Change to getMillis scrolling time delta
		_system->delayMillis(50);

	}

	delete locationIcons;
	delete locationNames;

	if (selectedLocationNum >= 0) {
		waitUntilLeftButtonIsReleased();
		_currSceneNum = kMapLocationRoomNumbers[selectedLocationNum];
		debug("_currSceneNum = %d", _currSceneNum);
		// TODO: Save current map position
		// TODO: Restore timer 18
		_player.x1 = _player.x2 = kPlayerMapEntryPositions[selectedLocationNum].x;
		_player.y1 = _player.y2 = kPlayerMapEntryPositions[selectedLocationNum].y;
		// TODO: initStilettoPosition();
		_gameState = AccessEngine_Noctropolis::GS_CHANGE_SCENE;
	}

}

}
