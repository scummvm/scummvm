package org.residualvm.residualvm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import tv.ouya.console.api.OuyaController;

import java.io.File;

public class ResidualVMActivity extends Activity {

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
			dirs[0] = ResidualVMApplication.getLastCacheDir().getPath();
			return dirs;
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
	private Thread _residualvm_thread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		setContentView(R.layout.main);
		OuyaController.init(this);
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
		String savePath = Environment.getExternalStorageDirectory() + "/ResidualVM/Saves/";
		File saveDir = new File(savePath);
		saveDir.mkdirs();
		if (!saveDir.isDirectory()) {
			// If it doesn't work, resort to the internal app path.
			savePath = getDir("saves", MODE_WORLD_READABLE).getPath();
		}

		// Start ResidualVM
		_residualvm = new MyResidualVM(main_surface.getHolder());

		_residualvm.setArgs(new String[] {
			"ResidualVM",
			"--config=" + getFileStreamPath("residualvmrc").getPath(),
			"--path=" + Environment.getExternalStorageDirectory().getPath(),
			"--gui-theme=modern",
			"--savepath=" + savePath
		});

		_events = new ResidualVMEvents(this, _residualvm);

		main_surface.setOnKeyListener(_events);
		main_surface.setOnGenericMotionListener(_events);

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
