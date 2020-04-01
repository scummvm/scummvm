package org.scummvm.scummvm;

import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;

/**
 * Contains helper methods for mouse/hover events that were introduced in Android 4.0.
 */
public class MouseHelper {
	private View.OnHoverListener _listener;
	private ScummVM _scummvm;
	private boolean _rmbPressed;
	private boolean _lmbPressed;
	private boolean _mmbPressed;
	private boolean _bmbPressed;
	private boolean _fmbPressed;
	private boolean _srmbPressed;
	private boolean _smmbPressed;

	/**
	 * Class initialization fails when this throws an exception.
	 * Checking hover availability is done on static class initialization for Android 1.6 compatibility.
	 */
	static {
		try {
			Class.forName("android.view.View$OnHoverListener");
		} catch (Exception ex) {
			throw new RuntimeException(ex);
		}
	}

	/**
	 * Calling this forces class initialization
	 */
	public static void checkHoverAvailable() {}

	public MouseHelper(ScummVM scummvm) {
		_scummvm = scummvm;
		_listener = createListener();
	}

	private View.OnHoverListener createListener() {
		return new View.OnHoverListener() {
			@Override
			public boolean onHover(View view, MotionEvent e) {
				return onMouseEvent(e, true);
			}
		};
	}

	public void attach(SurfaceView main_surface) {
		main_surface.setOnHoverListener(_listener);
	}

	public static boolean isMouse(KeyEvent e) {
		int source = e.getSource();

		return ((source & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE) ||
		       ((source & InputDevice.SOURCE_STYLUS) == InputDevice.SOURCE_STYLUS) ||
		       ((source & InputDevice.SOURCE_TOUCHPAD) == InputDevice.SOURCE_TOUCHPAD);
	}

	public static boolean isMouse(MotionEvent e) {
		if (e == null) {
			return false;
		}

		InputDevice device = e.getDevice();

		if (device == null) {
			return false;
		}

		int sources = device.getSources();

		return ((sources & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE) ||
		       ((sources & InputDevice.SOURCE_STYLUS) == InputDevice.SOURCE_STYLUS) ||
		       ((sources & InputDevice.SOURCE_TOUCHPAD) == InputDevice.SOURCE_TOUCHPAD);
	}

	private boolean handleButton(MotionEvent e, boolean mbPressed, int mask, int downEvent, int upEvent) {
		boolean mbDown = (e.getButtonState() & mask) == mask;
		if (mbDown) {
			if (!mbPressed) {
				// left mouse button was pressed just now
				_scummvm.pushEvent(downEvent, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0, 0);
			}

			return true;
		} else {
			if (mbPressed) {
				// left mouse button was released just now
				_scummvm.pushEvent(upEvent, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0, 0);
			}

			return false;
		}
	}

	public boolean onMouseEvent(MotionEvent e, boolean hover) {
		_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_MOVE, (int)e.getX(), (int)e.getY(), 0, 0, 0, 0);

		int buttonState = e.getButtonState();

		boolean lmbDown = (buttonState & MotionEvent.BUTTON_PRIMARY) == MotionEvent.BUTTON_PRIMARY;

		if (!hover && e.getAction() != MotionEvent.ACTION_UP && buttonState == 0) {
			// On some device types, ButtonState is 0 even when tapping on the touchpad or using the stylus on the screen etc.
			lmbDown = true;
		}

		if (lmbDown) {
			if (!_lmbPressed) {
				// left mouse button was pressed just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_DOWN, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0, 0);
			}

			_lmbPressed = true;
		} else {
			if (_lmbPressed) {
				// left mouse button was released just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_UP, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0, 0);
			}

			_lmbPressed = false;
		}

		_rmbPressed = handleButton(e, _rmbPressed, MotionEvent.BUTTON_SECONDARY, ScummVMEvents.JE_RMB_DOWN, ScummVMEvents.JE_RMB_UP);
		_mmbPressed = handleButton(e, _mmbPressed, MotionEvent.BUTTON_TERTIARY, ScummVMEvents.JE_MMB_DOWN, ScummVMEvents.JE_MMB_UP);
		_bmbPressed = handleButton(e, _bmbPressed, MotionEvent.BUTTON_BACK, ScummVMEvents.JE_BMB_DOWN, ScummVMEvents.JE_BMB_UP);
		_fmbPressed = handleButton(e, _fmbPressed, MotionEvent.BUTTON_FORWARD, ScummVMEvents.JE_FMB_DOWN, ScummVMEvents.JE_FMB_UP);
		_srmbPressed = handleButton(e, _srmbPressed, MotionEvent.BUTTON_STYLUS_PRIMARY, ScummVMEvents.JE_RMB_DOWN, ScummVMEvents.JE_RMB_UP);
		_smmbPressed = handleButton(e, _smmbPressed, MotionEvent.BUTTON_STYLUS_SECONDARY, ScummVMEvents.JE_MMB_DOWN, ScummVMEvents.JE_MMB_UP);

		return true;
	}
}
