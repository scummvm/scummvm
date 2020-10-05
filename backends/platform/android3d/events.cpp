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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android3d/android.h"
#include "backends/platform/android3d/graphics.h"
#include "backends/platform/android3d/events.h"
#include "backends/platform/android3d/jni-android.h"

#include "engines/engine.h"
#include "gui/gui-manager.h"

// floating point. use sparingly
template<class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

static const int kQueuedInputEventDelay = 50;

void OSystem_Android::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareInputSet *inputSet = new HardwareInputSet();

	keySet->addHardwareInput(
		new HardwareInput("n", KeyState(KEYCODE_n), "n (vk)"));

	mapper->registerHardwareInputSet(inputSet);

	Keymap *globalMap = new Keymap(kGlobalKeymapName);
	Action *act;

	act = new Action(globalMap, "VIRT", "Display keyboard");
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, KBD_CTRL));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap(kGlobalKeymapName);
#endif
}

void OSystem_Android::clipMouse(Common::Point &p) {
	dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->clipMouse(p);
}

void OSystem_Android::scaleMouse(Common::Point &p, int x, int y, bool touchpadMode) {
	const GLESBaseTexture *tex =
		dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getActiveTexture();

	const Common::Rect &r = tex->getDrawRect();

	if (touchpadMode) {
		x = x * 100 / _touchpad_scale;
		y = y * 100 / _touchpad_scale;
	}

	p.x = scalef(x, tex->width(), r.width());
	p.y = scalef(y, tex->height(), r.height());
}

void OSystem_Android::updateEventScale(const GLESBaseTexture *tex) {
    if (tex && (tex->height() != 0) && (tex->width() != 0)) {
	    _eventScaleY = 100 * 480 / tex->height();
	    _eventScaleX = 100 * 640 / tex->width();
    }
}

void OSystem_Android::pushEvent(int type, int arg1, int arg2, int arg3,
								int arg4, int arg5, int arg6) {
	Common::Event e;

	switch (type) {
	case JE_SYS_KEY:
		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on system key: %d", arg1);
			return;
		}

		switch (arg2) {

		// special case. we'll only get it's up event
		case JKEYCODE_BACK:
			e.kbd.keycode = Common::KEYCODE_ESCAPE;
			e.kbd.ascii = Common::ASCII_ESCAPE;

			_event_queue_lock->lock();
			e.type = Common::EVENT_KEYDOWN;
			_event_queue.push(e);
			e.type = Common::EVENT_KEYUP;
			_event_queue.push(e);
			_event_queue_lock->unlock();

			return;

		// special case. we'll only get it's up event
		case JKEYCODE_MENU:
			e.type = Common::EVENT_MAINMENU;

			pushEvent(e);

			return;

		case JKEYCODE_MEDIA_PAUSE:
		case JKEYCODE_MEDIA_PLAY:
		case JKEYCODE_MEDIA_PLAY_PAUSE:
			if (arg1 == JACTION_DOWN) {
				e.type = Common::EVENT_MAINMENU;

				pushEvent(e);
			}

			return;

		case JKEYCODE_CAMERA:
		case JKEYCODE_SEARCH:
			if (arg1 == JACTION_DOWN)
				e.type = Common::EVENT_RBUTTONDOWN;
			else
				e.type = Common::EVENT_RBUTTONUP;

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			pushEvent(e);

			return;

		default:
			LOGW("unmapped system key: %d", arg2);
			return;
		}

		break;

	case JE_KEY:
		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on key: %d", arg1);
			return;
		}

		if (arg2 < 1 || arg2 > ARRAYSIZE(jkeymap)) {
			if (arg3 < 1) {
				LOGE("received invalid keycode: %d (%d)", arg2, arg3);
				return;
			} else {
				// lets bet on the ascii code
				e.kbd.keycode = Common::KEYCODE_INVALID;
			}
		} else {
			e.kbd.keycode = jkeymap[arg2];
		}

		if (arg5 > 0)
			e.kbdRepeat = true;

		// map special keys to 'our' ascii codes
		switch (e.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			e.kbd.ascii = Common::ASCII_BACKSPACE;
			break;
		case Common::KEYCODE_TAB:
			e.kbd.ascii = Common::ASCII_TAB;
			break;
		case Common::KEYCODE_RETURN:
			e.kbd.ascii = Common::ASCII_RETURN;
			break;
		case Common::KEYCODE_ESCAPE:
			e.kbd.ascii = Common::ASCII_ESCAPE;
			break;
		case Common::KEYCODE_SPACE:
			e.kbd.ascii = Common::ASCII_SPACE;
			break;
		case Common::KEYCODE_F1:
			e.kbd.ascii = Common::ASCII_F1;
			break;
		case Common::KEYCODE_F2:
			e.kbd.ascii = Common::ASCII_F2;
			break;
		case Common::KEYCODE_F3:
			e.kbd.ascii = Common::ASCII_F3;
			break;
		case Common::KEYCODE_F4:
			e.kbd.ascii = Common::ASCII_F4;
			break;
		case Common::KEYCODE_F5:
			e.kbd.ascii = Common::ASCII_F5;
			break;
		case Common::KEYCODE_F6:
			e.kbd.ascii = Common::ASCII_F6;
			break;
		case Common::KEYCODE_F7:
			e.kbd.ascii = Common::ASCII_F7;
			break;
		case Common::KEYCODE_F8:
			e.kbd.ascii = Common::ASCII_F8;
			break;
		case Common::KEYCODE_F9:
			e.kbd.ascii = Common::ASCII_F9;
			break;
		case Common::KEYCODE_F10:
			e.kbd.ascii = Common::ASCII_F10;
			break;
		case Common::KEYCODE_F11:
			e.kbd.ascii = Common::ASCII_F11;
			break;
		case Common::KEYCODE_F12:
			e.kbd.ascii = Common::ASCII_F12;
			break;
		default:
			e.kbd.ascii = arg3;
			break;
		}

		if (arg4 & JMETA_SHIFT)
			e.kbd.flags |= Common::KBD_SHIFT;
		// JMETA_ALT is Fn on physical keyboards!
		// when mapping this to ALT - as we know it from PC keyboards - all
		// Fn combos will be broken (like Fn+q, which needs to end as 1 and
		// not ALT+1). Do not want.
		//if (arg4 & JMETA_ALT)
		//	e.kbd.flags |= Common::KBD_ALT;
		if (arg4 & (JMETA_SYM | JMETA_CTRL))
			e.kbd.flags |= Common::KBD_CTRL;

		pushEvent(e);

		return;

	case JE_DPAD:
		switch (arg2) {
		case JKEYCODE_DPAD_UP:
		case JKEYCODE_DPAD_DOWN:
		case JKEYCODE_DPAD_LEFT:
		case JKEYCODE_DPAD_RIGHT:
			if (arg1 != JACTION_DOWN)
				return;

			e.type = Common::EVENT_MOUSEMOVE;

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			{
				int16 *c;
				int s;

				if (arg2 == JKEYCODE_DPAD_UP || arg2 == JKEYCODE_DPAD_DOWN) {
					c = &e.mouse.y;
					s = _eventScaleY;
				} else {
					c = &e.mouse.x;
					s = _eventScaleX;
				}

				// the longer the button held, the faster the pointer is
				// TODO put these values in some option dlg?
				int f = CLIP(arg4, 1, 8) * _dpad_scale * 100 / s;

				if (arg2 == JKEYCODE_DPAD_UP || arg2 == JKEYCODE_DPAD_LEFT)
					*c -= f;
				else
					*c += f;
			}

			clipMouse(e.mouse);

			pushEvent(e);

			return;

		case JKEYCODE_DPAD_CENTER:
			switch (arg1) {
			case JACTION_DOWN:
				e.type = Common::EVENT_LBUTTONDOWN;
				break;
			case JACTION_UP:
				e.type = Common::EVENT_LBUTTONUP;
				break;
			default:
				LOGE("unhandled jaction on dpad key: %d", arg1);
				return;
			}

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			pushEvent(e);

			return;
		}

	case JE_DOWN:
		_touch_pt_down = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
		break;

	case JE_SCROLL:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			scaleMouse(e.mouse, arg3 - _touch_pt_scroll.x,
						arg4 - _touch_pt_scroll.y, true);
			e.mouse += _touch_pt_down;
			scaleMouse(e.relMouse, arg3 - _touch_pt_scroll.x,
						arg4 - _touch_pt_scroll.y, true);
		} else {
			scaleMouse(e.mouse, arg3, arg4);
		}

		pushEvent(e);

		return;

	case JE_TAP:
		if (_fingersDown > 0) {
			_fingersDown = 0;
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		} else {
			scaleMouse(e.mouse, arg1, arg2);
		}

		{
			Common::EventType down, up;

			// TODO put these values in some option dlg?
			if (arg3 > 1000) {
				down = Common::EVENT_MBUTTONDOWN;
				up = Common::EVENT_MBUTTONUP;
			} else if (arg3 > 500) {
				down = Common::EVENT_RBUTTONDOWN;
				up = Common::EVENT_RBUTTONUP;
			} else {
				down = Common::EVENT_LBUTTONDOWN;
				up = Common::EVENT_LBUTTONUP;
			}

			_event_queue_lock->lock();

			if (_queuedEventTime)
				_event_queue.push(_queuedEvent);

			if (!_touchpad_mode)
				_event_queue.push(e);

			e.type = down;
			_event_queue.push(e);

			e.type = up;
			_queuedEvent = e;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;

			_event_queue_lock->unlock();
		}

		return;

	case JE_DOUBLE_TAP:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		} else {
			scaleMouse(e.mouse, arg1, arg2);
		}

		{
			Common::EventType dptype = Common::EVENT_INVALID;

			switch (arg3) {
			case JACTION_DOWN:
				dptype = Common::EVENT_LBUTTONDOWN;
				_touch_pt_dt.x = -1;
				_touch_pt_dt.y = -1;
				break;
			case JACTION_UP:
				dptype = Common::EVENT_LBUTTONUP;
				break;
			// held and moved
			case JACTION_MULTIPLE:
				if (_touch_pt_dt.x == -1 && _touch_pt_dt.y == -1) {
					_touch_pt_dt.x = arg1;
					_touch_pt_dt.y = arg2;
					return;
				}

				dptype = Common::EVENT_MOUSEMOVE;

				if (_touchpad_mode) {
					scaleMouse(e.mouse, arg1 - _touch_pt_dt.x,
								arg2 - _touch_pt_dt.y, true);
					e.mouse += _touch_pt_down;
				}

				break;
			default:
				LOGE("unhandled jaction on double tap: %d", arg3);
				return;
			}

			_event_queue_lock->lock();
			_event_queue.push(e);
			e.type = dptype;
			_event_queue.push(e);
			_event_queue_lock->unlock();
		}

		return;


	case JE_TOUCH:
	case JE_MULTI:
		switch (arg2) {
		case JACTION_POINTER_DOWN:
			if (arg1 > _fingersDown)
				_fingersDown = arg1;

			return;

		case JACTION_POINTER_UP:
			if (arg1 != _fingersDown)
				return;

			{
				Common::EventType up;

				switch (_fingersDown) {
				case 1:
					e.type = Common::EVENT_RBUTTONDOWN;
					up = Common::EVENT_RBUTTONUP;
					break;
				case 2:
					e.type = Common::EVENT_MBUTTONDOWN;
					up = Common::EVENT_MBUTTONUP;
					break;
				default:
					LOGD("unmapped multi tap: %d", _fingersDown);
					return;
				}

				e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

				_event_queue_lock->lock();

				if (_queuedEventTime)
					_event_queue.push(_queuedEvent);

				_event_queue.push(e);

				e.type = up;
				_queuedEvent = e;
				_queuedEventTime = getMillis() + kQueuedInputEventDelay;

				_event_queue_lock->unlock();
				return;

			default:
				LOGE("unhandled jaction on multi tap: %d", arg2);
				return;
			}
		}

		return;

	case JE_BALL:
		e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_LBUTTONDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_LBUTTONUP;
			break;
		case JACTION_MULTIPLE:
			e.type = Common::EVENT_MOUSEMOVE;

			// already multiplied by 100
			e.mouse.x += arg2 * _trackball_scale / _eventScaleX;
			e.mouse.y += arg3 * _trackball_scale / _eventScaleY;

			clipMouse(e.mouse);

			break;
		default:
			LOGE("unhandled jaction on system key: %d", arg1);
			return;
		}

		pushEvent(e);

		return;

	case JE_MOUSE_MOVE:
		e.type = Common::EVENT_MOUSEMOVE;
		scaleMouse(e.mouse, arg1, arg2);

		pushEvent(e);

		return;

	case JE_LMB_DOWN:
		e.type = Common::EVENT_LBUTTONDOWN;
		scaleMouse(e.mouse, arg1, arg2);

		pushEvent(e);

		return;

	case JE_LMB_UP:
		e.type = Common::EVENT_LBUTTONUP;
		scaleMouse(e.mouse, arg1, arg2);

		pushEvent(e);

		return;

	case JE_RMB_DOWN:
		e.type = Common::EVENT_RBUTTONDOWN;
		scaleMouse(e.mouse, arg1, arg2);

		pushEvent(e);

		return;

	case JE_RMB_UP:
		e.type = Common::EVENT_RBUTTONUP;
		scaleMouse(e.mouse, arg1, arg2);

		pushEvent(e);

		return;

	case JE_MMB_DOWN:
		e.type = Common::EVENT_MAINMENU;

		pushEvent(e);

		return;

	case JE_MMB_UP:
		// No action

		return;

	case JE_QUIT:
		e.type = Common::EVENT_QUIT;

		pushEvent(e);

		return;

	default:
		LOGE("unknown jevent type: %d", type);

		break;
	}
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER();

	if (pthread_self() == _main_thread) {
		if (_screen_changeid != JNI::surface_changeid) {
			_screen_changeid = JNI::surface_changeid;

			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				// surface changed
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->deinitSurface();
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->initSurface();
				_touchControls.init(JNI::egl_surface_width, JNI::egl_surface_height);

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			} else {
				// surface lost
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->deinitSurface();
			}
		}

		if (JNI::pause) {
			LOGD("main thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("main thread woke up");
		}
	}

	_event_queue_lock->lock();

	if (_queuedEventTime && (getMillis() > _queuedEventTime)) {
		event = _queuedEvent;
		_queuedEventTime = 0;
		// _event_queue_lock->unlock();
		// return true;
	} else if (_event_queue.empty()) {
		_event_queue_lock->unlock();
		return false;
	} else {
		event = _event_queue.pop();
	}

	_event_queue_lock->unlock();

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		if (_graphicsManager)
			return dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->notifyMousePosition(event.mouse);
		break;
	default:
		break;
	}

	return true;
}

void OSystem_Android::pushEvent(const Common::Event &event) {
	_event_queue_lock->lock();
	_event_queue.push(event);
	_event_queue_lock->unlock();
}

void OSystem_Android::pushKeyPressEvent(Common::Event &event) {
	_event_queue_lock->lock();
	event.type = Common::EVENT_KEYDOWN;
	_event_queue.push(event);
	event.type = Common::EVENT_KEYUP;
	_event_queue.push(event);
	_event_queue_lock->unlock();
}

#endif
