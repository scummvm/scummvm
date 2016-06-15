#include "eventman.h"
#include "dm.h"
#include "common/system.h"
#include "dungeonman.h"



using namespace DM;


EventManager::EventManager(DMEngine *vm) : _vm(vm) {}


void EventManager::initMouse() {
	_mousePos = Common::Point(0, 0);
	_dummyMapIndex = 0;				
	// TODO: add cursor creatin, set to hidden
}

void showMouse(bool visibility) {
	// TODO: add code
}

void EventManager::setMousePos(Common::Point pos) {
	_vm->_system->warpMouse(pos.x, pos.y);
}


void EventManager::processInput() {
	DungeonMan &dungeonMan = *_vm->_dungeonMan;
	CurrMapData &currMap = dungeonMan._currMap;

	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_KEYDOWN && !event.synthetic) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap.partyDir, 1, 0, currMap.partyPosX, currMap.partyPosY);
				break;
			case Common::KEYCODE_a:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap.partyDir, 0, -1, currMap.partyPosX, currMap.partyPosY);
				break;
			case Common::KEYCODE_s:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap.partyDir, -1, 0, currMap.partyPosX, currMap.partyPosY);
				break;
			case Common::KEYCODE_d:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap.partyDir, 0, 1, currMap.partyPosX, currMap.partyPosY);
				break;
			case Common::KEYCODE_q:
				turnDirLeft(currMap.partyDir);
				break;
			case Common::KEYCODE_e:
				turnDirRight(currMap.partyDir);
				break;
			case Common::KEYCODE_UP:
				if (_dummyMapIndex < 13)
					dungeonMan.setCurrentMapAndPartyMap(++_dummyMapIndex);
				break;
			case Common::KEYCODE_DOWN:
				if (_dummyMapIndex > 0)
					dungeonMan.setCurrentMapAndPartyMap(--_dummyMapIndex);
				break;
			}
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			_mousePos = event.mouse;
		}
	}
}
