package org.scummvm.scummvm;

import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import androidx.annotation.RequiresApi;

class CompatHelpers {
	static class HideSystemStatusBar {

		public static void hide(final Window window) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				HideSystemStatusBarR.hide(window);
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
				HideSystemStatusBarKitKat.hide(window);
			} else {
				HideSystemStatusBarJellyBean.hide(window);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.JELLY_BEAN)
		@SuppressWarnings("deprecation")
		private static class HideSystemStatusBarJellyBean {
			public static void hide(final Window window) {
				View view = window.getDecorView();
				view.setSystemUiVisibility(
					view.getSystemUiVisibility() |
					View.SYSTEM_UI_FLAG_LOW_PROFILE);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.KITKAT)
		@SuppressWarnings("deprecation")
		private static class HideSystemStatusBarKitKat {
			public static void hide(final Window window) {
				View view = window.getDecorView();
				view.setSystemUiVisibility(
					(view.getSystemUiVisibility() & ~View.SYSTEM_UI_FLAG_IMMERSIVE) |
					View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
					View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_FULLSCREEN);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class HideSystemStatusBarR {
			public static void hide(final Window window) {
				WindowInsetsController insetsController = window.getInsetsController();
				insetsController.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
				insetsController.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
			}
		}
	}
}
