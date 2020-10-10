package org.scummvm.scummvm;

import android.content.Context;
import android.view.MotionEvent;
import android.view.InputDevice;

// A class that extends the basic ScummVMEventsBase, supporting Android APIs > HONEYCOMB_MR1 (API 12)
public class ScummVMEventsModern extends ScummVMEventsBase {

	public ScummVMEventsModern(Context context, ScummVM scummvm, MouseHelper mouseHelper) {
		super(context, scummvm, mouseHelper);
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent e) {
		if ((e.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
			_scummvm.pushEvent(JE_JOYSTICK, e.getAction(),
					   (int)(e.getAxisValue(MotionEvent.AXIS_X)*100),
					   (int)(e.getAxisValue(MotionEvent.AXIS_Y)*100),
					   0, 0, 0);
			return true;
		}

		return false;
	}
}
