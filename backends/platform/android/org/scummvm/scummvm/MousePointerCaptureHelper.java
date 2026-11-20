package org.scummvm.scummvm;

import android.os.Build;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.RequiresApi;

@RequiresApi(api = Build.VERSION_CODES.O)
public class MousePointerCaptureHelper implements View.OnCapturedPointerListener, View.OnLayoutChangeListener {

	private final MouseHelper _mouseHelper;
	private float _capturedMousePosX;
	private float _capturedMousePosY;
	private final View _targetView;
	private float _pointerSensitivity;
	private int _lastViewWidth;
	private int _lastViewHeight;

	public MousePointerCaptureHelper(View targetView, MouseHelper mouseHelper) {
		_targetView = targetView;
		_mouseHelper = mouseHelper;
		updateSensitivity(targetView);
		resetCapturedMousePos();

		// Handle layout changes (orientation, window resize) immediately
		_targetView.addOnLayoutChangeListener(this);
	}

	@Override
	public void onLayoutChange(View v, int left, int top, int right, int bottom,
							   int oldLeft, int oldTop, int oldRight, int oldBottom) {
		// Recalculate density in case of foldable unfold, multi-window resize, or external display move
		updateSensitivity(v);

		int newWidth = right - left;
		int newHeight = bottom - top;

		if (newWidth <= 0 || newHeight <= 0) {
			return;
		}

		// If this is the first valid layout, initialize dimensions without scaling
		if (_lastViewWidth <= 0 || _lastViewHeight <= 0) {
			_lastViewWidth = newWidth;
			_lastViewHeight = newHeight;
			_capturedMousePosX = newWidth / 2.0f;
			_capturedMousePosY = newHeight / 2.0f;
			return;
		}

		// Scale coordinates proportionally when layout dimensions change
		if (newWidth != _lastViewWidth || newHeight != _lastViewHeight) {
			// Proportionally scale cursor position
			_capturedMousePosX = _capturedMousePosX * (newWidth / (float) _lastViewWidth);
			_capturedMousePosY = _capturedMousePosY * (newHeight / (float) _lastViewHeight);

			// Safety check against NaN/Infinity after division
			if (Float.isNaN(_capturedMousePosX) || Float.isInfinite(_capturedMousePosX)) {
				_capturedMousePosX = newWidth / 2.0f;
			}
			if (Float.isNaN(_capturedMousePosY) || Float.isInfinite(_capturedMousePosY)) {
				_capturedMousePosY = newHeight / 2.0f;
			}

			// Clamp cursor coordinates, so as not to go outside screen dimensions
			float maxX = newWidth - 1.0f;
			float maxY = newHeight - 1.0f;
			_capturedMousePosX = Math.max(0.0f, Math.min(_capturedMousePosX, maxX));
			_capturedMousePosY = Math.max(0.0f, Math.min(_capturedMousePosY, maxY));

			_lastViewWidth = newWidth;
			_lastViewHeight = newHeight;

			// Notify ScummVM instantly during the layout pass
			long now = SystemClock.uptimeMillis();
			MotionEvent syntheticEvent = MotionEvent.obtain(now, now, MotionEvent.ACTION_HOVER_MOVE, _capturedMousePosX, _capturedMousePosY, 0);
			_mouseHelper.onMouseEvent(syntheticEvent, true, false, 0, 0, 0, 0);
			syntheticEvent.recycle();
		}
	}

	public void resetCapturedMousePos() {
		int w = Math.max(0, _targetView.getWidth());
		int h = Math.max(0, _targetView.getHeight());
		_lastViewWidth = w;
		_lastViewHeight = h;
		_capturedMousePosX = w / 2.0f;
		_capturedMousePosY = h / 2.0f;
	}

	@Override
	public boolean onCapturedPointer(View view, MotionEvent motionEvent) {
		int currViewWidth = _targetView.getWidth();
		int currViewHeight = _targetView.getHeight();

		if (currViewWidth <= 0 || currViewHeight <= 0) {
			return false;
		}

		float horizontalOffset = 0.0f;
		float verticalOffset = 0.0f;

		// For Android > 7.0 Instead of getting the relative movement values from getX() and getY(),
		// try to process all historical samples in the batch if they are available.
		// This should account for trackball rolls and fast mouse movements
		int movementHistorySize = motionEvent.getHistorySize();
		int action = motionEvent.getActionMasked();

		if (action == MotionEvent.ACTION_MOVE || action == MotionEvent.ACTION_HOVER_MOVE) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
				for (int i = 0; i < movementHistorySize; ++i) {
					horizontalOffset += motionEvent.getHistoricalAxisValue(MotionEvent.AXIS_RELATIVE_X, i);
					verticalOffset += motionEvent.getHistoricalAxisValue(MotionEvent.AXIS_RELATIVE_Y, i);
				}
				horizontalOffset += motionEvent.getAxisValue(MotionEvent.AXIS_RELATIVE_X);
				verticalOffset += motionEvent.getAxisValue(MotionEvent.AXIS_RELATIVE_Y);
			}

			// Fallback to getX()/getY() if relative axes are zero
			// (eg. on Android <= 10 or devices where relative axes are not populated!)
			if (horizontalOffset == 0.0f && verticalOffset == 0.0f) {
				for (int i = 0; i < movementHistorySize; ++i) {
					horizontalOffset += motionEvent.getHistoricalX(i);
					verticalOffset += motionEvent.getHistoricalY(i);
				}
				horizontalOffset += motionEvent.getX();
				verticalOffset += motionEvent.getY();
			}
		}

		// Clamping; We don't allow the cursor to go off-screen
		// (fixes bug with Myst 3 when panning the view and then returning to locked view/free mouse)
		float maxX = currViewWidth - 1.0f;
		float maxY = currViewHeight - 1.0f;

		float capturedMouseOffsetX = _pointerSensitivity * horizontalOffset;
		float capturedMouseOffsetY = _pointerSensitivity * verticalOffset;

		_capturedMousePosX = Math.max(0.0f, Math.min(_capturedMousePosX + capturedMouseOffsetX, maxX));
		_capturedMousePosY = Math.max(0.0f, Math.min(_capturedMousePosY + capturedMouseOffsetY, maxY));

		return _mouseHelper.onMouseEvent(motionEvent, true, true, (int)_capturedMousePosX, (int)_capturedMousePosY, (int)capturedMouseOffsetX, (int)capturedMouseOffsetY);
	}

	private void updateSensitivity(View trgView) {
		float density = trgView.getContext().getResources().getDisplayMetrics().density;
		// Use a multiplier, also factoring in screen dpi, to set mouse pointer speed
		// to something acceptable (it was noticeably slow otherwise)
		_pointerSensitivity = 1.15f * density;
	}

}
