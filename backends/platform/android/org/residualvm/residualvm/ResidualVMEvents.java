package org.residualvm.residualvm;

import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.content.Context;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.GestureDetector;
import android.view.inputmethod.InputMethodManager;

import tv.ouya.console.api.OuyaController;

public class ResidualVMEvents implements
		android.view.View.OnKeyListener,
		android.view.View.OnGenericMotionListener,
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
	public static final int JE_TOUCH = 9;
	public static final int JE_LONG = 10;
	public static final int JE_FLING = 11;
	public static final int JE_QUIT = 0x1000;
	
	private final int REL_SWIPE_MIN_DISTANCE;
	private final int REL_SWIPE_THRESHOLD_VELOCITY;

	final protected Context _context;
	final protected ResidualVM _residualvm;
	final protected GestureDetector _gd;
	final protected int _longPress;
	final protected int _width;

	public ResidualVMEvents(Context context, ResidualVM residualvm) {
		_context = context;
		_residualvm = residualvm;

		_gd = new GestureDetector(context, this);
		_gd.setOnDoubleTapListener(this);
		//_gd.setIsLongpressEnabled(false);

		_longPress = ViewConfiguration.getLongPressTimeout();
		
		DisplayMetrics dm = context.getResources().getDisplayMetrics();
		REL_SWIPE_MIN_DISTANCE       = (int)(120 * dm.densityDpi / 160.0f);
		REL_SWIPE_THRESHOLD_VELOCITY = (int)(100 * dm.densityDpi / 160.0f);
		_width = dm.widthPixels;
	}

	final public void sendQuitEvent() {
		_residualvm.pushEvent(JE_QUIT, 0, 0, 0, 0, 0, 0);
	}

	public boolean onTrackballEvent(MotionEvent e) {
		_residualvm.pushEvent(JE_BALL, e.getAction(),
							(int)(e.getX() * e.getXPrecision() * 100),
							(int)(e.getY() * e.getYPrecision() * 100),
							0, 0, 0);
		return true;
	}

	final static int MSG_MENU_LONG_PRESS = 1;

	final private Handler keyHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			if (msg.what == MSG_MENU_LONG_PRESS) {
				InputMethodManager imm = (InputMethodManager)
					_context.getSystemService(_context.INPUT_METHOD_SERVICE);

				if (imm != null)
					imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
			}
		}
	};


	// OnKeyListener
	final public boolean onKey(View v, int keyCode, KeyEvent e) {
		final int action = e.getAction();

		if (e.isSystem()) {
			// filter what we handle
			switch (keyCode) {
			case KeyEvent.KEYCODE_BACK:
			case KeyEvent.KEYCODE_MENU:
			case KeyEvent.KEYCODE_CAMERA:
			case KeyEvent.KEYCODE_SEARCH:
				break;

			default:
				return false;
			}

			// no repeats for system keys
			if (e.getRepeatCount() > 0)
				return false;

			// Have to reimplement hold-down-menu-brings-up-softkeybd
			// ourselves, since we are otherwise hijacking the menu key :(
			// See com.android.internal.policy.impl.PhoneWindow.onKeyDownPanel()
			// for the usual Android implementation of this feature.
			if (keyCode == KeyEvent.KEYCODE_MENU) {
				final boolean fired =
					!keyHandler.hasMessages(MSG_MENU_LONG_PRESS);

				keyHandler.removeMessages(MSG_MENU_LONG_PRESS);

				if (action == KeyEvent.ACTION_DOWN) {
					keyHandler.sendMessageDelayed(keyHandler.obtainMessage(
									MSG_MENU_LONG_PRESS), _longPress);
					return true;
				}

				if (fired)
					return true;

				// only send up events of the menu button to the native side
				if (action != KeyEvent.ACTION_UP)
					return true;
			}

			_residualvm.pushEvent(JE_SYS_KEY, action, keyCode, 0, 0, 0, 0);

			return true;
		}

		// sequence of characters
		if (action == KeyEvent.ACTION_MULTIPLE &&
				keyCode == KeyEvent.KEYCODE_UNKNOWN) {
			final KeyCharacterMap m = KeyCharacterMap.load(e.getDeviceId());
			final KeyEvent[] es = m.getEvents(e.getCharacters().toCharArray());

			if (es == null)
				return true;

			for (KeyEvent s : es) {
				_residualvm.pushEvent(JE_KEY, s.getAction(), s.getKeyCode(),
					s.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK,
					s.getMetaState(), s.getRepeatCount(), 0);
			}

			return true;
		}

		switch (keyCode) {
		case KeyEvent.KEYCODE_DPAD_UP:
		case KeyEvent.KEYCODE_DPAD_DOWN:
		case KeyEvent.KEYCODE_DPAD_LEFT:
		case KeyEvent.KEYCODE_DPAD_RIGHT:
		case KeyEvent.KEYCODE_DPAD_CENTER:
			_residualvm.pushEvent(JE_DPAD, action, keyCode,
								(int)(e.getEventTime() - e.getDownTime()),
								e.getRepeatCount(), 0, 0);
			return true;
		}

		_residualvm.pushEvent(JE_KEY, action, keyCode,
					e.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK,
					e.getMetaState(), e.getRepeatCount(), 0);

		return true;
	}

	final public boolean onTouchEvent(MotionEvent e) {
		
		_gd.onTouchEvent(e);
		
		final int action = e.getActionMasked();

		// ACTION_MOVE always returns the first pointer as the "active" one.
		if (action == MotionEvent.ACTION_MOVE) {
			for (int idx = 0; idx < e.getPointerCount(); ++idx) {
				final int pointer = e.getPointerId(idx);
				
				final int x = (int)e.getX(idx);
				final int y = (int)e.getY(idx);
				
				_residualvm.pushEvent(JE_TOUCH, pointer, action, x, y, 0, 0);	
			}
		} else {
			final int idx = e.getActionIndex();
			final int pointer = e.getPointerId(idx);
			
			final int x = (int)e.getX(idx);
			final int y = (int)e.getY(idx);
			
			_residualvm.pushEvent(JE_TOUCH, pointer, action, x, y, 0, 0);	
		}
		
		
		return true;
	}

	// OnGestureListener
	final public boolean onDown(MotionEvent e) {
		_residualvm.pushEvent(JE_DOWN, (int)e.getX(), (int)e.getY(), 0, 0, 0, 0);
		return true;
	}

	final public boolean onFling(MotionEvent e1, MotionEvent e2,
									float velocityX, float velocityY) {
		return false;
//		if (e1.getX() < 0.4 * _width
//			|| Math.abs(e1.getX() - e2.getX()) < REL_SWIPE_MIN_DISTANCE
//			|| velocityX < REL_SWIPE_THRESHOLD_VELOCITY
//		    || Math.abs(e1.getY() - e2.getY()) < REL_SWIPE_MIN_DISTANCE
//			|| velocityY < REL_SWIPE_THRESHOLD_VELOCITY)
//		  return false;
//		
//		_residualvm.pushEvent(JE_FLING, (int)e1.getX(), (int)e1.getY(),
//							(int)e2.getX(), (int)e2.getY(), 0, 0);
	}

	final public void onLongPress(MotionEvent e) {
		_residualvm.pushEvent(JE_LONG, (int)e.getX(), (int)e.getY(),
				0, 0, 0, 0);
	}

	final public boolean onScroll(MotionEvent e1, MotionEvent e2,
									float distanceX, float distanceY) {
		_residualvm.pushEvent(JE_SCROLL, (int)e1.getX(), (int)e1.getY(),
							(int)e2.getX(), (int)e2.getY(), 0, 0);
		return true;
	}

	final public void onShowPress(MotionEvent e) {
	}

	final public boolean onSingleTapUp(MotionEvent e) {
		return false;
	}

	// OnDoubleTapListener
	final public boolean onDoubleTap(MotionEvent e) {
		return false;
	}

	final public boolean onDoubleTapEvent(MotionEvent e) {
		_residualvm.pushEvent(JE_DOUBLE_TAP, (int)e.getX(), (int)e.getY(),
							e.getAction(), 0, 0, 0);

		return true;
	}

	final public boolean onSingleTapConfirmed(MotionEvent e) {
		_residualvm.pushEvent(JE_TAP, (int)e.getX(), (int)e.getY(),
				(int)(e.getEventTime() - e.getDownTime()), 0, 0, 0);
		return true;
	}

	private static boolean rightJoystickHeld = false;
	private static int rightJoystickKeyCode = -1;

	public boolean onGenericMotion(View v, final MotionEvent event) {
		if ((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
			float RS_Y = event.getAxisValue(OuyaController.AXIS_RS_Y);
			if (Math.abs(RS_Y) > OuyaController.STICK_DEADZONE && !rightJoystickHeld) {
				rightJoystickHeld = true;
				rightJoystickKeyCode = RS_Y > 0 ? KeyEvent.KEYCODE_PAGE_DOWN : KeyEvent.KEYCODE_PAGE_UP;
				_residualvm.pushEvent(JE_KEY, KeyEvent.ACTION_DOWN, rightJoystickKeyCode, 0, 0, 0, 0);
				return true;
			} else if (Math.abs(RS_Y) <= OuyaController.STICK_DEADZONE && rightJoystickHeld) {
				rightJoystickHeld = false;
				_residualvm.pushEvent(JE_KEY, KeyEvent.ACTION_UP, rightJoystickKeyCode, 0, 0, 0, 0);
				rightJoystickKeyCode = -1;
				return true;
			} else {
				return false;
			}
		}
		return false;
	}
}
