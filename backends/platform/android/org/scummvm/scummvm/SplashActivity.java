package org.scummvm.scummvm;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;

public class SplashActivity extends Activity {

	/**
	 * Ids to identify an external storage read (and write) request.
	 * They are app-defined int constants. The callback method gets the result of the request.
	 * Ie. returned in the Activity's onRequestPermissionsResult()
	 */
	private static final int MY_PERMISSION_ALL = 110;

	private static final String[] MY_PERMISSIONS_STR_LIST = {
		Manifest.permission.READ_EXTERNAL_STORAGE,
		Manifest.permission.WRITE_EXTERNAL_STORAGE,
	};


	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		hideSystemUI();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
		    && (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
		        || checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED)
		) {
			requestPermissions(MY_PERMISSIONS_STR_LIST, MY_PERMISSION_ALL);
		} else {
			startActivity(new Intent(this, ScummVMActivity.class));
			finish();
		}
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		if (requestCode == MY_PERMISSION_ALL) {
			int numOfReqPermsGranted = 0;
			// If request is cancelled, the result arrays are empty.
			if (grantResults.length > 0) {
				for (int iterateGrantResult: grantResults) {
					if (iterateGrantResult == PackageManager.PERMISSION_GRANTED) {
						Log.i(ScummVM.LOG_TAG, permissions[0] + " permission was granted at Runtime");
						++numOfReqPermsGranted;
					} else {
						Log.i(ScummVM.LOG_TAG, permissions[0] + " permission was denied at Runtime");
					}
				}
			}

			if (numOfReqPermsGranted != grantResults.length) {
				// permission denied! We won't be able to make use of functionality depending on this permission.
				Toast.makeText(this, "Until permission is granted, some storage locations may be inaccessible for r/w!", Toast.LENGTH_SHORT)
					.show();
			}
		}
		startActivity(new Intent(this, ScummVMActivity.class));
		finish();
	}

	// TODO setSystemUiVisibility is introduced in API 11 and deprecated in API 30 - When we move to API 30 we will have to replace this code
	//	https://developer.android.com/training/system-ui/immersive.html#java
	//
	//  The code sample in the url below contains code to switch between immersive and default mode
	//	https://github.com/android/user-interface-samples/tree/master/AdvancedImmersiveMode
	//  We could do something similar by making it a Global UI option.
	@TargetApi(Build.VERSION_CODES.KITKAT)
	private void hideSystemUI() {
		// Enables regular immersive mode.
		// For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
		// Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
		View decorView = getWindow().getDecorView();
		decorView.setSystemUiVisibility(
			View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
				// Set the content to appear under the system bars so that the
				// content doesn't resize when the system bars hide and show.
				| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
				// Hide the nav bar and status bar
				| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_FULLSCREEN);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			hideSystemUI();
		}
	}

}
