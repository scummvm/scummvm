package org.scummvm.scummvm;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
//import android.util.Log;
import android.view.MotionEvent;
import android.view.InputDevice;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;
//import java.util.ArrayList;
//import java.util.Collections;
//import java.util.Comparator;
//import java.util.List;

// A class that extends the basic ScummVMEventsBase, supporting Android APIs > HONEYCOMB_MR1 (API 12)
public class ScummVMEventsModern extends ScummVMEventsBase {

	private static final int MSG_REPEAT = 3;
	private static final int REPEAT_INTERVAL = 20; // ~50 keys per second
	private static final int REPEAT_START_DELAY = 40;

	public ScummVMEventsModern(Context context, ScummVM scummvm, MouseHelper mouseHelper) {
		super(context, scummvm, mouseHelper);
	}

	// Custom handler code (to avoid mem leaks, see warning "This Handler Class Should Be Static Or Leaks Might Occur”) based on:
	// https://stackoverflow.com/a/27826094
	public static class ScummVMEventsModernHandler extends Handler {

		private final WeakReference<ScummVMEventsModern> mListenerReference;

		public ScummVMEventsModernHandler(ScummVMEventsModern listener) {
			super(Looper.getMainLooper());
			mListenerReference = new WeakReference<>(listener);
		}

		@Override
		public synchronized void handleMessage(@NonNull Message msg) {
			ScummVMEventsModern listener = mListenerReference.get();
			if(listener != null) {
				switch (msg.what) {
					case MSG_REPEAT:
						if (listener.repeatMove(0, false)) {
							Message repeat = Message.obtain(this, MSG_REPEAT);
							sendMessageDelayed(repeat, REPEAT_INTERVAL);
						}
						break;
				}
			}
		}

		public void clear() {
			this.removeCallbacksAndMessages(null);
		}
	}

	@Override
	public void clearEventHandler() {
		super.clearEventHandler();
		mHandler.clear();
	}

	private final ScummVMEventsModernHandler mHandler = new ScummVMEventsModernHandler(this);
	private final float[] repeatingCenteredAxisValuesArray = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

	// auxiliary movement axis bitflags
	// Also repeated (and used) in android's events.cpp (JE_JOYSTICK case)
	private static final int AXIS_X_bf        = 0x01; // (0x01 << 0)
	private static final int AXIS_Y_bf        = 0x02; // (0x01 << 1)
	private static final int AXIS_HAT_X_bf    = 0x04; // (0x01 << 2)
	private static final int AXIS_HAT_Y_bf    = 0x08; // (0x01 << 3)
	private static final int AXIS_Z_bf        = 0x10; // (0x01 << 4)
	private static final int AXIS_RZ_bf       = 0x20; // (0x01 << 5)
	private static final int AXIS_LTRIGGER_bf = 0x40; // (0x01 << 6)
	private static final int AXIS_RTRIGGER_bf = 0x80; // (0x01 << 7)

	private int repeatingAxisIdBitFlags = 0x00;

	private static float getCenteredAxis(MotionEvent event, InputDevice device, int axisId, int historyPos) {
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

	private void removeMessages() {
		if (mHandler != null) {
			mHandler.removeMessages(MSG_REPEAT);
		}
	}

	private boolean repeatMove(final int axisBitFlags, boolean useArgumentAxis) {
		// The argument axisFlags has flags set for the axis that have movement.
		// Send pushEvents per axis (with bitflag set) as opposed to using a version of pushEvent with many arguments.
		int axisBfs = axisBitFlags;
		if (!useArgumentAxis) {
			axisBfs = repeatingAxisIdBitFlags;
		}
		for (int i = 0; i < repeatingCenteredAxisValuesArray.length; ++i) {
			if ((axisBfs & (0x01 <<  i)) != 0) {
				_scummvm.pushEvent(JE_JOYSTICK, MotionEvent.ACTION_MOVE,
				(int) (repeatingCenteredAxisValuesArray[i] * JOYSTICK_AXIS_MAX),
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
		float[] centeredAxisValuesArray = {getCenteredAxis(event, inputDevice, MotionEvent.AXIS_X, historyPos),
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Y, historyPos),
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_X, historyPos) * JOYSTICK_AXIS_HAT_SCALE,
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_Y, historyPos) * JOYSTICK_AXIS_HAT_SCALE,
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Z, historyPos),
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_RZ, historyPos),
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_LTRIGGER, historyPos),
		                                   getCenteredAxis(event, inputDevice, MotionEvent.AXIS_RTRIGGER, historyPos)};

		float currX    = 0.0f;
		float absCurrX = 0.0f;
		float currY    = 0.0f;
		float absCurrY = 0.0f;
		int stoppingMovementAxisIdBitFlags = 0;
		int prevRepeatingAxisIdBitFlags = repeatingAxisIdBitFlags;

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
					repeatingCenteredAxisValuesArray[i] = currX;
					stoppingMovementAxisIdBitFlags |= (0x01 << i);
					if (i < 5) {
						repeatingCenteredAxisValuesArray[i+1] = currY;
						stoppingMovementAxisIdBitFlags |= (0x01 << (i+1));
					}
					repeatMove(stoppingMovementAxisIdBitFlags, true);

					repeatingCenteredAxisValuesArray[i] = 0.0f;
					repeatingAxisIdBitFlags &= ~(0x01 << i);
					if (i < 5) {
						repeatingCenteredAxisValuesArray[i+1] = 0.0f;
						repeatingAxisIdBitFlags &= ~(0x01 << (i+1));
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
				repeatingCenteredAxisValuesArray[i] = currX; // X AXIS
				repeatingAxisIdBitFlags |= (0x01 <<  i);
				if (i < 5) {
					repeatingCenteredAxisValuesArray[i+1] = currY; // Y AXIS
					repeatingAxisIdBitFlags |= (0x01 <<  (i+1));
				}
			}
			if (i < 5) ++i; // skip next index as we handled it already in this case
		}

		if (repeatingAxisIdBitFlags == 0) {
			// Only removeMessages() if all motions are supposed to stop, ie final moving stick/hat/trigger etc???
			removeMessages();
		} else if (prevRepeatingAxisIdBitFlags == 0) {
			// Start repetition message sending (sendMessageDelayed)
			Message msg = mHandler.obtainMessage(MSG_REPEAT);
			mHandler.sendMessageDelayed(msg, REPEAT_START_DELAY);
			repeatMove(repeatingAxisIdBitFlags, true);
		}
	}

	@Override
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
			// in MouseHelper's onMouseEvent() called from ScummVMEventsBase onTouch().
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
		// this basically returns false since the super just returns false
		return super.onGenericMotionEvent(event);
	}
}
