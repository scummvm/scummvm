package org.scummvm.scummvm;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

public class LedView extends View {
	public static final int DEFAULT_LED_COLOR = 0xffff0000;
	private static final int BLINK_TIME = 30; // ms

	private boolean _state;
	private Runnable _blink;
	private Paint _painter;
	private int _radius;
	private int _centerX;
	private int _centerY;

	public LedView(Context context) {
		this(context, true, DEFAULT_LED_COLOR);
	}

	public LedView(Context context, boolean state) {
			this(context, state, DEFAULT_LED_COLOR);
	}

	public LedView(Context context, boolean state, int color) {
		super(context);
		_state = state;
		init(color);
	}

	public LedView(Context context, @Nullable AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public LedView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		init(context, attrs, defStyleAttr, 0);
	}

	@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
	public LedView(
		Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		init(context, attrs, defStyleAttr, defStyleRes);
	}

	private void init(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		TypedArray a = context.getTheme().obtainStyledAttributes(
			attrs,
			R.styleable.LedView,
			defStyleAttr, defStyleRes);

		try {
			_state = a.getBoolean(R.styleable.LedView_state, true);
			int color = a.getColor(R.styleable.LedView_color, DEFAULT_LED_COLOR);
			init(color);
		} finally {
			a.recycle();
		}
	}

	private void init(int color) {
		_painter = new Paint();
		_painter.setStyle(Paint.Style.FILL);
		if (isInEditMode()) {
			_painter.setStrokeWidth(2);
			_painter.setStyle(_state ? Paint.Style.FILL : Paint.Style.STROKE);
		}
		_painter.setColor(color);
		_painter.setAntiAlias(true);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int minw = getPaddingLeft() + getPaddingRight() + getSuggestedMinimumWidth();
		int w = resolveSizeAndState(minw, widthMeasureSpec, 0);

		int minh = MeasureSpec.getSize(w) - getPaddingLeft() - getPaddingRight() +
			getPaddingBottom() + getPaddingTop();
		int h = resolveSizeAndState(minh, heightMeasureSpec, 0);

		setMeasuredDimension(w, h);
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);

		int xpad = (getPaddingLeft() + getPaddingRight());
		int ypad = (getPaddingTop() + getPaddingBottom());

		int ww = w - xpad;
		int hh = h - ypad;

		_radius = Math.min(ww, hh) / 2 - 2;
		_centerX = w / 2;
		_centerY = h / 2;
	}

	@Override
	protected void onDraw(@NonNull Canvas canvas) {
		super.onDraw(canvas);

		if (!_state && !isInEditMode()) {
			return;
		}

		canvas.drawCircle(_centerX, _centerY, _radius, _painter);
	}

	public void on() {
		setState(true);
	}

	public void off() {
		setState(false);
	}

	public void setState(boolean state) {
		if (_blink != null) {
			removeCallbacks(_blink);
			_blink = null;
		}

		if (_state == state) {
			return;
		}
		_state = state;
		invalidate();
	}

	public void blinkOnce() {
		if (_blink != null) {
			return;
		}

		boolean oldState = _state;
		_state = !oldState;
		invalidate();

		_blink = new Runnable() {
			private boolean _ran;

			@Override
			public void run() {
				if (_ran) {
					_blink = null;
					return;
				}

				_ran = true;
				_state = oldState;
				invalidate();

				postDelayed(this, BLINK_TIME);
			}
		};
		postDelayed(_blink, BLINK_TIME);
	}
}
