package org.scummvm.scummvm;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.InputDevice;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

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
			mListenerReference = new WeakReference<>(listener);
		}

		@Override
		public synchronized void handleMessage(@NonNull Message msg) {
			ScummVMEventsModern listener = mListenerReference.get();
			if(listener != null) {
				switch (msg.what) {
					case MSG_REPEAT:
						if (listener.repeatMove()) {
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

	private ScummVMEventsModernHandler mHandler = new ScummVMEventsModernHandler(this);
	private float repeatingX = 0.0f;
	private float repeatingY = 0.0f;

	private static float getCenteredAxis(MotionEvent event, InputDevice device, int axis, int historyPos) {
		final InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());
		final int actionPointerIndex = event.getActionIndex();

		// A joystick at rest does not always report an absolute position of
		// (0,0). Use the getFlat() method to determine the range of values
		// bounding the joystick axis center.
		if (range != null) {
			final float flat = range.getFlat();

//			if (axis == MotionEvent.AXIS_X
//				|| axis == MotionEvent.AXIS_HAT_X
//				|| axis == MotionEvent.AXIS_Z) {
//				Log.d(ScummVM.LOG_TAG, "Flat X= " + flat);
//			} else {
//				Log.d(ScummVM.LOG_TAG, "Flat Y= " + flat);
//			}

			float axisVal = (historyPos < 0) ? event.getAxisValue( range.getAxis(), actionPointerIndex) : event.getHistoricalAxisValue( range.getAxis(), actionPointerIndex, historyPos);
			// Normalize
			final float value =  (axisVal - range.getMin() ) / range.getRange() * 2.0f - 1.0f;

			// Ignore axis values that are within the 'flat' region of the
			// joystick axis center.
			if (Math.abs(value) > flat) {
				return value;
			}
		}
		return 0;
	}

	private void removeMessages() {
		if (mHandler != null) {
			mHandler.removeMessages(MSG_REPEAT);
		}
	}

	private boolean repeatMove() {
		_scummvm.pushEvent(JE_JOYSTICK, MotionEvent.ACTION_MOVE,
			(int) (repeatingX * 100),
			(int) (repeatingY * 100),
			0, 0, 0);
		return true;
	}

	private void processJoystickInput(MotionEvent event, int historyPos) {

		InputDevice inputDevice = event.getDevice();

		// Calculate the horizontal distance to move by
		// using the input value from one of these physical controls:
		// the left control stick, hat axis, or the right control stick.
		float x = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_X, historyPos);
		//Log.d(ScummVM.LOG_TAG, "JOYSTICK - LEFT: x= " +x);
		if (x == 0) {
			x = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_X, historyPos);
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - HAT: x= " +x);
		}
		if (x == 0) {
			x = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Z, historyPos);
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - RIGHT: x= " +x);
		}

		// Calculate the vertical distance to move by
		// using the input value from one of these physical controls:
		// the left control stick, hat switch, or the right control stick.
		float y = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Y, historyPos);
		//Log.d(ScummVM.LOG_TAG, "JOYSTICK - LEFT: y= " +y);
		if (y == 0) {
			y = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_Y, historyPos);
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - HAT: y= " +y);
		}
		if (y == 0) {
			y = getCenteredAxis(event, inputDevice, MotionEvent.AXIS_RZ, historyPos);
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - RIGHT: y= " +y);
		}

		// extra filter to stop repetition in order to avoid cases when android does not send onGenericMotionEvent()
		// for small x or y (while abs is still larger than range.getflat())
		// In such case we would end up with a slow moving "mouse" cursor - so we need this extra filter
		if (Math.abs(x * 100) < 20.0f && Math.abs(y * 100) < 20.0f) {
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - pushEvent(): STOPPED: " + (int)(x * 100) + " y= " + (int)(y * 100));
			removeMessages();
			// do the move anyway, just don't repeat
			repeatMove();
			repeatingX = 0.0f;
			repeatingY = 0.0f;
		} else {
			//Log.d(ScummVM.LOG_TAG, "JOYSTICK - pushEvent(): x= " + (int)(x * 100) + " y= " + (int)(y * 100));
			if (repeatingX != 0.0f || repeatingY != 0.0f) {
				// already repeating - just update the movement co-ords
				repeatingX = x;
				repeatingY = y;
			} else {
				// start repeating
				//removeMessages();
				repeatingX = x;
				repeatingY = y;
				Message msg = mHandler.obtainMessage(MSG_REPEAT);
				mHandler.sendMessageDelayed(msg, REPEAT_START_DELAY);
				repeatMove();
			}
		}
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		// Check that the event came from a joystick
		if (((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK
			 || (event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0)) {
			int action = event.getActionMasked();
			if (action == MotionEvent.ACTION_MOVE) {

				// Process all historical movement samples in the batch
				final int historySize = event.getHistorySize();

				// Process the movements starting from the
				// earliest historical position in the batch
				for (int i = 0; i < historySize; i++) {
					// Process the event at historical position i
					//Log.d(ScummVM.LOG_TAG, "JOYSTICK - onGenericMotionEvent(m) hist: ");
					processJoystickInput(event, i);
				}

				// Process the current movement sample in the batch (position -1)
				//Log.d(ScummVM.LOG_TAG, "JOYSTICK - onGenericMotionEvent(m): "  );
				processJoystickInput(event, -1);
				return true;
			}
		}
		// this basically returns false since the super just returns false
		return super.onGenericMotionEvent(event);
	}
}
