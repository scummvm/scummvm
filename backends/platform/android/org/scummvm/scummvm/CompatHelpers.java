package org.scummvm.scummvm;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.accessibility.AccessibilityEvent;

import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import java.util.Objects;

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

	static class AudioTrackCompat {
		public static class AudioTrackCompatReturn {
			public AudioTrack audioTrack;
			public int bufferSize;
		}

		public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
				return AudioTrackCompatM.make(sample_rate, buffer_size);
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
				return AudioTrackCompatLollipop.make(sample_rate, buffer_size);
			} else {
				return AudioTrackCompatOld.make(sample_rate, buffer_size);
			}
		}

		/**
		 * Support for Android KitKat or lower
		 */
		@SuppressWarnings("deprecation")
		private static class AudioTrackCompatOld {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					AudioManager.STREAM_MUSIC,
					sample_rate,
					AudioFormat.CHANNEL_OUT_STEREO,
					AudioFormat.ENCODING_PCM_16BIT,
					buffer_size,
					AudioTrack.MODE_STREAM);
				ret.bufferSize = buffer_size;
				return ret;
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
		private static class AudioTrackCompatLollipop {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					new AudioAttributes.Builder()
						.setUsage(AudioAttributes.USAGE_MEDIA)
						.setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
						.build(),
					new AudioFormat.Builder()
						.setSampleRate(sample_rate)
						.setEncoding(AudioFormat.ENCODING_PCM_16BIT)
						.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build(),
					buffer_size,
					AudioTrack.MODE_STREAM,
					AudioManager.AUDIO_SESSION_ID_GENERATE);
				ret.bufferSize = buffer_size;
				return ret;
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.M)
		private static class AudioTrackCompatM {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					new AudioAttributes.Builder()
						.setUsage(AudioAttributes.USAGE_MEDIA)
						.setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
						.build(),
					new AudioFormat.Builder()
						.setSampleRate(sample_rate)
						.setEncoding(AudioFormat.ENCODING_PCM_16BIT)
						.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build(),
					buffer_size,
					AudioTrack.MODE_STREAM,
					AudioManager.AUDIO_SESSION_ID_GENERATE);
				// Keep track of the actual obtained audio buffer size, if supported.
				// We just requested 16 bit PCM stereo pcm so there are 4 bytes per frame.
				ret.bufferSize = ret.audioTrack.getBufferSizeInFrames() * 4;
				return ret;
			}
		}
	}

	static class AccessibilityEventConstructor {
		public static AccessibilityEvent make(int eventType) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				return AccessibilityEventConstructorR.make(eventType);
			} else {
				return AccessibilityEventConstructorOld.make(eventType);
			}
		}

		@SuppressWarnings("deprecation")
		private static class AccessibilityEventConstructorOld {
			public static AccessibilityEvent make(int eventType) {
				return AccessibilityEvent.obtain(eventType);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class AccessibilityEventConstructorR {
			public static AccessibilityEvent make(int eventType) {
				return new AccessibilityEvent(eventType);
			}
		}
	}

	static class ShortcutCreator {
		public static Intent createShortcutResultIntent(@NonNull Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
				return ShortcutCreatorO.createShortcutResultIntent(context, id, intent, label, icon, fallbackIconId);
			} else {
				return ShortcutCreatorOld.createShortcutResultIntent(context, id, intent, label, icon, fallbackIconId);
			}
		}

		@SuppressWarnings("deprecation")
		private static class ShortcutCreatorOld {
			public static Intent createShortcutResultIntent(@NonNull Context context, String ignoredId, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				Intent result = new Intent();

				if (icon == null) {
					icon = DrawableCompat.getDrawable(context, fallbackIconId);
				}
				Objects.requireNonNull(icon);
				Bitmap bmIcon = drawableToBitmap(icon);

				addToIntent(result, intent, label, bmIcon);
				return result;
			}

			public static void addToIntent(Intent result, @NonNull Intent intent, @NonNull String label, @NonNull Bitmap icon) {
				result.putExtra(Intent.EXTRA_SHORTCUT_INTENT, intent);
				result.putExtra(Intent.EXTRA_SHORTCUT_NAME, label);
				result.putExtra(Intent.EXTRA_SHORTCUT_ICON, icon);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.O)
		private static class ShortcutCreatorO {
			public static Intent createShortcutResultIntent(Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);
				ShortcutInfo.Builder builder = new ShortcutInfo.Builder(context, id);
				builder.setIntent(intent);
				builder.setShortLabel(label);
				Bitmap bm;
				if (icon != null) {
					bm = drawableToBitmap(icon);
					builder.setIcon(Icon.createWithBitmap(bm));
				} else {
					icon = DrawableCompat.getDrawable(context, fallbackIconId);
					Objects.requireNonNull(icon);
					bm = drawableToBitmap(icon);
					builder.setIcon(Icon.createWithResource(context, fallbackIconId));
				}
				Intent result = shortcutManager.createShortcutResultIntent(builder.build());
				ShortcutCreatorOld.addToIntent(result, intent, label, bm);
				return result;
			}
		}

		private static Bitmap drawableToBitmap(@NonNull Drawable drawable) {
			// We resize to 128x128 to avoid having too big bitmaps for Binder
			if (drawable instanceof BitmapDrawable) {
				Bitmap bm = ((BitmapDrawable)drawable).getBitmap();
				bm = Bitmap.createScaledBitmap(bm, 128, 128, true);
				return bm.copy(bm.getConfig(), false);
			}

			Bitmap bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888);
			Canvas canvas = new Canvas(bitmap);
			drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
			drawable.draw(canvas);

			// Create an immutable bitmap
			return bitmap.copy(bitmap.getConfig(), false);
		}
	}

	static class DrawableCompat {
		public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
				return DrawableCompatLollipop.getDrawable(context, id);
			} else {
				return DrawableCompatOld.getDrawable(context, id);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
		private static class DrawableCompatLollipop {
			@SuppressLint("UseCompatLoadingForDrawables")
			public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
				return context.getDrawable(id);
			}
		}

		@SuppressWarnings("deprecation")
		private static class DrawableCompatOld {
			@SuppressLint("UseCompatLoadingForDrawables")
			public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
				return context.getResources().getDrawable(id);
			}
		}
	}
}
