package org.scummvm.scummvm;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
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

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
		    && Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU
		    && (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
		        || checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED)
		) {
			// In Tiramisu (API 33) and above, READ and WRITE external storage permissions have no effect,
			// and they are automatically denied -- onRequestPermissionsResult() will be called without user's input
			requestPermissions(MY_PERMISSIONS_STR_LIST, MY_PERMISSION_ALL);
		} else {
			Intent next = new Intent(this, ScummVMActivity.class);
			next.fillIn(getIntent(), Intent.FILL_IN_ACTION | Intent.FILL_IN_DATA);
			startActivity(next);
			finish();
		}
	}

	@Override
	public void onResume() {
		super.onResume();
		CompatHelpers.HideSystemStatusBar.hide(getWindow());
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		if (requestCode == MY_PERMISSION_ALL) {
			int numOfReqPermsGranted = 0;
			// If request is canceled, the result arrays are empty.
			for (int i = 0; i < grantResults.length; ++i) {
				if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
					Log.i(ScummVM.LOG_TAG, permissions[i] + " permission was granted at Runtime");
					++numOfReqPermsGranted;
				} else {
					Log.i(ScummVM.LOG_TAG, permissions[i] + " permission was denied at Runtime");
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

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			CompatHelpers.HideSystemStatusBar.hide(getWindow());
		}
	}

	@Override
	public void onConfigurationChanged(@NonNull Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

}
