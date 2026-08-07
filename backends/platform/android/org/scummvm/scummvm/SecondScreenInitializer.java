package org.scummvm.scummvm;

import android.app.Activity;
import android.app.Application;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import androidx.annotation.RequiresApi;

@RequiresApi(api = Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class SecondScreenInitializer implements Application.ActivityLifecycleCallbacks {

	private SecondScreenManager _manager;
	private ImageView _toggleBtn;

	@Override
	public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
		if (!(activity instanceof ScummVMActivity)) return;
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) return;

		ScummVMActivity svm = (ScummVMActivity) activity;

		try {
			java.lang.reflect.Field field = ScummVMActivity.class.getDeclaredField("_scummvm");
			field.setAccessible(true);
			ScummVM scummvm = (ScummVM) field.get(svm);
			if (scummvm == null) return;

			_manager = new SecondScreenManager(activity, scummvm);
			_manager.start();

			_toggleBtn = activity.findViewById(
				activity.getResources().getIdentifier(
					"toggle_second_screen_button", "id", activity.getPackageName()));

			if (_toggleBtn != null && _manager.hasSecondaryDisplay()) {
				_toggleBtn.setVisibility(View.VISIBLE);
				_toggleBtn.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						if (_manager != null) {
							boolean enabled = _manager.toggle();
							_toggleBtn.setAlpha(enabled ? 1.0f : 0.4f);
						}
					}
				});
			}
		} catch (Exception e) {
			// _scummvm field not accessible or not yet initialized
		}
	}

	@Override
	public void onActivityStarted(Activity activity) {}

	@Override
	public void onActivityResumed(Activity activity) {}

	@Override
	public void onActivityPaused(Activity activity) {}

	@Override
	public void onActivityStopped(Activity activity) {}

	@Override
	public void onActivitySaveInstanceState(Activity activity, Bundle outState) {}

	@Override
	public void onActivityDestroyed(Activity activity) {
		if (!(activity instanceof ScummVMActivity)) return;
		if (_manager != null) {
			_manager.stop();
			_manager = null;
		}
	}
}
