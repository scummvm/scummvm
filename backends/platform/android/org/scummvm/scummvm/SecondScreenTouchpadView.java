package org.scummvm.scummvm;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Build;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.RequiresApi;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
public class SecondScreenTouchpadView extends View {

	private static final int JE_LMB_DOWN = 9;
	private static final int JE_LMB_UP = 10;
	private static final int JE_RMB_DOWN = 11;
	private static final int JE_RMB_UP = 12;
	private static final int JE_MOUSE_MOVE = 13;
	private static final int JE_MOUSE_WHEEL_UP = 22;
	private static final int JE_MOUSE_WHEEL_DOWN = 23;

	private static final int TAP_TIMEOUT_MS = 300;
	private static final float TAP_SLOP_PX = 20f;
	private static final float ACCELERATION_THRESHOLD = 6f;
	private static final float ACCELERATION_MULTIPLIER = 1.5f;
	private static final float SCROLL_THRESHOLD = 30f;
	private static final float DEFAULT_SENSITIVITY = 1.5f;

	private final ScummVM _scummvm;
	private final Paint _bgPaint;
	private final Paint _borderPaint;
	private final Paint _textPaint;
	private final Paint _touchPaint;
	private final Paint _touchPaintActive;

	private int _cursorX;
	private int _cursorY;
	private int _screenWidth;
	private int _screenHeight;

	private float _lastTouchX;
	private float _lastTouchY;
	private boolean _isDragging;
	private int _activePointers;

	private long _touchDownTime;
	private float _touchDownX;
	private float _touchDownY;

	private float _scrollBaseY;
	private boolean _isScrolling;
	private float _scrollAccum;

	private float _sensitivity = DEFAULT_SENSITIVITY;
	private boolean _enabled = true;

	private float _currentTouchX = -1;
	private float _currentTouchY = -1;

	public SecondScreenTouchpadView(Context context, ScummVM scummvm,
			int mainScreenWidth, int mainScreenHeight) {
		super(context);
		_scummvm = scummvm;
		_screenWidth = mainScreenWidth;
		_screenHeight = mainScreenHeight;
		_cursorX = _screenWidth / 2;
		_cursorY = _screenHeight / 2;

		_bgPaint = new Paint();
		_bgPaint.setColor(0xFF000000);
		_bgPaint.setStyle(Paint.Style.FILL);

		_borderPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		_borderPaint.setColor(0x20FFFFFF);
		_borderPaint.setStyle(Paint.Style.STROKE);
		_borderPaint.setStrokeWidth(1f);

		_textPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		_textPaint.setColor(0x30FFFFFF);
		_textPaint.setTextSize(28f);
		_textPaint.setTextAlign(Paint.Align.CENTER);

		_touchPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		_touchPaint.setColor(0x18FFFFFF);
		_touchPaint.setStyle(Paint.Style.FILL);

		_touchPaintActive = new Paint(Paint.ANTI_ALIAS_FLAG);
		_touchPaintActive.setColor(0x30FFFFFF);
		_touchPaintActive.setStyle(Paint.Style.FILL);
	}

	public void setScreenResolution(int w, int h) {
		_screenWidth = Math.max(w, 1);
		_screenHeight = Math.max(h, 1);
		_cursorX = clamp(_cursorX, 0, _screenWidth - 1);
		_cursorY = clamp(_cursorY, 0, _screenHeight - 1);
	}

	public void setSensitivity(float sensitivity) {
		_sensitivity = sensitivity;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		int w = getWidth();
		int h = getHeight();

		canvas.drawRect(0, 0, w, h, _bgPaint);

		float pad = 16f;
		canvas.drawRoundRect(pad, pad, w - pad, h - pad, 16, 16, _borderPaint);

		if (_currentTouchX >= 0 && _currentTouchY >= 0) {
			canvas.drawCircle(_currentTouchX, _currentTouchY, 40f,
				_isDragging ? _touchPaintActive : _touchPaint);
		}

		canvas.drawText("ScummVM Touchpad", w / 2f, h / 2f - 20, _textPaint);

		Paint hintPaint = new Paint(_textPaint);
		hintPaint.setTextSize(20f);
		hintPaint.setColor(0x40FFFFFF);
		canvas.drawText("tap = click • 2-finger tap = right click • 2-finger drag = scroll",
			w / 2f, h / 2f + 20, hintPaint);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (!_enabled) return false;

		int action = event.getActionMasked();

		switch (action) {
			case MotionEvent.ACTION_DOWN:
				_activePointers = 1;
				_touchDownX = event.getX();
				_touchDownY = event.getY();
				_touchDownTime = System.currentTimeMillis();
				_lastTouchX = event.getX();
				_lastTouchY = event.getY();
				_isDragging = false;
				_isScrolling = false;
				_scrollAccum = 0;
				_currentTouchX = event.getX();
				_currentTouchY = event.getY();
				invalidate();
				return true;

			case MotionEvent.ACTION_POINTER_DOWN:
				_activePointers = event.getPointerCount();
				if (_activePointers == 2) {
					_scrollBaseY = averageY(event);
					_isScrolling = false;
					_scrollAccum = 0;
				}
				return true;

			case MotionEvent.ACTION_MOVE:
				if (_activePointers == 1) {
					float dx = event.getX() - _lastTouchX;
					float dy = event.getY() - _lastTouchY;
					_lastTouchX = event.getX();
					_lastTouchY = event.getY();
					_currentTouchX = event.getX();
					_currentTouchY = event.getY();

					float dist = distance(_touchDownX, _touchDownY, event.getX(), event.getY());
					if (dist > TAP_SLOP_PX) {
						_isDragging = true;
					}
					if (_isDragging) {
						moveCursor(dx, dy);
					}
					invalidate();
				} else if (_activePointers == 2) {
					float currentAvgY = averageY(event);
					float scrollDelta = currentAvgY - _scrollBaseY;
					_scrollAccum += scrollDelta;
					_scrollBaseY = currentAvgY;

					if (Math.abs(_scrollAccum) > SCROLL_THRESHOLD) {
						_isScrolling = true;
						if (_scrollAccum < 0) {
							_scummvm.pushEvent(JE_MOUSE_WHEEL_UP, _cursorX, _cursorY, 0, 0, 0, 0);
						} else {
							_scummvm.pushEvent(JE_MOUSE_WHEEL_DOWN, _cursorX, _cursorY, 0, 0, 0, 0);
						}
						_scrollAccum = 0;
					}
				}
				return true;

			case MotionEvent.ACTION_POINTER_UP:
				int newCount = event.getPointerCount() - 1;
				if (_activePointers == 2 && newCount == 1 && !_isScrolling) {
					long elapsed = System.currentTimeMillis() - _touchDownTime;
					if (elapsed < TAP_TIMEOUT_MS) {
						rightClick();
					}
				}
				_activePointers = newCount;
				if (_activePointers == 1) {
					int remaining = findRemainingPointer(event);
					if (remaining >= 0) {
						_lastTouchX = event.getX(remaining);
						_lastTouchY = event.getY(remaining);
					}
				}
				return true;

			case MotionEvent.ACTION_UP:
				if (_activePointers == 1 && !_isDragging) {
					long elapsed = System.currentTimeMillis() - _touchDownTime;
					if (elapsed < TAP_TIMEOUT_MS) {
						leftClick();
					}
				}
				_activePointers = 0;
				_currentTouchX = -1;
				_currentTouchY = -1;
				invalidate();
				return true;

			case MotionEvent.ACTION_CANCEL:
				_activePointers = 0;
				_currentTouchX = -1;
				_currentTouchY = -1;
				invalidate();
				return true;
		}
		return false;
	}

	private void moveCursor(float rawDx, float rawDy) {
		float dx = rawDx * _sensitivity;
		float dy = rawDy * _sensitivity;

		float mag = (float) Math.sqrt(dx * dx + dy * dy);
		if (mag > ACCELERATION_THRESHOLD) {
			dx *= ACCELERATION_MULTIPLIER;
			dy *= ACCELERATION_MULTIPLIER;
		}

		_cursorX = clamp(_cursorX + Math.round(dx), 0, _screenWidth - 1);
		_cursorY = clamp(_cursorY + Math.round(dy), 0, _screenHeight - 1);

		_scummvm.pushEvent(JE_MOUSE_MOVE, _cursorX, _cursorY, 0, 0, 0, 0);
	}

	private void leftClick() {
		_scummvm.pushEvent(JE_LMB_DOWN, _cursorX, _cursorY, 0, 0, 0, 0);
		_scummvm.pushEvent(JE_LMB_UP, _cursorX, _cursorY, 0, 0, 0, 0);
	}

	private void rightClick() {
		_scummvm.pushEvent(JE_RMB_DOWN, _cursorX, _cursorY, 0, 0, 0, 0);
		_scummvm.pushEvent(JE_RMB_UP, _cursorX, _cursorY, 0, 0, 0, 0);
	}

	private float averageY(MotionEvent event) {
		float sum = 0;
		int count = Math.min(event.getPointerCount(), 2);
		for (int i = 0; i < count; i++) {
			sum += event.getY(i);
		}
		return sum / count;
	}

	private int findRemainingPointer(MotionEvent event) {
		int actionIndex = event.getActionIndex();
		for (int i = 0; i < event.getPointerCount(); i++) {
			if (i != actionIndex) return i;
		}
		return -1;
	}

	private static float distance(float x1, float y1, float x2, float y2) {
		float dx = x2 - x1;
		float dy = y2 - y1;
		return (float) Math.sqrt(dx * dx + dy * dy);
	}

	private static int clamp(int val, int min, int max) {
		return Math.max(min, Math.min(max, val));
	}
}
