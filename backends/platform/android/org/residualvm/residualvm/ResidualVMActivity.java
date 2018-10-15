package org.residualvm.residualvm;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.os.Bundle;
import android.os.Environment;
import android.text.ClipboardManager;
import android.support.v4.content.ContextCompat;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;

import java.io.File;

public class ResidualVMActivity extends Activity {

private boolean isBtnsShowing = false;

public View.OnClickListener optionsBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {

	if(!isBtnsShowing)
            ((HorizontalScrollView)findViewById(R.id.btns_scrollview)).setVisibility(View.VISIBLE);
	else
	    ((HorizontalScrollView)findViewById(R.id.btns_scrollview)).setVisibility(View.GONE);

	    isBtnsShowing = !isBtnsShowing;

        }
    };

    private void emulateKeyPress(int keyCode){
		_residualvm.pushEvent(ResidualVMEvents.JE_KEY, KeyEvent.ACTION_DOWN, keyCode, 0, 0, 0, 0);
		_residualvm.pushEvent(ResidualVMEvents.JE_KEY, KeyEvent.ACTION_UP, keyCode, 0, 0, 0, 0);
    }

public View.OnClickListener menuBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	emulateKeyPress(KeyEvent.KEYCODE_F1);
        }
    };

public View.OnClickListener inventoryBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	emulateKeyPress(KeyEvent.KEYCODE_I);
        }
    };

public View.OnClickListener lookAtBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	emulateKeyPress(KeyEvent.KEYCODE_E);
        }
    };

public View.OnClickListener useBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	emulateKeyPress(KeyEvent.KEYCODE_ENTER);
        }
    };

public View.OnClickListener pickUpBtnOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        	emulateKeyPress(KeyEvent.KEYCODE_P);
        }
    };

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

	private class MyResidualVM extends ResidualVM {
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

		public MyResidualVM(SurfaceHolder holder) {
			super(ResidualVMActivity.this.getAssets(), holder);

			// Enable ResidualVM zoning on 'small' screens.
			// FIXME make this optional for the user
			// disabled for now since it crops too much
			//enableZoning(usingSmallScreen());
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
			Toast.makeText(ResidualVMActivity.this, msg, Toast.LENGTH_LONG).show();
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
			WifiManager wifiMgr = (WifiManager)getApplicationContext().getSystemService(Context.WIFI_SERVICE);
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
		protected String[] getSysArchives() {
			return new String[0];
		}

	}

	private MyResidualVM _residualvm;
	private ResidualVMEvents _events;
	private MouseHelper _mouseHelper;
	private Thread _residualvm_thread;

	private boolean checkPermissions() {
		return ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
				&& ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		boolean have_permissions = checkPermissions();

		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		setContentView(R.layout.main);
		takeKeyEvents(true);

		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);

		main_surface.requestFocus();

		getFilesDir().mkdirs();

		// Store savegames on external storage if we can, which means they're
		// world-readable and don't get deleted on uninstall.
		String savePath = Environment.getExternalStorageDirectory() + "/ResidualVM/Saves/";
		File saveDir = new File(savePath);
		saveDir.mkdirs();
		if (!saveDir.isDirectory()) {
			// If it doesn't work, resort to the internal app path.
			savePath = getDir("saves", MODE_WORLD_READABLE).getPath();
		}

		_clipboard = (ClipboardManager)getSystemService(CLIPBOARD_SERVICE);

		// Start ResidualVM
		_residualvm = new MyResidualVM(main_surface.getHolder());

		_residualvm.setArgs(new String[] {
			"ResidualVM",
			"--config=" + getFileStreamPath("residualvmrc").getPath(),
			"--path=" + Environment.getExternalStorageDirectory().getPath(),
			"--savepath=" + savePath
		});

		Log.d(ResidualVM.LOG_TAG, "Hover available: " + _hoverAvailable);
		if (_hoverAvailable) {
			_mouseHelper = new MouseHelper(_residualvm);
			_mouseHelper.attach(main_surface);
		}

		_events = new ResidualVMEvents(this, _residualvm, _mouseHelper);

		// On screen buttons listeners
		((ImageView)findViewById(R.id.options)).setOnClickListener(optionsBtnOnClickListener);
		((Button)findViewById(R.id.menu_btn)).setOnClickListener(menuBtnOnClickListener);
		((Button)findViewById(R.id.inventory_btn)).setOnClickListener(inventoryBtnOnClickListener);
		((Button)findViewById(R.id.use_btn)).setOnClickListener(useBtnOnClickListener);
		((Button)findViewById(R.id.pick_up_btn)).setOnClickListener(pickUpBtnOnClickListener);
		((Button)findViewById(R.id.look_at_btn)).setOnClickListener(lookAtBtnOnClickListener);

		main_surface.setOnKeyListener(_events);

		_residualvm_thread = new Thread(_residualvm, "ResidualVM");
		_residualvm_thread.start();
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.game_menu, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.show_menu:
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onStart() {
		Log.d(ResidualVM.LOG_TAG, "onStart");

		super.onStart();
	}

	@Override
	public void onResume() {
		Log.d(ResidualVM.LOG_TAG, "onResume");

		super.onResume();

		if (_residualvm != null)
			_residualvm.setPause(false);
	}

	@Override
	public void onPause() {
		Log.d(ResidualVM.LOG_TAG, "onPause");

		super.onPause();

		if (_residualvm != null)
			_residualvm.setPause(true);
	}

	@Override
	public void onStop() {
		Log.d(ResidualVM.LOG_TAG, "onStop");

		super.onStop();
	}

	@Override
	public void onDestroy() {
		Log.d(ResidualVM.LOG_TAG, "onDestroy");

		super.onDestroy();

		if (_events != null) {
			_events.sendQuitEvent();

			try {
				// 1s timeout
				_residualvm_thread.join(1000);
			} catch (InterruptedException e) {
				Log.i(ResidualVM.LOG_TAG, "Error while joining ResidualVM thread", e);
			}

			_residualvm = null;
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTrackballEvent(e);

		return false;
	}

	@Override
	public boolean onTouchEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTouchEvent(e);

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
}
