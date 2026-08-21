package org.scummvm.scummvm;

import android.app.Activity;
import android.app.Application;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;

public class SecondScreenHelper {

	private static SecondScreenManager sManager;
	private static ImageView sToggleBtn;
	private static Application.ActivityLifecycleCallbacks sLifecycleCallbacks;

	public static void init(Activity activity, ScummVM scummvm) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) return;

		DisplayMetrics dm = new DisplayMetrics();
		WindowManager wm = (WindowManager) activity.getSystemService(Activity.WINDOW_SERVICE);
		if (wm != null) {
			wm.getDefaultDisplay().getRealMetrics(dm);
		}
		int screenW = Math.max(dm.widthPixels, dm.heightPixels);
		int screenH = Math.min(dm.widthPixels, dm.heightPixels);

		sManager = new SecondScreenManager(activity, scummvm, screenW, screenH);
		sManager.start();

		int btnId = activity.getResources().getIdentifier(
			"toggle_second_screen_button", "id", activity.getPackageName());
		if (btnId != 0) {
			sToggleBtn = activity.findViewById(btnId);
		}

		if (sToggleBtn != null && sManager.hasSecondaryDisplay()) {
			sToggleBtn.setVisibility(View.VISIBLE);
			sToggleBtn.setAlpha(1.0f);
			sToggleBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if (sManager != null) {
						boolean enabled = sManager.toggle();
						if (sToggleBtn != null) {
							sToggleBtn.setAlpha(enabled ? 1.0f : 0.4f);
						}
					}
				}
			});

			sManager.toggle();

			sLifecycleCallbacks = new Application.ActivityLifecycleCallbacks() {
				@Override public void onActivityCreated(Activity a, Bundle s) {}
				@Override public void onActivityStarted(Activity a) {}
				@Override
				public void onActivityResumed(Activity a) {
					if (a instanceof ScummVMActivity && sManager != null) {
						sManager.onResume();
					}
				}
				@Override
				public void onActivityPaused(Activity a) {
					if (a instanceof ScummVMActivity && sManager != null) {
						sManager.onPause();
					}
				}
				@Override public void onActivityStopped(Activity a) {}
				@Override public void onActivitySaveInstanceState(Activity a, Bundle o) {}
				@Override public void onActivityDestroyed(Activity a) {}
			};
			activity.getApplication().registerActivityLifecycleCallbacks(sLifecycleCallbacks);
		}
	}

	public static void destroy() {
		if (sManager != null) {
			sManager.stop();
			sManager = null;
		}
		sToggleBtn = null;
		sLifecycleCallbacks = null;
	}
}
