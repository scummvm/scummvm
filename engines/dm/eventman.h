

#ifndef DM_EVENTMAN_H
#define DM_EVENTMAN_H

#include "common/events.h"

namespace DM {

class DMEngine;

class EventManager {
	DMEngine *_vm;

	Common::Point _mousePos;
	uint16 _dummyMapIndex;
public:
	EventManager(DMEngine *vm);
	void initMouse();
	void showMouse(bool visibility);

	void setMousePos(Common::Point pos);
	void processInput();
};

}


#endif DM_EVENTMAN_H
