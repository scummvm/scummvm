package org.scummvm.scummvm;

import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.GestureDetector;
import android.view.HapticFeedbackConstants;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import java.lang.ref.WeakReference;

public class ScummVMEvents implements
		android.view.View.OnKeyListener,
		android.view.View.OnTouchListener,
		android.view.GestureDetector.OnGestureListener,
		android.view.GestureDetector.OnDoubleTapListener {

	public static final int JE_SYS_KEY = 0;
	public static final int JE_KEY = 1;
	public static final int JE_DPAD = 2;
	public static final int JE_DOWN = 3;
	public static final int JE_SCROLL = 4;
	public static final int JE_TAP = 5;
	public static final int JE_DOUBLE_TAP = 6;
	public static final int JE_MULTI = 7;
	public static final int JE_BALL = 8;
	public static final int JE_LMB_DOWN = 9;
	public static final int JE_LMB_UP = 10;
	public static final int JE_RMB_DOWN = 11;
	public static final int JE_RMB_UP = 12;
	public static final int JE_MOUSE_MOVE = 13;
	public static final int JE_GAMEPAD = 14;
	public static final int JE_JOYSTICK = 15;
	public static final int JE_MMB_DOWN = 16;
	public static final int JE_MMB_UP = 17;
	public static final int JE_BMB_DOWN = 18;
	public static final int JE_BMB_UP = 19;
	public static final int JE_FMB_DOWN = 20;
	public static final int JE_FMB_UP = 21;
	public static final int JE_MOUSE_WHEEL_UP = 22;
	public static final int JE_MOUSE_WHEEL_DOWN = 23;
	public static final int JE_TV_REMOTE = 24;
	public static final int JE_QUIT = 0x1000;
	public static final int JE_MENU = 0x1001;

	public static final int JACTION_DOWN = 0;
	public static final int JACTION_MOVE = 1;
	public static final int JACTION_UP = 2;
	public static final int JACTION_CANCEL = 3;

	public static final int TOUCH_MODE_TOUCHPAD = 0;
	public static final int TOUCH_MODE_MOUSE = 1;
	public static final int TOUCH_MODE_GAMEPAD = 2;
	public static final int TOUCH_MODE_MAX = 3;

	public static final int JOYSTICK_AXIS_MAX = 32767; // matches the definition in common/events of "const int16 JOYAXIS_MAX = 32767;"
	public static final float JOYSTICK_AXIS_HAT_SCALE = 0.66f; // ie. 2/3 to be applied to JOYSTICK_AXIS_MAX

	// auxiliary movement axis bitflags
	// Also repeated (and used) in android's events.cpp (JE_JOYSTICK case)
	private static final int JOYSTICK_AXIS_X_bf        = 0x01; // (0x01 << 0)
	private static final int JOYSTICK_AXIS_Y_bf        = 0x02; // (0x01 << 1)
	private static final int JOYSTICK_AXIS_HAT_X_bf    = 0x04; // (0x01 << 2)
	private static final int JOYSTICK_AXIS_HAT_Y_bf    = 0x08; // (0x01 << 3)
	private static final int JOYSTICK_AXIS_Z_bf        = 0x10; // (0x01 << 4)
	private static final int JOYSTICK_AXIS_RZ_bf       = 0x20; // (0x01 << 5)
	private static final int JOYSTICK_AXIS_LTRIGGER_bf = 0x40; // (0x01 << 6)
	private static final int JOYSTICK_AXIS_RTRIGGER_bf = 0x80; // (0x01 << 7)

	private static final int REPEAT_INTERVAL = 20; // ~50 keys per second
	private static final int REPEAT_START_DELAY = 40;

	private final float[] _repeatingJoystickCenteredAxisValuesArray = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	private int _repeatingJoystickAxisIdBitFlags = 0x00;

	final protected ScummVMActivity _activity;
	final protected ScummVM _scummvm;
	final protected GestureDetector _gd;
	final protected int _longPressTimeout;
	final protected MouseHelper _mouseHelper;
	final protected MultitouchHelper _multitouchHelper;

	protected View _currentView;
	protected int _touchMode;

	protected boolean _doubleTapMode;

	// Custom handler code (to avoid mem leaks, see warning "This Handler Class Should Be Static Or Leaks Might Occur”) based on:
	// https://stackoverflow.com/a/27826094
	public static class ScummVMEventHandler extends Handler {

		private final WeakReference<ScummVMEvents> mListenerReference;

		public ScummVMEventHandler(ScummVMEvents listener) {
			super(Looper.getMainLooper());
			mListenerReference = new WeakReference<>(listener);
		}

		@Override
		public synchronized void handleMessage(@NonNull Message msg) {
			ScummVMEvents listener = mListenerReference.get();
			if(listener != null) {
				switch (msg.what) {
					case MSG_REPEAT:
						if (listener.repeatMove(0, false)) {
							Message repeat = Message.obtain(this, MSG_REPEAT);
							sendMessageDelayed(repeat, REPEAT_INTERVAL);
						}
						break;
					default:
						listener.handleEVHMessage(msg);
						break;
				}
			}
		}

		public void clear() {
			this.removeCallbacksAndMessages(null);
		}
	}

	@RequiresApi(android.os.Build.VERSION_CODES.TIRAMISU)
	private class OnBackInvoked implements OnBackInvokedCallback {
		@Override
		public void onBackInvoked() {
			//Log.d(ScummVM.LOG_TAG,"Sending back key");
			ScummVMEvents.this._scummvm.pushEvent(JE_SYS_KEY, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK,
					0, 0, 0, 0);
			ScummVMEvents.this._scummvm.pushEvent(JE_SYS_KEY, KeyEvent.ACTION_UP, KeyEvent.KEYCODE_BACK,
					0, 0, 0, 0);
		}
	}

	final private ScummVMEventHandler _handler = new ScummVMEventHandler(this);

//	/**
//	 * An example getter to provide it to some external class
//	 * or just use 'new MyHandler(this)' if you are using it internally.
//	 * If you only use it internally you might even want it as final member:
//	 * private final MyHandler mHandler = new MyHandler(this);
//	 */
//	public Handler ScummVMEventHandler() {
//		return new ScummVMEventHandler(this);
//	}

	public ScummVMEvents(ScummVMActivity activity, ScummVM scummvm, MouseHelper mouseHelper) {
		_activity = activity;
		_scummvm = scummvm;
		// Careful, _mouseHelper can be null (if HoverListener is not available for the device API -- old devices, API < 9)
		_mouseHelper = mouseHelper;

		_multitouchHelper = new MultitouchHelper(_scummvm);

		_gd = new GestureDetector(activity, this);
		_gd.setOnDoubleTapListener(this);
		_gd.setIsLongpressEnabled(false);

		_doubleTapMode = false;
		_longPressTimeout = ViewConfiguration.getLongPressTimeout();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
			activity.getOnBackInvokedDispatcher().registerOnBackInvokedCallback(OnBackInvokedDispatcher.PRIORITY_DEFAULT, new OnBackInvoked());
		}
	}

	final static int MSG_SMENU_LONG_PRESS = 1;
	final static int MSG_SBACK_LONG_PRESS = 2;
	final static int MSG_LONG_TOUCH_EVENT = 3;
	final static int MSG_REPEAT = 4;

	private void handleEVHMessage(final Message msg) {
		if (msg.what == MSG_SMENU_LONG_PRESS) {
			// this toggles the android keyboard (see showVirtualKeyboard() in ScummVMActivity.java)
			// when menu key is long-pressed
//			InputMethodManager imm = (InputMethodManager)
//				_activity.getSystemService(Context.INPUT_METHOD_SERVICE);
//
//			if (imm != null)
//				imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
			_activity.toggleScreenKeyboard();
		} else if (msg.what == MSG_SBACK_LONG_PRESS) {
			_scummvm.pushEvent(JE_SYS_KEY,
			                   KeyEvent.ACTION_DOWN,
			                   KeyEvent.KEYCODE_MENU,
			                   0,
			                   0,
			                   0,
			                   0);
			_scummvm.pushEvent(JE_SYS_KEY,
			                   KeyEvent.ACTION_UP,
			                   KeyEvent.KEYCODE_MENU,
			                   0,
			                   0,
			                   0,
			                   0);
		} else if (msg.what == MSG_LONG_TOUCH_EVENT) {
			if (!_multitouchHelper.isMultitouchMode() && getTouchMode() != TOUCH_MODE_GAMEPAD && !_doubleTapMode) {
				_currentView.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
			}
		}
	}

	final public int getTouchMode() {
		return _touchMode;
	}

	final public void setTouchMode(int touchMode) {
		assert (touchMode >= 0) && (touchMode < TOUCH_MODE_MAX);

		if (_touchMode == touchMode) {
			return;
		}

		if (_touchMode == TOUCH_MODE_GAMEPAD) {
			// We were in gamepad mode and we leave it
			_scummvm.updateTouch(JACTION_CANCEL, 0, 0, 0);
		}
		int oldTouchMode = _touchMode;
		_touchMode = touchMode;
		_scummvm.setupTouchMode(oldTouchMode, _touchMode);
	}

	final public int nextTouchMode() {
		if (_touchMode == TOUCH_MODE_GAMEPAD) {
			// We leave gamepad mode
			_scummvm.updateTouch(JACTION_CANCEL, 0, 0, 0);
		}
		int oldTouchMode = _touchMode;
		_touchMode = (_touchMode + 1) % TOUCH_MODE_MAX;
		_scummvm.setupTouchMode(oldTouchMode, _touchMode);

		return _touchMode;
	}

	public void clearEventHandler() {
		_handler.clear();
		_multitouchHelper.clearEventHandler();
	}

	final public void sendQuitEvent() {
		_scummvm.pushEvent(JE_QUIT, 0, 0, 0, 0, 0, 0);
	}

	public boolean onTrackballEvent(MotionEvent e) {
//		Log.d(ScummVM.LOG_TAG, "SCUMMV-EVENTS-BASE - onTrackballEvent");
		_scummvm.pushEvent(JE_BALL, e.getAction(),
			(int)(e.getX() * e.getXPrecision() * 100),
			(int)(e.getY() * e.getYPrecision() * 100),
			0, 0, 0);
		return true;
	}

	public boolean onGenericMotionEvent(MotionEvent event) {
		// TODO Make Use of MotionEvent.getToolType() ie. for handling TOOL_TYPE_FINGER/ TOOL_TYPE_MOUSE/ TOOL_TYPE_STYLUS/ TOOL_TYPE_UNKNOWN?
		// Check that the event came from a joystick
		if (((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK
			 || (event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0)) {
			switch(event.getActionMasked()) {
			case MotionEvent.ACTION_MOVE:
				//InputDevice inputDevice = event.getDevice();
				//Log.d(ScummVM.LOG_TAG, "JOYSTICK GENERIC MOTION: MOVE, Devname=" + inputDevice.getName() + " pid=" + inputDevice.getProductId() + " vid=" + inputDevice.getVendorId());
				// NOTE In Android 12 (on some early version patch) support for PS5's DualSense broke, and the key mappings are messed up.
				// This was fixed in another Android 12 patch, but not all devices got that. (eg Redmi 9 Pro does not have this update)
				// Details about this here: https://stackoverflow.com/questions/68190869/dualshock-5-and-android
				// Not much we can do about this.

				// Process all historical movement samples in the batch
				final int historySize = event.getHistorySize();

				// Process the movements starting from the
				// earliest historical position in the batch
				for (int i = 0; i < historySize; ++i) {
					// Process the event at historical position i
					//Log.d(ScummVM.LOG_TAG, "JOYSTICK - onGenericMotionEvent(m) hist: ");
					processJoystickInput(event, i);
				}

				// Process the current movement sample in the batch (position -1)
				//Log.d(ScummVM.LOG_TAG, "JOYSTICK - onGenericMotionEvent(m): "  );
				processJoystickInput(event, -1);
				return true;

			default:
				break;
			}
		} else if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
			//Log.d(ScummVM.LOG_TAG, "MOUSE PHYSICAL POINTER - onGenericMotionEvent(m) ");
			//
			// Check that the event might be a mouse scroll wheel (ACTION_SCROLL)
			// Code inspired from https://stackoverflow.com/a/33086042
			//
			// NOTE Other GenericMotionEvent are also triggered for InputDevice of SOURCE_CLASS_POINTER (eg. physical mouse).
			// These seem to be for button down/up events, which are handled along with pushing a JE_MOVE event
			// in MouseHelper's onMouseEvent() called from onTouch().
			switch (event.getActionMasked()) {
			case MotionEvent.ACTION_SCROLL:
				//Log.d(ScummVM.LOG_TAG, "MOUSE PHYSICAL POINTER - ACTION SCROLL");
				// This action is not a touch event so it is delivered to
				// View#onGenericMotionEvent(MotionEvent) rather than View#onTouchEvent(MotionEvent).
				if (_mouseHelper != null) {
					return _mouseHelper.onMouseEvent(event, false);
				}
				break;

			default:
				break;
			}
		}
		//Log.d(ScummVM.LOG_TAG, "MOTION NOT HANDLED, source: " + event.getSource() + " event: "+ event.getActionMasked());
		return false;
	}

	// OnKeyListener
	@Override
	final public boolean onKey(View v, int keyCode, KeyEvent e) {

//		String actionStr = "";
//		switch (e.getAction()) {
//			case KeyEvent.ACTION_UP:
//				actionStr = "KeyEvent.ACTION_UP";
//				break;
//			case KeyEvent.ACTION_DOWN:
//				actionStr = "KeyEvent.ACTION_DOWN";
//				break;
//			default:
//				if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && e.getAction() == KeyEvent.ACTION_MULTIPLE) {
//					actionStr = "KeyEvent.ACTION_MULTIPLE";
//				} else {
//					actionStr = e.toString();
//				}
//		}
//		Log.d(ScummVM.LOG_TAG, "SCUMMV-EVENTS-BASE - onKEY:::" + keyCode + " Action::" + actionStr); // Called

		_currentView = v;
		final int action = e.getAction();

		int eventUnicodeChar = e.getUnicodeChar();
		if (e.getDeviceId() != 0) {
			// getDeviceId: Gets the id for the device that this event came from.
			// An id of zero indicates that the event didn't come from a physical device and maps to the default keymap.
			// The other numbers are arbitrary and you shouldn't depend on the values.
			final KeyCharacterMap m = KeyCharacterMap.load(e.getDeviceId());
			eventUnicodeChar = m.get(e.getKeyCode(), e.getMetaState());
		}

		if (eventUnicodeChar == (int)EditableAccommodatingLatinIMETypeNullIssues.ONE_UNPROCESSED_CHARACTER.charAt(0)) {
			//We are ignoring this character, and we want everyone else to ignore it, too, so
			// we return true indicating that we have handled it (by ignoring it).
			return true;
		}

		if (keyCode == 238) {
			// this (undocumented) event is sent when ACTION_HOVER_ENTER or ACTION_HOVER_EXIT occurs
			return false;
		}

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if (_mouseHelper != null) {
				if (MouseHelper.isMouse(e)) {
					// right mouse button was just clicked which sends an extra back button press (which should be ignored)
					return true;
				}
			}

			if (ScummVMActivity.keyboardWithoutTextInputShown ) {
				if (action == KeyEvent.ACTION_DOWN) {
					return true;
				} else if (action == KeyEvent.ACTION_UP) {
					// Hide keyboard
					if (_activity.isScreenKeyboardShown()) {
						_activity.hideScreenKeyboard();
					}
					return true;
				}
			}
		}

		if (e.isSystem() || keyCode == KeyEvent.KEYCODE_MENU) {
			// no repeats for system keys
			if (e.getRepeatCount() > 0) {
				return false;
			}

			// Have to reimplement hold-down-menu-brings-up-softkeybd
			// ourselves, since we are otherwise hijacking the menu key :(
			// See com.android.internal.policy.impl.PhoneWindow.onKeyDownPanel()
			// for the usual Android implementation of this feature.
			//
			// We adopt a similar behavior for the Back system button, as well.
			if (keyCode == KeyEvent.KEYCODE_MENU || keyCode == KeyEvent.KEYCODE_BACK) {
				//
				// Upon pressing the system menu or system back key:
				// (The example below assumes that system Back key was pressed)
				// 1. keyHandler.hasMessages(MSG_SBACK_LONG_PRESS) = false, and thus: fired = true
				// 2. Action will be KeyEvent.ACTION_DOWN, so a delayed message "MSG_SBACK_LONG_PRESS" will be sent to keyHandler after _longPressTimeout time
				//    The "MSG_SBACK_LONG_PRESS" will be handled (and removed) in the keyHandler.
				//    For the Back button, the keyHandler should forward a ACTION_UP for MENU (the alternate func of Back key!) to native)
				//    But if the code enters this section before the "MSG_SBACK_LONG_PRESS" was handled in keyHandler (probably due to a ACTION_UP)
				//        then fired = false and the message is removed from keyHandler, meaning we should treat the button press as a SHORT key press
				final int typeOfLongPressMessage;
				if (keyCode == KeyEvent.KEYCODE_MENU) {
					typeOfLongPressMessage = MSG_SMENU_LONG_PRESS;
				} else { // back button
					typeOfLongPressMessage = MSG_SBACK_LONG_PRESS;
				}

				final boolean fired = !_handler.hasMessages(typeOfLongPressMessage);

				_handler.removeMessages(typeOfLongPressMessage);

				if (action == KeyEvent.ACTION_DOWN) {
					_handler.sendMessageDelayed(_handler.obtainMessage(typeOfLongPressMessage), _longPressTimeout);
					return true;
				} else if (action != KeyEvent.ACTION_UP) {
					return true;
				}

				if (fired) {
					return true;
				}

				// It's still necessary to send a key down event to the backend.
//				Log.d(ScummVM.LOG_TAG, "JE_SYS_KEY");
				_scummvm.pushEvent(JE_SYS_KEY,
				                   KeyEvent.ACTION_DOWN,
				                   keyCode,
				                   eventUnicodeChar & KeyCharacterMap.COMBINING_ACCENT_MASK,
				                   e.getMetaState(),
				                   e.getRepeatCount(),
				                   (int)(e.getEventTime() - e.getDownTime()));
			}
		}

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
			if (onKeyMultiple(action, keyCode, e)) {
				return true;
			}
		}

		int type;
		switch (keyCode) {
		case KeyEvent.KEYCODE_VOLUME_DOWN:
		case KeyEvent.KEYCODE_VOLUME_UP:
			// We ignore these so that they can be handled by Android.
			return false;

//		case KeyEvent.KEYCODE_CHANNEL_UP:
//		case KeyEvent.KEYCODE_CHANNEL_DOWN:
		case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
		case KeyEvent.KEYCODE_MEDIA_REWIND:
		case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
			type = JE_TV_REMOTE;
			break;

		case KeyEvent.KEYCODE_DPAD_UP:
		case KeyEvent.KEYCODE_DPAD_DOWN:
		case KeyEvent.KEYCODE_DPAD_LEFT:
		case KeyEvent.KEYCODE_DPAD_RIGHT:
		case KeyEvent.KEYCODE_DPAD_CENTER:
			// NOTE 1 For now, we're handling DPAD keys as JE_GAMEPAD events, regardless the source InputDevice
			//        EXCEPT for the case where the event comes from our virtual keyboard (currently applicable for UP/DOWN/LEFT/RIGHT arrows, not CENTER)
			//
			//        We delegate these keypresses to ScummVM's keymapper as JOYSTICK_BUTTON_DPAD presses.
			//        (JOYSTICK_BUTTON_DPAD_UP, JOYSTICK_BUTTON_DPAD_DOWN, JOYSTICK_BUTTON_DPAD_LEFT, JOYSTICK_BUTTON_DPAD_RIGHT and JOYSTICK_BUTTON_DPAD_CENTER)
			//        By default mapped to virtual mouse (VMOUSE).
			//        As virtual mouse, cursor may be too fast/hard to control, so it's recommended to set and use a VMOUSESLOW binding too,
			//         (Simultaneous button pressing may work on physical TV remote controls, but may not work on apps for remote controls)
			//        or adjust the Pointer Speed setting from the "Control" tab.
			// NOTE 2 Modern gamepads/ game controllers treat the "DPAD" cross buttons as HATs that produce movement events
			// and *not* DPAD_UP/DOWN/LEFT/RIGHT button press events. Hence, for those controllers these DPAD key events won't be triggered.
			// Those are handled within onGenericMotionEvent().
			//
			if ((e.getFlags() & KeyEvent.FLAG_SOFT_KEYBOARD) == KeyEvent.FLAG_SOFT_KEYBOARD) {
				type = JE_KEY;
			} else {
				type = JE_GAMEPAD;
			}
			break;

		case KeyEvent.KEYCODE_BUTTON_A:
		case KeyEvent.KEYCODE_BUTTON_B:
		case KeyEvent.KEYCODE_BUTTON_C:
		case KeyEvent.KEYCODE_BUTTON_X:
		case KeyEvent.KEYCODE_BUTTON_Y:
		case KeyEvent.KEYCODE_BUTTON_Z:
		case KeyEvent.KEYCODE_BUTTON_L1:
		case KeyEvent.KEYCODE_BUTTON_R1:
		case KeyEvent.KEYCODE_BUTTON_L2:
		case KeyEvent.KEYCODE_BUTTON_R2:
		case KeyEvent.KEYCODE_BUTTON_THUMBL:
		case KeyEvent.KEYCODE_BUTTON_THUMBR:
		case KeyEvent.KEYCODE_BUTTON_START:
		case KeyEvent.KEYCODE_BUTTON_SELECT:
		case KeyEvent.KEYCODE_BUTTON_MODE:
			type = JE_GAMEPAD;
			break;

		case KeyEvent.KEYCODE_BUTTON_1:
		case KeyEvent.KEYCODE_BUTTON_2:
		case KeyEvent.KEYCODE_BUTTON_3:
		case KeyEvent.KEYCODE_BUTTON_4:
			// These are oddly detected with SOURCE_KEYBOARD for joystick so don't bother checking the e.getSource()
			// Tested on a Saitek ST200 USB Control Stick & Throttle
			type = JE_JOYSTICK;
			break;

		default:
			if (e.isSystem()) {
				type = JE_SYS_KEY;
			} else {
				type = JE_KEY;
			}
			break;
		}

		//_scummvm.displayMessageOnOSD("GetKey: " + keyCode + " unic=" + eventUnicodeChar+ " arg3= " + (eventUnicodeChar& KeyCharacterMap.COMBINING_ACCENT_MASK) + " meta: " + e.getMetaState());
		//_scummvm.displayMessageOnOSD("GetKey: " + keyCode + " type=" + type + " source=" + e.getSource() + " action= " + action + " arg5= " + e.getRepeatCount());
		//Log.d(ScummVM.LOG_TAG,"GetKey: " + keyCode + " unic=" + eventUnicodeChar+ " arg3= " + (eventUnicodeChar& KeyCharacterMap.COMBINING_ACCENT_MASK) + " meta: " + e.getMetaState());
		//Log.d(ScummVM.LOG_TAG,"GetKey: " + keyCode + " type=" + type + " source=" + e.getSource() + " flags=" + e.getFlags() + " action= " + action + " arg5= " + e.getRepeatCount());

		// look in events.cpp for how this is handled
		_scummvm.pushEvent(type,
		                   action,
		                   keyCode,
		                   eventUnicodeChar & KeyCharacterMap.COMBINING_ACCENT_MASK,
		                   e.getMetaState(),
		                   e.getRepeatCount(),
		                   (int)(e.getEventTime() - e.getDownTime()));

		return true;
	}

	/**
	 * This gets called only on Android < Q
	 */
	@SuppressWarnings("deprecation")
	private boolean onKeyMultiple(int action, int keyCode, KeyEvent e) {
		// The KeyEvent.ACTION_MULTIPLE constant was deprecated in API level 29 (Q).
		// No longer used by the input system.
		// getAction() value: multiple duplicate key events have occurred in a row, or a complex string is being delivered.
		//    If the key code is not KEYCODE_UNKNOWN then the getRepeatCount() method returns the number of times the given key code should be executed.
		//    Otherwise, if the key code is KEYCODE_UNKNOWN, then this is a sequence of characters as returned by getCharacters().
		//    sequence of characters
		// getCharacters() is also deprecated in API level 29
		//    For the special case of a ACTION_MULTIPLE event with key code of KEYCODE_UNKNOWN,
		//    this is a raw string of characters associated with the event. In all other cases it is null.
		// TODO What is the use case for this?
		//  Does it make sense to keep it with a Build.VERSION.SDK_INT < Build.VERSION_CODES.Q check?
		if (action == KeyEvent.ACTION_MULTIPLE
			&& keyCode == KeyEvent.KEYCODE_UNKNOWN) {
			final KeyCharacterMap m = KeyCharacterMap.load(e.getDeviceId());
			final KeyEvent[] es = m.getEvents(e.getCharacters().toCharArray());

			if (es == null) {
				return true;
			}

			for (KeyEvent s : es) {
				_scummvm.pushEvent(JE_KEY,
					s.getAction(),
					s.getKeyCode(),
					s.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK,
					s.getMetaState(),
					s.getRepeatCount(),
					0);
			}
			return true;
		}

		return false;
	}

	/** Aux method to provide a description for a MotionEvent action
	 *  Given an action int, returns a string description
	 *  Use for debug purposes
	 * @param action the id of the action (as returned by getAction()
	 * @return the action description
	 */
	public static String motionEventActionToString(int action) {
		switch (action) {

			case MotionEvent.ACTION_DOWN: return "Down";
			case MotionEvent.ACTION_MOVE: return "Move";
			case MotionEvent.ACTION_POINTER_DOWN: return "Pointer Down";
			case MotionEvent.ACTION_UP: return "Up";
			case MotionEvent.ACTION_POINTER_UP: return "Pointer Up";
			case MotionEvent.ACTION_OUTSIDE: return "Outside";
			case MotionEvent.ACTION_CANCEL: return "Cancel";
//			case MotionEvent.ACTION_POINTER_2_DOWN: return "Pointer 2 Down"; // 261 - deprecated (but still fired for Android 9, Mi device)
//			case MotionEvent.ACTION_POINTER_2_UP: return "Pointer 2 Up"; // 262 - deprecated (but still fired for Android 9, Mi device)
//			case MotionEvent.ACTION_POINTER_3_DOWN: return "Pointer 3 Down"; // 517 - deprecated (but still fired for Android 9, Mi device)
//			case MotionEvent.ACTION_POINTER_3_UP: return "Pointer 3 Up"; // 518 - deprecated (but still fired for Android 9, Mi device)
			default:
				if ((action &  MotionEvent.ACTION_POINTER_DOWN) == MotionEvent.ACTION_POINTER_DOWN) {
					return "Pointer Down ***";
				} else if ((action &  MotionEvent.ACTION_POINTER_UP) == MotionEvent.ACTION_POINTER_UP) {
					return "Pointer Up ***";
				}
				return "Unknown:: " + action;
		}
	}

	// OnTouchListener
	@Override
	final public boolean onTouch(View v, final MotionEvent event) {

		// Note: In this article https://developer.android.com/training/gestures/multi
		//       it is recommended to use MotionEventCompat helper methods, instead of directly using MotionEvent getAction() etc.
		//       However, getActionMasked() and MotionEventCompat *are deprecated*, and now direct use of MotionEvent methods is recommended.
		//       https://developer.android.com/reference/androidx/core/view/MotionEventCompat

		// Note 2: Do not return intentionally false for the onTouch function because then getPointerCount() won't work as intended
		//         ie. it will always return 1,
		//         as noted here:
		//         https://stackoverflow.com/a/11709964

		_currentView = v;
		final int action = event.getAction();

		// Get the index of the pointer associated with the action.
//		int index = event.getActionIndex();
//		int xPos = (int)event.getX(index);
//		int yPos = (int)event.getY(index);

//		String prefixDBGMsg = "SPECIAL DBG action is " + motionEventActionToString(action) + " ";
//		if (event.getPointerCount() > 1) {
//			// The coordinates of the current screen contact, relative to
//			// the responding View or Activity.
//			Log.d(ScummVM.LOG_TAG,prefixDBGMsg + "Multitouch event (" + event.getPointerCount() + "):: x:" + xPos + " y: " + yPos);
//		} else {
//			// Single touch event
//			Log.d(ScummVM.LOG_TAG,prefixDBGMsg + "Single touch event:: x: " + xPos + " y: " + yPos);
//		}

		if (ScummVMActivity.keyboardWithoutTextInputShown
		    && _activity.isScreenKeyboardShown()
		    && _activity.getScreenKeyboard().getY() <= event.getY() ) {
			event.offsetLocation(-_activity.getScreenKeyboard().getX(), -_activity.getScreenKeyboard().getY());
			// TODO maybe call the onTouchEvent of something else here?
			_activity.getScreenKeyboard().onTouchEvent(event);
			// correct the offset for continuing handling the event
			event.offsetLocation(_activity.getScreenKeyboard().getX(), _activity.getScreenKeyboard().getY());
		}

		if (_mouseHelper != null) {
			boolean isMouse = MouseHelper.isMouse(event);
			if (isMouse) {
				// mouse button is pressed
				return _mouseHelper.onMouseEvent(event, false);
			}
		}

		if (_touchMode == TOUCH_MODE_GAMEPAD) {
			switch (event.getActionMasked()) {
				case MotionEvent.ACTION_DOWN:
				case MotionEvent.ACTION_POINTER_DOWN: {
					int idx = event.getActionIndex();
					_scummvm.updateTouch(JACTION_DOWN, event.getPointerId(idx), (int)event.getX(idx), (int)event.getY(idx));
					// fall through
				}
				case MotionEvent.ACTION_MOVE:
					for(int idx = 0; idx < event.getPointerCount(); idx++) {
						_scummvm.updateTouch(JACTION_MOVE, event.getPointerId(idx), (int)event.getX(idx), (int)event.getY(idx));
					}
					break;
				case MotionEvent.ACTION_UP:
				case MotionEvent.ACTION_POINTER_UP: {
					int idx = event.getActionIndex();
					_scummvm.updateTouch(JACTION_UP, event.getPointerId(idx), (int)event.getX(idx), (int)event.getY(idx));
					break;
				}
				case MotionEvent.ACTION_CANCEL:
					_scummvm.updateTouch(JACTION_CANCEL, 0, 0, 0);
					break;
			}

			//return _gd.onTouchEvent(event);
			return true;
		} else {
			// Deal with LINT warning "ScummVMEvents#onTouch should call View#performClick when a click is detected"
			switch (action) {
				case MotionEvent.ACTION_UP:
					_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
					v.performClick();
					break;
				case MotionEvent.ACTION_DOWN:
					// fall through
				default:
					break;
			}

			// check if the event can be handled as a multitouch event
			if (_multitouchHelper.handleMotionEvent(event)) {
				_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
				return true;
			}

			return _gd.onTouchEvent(event);
		}
	}

	// OnGestureListener
	@Override
	final public boolean onDown(MotionEvent e) {
//		Log.d(ScummVM.LOG_TAG, "SCUMMV-EVENTS-BASE - onDOWN MotionEvent");
		if (_touchMode != TOUCH_MODE_GAMEPAD) {
			_scummvm.pushEvent(JE_DOWN, (int)e.getX(), (int)e.getY(), 0, 0, 0, 0);
		}
		return true;
	}

	@Override
	final public boolean onFling(MotionEvent e1, MotionEvent e2,
									float velocityX, float velocityY) {
		//Log.d(ScummVM.LOG_TAG, String.format(Locale.ROOT, "onFling: %s -> %s (%.3f %.3f)",
		//										e1.toString(), e2.toString(),
		//										velocityX, velocityY));

//		Log.d(ScummVM.LOG_TAG, "onFling");
		_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
		return true;
	}

	@Override
	final public void onLongPress(MotionEvent e) {
		// disabled, interferes with drag&drop
	}

	@Override
	final public boolean onScroll(MotionEvent e1, MotionEvent e2,
									float distanceX, float distanceY) {
		_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
//		Log.d(ScummVM.LOG_TAG, "onScroll");
		if (_touchMode != TOUCH_MODE_GAMEPAD && e1 != null) {
			// typical use:
			// - move mouse cursor around (most traditional point and click games)
			// - mouse look (eg. Myst 3)
			_scummvm.pushEvent(JE_SCROLL, (int)e1.getX(), (int)e1.getY(),
							(int)e2.getX(), (int)e2.getY(), (int)distanceX, (int)distanceY);
		}
		return true;
	}

	@Override
	final public void onShowPress(MotionEvent e) {
//		Log.d(ScummVM.LOG_TAG, "onShowPress");
		_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
		if (_touchMode != TOUCH_MODE_GAMEPAD && !_doubleTapMode) {
			// Schedule a Right click notification
			_handler.sendMessageAtTime(_handler.obtainMessage(MSG_LONG_TOUCH_EVENT, 0, 0), e.getDownTime() + 500);
			// Middle click
			_handler.sendMessageAtTime(_handler.obtainMessage(MSG_LONG_TOUCH_EVENT, 1, 0), e.getDownTime() + 1500);
		}
	}

	@Override
	final public boolean onSingleTapUp(MotionEvent e) {
//		Log.d(ScummVM.LOG_TAG, "onSingleTapUp");
		_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
		if (_touchMode != TOUCH_MODE_GAMEPAD) {
			_scummvm.pushEvent(JE_TAP, (int)e.getX(), (int)e.getY(),
							(int)(e.getEventTime() - e.getDownTime()), 0, 0, 0);
		}
		return true;
	}

	// OnDoubleTapListener
	@Override
	final public boolean onDoubleTap(MotionEvent e) {
//		Log.d(ScummVM.LOG_TAG, "onDoubleTap");
		_doubleTapMode = true;
		_handler.removeMessages(MSG_LONG_TOUCH_EVENT);
		return true;
	}

	@Override
	final public boolean onDoubleTapEvent(MotionEvent e) {
		switch (e.getAction()) {
			case MotionEvent.ACTION_MOVE:
				//if the second tap hadn't been released and it's being moved
//				Log.d(ScummVM.LOG_TAG, "onDoubleTapEvent Moving X: " + Float.toString(e.getRawX()) + " Y: " + Float.toString(e.getRawY()));
				break;

			case MotionEvent.ACTION_UP:
//				Log.d(ScummVM.LOG_TAG, "onDoubleTapEvent Release!");
				//user released the screen
				_doubleTapMode = false;
				break;

			case MotionEvent.ACTION_DOWN:
//				Log.d(ScummVM.LOG_TAG, "onDoubleTapEvent DOWN!");
				break;

			default:
//				Log.d(ScummVM.LOG_TAG, "onDoubleTapEvent UNKNOWN!");
				break;
		}

		if (_touchMode != TOUCH_MODE_GAMEPAD) {
			_scummvm.pushEvent(JE_DOUBLE_TAP, (int)e.getX(), (int)e.getY(), e.getAction(), 0, 0, 0);
		}
		return true;
	}

	@Override
	final public boolean onSingleTapConfirmed(MotionEvent e) {
		// Note, timing thresholds for double tap detection seem to be hardcoded in the framework
		// as ViewConfiguration.getDoubleTapTimeout()
//		Log.d(ScummVM.LOG_TAG, "onSingleTapConfirmed - double tap failed");
		return true;
	}

	private static float getJoystickCenteredAxis(MotionEvent event, InputDevice device, int axisId, int historyPos) {
		final InputDevice.MotionRange range = device.getMotionRange(axisId, event.getSource());
		final int actionPointerIndex = event.getActionIndex();

		// A joystick at rest does not always report an absolute position of
		// (0,0). Use the getFlat() method to determine the range of values
		// bounding the joystick axis center.
		if (range != null) {
			final float axisFlat = range.getFlat();
			final float axisVal = (historyPos < 0) ? event.getAxisValue(range.getAxis(), actionPointerIndex) : event.getHistoricalAxisValue(range.getAxis(), actionPointerIndex, historyPos);

			// Ignore axis values that are within the 'flat' region of the
			// joystick axis center.
			if (Math.abs(axisVal) > axisFlat) {
//				Log.d(ScummVM.LOG_TAG, "JOYSTICK axis: " + MotionEvent.axisToString(axisId) + " id: " + axisId + " - Math.abs(" + value + ") > " + axisFlat + " (flat) - raw val=" + axisVal);
				// This value is already normalized in [-1.0, 1.0] (for sticks and "hats") or [0.0, 1.0] (for triggers)
				return axisVal;
			}
//			else {
//				Log.d(ScummVM.LOG_TAG, "JOYSTICK axis: " + MotionEvent.axisToString(axisId) + " id: " + axisId + " - Math.abs(" + value + ") <= " + axisFlat  + "(flat) - raw val=" + axisVal);
//			}
		}
//		else {
//			Log.d(ScummVM.LOG_TAG, "JOYSTICK axis: " + MotionEvent.axisToString(axisId) + " id: " + axisId + "- getCenteredAxis() range was null!");
//		}
		return 0;
	}

	private boolean repeatMove(final int axisBitFlags, boolean useArgumentAxis) {
		// The argument axisFlags has flags set for the axis that have movement.
		// Send pushEvents per axis (with bitflag set) as opposed to using a version of pushEvent with many arguments.
		int axisBfs = axisBitFlags;
		if (!useArgumentAxis) {
			axisBfs = _repeatingJoystickAxisIdBitFlags;
		}
		for (int i = 0; i < _repeatingJoystickCenteredAxisValuesArray.length; ++i) {
			if ((axisBfs & (0x01 <<  i)) != 0) {
				_scummvm.pushEvent(JE_JOYSTICK, MotionEvent.ACTION_MOVE,
				(int) (_repeatingJoystickCenteredAxisValuesArray[i] * JOYSTICK_AXIS_MAX),
				0,
				axisBfs & (0x01 <<  i), 0, 0);
			}
		}
		return true;
	}

	private void processJoystickInput(MotionEvent event, int historyPos) {
		InputDevice inputDevice = event.getDevice();

//		// DEBUG LOGGING CODE
//		final int actionPointerIndex = event.getActionIndex();
//
//		final int[] allAxisIdArray = {MotionEvent.AXIS_X, MotionEvent.AXIS_Y, MotionEvent.AXIS_Z,
//		                        MotionEvent.AXIS_RX, MotionEvent.AXIS_RY, MotionEvent.AXIS_RZ,
//		                        MotionEvent.AXIS_RTRIGGER, MotionEvent.AXIS_LTRIGGER,
//		                        MotionEvent.AXIS_HAT_X, MotionEvent.AXIS_HAT_Y,
//		                        MotionEvent.AXIS_WHEEL, MotionEvent.AXIS_VSCROLL, MotionEvent.AXIS_HSCROLL, MotionEvent.AXIS_SCROLL,
//		                        MotionEvent.AXIS_RELATIVE_X, MotionEvent.AXIS_RELATIVE_Y, MotionEvent.AXIS_DISTANCE,
//		                        MotionEvent.AXIS_THROTTLE, MotionEvent.AXIS_TILT, MotionEvent.AXIS_GAS, MotionEvent.AXIS_BRAKE, MotionEvent.AXIS_RUDDER,
//		                        MotionEvent.AXIS_GENERIC_1, MotionEvent.AXIS_GENERIC_2, MotionEvent.AXIS_GENERIC_3, MotionEvent.AXIS_GENERIC_4,
//		                        MotionEvent.AXIS_GENERIC_5, MotionEvent.AXIS_GENERIC_6, MotionEvent.AXIS_GENERIC_7, MotionEvent.AXIS_GENERIC_8,
//		                        MotionEvent.AXIS_GENERIC_9, MotionEvent.AXIS_GENERIC_10, MotionEvent.AXIS_GENERIC_11, MotionEvent.AXIS_GENERIC_12,
//		                        MotionEvent.AXIS_GENERIC_13, MotionEvent.AXIS_GENERIC_14, MotionEvent.AXIS_GENERIC_15, MotionEvent.AXIS_GENERIC_16};
//
//		// These correspond to our bitflags order for movement/repetition
//		final String[] axisBitFlagIndexToString = {"LEFT_JOY_X", "LEFT_JOY_Y", "HAT_X", "HAT_Y", "RIGHT_JOY_X", "RIGHT_JOY_Y", "LEFT_TRIGGER", "RIGHT_TRIGGER"};
//
//		for (int i = 0; i < allAxisIdArray.length; ++i) {
//			float axisVal = event.getAxisValue(allAxisIdArray[i], actionPointerIndex);
//			if (Math.abs(axisVal) > 0.0f) {
//				Log.d(ScummVM.LOG_TAG, "JOYSTICK MOTION ON AXIS: " + MotionEvent.axisToString(allAxisIdArray[i]) + " id: " + allAxisIdArray[i] + " for (raw): " + axisVal);
//			}
//		}
//		// END OF DEBUG LOGGING CODE

		// Scaling the HAT movement to 1/3 causes it to be filtered out by the backend (see backends/keymapper/hardware-input.cpp, method: findHardwareInput() )
		// ie. "Ignore incomplete presses for remapping purposes" (lower than 1/2 of Common::JOYAXIS_MAX which is defined as 32767 in common/events.h)
		// Hence we scale to 2/3 since hat axis is non-analog, and 100% of axis max is way too fast when used for cursor movement

		// Calculate the distance(s) to move by - for each supported AXIS
		// ie. the left control stick, hat switch, the right control stick, or the R/L triggers
		// NOTE The order of entries in the moveDistanceArray array is important. It corresponds to our auxiliary movement axis bitflags values order
		float[] centeredAxisValuesArray = {getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_X, historyPos),
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_Y, historyPos),
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_X, historyPos) * JOYSTICK_AXIS_HAT_SCALE,
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_Y, historyPos) * JOYSTICK_AXIS_HAT_SCALE,
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_Z, historyPos),
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_RZ, historyPos),
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_LTRIGGER, historyPos),
		                                   getJoystickCenteredAxis(event, inputDevice, MotionEvent.AXIS_RTRIGGER, historyPos)};

		float currX    = 0.0f;
		float absCurrX = 0.0f;
		float currY    = 0.0f;
		float absCurrY = 0.0f;
		int stoppingMovementAxisIdBitFlags = 0;
		int prevRepeatingAxisIdBitFlags = _repeatingJoystickAxisIdBitFlags;

		for (int i = 0; i < centeredAxisValuesArray.length; ++i) {
			currX = centeredAxisValuesArray[i];
			absCurrX = Math.abs(currX);

			// 0 - 5 (ie. 4+1) we deal with x-axis, y-axis together for LEFT STICK, HAT (DPAD), RIGHT STICK.
			if (i < 5) {
				currY = centeredAxisValuesArray[i+1];
				absCurrY = Math.abs(currY);
			}
			// TODO Make this limit dependant on the ConfMan joystick_deadzone setting -- but avoid using frequent JNI to get the value
			//      The virtual mouse already uses joystick_deadzone to handle input, along with the "kbdmouse_speed". (see backends/keymapper/virtual-mouse.cpp)
			//      PSP also uses joystick_deadzone for its input so maybe we could do something similar. (see backends/platform/psp/input.cpp).
			//      If this filtering goes to the native side (eg. in backends/platform/android/events.cpp) we'll still need to somehow update the repeating-axis bitflag.
			if (absCurrX < 0.209f
			    && (i >= 6 || absCurrY < 0.209f)) {
				// When on all the axis for the current control (or the only axis) we have negligible movement that could still be greater than "flat" range,
				// we do extra filter to stop repetition in order to avoid cases when Android does not send onGenericMotionEvent()
				// for small x or y (while abs is still greater than range.getflat())!
				// In such case we would end up with a slow moving "mouse" cursor - so we need this extra filter.

				// If previously we had movement on at least one of the axis for the current control (or the only axis),
				// then stop movement, reset values to 0 and clear pertinent repeating axis bitflags.
				if ((prevRepeatingAxisIdBitFlags & (0x01 <<  i)) != 0
				    || (i < 5 && (prevRepeatingAxisIdBitFlags & (0x01 << (i+1))) != 0)) {
//					if (i < 5) {
//						Log.d(ScummVM.LOG_TAG, "JOYSTICK " + axisBitFlagIndexToString[i] + ", " + axisBitFlagIndexToString[i+1] + "- pushEvent(): STOPPED: x=" + (int)(currX * 100) + " y=" + (int)(currY * 100));
//					} else {
//						Log.d(ScummVM.LOG_TAG, "JOYSTICK " + axisBitFlagIndexToString[i] + "- pushEvent(): STOPPED: x=" + (int)(currX * 100));
//					}
					// do the move, then signal the joystick has returned to center pos
					stoppingMovementAxisIdBitFlags = 0;
					_repeatingJoystickCenteredAxisValuesArray[i] = currX;
					stoppingMovementAxisIdBitFlags |= (0x01 << i);
					if (i < 5) {
						_repeatingJoystickCenteredAxisValuesArray[i+1] = currY;
						stoppingMovementAxisIdBitFlags |= (0x01 << (i+1));
					}
					repeatMove(stoppingMovementAxisIdBitFlags, true);

					_repeatingJoystickCenteredAxisValuesArray[i] = 0.0f;
					_repeatingJoystickAxisIdBitFlags &= ~(0x01 << i);
					if (i < 5) {
						_repeatingJoystickCenteredAxisValuesArray[i+1] = 0.0f;
						_repeatingJoystickAxisIdBitFlags &= ~(0x01 << (i+1));
					}
					// This return-to-zero (center pos) is sent as an explicit extra event, so it's considered "movement" on the axis
					repeatMove(stoppingMovementAxisIdBitFlags, true);
				}
			} else {
				// Here we have significant movement on at least one of the axis for the current control (or the only axis).
//				if (i < 5) {
//					Log.d(ScummVM.LOG_TAG, "JOYSTICK " + axisBitFlagIndexToString[i] + ", " + axisBitFlagIndexToString[i+1] + "- update movement: x= " + (int)(currX * 100) + " y= " + (int)(currY * 100));
//				} else {
//					Log.d(ScummVM.LOG_TAG, "JOYSTICK " + axisBitFlagIndexToString[i] + "- update movement: x= " + (int)(currX * 100));
//				}
				// We update the axis values (for controls like sticks or hats we update both pertinent axis values here)
				// and set the respective repetition bit flag(s).
				_repeatingJoystickCenteredAxisValuesArray[i] = currX; // X AXIS
				_repeatingJoystickAxisIdBitFlags |= (0x01 <<  i);
				if (i < 5) {
					_repeatingJoystickCenteredAxisValuesArray[i+1] = currY; // Y AXIS
					_repeatingJoystickAxisIdBitFlags |= (0x01 <<  (i+1));
				}
			}
			if (i < 5) ++i; // skip next index as we handled it already in this case
		}

		if (_repeatingJoystickAxisIdBitFlags == 0) {
			// Only removeMessages() if all motions are supposed to stop, ie final moving stick/hat/trigger etc???
			_handler.removeMessages(MSG_REPEAT);
		} else if (prevRepeatingAxisIdBitFlags == 0) {
			// Start repetition message sending (sendMessageDelayed)
			Message msg = _handler.obtainMessage(MSG_REPEAT);
			_handler.sendMessageDelayed(msg, REPEAT_START_DELAY);
			repeatMove(_repeatingJoystickAxisIdBitFlags, true);
		}
	}

}
