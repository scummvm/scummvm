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
 */

#include <unistd.h>
#include <malloc.h>

#ifndef GAMECUBE
#include <wiiuse/wpad.h>
#endif

#include <ogc/lwp_watchdog.h>

#include "osystem.h"

#define TIMER_THREAD_STACKSIZE (1024 * 32)
#define TIMER_THREAD_PRIO 64

#define PAD_CHECK_TIME 40

#ifndef GAMECUBE
#define PADS_A (PAD_BUTTON_A | (WPAD_BUTTON_A << 16))
#define PADS_B (PAD_BUTTON_B | (WPAD_BUTTON_B << 16))
#define PADS_X (PAD_BUTTON_X | (WPAD_BUTTON_MINUS << 16))
#define PADS_Y (PAD_BUTTON_Y | (WPAD_BUTTON_PLUS << 16))
#define PADS_Z (PAD_TRIGGER_Z | (WPAD_BUTTON_2 << 16))
#define PADS_START (PAD_BUTTON_START | (WPAD_BUTTON_HOME << 16))
#define PADS_UP (PAD_BUTTON_UP | (WPAD_BUTTON_UP << 16))
#define PADS_DOWN (PAD_BUTTON_DOWN | (WPAD_BUTTON_DOWN << 16))
#define PADS_LEFT (PAD_BUTTON_LEFT | (WPAD_BUTTON_LEFT << 16))
#define PADS_RIGHT (PAD_BUTTON_RIGHT | (WPAD_BUTTON_RIGHT << 16))
#else
#define PADS_A PAD_BUTTON_A
#define PADS_B PAD_BUTTON_B
#define PADS_X PAD_BUTTON_X
#define PADS_Y PAD_BUTTON_Y
#define PADS_Z PAD_TRIGGER_Z
#define PADS_START PAD_BUTTON_START
#define PADS_UP PAD_BUTTON_UP
#define PADS_DOWN PAD_BUTTON_DOWN
#define PADS_LEFT PAD_BUTTON_LEFT
#define PADS_RIGHT PAD_BUTTON_RIGHT
#endif

static lwpq_t timer_queue;
static lwp_t timer_thread;
static u8 *timer_stack;
static bool timer_thread_running = false;
static bool timer_thread_quit = false;

static void * timer_thread_func(void *arg) {
	while (!timer_thread_quit) {
		DefaultTimerManager *tm =
			(DefaultTimerManager *) g_system->getTimerManager();
		tm->handler();

		usleep(1000 * 10);
	}

	return NULL;
}

void OSystem_Wii::initEvents() {
	timer_thread_quit = false;

	timer_stack = (u8 *) memalign(32, TIMER_THREAD_STACKSIZE);
	memset(timer_stack, 0, TIMER_THREAD_STACKSIZE);

	LWP_InitQueue(&timer_queue);

	s32 res = LWP_CreateThread(&timer_thread, timer_thread_func, NULL,
								timer_stack, TIMER_THREAD_STACKSIZE,
								TIMER_THREAD_PRIO);

	if (res) {
		printf("ERROR creating timer thread: %d\n", res);
		LWP_CloseQueue(timer_queue);
	}

	timer_thread_running = res == 0;

#ifndef GAMECUBE
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetIdleTimeout(120);
#endif
}

void OSystem_Wii::deinitEvents() {
	if (timer_thread_running) {
		timer_thread_quit = true;
		LWP_ThreadBroadcast(timer_queue);

		LWP_JoinThread(timer_thread, NULL);
		LWP_CloseQueue(timer_queue);

		timer_thread_running = false;
	}

#ifndef GAMECUBE
	WPAD_Shutdown();
#endif
}

void OSystem_Wii::updateEventScreenResolution() {
#ifndef GAMECUBE
	WPAD_SetVRes(WPAD_CHAN_0, _currentWidth + _currentWidth / 5,
					_currentHeight + _currentHeight / 5);
#endif
}

#define KBD_EVENT(pad_button, kbd_keycode, kbd_ascii) \
	do { \
		if ((bd | bu) & pad_button) { \
			if (bd & pad_button) \
				event.type = Common::EVENT_KEYDOWN; \
			else \
				event.type = Common::EVENT_KEYUP; \
			event.kbd.keycode = kbd_keycode; \
			event.kbd.ascii = kbd_ascii; \
			return true; \
		} \
	} while (0)

bool OSystem_Wii::pollEvent(Common::Event &event) {
	u32 bd, bh, bu;

	PAD_ScanPads();

	bd = PAD_ButtonsDown(0);
	bh = PAD_ButtonsHeld(0);
	bu = PAD_ButtonsUp(0);

#ifndef GAMECUBE
	WPAD_ScanPads();

	s32 res = WPAD_Probe(0, NULL);

	if (res == WPAD_ERR_NONE) {

		bd |= WPAD_ButtonsDown(0) << 16;
		bh |= WPAD_ButtonsHeld(0) << 16;
		bu |= WPAD_ButtonsUp(0) << 16;
	}
#endif

	if (bd || bu) {
		if (bh & PADS_UP)
			event.kbd.flags = Common::KBD_SHIFT;

		KBD_EVENT(PADS_Z, Common::KEYCODE_RETURN, Common::ASCII_RETURN);
		KBD_EVENT(PADS_X, Common::KEYCODE_ESCAPE, Common::ASCII_ESCAPE);
		KBD_EVENT(PADS_Y, Common::KEYCODE_PERIOD, '.');
		KBD_EVENT(PADS_START, Common::KEYCODE_F5, Common::ASCII_F5);
		KBD_EVENT(PADS_UP, Common::KEYCODE_LSHIFT, 0);
		KBD_EVENT(PADS_DOWN, Common::KEYCODE_0, '0');
		KBD_EVENT(PADS_LEFT, Common::KEYCODE_y, 'y');
		KBD_EVENT(PADS_RIGHT, Common::KEYCODE_n, 'n');

		if ((bd | bu) & (PADS_A | PADS_B)) {
			if (bd & PADS_A)
				event.type = Common::EVENT_LBUTTONDOWN;
			else if (bu & PADS_A)
				event.type = Common::EVENT_LBUTTONUP;
			else if (bd & PADS_B)
				event.type = Common::EVENT_RBUTTONDOWN;
			else if (bu & PADS_B)
				event.type = Common::EVENT_RBUTTONUP;

			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;

			return true;
		}
	}

	s32 mx = _mouseX;
	s32 my = _mouseY;

#ifndef GAMECUBE
	if (res == WPAD_ERR_NONE) {
		struct ir_t ir;

		WPAD_IR(0, &ir);

		if (ir.valid) {
			mx = ir.x - _currentWidth / 10;
			my = ir.y - _currentHeight / 10;

			if (mx < 0)
				mx = 0;

			if (mx >= _currentWidth)
				mx = _currentWidth - 1;

			if (my < 0)
				my = 0;

			if (my >= _currentHeight)
				my = _currentHeight - 1;

			if ((mx != _mouseX) || (my != _mouseY)) {
				event.type = Common::EVENT_MOUSEMOVE;
				event.mouse.x = _mouseX = mx;
				event.mouse.y = _mouseY = my;

				return true;
			}
		}
	}
#endif

	uint32 time = getMillis();
	if (time - _lastPadCheck > PAD_CHECK_TIME) {
		_lastPadCheck = time;

		if (abs (PAD_StickX(0)) > 16)
			mx += PAD_StickX(0) / (4 * _overlayWidth / _currentWidth);
		if (abs (PAD_StickY(0)) > 16)
			my -= PAD_StickY(0) / (4 * _overlayHeight / _currentHeight);

		if (mx < 0)
			mx = 0;

		if (mx >= _currentWidth)
			mx = _currentWidth - 1;

		if (my < 0)
			my = 0;

		if (my >= _currentHeight)
			my = _currentHeight - 1;

		if ((mx != _mouseX) || (my != _mouseY)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse.x = _mouseX = mx;
			event.mouse.y = _mouseY = my;

			return true;
		}
	}

	return false;
}

