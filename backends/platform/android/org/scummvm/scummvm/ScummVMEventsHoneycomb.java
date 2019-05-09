package org.scummvm.scummvm;

import android.content.Context;
import android.view.MotionEvent;
import android.view.InputDevice;

public class ScummVMEventsHoneycomb extends ScummVMEvents {

	public ScummVMEventsHoneycomb(Context context, ScummVM scummvm, MouseHelper mouseHelper) {
		super(context, scummvm, mouseHelper);
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent e) {
		if((e.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
			_scummvm.pushEvent(JE_JOYSTICK, e.getAction(),
					   (int)(e.getAxisValue(MotionEvent.AXIS_X)*100),
					   (int)(e.getAxisValue(MotionEvent.AXIS_Y)*100),
					   0, 0, 0);
			return true;
		}

		return false;
	}
}
