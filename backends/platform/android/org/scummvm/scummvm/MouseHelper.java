/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package org.scummvm.scummvm;

import android.os.Build;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

/**
 * Contains helper methods for mouse/hover events that were introduced in Android 4.0.
 */
public class MouseHelper implements View.OnHoverListener {
	//private final View.OnHoverListener _listener;
	private final ScummVM _scummvm;
	private boolean _rmbPressed;
	private boolean _lmbPressed;
	private boolean _mmbPressed;
	private boolean _bmbPressed;
	private boolean _fmbPressed;
	private boolean _srmbPressed;
	private boolean _smmbPressed;

	//
	// Class initialization fails when this throws an exception.
	// Checking hover availability is done on static class initialization for Android 1.6 compatibility.
	//
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
		//_listener = createListener();
	}

//	private View.OnHoverListener createListener() {
//		return new View.OnHoverListener() {
//			@Override
//			public boolean onHover(View view, MotionEvent e) {
//				Log.d(ScummVM.LOG_TAG, "onHover mouseEvent");
//				return onMouseEvent(e, true);
//			}
//		};
//	}

	@Override
	public boolean onHover(View view, MotionEvent motionEvent) {
		return onMouseEvent(motionEvent, true, false, 0, 0, 0, 0);
	}

//	public void attach(SurfaceView main_surface) {
//		main_surface.setOnHoverListener(_listener);
//	}

	// isTrackball is a subcase of isMouse (meaning isMouse will also return true)
	private static boolean isTrackball(KeyEvent e) {
		if (e == null) {
			return false;
		}

		int source = e.getSource();
		return ((source & InputDevice.SOURCE_TRACKBALL) == InputDevice.SOURCE_TRACKBALL)
		        || (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && ((source & InputDevice.SOURCE_MOUSE_RELATIVE) == InputDevice.SOURCE_MOUSE_RELATIVE));


	}

	// isTrackball is a subcase of isMouse (meaning isMouse will also return true)
	private static boolean isTrackball(MotionEvent e) {
		if (e == null) {
			return false;
		}
		//int source = e.getSource();

		InputDevice device = e.getDevice();

		if (device == null) {
			return false;
		}

		int sources = device.getSources();

		return ((sources & InputDevice.SOURCE_TRACKBALL) == InputDevice.SOURCE_TRACKBALL)
		        || (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && ((sources & InputDevice.SOURCE_MOUSE_RELATIVE) == InputDevice.SOURCE_MOUSE_RELATIVE));

	}

	// "Checking against SOURCE_STYLUS only indicates "an input device is capable of obtaining input
	// from a stylus. To determine whether a given touch event was produced by a stylus, examine
	// the tool type returned by MotionEvent#getToolType(int) for each individual pointer."
	// https://developer.android.com/reference/android/view/InputDevice#SOURCE_STYLUS
	private static boolean isStylus(MotionEvent e){
		if (e == null) {
			return false;
		}

		for(int idx = 0; idx < e.getPointerCount(); idx++) {
			if (e.getToolType(idx) == MotionEvent.TOOL_TYPE_STYLUS)
				return true;
		}

		return false;
	}

	public static boolean isMouse(KeyEvent e) {
		if (e == null) {
			return false;
		}

		int source = e.getSource();

		//Log.d(ScummVM.LOG_TAG, "isMouse keyEvent source: " + source);

		return ((source & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE)
		        || ((source & InputDevice.SOURCE_STYLUS) == InputDevice.SOURCE_STYLUS)
		        || ((source & InputDevice.SOURCE_TOUCHPAD) == InputDevice.SOURCE_TOUCHPAD)
		        ||  isTrackball(e);
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

		// SOURCE_MOUSE_RELATIVE is sent when mouse is detected as trackball
		// TODO: why does this happen? Do we need to also check for SOURCE_TRACKBALL here?
		// TODO: should these all be checks against TOOL_TYPEs instead of SOURCEs?
		return ((sources & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE)
		        || ((sources & InputDevice.SOURCE_TOUCHPAD) == InputDevice.SOURCE_TOUCHPAD)
		        ||  isStylus(e)
		        ||  isTrackball(e);
	}


	private static boolean isMbDown(MotionEvent e, int mask, boolean hover) {
		boolean mbDown = (e.getButtonState() & mask) == mask;

		if (!hover && e.getActionMasked() != MotionEvent.ACTION_UP && e.getButtonState() == 0) {
			// On some device types, ButtonState is 0 even when tapping on the touch-pad or using the stylus on the screen etc.
			mbDown = (mask == MotionEvent.BUTTON_PRIMARY);
		} else if (isMouse(e) && (e.getButtonState() & MotionEvent.BUTTON_BACK) == MotionEvent.BUTTON_BACK) {
			mbDown = (mask == MotionEvent.BUTTON_SECONDARY);
		}
		return mbDown;
	}

	private boolean handleButton(MotionEvent e, boolean hover, boolean mbPressed, int mask, int downEvent, int upEvent, int hoverPosX, int hoverPosY) {
		boolean mbDown = isMbDown(e, mask, hover);

		if (mbDown) {
			if (!mbPressed) {
				// mouse button was pressed just now
				//Log.d(ScummVM.LOG_TAG, "handleButton mbDown, not mbPressed, mask = " + mask);
				_scummvm.pushEvent(downEvent, hoverPosX, hoverPosY, e.getButtonState(), 0, 0, 0);
			}
			return true;
		} else {
			if (mbPressed) {
				//Log.d(ScummVM.LOG_TAG, "handleButton not mbDown, mbPressed, mask = " + mask);
				// mouse button was released just now
				_scummvm.pushEvent(upEvent, hoverPosX, hoverPosY, e.getButtonState(), 0, 0, 0);
			}
			return false;
		}
	}

	public boolean onMouseEvent(MotionEvent e, boolean hover, boolean capturedPointer, int cPosX, int cPosY, int cOffsetX, int cOffsetY) {
		//Log.d(ScummVM.LOG_TAG, "onMouseEvent hover: " + hover + " capturedPointer: " + capturedPointer);
		if (e != null && !capturedPointer) {
			cPosX = (int) e.getX();
			cPosY = (int) e.getY();
		}

		if (!capturedPointer || cOffsetX != 0 || cOffsetY != 0) {
			_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_MOVE,
					   cPosX,
					   cPosY,
					   cOffsetX,
					   cOffsetY,
					   0, 0);
		}

		if (e != null) {
			if (e.getActionMasked() == MotionEvent.ACTION_SCROLL) {
				// The call is coming from ScummVMEvents, from a GenericMotionEvent (scroll wheel movement)
				float vScroll = e.getAxisValue(MotionEvent.AXIS_VSCROLL);
				if (vScroll > 0.0f) {
					//Log.d(ScummVM.LOG_TAG, "onMouseEvent Wheel Up);
					_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_WHEEL_UP,
					                   cPosX,
					                   cPosY,
					                   0,
					                   0, 0, 0);
				} else if (vScroll < 0.0f) {
					//Log.d(ScummVM.LOG_TAG, "onMouseEvent Wheel Down);
					_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_WHEEL_DOWN,
					                   cPosX,
					                   cPosY,
					                   0,
					                   0, 0, 0);
				}
			} else {
				//Log.d(ScummVM.LOG_TAG, "onMouseEvent buttonState = " + buttonState);

				_lmbPressed = handleButton(e, hover, _lmbPressed, MotionEvent.BUTTON_PRIMARY, ScummVMEvents.JE_LMB_DOWN, ScummVMEvents.JE_LMB_UP, cPosX, cPosY);
				_rmbPressed = handleButton(e, hover, _rmbPressed, MotionEvent.BUTTON_SECONDARY, ScummVMEvents.JE_RMB_DOWN, ScummVMEvents.JE_RMB_UP, cPosX, cPosY);
				_mmbPressed = handleButton(e, hover, _mmbPressed, MotionEvent.BUTTON_TERTIARY, ScummVMEvents.JE_MMB_DOWN, ScummVMEvents.JE_MMB_UP, cPosX, cPosY);
				_bmbPressed = handleButton(e, hover, _bmbPressed, MotionEvent.BUTTON_BACK, ScummVMEvents.JE_BMB_DOWN, ScummVMEvents.JE_BMB_UP, cPosX, cPosY);
				_fmbPressed = handleButton(e, hover, _fmbPressed, MotionEvent.BUTTON_FORWARD, ScummVMEvents.JE_FMB_DOWN, ScummVMEvents.JE_FMB_UP, cPosX, cPosY);

				// Lint warning for BUTTON_STYLUS... "
				//  Field requires API level 23 (current min is 16): android.view.MotionEvent#BUTTON_STYLUS_PRIMARY"
				//  Field requires API level 23 (current min is 16): android.view.MotionEvent#BUTTON_STYLUS_SECONDARY"
				// https://stackoverflow.com/a/48588149
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
					_srmbPressed = handleButton(e, hover, _srmbPressed, MotionEvent.BUTTON_STYLUS_PRIMARY, ScummVMEvents.JE_RMB_DOWN, ScummVMEvents.JE_RMB_UP, cPosX, cPosY);
					_smmbPressed = handleButton(e, hover, _smmbPressed, MotionEvent.BUTTON_STYLUS_SECONDARY, ScummVMEvents.JE_MMB_DOWN, ScummVMEvents.JE_MMB_UP, cPosX, cPosY);
				}
			}
		}
		return true;
	}
}
