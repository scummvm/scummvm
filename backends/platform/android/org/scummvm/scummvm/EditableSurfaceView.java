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
		outAttrs.inputType = (InputType.TYPE_CLASS_TEXT |
								InputType.TYPE_TEXT_VARIATION_NORMAL |
								InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE);
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
