package org.scummvm.scummvm;

import android.app.Presentation;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.RequiresApi;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
public class SecondScreenPresentation extends Presentation {

	private final ScummVM _scummvm;
	private final int _mainScreenWidth;
	private final int _mainScreenHeight;
	private SecondScreenTouchpadView _touchpadView;

	public SecondScreenPresentation(Context context, Display display, ScummVM scummvm,
			int mainScreenWidth, int mainScreenHeight) {
		super(context, display);
		_scummvm = scummvm;
		_mainScreenWidth = mainScreenWidth;
		_mainScreenHeight = mainScreenHeight;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Window window = getWindow();
		if (window != null) {
			window.setFlags(
				WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
					| WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
				WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
					| WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
			);
		}

		_touchpadView = new SecondScreenTouchpadView(getContext(), _scummvm,
			_mainScreenWidth, _mainScreenHeight);
		setContentView(_touchpadView);
	}

	public SecondScreenTouchpadView getTouchpadView() {
		return _touchpadView;
	}
}
