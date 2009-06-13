/*
 * State.h
 *
 *  Created on: 13-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_STATE_H_
#define ASYLUM_STATE_H_

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/resman.h"

namespace Asylum {

class AsylumEngine;
class ResourceManager;

class MenuState;

class State {
public:
	State(AsylumEngine *vm);
	virtual ~State() {}

	void handleEvent(Common::Event *event);

protected:
	Common::Event *_ev;
	int _mouseX;
	int _mouseY;
	AsylumEngine    *_vm;
	ResourceManager *_resMgr;

private:
	virtual void init()   = 0;
	virtual void update() = 0;


}; // end of class State


class MenuState: public State {
public:
	MenuState(AsylumEngine *vm);
	~MenuState() {}

private:

	enum EyesAnimation {
		kEyesFront       = 0,
		kEyesLeft        = 1,
		kEyesRight       = 2,
		kEyesTop         = 3,
		kEyesBottom      = 4,
		kEyesTopLeft     = 5,
		kEyesTopRight    = 6,
		kEyesBottomLeft  = 7,
		kEyesBottomRight = 8,
		kEyesCrossed     = 9
	};

	Audio::SoundHandle _sfxHandle;
	int _activeIcon;
	int _previousActiveIcon;
	int _curIconFrame;
	int _curMouseCursor;
	int _cursorStep;

	void updateCursor();

	void init();
	void update();
}; // end of class MenuState

} // end of namespace Asylum

#endif
