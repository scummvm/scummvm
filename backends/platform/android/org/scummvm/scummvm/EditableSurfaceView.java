package org.scummvm.scummvm;

import android.content.Context;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.PointerIcon;
import android.view.SurfaceView;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.annotation.TargetApi;

public class EditableSurfaceView extends SurfaceView {
	Context _context;
	public EditableSurfaceView(Context context) {

		super(context);
		_context = context;
	}

	public EditableSurfaceView(Context context, AttributeSet attrs) {
		super(context, attrs);
		_context = context;
	}

	public EditableSurfaceView(Context context, AttributeSet attrs,
								int defStyle) {
		super(context, attrs, defStyle);
		_context = context;
	}

	@Override
	public boolean onCheckIsTextEditor() {
		return false;
	}

	private class MyInputConnection extends BaseInputConnection {
		// The false second argument puts the BaseInputConnection into "dummy" mode, which is also required in order for the raw key events to be sent to your view.
		// In the BaseInputConnection code, you can find several comments such as the following: "only if dummy mode, a key event is sent for the new text and the current editable buffer cleared."
		// Reference: https://stackoverflow.com/a/7386854
		public MyInputConnection() {
			super(EditableSurfaceView.this, false);
		}

		@Override
		public boolean performEditorAction(int actionCode) {
			if (actionCode == EditorInfo.IME_ACTION_DONE) {
				InputMethodManager imm = (InputMethodManager)
					getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(getWindowToken(), 0);
			}

			// Sends enter key
			return super.performEditorAction(actionCode);
		}
	}

	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
		outAttrs.initialCapsMode = 0;
		outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;

		// Per the documentation for InputType.TYPE_NULL:
		// "This should be interpreted to mean that the target input connection is not rich,
		//   it can not process and show things like candidate text nor retrieve the current text,
		//   so the input method will need to run in a limited 'generate key events' mode."
		// Reference: https://stackoverflow.com/a/7386854
		// We lose auto-complete, but that is ok, because we *really* want direct input key handling
		outAttrs.inputType = InputType.TYPE_NULL;

		// IME_FLAG_NO_EXTRACT_UI used to specify that the IME does not need to show its extracted text UI
		// IME_ACTION_NONE Bits of IME_MASK_ACTION: there is no available action.
		// TODO should we have a IME_ACTION_DONE?
		outAttrs.imeOptions = (EditorInfo.IME_ACTION_NONE |
								EditorInfo.IME_FLAG_NO_EXTRACT_UI);

		return new MyInputConnection();
	}

	// This re-inforces the code for hiding the system mouse.
	// We already had code for this in ScummVMActivity (see showMouseCursor())
	// so this might be redundant
	//
	// It applies on devices running Android 7 and above
	// https://stackoverflow.com/a/55482761
	// https://developer.android.com/reference/android/view/PointerIcon.html
	//
	@TargetApi(24)
	@Override
	public PointerIcon onResolvePointerIcon(MotionEvent me, int pointerIndex) {
		return PointerIcon.getSystemIcon(_context, PointerIcon.TYPE_NULL);
	}
}
