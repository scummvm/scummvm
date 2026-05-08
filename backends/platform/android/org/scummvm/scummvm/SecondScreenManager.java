package org.scummvm.scummvm;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Display;

import androidx.annotation.RequiresApi;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
public class SecondScreenManager {

	private final Context _context;
	private final ScummVM _scummvm;
	private final DisplayManager _displayManager;
	private final int _mainScreenWidth;
	private final int _mainScreenHeight;
	private SecondScreenPresentation _presentation;
	private DisplayManager.DisplayListener _displayListener;
	private boolean _enabled = false;

	public SecondScreenManager(Context context, ScummVM scummvm, int mainScreenWidth, int mainScreenHeight) {
		_context = context;
		_scummvm = scummvm;
		_displayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
		_mainScreenWidth = mainScreenWidth;
		_mainScreenHeight = mainScreenHeight;
	}

	public void start() {
		if (_displayManager == null) return;

		_displayListener = new DisplayManager.DisplayListener() {
			@Override
			public void onDisplayAdded(int displayId) {
				if (_enabled) {
					tryShowPresentation();
				}
			}

			@Override
			public void onDisplayRemoved(int displayId) {
				dismissPresentation();
			}

			@Override
			public void onDisplayChanged(int displayId) {
			}
		};

		_displayManager.registerDisplayListener(_displayListener,
			new Handler(Looper.getMainLooper()));
	}

	public void stop() {
		dismissPresentation();
		if (_displayManager != null && _displayListener != null) {
			_displayManager.unregisterDisplayListener(_displayListener);
			_displayListener = null;
		}
	}

	public boolean toggle() {
		_enabled = !_enabled;
		if (_enabled) {
			tryShowPresentation();
		} else {
			dismissPresentation();
		}
		return _enabled;
	}

	public boolean isEnabled() {
		return _enabled;
	}

	public boolean hasSecondaryDisplay() {
		return findPresentationDisplay() != null;
	}

	public void onPause() {
		dismissPresentation();
	}

	public void onResume() {
		if (_enabled) {
			tryShowPresentation();
		}
	}

	private void tryShowPresentation() {
		Display display = findPresentationDisplay();
		if (display != null) {
			showPresentation(display);
		}
	}

	private void showPresentation(Display display) {
		dismissPresentation();
		try {
			_presentation = new SecondScreenPresentation(
				_context, display, _scummvm, _mainScreenWidth, _mainScreenHeight);
			_presentation.show();
		} catch (Exception e) {
			_presentation = null;
		}
	}

	private void dismissPresentation() {
		if (_presentation != null) {
			try {
				_presentation.dismiss();
			} catch (Exception ignored) {
			}
			_presentation = null;
		}
	}

	private Display findPresentationDisplay() {
		if (_displayManager == null) return null;

		Display[] displays = _displayManager.getDisplays(
			DisplayManager.DISPLAY_CATEGORY_PRESENTATION);

		for (Display d : displays) {
			if (d.getDisplayId() != 0 && !"HiddenDisplay".equals(d.getName())) {
				return d;
			}
		}
		return null;
	}
}
