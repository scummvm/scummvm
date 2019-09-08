package org.scummvm.scummvm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.text.ClipboardManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.File;

public class ScummVMActivity extends Activity {

	/* Establish whether the hover events are available */
	private static boolean _hoverAvailable;

	private ClipboardManager _clipboard;

	static {
		try {
			MouseHelper.checkHoverAvailable(); // this throws exception if we're on too old version
			_hoverAvailable = true;
		} catch (Throwable t) {
			_hoverAvailable = false;
		}
	}

	public View.OnClickListener keyboardBtnOnClickListener = new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			runOnUiThread(new Runnable() {
					public void run() {
						toggleKeyboard();
					}
				});
		}
	};

	private class MyScummVM extends ScummVM {
		public MyScummVM(SurfaceHolder holder) {
			super(ScummVMActivity.this.getAssets(), holder);
		}

		@Override
		protected void getDPI(float[] values) {
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			values[0] = metrics.xdpi;
			values[1] = metrics.ydpi;
		}

		@Override
		protected void displayMessageOnOSD(String msg) {
			Log.i(LOG_TAG, "OSD: " + msg);
			Toast.makeText(ScummVMActivity.this, msg, Toast.LENGTH_LONG).show();
		}

		@Override
		protected void openUrl(String url) {
			startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
		}

		@Override
		protected boolean hasTextInClipboard() {
			return _clipboard.hasText();
		}

		@Override
		protected byte[] getTextFromClipboard() {
			CharSequence text = _clipboard.getText();
			if (text != null) {
				String encoding = getCurrentCharset();
				byte[] out;
				Log.d(LOG_TAG, String.format("Converting from UTF-8 to %s", encoding));
				try {
					out = text.toString().getBytes(encoding);
				} catch (java.io.UnsupportedEncodingException e) {
					out = text.toString().getBytes();
				}
				return out;
			}
			return null;
		}

		@Override
		protected boolean setTextInClipboard(byte[] text) {
			String encoding = getCurrentCharset();
			String out;
			Log.d(LOG_TAG, String.format("Converting from %s to UTF-8", encoding));
			try {
				out = new String(text, encoding);
			} catch (java.io.UnsupportedEncodingException e) {
				out = new String(text);
			}
			_clipboard.setText(out);
			return true;
		}

		@Override
		protected boolean isConnectionLimited() {
			WifiManager wifiMgr = (WifiManager)getSystemService(Context.WIFI_SERVICE);
			if (wifiMgr != null && wifiMgr.isWifiEnabled()) {
				WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
				return (wifiInfo == null || wifiInfo.getNetworkId() == -1); //WiFi is on, but it's not connected to any network
			}
			return true;
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
		protected void showVirtualKeyboard(final boolean enable) {
			runOnUiThread(new Runnable() {
					public void run() {
						showKeyboard(enable);
					}
				});
		}

		@Override
		protected void showKeyboardControl(final boolean enable) {
			runOnUiThread(new Runnable() {
					public void run() {
						showKeyboardView(enable);
					}
				});
		}

		@Override
		protected String[] getSysArchives() {
			return new String[0];
		}

	}

	private MyScummVM _scummvm;
	private ScummVMEvents _events;
	private MouseHelper _mouseHelper;
	private Thread _scummvm_thread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

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

		main_surface.requestFocus();

		getFilesDir().mkdirs();

		// Store savegames on external storage if we can, which means they're
		// world-readable and don't get deleted on uninstall.
		String savePath = Environment.getExternalStorageDirectory() + "/ScummVM/Saves/";
		File saveDir = new File(savePath);
		saveDir.mkdirs();
		if (!saveDir.isDirectory()) {
			// If it doesn't work, resort to the internal app path.
			savePath = getDir("saves", MODE_WORLD_READABLE).getPath();
		}

		_clipboard = (ClipboardManager)getSystemService(CLIPBOARD_SERVICE);

		// Start ScummVM
		_scummvm = new MyScummVM(main_surface.getHolder());

		_scummvm.setArgs(new String[] {
			"ScummVM",
			"--config=" + getFileStreamPath("scummvmrc").getPath(),
			"--path=" + Environment.getExternalStorageDirectory().getPath(),
			"--savepath=" + savePath
		});

		Log.d(ScummVM.LOG_TAG, "Hover available: " + _hoverAvailable);
		if (_hoverAvailable) {
			_mouseHelper = new MouseHelper(_scummvm);
			_mouseHelper.attach(main_surface);
		}

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB_MR1)
		{
			_events = new ScummVMEvents(this, _scummvm, _mouseHelper);
		}
		else
		{
			_events = new ScummVMEventsHoneycomb(this, _scummvm, _mouseHelper);
		}

		// On screen button listener
		((ImageView)findViewById(R.id.show_keyboard)).setOnClickListener(keyboardBtnOnClickListener);

		main_surface.setOnKeyListener(_events);
		main_surface.setOnTouchListener(_events);

		_scummvm_thread = new Thread(_scummvm, "ScummVM");
		_scummvm_thread.start();
	}

	@Override
	public void onStart() {
		Log.d(ScummVM.LOG_TAG, "onStart");

		super.onStart();
	}

	@Override
	public void onResume() {
		Log.d(ScummVM.LOG_TAG, "onResume");

		super.onResume();

		if (_scummvm != null)
			_scummvm.setPause(false);
		showMouseCursor(false);
	}

	@Override
	public void onPause() {
		Log.d(ScummVM.LOG_TAG, "onPause");

		super.onPause();

		if (_scummvm != null)
			_scummvm.setPause(true);
		showMouseCursor(true);
	}

	@Override
	public void onStop() {
		Log.d(ScummVM.LOG_TAG, "onStop");

		super.onStop();
	}

	@Override
	public void onDestroy() {
		Log.d(ScummVM.LOG_TAG, "onDestroy");

		super.onDestroy();

		if (_events != null) {
			_events.sendQuitEvent();

			try {
				// 1s timeout
				_scummvm_thread.join(1000);
			} catch (InterruptedException e) {
				Log.i(ScummVM.LOG_TAG, "Error while joining ScummVM thread", e);
			}

			_scummvm = null;
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTrackballEvent(e);

		return false;
	}

	@Override
	public boolean onGenericMotionEvent(final MotionEvent e) {
		if (_events != null)
			return _events.onGenericMotionEvent(e);

		return false;
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

	private void toggleKeyboard() {
		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);
		InputMethodManager imm = (InputMethodManager)
			getSystemService(INPUT_METHOD_SERVICE);

		imm.toggleSoftInputFromWindow(main_surface.getWindowToken(),
		                              InputMethodManager.SHOW_IMPLICIT,
		                              InputMethodManager.HIDE_IMPLICIT_ONLY);
	}

	private void showKeyboardView(boolean show) {
		ImageView keyboardBtn = (ImageView)findViewById(R.id.show_keyboard);

		if (show)
			keyboardBtn.setVisibility(View.VISIBLE);
		else
			keyboardBtn.setVisibility(View.GONE);
	}

	private void showMouseCursor(boolean show) {
		/* Currently hiding the system mouse cursor is only
		   supported on OUYA.  If other systems provide similar
		   intents, please add them here as well */
		Intent intent =
			new Intent(show?
				   "tv.ouya.controller.action.SHOW_CURSOR" :
				   "tv.ouya.controller.action.HIDE_CURSOR");
		sendBroadcast(intent);
	}
}
