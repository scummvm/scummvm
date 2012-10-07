package org.scummvm.scummvm;

import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;

/**
 * Contains helper methods for mouse/hover events that were introduced in Android 4.0.
 *
 * Mouse (hover) events seem to be a bit arbitrary, so here's couple of scenarios:
 *
 *
 * 1. Galaxy Note 2 (4.1) + stylus:
 * Tool type: TOOL_TYPE_STYLUS
 *
 * Key: 238 ACTION_DOWN (once)
 * Key: 238 ACTION_UP (once)
 * Hover: ACTION_HOVER_ENTER (once)
 * Hover: ACTION_HOVER_MOVE (multiple, while hovering)
 *
 * touch screen with the pen:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 0 (once)
 * Touch: ACTION_DOWN, ButtonState: 0 (once)
 * Touch: ACTION_MOVE, ButtonState: 0 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER (once)
 *
 * press the stylus button while hovering:
 * Hover: ACTION_HOVER_MOVE, ButtonState: 2 (multiple, while pressing button)
 * Hover: ACTION_HOVER_MOVE, ButtonState: 0 (multiple, after release)
 *
 *
 * 2. Galaxy Note 2 (4.1) + mouse (usb):
 * Tool type: TOOL_TYPE_MOUSE
 *
 * Hover: ACTION_HOVER_ENTER (once)
 * Hover: ACTION_HOVER_MOVE (multiple, while hovering)
 *
 * press left button:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 1 (once)
 * Touch: ACTION_DOWN, ButtonState: 1 (once)
 * Touch: ACTION_MOVE, ButtonState: 1 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER, ButtonState: 0 (once)
 *
 * press right button:
 * Key: KEYCODE_BACK, ACTION_DOWN
 * Hover: ACTION_HOVER_MOVE, ButtonState: 2 (multiple, while pressing button)
 * Hover: ACTION_HOVER_MOVE, ButtonState: 0 (once)
 * Key: KEYCODE_BACK, ACTION_UP
 *
 *
 * 3. Asus eeePad Transformer Prime running CyanogenMod 10 (Android 4.1) + mouse (usb):
 * Tool type: TOOL_TYPE_MOUSE
 *
 * Hover: ACTION_HOVER_ENTER (once)
 * Hover: ACTION_HOVER_MOVE (multiple, while hovering)
 *
 * press left button:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 1 (once)
 * Touch: ACTION_DOWN, ButtonState: 1 (once)
 * Touch: ACTION_MOVE, ButtonState: 1 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER, ButtonState: 0 (once)
 *
 * press right button:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 2 (once)
 * Touch: ACTION_DOWN, ButtonState: 2 (once)
 * Touch: ACTION_MOVE, ButtonState: 2 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER, ButtonState: 0 (once)
 *
 *
 * 4. Asus eeePad Transformer Prime running CyanogenMod 10 (Android 4.1) + touchpad:
 * Tool type: TOOL_TYPE_FINGER
 *
 * Hover: ACTION_HOVER_ENTER (once)
 * Hover: ACTION_HOVER_MOVE (multiple, while hovering)
 *
 * press left button:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 1 (once)
 * Touch: ACTION_DOWN, ButtonState: 1 (once)
 * Touch: ACTION_MOVE, ButtonState: 1 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER, ButtonState: 0 (once)
 *
 * press right button:
 * Hover: ACTION_HOVER_EXIT, ButtonState: 2 (once)
 * Touch: ACTION_DOWN, ButtonState: 2 (once)
 * Touch: ACTION_MOVE, ButtonState: 2 (multiple, while pressing)
 * Touch: ACTION_UP, ButtonState: 0 (once)
 * Hover: ACTION_HOVER_ENTER, ButtonState: 0 (once)
 *
 */
public class MouseHelper {
	private View.OnHoverListener _listener;
	private ScummVM _scummvm;
	private long _rmbGuardTime;
	private boolean _rmbPressed;
	private boolean _lmbPressed;

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
				return onTouch(e, true);
			}
		};
	}

	public void attach(SurfaceView main_surface) {
		main_surface.setOnHoverListener(_listener);
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

	public boolean onTouch(MotionEvent e, boolean hover) {
		_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_MOVE, (int)e.getX(), (int)e.getY(), 0, 0, 0);
		
		int buttonState = e.getButtonState();

		boolean lmbDown = (buttonState & MotionEvent.BUTTON_PRIMARY) == MotionEvent.BUTTON_PRIMARY;

		if (e.getToolType(0) == MotionEvent.TOOL_TYPE_STYLUS) {
			// when using stylus, ButtonState is 0
			lmbDown = !hover;
		}

		if (lmbDown) {
			if (!_lmbPressed) {
				// left mouse button was pressed just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_DOWN, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_lmbPressed = true;
		} else {
			if (_lmbPressed) {
				// left mouse button was released just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_UP, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_lmbPressed = false;
		}

		boolean rmbDown = (buttonState & MotionEvent.BUTTON_SECONDARY) == MotionEvent.BUTTON_SECONDARY;
		if (rmbDown) {
			if (!_rmbPressed) {
				// right mouse button was pressed just now
				_scummvm.pushEvent(ScummVMEvents.JE_RMB_DOWN, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_rmbPressed = true;
		} else {
			if (_rmbPressed) {
				// right mouse button was released just now
				_scummvm.pushEvent(ScummVMEvents.JE_RMB_UP, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
				_rmbGuardTime = System.currentTimeMillis();
			}

			_rmbPressed = false;
		}

		return true;
	}

	/**
	 * Checks whether right mouse button is pressed or was pressed just previously. This is used to prevent sending
	 * extra back key on right mouse click which is the default behaviour in some platforms.
	 *
	 * @return true if right mouse button is (or was in the last 200ms) pressed
	 */
	public boolean getRmbGuard() {
		return _rmbPressed || _rmbGuardTime + 200 > System.currentTimeMillis();
	}
}
