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
 * $URL$
 * $Id$
 *
 */

/*
 * GP2X: Common::Event Handling.
 *
 */

#include "backends/platform/gp2x/gp2x-common.h"
#include "backends/platform/gp2x/gp2x-hw.h"
#include "common/util.h"
#include "common/events.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button
#define JOY_DEADZONE 2200

// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1

// GP2X Stick Buttons (Note: The Stick is read as a set of buttons not a HAT type of setup).
#define JOY_BUT_LMOUSE 0x0D
#define JOY_BUT_RMOUSE 0x0E

#define JOY_BUT_RETURN 0x08
#define JOY_BUT_ESCAPE 0x09
#define JOY_BUT_F5 0x0B
#define JOY_BUT_SPACE 0x0F
#define JOY_BUT_TALKUP 0x10
#define JOY_BUT_TALKDN 0x11
#define JOY_BUT_ZERO 0x12

#define JOY_BUT_COMB 0x0A
#define JOY_BUT_EXIT 0x12
#define JOY_BUT_PERIOD 0x0C


//TODO: Quick hack 101 ;-) Clean this up,
#define TRUE 1
#define FALSE 0

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void OSystem_GP2X::fillMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;

	// Adjust for the screen scaling
	if (!_overlayVisible) {
		event.mouse.x /= _scaleFactor;
		event.mouse.y /= _scaleFactor;
		if (_adjustAspectRatio)
			event.mouse.y = aspect2Real(event.mouse.y);
	}
}

void OSystem_GP2X::handleKbdMouse() {
	uint32 curTime = getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
		_km.last_time = curTime;
		if (_km.x_down_count == 1) {
			_km.x_down_time = curTime;
			_km.x_down_count = 2;
		}
		if (_km.y_down_count == 1) {
			_km.y_down_time = curTime;
			_km.y_down_count = 2;
		}

		if (_km.x_vel || _km.y_vel) {
			if (_km.x_down_count) {
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			SDL_WarpMouse(_km.x, _km.y);
		}
	}
}

static byte SDLModToOSystemKeyFlags(SDLMod mod) {
	byte b = 0;
#ifdef LINUPY
	// Yopy has no ALT key, steal the SHIFT key
	// (which isn't used much anyway)
	if (mod & KMOD_SHIFT)
		b |= Common::KBD_ALT;
#else
	if (mod & KMOD_SHIFT)
		b |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		b |= Common::KBD_ALT;
#endif
	if (mod & KMOD_CTRL)
		b |= Common::KBD_CTRL;

	return b;
}

void OSystem_GP2X::moveStick() {
	bool stickBtn[32];

	memcpy(stickBtn, _stickBtn, sizeof(stickBtn));

	if((stickBtn[0])||(stickBtn[2])||(stickBtn[4])||(stickBtn[6]))
		stickBtn[1] = stickBtn[3] = stickBtn[5] = stickBtn[7] = 0;

	if((stickBtn[1])||(stickBtn[2])||(stickBtn[3])){
		if(_km.x_down_count!=2){
			_km.x_vel = -1;
			_km.x_down_count = 1;
		}else
			_km.x_vel = -4;
	} else if((stickBtn[5])||(stickBtn[6])||(stickBtn[7])){
		if(_km.x_down_count!=2){
			_km.x_vel = 1;
			_km.x_down_count = 1;
		}else
			_km.x_vel = 4;
	}
	 else{
		_km.x_vel = 0;
		_km.x_down_count = 0;
	}


	if((stickBtn[0])||(stickBtn[1])||(stickBtn[7])){
		if(_km.y_down_count!=2){
			_km.y_vel = -1;
			_km.y_down_count = 1;
		}else
			_km.y_vel = -4;
	} else if((stickBtn[3])||(stickBtn[4])||(stickBtn[5])){
		if(_km.y_down_count!=2){
			_km.y_vel = 1;
		 	_km.y_down_count = 1;
		}else
			_km.y_vel = 4;
	}
	 else{
		_km.y_vel = 0;
		_km.y_down_count = 0;
	}
}

	//Quick default button states for modifier.

	//int GP2X_BUTTON_STATE_UP              =	FALSE;
	//int GP2X_BUTTON_STATE_DOWN            =	FALSE;
	//int GP2X_BUTTON_STATE_LEFT            =	FALSE;
	//int GP2X_BUTTON_STATE_RIGHT           =	FALSE;
	//int GP2X_BUTTON_STATE_UPLEFT          =	FALSE;
	//int GP2X_BUTTON_STATE_UPRIGHT         =	FALSE;
	//int GP2X_BUTTON_STATE_DOWNLEFT        =	FALSE;
	//int GP2X_BUTTON_STATE_DOWNRIGHT       =	FALSE;
	//int GP2X_BUTTON_STATE_CLICK           =	FALSE;
	//int GP2X_BUTTON_STATE_A               =	FALSE;
	//int GP2X_BUTTON_STATE_B               =	FALSE;
	//int GP2X_BUTTON_STATE_Y               =	FALSE;
	//int GP2X_BUTTON_STATE_X               =	FALSE;
	int GP2X_BUTTON_STATE_L               =	FALSE;
	//int GP2X_BUTTON_STATE_R               =	FALSE;
	//int GP2X_BUTTON_STATE_START           =	FALSE;
	//int GP2X_BUTTON_STATE_SELECT          =	FALSE;
	//int GP2X_BUTTON_STATE_VOLUP           =	FALSE;
	//int GP2X_BUTTON_STATE_VOLDOWN         =	FALSE;

bool OSystem_GP2X::pollEvent(Common::Event &event) {
	SDL_Event ev;
	int axis;
	byte b = 0;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		_screenChangeCount++;
		return true;
	}

	// GP2X Input mappings.

	/*
	Single Button

	Movement:

	GP2X_BUTTON_UP              Cursor Up
	GP2X_BUTTON_DOWN            Cursor Down
	GP2X_BUTTON_LEFT            Cursor Left
	GP2X_BUTTON_RIGHT           Cursor Right

	TODO: Add extra directions to cursor mappings.

	GP2X_BUTTON_UPLEFT          Cursor Up Left
	GP2X_BUTTON_UPRIGHT         Cursor Up Right
	GP2X_BUTTON_DOWNLEFT        Cursor Down Left
	GP2X_BUTTON_DOWNRIGHT       Cursor Down Right

	Button Emulation:

	GP2X_BUTTON_CLICK           Left Mouse Click
	GP2X_BUTTON_A				. (Period)
	GP2X_BUTTON_B               Left Mouse Click
	GP2X_BUTTON_Y               Space Bar
	GP2X_BUTTON_X               Right Mouse Click
	GP2X_BUTTON_L				Combo Modifier (Left Trigger)
	GP2X_BUTTON_R               F5 (Right Trigger)
	GP2X_BUTTON_START           Return
	GP2X_BUTTON_SELECT          Escape
	GP2X_BUTTON_VOLUP           /dev/mixer Global Volume Up
	GP2X_BUTTON_VOLDOWN         /dev/mixer Global Volume Down

	Combos:

	GP2X_BUTTON_VOLUP &	GP2X_BUTTON_VOLDOWN		0 (For Monkey 2 CP)
	GP2X_BUTTON_L &	GP2X_BUTTON_SELECT 			Common::EVENT_QUIT (Calls Sync() to make sure SD is flushed)
	GP2X_BUTTON_L &	GP2X_BUTTON_Y				Toggles setZoomOnMouse() for larger then 320*240 games to scale to the point + raduis.
	*/

	while(SDL_PollEvent(&ev)) {

		switch(ev.type) {
		case SDL_KEYDOWN:{
			b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

			// Alt-Return and Alt-Enter toggle full screen mode
			if (b == Common::KBD_ALT && (ev.key.keysym.sym == SDLK_RETURN
			                  || ev.key.keysym.sym == SDLK_KP_ENTER)) {
				setFullscreenMode(!_fullscreen);
				break;
			}

			// Alt-S: Create a screenshot
			if (b == Common::KBD_ALT && ev.key.keysym.sym == 's') {
				char filename[20];

				for (int n = 0;; n++) {
					SDL_RWops *file;

					sprintf(filename, "scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename, "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (saveScreenshot(filename))
					printf("Saved '%s'\n", filename);
				else
					printf("Could not save screenshot!\n");
				break;
			}

			// Ctrl-m toggles mouse capture
			//if (b == Common::KBD_CTRL && ev.key.keysym.sym == 'm') {
			//	toggleMouseGrab();
			//	break;
			//}

//#ifdef MACOSX
//			// On Macintosh', Cmd-Q quits
//			if ((ev.key.keysym.mod & KMOD_META) && ev.key.keysym.sym == 'q') {
//				event.type = Common::EVENT_QUIT;
//				return true;
//			}
//#elif defined(UNIX)
//			// On other unices, Control-Q quits
//			if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'q') {
//				event.type = Common::EVENT_QUIT;
//				return true;
//			}
//#else
//			// Ctrl-z and Alt-X quit
//			if ((b == Common::KBD_CTRL && ev.key.keysym.sym == 'z') || (b == Common::KBD_ALT && ev.key.keysym.sym == 'x')) {
//				event.type = Common::EVENT_QUIT;
//				return true;
//			}
//#endif
//
//			// Ctrl-Alt-<key> will change the GFX mode
//			if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
//
//				handleScalerHotkeys(ev.key);
//				break;
//			}
			const bool event_complete = remapKey(ev,event);

			if (event_complete)
				return true;

			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			return true;
			}
		case SDL_KEYUP:
			{
			const bool event_complete = remapKey(ev,event);

			if (event_complete)
				return true;

			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

			// Ctrl-Alt-<key> will change the GFX mode
			if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
				// Swallow these key up events
				break;
			}

			return true;
			}
		case SDL_MOUSEMOTION:
			event.type = Common::EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);

			setMousePos(event.mouse.x, event.mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
			else if (ev.button.button == SDL_BUTTON_WHEELUP)
				event.type = Common::EVENT_WHEELUP;
			else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
				event.type = Common::EVENT_WHEELDOWN;
#endif
			else
				break;

			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONUP;
			else
				break;
			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		// GP2X Button mapings. Main code

		case SDL_JOYBUTTONDOWN:
			_stickBtn[ev.jbutton.button] = 1;
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == GP2X_BUTTON_CLICK) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event.type = Common::EVENT_RBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (_stickBtn[JOY_BUT_COMB] && (ev.jbutton.button == JOY_BUT_EXIT)) {
				event.type = Common::EVENT_QUIT;
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
			} else {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case GP2X_BUTTON_L:
						GP2X_BUTTON_STATE_L = TRUE;
						break;
					case GP2X_BUTTON_R:
						if (GP2X_BUTTON_STATE_L == TRUE) {
							event.kbd.keycode = SDLK_0;
							event.kbd.ascii = mapKey(SDLK_0, ev.key.keysym.mod, 0);
						} else {
							event.kbd.keycode = SDLK_F5;
							event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_SELECT:
						if (GP2X_BUTTON_STATE_L == TRUE) {
							event.type = Common::EVENT_QUIT;
						} else {
							event.kbd.keycode = SDLK_ESCAPE;
							event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_A:
						event.kbd.keycode = SDLK_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_Y:
						if (GP2X_BUTTON_STATE_L == TRUE) {
							setZoomOnMouse();
						} else {
							event.kbd.keycode = SDLK_SPACE;
							event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						}
						break;
					case JOY_BUT_RETURN:
						event.kbd.keycode = SDLK_RETURN;
						event.kbd.ascii = mapKey(SDLK_RETURN, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_ZERO:
						event.kbd.keycode = SDLK_0;
						event.kbd.ascii = mapKey(SDLK_0, ev.key.keysym.mod, 0);
						break;

					//case GP2X_BUTTON_R:
					//	if ((ev.jbutton.button == GP2X_BUTTON_L) && (ev.jbutton.button == GP2X_BUTTON_R)) {
					//		displayMessageOnOSD("Exiting ScummVM");
					//		//Sync();
					//		event.type = Common::EVENT_QUIT;
					//		break;
					//	} else if ((ev.jbutton.button == GP2X_BUTTON_L) && (ev.jbutton.button != GP2X_BUTTON_R)) {
					//		displayMessageOnOSD("Left Trigger Pressed");
					//		break;
					//	} else if ((ev.jbutton.button == GP2X_BUTTON_R) && (ev.jbutton.button != GP2X_BUTTON_L)) {
					//		event.kbd.keycode = SDLK_F5;
					//		event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
					//		break;
					//	} else {
					//		break;
					//	}
					//	break;
					case GP2X_BUTTON_VOLUP:
						if (GP2X_BUTTON_STATE_L == TRUE) {
							displayMessageOnOSD("Setting CPU Speed at 230MHz");
							GP2X_setCpuspeed(200);
							//event.kbd.keycode = SDLK_PLUS;
							//event.kbd.ascii = mapKey(SDLK_PLUS, ev.key.keysym.mod, 0);
						} else {
							GP2X_mixer_move_volume(1);
							displayMessageOnOSD("Increasing Volume");
						}
						break;
					case GP2X_BUTTON_VOLDOWN:
						if (GP2X_BUTTON_STATE_L == TRUE) {
							displayMessageOnOSD("Setting CPU Speed at 60MHz");
							GP2X_setCpuspeed(60);
							//event.kbd.keycode = SDLK_MINUS;
							//event.kbd.ascii = mapKey(SDLK_MINUS, ev.key.keysym.mod, 0);
						} else {
							GP2X_mixer_move_volume(0);
							displayMessageOnOSD("Decreasing Volume");
						}
						break;
				}
			}
			return true;

		case SDL_JOYBUTTONUP:
			_stickBtn[ev.jbutton.button] = 0;
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
			} else {
				event.type = Common::EVENT_KEYUP;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case GP2X_BUTTON_SELECT:
						event.kbd.keycode = SDLK_ESCAPE;
						event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_A:
						event.kbd.keycode = SDLK_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_Y:
//						event.kbd.keycode = SDLK_SPACE;
//						event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_START:
						event.kbd.keycode = SDLK_RETURN;
						event.kbd.ascii = mapKey(SDLK_RETURN, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_L:
						GP2X_BUTTON_STATE_L = FALSE;
						break;
					case GP2X_BUTTON_R:
						event.kbd.keycode = SDLK_F5;
						event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_VOLUP:
						break;
					case GP2X_BUTTON_VOLDOWN:
						break;
				}
			}
			return true;

		case SDL_JOYAXISMOTION:
			axis = ev.jaxis.value;
			if ( axis > JOY_DEADZONE) {
				axis -= JOY_DEADZONE;
				event.type = Common::EVENT_MOUSEMOVE;
			} else if ( axis < -JOY_DEADZONE ) {
				axis += JOY_DEADZONE;
				event.type = Common::EVENT_MOUSEMOVE;
			} else
				axis = 0;

			if ( ev.jaxis.axis == JOY_XAXIS) {
#ifdef JOY_ANALOG
				_km.x_vel = axis/2000;
				_km.x_down_count = 0;
#else
				if (axis != 0) {
					_km.x_vel = (axis > 0) ? 1:-1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
#endif

			} else if (ev.jaxis.axis == JOY_YAXIS) {
#ifndef JOY_INVERT_Y
				axis = -axis;
#endif
#ifdef JOY_ANALOG
				_km.y_vel = -axis / 2000;
				_km.y_down_count = 0;
#else
				if (axis != 0) {
					_km.y_vel = (-axis > 0) ? 1: -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
#endif
			}

			fillMouseEvent(event, _km.x, _km.y);

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event.type = Common::EVENT_QUIT;
			return true;
		}
	}
	return false;
}

bool OSystem_GP2X::remapKey(SDL_Event &ev,Common::Event &event) {
	return false;
}

