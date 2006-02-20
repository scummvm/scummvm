// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "debug.h"
#include "driver_ronin.h"

#include <ronin/ronin.h>
#include <ronin/report.h>

// Control allocation 

/* Controller: 16 buttons, 6 axises */
/* Mouse: 8 buttons, 8 axises */

/* 0x02 - 0x9f  Keyboard     */
/* 0xa0 - 0xb7  Controller 1 */
/* 0xb8 - 0xcf  Controller 2 */
/* 0xd0 - 0xdf  Mouse        */
/* 0xe0 - 0xe7  Shift keys   */


#define JOY1  0xa0
#define JOY2  0xb8
#define MOUSE 0xd0


static const Driver::ControlDescriptor controls[] = {
	{ "KEY_ESCAPE", 0x29 },
	{ "KEY_1", 0x1e },
	{ "KEY_2", 0x1f },
	{ "KEY_3", 0x20 },
	{ "KEY_4", 0x21 },
	{ "KEY_5", 0x22 },
	{ "KEY_6", 0x23 },
	{ "KEY_7", 0x24 },
	{ "KEY_8", 0x25 },
	{ "KEY_9", 0x26 },
	{ "KEY_0", 0x27 },
	{ "KEY_MINUS", 0x2d },
	{ "KEY_EQUALS", 0x2e },
	{ "KEY_BACK", 0x2a },
	{ "KEY_TAB", 0x2b },
	{ "KEY_Q", 0x14 },
	{ "KEY_W", 0x1a },
	{ "KEY_E", 0x08 },
	{ "KEY_R", 0x15 },
	{ "KEY_T", 0x17 },
	{ "KEY_Y", 0x1c },
	{ "KEY_U", 0x18 },
	{ "KEY_I", 0x0c },
	{ "KEY_O", 0x12 },
	{ "KEY_P", 0x13 },
	{ "KEY_LBRACKET", 0x2f },
	{ "KEY_RBRACKET", 0x30 },
	{ "KEY_RETURN", 0x28 },
	{ "KEY_LCONTROL", 0xe0 },
	{ "KEY_A", 0x04 },
	{ "KEY_S", 0x16 },
	{ "KEY_D", 0x07 },
	{ "KEY_F", 0x09 },
	{ "KEY_G", 0x0a },
	{ "KEY_H", 0x0b },
	{ "KEY_J", 0x0d },
	{ "KEY_K", 0x0e },
	{ "KEY_L", 0x0f },
	{ "KEY_SEMICOLON", 0x33 },
	{ "KEY_APOSTROPHE", 0x34 },
	{ "KEY_GRAVE", 0x32 },
	{ "KEY_LSHIFT", 0xe1 },
	{ "KEY_BACKSLASH", 0x87 },
	{ "KEY_Z", 0x1d },
	{ "KEY_X", 0x1b },
	{ "KEY_C", 0x06 },
	{ "KEY_V", 0x19 },
	{ "KEY_B", 0x05 },
	{ "KEY_N", 0x11 },
	{ "KEY_M", 0x10 },
	{ "KEY_COMMA", 0x36 },
	{ "KEY_PERIOD", 0x37 },
	{ "KEY_SLASH", 0x38 },
	{ "KEY_RSHIFT", 0xe5 },
	{ "KEY_MULTIPLY", 0x55 },
	{ "KEY_LMENU", 0xe2 },
	{ "KEY_SPACE", 0x2c },
	{ "KEY_CAPITAL", 0x39 },
	{ "KEY_F1", 0x3a },
	{ "KEY_F2", 0x3b },
	{ "KEY_F3", 0x3c },
	{ "KEY_F4", 0x3d },
	{ "KEY_F5", 0x3e },
	{ "KEY_F6", 0x3f },
	{ "KEY_F7", 0x40 },
	{ "KEY_F8", 0x41 },
	{ "KEY_F9", 0x42 },
	{ "KEY_F10", 0x43 },
	{ "KEY_NUMLOCK", 0x53 },
	{ "KEY_SCROLL", 0x47 },
	{ "KEY_NUMPAD7", 0x5f },
	{ "KEY_NUMPAD8", 0x60 },
	{ "KEY_NUMPAD9", 0x61 },
	{ "KEY_SUBTRACT", 0x56 },
	{ "KEY_NUMPAD4", 0x5c },
	{ "KEY_NUMPAD5", 0x5d },
	{ "KEY_NUMPAD6", 0x5e },
	{ "KEY_ADD", 0x57 },
	{ "KEY_NUMPAD1", 0x59 },
	{ "KEY_NUMPAD2", 0x5a },
	{ "KEY_NUMPAD3", 0x5b },
	{ "KEY_NUMPAD0", 0x62 },
	{ "KEY_DECIMAL", 0x63 },
	{ "KEY_F11", 0x44 },
	{ "KEY_F12", 0x45 },
	{ "KEY_STOP", 0x48 },
	{ "KEY_NUMPADENTER", 0x58 },
	{ "KEY_RCONTROL", 0xe4 },
	{ "KEY_DIVIDE", 0x54 },
	{ "KEY_SYSRQ", 0x46 },
	{ "KEY_RMENU", 0xe6 },
	{ "KEY_HOME", 0x4a },
	{ "KEY_UP", 0x52 },
	{ "KEY_PRIOR", 0x4b },
	{ "KEY_LEFT", 0x50 },
	{ "KEY_RIGHT", 0x4f },
	{ "KEY_END", 0x4d },
	{ "KEY_DOWN", 0x51 },
	{ "KEY_NEXT", 0x4e },
	{ "KEY_INSERT", 0x49 },
	{ "KEY_DELETE", 0x4c },
	{ "KEY_LWIN", 0xe3 },
	{ "KEY_RWIN", 0xe7 },
	{ "KEY_APPS", 0x65 },
	{ "KEY_JOY1_B1", JOY1+0x02 },
	{ "KEY_JOY1_B2", JOY1+0x01 },
	{ "KEY_JOY1_B3", JOY1+0x00 },
	{ "KEY_JOY1_B4", JOY1+0x0b },
	{ "KEY_JOY1_B5", JOY1+0x0a },
	{ "KEY_JOY1_B6", JOY1+0x09 },
	{ "KEY_JOY1_B7", JOY1+0x08 },
	{ "KEY_JOY1_B8", JOY1+0x03 },
	{ "KEY_JOY1_B9", JOY1+0x0e },
	{ "KEY_JOY1_B10", JOY1+0x0f },
	{ "KEY_JOY1_HLEFT", JOY1+0x06 },
	{ "KEY_JOY1_HUP", JOY1+0x04 },
	{ "KEY_JOY1_HRIGHT", JOY1+0x07 },
	{ "KEY_JOY1_HDOWN", JOY1+0x05 },
	{ "KEY_JOY2_B1", JOY2+0x02 },
	{ "KEY_JOY2_B2", JOY2+0x01 },
	{ "KEY_JOY2_B3", JOY2+0x00 },
	{ "KEY_JOY2_B4", JOY2+0x0b },
	{ "KEY_JOY2_B5", JOY2+0x0a },
	{ "KEY_JOY2_B6", JOY2+0x09 },
	{ "KEY_JOY2_B7", JOY2+0x08 },
	{ "KEY_JOY2_B8", JOY2+0x03 },
	{ "KEY_JOY2_B9", JOY2+0x0e },
	{ "KEY_JOY2_B10", JOY2+0x0f },
	{ "KEY_JOY2_HLEFT", JOY2+0x06 },
	{ "KEY_JOY2_HUP", JOY2+0x04 },
	{ "KEY_JOY2_HRIGHT", JOY2+0x07 },
	{ "KEY_JOY2_HDOWN", JOY2+0x05 },
	{ "KEY_MOUSE_B1", MOUSE+0x02 },
	{ "KEY_MOUSE_B2", MOUSE+0x01 },
	{ "KEY_MOUSE_B3", MOUSE+0x00 },
	{ "KEY_MOUSE_B4", MOUSE+0x03 },
	{ "AXIS_JOY1_X", JOY1+0x12 },
	{ "AXIS_JOY1_Y", JOY1+0x13 },
	{ "AXIS_JOY1_Z", JOY1+0x10 },
	{ "AXIS_JOY1_R", JOY1+0x11 },
	{ "AXIS_JOY1_U", JOY1+0x14 },
	{ "AXIS_JOY1_V", JOY1+0x15 },
	{ "AXIS_JOY2_X", JOY2+0x12 },
	{ "AXIS_JOY2_Y", JOY2+0x13 },
	{ "AXIS_JOY2_Z", JOY2+0x10 },
	{ "AXIS_JOY2_R", JOY2+0x11 },
	{ "AXIS_JOY2_U", JOY2+0x14 },
	{ "AXIS_JOY2_V", JOY2+0x15 },
	{ "AXIS_MOUSE_X", MOUSE+0x08 },
	{ "AXIS_MOUSE_Y", MOUSE+0x09 },
	{ "AXIS_MOUSE_Z", MOUSE+0x0a },
	{ NULL, 0 }
};

const Driver::ControlDescriptor *DriverRonin::listControls()
{
	return controls;
}

int DriverRonin::getNumControls()
{
	return 0xe8;
}

bool DriverRonin::controlIsAxis(int num)
{
	return	(num >= JOY1+0x10 && num <= JOY1+0x15) ||
		(num >= JOY2+0x10 && num <= JOY2+0x15) ||
		(num >= MOUSE+0x08 && num <= MOUSE+0x0f);
}

float DriverRonin::getControlAxis(int num)
{
	JoyState *state;
	if(num >= MOUSE+0x08 && num <= MOUSE+0x0f) {
		short *axis = &_mouse_state.axis1 + (num - MOUSE+0x08);
		return *axis * 1.0/0x200;
	} else if(num >= JOY1+0x10 && num <= JOY1+0x15) {
		num -= JOY1+0x10;
		state = &_joy1_state;
	} else if(num >= JOY2+0x10 && num <= JOY2+0x15) {
		num -= JOY2+0x10;
		state = &_joy2_state;
	} else
		return 0;

	unsigned char *axis = &state->rtrigger + num;
	if(num < 2)
		// Triggers
		return *axis * 1.0/256;
	else
		// Joystick X/Y
		return *axis * 1.0/128 - 1.0;
}

bool DriverRonin::getControlState(int num)
{
	if(num >= 2 && num <= 0x9f) {
		for(int i=0; i<6; i++)
			if(_kbd_state.key[i] == num)
				return true;
	} else if(num >= 0xe0 && num <= 0xe7) {
		return (_kbd_state.shift & (1 << (num-0xe0))) != 0;
	} else if(num >= JOY1 && num <= JOY1+0x0f) {
		return (_joy1_state.buttons & (1 << (num-JOY1))) != 0;
	} else if(num >= JOY2 && num <= JOY2+0x0f) {
		return (_joy2_state.buttons & (1 << (num-JOY2))) != 0;
	} else if(num >= MOUSE && num <= MOUSE+0x07) {
		return (_mouse_state.buttons & (1 << (num-MOUSE))) != 0;
	}
	return false;
}

uint16 DriverRonin::makeAscii(int keycode, int shift_state)
{
	if(keycode >= 0x04 && keycode <= 0x1d) {
		uint16 ascii = keycode + ('a' - 0x04);
		if(shift_state & KBD_SHIFT)
			ascii -= ('a' - 'A');
		if(shift_state & KBD_CTRL)
			ascii &= 0x1f;
		return ascii;
	} else if(keycode >= 0x1e && keycode <= 0x26)
		return keycode + ((shift_state & KBD_SHIFT)?
				  ('!'-0x1e) : ('1'-0x1e));
	else if(keycode >= 0x59 && keycode <= 0x61)
		return keycode + ('1'-0x59);
	else if(keycode >= 0x2d && keycode <= 0x38 && keycode != 0x31)
		return ((shift_state & KBD_SHIFT)?
			"=¯`{ }+*½<>?" :
			"-^@[ ];:§,./")[keycode - 0x2d];
	else if(keycode >= 0x54 && keycode <= 0x57)
		return "/*-+"[keycode-0x54];
	else switch(keycode) {
	case 0x27:
		return ((shift_state & KBD_SHIFT)? '~' : '0');
	case 0x62:
		return '0';
	case 0x28: case 0x58:
		return 13;
	case 0x29:
		return 27;
	case 0x2a:
		return 8;
	case 0x2b:
		return 9;
	case 0x2c:
		return ' ';
	case 0x4c:
		return 127;
	case 0x63:
		return '.';
	case 0x64: case 0x87:
		return ((shift_state & KBD_SHIFT)? '_' : '\\');
	case 0x89:
		return ((shift_state & KBD_SHIFT)? '|' : '¥'); break;
	}

	return 0;
}

bool DriverRonin::checkJoystick(struct mapledev *pad, JoyState &state, int base, Event &event)
{
	if(!state.present) {
		state.present = true;
		state.buttons = 0;
	}

	state.rtrigger = pad->cond.controller.rtrigger;
	state.ltrigger = pad->cond.controller.ltrigger;
	state.joyx = pad->cond.controller.joyx;
	state.joyy = pad->cond.controller.joyy;
	state.joyx2 = pad->cond.controller.joyx2;
	state.joyy2 = pad->cond.controller.joyy2;

	unsigned short buttons = ~pad->cond.controller.buttons;
	if(buttons != state.buttons) {
		int up = state.buttons & ~buttons;
		int down = buttons & ~state.buttons;
		for(int i=0; i<8; i++)
			if(up & (i<<i)) {
				event.type = EVENT_KEYUP;
				event.kbd.num = base + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _kbd_state.flags;
				state.buttons &= ~(1<<i);
				return true;
			} else if(down & (i<<8)) {
				event.type = EVENT_KEYDOWN;
				event.kbd.num = base + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _kbd_state.flags;
				state.buttons |= (1<<i);
				return true;
			}
		state.buttons = buttons;
	}
	return false;
}

bool DriverRonin::checkMouse(struct mapledev *pad, Event &event)
{
	if(!_mouse_state.present) {
		_mouse_state.present = true;
		_mouse_state.buttons = 0;
	}

	_mouse_state.axis1 = pad->cond.mouse.axis1;
	_mouse_state.axis2 = pad->cond.mouse.axis2;
	_mouse_state.axis3 = pad->cond.mouse.axis3;
	_mouse_state.axis4 = pad->cond.mouse.axis4;
	_mouse_state.axis5 = pad->cond.mouse.axis5;
	_mouse_state.axis6 = pad->cond.mouse.axis6;
	_mouse_state.axis7 = pad->cond.mouse.axis7;
	_mouse_state.axis8 = pad->cond.mouse.axis8;

	unsigned char buttons = (~pad->cond.mouse.buttons) & 0xff;
	if(buttons != _mouse_state.buttons) {
		int up = _mouse_state.buttons & ~buttons;
		int down = buttons & ~_mouse_state.buttons;
		for(int i=0; i<8; i++)
			if(up & (i<<i)) {
				event.type = EVENT_KEYUP;
				event.kbd.num = MOUSE + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _kbd_state.flags;
				_mouse_state.buttons &= ~(1<<i);
				return true;
			} else if(down & (i<<8)) {
				event.type = EVENT_KEYDOWN;
				event.kbd.num = MOUSE + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _kbd_state.flags;
				_mouse_state.buttons |= (1<<i);
				return true;
			}
		_mouse_state.buttons = buttons;
	}
	return false;
}

bool DriverRonin::checkKeyboard(struct mapledev *pad, Event &event)
{
	int _flags = 0;

	if(pad->cond.kbd.shift & 0x11)
		_flags |= KBD_CTRL;
	if(pad->cond.kbd.shift & 0x22)
		_flags |= KBD_SHIFT;
	if(pad->cond.kbd.shift & 0x44)
		_flags |= KBD_ALT;

	if(!_kbd_state.present) {
		_kbd_state.present = true;
		_kbd_state.shift = 0;
		memset(_kbd_state.key, 0, sizeof(_kbd_state.key));
	}

	_kbd_state.flags = _flags;

	if(pad->cond.kbd.shift != _kbd_state.shift) {
		int up = _kbd_state.shift & ~pad->cond.kbd.shift;
		int down = pad->cond.kbd.shift & ~_kbd_state.shift;
		for(int i=0; i<8; i++)
			if(up & (i<<i)) {
				event.type = EVENT_KEYUP;
				event.kbd.num = 0xe0 + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _flags;
				_kbd_state.shift &= ~(1<<i);
				return true;
			} else if(down & (i<<8)) {
				event.type = EVENT_KEYDOWN;
				event.kbd.num = 0xe0 + i;
				event.kbd.ascii = 0;
				event.kbd.flags = _flags;
				_kbd_state.shift |= (1<<i);
				return true;
			}
		_kbd_state.shift = pad->cond.kbd.shift;
	}
	
	if(!memcmp(pad->cond.kbd.key, _kbd_state.key, sizeof(_kbd_state.key)))
		return false;

	for(int i=5; i>=0; --i) {
		unsigned char old_key = _kbd_state.key[i];
		if(old_key >= 2 && old_key <= 0x9f) {
			int found = false;
			for(int j=0; j<6; j++)
				if(pad->cond.kbd.key[j] == old_key) {
					found = true;
					break;
				}
			if(!found) {
				event.type = EVENT_KEYUP;
				event.kbd.num = old_key;
				event.kbd.ascii = makeAscii(old_key, _flags);
				event.kbd.flags = _flags;
#ifdef KBD_DEBUG
				printf("KEYUP %02x state=[", old_key);
				for(int z=0; z<6; z++)
				  printf((z == i?"<%02x> ":"%02x "), _kbd_state.key[z]);
				printf("] cond=[");
				for(int z=0; z<6; z++)
				  printf("%02x ", pad->cond.kbd.key[z]);
				printf("]\n");
#endif
				_kbd_state.key[i] = 0;
				return true;
			}
		}
	}
	for(int i=0; i<6; i++) {
		unsigned char new_key = pad->cond.kbd.key[i];
		if(new_key >= 2 && new_key <= 0x9f) {
			int found = false;
			int free_pos = -1;
			for(int j=0; j<6; j++)
				if(_kbd_state.key[j] == new_key) {
					found = true;
					break;
				} else if(free_pos < 0 &&
					  (_kbd_state.key[j] < 2 ||
					   _kbd_state.key[j] > 0xa0))
					free_pos = j;
			if(!found && free_pos >= 0) {
				event.type = EVENT_KEYDOWN;
				event.kbd.num = new_key;
				event.kbd.ascii = makeAscii(new_key, _flags);
				event.kbd.flags = _flags;
#ifdef KBD_DEBUG
				printf("KEYDOWN %02x state=[", new_key);
				for(int z=0; z<6; z++)
				  printf((z == free_pos?"{%02x} ":"%02x "), _kbd_state.key[z]);
				printf("] cond=[");
				for(int z=0; z<6; z++)
				  printf((z==i? "<%02x> ":"%02x "), pad->cond.kbd.key[z]);
				printf("]\n");
#endif
				_kbd_state.key[free_pos] = new_key;
				return true;
			}
		}
	}
	
	memcpy(_kbd_state.key, pad->cond.kbd.key, sizeof(_kbd_state.key));
	return false;
}

bool DriverRonin::checkInput(struct mapledev *pad, Event &event)
{
	int joyCnt=0, mouseCnt=0, kbdCnt=0;
	for(int i=0; i<4; i++, pad++)
		if((pad->func & MAPLE_FUNC_CONTROLLER) &&
		   joyCnt < 2) {
			if(checkJoystick(pad, (joyCnt? _joy2_state
					       : _joy1_state),
					 (joyCnt? JOY2 : JOY1), event))
				return true;
			joyCnt++;
		} else if((pad->func & MAPLE_FUNC_MOUSE) &&
			  mouseCnt < 1) {
			if(checkMouse(pad, event))
				return true;
			mouseCnt++;
		} else if((pad->func & MAPLE_FUNC_KEYBOARD) &&
			  kbdCnt < 1) {
			if(checkKeyboard(pad, event))
				return true;
			kbdCnt++;
		}
	return false;
}

bool DriverRonin::pollEvent(Event &event)
{
	unsigned int t = Timer();
	
	if(_timer_active && ((int)(t-_timer_next_expiry))>=0) {
		_timer_duration = _timer_callback(_timer_duration);
		_timer_next_expiry = t+USEC_TO_TIMER(1000*_timer_duration);
	}
	
	if(((int)(t-_devpoll))<0)
		return false;
	
	_devpoll += USEC_TO_TIMER(17000);
	if(((int)(t-_devpoll))>=0)
		_devpoll = t + USEC_TO_TIMER(17000);
	
	getMillis();
	int mask = getimask();
	setimask(15);
	checkSound();
	bool r = checkInput(locked_get_pads(), event);
	setimask(mask);
	return r;
}

uint32 DriverRonin::getMillis()
{
	unsigned int t = Timer();
	unsigned int dm, dt = t - _t0;
	
	_t0 = t;
	dm = (dt << 6)/3125U;
	dt -= (dm * 3125U)>>6;
	_t0 -= dt;
	
	return _msecs += dm;
}

void DriverRonin::delayMillis(uint msecs)
{
	getMillis();
	unsigned int t, start = Timer();
	int time = (((unsigned int)msecs)*100000U)>>11;
	while(((int)((t = Timer())-start))<time) {
		if(_timer_active && ((int)(t-_timer_next_expiry))>=0) {
			_timer_duration = _timer_callback(_timer_duration);
			_timer_next_expiry = t+USEC_TO_TIMER(1000*_timer_duration);
		}
		checkSound();
	}
	getMillis();
}

void DriverRonin::setTimerCallback(TimerProc callback, int interval)
{
	if (callback != NULL) {
		_timer_duration = interval;
		_timer_next_expiry = Timer() + USEC_TO_TIMER(1000*interval);
		_timer_callback = callback;
		_timer_active = true;
	} else {
		_timer_active = false;
	}
}

void DriverRonin::initEvent()
{
	_devpoll = 0;
	_msecs = 0;
	_t0 = 0;
	_timer_active = false;
	maple_init();
	_joy1_state.present = false;
	_joy2_state.present = false;
	_mouse_state.present = false;
	_kbd_state.present = false;
	_kbd_state.flags = 0;
}
