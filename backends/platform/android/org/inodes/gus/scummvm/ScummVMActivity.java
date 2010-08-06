package org.inodes.gus.scummvm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import java.io.IOException;

public class ScummVMActivity extends Activity {
	private boolean _do_right_click;
	private boolean _last_click_was_right;

	// game pixels to move per trackball/dpad event.
	// FIXME: replace this with proper mouse acceleration
	private final static int TRACKBALL_SCALE = 2;

	private class MyScummVM extends ScummVM {
		private boolean scummvmRunning = false;

		private boolean usingSmallScreen() {
			// Multiple screen sizes came in with Android 1.6.  Have
			// to use reflection in order to continue supporting 1.5
			// devices :(
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);
			try {
				// This 'density' term is very confusing.
				int DENSITY_LOW = metrics.getClass().getField("DENSITY_LOW").getInt(null);
				int densityDpi = metrics.getClass().getField("densityDpi").getInt(metrics);
				return densityDpi <= DENSITY_LOW;
			} catch (Exception e) {
				return false;
			}
		}

		public MyScummVM() {
			super(ScummVMActivity.this);

			// Enable ScummVM zoning on 'small' screens.
			enableZoning(usingSmallScreen());
		}

		@Override
		protected void initBackend() throws ScummVM.AudioSetupException {
			synchronized (this) {
				scummvmRunning = true;
				notifyAll();
			}
			super.initBackend();
		}

		public void waitUntilRunning() throws InterruptedException {
			synchronized (this) {
				while (!scummvmRunning)
					wait();
			}
		}

		@Override
		protected void displayMessageOnOSD(String msg) {
			Log.i(this.toString(), "OSD: " + msg);
			Toast.makeText(ScummVMActivity.this, msg, Toast.LENGTH_LONG).show();
		}

		@Override
		protected void setWindowCaption(final String caption) {
			runOnUiThread(new Runnable() {
					public void run() {
						setTitle(caption);
					}
				});
		}

		@Override
		protected String[] getPluginDirectories() {
			String[] dirs = new String[1];
			dirs[0] = ScummVMApplication.getLastCacheDir().getPath();
			return dirs;
		}

		@Override
		protected void showVirtualKeyboard(final boolean enable) {
			if (getResources().getConfiguration().keyboard ==
				Configuration.KEYBOARD_NOKEYS) {
				runOnUiThread(new Runnable() {
						public void run() {
							showKeyboard(enable);
						}
					});
			}
		}
	}
	private MyScummVM scummvm;
	private Thread scummvm_thread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		_do_right_click = false;
		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		setContentView(R.layout.main);
		takeKeyEvents(true);

		// This is a common enough error that we should warn about it
		// explicitly.
		if (!Environment.getExternalStorageDirectory().canRead()) {
			new AlertDialog.Builder(this)
				.setTitle(R.string.no_sdcard_title)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(R.string.no_sdcard)
				.setNegativeButton(R.string.quit,
								   new DialogInterface.OnClickListener() {
									   public void onClick(DialogInterface dialog,
														   int which) {
										   finish();
									   }
								   })
				.show();
			return;
		}

		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);
		main_surface.setOnTouchListener(new View.OnTouchListener() {
				public boolean onTouch(View v, MotionEvent event) {
					return onTouchEvent(event);
				}
			});
		main_surface.setOnKeyListener(new View.OnKeyListener() {
				public boolean onKey(View v, int code, KeyEvent ev) {
					return onKeyDown(code, ev);
				}
			});
		main_surface.requestFocus();

		// Start ScummVM
		scummvm = new MyScummVM();
		scummvm_thread = new Thread(new Runnable() {
				public void run() {
					try {
						runScummVM();
					} catch (Exception e) {
						Log.e("ScummVM", "Fatal error in ScummVM thread", e);
						new AlertDialog.Builder(ScummVMActivity.this)
							.setTitle("Error")
							.setMessage(e.toString())
							.setIcon(android.R.drawable.ic_dialog_alert)
							.show();
						finish();
					}
				}
			}, "ScummVM");
		scummvm_thread.start();

		// Block UI thread until ScummVM has started.  In particular,
		// this means that surface and event callbacks should be safe
		// after this point.
		try {
			scummvm.waitUntilRunning();
		} catch (InterruptedException e) {
			Log.e(this.toString(),
				  "Interrupted while waiting for ScummVM.initBackend", e);
			finish();
		}

		scummvm.setSurface(main_surface.getHolder());
	}

	// Runs in another thread
	private void runScummVM() throws IOException {
		getFilesDir().mkdirs();
		String[] args = {
			"ScummVM-lib",
			"--config=" + getFileStreamPath("scummvmrc").getPath(),
			"--path=" + Environment.getExternalStorageDirectory().getPath(),
			"--gui-theme=scummmodern",
			"--savepath=" + getDir("saves", 0).getPath(),
		};

		int ret = scummvm.scummVMMain(args);

		// On exit, tear everything down for a fresh
		// restart next time.
		System.exit(ret);
	}

	private boolean was_paused = false;

	@Override
	public void onPause() {
		if (scummvm != null) {
			was_paused = true;
			scummvm.pause();
		}
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		if (scummvm != null && was_paused)
			scummvm.resume();
		was_paused = false;
	}

	@Override
	public void onStop() {
		if (scummvm != null) {
			scummvm.pushEvent(new Event(Event.EVENT_QUIT));
			try {
				scummvm_thread.join(1000);	// 1s timeout
			} catch (InterruptedException e) {
				Log.i(this.toString(),
					  "Error while joining ScummVM thread", e);
			}
		}
		super.onStop();
	}

	static final int MSG_MENU_LONG_PRESS = 1;
	private final Handler keycodeMenuTimeoutHandler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				if (msg.what == MSG_MENU_LONG_PRESS) {
					InputMethodManager imm = (InputMethodManager)
						getSystemService(INPUT_METHOD_SERVICE);
					if (imm != null)
						imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
				}
			}
		};

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent kevent) {
		return onKeyDown(keyCode, kevent);
	}

	@Override
	public boolean onKeyMultiple(int keyCode, int repeatCount,
									 KeyEvent kevent) {
		return onKeyDown(keyCode, kevent);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent kevent) {
		// Filter out "special" keys
		switch (keyCode) {
		case KeyEvent.KEYCODE_MENU:
			// Have to reimplement hold-down-menu-brings-up-softkeybd
			// ourselves, since we are otherwise hijacking the menu
			// key :(
			// See com.android.internal.policy.impl.PhoneWindow.onKeyDownPanel()
			// for the usual Android implementation of this feature.
			if (kevent.getRepeatCount() > 0)
				// Ignore keyrepeat for menu
				return false;
			boolean timeout_fired = false;
			if (getResources().getConfiguration().keyboard ==
				Configuration.KEYBOARD_NOKEYS) {
				timeout_fired = !keycodeMenuTimeoutHandler.hasMessages(MSG_MENU_LONG_PRESS);
				keycodeMenuTimeoutHandler.removeMessages(MSG_MENU_LONG_PRESS);
				if (kevent.getAction() == KeyEvent.ACTION_DOWN) {
					keycodeMenuTimeoutHandler.sendMessageDelayed(
																 keycodeMenuTimeoutHandler.obtainMessage(MSG_MENU_LONG_PRESS),
																 ViewConfiguration.getLongPressTimeout());
					return true;
				}
			}
			if (kevent.getAction() == KeyEvent.ACTION_UP) {
				if (!timeout_fired)
					scummvm.pushEvent(new Event(Event.EVENT_MAINMENU));
				return true;
			}
			return false;
		case KeyEvent.KEYCODE_CAMERA:
		case KeyEvent.KEYCODE_SEARCH:
			_do_right_click = (kevent.getAction() == KeyEvent.ACTION_DOWN);
			return true;
		case KeyEvent.KEYCODE_DPAD_CENTER:
		case KeyEvent.KEYCODE_DPAD_UP:
		case KeyEvent.KEYCODE_DPAD_DOWN:
		case KeyEvent.KEYCODE_DPAD_LEFT:
		case KeyEvent.KEYCODE_DPAD_RIGHT: {
			// HTC Hero doesn't seem to generate
			// MotionEvent.ACTION_DOWN events on trackball press :(
			// We'll have to just fake one here.
			// Some other handsets lack a trackball, so the DPAD is
			// the only way of moving the cursor.
			int motion_action;
			// FIXME: this logic is a mess.
			if (keyCode == KeyEvent.KEYCODE_DPAD_CENTER) {
				switch (kevent.getAction()) {
				case KeyEvent.ACTION_DOWN:
					motion_action = MotionEvent.ACTION_DOWN;
					break;
				case KeyEvent.ACTION_UP:
					motion_action = MotionEvent.ACTION_UP;
					break;
				default:  // ACTION_MULTIPLE
					return false;
				}
			} else
				motion_action = MotionEvent.ACTION_MOVE;

			Event e = new Event(getEventType(motion_action));
			e.mouse_x = 0;
			e.mouse_y = 0;
			e.mouse_relative = true;
			switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_UP:
				e.mouse_y = -TRACKBALL_SCALE;
				break;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				e.mouse_y = TRACKBALL_SCALE;
				break;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				e.mouse_x = -TRACKBALL_SCALE;
				break;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				e.mouse_x = TRACKBALL_SCALE;
				break;
			}
			scummvm.pushEvent(e);
			return true;
		}
		case KeyEvent.KEYCODE_BACK:
			// skip isSystem() check and fall through to main code
			break;
		default:
			if (kevent.isSystem())
				return false;
		}

		// FIXME: what do I need to do for composed characters?

		Event e = new Event();

		switch (kevent.getAction()) {
		case KeyEvent.ACTION_DOWN:
			e.type = Event.EVENT_KEYDOWN;
			e.synthetic = false;
			break;
		case KeyEvent.ACTION_UP:
			e.type = Event.EVENT_KEYUP;
			e.synthetic = false;
			break;
		case KeyEvent.ACTION_MULTIPLE:
			// e.type is handled below
			e.synthetic = true;
			break;
		default:
			return false;
		}

		e.kbd_keycode = Event.androidKeyMap.containsKey(keyCode) ?
			Event.androidKeyMap.get(keyCode) : Event.KEYCODE_INVALID;
		e.kbd_ascii = kevent.getUnicodeChar();
		if (e.kbd_ascii == 0)
			e.kbd_ascii = e.kbd_keycode; // scummvm keycodes are mostly ascii


		e.kbd_flags = 0;
		if (kevent.isAltPressed())
			e.kbd_flags |= Event.KBD_ALT;
		if (kevent.isSymPressed()) // no ctrl key in android, so use sym (?)
			e.kbd_flags |= Event.KBD_CTRL;
		if (kevent.isShiftPressed()) {
			if (keyCode >= KeyEvent.KEYCODE_0 &&
				keyCode <= KeyEvent.KEYCODE_9) {
				// Shift+number -> convert to F* key
				int offset = keyCode == KeyEvent.KEYCODE_0 ?
					10 : keyCode - KeyEvent.KEYCODE_1; // turn 0 into 10
				e.kbd_keycode = Event.KEYCODE_F1 + offset;
				e.kbd_ascii = Event.ASCII_F1 + offset;
			} else
				e.kbd_flags |= Event.KBD_SHIFT;
		}

		if (kevent.getAction() == KeyEvent.ACTION_MULTIPLE) {
			for (int i = 0; i <= kevent.getRepeatCount(); i++) {
				e.type = Event.EVENT_KEYDOWN;
				scummvm.pushEvent(e);
				e.type = Event.EVENT_KEYUP;
				scummvm.pushEvent(e);
			}
		} else
			scummvm.pushEvent(e);

		return true;
	}

	private int getEventType(int action) {
		switch (action) {
		case MotionEvent.ACTION_DOWN:
			_last_click_was_right = _do_right_click;
			return _last_click_was_right ?
				Event.EVENT_RBUTTONDOWN : Event.EVENT_LBUTTONDOWN;
		case MotionEvent.ACTION_UP:
			return _last_click_was_right ?
				Event.EVENT_RBUTTONUP : Event.EVENT_LBUTTONUP;
		case MotionEvent.ACTION_MOVE:
			return Event.EVENT_MOUSEMOVE;
		default:
			return Event.EVENT_INVALID;
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent event) {
		int type = getEventType(event.getAction());
		if (type == Event.EVENT_INVALID)
			return false;

		Event e = new Event(type);
		e.mouse_x =
			(int)(event.getX() * event.getXPrecision()) * TRACKBALL_SCALE;
		e.mouse_y =
			(int)(event.getY() * event.getYPrecision()) * TRACKBALL_SCALE;
		e.mouse_relative = true;
		scummvm.pushEvent(e);

		return true;
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		int type = getEventType(event.getAction());
		if (type == Event.EVENT_INVALID)
			return false;

		Event e = new Event(type);
		e.mouse_x = (int)event.getX();
		e.mouse_y = (int)event.getY();
		e.mouse_relative = false;
		scummvm.pushEvent(e);

		return true;
	}

	private void showKeyboard(boolean show) {
		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);
		InputMethodManager imm = (InputMethodManager)
			getSystemService(INPUT_METHOD_SERVICE);
		if (show)
			imm.showSoftInput(main_surface, InputMethodManager.SHOW_IMPLICIT);
		else
			imm.hideSoftInputFromWindow(main_surface.getWindowToken(),
										InputMethodManager.HIDE_IMPLICIT_ONLY);
	}
}
